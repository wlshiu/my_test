/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file mfcc.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/04/24
 * @license
 * @description
 */

#ifndef __mfcc_H_wpDeD2kw_lFSd_Hl0f_s5OD_uuLTEePISSoH__
#define __mfcc_H_wpDeD2kw_lFSd_Hl0f_s5OD_uuLTEePISSoH__

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
void mfcc_init(int num_mfcc_features, int frame_len, int mfcc_dec_bits);
void mfcc_deinit(void);

void mfcc_compute(const int16_t *audio_data, q7_t *mfcc_out);

#ifdef __cplusplus
}
#endif

#endif
