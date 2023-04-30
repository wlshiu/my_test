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

/*
 * Description: Example code for running keyword spotting
 */

#include <stdlib.h>
#include <stdio.h>
#include "kws.h"

#include "yes.0ac15fe9_nohash_0.wav.h"
//#include "right_1.wav.h"
//#include "right_2.wav.h"

static char  g_classes[12][8] =
{
    "Silence", "Unknown", "yes", "no", "up", "down", "left", "right", "on", "off", "stop", "go"
};

static int16_t     *pADC_input = (int16_t*)audio_buffer;

int main()
{
    kws_ds_cnn_cfg_t    cfg = {0};
    int                 max_ind = 0;

    #if 0
    g_audio_buffer_size = sizeof(audio_buffer);

    cfg.audio_data_buffer = (int16_t*)&audio_buffer;
    #else
    cfg.record_win = 3;
    cfg.sliding_win_len = 3;
    #endif

    kws_ds_cnn_init(&cfg);

    while( pADC_input < &audio_buffer[sizeof(audio_buffer) >> 1] )
    {
        memcpy((void*)&g_audio_buffer[g_frame_len - g_frame_shift], pADC_input, g_audio_buffer_size);
        pADC_input += (g_audio_buffer_size >> 1);

        kws_extract_features(); //extract mfcc features
        kws_classify();	        //classify using dnn
    }

    q7_t    *pKWS_out = kws_get_output();
    max_ind = kws_get_top_class(pKWS_out);

    printf("Detected %s (%d%%)\n", g_classes[max_ind], ((int)pKWS_out[max_ind] * 100 / 128));

    system("pause");
    return 0;
}



