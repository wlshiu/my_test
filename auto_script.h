/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file auto_script.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/12/20
 * @license
 * @description
 */

#ifndef __auto_script_H_wgJw5pNm_lv0V_HjKa_sVLm_u0fs8Oss4Kq0__
#define __auto_script_H_wgJw5pNm_lv0V_HjKa_sVLm_u0fs8Oss4Kq0__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  operation code of auto-script
 */
typedef enum ascript_opc
{
    ASCRIPT_OPC_READ        = 0x55A0,
    ASCRIPT_OPC_WRITE       = 0x55A1,
    ASCRIPT_OPC_DELAY       = 0x55A2,
    ASCRIPT_OPC_WAIT_STATE  = 0x55A3,
    ASCRIPT_OPC_MASK_WRITE  = 0x55A4,
    ASCRIPT_OPC_MASK_READ   = 0x55A5,
    ASCRIPT_OPC_OR          = 0x55A6,

} ascript_opc_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct opc_argv
{
    union {
        uint32_t    reg_addr;
        uint32_t    ticks;
        uint32_t    or_value;
    };

    union {
        uint32_t    value;
        uint32_t    mask;
    };

    uint32_t        rd_mask;
} opc_argv_t;

typedef struct ascript_line
{
    uint32_t        op_code; // enum ascript_opc

    union {
        opc_argv_t      opc_argv;
        struct {
            uint32_t    argv[3];
        } def;
    };

} ascript_line_t;

typedef struct ascript_cmd_desc
{
    ascript_opc_t   op_code;
    uint32_t        argument_cnt;

    int     (*handler)(opc_argv_t *pArgv);

} ascript_cmd_desc_t;

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
ascript_exec(
    uint8_t     *pBuf_script,
    uint32_t    buf_len);


#ifdef __cplusplus
}
#endif

#endif
