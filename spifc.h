/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file spifc.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/06/09
 * @license
 * @description
 */


#ifndef __spifc_H_0284eac6_60f4_4184_b63e_14fb449badb8__
#define __spifc_H_0284eac6_60f4_4184_b63e_14fb449badb8__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_SPIFC_MEM_BASE               0x60000000

#define SPIFC_1_SECTOR_SIZE                 (4 << 10)
#define SPIFC_PAGE_SIZE_POW_2               8
#define SPIFC_PAGE_SIZE                     (1 << SPIFC_PAGE_SIZE_POW_2)

typedef enum spifc_err
{
    SPIFC_ERR_OK    = 0,
    SPIFC_ERR_ADDR_NOT_ALIGNMENT,
    SPIFC_ERR_WRONG_ADDR,
    SPIFC_ERR_WRONG_ARGUMENT,


} spifc_err_t;

typedef enum spifc_erase_mode
{
    SPIFC_ERASE_NONE        = 0,
    SPIFC_ERASE_SECTOR,
    SPIFC_ERASE_BLOCK,
    SPIFC_ERASE_CHIP,

} spifc_erase_mode_t;
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
spifc_err_t
spifc_erase(
    spifc_erase_mode_t  mode,
    uintptr_t           flash_addr,
    int                 unit_num);


spifc_err_t
spifc_read(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes);


spifc_err_t
spifc_write(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes);


spifc_err_t
spifc_program(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes);


#ifdef __cplusplus
}
#endif

#endif


