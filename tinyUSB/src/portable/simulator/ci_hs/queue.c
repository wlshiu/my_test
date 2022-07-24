/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ev_queue.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/10/09
 * @license
 * @description
 */


#include "queue.h"

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

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

void queue_init(queue_handle_t *pHQ)
{
    pHQ->queue_in = pHQ->queue_out = 0;
}

int queue_push(queue_handle_t *pHQ, int _new)
{
    int q_in, q_out;

    q_in  = pHQ->queue_in;
    q_out = pHQ->queue_out;

    if( q_in == ((q_out - 1 + QUEUE_SIZE) & (QUEUE_SIZE - 1)) )
    {
        return -1; /* Queue Full*/
    }

    pHQ->queue_buf[q_in] = _new;

    pHQ->queue_in = (q_in + 1) & (QUEUE_SIZE - 1);

    return 0; // No errors
}

int queue_pop(queue_handle_t *pHQ, int *_old)
{
    int q_in, q_out;

    q_in  = pHQ->queue_in;
    q_out = pHQ->queue_out;

    if( q_in == q_out )
    {
        return -1; /* Queue Empty - nothing to get*/
    }

    *_old = pHQ->queue_buf[q_out];

    pHQ->queue_out = (q_out + 1) & (QUEUE_SIZE - 1);

    return 0; // No errors
}

int
queue_is_empty(queue_handle_t *pHQ)
{
    int q_in, q_out;
    q_in  = pHQ->queue_in;
    q_out = pHQ->queue_out;
    return (q_in == q_out);
}

