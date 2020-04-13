/**
 * Copyright (C) 2012 Atmel Corporation. All rights reserved.
 */
/** @file nwk.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/13
 * @license
 * @description
 */

#ifndef __nwk_H_wrwLKhVz_liNU_HaZS_sV7V_u66kcydFa99n__
#define __nwk_H_wrwLKhVz_liNU_HaZS_sV7V_u66kcydFa99n__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum
{
    NWK_SUCCESS_STATUS                      = 0x00,
    NWK_ERROR_STATUS                        = 0x01,
    NWK_OUT_OF_MEMORY_STATUS                = 0x02,

    NWK_NO_ACK_STATUS                       = 0x10,

    NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS   = 0x20,
    NWK_PHY_NO_ACK_STATUS                   = 0x21,
} NWK_Status_t;

enum
{
    NWK_OPT_ACK_REQUEST          = 1 << 0,
    NWK_OPT_ENABLE_SECURITY      = 1 << 1,
    NWK_OPT_BROADCAST_PAN_ID     = 1 << 2,
    NWK_OPT_LINK_LOCAL           = 1 << 3,
};

enum
{
    NWK_IND_OPT_ACK_REQUESTED     = 1 << 0,
    NWK_IND_OPT_SECURED           = 1 << 1,
    NWK_IND_OPT_BROADCAST         = 1 << 2,
    NWK_IND_OPT_LOCAL             = 1 << 3,
    NWK_IND_OPT_BROADCAST_PAN_ID  = 1 << 4,
    NWK_IND_OPT_LINK_LOCAL        = 1 << 5,
};

enum
{
    NWK_ACK_CONTROL_NONE         = 0,
    NWK_ACK_CONTROL_PENDING      = 1 << 0,
};
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct NWK_DataReq
{
    // service fields
    void         *next;
    void         *frame;
    uint8_t      state;

    // request parameters
    uint16_t     dstAddr;
    uint8_t      dstEndpoint;
    uint8_t      srcEndpoint;
    uint8_t      options;

    uint8_t      *data;
    uint8_t      size;

    void         (*confirm)(struct NWK_DataReq *req);

    // confirmation parameters
    uint8_t      status;
    uint8_t      control;
} NWK_DataReq_t;

typedef struct NWK_DataInd
{
    uint16_t     srcAddr;
    uint8_t      srcEndpoint;
    uint8_t      dstEndpoint;
    uint8_t      options;
    uint8_t      *data;
    uint8_t      size;
    uint8_t      lqi;
    int8_t       rssi;
} NWK_DataInd_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void NWK_Init(void);
void NWK_SetAddr(uint16_t addr);
void NWK_SetPanId(uint16_t panId);
void NWK_OpenEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind));

#ifdef NWK_ENABLE_SECURITY
void NWK_SetSecurityKey(uint8_t *key);
#endif
bool NWK_Busy(void);
void NWK_SleepReq(void);
void NWK_WakeupReq(void);
void NWK_DataReq(NWK_DataReq_t *req);
void NWK_SetAckControl(uint8_t control);
void NWK_TaskHandler(void);

#ifdef NWK_ENABLE_ROUTING
uint16_t NWK_RouteNextHop(uint16_t dst);
#endif


#ifdef __cplusplus
}
#endif

#endif
