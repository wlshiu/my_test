/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ds_cnn.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/04/24
 * @license
 * @description
 */

#ifndef __ds_cnn_H_wrPInkg4_lIA7_HWLC_s7QV_uMHKlfQPJ1s3__
#define __ds_cnn_H_wrPInkg4_lIA7_HWLC_s7QV_uMHKlfQPJ1s3__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                  Constant Definition
//=============================================================================
#define SAMP_FREQ           16000
#define MFCC_DEC_BITS       1
#define FRAME_SHIFT_MS      20
#define FRAME_SHIFT         ((int16_t)(SAMP_FREQ * 0.001 * FRAME_SHIFT_MS))
#define NUM_FRAMES          49
#define NUM_MFCC_COEFFS     10
#define MFCC_BUFFER_SIZE    (NUM_FRAMES*NUM_MFCC_COEFFS)
#define FRAME_LEN_MS        40
#define FRAME_LEN           ((int16_t)(SAMP_FREQ * 0.001 * FRAME_LEN_MS))

#define IN_DIM              (NUM_FRAMES*NUM_MFCC_COEFFS)
#define OUT_DIM             12

#define CONV1_OUT_CH        64
#define CONV1_IN_X          NUM_MFCC_COEFFS
#define CONV1_IN_Y          NUM_FRAMES
#define CONV1_KX            4
#define CONV1_KY            10
#define CONV1_SX            2
#define CONV1_SY            2
#define CONV1_PX            1
#define CONV1_PY            4
#define CONV1_OUT_X         5
#define CONV1_OUT_Y         25
#define CONV1_BIAS_LSHIFT   2
#define CONV1_OUT_RSHIFT    6

#define CONV2_OUT_CH        64
#define CONV2_IN_X          CONV1_OUT_X
#define CONV2_IN_Y          CONV1_OUT_Y
#define CONV2_DS_KX         3
#define CONV2_DS_KY         3
#define CONV2_DS_SX         1
#define CONV2_DS_SY         1
#define CONV2_DS_PX         1
#define CONV2_DS_PY         1
#define CONV2_OUT_X         5
#define CONV2_OUT_Y         25
#define CONV2_DS_BIAS_LSHIFT    2
#define CONV2_DS_OUT_RSHIFT     5
#define CONV2_PW_BIAS_LSHIFT    4
#define CONV2_PW_OUT_RSHIFT     8

#define CONV3_OUT_CH        64
#define CONV3_IN_X          CONV2_OUT_X
#define CONV3_IN_Y          CONV2_OUT_Y
#define CONV3_DS_KX         3
#define CONV3_DS_KY         3
#define CONV3_DS_SX         1
#define CONV3_DS_SY         1
#define CONV3_DS_PX         1
#define CONV3_DS_PY         1
#define CONV3_OUT_X         CONV3_IN_X
#define CONV3_OUT_Y         CONV3_IN_Y
#define CONV3_DS_BIAS_LSHIFT    2
#define CONV3_DS_OUT_RSHIFT     4
#define CONV3_PW_BIAS_LSHIFT    5
#define CONV3_PW_OUT_RSHIFT     8

#define CONV4_OUT_CH        64
#define CONV4_IN_X          CONV3_OUT_X
#define CONV4_IN_Y          CONV3_OUT_Y
#define CONV4_DS_KX         3
#define CONV4_DS_KY         3
#define CONV4_DS_SX         1
#define CONV4_DS_SY         1
#define CONV4_DS_PX         1
#define CONV4_DS_PY         1
#define CONV4_OUT_X         CONV4_IN_X
#define CONV4_OUT_Y         CONV4_IN_Y
#define CONV4_DS_BIAS_LSHIFT    3
#define CONV4_DS_OUT_RSHIFT     5
#define CONV4_PW_BIAS_LSHIFT    5
#define CONV4_PW_OUT_RSHIFT     7

#define CONV5_OUT_CH        64
#define CONV5_IN_X          CONV4_OUT_X
#define CONV5_IN_Y          CONV4_OUT_Y
#define CONV5_DS_KX         3
#define CONV5_DS_KY         3
#define CONV5_DS_SX         1
#define CONV5_DS_SY         1
#define CONV5_DS_PX         1
#define CONV5_DS_PY         1
#define CONV5_OUT_X         CONV5_IN_X
#define CONV5_OUT_Y         CONV5_IN_Y
#define CONV5_DS_BIAS_LSHIFT    3
#define CONV5_DS_OUT_RSHIFT     5
#define CONV5_PW_BIAS_LSHIFT    5
#define CONV5_PW_OUT_RSHIFT     8

#define FINAL_FC_BIAS_LSHIFT    2
#define FINAL_FC_OUT_RSHIFT     7

#define SCRATCH_BUFFER_SIZE     (2*2*CONV1_OUT_CH*CONV2_DS_KX*CONV2_DS_KY + 2*CONV2_OUT_CH*CONV2_OUT_X*CONV2_OUT_Y)
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

#ifdef __cplusplus
}
#endif

#endif
