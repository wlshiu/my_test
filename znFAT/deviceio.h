/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file deviceio.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/09/17
 * @license
 * @description
 */

#ifndef __deviceio_H_wqTSd0RO_lsWl_H1Ex_s5ja_uG6NCzJwqp2e__
#define __deviceio_H_wqTSd0RO_lsWl_H1Ex_s5ja_uG6NCzJwqp2e__

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
struct znFAT_IO_Ctl //底层驱动接口的IO频度控制体
{
    uint32_t just_sec;
    uint8_t  just_dev;
};
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
uint8_t znFAT_Device_Init(void);
uint8_t znFAT_Device_Read_Sector(uint32_t addr, uint8_t *buffer);
uint8_t znFAT_Device_Write_Sector(uint32_t addr, uint8_t *buffer);
uint8_t znFAT_Device_Read_nSector(uint32_t nsec, uint32_t addr, uint8_t *buffer);
uint8_t znFAT_Device_Write_nSector(uint32_t nsec, uint32_t addr, uint8_t *buffer);
uint8_t znFAT_Device_Clear_nSector(uint32_t nsec, uint32_t addr);


#ifdef __cplusplus
}
#endif

#endif
