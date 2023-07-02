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


#include "usb_osal.h"

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

usb_osal_thread_t
usb_osal_thread_create(
    const char          *name,
    uint32_t            stack_size,
    uint32_t            prio,
    usb_thread_entry_t  entry,
    void                *args)
{
    usb_osal_thread_t   th;
    return (usb_osal_thread_t)th;
}

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count)
{
    usb_osal_sem_t      sem = {0};
    return (usb_osal_sem_t)sem;
}

void usb_osal_sem_delete(usb_osal_sem_t sem)
{
    return;
}

int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout)
{
    int ret = 0;

    return (int)ret;
}

int usb_osal_sem_give(usb_osal_sem_t sem)
{
    return (int)1;
}

usb_osal_mutex_t usb_osal_mutex_create(void)
{
    return (usb_osal_mutex_t)1;
}

void usb_osal_mutex_delete(usb_osal_mutex_t mutex)
{
    return;
}

int usb_osal_mutex_take(usb_osal_mutex_t mutex)
{
    return (int)1;
}

int usb_osal_mutex_give(usb_osal_mutex_t mutex)
{
    return (int)1;
}

usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs)
{
    static usb_osal_mq_t        mq = {0};
    return mq;
}

int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr)
{
    return 0;
}

int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout)
{
    int ret = 0;

    return (int)ret;
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
