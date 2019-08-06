/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ev_prober.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/07/18
 * @license
 * @description
 */


#ifndef __ev_prober_H_3c1756c8_ba8f_4ae8_992c_f1794e7a34c8__
#define __ev_prober_H_3c1756c8_ba8f_4ae8_992c_f1794e7a34c8__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
ev_prober_init(uint32_t local_uid);

void
ev_prober_appcall(void);

/**
 *  @brief  ev_prober_is_burn_img()
 *              start burning image or net
 *
 *  @return     true/false
 *
 */
bool
ev_prober_is_burn_img(void);

/**
 *  @brief  ev_prober_has_resp_discovery()
 *              has respond an ack for discovery event
 *
 *  @return     true/false
 *
 */
bool
ev_prober_has_resp_discovery();

char*
ev_prober_get_pkg_name(void);

int
ev_prober_get_tftp_svr_ip(uip_ipaddr_t *pIpaddr);

uint32_t
ev_prober_get_return_port(void);

uint32_t
ev_prober_is_sent(void);

void
ev_prober_send_discovery_ack(int rval);

void
ev_prober_send_finish_ack(int rval);


#ifdef __cplusplus
}
#endif

#endif


