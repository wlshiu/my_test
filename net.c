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


#include "pt.h"
#include "uip_timer.h"
#include "uip_arp.h"
#include "net.h"
#include "dhcpc.h"
#include "tftp.h"
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
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct net_mgr
{
    struct pt       pt;
    struct timer    timer;

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
//    uint16_t        retries = 0;

    PT_BEGIN(&g_net_mgr.pt);

    g_net_act[NET_ACT_UPGRADE_EVENT] = g_net_mgr.pt.lc;
    // receive broadcast packet for upgrading

    #if 0
    PT_WAIT_UNTIL(&g_net_mgr.pt, is_upgrade_event());
    #endif // 0

    g_net_act[NET_ACT_DHCP] = g_net_mgr.pt.lc;

    {   // DHCP handle
        net_app__set_callback(dhcpc_appcall);

        PT_WAIT_UNTIL(&g_net_mgr.pt, (dhcpc_init(&g_mac_addr) == DHCPC_ERR_OK));

        PT_WAIT_UNTIL(&g_net_mgr.pt, !dhcpc_is_busy());
    }

    g_net_act[NET_ACT_TFTPC] = g_net_mgr.pt.lc;

#if 0
    {   // TFTP handle
        uip_ipaddr_t    svr_addr;

        net_app__set_callback(tftpc_appcall);
        tftpc_init();

        uip_ipaddr(&svr_addr, TFTP_IP1, TFTP_IP2, TFTP_IP3, TFTP_IP4);
        tftpc_get(&svr_addr, CONFIG_FW_FILE_NAME);

        PT_WAIT_UNTIL(&g_net_mgr.pt, !tftpc_is_busy());
    }
#endif // 0

    #if 0
    timer_set(&g_net_mgr.timer, CONFIG_TIMEOUT_SEC);
    PT_WAIT_UNTIL(&g_net_mgr.pt, timer_expired(&g_net_mgr.timer));
    #endif // 0

    while(1)
    {
        PT_YIELD(&g_net_mgr.pt);
    }

    PT_END(&g_net_mgr.pt);
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void net_init(void)
{
    g_mac_addr.addr[0] = MY_MAC1;
    g_mac_addr.addr[1] = MY_MAC2;
    g_mac_addr.addr[2] = MY_MAC3;
    g_mac_addr.addr[3] = MY_MAC4;
    g_mac_addr.addr[4] = MY_MAC5;
    g_mac_addr.addr[5] = MY_MAC6;

    uip_setethaddr(g_mac_addr);

    PT_INIT(&g_net_mgr.pt);
    return;
}

void net_proc(void)
{
    _net_handler();
    return;
}
