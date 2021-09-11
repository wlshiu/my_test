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
#include <string.h>
#include "cJSON.h"
#include "ut_common.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_SUPPORT_MAX_NUM      200


#define CONFIG_KEY_UART         "UART"
#define CONFIG_KEY_SPI          "SPI"
#define CONFIG_KEY_I2C          "I2C"
#define CONFIG_KEY_GPIO         "GPIO"
#define CONFIG_KEY_TIMER        "TIMER"


#define CONFIG_KEY_ATTR         "attr"
#define CONFIG_KEY_TYPE         "type"
#define CONFIG_KEY_PORT         "port"
#define CONFIG_KEY_PIN          "pin"
#define CONFIG_KEY_AF           "af"


#define CONFIG_TYPE_TX          "TX"
#define CONFIG_TYPE_RX          "RX"
#define CONFIG_TYPE_IO          "IO"
#define CONFIG_TYPE_PWM         "PWM"
#define CONFIG_TYPE_CAP         "CAP"
#define CONFIG_TYPE_CLK         "CLK"
#define CONFIG_TYPE_DAT         "DAT"
#define CONFIG_TYPE_CS          "CS"
#define CONFIG_TYPE_MOSI        "MOSI"
#define CONFIG_TYPE_MISO        "MISO"


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

static char     *g_out_bin_path = 0;
static char     *g_out_header_path = 0;
static char     *g_json_patn = 0;

static pin_attr_t       g_pin_attr[CONFIG_SUPPORT_MAX_NUM] = {0};
static int              g_pin_attr_cnt = 0;
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

static void
_usage(char **argv)
{
    printf("usage: %s [--ob xxx.bin] [--oh xxx.h] xxx.json\n", argv[0]);
    system("pause");
    exit(-1);
}

static void
_get_params(int argc, char **argv)
{
    if (argc < 2) {
        _usage(argv);
        exit(-1);
    }

    argv++; argc--;
    while(argc) {
        if (!strncmp(argv[0], "--ob", strlen("--ob"))) {
            argv++; argc--;
            g_out_bin_path = argv[0];
        } else if (!strncmp(argv[0], "--oh", strlen("--oh"))) {
            argv++; argc--;
            g_out_header_path = argv[0];
        } else {
            g_json_patn = argv[0];
        }
        argv++; argc--;
    }

    if( !g_json_patn )
        _usage(argv);
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    FILE    *fjson = 0;
    FILE    *fhdr = 0;
    FILE    *fbin = 0;
    char    *pBuf = 0;

    g_out_bin_path    = 0;
    g_out_header_path = 0;
    g_json_patn       = 0;

    g_pin_attr_cnt = 0;
    memset(g_pin_attr, 0x0, sizeof(g_pin_attr));

    do {
        uint32_t    filesize = 0;
        cJSON       *pJRoot = 0;

        _get_params(argc, argv);

        if( !(fjson = fopen(g_json_patn, "rb")) )
        {
            printf("open %s fail \n", g_json_patn);
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
                        cJSON   *pJType = 0;
                        cJSON   *pJPin = 0;
                        cJSON   *pJAF = 0;

                        pJAttr = cJSON_GetArrayItem(pJAttr_arr, k);
                        if( !pJAttr )   continue;

                        pJType = cJSON_GetObjectItem(pJAttr, CONFIG_KEY_TYPE);
                        pJPort = cJSON_GetObjectItem(pJAttr, CONFIG_KEY_PORT);
                        pJPin  = cJSON_GetObjectItem(pJAttr, CONFIG_KEY_PIN);
                        pJAF   = cJSON_GetObjectItem(pJAttr, CONFIG_KEY_AF);
                        printf("    P%s%d (type '%s', af= %d)\n",
                               cJSON_GetStringValue(pJPort),
                               (int)cJSON_GetNumberValue(pJPin),
                               cJSON_GetStringValue(pJType),
                               (int)cJSON_GetNumberValue(pJAF));

                        if( g_pin_attr_cnt < CONFIG_SUPPORT_MAX_NUM )
                        {
                            char    *pPort = cJSON_GetStringValue(pJPort);
                            char    *pType = cJSON_GetStringValue(pJType);

                            if( pPort[0] == '0' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x0;
                            else if( pPort[0] == '1' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x1;
                            else if( pPort[0] == '2' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x2;
                            else if( pPort[0] == '3' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x3;
                            else if( pPort[0] == '4' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x4;
                            else if( pPort[0] == '5' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x5;
                            else if( pPort[0] == '6' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x6;
                            else if( pPort[0] == '7' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x7;
                            else if( pPort[0] == '8' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x8;
                            else if( pPort[0] == '9' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0x9;
                            else if( pPort[0] == 'A' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0xA;
                            else if( pPort[0] == 'B' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0xB;
                            else if( pPort[0] == 'C' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0xC;
                            else if( pPort[0] == 'D' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0xD;
                            else if( pPort[0] == 'E' )
                                g_pin_attr[g_pin_attr_cnt].attr.port = 0xE;


                            if( !strncmp(g_hw_ip[i], CONFIG_KEY_UART, strlen(CONFIG_KEY_UART)) )
                            {
                                if( !strncmp(pType, CONFIG_TYPE_TX, strlen(CONFIG_TYPE_TX)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = UART_PIN_TX;
                                else if( !strncmp(pType, CONFIG_TYPE_RX, strlen(CONFIG_TYPE_RX)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = UART_PIN_RX;
                            }
                            else if( !strncmp(g_hw_ip[i], CONFIG_KEY_I2C, strlen(CONFIG_KEY_I2C)) )
                            {
                                if( !strncmp(pType, CONFIG_TYPE_CLK, strlen(CONFIG_TYPE_CLK)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = I2C_PIN_SCL;
                                else if( !strncmp(pType, CONFIG_TYPE_DAT, strlen(CONFIG_TYPE_DAT)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = I2C_PIN_SDA;
                            }
                            else if( !strncmp(g_hw_ip[i], CONFIG_KEY_SPI, strlen(CONFIG_KEY_SPI)) )
                            {
                                if( !strncmp(pType, CONFIG_TYPE_CLK, strlen(CONFIG_TYPE_CLK)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = SPI_PIN_CLK;
                                else if( !strncmp(pType, CONFIG_TYPE_CS, strlen(CONFIG_TYPE_CS)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = SPI_PIN_CS;
                                else if( !strncmp(pType, CONFIG_TYPE_MOSI, strlen(CONFIG_TYPE_MOSI)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = SPI_PIN_MOSI;
                                else if( !strncmp(pType, CONFIG_TYPE_MISO, strlen(CONFIG_TYPE_MISO)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = SPI_PIN_MISO;
                            }
                            else if( !strncmp(g_hw_ip[i], CONFIG_KEY_GPIO, strlen(CONFIG_KEY_GPIO)) )
                            {
                                g_pin_attr[g_pin_attr_cnt].pin_type = GPIO_IO_PIN;
                            }
                            else if( !strncmp(g_hw_ip[i], CONFIG_KEY_TIMER, strlen(CONFIG_KEY_TIMER)) )
                            {
                                if( !strncmp(pType, CONFIG_TYPE_PWM, strlen(CONFIG_TYPE_PWM)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = TIM_PIN_PWM;
                                else if( !strncmp(pType, CONFIG_TYPE_CAP, strlen(CONFIG_TYPE_CAP)) )
                                    g_pin_attr[g_pin_attr_cnt].pin_type = TIM_PIN_CAPTURE;
                            }

                            g_pin_attr[g_pin_attr_cnt].index        = j;
                            g_pin_attr[g_pin_attr_cnt].attr.pin     = (int)cJSON_GetNumberValue(pJPin);
                            g_pin_attr[g_pin_attr_cnt].attr.af_mode = (int)cJSON_GetNumberValue(pJAF);

                            g_pin_attr_cnt++;
                        }
                    }

                }
            }

        }

        cJSON_Delete(pJRoot);

        do {
            if( g_out_bin_path )
            {
                uint8_t     *pCur = (uint8_t*)g_pin_attr;

                if( !(fbin = fopen(g_out_bin_path, "wb")) )
                {
                    printf("open %s fail\n", g_out_bin_path);
                    break;
                }

            #if 1
                fwrite(pCur, 1, sizeof(pin_attr_t) * g_pin_attr_cnt, fbin);
            #else
                fprintf(fbin, "uint8_t const   g_pin_table_bin[] =\n{\n    ");

                for(int i = 0; i < sizeof(pin_attr_t) * g_pin_attr_cnt; i++)
                {
                    if( i && !(i & 0xF) )
                        fprintf(fbin, "\n    ");

                    fprintf(fbin, "0x%02x, ", *pCur++);
                }

                fprintf(fbin, "\n};\n\n");
            #endif
            }
        } while(0);

        do {
            if( g_out_header_path )
            {
                if( !(fhdr = fopen(g_out_header_path, "wb")) )
                {
                    printf("open %s fail\n", g_out_header_path);
                    break;
                }

                fprintf(fhdr, "\n\n\ntypedef struct ut_pin_attr\n"
                              "{\n"
                              "    uint32_t   pin_type;\n"
                              "    uint32_t   index;\n"
                              "    union {\n"
                              "        uint32_t    value;\n"
                              "        struct {\n"
                              "            uint32_t    port    : 8;\n"
                              "            uint32_t    pin     : 8;\n"
                              "            uint32_t    af_mode : 8;\n"
                              "        } attr;\n"
                              "    };\n"
                              "} ut_pin_attr_t;\n\n");

                fprintf(fhdr, "\nconst ut_pin_attr_t      g_pin_table[] =\n{\n");

                for(int i = 0; i < g_pin_attr_cnt; i++)
                {
                    char    *pType = 0;

                    if( g_pin_attr[i].pin_type == UART_PIN_TX )             pType = "UART_PIN_TX";
                    else if( g_pin_attr[i].pin_type == UART_PIN_RX )        pType = "UART_PIN_RX";
                    else if( g_pin_attr[i].pin_type == I2C_PIN_SCL )        pType = "I2C_PIN_SCL";
                    else if( g_pin_attr[i].pin_type == I2C_PIN_SDA )        pType = "I2C_PIN_SDA";
                    else if( g_pin_attr[i].pin_type == SPI_PIN_CS )         pType = "SPI_PIN_CS";
                    else if( g_pin_attr[i].pin_type == SPI_PIN_CLK )        pType = "SPI_PIN_CLK";
                    else if( g_pin_attr[i].pin_type == SPI_PIN_MISO )       pType = "SPI_PIN_MISO";
                    else if( g_pin_attr[i].pin_type == SPI_PIN_MOSI )       pType = "SPI_PIN_MOSI";
                    else if( g_pin_attr[i].pin_type == TIM_PIN_PWM )        pType = "TIM_PIN_PWM";
                    else if( g_pin_attr[i].pin_type == TIM_PIN_CAPTURE )    pType = "TIM_PIN_CAPTURE";
                    else if( g_pin_attr[i].pin_type == GPIO_IO_PIN )        pType = "GPIO_IO_PIN";


                    fprintf(fhdr, "    { .index = %d, .pin_type = %+17s,  .attr = { .port = 0x%X, .pin = %d, .af_mode = 0x%X }, },\n",
                            g_pin_attr[i].index,
                            pType,
                            g_pin_attr[i].attr.port,
                            g_pin_attr[i].attr.pin,
                            g_pin_attr[i].attr.af_mode);
                }

                fprintf(fhdr, "};\n");

            }
        } while(0);
    } while(0);


    if( fjson )     fclose(fjson);
    if( fhdr )      fclose(fhdr);
    if( fbin )      fclose(fbin);
    if( pBuf )      free(pBuf);

    system("pause");
    return 0;
}



