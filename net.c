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

#define CONFIG_FW_FILE_NAME         "tftp_patt.bin"
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
    PT_BEGIN(&g_net_mgr.pt);

    g_net_act[NET_ACT_DHCP] = g_net_mgr.pt.lc;

    // DHCP handle

    #if 0
    PT_WAIT_UNTIL(&g_net_mgr.pt, dhcp_done());
    #endif // 0

    g_net_act[NET_ACT_TFTPC] = g_net_mgr.pt.lc;

    // TFTP handle
    net_app__set_callback(tftpc_appcall);
    tftpc_init();
    tftpc_get(CONFIG_FW_FILE_NAME);

    PT_WAIT_UNTIL(&g_net_mgr.pt, !tftpc_busy());

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
