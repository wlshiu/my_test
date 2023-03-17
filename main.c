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

#include "extfc.h"
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
#include "lfs_port.h"
static lfs_t                g_lfs_flash = {0};
static lfs_file_t           g_lfs_file = {0};
static lfs_config_t         g_lfs_cfg = {0};
static lfs_file_config_t    g_lfs_file_cfg = {0};

#define CONFIG_FS_CACHE_SZ              32
static uint32_t    g_afile_cache[32 >> 2];

static uint32_t    g_read_buf[CONFIG_FS_CACHE_SZ >> 2];
static uint32_t    g_prog_buf[CONFIG_FS_CACHE_SZ >> 2];
static uint32_t    g_lookahead_buf[CONFIG_FS_CACHE_SZ >> 2];

static int test_lfs(void)
{
    int     rval = 0;
    int     len = 0;
    uint8_t buffer[30];

    lfs_fs_get_dev_cfg(&g_lfs_cfg);

    g_lfs_cfg.read_buffer       = (void*)g_read_buf;
    g_lfs_cfg.prog_buffer       = (void*)g_prog_buf;
    g_lfs_cfg.lookahead_buffer  = (void*)g_lookahead_buf;
    g_lfs_cfg.read_size         = sizeof(g_read_buf);
    g_lfs_cfg.prog_size         = sizeof(g_prog_buf);
    g_lfs_cfg.block_size        = 4096;
    g_lfs_cfg.block_count       = 64;
    g_lfs_cfg.cache_size        = sizeof(g_afile_cache);
    g_lfs_cfg.lookahead_size    = sizeof(g_lookahead_buf);
    g_lfs_cfg.block_cycles      = 500;

    rval = lfs_fs_init(&g_lfs_flash, &g_lfs_cfg);
    if( rval )  return rval;

    // read current count
    uint32_t boot_count = 0xAA;

#if defined(LFS_NO_MALLOC)
    g_lfs_file_cfg.buffer       = (void*)g_afile_cache;
    g_lfs_file_cfg.attr_count   = 0;

    lfs_file_opencfg(&g_lfs_flash, &g_lfs_file, "my_file", LFS_O_RDWR | LFS_O_CREAT, &g_lfs_file_cfg);
#else
    lfs_file_open(&g_lfs_flash, &g_lfs_file, "my_file", LFS_O_RDWR | LFS_O_CREAT);
#endif

    lfs_file_read(&g_lfs_flash, &g_lfs_file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&g_lfs_flash, &g_lfs_file);

    #if 1
    {

        for (uint32_t i = 0; i < 9; i++)
        {
            snprintf((char*)&buffer, sizeof(buffer), "butr_%d", i);
            len = strlen((char*)&buffer);
            rval = lfs_file_write(&g_lfs_flash, &g_lfs_file, buffer, len);
            if (rval < 0)
            {
                printf("fail\n");
            }
        }
    }
    #else
    lfs_file_write(&g_lfs_flash, &g_lfs_file, &boot_count, sizeof(boot_count));
    #endif

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&g_lfs_flash, &g_lfs_file);

#if defined(LFS_NO_MALLOC)
    g_lfs_file_cfg.buffer       = (void*)g_afile_cache;
    g_lfs_file_cfg.attr_count   = 0;

    lfs_file_opencfg(&g_lfs_flash, &g_lfs_file, "my_file", LFS_O_RDWR | LFS_O_CREAT, &g_lfs_file_cfg);
#else
    lfs_file_open(&g_lfs_flash, &g_lfs_file, "my_file", LFS_O_RDWR | LFS_O_CREAT);
#endif

    {
        for (uint32_t i = 0; i < 10; i++)
        {
            memset(buffer, 0x0, sizeof(buffer));
            rval = lfs_file_read(&g_lfs_flash, &g_lfs_file, buffer, len);
            if (rval < 0)
            {
                printf("fail\n");
            }
            printf("'%s'\n", (char*)&buffer);
        }
    }
    lfs_file_close(&g_lfs_flash, &g_lfs_file);

    // release any resources we were using
    lfs_unmount(&g_lfs_flash);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
    return 0;
}
#endif // 0


static int
_cb_file_ls(char *name, int size)
{
    msg("  %7s %6d Bytes\n", name, size);
    return 0;
}

static filesys_err_t
_cb_flash_sector_erase(uint32_t flash_addr, int sector_cnt)
{
    extfc_erase(EXTFC_ERASE_SECTOR, flash_addr, sector_cnt);
    return FILESYS_ERR_OK;
}

static filesys_err_t _cb_flash_read(uint8_t *pSys_buf, uint32_t flash_addr, int nbytes)
{
    extfc_read(pSys_buf, flash_addr, nbytes);
    return FILESYS_ERR_OK;
}

static filesys_err_t _cb_flash_prog(uint8_t *pSys_buf, uint32_t flash_addr, int nbytes)
{
    extfc_program(pSys_buf, flash_addr, nbytes);
    return FILESYS_ERR_OK;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
#if 0
    test_lfs();
#else
    filesys_handle_t    hFilesys;

    do {
        int     rval = 0;
        char    *path = "my_file";
        filesys_init_cfg_t  init_cfg = {0};

        init_cfg.sys_type      = FILESYS_SYS_TYPE_LFS;//FILESYS_SYS_TYPE_SPIFFS;
        init_cfg.cb_flash_prog = _cb_flash_prog;
        init_cfg.cb_flash_read = _cb_flash_read;
        init_cfg.cb_sec_erase  = _cb_flash_sector_erase;

        filesys_init(&hFilesys, &init_cfg);

        hFilesys.cb_file_ls = _cb_file_ls;

//        filesys_format(&hFilesys);

        for(int i = 0; i < 2; i++)
        {
            HAFILE      hAFile;
            char        buf[128] = {0};

            hAFile = filesys_open(&hFilesys, path, FILESYS_MODE_RDWR | FILESYS_MODE_APPEND);
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

            {
                filesys_stat_t      stat = {0};
                if( filesys_stat(&hFilesys, path, &stat) )
                {
                    printf("stat fail \n");
                    continue;
                }

                printf("  %s, size= %d\n", stat.name, stat.size);
            }

            hAFile = filesys_open(&hFilesys, path, FILESYS_MODE_RDWR);
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

        printf("\n  NAME        SIZE\n");
        filesys_ls(&hFilesys, "/");

        printf("\n");
    } while(0);

    filesys_deinit(&hFilesys);
#endif // 1

    system("pause");
    return 0;
}
