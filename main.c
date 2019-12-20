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

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    FILE        *fin = 0;
    uint8_t     *pBuf = 0;
    char        *pFilename = "example.raw";

    do {
        uint32_t    filesize = 0;

        if( !(fin = fopen(pFilename, "rb")) )
        {
            printf("open %s fail !\n", pFilename);
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

    system("pause");
    return 0;
}
