/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file backtrace.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/01/02
 * @license
 * @description
 */

#ifndef __backtrace_H_w5WWBoxq_lMZq_HeoK_sGmt_uHlFxz6oZh7N__
#define __backtrace_H_w5WWBoxq_lMZq_HeoK_sGmt_uHlFxz6oZh7N__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct backtrace_txt_range
{
    unsigned long       *star;
    unsigned long       *end;
} backtrace_txt_range_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void
backtrace_init(
    backtrace_txt_range_t   *pRange,
    unsigned long           number);


void
backtrace(
    unsigned long   sp_cur,
    unsigned long   sp_start,
    unsigned long   *pFrame,
    int             frame_level);


#ifdef __cplusplus
}
#endif

#endif
