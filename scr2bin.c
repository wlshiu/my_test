/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file scr2bin.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/04/04
 * @license
 * @description
 */


#include "scr2bin.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum    opcode
{
    OPCODE_READ         = 0x55A0,
    OPCODE_WRITE        = 0x55A1,
    OPCODE_DELAY        = 0x55A2,
    OPCODE_WAIT_STATUS  = 0x55A3,
    OPCODE_MASK_WRITE   = 0x55A4,
    OPCODE_MASK_READ    = 0x55A5,
    OPCODE_OR           = 0x55A6,
} opcode_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct param
{
    opcode_t        opcode;

    union {
        struct {
            uint32_t    reg;
            uint32_t    value;
            uint32_t    mask;
        } maskwr;

        struct {
            uint32_t    reg;
            uint32_t    value;
        } wr;

        struct {
            uint32_t    reg;
        } rd;

        struct {
            uint32_t    reg;
            uint32_t    mask;
        } maskrd;

        struct {
            uint32_t    mask;
        } or;

        struct {
            uint32_t    ticks;
        } delay;

        struct {
            uint32_t    reg;
            uint32_t    value;
            uint32_t    mask;
        } wait_state;
    };
} param_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void _usage(char *argv)
{
    /**
     *  # API:
     *  # write(register, value)
     *  #   *register = value
     *
     *  # read(register)
     *
     *  # mask_write(register, value, mask)
     *  #   *register = value & mask
     *
     *  # wait_status(register, value, mask)
     *  #   while( (*register & mask) != value ) {}
     *  #
     *
     *  # mask_read(register, mask)
     *  #
     *
     *  # or(value1, value2)
     *  #       (value1 | value2)
     *
     *  # delay(ticks)
     *  #
     */
    printf("usage: %s <scr file> <output bin file>\n", argv[0]);
    printf("    script to binary file\n");
    exit(-1);
    return;
}


static void
_trim_all_spaces(char *str)
{
    char    *dest = str;
    do {
        while( *dest == ' ' )
            ++dest;
    } while( (*str++ = *dest++) != 0 );

    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
scr_to_bin(char *pLine, uint32_t *pBuf, int *pBuf_len)
{
    int         rval = 0;
    param_t     param = { .opcode = 0, };

    _trim_all_spaces(pLine);

    do {
        int     i = 0;
        if( sscanf(pLine, "mask_write(0x%x,data,0x%x)", &param.maskwr.reg, &param.maskwr.mask) == 2 )
        {
            pBuf[i++] = OPCODE_MASK_WRITE;
            pBuf[i++] = param.maskwr.reg;
            pBuf[i++] = param.maskwr.mask;
            *pBuf_len = sizeof(uint32_t) * i;
            break;
        }
        else if( sscanf(pLine, "write(0x%x,0x%x)", &param.wr.reg, &param.wr.value) == 2 )
        {
            pBuf[i++] = OPCODE_WRITE;
            pBuf[i++] = param.wr.reg;
            pBuf[i++] = param.wr.value;
            *pBuf_len = sizeof(uint32_t) * i;
            break;
        }
        else if( sscanf(pLine, "data=read(0x%x)", &param.rd.reg) == 1 )
        {
            pBuf[i++] = OPCODE_READ;
            pBuf[i++] = param.rd.reg;
            *pBuf_len = sizeof(uint32_t) * i;
            break;
        }
        else if( sscanf(pLine, "delay(0x%x)", &param.delay.ticks) == 1 )
        {
            pBuf[i++] = OPCODE_DELAY;
            pBuf[i++] = param.delay.ticks;
            *pBuf_len = sizeof(uint32_t) * i;
            break;
        }
        else if( sscanf(pLine, "delay(%d)", &param.delay.ticks) == 1 )
        {
            pBuf[i++] = OPCODE_DELAY;
            pBuf[i++] = param.delay.ticks;
            *pBuf_len = sizeof(uint32_t) * i;
            break;
        }
        else if( sscanf(pLine, "or(data,0x%x)", &param.or.mask) == 1 )
        {
            pBuf[i++] = OPCODE_OR;
            pBuf[i++] = param.or.mask;
            *pBuf_len = sizeof(uint32_t) * i;
            break;
        }
        else if( sscanf(pLine, "data=mask_read(0x%x,0x%x)", &param.maskrd.reg, &param.maskrd.mask) == 2 )
        {
            pBuf[i++] = OPCODE_MASK_READ;
            pBuf[i++] = param.maskrd.reg;
            pBuf[i++] = param.maskrd.mask;
            *pBuf_len = sizeof(uint32_t) * i;
            break;
        }
        else if( sscanf(pLine, "wait_status(0x%x,0x%x,0x%x)", &param.wait_state.reg, &param.wait_state.value, &param.wait_state.mask) == 3 )
        {
            pBuf[i++] = OPCODE_WAIT_STATUS;
            pBuf[i++] = param.wait_state.reg;
            pBuf[i++] = param.wait_state.value;
            pBuf[i++] = param.wait_state.mask;
            *pBuf_len = sizeof(uint32_t) * i;
            break;
        }
        else
        {
            *pBuf_len = 0;
        }

        rval = 0;
    } while(0);

    return rval;
}
