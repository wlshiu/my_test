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
/*! \name VT100 Common Commands (VT100 escape codes)
 */

/**
 *  VT100 color of text
 */
#define VT100_COLOR_BLACK             "\033[30m"
#define VT100_COLOR_RED               "\033[31m"
#define VT100_COLOR_GREEN             "\033[32m"
#define VT100_COLOR_YELLOW            "\033[33m"
#define VT100_COLOR_BLUE              "\033[34m"
#define VT100_COLOR_MAGENTA           "\033[35m"
#define VT100_COLOR_CYAN              "\033[36m"
#define VT100_COLOR_WHITE             "\033[37m"
#define VT100_COLOR_DEFAULT           "\033[39m"
#define VT100_COLOR_RESET             "\033[m"

/**
 *  VT100 background color
 */
#define VT100_BG_BLACK                "\033[40m"
#define VT100_BG_RED                  "\033[41m"
#define VT100_BG_GREEN                "\033[42m"
#define VT100_BG_YELLOW               "\033[43m"
#define VT100_BG_BLUE                 "\033[44m"
#define VT100_BG_MAGENTA              "\033[45m"
#define VT100_BG_CYAN                 "\033[46m"
#define VT100_BG_WHITE                "\033[47m"
#define VT100_BG_DEFAULT              "\033[49m"



#define SHELL_PROMPT                  "~# "

typedef int (*cb_shell_out_t)(const char *str, ...);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct sh_set
{
    char        *pLine_buf;
    uint32_t    line_buf_len;

    char        *pIO_cache;
    uint32_t    io_cache_len;

    void        *pUser_data;

} sh_set_t;

/**
 *  the I/O description of shell module
 */
typedef struct sh_io_desc
{
    int     (*cb_init)(sh_set_t *pSet_info);
    int     (*cb_deinit)(void *pInfo);

    int     (*cb_read)(uint8_t *pBuf, uint32_t length, void *pExtra);
    int     (*cb_write)(uint8_t *pBuf, uint32_t length, void *pExtra);
} sh_io_desc_t;

/**
 *  the command description
 */
typedef struct sh_cmd
{
    struct sh_cmd   *next;

    char        *pCmd_name;
    uint32_t    cmd_name_len;
    char        *pDescription;

    void        *pExtra;

    /**
     *  @brief  cmd_exec
     *              executable program of shell command 
     *  
     *  @param [in] argc        count of arguments
     *  @param [in] argv        values of arguments
     *  @param [in] out_func    function of shell log output 
     *  @param [in] pExtra      extra data from (struct sh_cmd)->pExtra
     *  @return 
     *      0: ok, other fail
     */
    int         (*cmd_exec)(int argc, char **argv, cb_shell_out_t out_func, void *pExtra);

} sh_cmd_t;

typedef struct sh_args
{
    uint32_t    is_blocking;

    int     (*cb_regular_alarm)(struct sh_args *pArg);

    void    *pTunnel_info;

} sh_args_t;
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
shell_init(
    sh_io_desc_t *pDesc,
    sh_set_t     *pSet_info);


int
shell_deinit(void *pInfo);


int
shell_register_cmd(sh_cmd_t *pCmd);


void*
shell_proc(sh_args_t *pArg);


#ifdef __cplusplus
}
#endif

#endif


