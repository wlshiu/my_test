/**
 * Copyright (C) 2012 Atmel Corporation. All rights reserved.
 */
/** @file nwk_dev.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/13
 * @license
 * @description
 */

#ifndef __nwk_dev_H_wrwLKhVz_liNU_HaZS_sV7V_u66kcydFa99n__
#define __nwk_dev_H_wrwLKhVz_liNU_HaZS_sV7V_u66kcydFa99n__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "nwk_config.h"
#include "nwk.h"
#include "nwkRx.h"
#include "nwkTx.h"
#include "nwkGroup.h"
#include "nwkFrame.h"
#include "nwkRoute.h"
#include "nwkSecurity.h"
#include "nwkRouteDiscovery.h"
#include "sysTimer.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define APP_ADDR                    1
#define APP_CHANNEL                 0x0f
#define APP_PANID                   0x1234
#define APP_ENDPOINT                1
#define APP_SECURITY_KEY            "TestSecurityKey0"
#define APP_FLUSH_TIMER_INTERVAL    20

#define APP_BUFFER_SIZE             (127 - 16/*NwkFrameHeader_t*/ - 2/*crc*/)

typedef enum App_State
{
    APP_STATE_INITIAL,
    APP_STATE_IDLE,
} App_State_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct NwkRouteDiscoveryTableEntry_t
{
    uint16_t   srcAddr;
    uint16_t   dstAddr;
    uint8_t    multicast;
    uint16_t   senderAddr;
    uint8_t    forwardLinkQuality;
    uint8_t    reverseLinkQuality;
    uint16_t   timeout;
} NwkRouteDiscoveryTableEntry_t;

typedef struct NwkDuplicateRejectionEntry_t
{
    uint16_t src;
    uint8_t  seq;
    uint8_t  mask;
    uint8_t  ttl;
} NwkDuplicateRejectionEntry_t;

typedef struct PHY_DataInd_t
{
  uint8_t    *data;
  uint8_t    size;
  uint8_t    lqi;
  int8_t     rssi;
} PHY_DataInd_t;

typedef struct nwk_dev
{
    NwkIb_t         nwkIb;

    // sysTime
    SYS_Timer_t     *timers;

    // frame
    NwkFrame_t      nwkFrameFrames[NWK_BUFFERS_AMOUNT];
    NWK_DataReq_t   *nwkDataReqQueue;

    // route discovery
    NwkRouteDiscoveryTableEntry_t   nwkRouteDiscoveryTable[NWK_ROUTE_DISCOVERY_TABLE_SIZE];
    SYS_Timer_t                     nwkRouteDiscoveryTimer;

    // route
    NWK_RouteTableEntry_t           nwkRouteTable[NWK_ROUTE_TABLE_SIZE];

    // group
    uint16_t        nwkGroups[NWK_GROUPS_AMOUNT];

    // security
    uint8_t         nwkSecurityActiveFrames;
    NwkFrame_t      *nwkSecurityActiveFrame;
    uint8_t         nwkSecuritySize;
    uint8_t         nwkSecurityOffset;
    bool            nwkSecurityEncrypt;
    uint32_t        nwkSecurityVector[4];

    // TX
    NwkFrame_t      *nwkTxPhyActiveFrame;
    SYS_Timer_t     nwkTxAckWaitTimer;
    SYS_Timer_t     nwkTxDelayTimer;

    // RX
    NwkDuplicateRejectionEntry_t    nwkRxDuplicateRejectionTable[NWK_DUPLICATE_REJECTION_TABLE_SIZE];
    uint8_t                         nwkRxAckControl;
    SYS_Timer_t                     nwkRxDuplicateRejectionTimer;

    // APP
    App_State_t      appState;
    SYS_Timer_t      appTimer;
    NWK_DataReq_t    appDataReq;
    bool             appDataReqBusy;
    uint8_t          appDataReqBuffer[APP_BUFFER_SIZE];
    uint8_t          appUartBuffer[APP_BUFFER_SIZE];
    uint8_t          appUartBufferPtr;
} nwk_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern nwk_dev_t    g_nwk_dev;
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
