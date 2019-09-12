/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file shell.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/09/12
 * @license
 * @description
 */


#ifndef __shell_H_39950817_5188_493e_acc0_d78b22d58721__
#define __shell_H_39950817_5188_493e_acc0_d78b22d58721__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  the I/O description of shell module
 */
typedef struct sh_io_desc
{
    int     (*cb_init)(void *pInfo);
    int     (*cb_deint)(void *pInfo);

    int     (*cb_read)(uint8_t *pBuf, uint32_t length, void *pExtra);
    int     (*cb_write)(uint8_t *pBuf, uint32_t length, void *pExtra);
} sh_io_desc_t;

/**
 *  the command description
 */
typedef struct sh_cmd
{
    struct sh_cmd   *next;

    char    *pCmd_name;
    int     (*cmd_exec)(int argc, char **argv);

} sh_cmd_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
shell_init(sh_io_desc_t *pDesc);


int
shell_deinit(void);


int
shell_register_cmd(sh_cmd_t *pCmd);


void*
shell_routine(void *pArg);


#ifdef __cplusplus
}
#endif

#endif


