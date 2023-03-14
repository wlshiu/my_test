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

#include "lfs_port.h"
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
static lfs_t            g_lfs_flash = {0};
static lfs_file_t       g_lfs_file = {0};
static lfs_config_t     g_lfs_cfg = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

static void test_lfs(void)
{
    int     rval = 0;

    lfs_fs_get_cfg(&g_lfs_cfg);
    rval = lfs_fs_init(&g_lfs_flash, &g_lfs_cfg);
    if( rval )  return rval;

    // read current count
    uint32_t boot_count = 0;

#if defined(LFS_NO_MALLOC)
//    lfs_file_rawopencfg();
//    lfs_file_rawclose();
//    lfs_file_close()
#else
    lfs_file_open(&g_lfs_flash, &g_lfs_file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&g_lfs_flash, &g_lfs_file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&g_lfs_flash, &g_lfs_file);
    lfs_file_write(&g_lfs_flash, &g_lfs_file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&g_lfs_flash, &g_lfs_file);

#endif

    // release any resources we were using
    lfs_unmount(&g_lfs_flash);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
    return;
}


//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    test_lfs();

    system("pause");
    return 0;
}
