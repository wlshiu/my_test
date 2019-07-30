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
ev_prober_init(void);

void
ev_prober_appcall(void);

bool
ev_prober_is_burn_img(void);

char*
ev_prober_get_pkg_name(void);

int
ev_prober_get_tftp_svr_ip(uip_ipaddr_t *pIpaddr);

uint32_t
ev_prober_get_return_port(void);

#ifdef __cplusplus
}
#endif

#endif


