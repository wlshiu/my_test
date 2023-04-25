/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file kws.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/04/24
 * @license
 * @description
 */

#ifndef __kws_H_wFUD29wp_l66i_HVi1_sbo3_uZOM07RgTUrD__
#define __kws_H_wFUD29wp_l66i_HVi1_sbo3_uZOM07RgTUrD__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "arm_math.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct kws_ds_cnn_cfg
{
    int     record_win;
    int     sliding_win_len;
    int16_t *audio_data_buffer;
} kws_ds_cnn_cfg_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void kws_ds_cnn_init(kws_ds_cnn_cfg_t *pCfg);


void kws_ds_cnn_deinit(void);

void kws_extract_features(void);
void kws_classify(void);
int kws_get_top_class(q7_t *prediction);
void kws_average_predictions(void);

q7_t* kws_get_output(void);

#ifdef __cplusplus
}
#endif

#endif
