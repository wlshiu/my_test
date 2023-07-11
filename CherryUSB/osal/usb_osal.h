/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef USB_OSAL_H
#define USB_OSAL_H

#include <stdint.h>
#include <string.h>

typedef void *usb_osal_thread_t;
typedef void *usb_osal_sem_t;
typedef void *usb_osal_mutex_t;
typedef void *usb_osal_mq_t;
typedef void (*usb_thread_entry_t)(void *argument);

usb_osal_thread_t usb_osal_thread_create(const char *name, uint32_t stack_size, uint32_t prio, usb_thread_entry_t entry, void *args);

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count);
void usb_osal_sem_delete(usb_osal_sem_t sem);
int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout);
int usb_osal_sem_give(usb_osal_sem_t sem);

usb_osal_mutex_t usb_osal_mutex_create(void);
void usb_osal_mutex_delete(usb_osal_mutex_t mutex);
int usb_osal_mutex_take(usb_osal_mutex_t mutex);
int usb_osal_mutex_give(usb_osal_mutex_t mutex);

usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs);
int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr);
int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout);

size_t usb_osal_enter_critical_section(void);
void usb_osal_leave_critical_section(size_t flag);

void usb_osal_msleep(uint32_t delay);

/* memory pool of system */
/**
 *  \brief  Initialize memory pool of OSAL
 *
 *  \param [in] pMPool          Pointer to a memory pool (It MUST be 4-align)
 *  \param [in] pool_len        The length of a memory pool (It MUST be 4-align)
 *  \return
 *      errno
 */
int usb_osal_mpool_init(uint32_t *pMPool, int pool_len);
void *usb_osal_malloc(int length);
void usb_osal_free(void *pPtr);

#endif /* USB_OSAL_H */
