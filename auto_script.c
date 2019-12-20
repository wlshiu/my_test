/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file auto_script.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/12/20
 * @license
 * @description
 */


#include <string.h>
#include "auto_script.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ENABLE_SIM
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static ascript_line_t   g_cmd_line = {0};
static uint32_t         g_rd_value = 0ul;

#if defined(CONFIG_ENABLE_SIM)
static uint32_t         g_sim_registers[32] = {0};
#endif // defined
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_opc_write_handler(opc_argv_t *pArgv)
{
    int     rval = 0;
    *(uint32_t*)(pArgv->reg_addr) = pArgv->value;
    return rval;
}

static int
_opc_read_handler(opc_argv_t *pArgv)
{
    int     rval = 0;
    g_rd_value = *(uint32_t*)(pArgv->reg_addr);
    return rval;
}

static int
_opc_mask_rd_handler(opc_argv_t *pArgv)
{
    int     rval = 0;
    g_rd_value = *(uint32_t*)(pArgv->reg_addr) & pArgv->mask;
    return rval;
}

static int
_opc_delay_handler(opc_argv_t *pArgv)
{
    int     rval = 0;
    do{ __asm("nop"); } while( --pArgv->ticks );
    return rval;
}

static int
_opc_wait_handler(opc_argv_t *pArgv)
{
    int     rval = 0;
    while( (*(uint32_t*)(pArgv->reg_addr) & pArgv->rd_mask) != pArgv->value ) {}
    return rval;
}

static int
_opc_mask_wr_handler(opc_argv_t *pArgv)
{
    int     rval = 0;
    *(uint32_t*)(pArgv->reg_addr) = g_rd_value & pArgv->mask;
    return rval;
}

static int
_opc_or_handler(opc_argv_t *pArgv)
{
    int     rval = 0;
    g_rd_value |= pArgv->or_value;
    return rval;
}


static ascript_cmd_desc_t       g_cmd_desc[] =
{
    [0] = { .op_code = ASCRIPT_OPC_READ,        .argument_cnt = 1, .handler = _opc_read_handler,    },
    [1] = { .op_code = ASCRIPT_OPC_WRITE,       .argument_cnt = 2, .handler = _opc_write_handler,   },
    [2] = { .op_code = ASCRIPT_OPC_DELAY,       .argument_cnt = 1, .handler = _opc_delay_handler,   },
    [3] = { .op_code = ASCRIPT_OPC_WAIT_STATE,  .argument_cnt = 3, .handler = _opc_wait_handler,    },
    [4] = { .op_code = ASCRIPT_OPC_MASK_WRITE,  .argument_cnt = 2, .handler = _opc_mask_wr_handler, },
    [5] = { .op_code = ASCRIPT_OPC_MASK_READ,   .argument_cnt = 2, .handler = _opc_mask_rd_handler, },
    [6] = { .op_code = ASCRIPT_OPC_OR,          .argument_cnt = 1, .handler = _opc_or_handler,      },
};

static int              g_cmd_cnt = sizeof(g_cmd_desc) / sizeof(g_cmd_desc[0]);
//=============================================================================
//                  Public Function Definition
//=============================================================================
int
ascript_init()
{
    int     rval = 0;
    do {

    } while(0);
    return rval;
}

int
ascript_exec(
    uint8_t     *pBuf_script,
    uint32_t    buf_len)
{
    int                 rval = 0;
    uint32_t            *pCur = (uint32_t*)pBuf_script;
    uint8_t             *pEnd = pBuf_script + buf_len;
    ascript_cmd_desc_t  *pCmd_cur = 0;

    while( (uintptr_t)pCur < (uintptr_t)pEnd )
    {
        if( !pCmd_cur )
        {
            for(int i = 0; i < g_cmd_cnt; ++i )
            {
                if( *pCur == g_cmd_desc[i].op_code )
                {
                    pCmd_cur = &g_cmd_desc[i];
                    break;
                }
            }

            if( !pCmd_cur )
            {
                rval = -1;
                break;
            }

            pCur++;
        }
        else
        {
            for(int i = 0; i < pCmd_cur->argument_cnt; ++i)
            {
                g_cmd_line.def.argv[i] = *pCur++;
            }

            if( pCmd_cur->handler )
            {
            #if defined(CONFIG_ENABLE_SIM)
                if( pCmd_cur->op_code != ASCRIPT_OPC_DELAY &&
                    pCmd_cur->op_code != ASCRIPT_OPC_OR )
                {
                    g_cmd_line.opc_argv.reg_addr &= 0xFF;
                    g_cmd_line.opc_argv.reg_addr += (uint32_t)g_sim_registers;
                }
            #endif // defined

                pCmd_cur->handler(&g_cmd_line.opc_argv);
            }

            pCmd_cur = 0;
            memset(&g_cmd_line, 0x0, sizeof(g_cmd_line));
        }
    }
    return rval;
}




