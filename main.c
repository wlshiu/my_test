/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/09/17
 * @license
 * @description
 */


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pff.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, ...)       do{ printf("[err:%s:%d] " str, __func__, __LINE__, ##__VA_ARGS__);  \
                                while(1);                                                       \
                            }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
FATFS    g_fatfs;			/* File system object */
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    int         rval = 0;
    uint32_t    rbytes = 0;
    uint32_t    wbytes = 0;
    uint8_t     g_buf_wr[32] = {0};
    uint8_t     g_buf_rd[32] = {0};

    for(int i = 0; i < sizeof(g_buf_wr); i++)
    {
        g_buf_wr[i] = 0x1 + i;
    }

    memset(g_buf_rd, 0x0, sizeof(g_buf_rd));

    pf_mount(&g_fatfs);
    printf("FAT type = %u\n"
            "Bytes/Cluster = %lu\n"
            "Root DIR entries = %u\n"
            "Number of clusters = %lu\n"
            "FAT start (lba) = %lu\n"
            "DIR start (lba,clustor) = %lu\n"
            "Data start (lba) = %lu\n\n",
            g_fatfs.fs_type, (uint32_t)g_fatfs.csize * 512,
            g_fatfs.n_rootdir, (uint32_t)g_fatfs.n_fatent - 2,
            g_fatfs.fatbase, g_fatfs.dirbase, g_fatfs.database);


#if (PF_USE_DIR)
    DIR     dir;        /* Directory object */
    FILINFO fno;        /* File information object */

    printf("\nOpen root directory.\n");
    rval = (int)pf_opendir(&dir, "");
    if( rval )  err("opendir fail (err_code: %d)!\n", rval);

    printf("\nDirectory listing...\n");
    for(;;)
    {
        /* Read a directory item */
        rval = (int)pf_readdir(&dir, &fno);
        if( rval || !fno.fname[0] )
            break; /* Error or end of dir */

        if( fno.fattrib & AM_DIR )
            printf("   <dir>  %s\n", fno.fname);
        else
            printf("%8lu  %s\n", fno.fsize, fno.fname);
    }

    if( rval )  err("readdir fail (err_code: %d)!\n", rval);

#endif // 1

    printf("\n--- Open file ---\n");

    rval = (int)pf_open("_SCRIPT");

    printf("--- Read File ---\n");
    rval = (int)pf_read(&g_buf_rd, sizeof(g_buf_rd), &rbytes);
    for(int i = 0; i < sizeof(g_buf_rd); i++)
    {
        if( i && !(i & 0xF) )
            printf("\n");
        printf("%02X ", g_buf_rd[i]);
    }

    pf_lseek(0);

    printf("\n\n--- Modify File ---\n");
    rval = (int)pf_write(&g_buf_wr, sizeof(g_buf_wr), &wbytes);
    rval = (int)pf_write(0, 0, &wbytes); // write_end

    memset(g_buf_rd, 0xAA, sizeof(g_buf_rd));
    pf_lseek(0);

    printf("--- Re-Read File ---\n");
    rval = (int)pf_read(&g_buf_rd, sizeof(g_buf_rd), &rbytes);
    for(int i = 0; i < sizeof(g_buf_rd); i++)
    {
        if( i && !(i & 0xF) )
            printf("\n");
        printf("%02X ", g_buf_rd[i]);
    }
    printf("\n");


    return 0;
}

