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

//=============================================================================
//                  Global Data Definition
//=============================================================================
static sys_vthread_t        g_vthread_list[CONFIG_VIRTUAL_THREADS_MAX] = {0};
static rbuf_t               g_rbi_mgr[CONFIG_RBI_INSTANCE_MAX] = {0};
static sys_vsem_t           g_vsem[CONFIG_VIRTUAL_SEM_MAX] = {0};
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
    sys_vsem_t      *pSem = (sys_vsem_t*)sem;

    pSem->used = 0;
    return;
}

int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout) // wait
{
    sys_vsem_t      *pSem = (sys_vsem_t*)sem;
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
    sys_vsem_t      *pSem = (sys_vsem_t*)sem;
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

    do {
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
    } while(0);
    return (usb_osal_mq_t)pRBI;
}

int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr)
{
    rbuf_t          *pRBI = (rbuf_t*)mq;
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
    rbuf_t          *pRBI = (rbuf_t*)mq;
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
