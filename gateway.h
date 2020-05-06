/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file gateway.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/04
 * @license
 * @description
 */

#ifndef __gateway_H_wWT5fO2O_lSX6_HRIn_sDhP_u8IjvtAvLlHN__
#define __gateway_H_wWT5fO2O_lSX6_HRIn_sDhP_u8IjvtAvLlHN__

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
gateway_init(void);

int
gateway_deinit(void);

int
gateway_routine(void);


#ifdef __cplusplus
}
#endif

#endif