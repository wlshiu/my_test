/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/12/20
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>

#include "auto_script.h"
#include "scr2bin.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_LINE_BUF_SIZE            200

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static char     g_line_buf[CONFIG_LINE_BUF_SIZE] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_do_scr2bin(char *pScr_name, char *pBin_name)
{
    int     rval = 0;
    FILE    *fin = 0;
    FILE    *fout = 0;

    do {
        uint32_t    buf[4] = {0};

        if( !(fin = fopen(pScr_name, "rb")) )
        {
            printf("open %s fail !\n", pScr_name);
            rval = -1;
            break;
        }

        if( !(fout = fopen(pBin_name, "wb")) )
        {
            printf("open %s fail !\n", pBin_name);
            rval = -1;
            break;
        }

        while( fgets(g_line_buf, sizeof(g_line_buf), fin)  )
        {
            int     buf_len = sizeof(buf);

            rval = scr_to_bin((char*)g_line_buf, &buf, &buf_len);
            if( rval )  break;

            if( buf_len )
                fwrite(buf, 1, buf_len, fout);

        }

    } while(0);

    if( fin )   fclose(fin);
    if( fout )  fclose(fout);

    return rval;
}

static int
_do_auto_script(char *pBin_name)
{
    int         rval = 0;
    FILE        *fin = 0;
    uint8_t     *pBuf = 0;

    do {
        uint32_t    filesize = 0;

        if( !(fin = fopen(pBin_name, "rb")) )
        {
            printf("open %s fail !\n", pBin_name);
            break;
        }

        fseek(fin, 0ul, SEEK_END);
        filesize = ftell(fin);
        fseek(fin, 0ul, SEEK_SET);

        if( !(pBuf = malloc(filesize)) )
        {
            printf("malloc %d fail !\n", filesize);
            break;
        }

        fread(pBuf, 1, filesize, fin);
        fclose(fin);
        fin = 0;

        ascript_exec(pBuf, filesize);

    } while(0);

    if( fin )   fclose(fin);

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(void)
{
    int     rval = 0;
    char    *pScr_name = "example.scr";
    char    *pBin_name = "example.bin";

    do {
        rval = _do_scr2bin(pScr_name, pBin_name);
        if( rval )
        {
            printf("convert script to bin fail !\n");
            break;
        }

        rval = _do_auto_script(pBin_name);
        if( rval )
        {
            printf("auto script fail !\n");
            break;
        }
    } while(0);

    system("pause");
    return 0;
}
