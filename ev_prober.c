/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ev_prober.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/07/18
 * @license
 * @description
 */


#include <string.h>
#include "pt.h"
#include "uip_timer.h"
#include "uip_arp.h"
#include "ev_prober.h"
#include "crc32.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_EV_PROBER_PORT         9999

typedef enum ev_prober_state
{
    EV_PROBER_STATE_NONE    = 0,
    EV_PROBER_STATE_CRC_FAIL,
    EV_PROBER_STATE_START_BURN_FW,
} ev_prober_state_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct ev_prober
{
    struct pt           pt;
    struct timer        timer;
    struct uip_udp_conn *conn;

    uint16_t            ticks;
    bool                has_burn_fw;
    char                pkg_name[128];
    uip_ipaddr_t        tftp_ipaddr;

} ev_prober_t;

#define EV_PROBER_UID_BURN_FW       FOUR_CC('n', 'b', 'r', 'n')

#pragma pack(1)
typedef struct ev_msg
{
    /**
     *  uid: 'nbrn'
     */
    uint32_t    uid;

    /**
     *  msg_len = sizeof(struct ev_msg) - 4
     */
    uint16_t    msg_len;

    /**
     *  when burn fw finial, after 'reboot_sec' seconds, system reboot
     */
    uint16_t    reboot_sec;

    /**
     *  F/W image file name (if no data, system will take from DHCP)
     */
    char        pkg_name[128];
    uint32_t    tftp_server_ip;
    uint32_t    crc32;
} ev_msg_t;
#pragma pack()
//=============================================================================
//                  Global Data Definition
//=============================================================================
static ev_prober_t       g_ev_prober;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_ev_prober_parse_msg(void)
{
    int     rval = EV_PROBER_STATE_NONE;
    do {
        ev_msg_t    *pMsg = (ev_msg_t*)uip_appdata;

        if( !uip_newdata() )    break;

        uip_udp_conn->rport = 0;

    #if 0
        {
            int         data_len = uip_datalen();
            char        *pData = (char*)uip_appdata;
            printf("\n\n==========\n");
            for(int i = 0; i < data_len; i++)
                printf("%c", pData[i]);
            printf("\n\n");
        }
    #endif

        // verify CRC32
        if( pMsg->crc32 != calc_crc32((uint8_t*)pMsg, sizeof(ev_msg_t) - 4) )
        {
            rval = EV_PROBER_STATE_CRC_FAIL;
            break;
        }

        #if 0
        {
            char    *pTag = (char*)&pMsg->uid;
            log("uid        = '%c%c%c%c'\n", pTag[0], pTag[1], pTag[2], pTag[3]);
            log("msg_len    = %d\n", pMsg->msg_len);
            log("reboot_sec = %d\n", pMsg->reboot_sec);
            log("pkg_name   = '%s'\n", strlen(pMsg->pkg_name) ? pMsg->pkg_name : "");
            log_ip("tftp ip    = ", &pMsg->tftp_server_ip, "%d\n", __LINE__);
            log("crc32      = 0x%x\n", pMsg->crc32);
        }
        #endif

        if( pMsg->uid != EV_PROBER_UID_BURN_FW )
            break;

        rval = EV_PROBER_STATE_START_BURN_FW;
        g_ev_prober.has_burn_fw = true;

        if( strlen(pMsg->pkg_name) )
            strcpy(g_ev_prober.pkg_name, pMsg->pkg_name);

        if( pMsg->tftp_server_ip )
            memcpy(&g_ev_prober.tftp_ipaddr, &pMsg->tftp_server_ip, sizeof(g_ev_prober.tftp_ipaddr));

    } while(0);
    return rval;
}

PT_THREAD(_ev_prober_handler(void))
{
    PT_BEGIN(&g_ev_prober.pt);

    g_ev_prober.ticks = CLOCK_SECOND;

    do {
        timer_set(&g_ev_prober.timer, g_ev_prober.ticks);
        PT_WAIT_UNTIL(&g_ev_prober.pt, _ev_prober_parse_msg() == EV_PROBER_STATE_START_BURN_FW);

    } while( g_ev_prober.has_burn_fw == false );

    uip_udp_remove(g_ev_prober.conn);

    while(1)
    {
        PT_YIELD(&g_ev_prober.pt);
    }

    PT_END(&g_ev_prober.pt);
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
ev_prober_init(void)
{
    int     rval = 0;
    do {
        uip_ipaddr_t    broadcast_ipaddr;

        memset(&g_ev_prober, 0x0, sizeof(g_ev_prober));
        PT_INIT(&g_ev_prober.pt);

        uip_ipaddr(broadcast_ipaddr, 255, 255, 255, 255);
        g_ev_prober.conn = uip_udp_new(&broadcast_ipaddr, UIP_HTONS(0));
        if( !g_ev_prober.conn )
        {
            log("No available udp connections\n");
            break;
        }

        uip_udp_bind(g_ev_prober.conn, UIP_HTONS(CONFIG_EV_PROBER_PORT));
    } while(0);

    return rval;
}

void
ev_prober_appcall(void)
{
    _ev_prober_handler();
}

bool
ev_prober_is_burn_img(void)
{
    return g_ev_prober.has_burn_fw;
}

char*
ev_prober_get_pkg_name(void)
{
    return (strlen(g_ev_prober.pkg_name)) ? g_ev_prober.pkg_name : 0;
}

int
ev_prober_get_tftp_svr_ip(uip_ipaddr_t *pIpaddr)
{
    int             rval = 0;
    uip_ipaddr_t    zero_ipaddr;
    uip_ipaddr(zero_ipaddr, 0, 0, 0, 0);

    do {
        if( uip_ipaddr_cmp(&g_ev_prober.tftp_ipaddr, &zero_ipaddr) )
        {
            rval = -1;
            break;
        }

        uip_ipaddr_copy(pIpaddr, &g_ev_prober.tftp_ipaddr);
    } while(0);
    return rval;
}
