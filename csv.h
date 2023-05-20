/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file csv.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/05/20
 * @license
 * @description
 */

#ifndef __csv_H_wrlHM1cn_lJft_H0JV_s2Dd_uBWVbs8tRyY0__
#define __csv_H_wrlHM1cn_lJft_H0JV_s2Dd_uBWVbs8tRyY0__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum csv_err
{
    CSV_ERR_OK              = 0,
    CSV_ERR_OPEN_FAIL       = -1,
    CSV_ERR_MALLOC_FAIL     = -2,
    CSV_ERR_NULL_POINTER    = -3,

} csv_err_t;

typedef enum csv_data_type
{
    CSV_DATA_TYPE_I8    = 1,
    CSV_DATA_TYPE_I16,
    CSV_DATA_TYPE_I32,
    CSV_DATA_TYPE_FLOAT,
} csv_data_type_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct csv_handle
{
    uint32_t    elem_per_line;
    uint32_t    line_num;

    union {
        char    *pTxt_data; // read CVS as array, elements number is the same in a line
        int     *pValue;
        float   *pFP_value;
    };

    union {
        int8_t      *pData_i8;
        int16_t     *pData_i16;
        int32_t     *pData_i32;
        float       *pData_fp;
    } arr;

} csv_handle_t;

typedef struct csv_wr_cfg
{
    csv_data_type_t     dtype;
    uint32_t            total_samples;
    uint32_t            elem_per_line;
    uint32_t            stride;

    union {
        int8_t      *pData_i8;
        int16_t     *pData_i16;
        int32_t     *pData_i32;
        float       *pData_fp;
    } arr;
} csv_wr_cfg_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int csv_read(char *pCsv_path, csv_handle_t *pHCsv);

int csv_txt2array(csv_handle_t *pHCsv, csv_data_type_t dtype);

int csv_write(char *pCsv_path, csv_handle_t *pHCsv, csv_data_type_t dtype);
int csv_write_ex(char *pCsv_path, csv_wr_cfg_t *pCfg);

int csv_clear(csv_handle_t *pHCsv);

#ifdef __cplusplus
}
#endif

#endif
