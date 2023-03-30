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
static int
_cb_file_ls(char *name, int size)
{
    printf("  %7s %6d Bytes\n", name, size);
    return 0;
}

static filesys_err_t
_cb_ll_init_t(void)
{
    return FILESYS_ERR_OK;
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
    filesys_handle_t    hFilesys;

    do {
        int     rval = 0;
        char    *path = "my_file";
        filesys_init_cfg_t  init_cfg = {0};

        init_cfg.sys_type      = FILESYS_SYS_TYPE_SPIFFS;//FILESYS_SYS_TYPE_LFS;//FILESYS_SYS_TYPE_SPIFFS;
        init_cfg.cb_ll_init    = _cb_ll_init_t;
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

            {
                filesys_stat_t      stat = {0};
                if( filesys_stat(&hFilesys, "777", &stat) )
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

        {
            uint32_t total = 0;
            uint32_t used = 0;
            filesys_capacity(&hFilesys, &total, &used);
            printf("\n total= %d KB, used= %d KB\n", (total >> 10), (used >> 10));
        }


        printf("\n");
    } while(0);

    filesys_deinit(&hFilesys);

    system("pause");
    return 0;
}
