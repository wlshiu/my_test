#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ff.h"
#include "ff_macro.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define FS_SD_VOLUME                 "0:"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, argv...)       do{ printf("%s[%d] " str, __func__, __LINE__, ##argv); while(1);}while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
FATFS                   g_sd_fatfs = {0};
FIL                     g_ff_file = {0};

#include <windows.h>
//=============================================================================
//                  Private Function Definition
//=============================================================================
static FRESULT
_fat_simulation_test(
    char        *pPath,
    uint8_t     *pBuf,
    uint32_t    buf_size)
{
    FRESULT         rst = FR_OK;
    UINT            len = 0;
    uint8_t         *pBuf_cmp = 0;
    char            full_path[64] = {0};

    snprintf(full_path, 64, "%s%s", FS_SD_VOLUME, pPath);

    FCHK(rst, f_open(&g_ff_file, full_path, FA_CREATE_NEW | FA_WRITE), while(1));

    printf("create '%s'\n", full_path);
    do{
        if( !(pBuf_cmp = malloc(buf_size)) )
        {
            err("malloc %u fail \n", buf_size);
            break;
        }

        FCHK(rst, f_write(&g_ff_file, pBuf, buf_size, &len), while(1));
        FCHK(rst, f_sync(&g_ff_file), while(1));
        FCHK(rst, f_close(&g_ff_file), while(1));

        #if 0
        FCHK(rst, f_open(&g_ff_file, full_path, FA_OPEN_EXISTING | FA_READ), while(1));
        FCHK(rst, f_read(&g_ff_file, pBuf_cmp, buf_size, &len), while(1));
        FCHK(rst, f_lseek(&g_ff_file, 0), while(1));

        if( memcmp(pBuf, pBuf_cmp, buf_size) )
        {
            err("write data '%s' fail \n", pPath);
            break;
        }
        #endif
    }while(0);

    if( pBuf_cmp )      free(pBuf_cmp);

#if 0
    FCHK(rst, f_close(&g_ff_file), while(1));
#endif // 0

    return rst;
}

static int
_init_fat(void)
{
    static uint8_t      g_wrok_buf[1024] = {0};
    FRESULT             rst = FR_OK;

    do {
        rst = f_mount(&g_sd_fatfs, _T(FS_SD_VOLUME), 1);
        if( rst != FR_OK )
        {
            if( rst != FR_NO_FILESYSTEM )
            {
                err("%s", "fat mount err !\n");
                break;
            }

            FCHK(rst, f_mkfs(_T(FS_SD_VOLUME), FM_ANY, 0, g_wrok_buf, sizeof(g_wrok_buf)), while(1));
            FCHK(rst, f_mount(&g_sd_fatfs, _T(FS_SD_VOLUME), 1), while(1));
        }

    } while(0);

    return (rst) ? -1 : 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    if( _init_fat() )
    {
        err("%s", "init fat fail !\n");
        return 1;
    }

    {
        FRESULT     rst = FR_OK;
        UINT        offset = 0;
        FATFS       *pFatfs = &g_sd_fatfs;
        DWORD       free_clust = 0;

        FCHK(rst, f_getfree(FS_SD_VOLUME, &free_clust, &pFatfs), while(1));
        printf("total: %d KBytes, free: %d KBytes\n",
               ((pFatfs->n_fatent - 2) * pFatfs->csize * FF_MAX_SS) >> 10,
               (free_clust * pFatfs->csize * FF_MAX_SS) >> 10);
    }

    {
        FRESULT         rst = FR_OK;
        uint32_t        pattern_len = 0;
        uint8_t         pattern[1 << 10] = {0};
        char            full_path[64] = {0};
        UINT            len = 0;
        uint32_t        cur_pos = 0;
        uint32_t        tmp_value = 0;

        snprintf(full_path, 64, "%s%s", FS_SD_VOLUME, "test.avi");

        FCHK(rst, f_open(&g_ff_file, full_path, FA_CREATE_NEW | FA_READ | FA_WRITE), while(1));

        pattern_len = sizeof(pattern);
        memset(pattern, 0xAA, pattern_len);
        FCHK(rst, f_write(&g_ff_file, pattern, pattern_len, &len), while(1));
        cur_pos = f_tell(&g_ff_file);

        FCHK(rst, f_lseek(&g_ff_file, 4), while(1));

        FCHK(rst, f_read(&g_ff_file, pattern, 32, &len), while(1));
        pattern[3] = 0x55;
        pattern[4] = 0x55;
        pattern[5] = 0x55;

        FCHK(rst, f_lseek(&g_ff_file, 4), while(1));
        FCHK(rst, f_write(&g_ff_file, pattern, 32, &len), while(1));

        FCHK(rst, f_lseek(&g_ff_file, cur_pos), while(1));
        memset(pattern, 0xcc, pattern_len);
        FCHK(rst, f_write(&g_ff_file, pattern, pattern_len, &len), while(1));

        FCHK(rst, f_close(&g_ff_file), while(1));

        FCHK(rst, f_open(&g_ff_file, full_path, FA_OPEN_EXISTING | FA_READ), while(1));
        FCHK(rst, f_read(&g_ff_file, pattern, pattern_len, &len), while(1));
        FCHK(rst, f_read(&g_ff_file, pattern, pattern_len, &len), while(1));
        FCHK(rst, f_close(&g_ff_file), while(1));
    }

    {
        FRESULT     rst = FR_OK;
        UINT        offset = 0;
        FATFS       *pFatfs = &g_sd_fatfs;
        DWORD       free_clust = 0;

        FCHK(rst, f_getfree(FS_SD_VOLUME, &free_clust, &pFatfs), while(1));
        printf("total: %d KBytes, free: %d KBytes\n",
               ((pFatfs->n_fatent - 2) * pFatfs->csize * FF_MAX_SS) >> 10,
               (free_clust * pFatfs->csize * FF_MAX_SS) >> 10);
    }

    return 0;
}
