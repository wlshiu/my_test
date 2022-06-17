/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file rbuf.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/03/12
 * @license
 * @description
 */


#include "rbuf.h"
#include <string.h>
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
void
rbi_init(rbi_t *pRB)
{
    pRB->head = pRB->tail = 0;
}

#if 0
static uint32_t
rbi_is_empty(rbi_t *pRB)
{
    uint32_t    head = pRB->head;
    uint32_t    tail = pRB->tail;
    return (head == tail);
}

uint32_t
rbi_is_full(rbi_t *pRB)
{
    uint32_t    head = pRB->head;
    uint32_t    tail = pRB->tail;
    return (((tail + 1) % RBI_SLOT_MAX_LEN) == head);
}
#endif

int
rbi_pop(rbi_t *pRB, uint8_t *pData, int *pLen)
{
    int     rval = 0;
    do {
        uint32_t    head = pRB->head;
        uint32_t    tail = pRB->tail;
        uint8_t     *pCur = 0;
        int         len = *pLen;
        int         data_len = (len < (RBI_BUF_ITEM_LEN - 4)) ? len : (RBI_BUF_ITEM_LEN - 4);

        if( head == tail )
        {
            rval = -1;
            break;
        }

        pCur = (uint8_t*)pRB->buf[head];
        *pLen = *((int*)pCur);

        data_len = (len < (*pLen)) ? len : (*pLen);
        memcpy(pData, pCur + 4, data_len);

        *pLen = data_len;

        pRB->head = (head + 1) % RBI_SLOT_MAX_LEN;

    } while(0);

    return rval;
}

int
rbi_push(rbi_t *pRB, uint8_t *pData, int len)
{
    int        rval = 0;
    do {
        uint32_t    head = pRB->head;
        uint32_t    tail = pRB->tail;
        uint32_t    pos = 0;
        uint8_t     *pCur = 0;
        int         *pRaw_len = 0;
        int         data_len = (len < (RBI_BUF_ITEM_LEN - 4)) ? len : (RBI_BUF_ITEM_LEN - 4);

        pos = (tail + 1) % RBI_SLOT_MAX_LEN;

        if( pos == head )
        {
            rval = -1;
            break;
        }

        pCur = (uint8_t*)pRB->buf[tail];
        memset(pCur, 0x0, RBI_BUF_ITEM_LEN);
        memcpy(pCur + 4, pData, data_len);

        pRaw_len = (int*)pCur;
        *pRaw_len = data_len;

        pRB->tail = pos;
    } while(0);

    return rval;
}

