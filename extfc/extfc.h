/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file extfc.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/06/09
 * @license
 * @description
 */


#ifndef __extfc_H_0284eac6_60f4_4184_b63e_14fb449badb8__
#define __extfc_H_0284eac6_60f4_4184_b63e_14fb449badb8__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_EXTFC_MEM_BASE       0//0x60000000


typedef enum extfc_err
{
    EXTFC_ERR_OK    = 0,
    EXTFC_ERR_ADDR_NOT_ALIGNMENT,
    EXTFC_ERR_WRONG_ADDR,
    EXTFC_ERR_WRONG_ARGUMENT,

} extfc_err_t;

typedef enum extfc_erase_mode
{
    EXTFC_ERASE_NONE        = 0,
    EXTFC_ERASE_SECTOR,
    EXTFC_ERASE_BLOCK,
    EXTFC_ERASE_CHIP,

} extfc_erase_mode_t;
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
extfc_err_t
extfc_erase(
    extfc_erase_mode_t  mode,
    uintptr_t           flash_addr,
    int                 unit_num);


extfc_err_t
extfc_read(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes);


extfc_err_t
extfc_write(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes);


extfc_err_t
extfc_program(
    uint8_t     *pSys_buf,
    uintptr_t   flash_addr,
    int         bytes);


#ifdef __cplusplus
}
#endif

#endif


