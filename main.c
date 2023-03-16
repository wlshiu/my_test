/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/03/14
 * @license
 * @description
 */

#include <stdio.h>
#include <stdlib.h>

#include "filesys.h"

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

#if 0
static lfs_t            g_lfs_flash = {0};
static lfs_file_t       g_lfs_file = {0};
static lfs_config_t     g_lfs_cfg = {0};
static lfs_file_config_t    g_lfs_file_cfg = {0};
static void test_lfs(void)
{
    int     rval = 0;

    lfs_fs_get_dev_cfg(&g_lfs_cfg);

    g_lfs_cfg.read_buffer       = (void*)g_read_buf;
    g_lfs_cfg.prog_buffer       = (void*)g_prog_buf;
    g_lfs_cfg.lookahead_buffer  = (void*)g_lookahead_buf;
    g_lfs_cfg.read_size         = sizeof(g_read_buf);
    g_lfs_cfg.prog_size         = sizeof(g_prog_buf);
    g_lfs_cfg.block_size        = 4096;
    g_lfs_cfg.block_count       = 64;
    g_lfs_cfg.cache_size        = sizeof(g_file_cache);
    g_lfs_cfg.lookahead_size    = sizeof(g_lookahead_buf);
    g_lfs_cfg.block_cycles      = 500;

    rval = lfs_fs_init(&g_lfs_flash, &g_lfs_cfg);
    if( rval )  return rval;

    // read current count
    uint32_t boot_count = 0xAA;

#if defined(LFS_NO_MALLOC)
    g_lfs_file_cfg.buffer       = (void*)g_file_cache;
    g_lfs_file_cfg.attr_count   = 0;

    lfs_file_opencfg(&g_lfs_flash, &g_lfs_file, "boot_count", LFS_O_RDWR | LFS_O_CREAT, &g_lfs_file_cfg);
#else
    lfs_file_open(&g_lfs_flash, &g_lfs_file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
#endif

    lfs_file_read(&g_lfs_flash, &g_lfs_file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&g_lfs_flash, &g_lfs_file);
    lfs_file_write(&g_lfs_flash, &g_lfs_file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&g_lfs_flash, &g_lfs_file);

    // release any resources we were using
    lfs_unmount(&g_lfs_flash);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
    return;
}
#endif // 0

#if 0
static spiffs               g_hSpiffs = {0};
static void test_spiffs(void)
{
    spiffs_init(&g_hSpiffs, 0);

    for(int i = 0; i < 2; i++)
    {
        int     rval = 0;
        char    buf[20] = {0};

        // create a file, delete previous if it already exists, and open it for reading and writing
//        SPIFFS_TRUNC
        spiffs_file     fd = SPIFFS_open(&g_hSpiffs, "my_file", SPIFFS_O_CREAT | SPIFFS_O_WRONLY | SPIFFS_O_APPEND, 0);

        if( fd < 0 )
        {
            printf("errno %i\n", SPIFFS_errno(&g_hSpiffs));
            spiffs_err_log(SPIFFS_errno(&g_hSpiffs));
            break;
        }

        // write to it
        if( SPIFFS_write(&g_hSpiffs, fd, (u8_t*)"Hello world", 12) < 0 )
        {
            printf("errno %i\n", SPIFFS_errno(&g_hSpiffs));
            spiffs_err_log(SPIFFS_errno(&g_hSpiffs));
            break;
        }

        SPIFFS_fflush(&g_hSpiffs, fd);

        if( SPIFFS_write(&g_hSpiffs, fd, (u8_t*)"Hello world", 12) < 0 )
        {
            printf("errno %i\n", SPIFFS_errno(&g_hSpiffs));
            spiffs_err_log(SPIFFS_errno(&g_hSpiffs));
            break;
        }
        printf("\n\n");
        // close it
        if( SPIFFS_close(&g_hSpiffs, fd) < 0 )
        {
            printf("errno %i\n", SPIFFS_errno(&g_hSpiffs));
            spiffs_err_log(SPIFFS_errno(&g_hSpiffs));
            break;
        }

        // open it
        fd = SPIFFS_open(&g_hSpiffs, "my_file", SPIFFS_CREAT | SPIFFS_RDWR, 0);
        if( fd < 0 )
        {
            printf("errno %i\n", SPIFFS_errno(&g_hSpiffs));
            spiffs_err_log(SPIFFS_errno(&g_hSpiffs));
            break;
        }

        // read it
        if( SPIFFS_read(&g_hSpiffs, fd, (u8_t *)buf, sizeof(buf)) < 0 )
        {
            printf("errno %i\n", SPIFFS_errno(&g_hSpiffs));
            spiffs_err_log(SPIFFS_errno(&g_hSpiffs));
            break;
        }

        // close it
        if( SPIFFS_close(&g_hSpiffs, fd) < 0 )
        {
            printf("errno %i\n", SPIFFS_errno(&g_hSpiffs));
            spiffs_err_log(SPIFFS_errno(&g_hSpiffs));
            break;
        }

        // check it
        printf("-->%s<--\n", buf);
    }
    return;
}
#endif
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
#if 0
    test_spiffs();
#else
    filesys_handle_t    hFilesys;

    do {
        int     rval = 0;

        filesys_init(&hFilesys, 0);

//        filesys_format(&hFilesys);

        for(int i = 0; i < 2; i++)
        {
            HAFILE      hAFile;
            char        buf[128] = {0};

            hAFile = filesys_open(&hFilesys, "my_file", FILESYS_MODE_RDWR | FILESYS_MODE_APPEND);
            if( !hAFile )
            {
                printf("open fail \n");
                continue;
            }

            snprintf(buf, sizeof(buf), "test_%d", i);
            rval = filesys_write(buf, 1, strlen(buf), hAFile);
            if( rval < 0 )
            {
                printf("write fail \n");
                filesys_close(hAFile);
                continue;
            }

            if( filesys_close(hAFile) < 0 )
                printf("close fail \n");

            hAFile = filesys_open(&hFilesys, "my_file", FILESYS_MODE_RDWR);
            if( !hAFile )
            {
                printf("open fail \n");
                continue;
            }

            memset(buf, 0x0, sizeof(buf));
            rval = filesys_read(buf, 1, 20, hAFile);
            if( rval < 0 )
            {
                printf("read fail \n");
            }

            printf("--'%s'--\n", buf);

            if( filesys_close(hAFile) < 0 )
                printf("close fail \n");
        }
    } while(0);

    filesys_deinit(&hFilesys);
#endif // 1

    system("pause");
    return 0;
}
