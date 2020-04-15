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
#define SYS_SECURITY_MODE                   1

#define NWK_BUFFERS_AMOUNT                  3
#define NWK_MAX_ENDPOINTS_AMOUNT            3
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         3000 // ms
#define NWK_ROUTE_TABLE_SIZE                100
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   1000 // ms

// move to environment variables
#if 0
#define NWK_ENABLE_ROUTING
#define NWK_ENABLE_SECURITY
#endif // 0

/*- Definitions ------------------------------------------------------------*/
#ifndef NWK_BUFFERS_AMOUNT
#define NWK_BUFFERS_AMOUNT                       5
#endif

#ifndef NWK_DUPLICATE_REJECTION_TABLE_SIZE
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE       10
#endif

#ifndef NWK_DUPLICATE_REJECTION_TTL
#define NWK_DUPLICATE_REJECTION_TTL              1000 // ms
#endif

#ifndef NWK_ROUTE_TABLE_SIZE
#define NWK_ROUTE_TABLE_SIZE                     10
#endif

#ifndef NWK_ROUTE_DEFAULT_SCORE
#define NWK_ROUTE_DEFAULT_SCORE                  3
#endif

#ifndef NWK_ACK_WAIT_TIME
#define NWK_ACK_WAIT_TIME                        1000 // ms
#endif

#ifndef NWK_GROUPS_AMOUNT
#define NWK_GROUPS_AMOUNT                        10
#endif

#ifndef NWK_ROUTE_DISCOVERY_TABLE_SIZE
#define NWK_ROUTE_DISCOVERY_TABLE_SIZE           5
#endif

#ifndef NWK_ROUTE_DISCOVERY_TIMEOUT
#define NWK_ROUTE_DISCOVERY_TIMEOUT              1000 // ms
#endif

//#define NWK_ENABLE_ROUTING
//#define NWK_ENABLE_SECURITY
//#define NWK_ENABLE_MULTICAST
//#define NWK_ENABLE_ROUTE_DISCOVERY
//#define NWK_ENABLE_SECURE_COMMANDS

#ifndef SYS_SECURITY_MODE
#define SYS_SECURITY_MODE                        0
#endif

#if 0
/*- Sanity checks ----------------------------------------------------------*/
#if defined(NWK_ENABLE_SECURITY) && (SYS_SECURITY_MODE == 0)
  #define PHY_ENABLE_AES_MODULE
#endif
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================
#define PACK        __attribute__ ((packed))
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
