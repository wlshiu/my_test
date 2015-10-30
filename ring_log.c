/**
 * Copyright (c) 2015 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ring_log.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2015/10/22
 * @license
 * @description
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "mleak_check.h"

#include "ring_log.h"

#if 0 //def _WIN32
    #include <windows.h>
    #define pthread_self    GetCurrentThreadId
    #define vsnprintf       _vsnprintf
#else
    #include <pthread.h>
#endif
//=============================================================================
//                Constant Definition
//=============================================================================
#ifndef uint8_t
#define uint8_t    unsigned char
#endif

#ifndef uint16_t
#define uint16_t    unsigned short
#endif

#ifndef uint32_t
#define uint32_t    unsigned int
#endif


//=============================================================================
//                Macro Definition
//=============================================================================
#if 1
    #define _err(st, args...)       do{fprintf(stderr, "[%u] %s: " st, __LINE__, __FUNCTION__, ## args); while(1);}while(0)
#else
    #define _err(st, args...)       fprintf(stderr, "[%u] %s: " st, __LINE__, __FUNCTION__, ## args)
#endif

#ifdef _WIN32
    #define _get_thread_id(pPid)    do{ pthread_t    t;                \
                                        t = pthread_self();            \
                                        *pPid = (unsigned long)t.p;     \
                                    }while(0)
#else
    #define _get_thread_id(pPid)    do{ *pPid = (unsigned long)pthread_self(); }while(0)
#endif
//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct ring_log_dev
{
    pthread_mutex_t         mutex;

    uint8_t                 *pLog_buf;
    uint32_t                log_buf_size;

    unsigned long           buf_end;
    unsigned long           buf_idx_w;
} ring_log_dev_t;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
int
rlog_create(
    HRLOG               *pHRlog,
    rlog_init_info_t    *pInit_info)
{
    // thread self case, one thread one log file
    int             result = -1;
    ring_log_dev_t  *pDev = 0;

    do{
        if( !pHRlog || !pInit_info )
        {
            _err("wrong input, pHRlog(%p) and pInit_info(%p) !\n", pHRlog, pInit_info);
            break;
        }

        if( !pInit_info->buffer_size )
        {
            _err("err, buffer size = %d\n", pInit_info->buffer_size);
            break;
        }

        *pHRlog = NULL;

        // create handle
        if( !(pDev = malloc(sizeof(ring_log_dev_t))) )
        {
            _err("create handle fail !\n");
            break;
        }
        memset(pDev, 0x0, sizeof(ring_log_dev_t));

        // -----------------------------
        // init parameters
        pDev->log_buf_size = pInit_info->buffer_size;

        if( !(pDev->pLog_buf = malloc(pDev->log_buf_size)) )
        {
            _err("malloc buffer fail !\n");
            break;
        }
        memset(pDev->pLog_buf, '\n', pDev->log_buf_size);

        pDev->buf_idx_w = (unsigned long)((void*)pDev->pLog_buf);
        pDev->buf_end   = (unsigned long)((void*)pDev->pLog_buf) + pDev->log_buf_size;

        if( pthread_mutex_init(&pDev->mutex, NULL) )
            _err("init mutex fail !!!\n");

        // -----------------------------
        *pHRlog = (HRLOG)pDev;
        result = 0;
    }while(0);

    if( result )
    {
        HRLOG   hRlog = pDev;
        rlog_destroy(&hRlog);
    }

    return result;
}

int
rlog_destroy(
    HRLOG   *pHRlog)
{
    int     result = 0;
    do{
        ring_log_dev_t  *pDev = (ring_log_dev_t*)(*pHRlog);

        if( !pHRlog )       break;

        if( pDev->pLog_buf )    free(pDev->pLog_buf);

        pthread_mutex_destroy(&pDev->mutex);
        free(pDev);
    }while(0);

    return result;
}

int
rlog_post(
    HRLOG       hRlog,
    const char  *pStr_format, ...)
{
    int     result = -1;

    do{
        ring_log_dev_t  *pDev = 0;
        unsigned long   pid;
        char            tmp[512] = {0};
        size_t          in_len = 0;
        va_list         args;

        if( !hRlog )    break;

        pDev = (ring_log_dev_t*)hRlog;
        _get_thread_id(&pid);

        va_start(args, pStr_format);
        snprintf(tmp, sizeof(tmp), "[pid=%-8lx] ", pid);
        vsnprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), pStr_format, args);
        va_end(args);

        in_len = strlen(tmp);
        pthread_mutex_lock(&pDev->mutex);
        {
            unsigned long   cur_idx_w = pDev->buf_idx_w;
            unsigned long   buf_end = pDev->buf_end;

            if( (cur_idx_w + in_len) < buf_end )
            {
                memcpy((void*)cur_idx_w, tmp, in_len);
                cur_idx_w += in_len;
            }
            else
            {
                long    buf_remain = buf_end - cur_idx_w;

                memcpy((void*)cur_idx_w, tmp, buf_remain);
                cur_idx_w = (unsigned long)((void*)pDev->pLog_buf);

                memcpy((void*)cur_idx_w, &tmp[buf_remain], in_len - buf_remain);
                cur_idx_w += (in_len - buf_remain);
            }

            pDev->buf_idx_w = cur_idx_w;
        }
        pthread_mutex_unlock(&pDev->mutex);

        result = 0;
    }while(0);
    return result;
}

int
rlog_dump(
    HRLOG   hRlog,
    char    *pOut_path)
{
    int     result = -1;
    do{
        ring_log_dev_t  *pDev = 0;
        FILE            *fout = 0;

        if( !hRlog )    break;

        pDev = (ring_log_dev_t*)hRlog;

        if( !(fout = fopen(pOut_path, "wb")) )
        {
            _err("open %s fail !\n", pOut_path);
            break;
        }

        pthread_mutex_lock(&pDev->mutex);
        {
            unsigned long   cur_idx_w = pDev->buf_idx_w;
            unsigned long   buf_end = pDev->buf_end;
            long            buf_remain = buf_end - cur_idx_w;

            fwrite((void*)cur_idx_w, 1, buf_remain, fout);
            fwrite((void*)pDev->pLog_buf, 1, pDev->log_buf_size - buf_remain, fout);
        }
        pthread_mutex_unlock(&pDev->mutex);

        if( fout )  fclose(fout);

        result = 0;
    }while(0);

    return result;
}
