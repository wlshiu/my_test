/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file log.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/07
 * @license
 * @description
 */


#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include "log.h"

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
static pthread_mutex_t      g_log_mtx;
//=============================================================================
//                  Public Function Definition
//=============================================================================
void
log_out(const char *fmt, ...)
{
    static int  is_initialized = 0;
    va_list     args;

    if( !is_initialized )
    {
        pthread_mutex_init(&g_log_mtx, 0);
        is_initialized = 1;
    }

    pthread_mutex_lock(&g_log_mtx);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fflush(stderr);

    pthread_mutex_unlock(&g_log_mtx);
    return;
}


