/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file rbi.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */


#include <string.h>
#include "rbi.h"

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
rbi_init(rbi_t *pRB, uint32_t *pArray, uint32_t slot_size)
{
    pRB->head = pRB->tail = pRB->pick = 0;
    pRB->pBuf      = pArray;
    pRB->slot_size = slot_size;
    pRB->unit_size = sizeof(void*);
    return;
}

uint32_t
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
    return (((tail + 1) % pRB->slot_size) == head);
}


uint32_t
rbi_pick(rbi_t *pRB)
{
    uint32_t    data = 0;

    do {
        uint32_t    pick = pRB->pick;
        uint32_t    tail = pRB->tail;

        if( pick == tail )
            break;

        data = *((uint32_t*)((unsigned long)pRB->pBuf + (pick * pRB->unit_size)));
        pRB->pick = (pick + 1) % pRB->slot_size;
    } while(0);

    return data;
}

uint32_t
rbi_pop(rbi_t *pRB)
{
    uint32_t    data = 0;

    do {
        uint32_t    head = pRB->head;
        uint32_t    tail = pRB->tail;

        if( head == tail )
            break;

        data = *((uint32_t*)((uint32_t)pRB->pBuf + (head * pRB->unit_size)));
        pRB->head = (head + 1) % pRB->slot_size;
    } while(0);

    return data;
}

int
rbi_push(rbi_t *pRB, uint32_t data)
{
    int        rval = 0;
    do {
        uint32_t    head = pRB->head;
        uint32_t    tail = pRB->tail;
        uint32_t    pos = 0;

        pos = (tail + 1) % pRB->slot_size;

        if( pos == head )
        {
            rval = -1;
            break;
        }

        *((uint32_t*)((unsigned long)pRB->pBuf + (tail * pRB->unit_size))) = data;

        pRB->tail = pos;
    } while(0);

    return rval;
}
