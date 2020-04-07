/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file rbi.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/07
 * @license
 * @description
 */


#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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
typedef struct rbi_dev
{
    volatile uint32_t   head;
    volatile uint32_t   pick;
    volatile uint32_t   tail;

    uint32_t        slot_size;
    uint32_t        item_size;
    uint32_t        **ppBuf;

    pthread_mutex_t     mtu;
} rbi_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
rbi_t
rbi_init(
    uint32_t    slot_size,
    uint32_t    item_size)
{
    rbi_dev_t   *pDev = 0;

    do {
        if( !slot_size || !item_size )
        {
            break;
        }

        if( !(pDev = malloc(sizeof(rbi_dev_t))) )
        {
            break;
        }
        memset(pDev, 0x0, sizeof(rbi_dev_t));

        item_size = (item_size + 0x3) & ~0x3;

        if( !(pDev->ppBuf = malloc((sizeof(uint32_t*) + item_size) * slot_size)) )
        {
            break;
        }
        memset(pDev->ppBuf, 0x0, (sizeof(uint32_t*) + item_size) * slot_size);

        pDev->slot_size = slot_size;
        pDev->item_size = item_size;
        pDev->ppBuf[0]  = (uint32_t*)((uintptr_t)pDev->ppBuf + (sizeof(uint32_t*) * slot_size));

        for(int i = 1; i < slot_size; i++)
        {
            pDev->ppBuf[i] = (uint32_t*)((uintptr_t)pDev->ppBuf[0] + i * item_size);
        }

        pthread_mutex_init(&pDev->mtu, 0);
    } while(0);

    return (rbi_t)pDev;
}

int
rbi_deinit(rbi_t hRbi)
{
    do {
        rbi_dev_t   *pDev = (rbi_dev_t*)hRbi;

        if( !hRbi )
            break;

        if( pDev->ppBuf )
        {
            pthread_mutex_destroy(&pDev->mtu);

            free(pDev->ppBuf);
        }

        free(pDev);

    } while(0);
    return 0;
}

int
rbi_pop(rbi_t hRbi, uint8_t *pData, int len)
{
    int     length = 0;

    do {
        rbi_dev_t   *pDev = (rbi_dev_t*)hRbi;
        uint32_t    head = 0;
        uint32_t    tail = 0;
        uint8_t     *pCur = 0;

        if( !hRbi )
            break;

        pthread_mutex_lock(&pDev->mtu);

        head = pDev->head;
        tail = pDev->tail;

        pthread_mutex_unlock(&pDev->mtu);

        if( head == tail )
            break;

        pCur   = (uint8_t*)pDev->ppBuf[head];
        length = (len < pDev->item_size) ? len : pDev->item_size;

        memcpy(pData, pCur, length);

        pthread_mutex_lock(&pDev->mtu);

        pDev->head = (head + 1) % pDev->slot_size;

        pthread_mutex_unlock(&pDev->mtu);
    } while(0);

    return length;
}

int
rbi_push(rbi_t hRbi, uint8_t *pData, int len)
{
    int        rval = 0;

    do {
        rbi_dev_t   *pDev = (rbi_dev_t*)hRbi;
        uint32_t    head = 0;
        uint32_t    tail = 0;
        uint32_t    pos = 0;
        uint8_t     *pCur = 0;

        pthread_mutex_lock(&pDev->mtu);

        head = pDev->head;
        tail = pDev->tail;

        pthread_mutex_unlock(&pDev->mtu);

        pos = (tail + 1) % pDev->slot_size;

        if( pos == head )
        {
            rval = -1;
            break;
        }

        pCur = (uint8_t*)pDev->ppBuf[tail];

        memcpy(pCur, pData, (len < pDev->item_size) ? len : pDev->item_size);

        pthread_mutex_lock(&pDev->mtu);

        pDev->tail = pos;

        pthread_mutex_unlock(&pDev->mtu);
    } while(0);

    return rval;
}

