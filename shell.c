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
#define SH_LF       '\n'
#define SH_CR       '\r'

/**
 *  escape code
 */
#define ESC_START_CODE              0x1b
#define ESC_CODE_HOME               "\x1b[1~"
#define ESC_CODE_INSERT             "\x1b[2~"
#define ESC_CODE_DELET              "\x1b[3~"
#define ESC_CODE_END                "\x1b[4~"
#define ESC_CODE_PAGE_UP            "\x1b[5~"
#define ESC_CODE_PAGE_DOWN          "\x1b[6~"

#define ESC_CODE_UP                 "\x1b[A"
#define ESC_CODE_DOWN               "\x1b[B"
#define ESC_CODE_RIGHT              "\x1b[C"
#define ESC_CODE_LEFT               "\x1b[D"

#define ESC_CODE_DELETE             "\x1b[P"

#define ESC_CODE_BACKSPACE          "\x08"
#define ESC_CODE_TAB                "\x09"
#define ESC_CODE_CTRL_C             "\x03"

typedef enum sh_state
{
    SH_STATE_GET_LINE   =  1,
    SH_STATE_OK         =  0,
    SH_STATE_ERR        = -1,
} sh_state_t;

typedef enum sh_esc_code
{
    SH_ESC_CODE_HOME    = 0,
    SH_ESC_CODE_INSERT,
    SH_ESC_CODE_DELET,
    SH_ESC_CODE_END,
    SH_ESC_CODE_PAGE_UP,
    SH_ESC_CODE_PAGE_DOWN,
    SH_ESC_CODE_UP,
    SH_ESC_CODE_DOWN,
    SH_ESC_CODE_LEFT,
    SH_ESC_CODE_RIGHT,
    SH_ESC_CODE_BACKSPACE,
    SH_ESC_CODE_TAB,
    SH_ESC_CODE_CTRL_C,

    SH_ESC_CODE_TOTAL

} sh_esc_code_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#ifndef __unused
#define __unused            __attribute__ ((unused))
#endif

#if 1
    #include <stdio.h>
    #define err(str, ...)       do{ printf("[%s:%u] " str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)
#else
    #define err(str, ...)
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sh_esc_item
{
    sh_esc_code_t   esc_code;
    uint16_t        esc_code_len;
    char            *pEsc_ascii;

} sh_esc_item_t;

typedef struct sh_dev
{
    sh_cmd_t        *pCmd_head;
    sh_io_desc_t    *pIO;

    char            *pLine_buf;
    uint32_t        line_buf_len;
    uint32_t        rd_pos;
    uint32_t        wr_pos;
    uint32_t        cursor_pos;

    union {
        uint32_t        esc_u32_buf[2];
        char            esc_op_buf[8];
    };

    int             esc_input_cumulation;

    void            *pUser_data;

} sh_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static sh_dev_t         g_sh_dev = {0};

static sh_esc_item_t    g_esc_code_table[] =
{
    { .esc_code = SH_ESC_CODE_HOME,      .pEsc_ascii = ESC_CODE_HOME,      .esc_code_len = 4 },
    { .esc_code = SH_ESC_CODE_INSERT,    .pEsc_ascii = ESC_CODE_INSERT,    .esc_code_len = 4 },
    { .esc_code = SH_ESC_CODE_DELET,     .pEsc_ascii = ESC_CODE_DELET,     .esc_code_len = 4 },
    { .esc_code = SH_ESC_CODE_END,       .pEsc_ascii = ESC_CODE_END,       .esc_code_len = 4 },
    { .esc_code = SH_ESC_CODE_PAGE_UP,   .pEsc_ascii = ESC_CODE_PAGE_UP,   .esc_code_len = 4 },
    { .esc_code = SH_ESC_CODE_PAGE_DOWN, .pEsc_ascii = ESC_CODE_PAGE_DOWN, .esc_code_len = 4 },
    { .esc_code = SH_ESC_CODE_UP,        .pEsc_ascii = ESC_CODE_UP,        .esc_code_len = 3 },
    { .esc_code = SH_ESC_CODE_DOWN,      .pEsc_ascii = ESC_CODE_DOWN,      .esc_code_len = 3 },
    { .esc_code = SH_ESC_CODE_LEFT,      .pEsc_ascii = ESC_CODE_LEFT,      .esc_code_len = 3 },
    { .esc_code = SH_ESC_CODE_RIGHT,     .pEsc_ascii = ESC_CODE_RIGHT,     .esc_code_len = 3 },
    { .esc_code = SH_ESC_CODE_BACKSPACE, .pEsc_ascii = ESC_CODE_BACKSPACE, .esc_code_len = 1 },
    { .esc_code = SH_ESC_CODE_TAB,       .pEsc_ascii = ESC_CODE_TAB,       .esc_code_len = 1 },
    { .esc_code = SH_ESC_CODE_CTRL_C,    .pEsc_ascii = ESC_CODE_CTRL_C,    .esc_code_len = 1 },
    { .esc_code = SH_ESC_CODE_TOTAL, }
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_shell_get_esc_code(
    char            character,
    sh_esc_code_t   *pEsc_code)
{
    int     rval = 0;
    do {
        sh_esc_item_t   *pEsc_item_cur = g_esc_code_table;

        if( character == ESC_START_CODE ||
            g_sh_dev.esc_input_cumulation >= sizeof(g_sh_dev.esc_op_buf) )
        {
            g_sh_dev.esc_input_cumulation = 0;
            g_sh_dev.esc_u32_buf[0]       = 0;
            g_sh_dev.esc_u32_buf[1]       = 0;
        }

        g_sh_dev.esc_op_buf[g_sh_dev.esc_input_cumulation++] = character;

        while( pEsc_item_cur->esc_code != SH_ESC_CODE_TOTAL )
        {
            if( g_sh_dev.esc_input_cumulation < pEsc_item_cur->esc_code_len )
            {
                pEsc_item_cur++;
                continue;
            }

            if( !strncmp(pEsc_item_cur->pEsc_ascii,
                         g_sh_dev.esc_op_buf,
                         pEsc_item_cur->esc_code_len) )
            {
                *pEsc_code = pEsc_item_cur->esc_code;

                g_sh_dev.esc_input_cumulation = 0;
                g_sh_dev.esc_u32_buf[0]       = 0;
                g_sh_dev.esc_u32_buf[1]       = 0;
                break;
            }

            pEsc_item_cur++;
        }

        if( g_sh_dev.esc_input_cumulation == 1 &&
            character != ESC_START_CODE )
        {
            *pEsc_code = SH_ESC_CODE_TOTAL;
            g_sh_dev.esc_input_cumulation = 0;
            g_sh_dev.esc_u32_buf[0]       = 0;
            g_sh_dev.esc_u32_buf[1]       = 0;
        }

    } while(0);
    return rval;
}

__unused static int
_shell_esc_code_proc(
    sh_esc_code_t   esc_code,
    uint32_t        *pWr_pos,
    uint32_t        *pCursor_pos,
    int (*cb_write)(uint8_t*, uint32_t, void*))
{
    int     rval = 0;

    switch( esc_code )
    {
        case SH_ESC_CODE_BACKSPACE:
            if( *pWr_pos > 0 && *pCursor_pos > 0 )
            {
                char        *pLine_buf = g_sh_dev.pLine_buf;
                uint32_t    len = 0;
                uint32_t    cursor_pos = *pCursor_pos;

//                uint8_t left_seq[] = { 0x1b, 0x5b, 0x44, 0x00 };

                cb_write((uint8_t*)ESC_CODE_LEFT "\x1b[P", 6, g_sh_dev.pUser_data);
//                cb_write((uint8_t*)left_seq, sizeof(left_seq), g_sh_dev.pUser_data);

                len = strlen(pLine_buf);
                if( cursor_pos == len )
                {
                    // at line end
                    pLine_buf[len - 1] = '\0';
                    *pCursor_pos = *pCursor_pos - 1;
                }
                else
                {
                    len++;
                    for(int i = --cursor_pos; i < len; i++)
                        pLine_buf[i] = pLine_buf[i + 1];

                    *pCursor_pos = cursor_pos;

                    cb_write((uint8_t*)&pLine_buf[cursor_pos], len - cursor_pos, g_sh_dev.pUser_data);
                }

                *pWr_pos = *pWr_pos - 1;
            }
            break;
        case SH_ESC_CODE_LEFT:
            if( *pCursor_pos > 0 )
            {
                cb_write((uint8_t*)ESC_CODE_LEFT, 3, g_sh_dev.pUser_data);
                (*pCursor_pos)--;
            }
            break;
        case SH_ESC_CODE_RIGHT:
            if( *pCursor_pos < *pWr_pos )
            {
                cb_write((uint8_t*)ESC_CODE_RIGHT, 3, g_sh_dev.pUser_data);
                (*pCursor_pos)++;
            }
            break;
        case SH_ESC_CODE_HOME:
        case SH_ESC_CODE_INSERT:
        case SH_ESC_CODE_DELET:
        case SH_ESC_CODE_END:
        case SH_ESC_CODE_PAGE_UP:
        case SH_ESC_CODE_PAGE_DOWN:
        case SH_ESC_CODE_UP:
        case SH_ESC_CODE_DOWN:

        case SH_ESC_CODE_TAB:
        case SH_ESC_CODE_CTRL_C:
        default:    break;
    }
    return rval;
}

static sh_state_t
_shell_read_line(
    sh_io_desc_t    *pIO)
{
    sh_state_t      rval = SH_STATE_OK;

    do {
        int (*cb_read)(uint8_t*, uint32_t, void*);
        int (*cb_write)(uint8_t*, uint32_t, void*);
        char    c = 0;

        if( !pIO->cb_read || !pIO->cb_write )
            break;

        cb_read  = pIO->cb_read;
        cb_write = pIO->cb_write;

        if( cb_read((uint8_t*)&c, 1, g_sh_dev.pUser_data) == 0 )
            break;

        {   // read to line buffer
            uint32_t        pos = 0;
            uint32_t        wr_pos = g_sh_dev.wr_pos;
            uint32_t        rd_pos = g_sh_dev.rd_pos;
//            uint32_t        cursor_pos = g_sh_dev.cursor_pos;
            sh_esc_code_t   esc_code;

            pos = (wr_pos + 1) % g_sh_dev.line_buf_len;
            if( pos == rd_pos )     break;

            if( c == '\n' || c == '\r' )
            {
                rval = SH_STATE_GET_LINE;
                g_sh_dev.pLine_buf[wr_pos] = '\0';
                g_sh_dev.wr_pos            = pos;
                break;
            }

            _shell_get_esc_code(c, &esc_code);

            if( esc_code == SH_ESC_CODE_TOTAL )
            {
                // non-escape text
                g_sh_dev.pLine_buf[wr_pos] = c;
                g_sh_dev.wr_pos            = pos;
                g_sh_dev.cursor_pos        = pos;

                cb_write((uint8_t*)&c, 1, g_sh_dev.pUser_data);
            }
            else
            {
                // handle escape control code
                _shell_esc_code_proc(esc_code, &g_sh_dev.wr_pos, &g_sh_dev.cursor_pos, cb_write);
            }
        }

    } while(0);

    return rval;
}

static int
_shell_output(
    sh_io_desc_t    *pIO,
    char            *pBuf,
    uint32_t        len)
{
    int     rval = 0;

    if( pIO->cb_write )
        rval = pIO->cb_write((uint8_t*)pBuf, len, 0);
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int
shell_init(
    sh_io_desc_t *pDesc,
    sh_set_t     *pSet_info)
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

        g_sh_dev.pIO          = pDesc;
        g_sh_dev.pLine_buf    = pSet_info->pLine_buf;
        g_sh_dev.line_buf_len = pSet_info->line_buf_len;
        g_sh_dev.pUser_data   = pSet_info->pUser_data;

        if( g_sh_dev.pLine_buf )
            memset(g_sh_dev.pLine_buf, 0x0, g_sh_dev.line_buf_len);

        if( g_sh_dev.pIO && g_sh_dev.pIO->cb_init )
            rval = g_sh_dev.pIO->cb_init(pSet_info);

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
    uint32_t        is_line_head = 1;

    while( pArg->is_blocking )
    {
        sh_state_t  rval = SH_STATE_OK;

        if( pIO )
        {
            if( is_line_head )
            {
                if( pIO->cb_write )
                    pIO->cb_write((uint8_t*)SHELL_PROMPT, sizeof(SHELL_PROMPT), g_sh_dev.pUser_data);

                is_line_head = 0;
            }

            rval = _shell_read_line(pIO);
            if( rval == SH_STATE_GET_LINE )
            {
                // TODO: execute command

                is_line_head = 1;
                memset(g_sh_dev.pLine_buf, 0x0, g_sh_dev.line_buf_len);
            }

//            _shell_output(pIO, );
        }

        if( pArg->cb_regular_alarm )
            pArg->cb_regular_alarm(pArg);

    }
    return 0;
}

