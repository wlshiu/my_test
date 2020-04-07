/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file rbi.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/07
 * @license
 * @description
 */

#ifndef __rbi_H_wacjyBe6_lUs2_Hcqo_sz5Q_u1uqyxh5Uhqw__
#define __rbi_H_wacjyBe6_lUs2_Hcqo_sz5Q_u1uqyxh5Uhqw__

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
typedef uintptr_t rbi_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
rbi_t
rbi_init(
    uint32_t    slot_size,
    uint32_t    item_size);


int
rbi_deinit(
    rbi_t       hRbi);


int
rbi_pop(
    rbi_t       hRbi,
    uint8_t     *pData,
    int         len);


int
rbi_push(
    rbi_t       hRbi,
    uint8_t     *pData,
    int         len);


#ifdef __cplusplus
}
#endif

#endif
