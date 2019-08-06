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

typedef enum ev_prober_rval
{
    EV_PROBER_RVAL_NONE    = 0,
    EV_PROBER_RVAL_CRC_FAIL,
    EV_PROBER_RVAL_START_DISCOVERY,
    EV_PROBER_RVAL_START_BURN_FW,
} ev_prober_rval_t;
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
    bool                has_ack_discovery;
    uint32_t            return_port;
    char                pkg_name[128];
    uip_ipaddr_t        tftp_ipaddr;

} ev_prober_t;


#define EV_PROBER_TAG_DISCOVERY     FOUR_CC('d', 'v', 'r', 'y')
#define EV_PROBER_TAG_BURN_FW       FOUR_CC('n', 'b', 'r', 'n')
#define EV_PROBER_TAG_FINISH        FOUR_CC('f', 'n', 's', 'h')

#pragma pack(1)
typedef struct ev_msg
{
    /**
     *  uid: 'nbrn' or 'dvry'
     */
    uint32_t    tag;

    /**
     *  msg_len
     */
    uint16_t    msg_len;

    /**
     *  the port number to report to host after net burning
     */
    uint16_t    return_port;

} ev_msg_t;

typedef struct ev_msg_discovery
{
    ev_msg_t    ev_msg;
    char        meta[128];
    uint32_t    crc32;
} ev_msg_discovery_t;


typedef struct ev_msg_burn
{
    ev_msg_t    ev_msg;

    /**
     *  when burn fw finial, after 'reboot_sec' seconds, system reboot
     */
    uint32_t    reboot_sec;

    /**
     *  F/W image file name (if no data, system will take from DHCP)
     */
    char        pkg_name[128];
    uint32_t    tftp_server_ip;

    /**
     *  the target node (the unique id in every device)
     *  if target_dest == 0xFFFFFFFF, it means all nodes.
     */
    uint32_t    target_uid;

    uint32_t    crc32;
} ev_msg_burn_t;

typedef struct ev_msg_ack
{
    uint32_t    tag;
    uint32_t    local_id;
    int         result;
    uint8_t     mac_addr[6];
} ev_msg_ack_t;
#pragma pack()

typedef struct ev_prober_ack
{
    struct pt           pt;
    struct timer        timer;
    struct uip_udp_conn *conn;

    uint16_t            ticks;
    uint32_t            is_sending;
    ev_msg_ack_t        msg_ack;
} ev_prober_ack_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static ev_prober_t      g_ev_prober;
static ev_prober_ack_t  g_ev_ack;
static uint32_t         g_ev_prober_is_busy = 0;
static uint32_t         g_ev_prober_is_sending = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_ev_prober_parse_msg(void)
{
    int     rval = EV_PROBER_RVAL_NONE;
    do {
        ev_msg_t    *pMsg = (ev_msg_t*)uip_appdata;
        uint32_t    *pCRC32 = 0;

        if( !uip_newdata() )    break;

        uip_udp_conn->rport = 0;

        // verify CRC32
        pCRC32 = (uint32_t*)((uintptr_t)pMsg + pMsg->msg_len);
        if( *pCRC32 != calc_crc32((uint8_t*)pMsg, pMsg->msg_len) )
        {
            rval = EV_PROBER_RVAL_CRC_FAIL;
            break;
        }

        if( pMsg->tag == EV_PROBER_TAG_BURN_FW )
        {
            ev_msg_burn_t   *pMsg_burn = (ev_msg_burn_t*)pMsg;

            if( pMsg_burn->target_uid != 0xFFFFFFFF &&
                pMsg_burn->target_uid != g_ev_ack.msg_ack.local_id )
                break;

            #if 1
            {
                char    *pTag = (char*)&pMsg_burn->ev_msg.tag;
                printf("\n");
                log("tag         = '%c%c%c%c'\n", pTag[0], pTag[1], pTag[2], pTag[3]);
                log("msg_len     = %d\n", pMsg_burn->ev_msg.msg_len);
                log("reboot_sec  = %d\n", pMsg_burn->reboot_sec);
                log("pkg_name    = '%s'\n", strlen(pMsg_burn->pkg_name) ? pMsg_burn->pkg_name : "");
                log_ip("tftp ip    = ", &pMsg_burn->tftp_server_ip, "%d\n", __LINE__);
                log("target_uid  = x%08x\n", pMsg_burn->target_uid);
                log("return_port = %d\n", pMsg_burn->ev_msg.return_port);
                log("crc32       = 0x%x\n", pMsg_burn->crc32);
            }
            #endif

            rval = EV_PROBER_RVAL_START_BURN_FW;
            g_ev_prober.has_burn_fw     = true;
            g_ev_prober.return_port     = pMsg_burn->ev_msg.return_port;

            g_ev_ack.conn->rport = UIP_HTONS(g_ev_prober.return_port);

            if( strlen(pMsg_burn->pkg_name) )
                strcpy(g_ev_prober.pkg_name, pMsg_burn->pkg_name);

            if( pMsg_burn->tftp_server_ip )
                memcpy(&g_ev_prober.tftp_ipaddr, &pMsg_burn->tftp_server_ip, sizeof(g_ev_prober.tftp_ipaddr));

            g_ev_prober_is_busy = 0;
        }
        else if( pMsg->tag == EV_PROBER_TAG_DISCOVERY )
        {
            ev_msg_discovery_t  *pMsg_discovery = (ev_msg_discovery_t*)pMsg;

            #if 1
            {
                char    *pTag = (char*)&pMsg_discovery->ev_msg.tag;
                printf("\n");
                log("tag         = '%c%c%c%c'\n", pTag[0], pTag[1], pTag[2], pTag[3]);
                log("msg_len     = %d\n", pMsg_discovery->ev_msg.msg_len);
                log("return_port = %d\n", pMsg_discovery->ev_msg.return_port);
                log("meta        = %s\n", pMsg_discovery->meta);
                log("crc32       = 0x%x\n", pMsg_discovery->crc32);
            }
            #endif // 1

            rval = EV_PROBER_RVAL_START_DISCOVERY;
            g_ev_prober.has_ack_discovery   = true;
            g_ev_prober.return_port         = pMsg_discovery->ev_msg.return_port;

            g_ev_ack.conn->rport = UIP_HTONS(g_ev_prober.return_port);
        }

    } while(0);
    return rval;
}

PT_THREAD(_ev_prober_handler(void))
{
    PT_BEGIN(&g_ev_prober.pt);

    g_ev_prober.ticks = CLOCK_SECOND;

    do {
        timer_set(&g_ev_prober.timer, g_ev_prober.ticks);
        PT_WAIT_UNTIL(&g_ev_prober.pt, _ev_prober_parse_msg() == EV_PROBER_RVAL_START_BURN_FW);

    } while( g_ev_prober.has_burn_fw == false );

    uip_udp_remove(g_ev_prober.conn);

    while(1) {
        PT_YIELD(&g_ev_prober.pt);
    }

    PT_END(&g_ev_prober.pt);
}

PT_THREAD(_ev_prober_ack(void))
{
    PT_BEGIN(&g_ev_ack.pt);

    g_ev_ack.ticks = CLOCK_SECOND;

    timer_set(&g_ev_ack.timer, g_ev_ack.ticks);

    PT_WAIT_UNTIL(&g_ev_ack.pt, g_ev_prober_is_sending);

    {
        ev_msg_ack_t  *msg = (ev_msg_ack_t*)uip_appdata;
        msg->tag        = g_ev_ack.msg_ack.tag;
        msg->local_id   = g_ev_ack.msg_ack.local_id;
        msg->result     = g_ev_ack.msg_ack.result;
        uip_udp_send(sizeof(ev_msg_ack_t));

        g_ev_prober_is_sending = 0;
    }

    PT_RESTART(&g_ev_ack.pt);

    while(1) {
        PT_YIELD(&g_ev_ack.pt);
    }

    PT_END(&g_ev_ack.pt);
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
ev_prober_init(uint32_t local_uid)
{
    int     rval = 0;
    do {
        uip_ipaddr_t    broadcast_ipaddr;

        memset(&g_ev_prober, 0x0, sizeof(g_ev_prober));
        memset(&g_ev_ack, 0x0, sizeof(g_ev_ack));
        PT_INIT(&g_ev_prober.pt);

        uip_ipaddr(broadcast_ipaddr, 255, 255, 255, 255);
        g_ev_prober.conn = uip_udp_new(&broadcast_ipaddr, UIP_HTONS(0));
        if( !g_ev_prober.conn )
        {
            log("No available udp connections\n");
            break;
        }

        uip_udp_bind(g_ev_prober.conn, UIP_HTONS(CONFIG_EV_PROBER_PORT));

        g_ev_ack.msg_ack.local_id = local_uid;

        if(  !g_ev_ack.conn )
        {
            g_ev_ack.conn = uip_udp_new(&broadcast_ipaddr, UIP_HTONS(0));
            if( !g_ev_ack.conn )
            {
                log("No available udp-tx connections\n");
                break;
            }
        }

        g_ev_prober_is_busy = 1;
    } while(0);

    return rval;
}

void
ev_prober_appcall(void)
{
    if( UIP_HTONS(uip_udp_conn->lport) == CONFIG_EV_PROBER_PORT )
        _ev_prober_handler();
    else if( uip_udp_conn->rport == UIP_HTONS(g_ev_prober.return_port) )
        _ev_prober_ack();

    return;
}

bool
ev_prober_is_burn_img(void)
{
    return g_ev_prober.has_burn_fw;
}

bool
ev_prober_has_resp_discovery()
{
    return g_ev_prober.has_ack_discovery;
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

uint32_t
ev_prober_get_return_port(void)
{
    return g_ev_prober.return_port;
}

uint32_t
ev_prober_is_sent(void)
{
    return !g_ev_prober_is_sending;
}

void
ev_prober_send_discovery_ack(int rval)
{
    g_ev_ack.msg_ack.tag    = EV_PROBER_TAG_DISCOVERY;
    g_ev_ack.msg_ack.result = rval;
    g_ev_prober_is_sending = 1;

    g_ev_prober.has_ack_discovery = false;
    return;
}

void
ev_prober_send_finish_ack(int rval)
{
    g_ev_ack.msg_ack.tag    = EV_PROBER_TAG_FINISH;
    g_ev_ack.msg_ack.result = rval;
    g_ev_prober_is_sending = 1;
    return;
}
