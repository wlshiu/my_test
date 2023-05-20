/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file csv.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/05/20
 * @license
 * @description
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "csv.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_CSV_DELIMITER        ','
//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, ...)       printf("[error] " str, ##__VA_ARGS__)
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
int csv_read(char *pCsv_path, csv_handle_t *pHCsv)
{
    int     rval = 0;
    FILE    *fin = 0;

    if( !pCsv_path || !pHCsv )
        return CSV_ERR_NULL_POINTER;

    do {
        int     filesize = 0;
        int     is_1st_line_end = false;

        if( !(fin = fopen(pCsv_path, "rb")) )
        {
            err("open %s fail !\n", pCsv_path);
            rval = CSV_ERR_OPEN_FAIL;
            break;
        }

        fseek(fin, 0, SEEK_END);
        filesize = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        if( !(pHCsv->pTxt_data = malloc(filesize)) )
        {
            err("malloc %d fail !\n", filesize);
            rval = CSV_ERR_MALLOC_FAIL;
            break;
        }

        fread(pHCsv->pTxt_data, 1, filesize, fin);

        pHCsv->elem_per_line = 0;
        pHCsv->line_num      = 0;

        is_1st_line_end = false;

        for(int i = 0; i < filesize; i++)
        {
            if( pHCsv->pTxt_data[i] == '\n' )
            {
                if( i && is_1st_line_end == false &&
                    pHCsv->pTxt_data[i - 1] != CONFIG_CSV_DELIMITER )
                {
                    pHCsv->elem_per_line++;
                }

                pHCsv->line_num++;
                is_1st_line_end = true;
            }
            else if( is_1st_line_end == false &&
                     pHCsv->pTxt_data[i] == CONFIG_CSV_DELIMITER )
            {
                pHCsv->elem_per_line++;
            }
        }

    } while(0);

    if( fin )   fclose(fin);

    return rval;
}

int csv_txt2array(csv_handle_t *pHCsv, csv_data_type_t dtype)
{
    int     rval = 0;

    do {
        int     len = 0;
        char    *pCur = 0;
        char    delimiter[16] = {0};

        union {
            int     data_width;
            int     data_cnt;
        } u;

        u.data_width = (dtype == CSV_DATA_TYPE_I8)  ? sizeof(uint8_t) :
                     (dtype == CSV_DATA_TYPE_I16) ? sizeof(uint16_t) :
                     (dtype == CSV_DATA_TYPE_I32) ? sizeof(uint32_t) :
                     sizeof(float);

        len = (pHCsv->elem_per_line * (pHCsv->line_num + 1)) * u.data_width;
        if( !(pHCsv->arr.pData_i8 = malloc(len)) )
        {
            err("malloc %d fail !\n", len);
            rval = CSV_ERR_MALLOC_FAIL;
            break;
        }

        u.data_cnt = 0;
        snprintf(delimiter, sizeof(delimiter), "%c \n", CONFIG_CSV_DELIMITER);
        pCur = strtok(pHCsv->pTxt_data, delimiter);

        while( pCur )
        {
           switch( dtype )
            {
                default:
                case CSV_DATA_TYPE_I8:
                    pHCsv->arr.pData_i8[u.data_cnt++] = strtol(pCur, 0, 10);
                    break;
                case CSV_DATA_TYPE_I16:
                    pHCsv->arr.pData_i16[u.data_cnt++] = strtol(pCur, 0, 10);
                    break;
                case CSV_DATA_TYPE_I32:
                    pHCsv->arr.pData_i32[u.data_cnt++] = strtol(pCur, 0, 10);
                    break;
                case CSV_DATA_TYPE_FLOAT:
                    pHCsv->arr.pData_fp[u.data_cnt++] = strtod(pCur, 0);
                    break;
            }

            pCur = strtok(NULL, delimiter);
        }

    } while(0);

    return rval;
}

int csv_write(char *pCsv_path, csv_handle_t *pHCsv, csv_data_type_t dtype)
{
    int     rval = 0;
    FILE    *fout = 0;

    if( !pCsv_path || !pHCsv )
        return CSV_ERR_NULL_POINTER;

    do {
        if( !(fout = fopen(pCsv_path, "w")) )
        {
            err("open %s fail !\n", pCsv_path);
            rval = CSV_ERR_OPEN_FAIL;
            break;
        }

        for(int i = 0; i < pHCsv->line_num; i++)
        {
            for(int j = 0; j < pHCsv->elem_per_line; j++)
            {
                int     offset = i * pHCsv->elem_per_line + j;

                switch( dtype )
                {
                    default:
                    case CSV_DATA_TYPE_I8:
                        fprintf(fout, "%d,", pHCsv->arr.pData_i8[offset]);
                        break;
                    case CSV_DATA_TYPE_I16:
                        fprintf(fout, "%d,", pHCsv->arr.pData_i16[offset]);
                        break;
                    case CSV_DATA_TYPE_I32:
                        fprintf(fout, "%d,", pHCsv->arr.pData_i32[offset]);
                        break;
                    case CSV_DATA_TYPE_FLOAT:
                        fprintf(fout, "%f,", pHCsv->arr.pData_fp[offset]);
                        break;
                }
            }

            fprintf(fout, "\n");
        }

        fprintf(fout, "\n");

    } while(0);

    if( fout )   fclose(fout);

    return rval;
}

int csv_write_ex(char *pCsv_path, csv_wr_cfg_t *pCfg)
{
    int     rval = 0;
    FILE    *fout = 0;

    if( !pCsv_path || !pCfg )
        return CSV_ERR_NULL_POINTER;

    do {
        uint32_t    offset = 0;

        if( !(fout = fopen(pCsv_path, "w")) )
        {
            err("open %s fail !\n", pCsv_path);
            rval = CSV_ERR_OPEN_FAIL;
            break;
        }

        offset = 0;
        while( (offset + pCfg->elem_per_line) < pCfg->total_samples )
        {
            for(int i = 0; i < pCfg->elem_per_line; i++)
            {
                switch( pCfg->dtype )
                {
                    default:
                    case CSV_DATA_TYPE_I8:
                        fprintf(fout, "%d,", pCfg->arr.pData_i8[offset + i]);
                        break;
                    case CSV_DATA_TYPE_I16:
                        fprintf(fout, "%d,", pCfg->arr.pData_i16[offset + i]);
                        break;
                    case CSV_DATA_TYPE_I32:
                        fprintf(fout, "%d,", pCfg->arr.pData_i32[offset + i]);
                        break;
                    case CSV_DATA_TYPE_FLOAT:
                        fprintf(fout, "%f,", pCfg->arr.pData_fp[offset + i]);
                        break;
                }
            }

            fprintf(fout, "\n");

            offset += pCfg->stride;
        }

        fprintf(fout, "\n");

    } while(0);

    if( fout )   fclose(fout);

    return rval;
}

int csv_clear(csv_handle_t *pHCsv)
{
    int     rval = 0;

    if( pHCsv )
    {
        if( pHCsv->pTxt_data )
            free(pHCsv->pTxt_data);

        if( pHCsv->arr.pData_i8 )
            free(pHCsv->arr.pData_i8);

        memset(pHCsv, 0x0, sizeof(csv_handle_t));
    }

    return rval;
}
