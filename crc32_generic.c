/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file crc32_generic.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/11/24
 * @license
 * @description
 */



#include <stdio.h>
#include <stdlib.h>
#include "crc32_generic.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct crc_algo_attr
{
    char        *pName;
    crc_algo_t  algo_type;
    int         width;
    uint32_t    poly;
    uint32_t    init_value;
    bool        is_in_reflecte;
    bool        is_out_reflecte;
    uint32_t    xor_out;
} crc_algo_attr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint32_t         g_algo_idx = 0;
static uint32_t         g_crc_table[256] = {0};


/**
http://www.ip33.com/crc.html

| CRC-Algo           | Formula  representations                                                       | Width     | Polynomial    | Initial   | XOR Out   | Input Reflecte    | Output Reflecte |
| :-:                | :-:                                                                            | :-:       | :-:           | :-:       | :-:       | :-:               | :-:             |
| CRC-4/ITU          | x4 + x + 1                                                                     | 4         | 03            | 00        | 00        | true              | true            |
| CRC-5/EPC          | x5 + x3 + 1                                                                    | 5         | 09            | 09        | 00        | false             | false           |
| CRC-5/ITU          | x5 + x4 + x2 + 1                                                               | 5         | 15            | 00        | 00        | true              | true            |
| CRC-5/USB          | x5 + x2 + 1                                                                    | 5         | 05            | 1F        | 1F        | true              | true            |
| CRC-6/ITU          | x6 + x + 1                                                                     | 6         | 03            | 00        | 00        | true              | true            |
| CRC-7/MMC          | x7 + x3 + 1                                                                    | 7         | 09            | 00        | 00        | false             | false           |
| CRC-8              | x8 + x2 + x + 1                                                                | 8         | 07            | 00        | 00        | false             | false           |
| CRC-8/ITU          | x8 + x2 + x + 1                                                                | 8         | 07            | 00        | 55        | false             | false           |
| CRC-8/ROHC         | x8 + x2 + x + 1                                                                | 8         | 07            | FF        | 00        | true              | true            |
| CRC-8/MAXIM        | x8 + x5 + x4 + 1                                                               | 8         | 31            | 00        | 00        | true              | true            |
| CRC-16/IBM         | x16 + x15 + x2 + 1                                                             | 16        | 8005          | 0000      | 0000      |true               | true            |
| CRC-16/MAXIM       | x16 + x15 + x2 + 1                                                             | 16        | 8005          | 0000      | FFFF      |true               | true            |
| CRC-16/USB         | x16 + x15 + x2 + 1                                                             | 16        | 8005          | FFFF      | FFFF      |true               | true            |
| CRC-16/MODBUS      | x16 + x15 + x2 + 1                                                             | 16        | 8005          | FFFF      | 0000      |true               | true            |
| CRC-16/CCITT       | x16 + x12 + x5 + 1                                                             | 16        | 1021          | 0000      | 0000      |true               | true            |
| CRC-16/CCITT-FALSE | x16 + x12 + x5 + 1                                                             | 16        | 1021          | FFFF      | 0000      |false              | false           |
| CRC-16/X25         | x16 + x12 + x5 + 1                                                             | 16        | 1021          | FFFF      | FFFF      |true               | true            |
| CRC-16/XMODEM      | x16 + x12 + x5 + 1                                                             | 16        | 1021          | 0000      | 0000      |false              | false           |
| CRC-16/DNP         | x16 + x13 + x12 + x11 + x10 + x8 + x6 + x5 + x2 + 1                            | 16        | 3D65          | 0000      | FFFF      | true              | true            |
| CRC-32 (802.3)     | x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1 | 32        | 04C11DB7      | FFFFFFFF  | FFFFFFFF  | true              | true            |
| CRC-32/MPEG-2      | x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1 | 32        | 04C11DB7      | FFFFFFFF  | 00000000  | false             | false           |
 */

static crc_algo_attr_t          g_crc_algo[] =
{
    { .algo_type = CRC_ALGO_CRC32_802_3, .pName = "CRC32-803.2", .width = 32, .poly = 0x04C11DB7, .init_value = 0xFFFFFFFF, .xor_out = 0xFFFFFFFF, .is_in_reflecte = true,  .is_out_reflecte = true,  },
    { .algo_type = CRC_ALGO_CRC32_MPEG2, .pName = "CRC32-MPEG2", .width = 32, .poly = 0x04C11DB7, .init_value = 0xFFFFFFFF, .xor_out = 0x00000000, .is_in_reflecte = false, .is_out_reflecte = false, },
    { .algo_type = CRC_ALGO_CRC16_USB,   .pName = "CRC32-USB",   .width = 16, .poly = 0x8005,     .init_value = 0xFFFF,     .xor_out = 0xFFFF,     .is_in_reflecte = true,  .is_out_reflecte = true,  },
    { .algo_type = CRC_ALGO_ALL, .pName = 0, },
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t
ReverseBits(
    uint32_t  value,
    int       valueLength)
{
    uint32_t    output = 0;
    for (int i = valueLength - 1; i >= 0; i--)
    {
        output |= (value & 1) << i;
        value >>= 1;
    }
    return output;
}

static void
CRC_CreateTable(
    char        *pName,
    int         width,
    uint32_t    polynomial,
    bool        isInputReflected,
    bool        isOutputReflected)
{
    if (width < 8 || width > 32)
    {
        printf("width = 8 * n, and  8 <= width <= 32\n");
        return;
    }

    for(int i = 0; i < sizeof(g_crc_table) / sizeof(g_crc_table[0]); i++)
    {
        uint32_t r = (uint32_t)i;

        if (isInputReflected)
        {
            r = ReverseBits(r, width);
        }
        else if (width > 8)
        {
            r <<= width - 8;
        }

        uint32_t    lastBit = 1ul << (width - 1);

        for(int j = 0; j < 8; j++)
        {
            if( (r & lastBit) != 0 )
            {
                r = (r << 1) ^ polynomial;
            }
            else
            {
                r <<= 1;
            }
        }

        if( isInputReflected )
        {
            r = ReverseBits(r, width);
        }

        g_crc_table[i] = r;
    }

    printf("\nAlgo-Name: %s\n", pName);
    for(int i = 0; i < sizeof(g_crc_table) >> 3; i++)
    {
        if( !(i & 0x3) )
            printf("\n");
        printf("0x%08X, ", g_crc_table[i]);
    }
    printf("\n\n");

}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int CRC_Init(crc_algo_t algo_type, uint32_t *pCRC_InitValue)
{
    int                 rval = 0;
    crc_algo_attr_t     *pAlgo_act = 0;
    crc_algo_attr_t     *pAlgo_cur = 0;

    g_algo_idx = 0;

    pAlgo_cur = &g_crc_algo[g_algo_idx];
    while( pAlgo_cur->algo_type != CRC_ALGO_ALL )
    {
        if( pAlgo_cur->algo_type == CONFIG_TARGET_CRC_ALGO )
        {
            pAlgo_act = pAlgo_cur;
            break;
        }

        g_algo_idx++;
        pAlgo_cur = &g_crc_algo[g_algo_idx];
    }

    if( pCRC_InitValue )
        *pCRC_InitValue = pAlgo_act->init_value;

    CRC_CreateTable(pAlgo_act->pName,
                    pAlgo_act->width,
                    pAlgo_act->poly,
                    pAlgo_act->is_in_reflecte,
                    pAlgo_act->is_out_reflecte);
    return rval;
}

uint32_t CRC_Calc(uint32_t crc_value, uint8_t *data, int data_len)
{
    if( g_crc_algo[g_algo_idx].algo_type == CRC_ALGO_CRC32_MPEG2 )
    {
        int         ToRight = g_crc_algo[g_algo_idx].width - 8;
        uint32_t    offset = 0;

        ToRight = (ToRight < 0) ? 0 : ToRight;

        while( --data_len >= 0 )
        {
            crc_value = g_crc_table[((crc_value >> ToRight) ^ data[offset++]) & 0xFF] ^ (crc_value << 8);
        }
    }
    else
    {
        for(int i = 0; i < data_len; i++)
        {
            uint8_t index = (uint8_t)(crc_value ^ data[i]);
            crc_value = (crc_value >> 8) ^ g_crc_table[index];
        }
    }
    return crc_value ^ g_crc_algo[g_algo_idx].xor_out;
}
