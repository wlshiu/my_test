/**
 * Copyright (C) 2012 Atmel Corporation. All rights reserved.
 */
/** @file nwk_data_req.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/13
 * @license
 * @description
 */


#include "nwk_data_req.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nwk_private.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
enum
{
    NWK_DATA_REQ_STATE_INITIAL,
    NWK_DATA_REQ_STATE_WAIT_CONF,
    NWK_DATA_REQ_STATE_CONFIRM,
};

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
static void nwkDataReqSendFrame(NWK_DataReq_t *req)
{
    NwkFrame_t *frame;
    uint8_t size = req->size;

    #ifdef NWK_ENABLE_SECURITY
    if (req->options & NWK_OPT_ENABLE_SECURITY)
        size += NWK_SECURITY_MIC_SIZE;
    #endif

    if (NULL == (frame = nwkFrameAlloc(size)))
    {
        req->state = NWK_DATA_REQ_STATE_CONFIRM;
        req->status = NWK_OUT_OF_MEMORY_STATUS;
        return;
    }

    req->frame = frame;
    req->state = NWK_DATA_REQ_STATE_WAIT_CONF;

    frame->tx.confirm = nwkDataReqTxConf;
    frame->tx.control = req->options & NWK_OPT_BROADCAST_PAN_ID ? NWK_TX_CONTROL_BROADCAST_PAN_ID : 0;

    frame->data.header.nwkFcf.ackRequest = req->options & NWK_OPT_ACK_REQUEST ? 1 : 0;
    #ifdef NWK_ENABLE_SECURITY
    frame->data.header.nwkFcf.securityEnabled = req->options & NWK_OPT_ENABLE_SECURITY ? 1 : 0;
    #endif
    frame->data.header.nwkFcf.linkLocal = req->options & NWK_OPT_LINK_LOCAL ? 1 : 0;
    frame->data.header.nwkFcf.reserved = 0;
    frame->data.header.nwkSeq = ++nwkIb.nwkSeqNum;
    frame->data.header.nwkSrcAddr = nwkIb.addr;
    frame->data.header.nwkDstAddr = req->dstAddr;
    frame->data.header.nwkSrcEndpoint = req->srcEndpoint;
    frame->data.header.nwkDstEndpoint = req->dstEndpoint;

    memcpy(frame->data.payload, req->data, req->size);

    nwkTxFrame(frame);
}


static void nwkDataReqTxConf(NwkFrame_t *frame)
{
    for (NWK_DataReq_t *req = nwkDataReqQueue; req; req = req->next)
    {
        if (req->frame == frame)
        {
            req->status = frame->tx.status;
            req->control = frame->tx.control;
            req->state = NWK_DATA_REQ_STATE_CONFIRM;
            break;
        }
    }

    nwkFrameFree(frame);
}


static void nwkDataReqConfirm(NWK_DataReq_t *req)
{
    if (nwkDataReqQueue == req)
    {
        nwkDataReqQueue = nwkDataReqQueue->next;
    }
    else
    {
        NWK_DataReq_t *prev = nwkDataReqQueue;
        while (prev->next != req)
            prev = prev->next;
        prev->next = ((NWK_DataReq_t *)prev->next)->next;
    }

    req->confirm(req);
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void nwkDataReqInit(void)
{
    nwkDataReqQueue = NULL;
}


void NWK_DataReq(NWK_DataReq_t *req)
{
    req->state = NWK_DATA_REQ_STATE_INITIAL;
    req->status = NWK_SUCCESS_STATUS;
    req->frame = NULL;

    if (NULL == nwkDataReqQueue)
    {
        req->next = NULL;
        nwkDataReqQueue = req;
    }
    else
    {
        req->next = nwkDataReqQueue;
        nwkDataReqQueue = req;
    }
}

bool nwkDataReqBusy(void)
{
    return NULL != nwkDataReqQueue;
}


void nwkDataReqTaskHandler(void)
{
    for (NWK_DataReq_t *req = nwkDataReqQueue; req; req = req->next)
    {
        switch (req->state)
        {
            case NWK_DATA_REQ_STATE_INITIAL:
                {
                    nwkDataReqSendFrame(req);
                    return;
                }
                break;

            case NWK_DATA_REQ_STATE_WAIT_CONF:
                break;

            case NWK_DATA_REQ_STATE_CONFIRM:
                {
                    nwkDataReqConfirm(req);
                    return;
                }
                break;

            default:
                break;
        };
    }
}