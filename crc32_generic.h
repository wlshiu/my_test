/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file crc32_generic.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/11/24
 * @license
 * @description
 */

#ifndef __crc32_generic_H_wjiVp4Vb_lGgR_HtkS_soHV_uHtTdv3x0uv9__
#define __crc32_generic_H_wjiVp4Vb_lGgR_HtkS_soHV_uHtTdv3x0uv9__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_TARGET_CRC_ALGO          CRC_ALGO_CRC32_802_3
//#define CONFIG_TARGET_CRC_ALGO          CRC_ALGO_CRC32_MPEG2
//#define CONFIG_TARGET_CRC_ALGO          CRC_ALGO_CRC16_USB

typedef enum crc_algo
{
    CRC_ALGO_CRC32_802_3      = 0,
    CRC_ALGO_CRC32_MPEG2,
    CRC_ALGO_CRC16_USB,

    CRC_ALGO_ALL,
} crc_algo_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

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
CRC_Init(crc_algo_t algo_type, uint32_t *pCRC_InitValue);

uint32_t
CRC_Calc(
    uint32_t    crc_value,
    uint8_t     *data,
    int         data_len);


#ifdef __cplusplus
}
#endif

#endif
