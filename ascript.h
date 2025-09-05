/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ascript.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/09/06
 * @license
 * @description
 */

#ifndef __ascript_H_wLvy63AI_lj6s_HCD7_ssK9_uQKrXkAaBuSU__
#define __ascript_H_wLvy63AI_lj6s_HCD7_ssK9_uQKrXkAaBuSU__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

#define __IO
#define __O
#define __I
#define __IM
#define __OM
#define __IOM

//=============================================================================
//                  Macro Definition
//=============================================================================
#define AScript_Delay(_ticks_)                          ascript_delay(_ticks_)
#define AScript_Reg_Write(_reg_, _val_)                 ascript_reg_write((uint32_t)&(_reg_), _val_)
#define AScript_Reg_Write_Msk(_reg_, _msk_, _val_)      ascript_reg_write_msk((uint32_t)&(_reg_), _msk_, _val_)
#define AScript_Reg_Read_Msk(_reg_, _msk_)              ascript_reg_read_msk((uint32_t)&(_reg_), _msk_)
#define AScript_Wait_Flags(_reg_, _flags_)              ascript_wait_flags((uint32_t)&(_reg_), _flags_)
#define AScript_Set_Flags(_reg_, _flags_)               ascript_set_flags((uint32_t)&(_reg_), _flags_)
#define AScript_Clr_Flags(_reg_, _flags_)               ascript_clr_flags((uint32_t)&(_reg_), _flags_)
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
void ascript_delay(uint32_t ticks);

void ascript_reg_write(uint32_t addr, uint32_t value);

void ascript_reg_write_msk(uint32_t addr, uint32_t mask, uint32_t value);

uint32_t ascript_reg_read_msk(uint32_t addr, uint32_t mask);

void ascript_wait_flags(uint32_t addr, uint32_t flags);

void ascript_set_flags(uint32_t addr, uint32_t flags);

void ascript_clr_flags(uint32_t addr, uint32_t flags);

#ifdef __cplusplus
}
#endif

#endif
