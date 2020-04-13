/**
 * Copyright (C) 2012 Atmel Corporation. All rights reserved.
 */
/** @file nwk_config.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/13
 * @license
 * @description
 */

#ifndef __nwk_config_H_wlvwjgqh_lk0d_H3N6_sGl9_umsVlU9mbUF1__
#define __nwk_config_H_wlvwjgqh_lk0d_H3N6_sGl9_umsVlU9mbUF1__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
#ifndef NWK_BUFFERS_AMOUNT
#define NWK_BUFFERS_AMOUNT                       1
#endif

#ifndef NWK_MAX_ENDPOINTS_AMOUNT
#define NWK_MAX_ENDPOINTS_AMOUNT                 1
#endif

#ifndef NWK_DUPLICATE_REJECTION_TABLE_SIZE
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE       1
#endif

#ifndef NWK_DUPLICATE_REJECTION_TTL
#define NWK_DUPLICATE_REJECTION_TTL              1000 // ms
#endif

#ifndef NWK_ROUTE_TABLE_SIZE
#define NWK_ROUTE_TABLE_SIZE                     0
#endif

#ifndef NWK_ROUTE_DEFAULT_SCORE
#define NWK_ROUTE_DEFAULT_SCORE                  3
#endif

#ifndef NWK_ACK_WAIT_TIME
#define NWK_ACK_WAIT_TIME                        1000 // ms
#endif

//#define NWK_ENABLE_ROUTING
//#define NWK_ENABLE_SECURITY

#ifndef SYS_SECURITY_MODE
#define SYS_SECURITY_MODE                        0
#endif

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

#ifdef __cplusplus
}
#endif

#endif
