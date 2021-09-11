/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/07/22
 * @license
 * @description
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_KEY_UART         "UART"
#define CONFIG_KEY_SPI          "SPI"
#define CONFIG_KEY_I2C          "I2C"
#define CONFIG_KEY_GPIO         "GPIO"
#define CONFIG_KEY_TIMER        "TIMER"


#define CONFIG_KEY_ATTR         "attr"
#define CONFIG_KEY_PORT         "port"
#define CONFIG_KEY_PIN          "pin"
#define CONFIG_KEY_AF           "af"

//=============================================================================
//                  Macro Definition
//=============================================================================
#define log_jobj(pJObj)                             \
            do {                                    \
                char        *pJStr = 0;             \
                pJStr = cJSON_Print((pJObj));       \
                printf("\n---\n%s\n---\n", pJStr);  \
                free(pJStr);                        \
            } while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static char     *g_hw_ip[] =
{
    CONFIG_KEY_UART,
    CONFIG_KEY_SPI,
    CONFIG_KEY_I2C,
    CONFIG_KEY_GPIO,
    CONFIG_KEY_TIMER,
    0
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_check_type(cJSON *pJObj)
{
    printf("    jobj type %s\n",
            cJSON_IsNumber(pJObj) ? "number" :
            cJSON_IsString(pJObj) ? "string" :
            cJSON_IsArray(pJObj)  ? "array" :
            cJSON_IsObject(pJObj) ? "object" : "unknown");
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    FILE    *fjson = 0;
    char    *pBuf = 0;

    do {
        uint32_t    filesize = 0;
        cJSON       *pJRoot = 0;

        if( !(fjson = fopen(argv[1], "rb")) )
        {
            printf("open %s fail \n", argv[0]);
            break;
        }

        fseek(fjson, 0, SEEK_END);
        filesize = ftell(fjson);
        fseek(fjson, 0, SEEK_SET);

        if( !(pBuf = malloc(filesize)) )
        {
            printf("malloc %d fail \n", filesize);
            break;
        }

        fread(pBuf, 1, filesize, fjson);
        fclose(fjson);
        fjson = 0;

        pJRoot = cJSON_Parse(pBuf);

        for(int i = 0; ; i++)
        {
            cJSON       *pJHW = 0;

            if( g_hw_ip[i] == 0 )
                break;

            printf("===== %s =====\n", g_hw_ip[i]);

            pJHW = cJSON_GetObjectItem(pJRoot, g_hw_ip[i]);
            if( !pJHW )     continue;

            if( cJSON_IsArray(pJHW) )
            {
                int         attr_num = 0;

                attr_num = cJSON_GetArraySize(pJHW);

                for(int j = 0; j < attr_num; j++)
                {
                    cJSON       *pJArr = 0;
                    cJSON       *pJAttr_arr = 0;
                    int         arr_num = 0;

                    pJArr = cJSON_GetArrayItem(pJHW, j);
                    if( !pJArr )   continue;

                    printf("  --- %s %d ---\n", g_hw_ip[i], j);

                    pJAttr_arr = cJSON_GetObjectItem(pJArr, CONFIG_KEY_ATTR);

                    arr_num = cJSON_GetArraySize(pJAttr_arr);

                    for(int k = 0; k < arr_num; k++)
                    {
                        cJSON   *pJAttr = 0;
                        cJSON   *pJPort = 0;
                        cJSON   *pJPin = 0;
                        cJSON   *pJAF = 0;

                        pJAttr = cJSON_GetArrayItem(pJAttr_arr, k);
                        if( !pJAttr )   continue;

                        pJPort = cJSON_GetObjectItem(pJAttr, CONFIG_KEY_PORT);
                        pJPin  = cJSON_GetObjectItem(pJAttr, CONFIG_KEY_PIN);
                        pJAF   = cJSON_GetObjectItem(pJAttr, CONFIG_KEY_AF);
                        printf("    P%s%d (af= %d)\n",
                               cJSON_GetStringValue(pJPort),
                               (int)cJSON_GetNumberValue(pJPin),
                               (int)cJSON_GetNumberValue(pJAF));
                    }

                }
            }

        }


        cJSON_Delete(pJRoot);
    } while(0);

    if( fjson )     fclose(fjson);
    if( pBuf )      free(pBuf);

    system("pause");
    return 0;
}
