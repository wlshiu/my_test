/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file log2mem.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/07/18
 * @license
 * @description
 */


#include "log2mem.h"

#if _WIN32
#include <windows.h>
#include <stdio.h>
#include "pthread.h"
#include <time.h>
#else

    #include "FreeRTOS.h"
    #include "task.h"
    #include "semphr.h"

#endif // _WIN32

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

#if _WIN32
static pthread_mutex_t          g_log2mem_mtx;

#define _mtx_init(p)            pthread_mutex_init(p, 0)
#define _mtx_lock(p)            pthread_mutex_lock(p)
#define _mtx_unlock(p)          pthread_mutex_unlock(p)

#define _get_time()             clock()

#else

static SemaphoreHandle_t        g_log2mem_mtx;

#define _mtx_init(p)            do{ *p = xSemaphoreCreateMutex(); }while(0)
#define _mtx_lock(p)            xSemaphoreTake(*p, portMAX_DELAY)
#define _mtx_unlock(p)          xSemaphoreGive(*p)

#define _get_time()             xTaskGetTickCount()

#endif

#define log_out(st, args...)            printf(st, ##args)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
unsigned int        g_log_idx = 0;
log_node_t          g_log_buf[LOG_BUF_MAX_SIZE] = {0};

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
log2mem_init(void)
{
    g_log_idx = 0;

    _mtx_init(&g_log2mem_mtx);

    memset(g_log_buf, 0x0, sizeof(g_log_buf));
    return 0;
}

int
log2mem_push(
    unsigned int    argv_0,
    unsigned int    argv_1,
    unsigned int    argv_2)
{
    int                 rval = 0;

    _mtx_lock(&g_log2mem_mtx);
    do {
        log_node_t          *plog = 0;

        if( g_log_idx == LOG_BUF_MAX_SIZE )
        {
            rval = -1;
            break;
        }

        plog = &g_log_buf[g_log_idx++];

        plog->time_stamp = (unsigned int)_get_time();

        plog->argv_0 = argv_0;
        plog->argv_1 = argv_1;
        plog->argv_2 = argv_2;
    } while(0);

    _mtx_unlock(&g_log2mem_mtx);
    return rval;
}

int
log2mem_show(void)
{
    int     rval = 0;
    do {
        int     i;
        for(i = 0; i < g_log_idx; i++)
        {
            log_node_t          *plog = &g_log_buf[i];
            log_out("%08d: x%08x, x%08x, x%08x\n", plog->time_stamp,
                    plog->argv_0, plog->argv_1, plog->argv_2);
        }
    } while(0);

    return rval;
}


