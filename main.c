/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/11/18
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PT_NWAIT (1 << 2)

#include "protothread.h"
#include "protothread_sem.h"
#include "protothread_lock.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct thread_argv
{
    /* base object */
    pt_thread_t     pt_thread;
    pt_func_t       pt_func;

    /* private data */
    uint32_t        *pSync_flag;
} thread_argv_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint32_t     g_data = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static pt_t
_thread_producer(void *const argv)
{
    thread_argv_t   *pHThArgv = (thread_argv_t*)argv;

    pt_resume(pHThArgv);

    while(1)
    {
        while( *pHThArgv->pSync_flag )
        {
            pt_wait(pHThArgv, pHThArgv->pSync_flag);
        }

        *pHThArgv->pSync_flag = 1;

        /* private data handshake */
        g_data++;
        printf("[%s] data= %d\n", __func__, g_data);

        pt_signal(pt_get_pt(pHThArgv), pHThArgv->pSync_flag);
    }

    return PT_DONE;
}

static pt_t
_thread_consumer(void *const argv)
{
    thread_argv_t   *pHThArgv = (thread_argv_t*)argv;

    pt_resume(pHThArgv);

    while(1)
    {
        while( !(*pHThArgv->pSync_flag) )
        {
            pt_wait(pHThArgv, pHThArgv->pSync_flag);
        }

        *pHThArgv->pSync_flag = 0;

        /* private data handshake */
        printf("[%s] data= %d\n", __func__, g_data);

        if( g_data == 5 )
            printf("\n");

        pt_signal(pt_get_pt(pHThArgv), pHThArgv->pSync_flag);
    }

    return PT_DONE;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    protothread_t       hProtoTh = protothread_create();
    thread_argv_t       argv_producer = {0};
    thread_argv_t       argv_consumer = {0};
    uint32_t            flag = 0;

    /* set up consumer context, start consumer thread */
    argv_consumer.pSync_flag = &flag;
    pt_create(hProtoTh, &argv_consumer.pt_thread, _thread_consumer, &argv_consumer);

    /* set up producer context, start producer thread */
    argv_producer.pSync_flag = &flag;
    pt_create(hProtoTh, &argv_producer.pt_thread, _thread_producer, &argv_producer);

    /* while threads are available to run ... */
    while( protothread_run(hProtoTh) )
    {
    }

    protothread_free(hProtoTh);
    return 0;
}
