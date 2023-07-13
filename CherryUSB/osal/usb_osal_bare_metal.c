/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file usb_osal_bare_metal.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/06/30
 * @license
 * @description
 */

#include <stdint.h>
#include <stdbool.h>

#include "usb_config.h"
#include "usb_errno.h"
#include "usb_osal.h"

#include "sys_uhost.h"

#include "sys_sim.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_VIRTUAL_THREADS_MAX      2
#define CONFIG_VIRTUAL_SEM_MAX          CONFIG_USBHOST_PIPE_NUM

#define CONFIG_RBI_INSTANCE_MAX         2
#define CONFIG_RBI_ELEM_MAX             16

#define CONFIG_TOTAL_MEMPOOL_BYTES      (4ul << 10)

#define MIN_BLOCK_SIZE                  ((uint32_t)(g_link_hdr_size << 1))
#define BITS_PER_BYTE                   8
#define BLK_USED_FLAG                   ((0x1ul) << ((sizeof(uint32_t) * BITS_PER_BYTE) - 1))

#define BYTE_ALIGN                      4ul
#define BYTE_ALIGN_MASK                 (BYTE_ALIGN - 1ul)
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  virtual thread configuration
 */
typedef struct sys_vthread
{
    const char          *name;
    uint32_t            prio;
    usb_thread_entry_t  entry;
//    void                *args;

} sys_vthread_t;

/**
 *  virtual semaphore configuration
 */
typedef struct sys_vsem
{
    uint16_t            used;
    volatile uint16_t   cnt;
} sys_vsem_t;

typedef struct rbuf
{
    uint32_t            max_elem;
    volatile uint32_t   tail;
    volatile uint32_t   head;

    uint32_t            ibuf[CONFIG_RBI_ELEM_MAX];
} rbuf_t;

typedef struct blk_link
{
    struct blk_link     *pNext_free_blk;
    uint32_t            blk_size;
} blk_link_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static sys_vthread_t        g_vthread_list[CONFIG_VIRTUAL_THREADS_MAX] = {0};
static rbuf_t               g_rbi_mgr[CONFIG_RBI_INSTANCE_MAX] = {0};
static sys_vsem_t           g_vsem[CONFIG_VIRTUAL_SEM_MAX] = {0};

static uint32_t         g_mempool_base = 0;
static uint32_t         g_mempool_size = 0;
static const uint32_t   g_link_hdr_size = (sizeof(blk_link_t) + BYTE_ALIGN_MASK) & ~BYTE_ALIGN_MASK;
static blk_link_t       g_blk_start;
static blk_link_t       *g_pBlk_end = 0;

static uint32_t         g_free_bytes = 0U;
static uint32_t         g_free_bytes_min = 0U;  // record the worst-case
static uint32_t         g_malloc_cnt = 0;
static uint32_t         g_free_cnt = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_uhost_sys_proc(void)
{
    for(int i = 0; i < CONFIG_VIRTUAL_THREADS_MAX; i++)
    {
        if( g_vthread_list[i].entry )
            g_vthread_list[i].entry(0);
    }
    return 0;
}

static void _usb_osal_mpool_init(uint32_t mpool_base, int mpool_len)
{
    blk_link_t  *pBlk_head;
    uint8_t     *pAlign_addr;
    uint32_t    addr = 0;
    uint32_t    total_size = mpool_len;

    /* Ensure the heap starts on a correctly aligned boundary. */
    addr = (uint32_t)mpool_base;

    if( (addr & BYTE_ALIGN_MASK) )
    {
        addr = (addr + BYTE_ALIGN_MASK) & ~BYTE_ALIGN_MASK;
        total_size -= (addr - (uint32_t)mpool_base);
    }

    pAlign_addr = (uint8_t *)addr;

    g_blk_start.pNext_free_blk = (void *)pAlign_addr;
    g_blk_start.blk_size       = 0ul;

    addr = ((uint32_t)pAlign_addr) + total_size;
    addr = (addr - g_link_hdr_size) & ~BYTE_ALIGN_MASK;

    g_pBlk_end = (void *)addr;

    g_pBlk_end->blk_size       = 0;
    g_pBlk_end->pNext_free_blk = 0;

    pBlk_head = (blk_link_t *)pAlign_addr;

    pBlk_head->blk_size       = addr - (uint32_t)pBlk_head;
    pBlk_head->pNext_free_blk = g_pBlk_end;

    g_free_bytes_min = pBlk_head->blk_size;
    g_free_bytes     = pBlk_head->blk_size;

    return;
}

static void _usb_osal_add_blk_to_free_list(blk_link_t *pBlk)
{
    blk_link_t  *pIterator;
    uint8_t     *pCur;

    for(pIterator = &g_blk_start;
            pIterator->pNext_free_blk < pBlk;
            pIterator = pIterator->pNext_free_blk)
    {
    }

    pCur = (uint8_t *)pIterator;

    if( (pCur + pIterator->blk_size) == (uint8_t *)pBlk )
    {
        pIterator->blk_size += pBlk->blk_size;
        pBlk                = pIterator;
    }

    pCur = (uint8_t *)pBlk;

    if( (pCur + pBlk->blk_size) == (uint8_t *)pIterator->pNext_free_blk )
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

sys_uhost_class_t   g_uhost_vthreads =
{
    .uclass_id           = SYS_UHOST_TAG('H', 'S', 'Y', 'S'),
    .uhost_class_init    = 0,
    .uhost_class_proc    = _uhost_sys_proc,
    .uhost_err_callback  = 0,
};

int usb_osal_init(void)
{
    memset(&g_vthread_list, 0x0, sizeof(g_vthread_list));
    memset(&g_rbi_mgr, 0x0, sizeof(g_rbi_mgr));
    return 0;
}

usb_osal_thread_t
usb_osal_thread_create(
    const char          *name,
    uint32_t            stack_size,
    uint32_t            prio,
    usb_thread_entry_t  entry,
    void                *args)
{
    sys_vthread_t       *pVThead = 0;

    for(int i = 0; i < CONFIG_VIRTUAL_THREADS_MAX; i++)
    {
        if( g_vthread_list[i].entry )
            continue;

        pVThead = &g_vthread_list[i];
        pVThead->name  = name;
        pVThead->entry = entry;
        pVThead->prio  = prio;
        break;
    }

    return (usb_osal_thread_t)pVThead;
}

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count)
{
    sys_vsem_t      *pSem = 0;

    for(int i = 0; i < CONFIG_VIRTUAL_SEM_MAX; i++)
    {
        if( g_vsem[i].used )
            continue;

        pSem = &g_vsem[i];

        pSem->used = true;
        pSem->cnt  = 0;
        break;
    }

    return (usb_osal_sem_t)pSem;
}

void usb_osal_sem_delete(usb_osal_sem_t sem)
{
    sys_vsem_t      *pSem = (sys_vsem_t *)sem;

    pSem->used = 0;
    return;
}

int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout) // wait
{
    sys_vsem_t      *pSem = (sys_vsem_t *)sem;
#if 1
    while( pSem->cnt == 0 )
    {
//        if( --timeout == 0 )
//            return -ETIMEDOUT;
        __asm("nop");
    }

    pSem->cnt--;
#endif
    return (int)0;
}

int usb_osal_sem_give(usb_osal_sem_t sem)   // post
{
    sys_vsem_t      *pSem = (sys_vsem_t *)sem;
    pSem->cnt++;
    return (int)0;
}

usb_osal_mutex_t usb_osal_mutex_create(void)
{
    return (usb_osal_mutex_t)usb_osal_sem_create(0);
}

void usb_osal_mutex_delete(usb_osal_mutex_t mutex)
{
    usb_osal_sem_delete(mutex);
    return;
}

int usb_osal_mutex_take(usb_osal_mutex_t mutex)
{
    return (int)usb_osal_sem_take(mutex, 0xFFFFFFFF);
}

int usb_osal_mutex_give(usb_osal_mutex_t mutex)
{
    return (int)usb_osal_sem_give(mutex);
}

usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs)
{
    rbuf_t      *pRBI = 0;

    do
    {
        if( max_msgs > CONFIG_RBI_ELEM_MAX )
            break;

        for(int i = 0; i < CONFIG_RBI_INSTANCE_MAX; i++)
        {
            if( g_rbi_mgr[i].max_elem )
                continue;

            pRBI = &g_rbi_mgr[i];

            pRBI->max_elem = max_msgs;
            pRBI->head     = 0;
            pRBI->tail     = 0;
            break;
        }
    }
    while(0);
    return (usb_osal_mq_t)pRBI;
}

int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr)
{
    rbuf_t          *pRBI = (rbuf_t *)mq;
    uint32_t        head = 0;
    uint32_t        tail = 0;

    if( !pRBI )
        return -EINVAL;

    head = pRBI->head;
    tail = pRBI->tail;

    // buffer full
    if( ((head - tail) % pRBI->max_elem) == (pRBI->max_elem - 1) )
        return -ENOBUFS;

    pRBI->ibuf[head] = addr;
    pRBI->head       = (head + 1) % pRBI->max_elem;

    return 0;
}

int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout)
{
    rbuf_t          *pRBI = (rbuf_t *)mq;
    uint32_t        head = 0;
    uint32_t        tail = 0;

    if( !pRBI )
        return -EINVAL;

    head = pRBI->head;
    tail = pRBI->tail;

    // No items
    if( head == tail )
        return -ENOMSG;

    *addr      = pRBI->ibuf[tail];
    pRBI->tail = (tail + 1) % pRBI->max_elem;

    return 0;
}

size_t usb_osal_enter_critical_section(void)
{
    return 0;
}

void usb_osal_leave_critical_section(size_t flag)
{
    return;
}

void usb_osal_msleep(uint32_t delay)
{
    return;
}

int usb_osal_mpool_init(uint32_t *pMPool, int pool_len)
{
    int     rval = 0;

    if( !pMPool || ((uint32_t)pMPool & BYTE_ALIGN_MASK) ||
            !pool_len || (pool_len & BYTE_ALIGN_MASK) )
        return -EINVAL;

    g_mempool_base = (uint32_t)pMPool;
    g_mempool_size = pool_len;

    return 0;
}

void *usb_osal_malloc(int length)
{
    blk_link_t  *pBlk_cur, *pBlk_prev, *pBlk_next;
    void        *pAddr = 0;

    if( g_pBlk_end == 0 )
    {
        _usb_osal_mpool_init(g_mempool_base, g_mempool_size);
    }

    if( (length & BLK_USED_FLAG) )
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
            pAddr = (void *)(((uint8_t *)pBlk_prev->pNext_free_blk) + g_link_hdr_size);

            pBlk_prev->pNext_free_blk = pBlk_cur->pNext_free_blk;

            if( (pBlk_cur->blk_size - length) > MIN_BLOCK_SIZE )
            {
                pBlk_next = (void *)(((uint8_t *)pBlk_cur) + length);

                pBlk_next->blk_size = pBlk_cur->blk_size - length;
                pBlk_cur->blk_size  = length;

                _usb_osal_add_blk_to_free_list(pBlk_next);
            }

            g_free_bytes -= pBlk_cur->blk_size;

            if( g_free_bytes < g_free_bytes_min )
            {
                g_free_bytes_min = g_free_bytes;
            }

            pBlk_cur->blk_size |= BLK_USED_FLAG;
            pBlk_cur->pNext_free_blk = 0;

            g_malloc_cnt++;
        }
    }

    return pAddr;
}

void usb_osal_free(void *pPtr)
{
    blk_link_t  *pBLink;

    if( pPtr == 0 )     return;

    pBLink = (void *)((uint32_t)pPtr - g_link_hdr_size);

    if( (pBLink->blk_size & BLK_USED_FLAG) == 0 ||
            pBLink->pNext_free_blk )
        return;

    pBLink->blk_size &= ~BLK_USED_FLAG;

    g_free_bytes += pBLink->blk_size;
    _usb_osal_add_blk_to_free_list(((blk_link_t *)pBLink));
    g_free_cnt++;

    return;
}
