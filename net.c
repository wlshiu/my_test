/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file net_burn.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/07/11
 * @license
 * @description
 */


#include <string.h>
#include "pt.h"
#include "uip_timer.h"
#include "uip_arp.h"
#include "net.h"
#include "ev_prober.h"
#include "dhcpc.h"
#include "tftp.h"
#include "crc32.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_TIMEOUT_SEC          (CLOCK_CONF_SECOND * 500)

#define CONFIG_FW_FILE_NAME         "ggg.jpg"

#if 0
    #define TFTP_IP1        127
    #define TFTP_IP2        0
    #define TFTP_IP3        0
    #define TFTP_IP4        1
#else
    #define TFTP_IP1        192
    #define TFTP_IP2        168
    #define TFTP_IP3        56
    #define TFTP_IP4        3
#endif


#if 1

    #define MY_MAC1        0xaa
    #define MY_MAC2        0xbb
    #define MY_MAC3        0xcc
    #define MY_MAC4        0xdd
    #define MY_MAC5        0x11
    #define MY_MAC6        0x22
#else
    #define MY_MAC1        0x0a
    #define MY_MAC2        0x00
    #define MY_MAC3        0x27
    #define MY_MAC4        0x00
    #define MY_MAC5        0x00
    #define MY_MAC6        0x0e
#endif

#define NET_BURNING_RST_OK          0
#define NET_BURNING_RST_FAIL        -1

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct net_mgr
{
    struct uip_udp_conn     *conn;
    struct pt           pt;
    struct timer        timer;

    uint16_t            retries;
    uint16_t            is_finish;
    int                 rval;

    uint32_t            uid[3];
} net_mgr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static net_mgr_t                g_net_mgr = {0};
static lc_t                     g_net_act[NET_ACT_ALL] = {0};
static struct uip_eth_addr      g_mac_addr = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
PT_THREAD(_net_handler(void))
{
    PT_BEGIN(&g_net_mgr.pt);

    g_net_mgr.is_finish = false;

    g_net_act[NET_ACT_UPGRADE_EVENT] = g_net_mgr.pt.lc;

    {   // receive broadcast packet for upgrading
        net_app__set_callback(ev_prober_appcall);

        ev_prober_init(g_net_mgr.uid[2]);

        do {
            timer_set(&g_net_mgr.timer, CLOCK_SECOND / 100);
            PT_WAIT_UNTIL(&g_net_mgr.pt, timer_expired(&g_net_mgr.timer));
            if( ev_prober_has_resp_discovery() )
            {
                // send ack of discovery if necessary
                ev_prober_send_discovery_ack(0);
                PT_WAIT_UNTIL(&g_net_mgr.pt, ev_prober_is_sent());
            }
        } while( !ev_prober_is_burn_img() );
    }

    PT_WAIT_UNTIL(&g_net_mgr.pt, 1);
    g_net_act[NET_ACT_DHCP] = g_net_mgr.pt.lc;

    {   // DHCP handle
        net_app__set_callback(dhcpc_appcall);

        PT_WAIT_UNTIL(&g_net_mgr.pt, (dhcpc_init(&g_mac_addr, g_net_mgr.uid[2]) == DHCPC_ERR_OK));

        PT_WAIT_UNTIL(&g_net_mgr.pt, !dhcpc_is_busy());
    }

    PT_WAIT_UNTIL(&g_net_mgr.pt, 1);
    g_net_act[NET_ACT_TFTPC] = g_net_mgr.pt.lc;
    g_net_mgr.retries = 3;
    g_net_mgr.rval    = NET_BURNING_RST_FAIL;

    do {   // TFTP handle
        int             rval = 0;
        uip_ipaddr_t    tftp_svr_ipaddr;
        char            *pFw_name = 0;

        if( (rval = ev_prober_get_tftp_svr_ip(&tftp_svr_ipaddr)) )
            rval = dhcpc_get_tftp_server(&tftp_svr_ipaddr);

        if( !(pFw_name = ev_prober_get_pkg_name()) )
            pFw_name = dhcpc_get_boot_filename();

        if( rval != DHCPC_ERR_OK || !pFw_name )
        {
            g_net_mgr.pt.lc = g_net_act[NET_ACT_DHCP];
            return PT_YIELDED;
        }

    #if 0
        {
            uip_ipaddr_t    ipaddr;
            uip_gethostaddr(&ipaddr);
            log("\n\n====================\n");
            log_ip("local ip: ", ipaddr, " ln.%d\n", __LINE__);
            log_ip("tftp-svr: ", tftp_svr_ipaddr, "=> GET %s\n", pFw_name);
        }
    #endif // 0

        net_app__set_callback(tftpc_appcall);
        tftpc_init();

        tftpc_get(&tftp_svr_ipaddr, pFw_name);

        PT_WAIT_UNTIL(&g_net_mgr.pt, !tftpc_is_busy());

        if( tftpc_get_result() == TFTPC_ERR_OK )
        {
            g_net_mgr.rval = NET_BURNING_RST_OK;
            break;
        }
    } while( --g_net_mgr.retries );

    {   // release IP for others
        net_app__set_callback(dhcpc_appcall);
        dhcpc_release_ip();
        PT_WAIT_UNTIL(&g_net_mgr.pt, !dhcpc_is_busy());
    }

    {   // report to host
        net_app__set_callback(ev_prober_appcall);
        ev_prober_send_finish_ack(g_net_mgr.rval);
        PT_WAIT_UNTIL(&g_net_mgr.pt, ev_prober_is_sent());
    }

    PT_RESTART(&g_net_mgr.pt);

    while(1) {
        PT_YIELD(&g_net_mgr.pt);
    }

    PT_END(&g_net_mgr.pt);
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void net_init(void)
{
    do {
        g_mac_addr.addr[0] = MY_MAC1;
        g_mac_addr.addr[1] = MY_MAC2;
        g_mac_addr.addr[2] = MY_MAC3;
        g_mac_addr.addr[3] = MY_MAC4;
        g_mac_addr.addr[4] = MY_MAC5;
        g_mac_addr.addr[5] = MY_MAC6;

        g_net_mgr.uid[2] = calc_crc32((uint8_t*)&g_mac_addr, sizeof(g_mac_addr));
        uip_setethaddr(g_mac_addr);

        memset(&g_net_mgr, 0x0, sizeof(g_net_mgr));

        PT_INIT(&g_net_mgr.pt);
    } while(0);
    return;
}

void net_proc(void)
{
    _net_handler();
    return;
}
