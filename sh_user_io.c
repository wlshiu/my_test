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

//=============================================================================
//                  Global Data Definition
//=============================================================================
static pthread_t    g_task;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_task_console_frontend(void *pArgs)
{
    pthread_detach(pthread_self());

    while(1)
    {
        int c = getch();
        putc(c, stdout);
    }
    pthread_exit(0);
    return 0;
}


static int
_win_sh_io_init(sh_set_t *pSet_info)
{
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
    return 0;
}

static int
_win_sh_io_write(
    uint8_t     *pBuf,
    uint32_t    length,
    void *pExtra)
{
    return 0;
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

