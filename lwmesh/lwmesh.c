/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file lwmesh.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/14
 * @license
 * @description
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lwmesh.h"
#include "sysTimer.h"
#include "nwk_dev.h"

#include "log.h"
#include "scheduler.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_LWMESH_MSG_SIZE      256

#define PHY_CRC_SIZE                2
#define PHY_RSSI_BASE_VAL           (-91)

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct payload_info
{
    uint8_t     *pBuf;
    int         buf_len;
} payload_info_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern void PHY_DataInd(PHY_DataInd_t *ind);

static uint32_t     g_act_uid = -1;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_create_job_ctxt(scheduler_job_t *pJob, uint8_t **ppCtxt, int *pCtxt_len)
{
    int     rval = 0;
    do {
        payload_info_t      *pPayload_info = (payload_info_t*)pJob->pExtra_data;

        *ppCtxt    = (uint8_t*)pPayload_info->pBuf;
        *pCtxt_len = pPayload_info->buf_len;
    } while(0);

    return rval;
}

static int
_destroy_job_ctxt(scheduler_job_t *pJob, uint8_t **ppCtxt, int *pCtxt_len)
{
    payload_info_t      *pPayload_info = (payload_info_t*)pJob->pExtra_data;

    free(pPayload_info->pBuf);
    free(pPayload_info);
    return 0;
}

static void
_PHY_DataInd(PHY_DataInd_t *ind)
{
    NwkFrame_t *frame;

    if (0x88 != ind->data[1] || (0x61 != ind->data[0] && 0x41 != ind->data[0]) ||
            ind->size < sizeof(NwkFrameHeader_t))
        return;

    if (NULL == (frame = nwkFrameAlloc()))
        return;

    frame->state   = NWK_RX_STATE_RECEIVED;
    frame->size    = ind->size;
    frame->rx.lqi  = ind->lqi;
    frame->rx.rssi = ind->rssi;
    memcpy(frame->data, ind->data, ind->size);
}

static void _APP_DataConf(NWK_DataReq_t *req);
static void
_APP_SendData(void)
{
    if( g_nwk_dev.appDataReqBusy /*|| !g_nwk_dev.appUartBufferPtr */ )
        return;

    memcpy(g_nwk_dev.appDataReqBuffer, g_nwk_dev.appUartBuffer, g_nwk_dev.appUartBufferPtr);

    //appDataReq.dstAddr = 1-APP_ADDR;
    g_nwk_dev.appDataReq.dstAddr     = 0xffff;
    g_nwk_dev.appDataReq.dstEndpoint = APP_ENDPOINT;
    g_nwk_dev.appDataReq.srcEndpoint = APP_ENDPOINT;
    g_nwk_dev.appDataReq.options     = NWK_OPT_ENABLE_SECURITY;
    g_nwk_dev.appDataReq.data        = g_nwk_dev.appDataReqBuffer;
    g_nwk_dev.appDataReq.size        = g_nwk_dev.appUartBufferPtr;
    g_nwk_dev.appDataReq.confirm     = _APP_DataConf;
    NWK_DataReq(&g_nwk_dev.appDataReq);

    g_nwk_dev.appUartBufferPtr = 0;
    g_nwk_dev.appDataReqBusy = true;
    return;
}

static void
_APP_DataConf(NWK_DataReq_t *req)
{
    g_nwk_dev.appDataReqBusy = false;
    _APP_SendData();
    (void)req;
}


static void
_app_timer_handler(SYS_Timer_t *timer)
{
    _APP_SendData();
    (void)timer;
}

static bool
_APP_DataInd(NWK_DataInd_t *ind)
{
    // console input
    return true;
}

static void
APP_TaskHandler(void)
{
    switch (g_nwk_dev.appState)
    {
        case APP_STATE_INITIAL:
            {
                #if 0
                NWK_SetAddr(APP_ADDR);
                #else
                NWK_SetAddr((uint16_t)(g_act_uid & 0xFFFF));
                #endif // 0

                NWK_SetPanId(APP_PANID);

                // End-point is like port number of network socket
                NWK_OpenEndpoint(APP_ENDPOINT, _APP_DataInd);

                g_nwk_dev.appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
                g_nwk_dev.appTimer.mode     = SYS_TIMER_PERIODIC_MODE;
                g_nwk_dev.appTimer.handler  = _app_timer_handler;

                SYS_TimerStart(&g_nwk_dev.appTimer);

                g_nwk_dev.appState = APP_STATE_IDLE;
            }
            break;

        case APP_STATE_IDLE:
        default:
            break;
    }
    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int lwmesh_init(void)
{
    g_act_uid = -1;
    return 0;
}

int lwmesh_process(uint32_t uid, uint8_t *pData_in, int data_size)
{
    g_act_uid = uid;

    {
        static int  is_nwk_initialized = 0;
        if( !is_nwk_initialized )
        {
            g_nwk_dev.appState = APP_STATE_INITIAL;

            SYS_TimerInit();
            NWK_Init();
            is_nwk_initialized = 1;
        }
    }

    // data input
    if( pData_in && data_size )
    {
        PHY_DataInd_t   ind = {0};
        int8_t          rssi = (rand() % 10) + 26;

        /**
         *  input format:
         *
         *  +------------+-------------+----------+-------------+-----------+----------+
         *  |    MAC     |   Network   |  payload |     MIC     |   CRC16   |   LQI    |
         *  |   header   |    header   |          |             |           |          |
         *  | (9 bytes)  | (7/9 bytes) |          | (0/4 bytes) | (2 bytes) | (1 byte) |
         *  +------------+-------------+----------+-------------+-----------+----------+
         *
         */

        ind.data = pData_in;
        ind.size = data_size - PHY_CRC_SIZE - 1;
        ind.lqi  = pData_in[data_size - 1];
        ind.rssi = rssi + PHY_RSSI_BASE_VAL; // RSSI normal range: -55 ~ -65 dBm
        _PHY_DataInd(&ind);
    }

    NWK_TaskHandler();
    SYS_TimerTaskHandler();
    APP_TaskHandler();
    return 0;
}

int lwmesh_send(uint8_t *pBuf, int length)
{
    int     rval = 0;

    do {
        uint32_t            certificate = (uint32_t)-1;
        scheduler_job_t     *pJob = 0;
        uint32_t            len = sizeof(scheduler_job_t);
        uint8_t             *pPayload = 0;
        payload_info_t      *pPayload_info = 0;

        if( !(pJob = malloc(len)) )
        {
            dbg_msg("fail\n");
            break;
        }
        memset(pJob, 0x0, len);

        if( !(pPayload_info = malloc(sizeof(payload_info_t))) )
        {
            dbg_msg("fail\n");
            break;
        }

        if( !(pPayload = malloc(length)) )
        {
            dbg_msg("fail\n");
            break;
        }
        memcpy(pPayload, pBuf, length);

        pPayload_info->pBuf    = pPayload;
        pPayload_info->buf_len = length;

        pJob->ev_type               = SCHEDULER_EV_MODE_INTERVAL;
        pJob->wait_time             = 0; // real waiting time = 2 * time_scaling (ms)
        pJob->src_uid               = g_act_uid;
        pJob->cb_create_job_ctxt    = _create_job_ctxt;
        pJob->cb_destroy_job_ctxt   = _destroy_job_ctxt;
        pJob->pExtra_data           = (void*)pPayload_info;
        pJob->destination_cnt       = SCHEDULER_DESTINATION_ALL;

        rval = scheduler_add_job(pJob, &certificate);
        if( rval )
            printf("job full...\n");

        free(pJob);
    } while(0);
    return rval;
}
