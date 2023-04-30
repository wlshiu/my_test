/**
 * Copyright (C) 2018 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file kws_ds_cnn.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/04/24
 * @license
 * @description
 */


#include "kws.h"
#include "nn.h"
#include "sys_methods.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define sys_malloc(sz, tag, subtag)     g_pSys_call->cb_malloc(sz, tag, subtag)
#define sys_free(ptr, tag, subtag)      g_pSys_call->cb_free(ptr, tag, subtag)
#define sys_mem_debug()                 g_pSys_call->cb_debug()
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern sys_methods_t       g_sys_methods_win32;
static sys_methods_t       *g_pSys_call = &g_sys_methods_win32;

int16_t     *g_audio_buffer;
int         g_audio_buffer_size;
int         g_frame_len;
int         g_frame_shift;

static int g_mfcc_buffer_size;
static int g_recording_win;
static int g_sliding_window_len;

static q7_t     *g_mfcc_buffer;
static q7_t     *g_output;
static q7_t     *g_predictions;
static q7_t     *g_averaged_output;
static int  g_num_frames;
static int  g_num_mfcc_features;

static int  g_num_out_classes;


//=============================================================================
//                  Private Function Definition
//=============================================================================
static void _init_kws()
{
    g_num_mfcc_features = nn_get_num_mfcc_features();
    g_num_frames        = nn_get_num_frames();
    g_frame_len         = nn_get_frame_len();
    g_frame_shift       = nn_get_frame_shift();
    g_num_out_classes   = nn_get_num_out_classes();

    int     mfcc_dec_bits = nn_get_in_dec_bits();

    mfcc_init(g_num_mfcc_features, g_frame_len, mfcc_dec_bits);

    g_mfcc_buffer     = (q7_t *)sys_malloc(sizeof(q7_t) * g_num_frames * g_num_mfcc_features, FOURCC('k', 'w', 'i', 'm'), 0);
    g_output          = (q7_t *)sys_malloc(sizeof(q7_t) * g_num_out_classes, FOURCC('k', 'w', 'i', 'o'), 0);
    g_averaged_output = (q7_t *)sys_malloc(sizeof(q7_t) * g_num_out_classes, FOURCC('k', 'w', 'i', 'a'), 0);
    g_predictions     = (q7_t *)sys_malloc(sizeof(q7_t) * g_sliding_window_len * g_num_out_classes, FOURCC('k', 'w', 'i', 'p'), 0);

    int  audio_block_size;

    audio_block_size  = g_recording_win * g_frame_shift;
    g_audio_buffer_size = audio_block_size + g_frame_len - g_frame_shift;

    if( !g_audio_buffer )
    {
        g_audio_buffer = sys_malloc(sizeof(int16_t) * g_audio_buffer_size, FOURCC('k', 'w', 'a', 'b'), 0);
    }

    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void kws_ds_cnn_init(kws_ds_cnn_cfg_t *pCfg)
{
    nn_init();

    if( pCfg->audio_data_buffer )
    {
        g_recording_win         = nn_get_num_frames();
        g_audio_buffer          = pCfg->audio_data_buffer;
        g_sliding_window_len    = 1;
    }
    else
    {
        g_recording_win      = pCfg->record_win;
        g_sliding_window_len = pCfg->sliding_win_len;
    }

    _init_kws();

    return;
}

void kws_ds_cnn_deinit(void)
{
    nn_deinit();
    return;
}

void kws_extract_features(void)
{
    if( g_num_frames > g_recording_win)
    {
        //move old features left
        memmove(g_mfcc_buffer, g_mfcc_buffer + (g_recording_win * g_num_mfcc_features),
                (g_num_frames - g_recording_win) * g_num_mfcc_features);
    }

    //compute features only for the newly recorded audio
    int32_t     mfcc_buffer_head = (g_num_frames - g_recording_win) * g_num_mfcc_features;
    for(uint16_t f = 0; f < g_recording_win; f++)
    {
        mfcc_compute(g_audio_buffer + (f * g_frame_shift), &g_mfcc_buffer[mfcc_buffer_head]);

        mfcc_buffer_head += g_num_mfcc_features;
    }
}

void kws_classify(void)
{
    nn_run(g_mfcc_buffer, g_output);

    // Softmax
    arm_softmax_q7(g_output, g_num_out_classes, g_output);
    return;
}

int kws_get_top_class(q7_t *prediction)
{
    int     max_ind = 0;
    int     max_val = -128;
    for(int i = 0; i < g_num_out_classes; i++)
    {
        if(max_val < prediction[i])
        {
            max_val = prediction[i];
            max_ind = i;
        }
    }
    return max_ind;
}

void kws_average_predictions(void)
{
    // shift the old predictions left
    arm_copy_q7((q7_t*)(g_predictions + g_num_out_classes),
                (q7_t *)g_predictions,
                (g_sliding_window_len - 1) * g_num_out_classes);

    // add new predictions at the end
    arm_copy_q7((q7_t *)g_output,
                (q7_t *)(g_predictions + (g_sliding_window_len - 1) * g_num_out_classes),
                g_num_out_classes);

    //compute averages
    for(int j = 0, sum = 0; j < g_num_out_classes; j++)
    {
        sum = 0;
        for(int i = 0; i < g_sliding_window_len; i++)
            sum += g_predictions[i * g_num_out_classes + j];

        g_averaged_output[j] = (q7_t)(sum / g_sliding_window_len);
    }
}

q7_t *kws_get_output(void)
{
    sys_mem_debug();
    return g_output;
}
