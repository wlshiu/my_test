/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sh_user_io.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/09/15
 * @license
 * @description
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include "shell.h"
#include "pthread.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct usr_io_dev
{
    char        *pIO_cache;
    uint32_t    io_cache_len;

    uint32_t    wr_idx;
    uint32_t    rd_idx;

} usr_io_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static pthread_t        g_task;
static usr_io_dev_t     g_io_dev = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_task_console_frontend(void *pArgs)
{
    volatile uint32_t    *pWr_idx = &g_io_dev.wr_idx;
    volatile uint32_t    *pRd_idx = &g_io_dev.rd_idx;

    pthread_detach(pthread_self());

    while(1)
    {
        uint32_t    wr_pos = *pWr_idx;
        uint32_t    rd_pos = *pRd_idx;
        uint32_t    pos = 0;

        pos = (wr_pos + 1) % g_io_dev.io_cache_len;

        if( pos == rd_pos )
            continue;

        g_io_dev.pIO_cache[wr_pos] = (char)(getch() & 0xFF);
        *pWr_idx                   = pos;

    }
    pthread_exit(0);
    return 0;
}


static int
_win_sh_io_init(sh_set_t *pSet_info)
{
    memset(&g_io_dev, 0x0, sizeof(g_io_dev));

    g_io_dev.pIO_cache    = pSet_info->pIO_cache;
    g_io_dev.io_cache_len = pSet_info->io_cache_len;

    pthread_create(&g_task, 0, _task_console_frontend, 0);
    return 0;
}

static int
_win_sh_io_deinit(void *pInfo)
{
    return 0;
}

static int
_win_sh_io_read(
    uint8_t     *pBuf,
    uint32_t    length,
    void        *pExtra)
{
    uint32_t    byte_cnt = 0;
    uint32_t    rd_pos = g_io_dev.rd_idx;
    uint32_t    wr_pos = g_io_dev.wr_idx;

    do {
        while( 1 )
        {
            if( rd_pos == wr_pos )      break;
            if( length == byte_cnt )    break;

            pBuf[byte_cnt++] = g_io_dev.pIO_cache[rd_pos];

            rd_pos = (rd_pos + 1) % g_io_dev.io_cache_len;
        }

        g_io_dev.rd_idx = rd_pos;
    } while(0);

    return byte_cnt;
}

static int
_win_sh_io_write(
    uint8_t     *pBuf,
    uint32_t    length,
    void        *pExtra)
{
    int     len  = 0;

    while( length-- )
    {
        putc(pBuf[len++], stdout);
    }

    return len;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
sh_io_desc_t    g_sh_io_win =
{
    .cb_init   = _win_sh_io_init,
    .cb_deinit = _win_sh_io_deinit,
    .cb_read   = _win_sh_io_read,
    .cb_write  = _win_sh_io_write,
};

