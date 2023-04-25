/*
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

#include "arm_math.h"
#include "mfcc.h"

#include "sys_methods.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define SAMP_FREQ           16000
#define NUM_FBANK_BINS      40
#define MEL_LOW_FREQ        20
#define MEL_HIGH_FREQ       4000

#define M_2PI               6.283185307179586476925286766559005f
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
#define sys_malloc(sz, tag, subtag)                     g_pSys_call->cb_malloc(sz, tag, subtag)
#define sys_free(ptr, tag, subtag)                      g_pSys_call->cb_free(ptr, tag, subtag)
#define sys_mem_debug()                                 g_pSys_call->cb_debug()
#define sys_dump_mem(prefix, pAddr, len, dump_type)     g_pSys_call->cb_dump_mem(prefix, pAddr, len, dump_type)

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern sys_methods_t       g_sys_methods_win32;
static sys_methods_t       *g_pSys_call = &g_sys_methods_win32;

static int      g_num_mfcc_features;
static int      g_frame_len;
static int      g_frame_len_padded;
static int      g_mfcc_dec_bits;
static float    *g_frame;
static float    *g_buffer;
static float    *g_mel_energies;
static float    *g_window_func;
static int32_t  *g_fbank_filter_first;
static int32_t  *g_fbank_filter_last;
static float    **g_mel_fbank;
static float    *g_dct_matrix;

static arm_rfft_fast_instance_f32 *g_rfft;

//=============================================================================
//                  Private Function Definition
//=============================================================================
static inline float InverseMelScale(float mel_freq)
{
    return 700.0f * (expf(mel_freq / 1127.0f) - 1.0f);
}

static inline float MelScale(float freq)
{
    return 1127.0f * logf(1.0f + freq / 700.0f);
}

static float** _create_mel_fbank()
{
    int32_t     num_fft_bins = g_frame_len_padded / 2;
    float       fft_bin_width = ((float)SAMP_FREQ) / g_frame_len_padded;
    float       mel_low_freq = MelScale(MEL_LOW_FREQ);
    float       mel_high_freq = MelScale(MEL_HIGH_FREQ);
    float       mel_freq_delta = (mel_high_freq - mel_low_freq) / (NUM_FBANK_BINS + 1);

    float   *this_bin = sys_malloc(sizeof(float) * num_fft_bins, FOURCC('m', 'f', 't', 'm'), 0);
    float   **mel_fbank = sys_malloc(sizeof(float*) * NUM_FBANK_BINS, FOURCC('m', 'f', 'm', 'f'), -1);

    for(int bin = 0; bin < NUM_FBANK_BINS; bin++)
    {
        float   left_mel = mel_low_freq + bin * mel_freq_delta;
        float   center_mel = mel_low_freq + (bin + 1) * mel_freq_delta;
        float   right_mel = mel_low_freq + (bin + 2) * mel_freq_delta;

        int32_t first_index = -1, last_index = -1;

        for(int i = 0; i < num_fft_bins; i++)
        {
            float freq = (fft_bin_width * i);  // center freq of this fft bin.
            float mel = MelScale(freq);
            this_bin[i] = 0.0;

            if (mel > left_mel && mel < right_mel)
            {
                float weight;
                if (mel <= center_mel)
                {
                    weight = (mel - left_mel) / (center_mel - left_mel);
                }
                else
                {
                    weight = (right_mel - mel) / (right_mel - center_mel);
                }

                this_bin[i] = weight;
                if (first_index == -1)
                    first_index = i;

                last_index = i;
            }
        }

        g_fbank_filter_first[bin] = first_index;
        g_fbank_filter_last[bin]  = last_index;
        mel_fbank[bin] = sys_malloc(sizeof(float) * (last_index - first_index + 1), FOURCC('m', 'f', 'm', 'f'), bin);

        //copy the part we care about
        for(int i = first_index, j= 0; i <= last_index; i++)
        {
            mel_fbank[bin][j++] = this_bin[i];
        }
    }

    sys_free(this_bin, FOURCC('m', 'f', 't', 'm'), 0);
    return mel_fbank;
}

static float* create_dct_matrix(int32_t input_length, int32_t coefficient_count)
{
    float   *M = sys_malloc(sizeof(float) * input_length * coefficient_count, FOURCC('m', 'f', 'd', 'c'), 0);
    float   normalizer = 0.0f;

    arm_sqrt_f32(2.0f / (float)input_length, &normalizer);

    for(int k = 0; k < coefficient_count; k++)
    {
        for(int n = 0; n < input_length; n++)
        {
            M[k * input_length + n] = normalizer * cos( ((double)M_PI) / input_length * (n + 0.5f) * k );
        }
    }
    return M;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
void mfcc_init(int num_mfcc_features, int frame_len, int mfcc_dec_bits)
{
    g_num_mfcc_features = num_mfcc_features;
    g_frame_len         = frame_len;
    g_mfcc_dec_bits     = mfcc_dec_bits;

    /**
     *  Round-up to nearest power of 2.
     */
    g_frame_len_padded = pow(2, ceil((log(frame_len)/log(2))));

    g_frame        = sys_malloc(sizeof(float) * g_frame_len_padded, FOURCC('m', 'f', 'i', 'f'), 0);
    g_buffer       = sys_malloc(sizeof(float) * g_frame_len_padded, FOURCC('m', 'f', 'i', 'b'), 0);
    g_mel_energies = sys_malloc(sizeof(float) * NUM_FBANK_BINS, FOURCC('m', 'f', 'i', 'm'), 0);

    /**
     *  create window function
     */
    g_window_func = sys_malloc(sizeof(float) * frame_len, FOURCC('m', 'f', 'i', 'w'), 0);
    for(int i = 0; i < frame_len; i++)
        g_window_func[i] = 0.5f - 0.5f * cos(M_2PI * ((float)i) / (frame_len));

    sys_dump_mem("window_func@", (void*)g_window_func, frame_len, (SYS_MEM_DUMP_TYPE_FLOAT | SYS_MEM_DUMP_TYPE_FILE));

    /**
     *  create mel filterbank
     */
    g_fbank_filter_first = sys_malloc(sizeof(int32_t) * NUM_FBANK_BINS, FOURCC('m', 'f', 'i', '1'), 0);
    g_fbank_filter_last  = sys_malloc(sizeof(int32_t) * NUM_FBANK_BINS, FOURCC('m', 'f', 'i', 'e'), 0);

    g_mel_fbank = _create_mel_fbank();

    sys_dump_mem("fbank_filter_first@", (void*)g_fbank_filter_first, NUM_FBANK_BINS, (SYS_MEM_DUMP_TYPE_U32 | SYS_MEM_DUMP_TYPE_FILE));
    sys_dump_mem("fbank_filter_last@", (void*)g_fbank_filter_last, NUM_FBANK_BINS, (SYS_MEM_DUMP_TYPE_U32 | SYS_MEM_DUMP_TYPE_FILE));

    /**
     *  create DCT matrix
     */
    g_dct_matrix = create_dct_matrix(NUM_FBANK_BINS, num_mfcc_features);

    sys_dump_mem("dct_matrix@", (void*)g_dct_matrix, NUM_FBANK_BINS*num_mfcc_features, (SYS_MEM_DUMP_TYPE_FLOAT | SYS_MEM_DUMP_TYPE_FILE));

    /**
     *  initialize FFT
     */
    g_rfft = sys_malloc(sizeof(arm_rfft_fast_instance_f32), FOURCC('m', 'f', 'i', 'r'), 0);
    arm_rfft_fast_init_f32(g_rfft, g_frame_len_padded);

    return;
}

void mfcc_deinit(void)
{
    sys_free(g_frame, FOURCC('m', 'f', 'i', 'f'), 0);
    sys_free(g_buffer, FOURCC('m', 'f', 'i', 'b'), 0);
    sys_free(g_mel_energies, FOURCC('m', 'f', 'i', 'm'), 0);
    sys_free(g_window_func, FOURCC('m', 'f', 'i', 'w'), 0);
    sys_free(g_fbank_filter_first, FOURCC('m', 'f', 'i', '1'), 0);
    sys_free(g_fbank_filter_last, FOURCC('m', 'f', 'i', 'e'), 0);
    sys_free(g_dct_matrix, FOURCC('m', 'f', 'd', 'c'), 0);
    sys_free(g_rfft, FOURCC('m', 'f', 'i', 'r'), 0);

    for(int i = 0; i < NUM_FBANK_BINS; i++)
        sys_free(g_mel_fbank[i], FOURCC('m', 'f', 'm', 'f'), i);
    sys_free(g_mel_fbank, FOURCC('m', 'f', 'm', 'f'), -1);

    return;
}


void mfcc_compute(const int16_t *audio_data, q7_t *mfcc_out)
{
    int32_t i, j, bin;

    /**
     *  TensorFlow way of normalizing .wav data to (-1,1)
     */
    for(int i = 0; i < g_frame_len; i++)
    {
        g_frame[i] = (float)audio_data[i] / (1 << 15);
    }

    /**
     *  Fill up remaining with zeros
     */
    memset(&g_frame[g_frame_len], 0, sizeof(float) * (g_frame_len_padded - g_frame_len));

    for(int i = 0; i < g_frame_len; i++)
    {
        g_frame[i] *= g_window_func[i];
    }

    /**
     *  Compute FFT
     */
    arm_rfft_fast_f32(g_rfft, g_frame, g_buffer, 0);

    /**
     *  Convert to power spectrum
     *  frame is stored as [real0, realN/2-1, real1, im1, real2, im2, ...]
     */
    int     half_dim = g_frame_len_padded / 2;
    float   first_energy = g_buffer[0] * g_buffer[0];
    float   last_energy =  g_buffer[1] * g_buffer[1];  // handle this special case

    for(int i = 1; i < half_dim; i++)
    {
        float   real = g_buffer[i * 2], im = g_buffer[i * 2 + 1];
        g_buffer[i] = real * real + im * im;
    }

    g_buffer[0]        = first_energy;
    g_buffer[half_dim] = last_energy;

    float   sqrt_data;

    /**
     *  Apply mel filterbanks
     */
    for(int bin = 0; bin < NUM_FBANK_BINS; bin++)
    {
        float   mel_energy = 0;
        int32_t first_index = g_fbank_filter_first[bin];
        int32_t last_index = g_fbank_filter_last[bin];

        for(int i = first_index, j = 0; i <= last_index; i++)
        {
            arm_sqrt_f32(g_buffer[i], &sqrt_data);
            mel_energy += (sqrt_data) * g_mel_fbank[bin][j++];
        }
        g_mel_energies[bin] = mel_energy;

        //avoid log of zero
        if( mel_energy == 0.0f )
            g_mel_energies[bin] = FLT_MIN;
    }

    /**
     *  Take log
     */
    for(int bin = 0; bin < NUM_FBANK_BINS; bin++)
        g_mel_energies[bin] = logf(g_mel_energies[bin]);

    /**
     *  Take DCT. Uses matrix mul.
     */
    for(int i = 0; i < g_num_mfcc_features; i++)
    {
        float   sum = 0.0;
        for(int j = 0; j < NUM_FBANK_BINS; j++)
        {
            sum += g_dct_matrix[i * NUM_FBANK_BINS + j] * g_mel_energies[j];
        }

        //Input is Qx.mfcc_dec_bits (from quantization step)
        sum *= (0x1 << g_mfcc_dec_bits);
        sum = round(sum);

        if(sum >= 127)
            mfcc_out[i] = 127;
        else if(sum <= -128)
            mfcc_out[i] = -128;
        else
            mfcc_out[i] = sum;
    }
    return;
}
