/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file shell.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/09/12
 * @license
 * @description
 */


#include <string.h>
#include "shell.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#if 1
    #include <stdio.h>
    #define err(str, ...)       do{ printf("[%s:%u] " str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)
#else
    #define err(str, ...)
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sh_dev
{
    sh_cmd_t        *pCmd_head;
    sh_io_desc_t    *pIO;
} sh_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static sh_dev_t     g_sh_dev = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
shell_init(
    sh_io_desc_t *pDesc,
    void         *pInfo)
{
    int     rval = 0;

    do {
        if( !pDesc )
        {
            rval = -1;
            err("input NULL data\n");
            break;
        }

        memset(&g_sh_dev, 0x0, sizeof(sh_dev_t));

        g_sh_dev.pIO = pDesc;

        if( g_sh_dev.pIO && g_sh_dev.pIO->cb_init )
            rval = g_sh_dev.pIO->cb_init(pInfo);

        if( rval )      break;

    } while(0);

    return rval;
}

int
shell_deinit(void *pInfo)
{
    int     rval = 0;

    if( g_sh_dev.pIO && g_sh_dev.pIO->cb_deinit )
        rval = g_sh_dev.pIO->cb_deinit(pInfo);

    return rval;
}

int
shell_register_cmd(sh_cmd_t *pCmd)
{
    int     rval = 0;

    if( g_sh_dev.pCmd_head )
    {
        sh_cmd_t    *pCur = g_sh_dev.pCmd_head;

        while( pCur->next )
            pCur = pCmd->next;

        pCur->next = pCmd;
    }
    else
    {
        g_sh_dev.pCmd_head = pCmd;
    }
    return rval;
}

void*
shell_proc(sh_args_t *pArg)
{
    sh_io_desc_t    *pIO = g_sh_dev.pIO;

    while( pArg->is_blocking )
    {
        if( pIO && pIO->cb_read )
            pIO->cb_read();

        if( pArg->cb_regular_alarm )
            pArg->cb_regular_alarm(pArg);

    }
    return 0;
}

