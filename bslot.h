/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file flag_buf.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/01/22
 * @license
 * @description
 */

#ifndef __flag_buf_H_wokYSMr8_l695_HftJ_swvR_uVOOXuZokVP8__
#define __flag_buf_H_wokYSMr8_l695_HftJ_swvR_uVOOXuZokVP8__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define BUF_BMP_SIZE            4
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
bslot_get_buffer(
    unsigned long   *bitmap,
    int             bitmap_len);


int
bslot_release_buffer(
    unsigned long   *bitmap,
    int             bitmap_len,
    int             idx);


int
bslot_is_buffer_set(
    unsigned long   *bitmap,
    int             bitmap_len,
    int             idx);


int
bslot_set_buffer(
    unsigned long   *bitmap,
    int             bitmap_len,
    int             idx);

#ifdef __cplusplus
}
#endif

#endif
