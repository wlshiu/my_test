/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ev_queue.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/10/09
 * @license
 * @description
 */

#ifndef __ev_queue_H_wPGPuItu_lo4g_HUKd_sZHl_uVfwggdOj2oG__
#define __ev_queue_H_wPGPuItu_lo4g_HUKd_sZHl_uVfwggdOj2oG__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
/* Queue structure */
#define QUEUE_SIZE              16 // power of 2
#define QUEUE_ELEMENTS          (QUEUE_SIZE - 1)
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct queue_handle
{
    int                  queue_buf[QUEUE_SIZE];
    volatile int         queue_in, queue_out;

#if defined(TUP_SIM_HS)
    unsigned char   *pBuf;
    int             cur_len;
#endif
} queue_handle_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void queue_init(queue_handle_t *pHQ);

int queue_push(queue_handle_t *pHQ, int _new);

int queue_pop(queue_handle_t *pHQ, int *_old);

int queue_is_empty(queue_handle_t *pHQ);

#ifdef __cplusplus
}
#endif

#endif
