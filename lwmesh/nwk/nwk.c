/**
 *  Copyright (C) 2012 Atmel Corporation. All rights reserved.
 */
/** @file nwk.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/13
 * @license
 * @description
 */


#include "nwk.h"

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
NwkIb_t     nwkIb;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void NWK_Init(void)
{
    nwkIb.nwkSeqNum = 0;
    nwkIb.macSeqNum = 0;
    nwkIb.addr = 0;

    for (uint8_t i = 0; i < NWK_MAX_ENDPOINTS_AMOUNT; i++)
        nwkIb.endpoint[i] = NULL;

    nwkTxInit();
    nwkRxInit();
    nwkFrameInit();
    nwkDataReqInit();

    #ifdef NWK_ENABLE_ROUTING
    nwkRouteInit();
    #endif

    #ifdef NWK_ENABLE_SECURITY
    nwkSecurityInit();
    #endif
}

void NWK_SetAddr(uint16_t addr)
{
    nwkIb.addr = addr;
    PHY_SetShortAddr(addr);
}


void NWK_SetPanId(uint16_t panId)
{
    nwkIb.panId = panId;
    PHY_SetPanId(panId);
}


void NWK_OpenEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind))
{
    nwkIb.endpoint[id] = handler;
}

#ifdef NWK_ENABLE_SECURITY


void NWK_SetSecurityKey(uint8_t *key)
{
    memcpy((uint8_t *)nwkIb.key, key, NWK_SECURITY_KEY_SIZE);
}
#endif



bool NWK_Busy(void)
{
    return nwkRxBusy() || nwkTxBusy() || nwkDataReqBusy() || PHY_Busy();
}



void NWK_SleepReq(void)
{
    PHY_Sleep();
}


void NWK_WakeupReq(void)
{
    PHY_Wakeup();
}


void NWK_TaskHandler(void)
{
    nwkRxTaskHandler();
    nwkTxTaskHandler();
    nwkDataReqTaskHandler();
    #ifdef NWK_ENABLE_SECURITY
    nwkSecurityTaskHandler();
    #endif
}