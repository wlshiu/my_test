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

//=============================================================================
//                  Public Function Definition
//=============================================================================
int lwmesh_init(void)
{
    g_act_uid = -1;
    SYS_TimerInit();
    NWK_Init();
    return 0;
}

int lwmesh_process(uint32_t uid, uint8_t *pData_in, int data_size)
{
    g_act_uid = uid;

    NWK_TaskHandler();
    SYS_TimerTaskHandler();
    return 10;
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
