/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/05/20
 * @license
 * @description
 */

#include <stdlib.h>
#include <stdio.h>
#include "wav.h"
#include "csv.h"

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
static csv_handle_t     g_hCsv = { .elem_per_line = 0, };
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_usage(char *pProg)
{
    printf("usage: %s <wav path> <output csv path>\n", pProg);
    return -1;
}

static void
_test_csv_read(char *pCsv_path)
{
    csv_read(pCsv_path, &g_hCsv);
    csv_txt2array(&g_hCsv, CSV_DATA_TYPE_FLOAT);

#if 1
    for(int i = 0; i < g_hCsv.line_num; i++)
    {
        for(int j = 0; j < g_hCsv.elem_per_line; j++)
        {
            printf("%f, ", g_hCsv.arr.pData_fp[i * g_hCsv.elem_per_line + j]);
        }
        printf("\n");
    }
    printf("\n");
#endif // 1
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    int             rval = 0;
    wav_handle_t    hWav = { .raw_data.pData = 0, };

    do {
        if( argc < 3 )
        {
            _usage(argv[0]);
            break;
        }

        rval = wav_init(argv[1], &hWav);
        if( rval )  break;

//        wav_dump_info(&hWav);

        csv_wr_cfg_t    cfg = {0};

        cfg.dtype         = CSV_DATA_TYPE_I16;
        cfg.arr.pData_i16 = hWav.raw_data.pPCM;
        cfg.total_samples = hWav.sample_num;
        cfg.elem_per_line = 8;
        cfg.stride        = 4;

        csv_write_ex("wav.csv", &cfg);


    } while(0);

    wav_deinit(&hWav);
    csv_clear(&g_hCsv);

    rval = (rval) ? -1 : 0;
    system("pause");

    return rval;
}
