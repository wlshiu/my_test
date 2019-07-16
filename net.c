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
#include "net.h"
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
static net_mgr_t        g_net_mgr = {0};
static lc_t             g_net_act[NET_ACT_ALL] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
PT_THREAD(_net_handler(void))
{
    uip_ipaddr_t    svr_addr;

    PT_BEGIN(&g_net_mgr.pt);

    g_net_act[NET_ACT_UPGRADE_EVENT] = g_net_mgr.pt.lc;
    // receive broadcast packet for upgrading

    #if 0
    PT_WAIT_UNTIL(&g_net_mgr.pt, dhcp_done());
    #endif // 0

    g_net_act[NET_ACT_DHCP] = g_net_mgr.pt.lc;

    // DHCP handle

    #if 0
    PT_WAIT_UNTIL(&g_net_mgr.pt, dhcp_done());
    #endif // 0

    g_net_act[NET_ACT_TFTPC] = g_net_mgr.pt.lc;

    // TFTP handle
    net_app__set_callback(tftpc_appcall);
    tftpc_init();

    uip_ipaddr(&svr_addr, TFTP_IP1, TFTP_IP2, TFTP_IP3, TFTP_IP4);
    tftpc_get(&svr_addr, CONFIG_FW_FILE_NAME);

    PT_WAIT_UNTIL(&g_net_mgr.pt, !tftpc_is_busy());

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
    PT_INIT(&g_net_mgr.pt);
    return;
}

void net_proc(void)
{
    _net_handler();
    return;
}
