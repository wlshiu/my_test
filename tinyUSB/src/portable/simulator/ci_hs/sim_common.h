/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sim_common.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/07/23
 * @license
 * @description
 */

#ifndef __sim_common_H_wahb4ang_lKVA_Ha3S_sWWK_uxI9G3baWhfh__
#define __sim_common_H_wahb4ang_lKVA_Ha3S_sWWK_uxI9G3baWhfh__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <assert.h>

#include "queue.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_BUFFER_SIZE      1024

#define MSG_MST_TX          (WM_USER + 100)
#define MSG_MST_RX          (WM_USER + 200)
//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, ...)       do{ printf("[%s:%d][Error] " str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)

#define trace(str, ...)     printf("[%s:%d] " str, __func__, __LINE__, ##__VA_ARGS__)

/**
 *  MCU HAL
 */
#define USB_IRQn                0
#define NVIC_EnableIRQ(a)
#define NVIC_DisableIRQ(a)

#define USB_SIE_STATUS_SPEED_BITS       0x00000300
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct my_msg_info
{
    uint8_t     *pMsg_buf;
    int         msg_len;
} my_msg_info_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern queue_handle_t       g_hQ_H2D; // host tx -> device rx
extern queue_handle_t       g_hQ_D2H; // device tx -> host rx

extern DWORD                g_th_hcd_id;
extern DWORD                g_th_dcd_id;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_sim_send_bytes(
    queue_handle_t  *pHQ,
    uint8_t         *pData,
    int             data_len)
{
    int     rval = 0;
    do {
        if( pHQ->pBuf == 0 )
        {
            if( !(pHQ->pBuf = malloc(CONFIG_BUFFER_SIZE)) )
            {
                rval = -1;
                break;
            }
            memset(pHQ->pBuf, 0x0, CONFIG_BUFFER_SIZE);
            pHQ->cur_len = 0;
        }

        memcpy(&pHQ->pBuf[pHQ->cur_len + 4], pData, data_len);
        pHQ->cur_len += data_len;
        *((int*)pHQ->pBuf) = pHQ->cur_len;

    } while(0);
    return rval;
}

static int
_sim_recv_bytes(
    queue_handle_t  *pHQ,
    uint8_t         *pData,
    int             *pData_len)
{
    int     rval = 0;
    do {
        uint8_t         *pBuf = 0;
        int             data_len = 0;
        int             rx_data_len = 0;

        data_len = *pData_len;
        *pData_len = 0;

        if( queue_is_empty(pHQ))
            break;

        if( (rval = queue_pop(pHQ, (int*)&pBuf)) < 0 )
            break;

        rx_data_len = *((int*)pBuf);

        if( data_len < rx_data_len )
        {
            err("rx data (len= %d) > buffer (len= %d)\n", rx_data_len, data_len);
        }
        else    data_len = rx_data_len;

        memcpy(pData, pBuf + sizeof(int), data_len);
        *pData_len = data_len;

        free(pBuf);
    } while(0);
    return rval;
}

static int
_wait_msg(uint8_t *pBuf, int *pBuf_size)
{
    MSG     msg;

    while(true)
    {
        if( !GetMessage(&msg, 0, 0, 0) ) //get msg from message queue
        {
            printf(" th %ld wait\n", (long)GetCurrentThreadId());
            Sleep(1);
            continue;
        }

        switch( msg.message )
        {
            case MSG_MST_TX:
            case MSG_MST_RX:
                {
                    int             len = *pBuf_size;
                    my_msg_info_t   *pMsg_info = (my_msg_info_t*)msg.wParam;

                    len = (pMsg_info->msg_len > len) ? len : pMsg_info->msg_len;
                    memcpy(pBuf, pMsg_info->pMsg_buf, len);
                }
                return 0;

            default:
                break;
        }
    }
    return 0;
}

static int
_send_msg(DWORD th_id, uint8_t *pBuf, int buf_len)
{
    my_msg_info_t    msg_mst_tx;

    msg_mst_tx.pMsg_buf = pBuf;
    msg_mst_tx.msg_len  = buf_len;
    if( !PostThreadMessage(th_id, MSG_MST_TX, (WPARAM)&msg_mst_tx, 0) ) //post thread msg
    {
        // fail
    }
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
