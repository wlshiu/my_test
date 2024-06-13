/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file mempool.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/07/03
 * @license
 * @description
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_TOTAL_MEMPOOL_BYTES           (4ul << 10)

#define MIN_BLOCK_SIZE                      ((uint32_t)(g_link_hdr_size << 1))
#define BITS_PER_BYTE                       8
//=============================================================================
//                  Macro Definition
//=============================================================================
#define BYTE_ALIGN              4ul
#define BYTE_ALIGN_MASK         (BYTE_ALIGN - 1ul)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct blk_link
{
    struct blk_link     *pNext_free_blk;
    uint32_t            blk_size;
} blk_link_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t          g_mempool[CONFIG_TOTAL_MEMPOOL_BYTES];
static const uint32_t   g_link_hdr_size = (sizeof(blk_link_t) + BYTE_ALIGN_MASK) & ~BYTE_ALIGN_MASK;
static blk_link_t       g_blk_start;
static blk_link_t       *g_pBlk_end = 0;

static uint32_t     g_free_bytes = 0U;
static uint32_t     g_free_bytes_min = 0U;  // record the worst-case
static uint32_t     g_malloc_cnt = 0;
static uint32_t     g_free_cnt = 0;
static uint32_t     g_blk_used_flag = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void _mpool_init(void)
{
    blk_link_t  *pBlk_head;
    uint8_t     *pAlign_addr;
    uint32_t    addr = 0;
    uint32_t    total_size = CONFIG_TOTAL_MEMPOOL_BYTES;

    /* Ensure the heap starts on a correctly aligned boundary. */
    addr = (uint32_t)&g_mempool;

    if( (addr & BYTE_ALIGN_MASK) )
    {
        addr = (addr + BYTE_ALIGN_MASK) & ~BYTE_ALIGN_MASK;
        total_size -= (addr - (uint32_t)&g_mempool);
    }

    pAlign_addr = (uint8_t*)addr;

    g_blk_start.pNext_free_blk = (void*)pAlign_addr;
    g_blk_start.blk_size       = 0ul;

    addr = ((uint32_t)pAlign_addr) + total_size;
    addr = (addr - g_link_hdr_size) & ~BYTE_ALIGN_MASK;

    g_pBlk_end = (void*)addr;

    g_pBlk_end->blk_size       = 0;
    g_pBlk_end->pNext_free_blk = 0;

    pBlk_head = (blk_link_t*)pAlign_addr;

    pBlk_head->blk_size       = addr - (uint32_t)pBlk_head;
    pBlk_head->pNext_free_blk = g_pBlk_end;

    g_free_bytes_min = pBlk_head->blk_size;

    g_free_bytes              = pBlk_head->blk_size;

    g_blk_used_flag = (0x1ul) << ((sizeof(uint32_t) * BITS_PER_BYTE) - 1);
    return;
}

static void _add_blk_to_free_list(blk_link_t *pBlk)
{
    blk_link_t  *pIterator;
    uint8_t     *pCur;

    for(pIterator = &g_blk_start;
        pIterator->pNext_free_blk < pBlk;
        pIterator = pIterator->pNext_free_blk)
    {
    }

    pCur = (uint8_t*)pIterator;

    if( (pCur + pIterator->blk_size) == (uint8_t*)pBlk )
    {
        pIterator->blk_size += pBlk->blk_size;
        pBlk                = pIterator;
    }

    pCur = (uint8_t*)pBlk;

    if( (pCur + pBlk->blk_size) == (uint8_t*)pIterator->pNext_free_blk )
    {
        if( pIterator->pNext_free_blk == g_pBlk_end )
            pBlk->pNext_free_blk = g_pBlk_end;
        else
        {
            pBlk->blk_size      += pIterator->pNext_free_blk->blk_size;
            pBlk->pNext_free_blk = pIterator->pNext_free_blk->pNext_free_blk;
        }
    }
    else
    {
        pBlk->pNext_free_blk = pIterator->pNext_free_blk;
    }

    if( pIterator != pBlk )
    {
        pIterator->pNext_free_blk = pBlk;
    }
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void *mpool_malloc(int length)
{
    blk_link_t  *pBlk_cur, *pBlk_prev, *pBlk_next;
    void        *pAddr = 0;

    if( g_pBlk_end == 0 )
    {
        _mpool_init();
    }

    if( (length & g_blk_used_flag) )
        return pAddr;

    if( length > 0 && (length + g_link_hdr_size) > length /* Overflow check */ )
    {
        length += g_link_hdr_size;

        if( (length & BYTE_ALIGN_MASK) )
        {
            if( (length + (BYTE_ALIGN - (length & BYTE_ALIGN_MASK))) > length )
            {
                /* alignment required. Check for overflow. */
                length += (BYTE_ALIGN - (length & BYTE_ALIGN_MASK));
            }
            else
            {
                length = 0;
            }
        }
    }
    else
    {
        length = 0;
    }

    if( length > 0 && length < g_free_bytes )
    {
        pBlk_prev = &g_blk_start;
        pBlk_cur  = g_blk_start.pNext_free_blk;

        while( (pBlk_cur->blk_size < length) && pBlk_cur->pNext_free_blk )
        {
            pBlk_prev = pBlk_cur;
            pBlk_cur  = pBlk_cur->pNext_free_blk;
        }

        if( pBlk_cur != g_pBlk_end )
        {
            pAddr = (void*)(((uint8_t*)pBlk_prev->pNext_free_blk) + g_link_hdr_size);

            pBlk_prev->pNext_free_blk = pBlk_cur->pNext_free_blk;

            if( (pBlk_cur->blk_size - length) > MIN_BLOCK_SIZE )
            {
                pBlk_next = (void*)(((uint8_t*)pBlk_cur) + length);

                pBlk_next->blk_size = pBlk_cur->blk_size - length;
                pBlk_cur->blk_size  = length;

                _add_blk_to_free_list(pBlk_next);
            }

            g_free_bytes -= pBlk_cur->blk_size;

            if( g_free_bytes < g_free_bytes_min )
            {
                g_free_bytes_min = g_free_bytes;
            }

            pBlk_cur->blk_size |= g_blk_used_flag;
            pBlk_cur->pNext_free_blk = 0;

            g_malloc_cnt++;
        }
    }

    return pAddr;
}

void mpool_free(void *pPtr)
{
    blk_link_t  *pBLink;

    if( pPtr == 0 )     return;

    pBLink = (void*)((uint32_t)pPtr - g_link_hdr_size);

    if( (pBLink->blk_size & g_blk_used_flag) == 0 ||
        pBLink->pNext_free_blk )
        return;

    pBLink->blk_size &= ~g_blk_used_flag;

    g_free_bytes += pBLink->blk_size;
    _add_blk_to_free_list(((blk_link_t*)pBLink));
    g_free_cnt++;

    return;
}


int main()
{
    uint32_t        *pAddr[100] = {0};

    srand(time(0));

    for(int i = 0; i < 10000; i++)
    {
        if( rand() & 0x4 )
        {
            for(int j = 0; j < 100; j++)
            {
                int     sz = 0;

                if( pAddr[j] )
                    continue;

                sz = rand() % 75;
                sz = (sz) ? sz : 1;
                pAddr[j] = mpool_malloc(sz);
                if( pAddr[j] == 0  )
                    printf("  malloc %d fail !\n", sz);

                break;
            }
        }
        else
        {
            for(int j = 0; j < 100; j++)
            {
                int     idx = rand() % 100;

                for(int k = 0; k < 100; k++)
                {
                    idx = (k + idx) % 100;
                    if( pAddr[idx] )
                    {
                        mpool_free(pAddr[idx]);
                        pAddr[idx] = 0;
                    }
                }
            }
        }
    }

    for(int j = 0; j < 100; j++)
    {
        if( pAddr[j] )
            printf("%p\n", pAddr[j]);
    }

    system("pause");
    return 0;
}
