/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file filesys_littlefs.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/03/16
 * @license
 * @description
 */


#include "filesys.h"
#include "lfs.h"

#include "extfc.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_FS_CACHE_SZ              32

#define CONFIG_AFILE_CACHE_SZ           32

#define CONFIG_EXT_FLASH_PHYS_SZ        (512ul << 10)//(512ul << 10)
#define CONFIG_EXT_FLASH_BLOCK_SZ       (32ul << 10)//(64ul << 10)
#define CONFIG_EXT_FLASH_PHYS_ADDR      (0)
#define CONFIG_EXT_FLASH_LOG_PAGE_SZ    (256)

#define EXT_FLASH_SECTOR_SZ             (4ul << 10)
#define EXT_FLASH_PAGE_SZ               256

#define CONFIG_DUMMY_BYTE               0xFF

#define SPIT_FLAG_TIMEOUT           ((uint32_t)0x1000)

#define CMD_PageProgram		        0x02
#define CMD_ReadData                0x03
#define CMD_SectorErase		        0x20

#define CMD_READ_STATU_REG_1        0x05
#define CMD_READ_STATU_REG_2        0x35
#define CMD_WriteEnable		        0x06


#define EXT_FLASH_WIP_FLAG          0x01  /* Write In Progress (WIP) flag */


//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct hfile_lfs
{
    lfs_t               *pHLfs;
    lfs_file_config_t   fd_cfg;
    lfs_file_t          fd;
    uint32_t            file_cache[CONFIG_AFILE_CACHE_SZ >> 2];

} hfile_lfs_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern filesys_ll_dev_t     g_filesys_dev;

uint32_t    g_read_buf[CONFIG_FS_CACHE_SZ >> 2];
uint32_t    g_prog_buf[CONFIG_FS_CACHE_SZ >> 2];
uint32_t    g_file_cache[CONFIG_FS_CACHE_SZ >> 2];
uint32_t    g_lookahead_buf[CONFIG_FS_CACHE_SZ >> 2];

static lfs_t        g_lfs_flash = {0};

static hfile_lfs_t      g_hfile[CONFIG_FILE_CNT] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void _lfs_err_string(lfs_error_t err_code)
{
    switch( err_code )
    {
        case LFS_ERR_OK         :    printf("No error                      \n");    break;
        case LFS_ERR_IO         :    printf("Error during device operation \n");    break;
        case LFS_ERR_CORRUPT    :    printf("Corrupted                     \n");    break;
        case LFS_ERR_NOENT      :    printf("No directory entry            \n");    break;
        case LFS_ERR_EXIST      :    printf("Entry already exists          \n");    break;
        case LFS_ERR_NOTDIR     :    printf("Entry is not a dir            \n");    break;
        case LFS_ERR_ISDIR      :    printf("Entry is a dir                \n");    break;
        case LFS_ERR_NOTEMPTY   :    printf("Dir is not empty              \n");    break;
        case LFS_ERR_BADF       :    printf("Bad file number               \n");    break;
        case LFS_ERR_FBIG       :    printf("File too large                \n");    break;
        case LFS_ERR_INVAL      :    printf("Invalid parameter             \n");    break;
        case LFS_ERR_NOSPC      :    printf("No space left on device       \n");    break;
        case LFS_ERR_NOMEM      :    printf("No more memory available      \n");    break;
        case LFS_ERR_NOATTR     :    printf("No data/attr available        \n");    break;
        case LFS_ERR_NAMETOOLONG:    printf("File name too long            \n");    break;
        default:                     printf("Unknown\n");                           break;
    }
    return;
}

static int
_lfs_deskio_read(
    const struct lfs_config *c,
    lfs_block_t             block,
    lfs_off_t               off,
    void                    *buffer,
    lfs_size_t              size)
{
//    extfc_read((uint8_t *)buffer, c->block_size * block + off, size);

    g_filesys_dev.cb_flash_read((uint8_t *)buffer, c->block_size * block + off, size);

    return LFS_ERR_OK;
}


static int
_lfs_deskio_prog(
    const struct lfs_config *c,
    lfs_block_t             block,
    lfs_off_t               off,
    const void              *buffer,
    lfs_size_t              size)
{
    uint8_t     *pBuffer = (uint8_t*)buffer;
    uint32_t    WriteAddr = c->block_size * block + off;
    uint16_t    NumByteToWrite = size;
    uint16_t    page_remain;

    page_remain = EXT_FLASH_PAGE_SZ - WriteAddr % EXT_FLASH_PAGE_SZ;
    if( NumByteToWrite <= page_remain )
        page_remain = NumByteToWrite;

    while (1)
    {
        g_filesys_dev.cb_flash_prog(pBuffer, WriteAddr, page_remain);
//        extfc_program(pBuffer, WriteAddr, page_remain);

        if( NumByteToWrite == page_remain )
            break;

        pBuffer   += page_remain;
        WriteAddr += page_remain;

        NumByteToWrite -= page_remain;

        page_remain = (NumByteToWrite > EXT_FLASH_PAGE_SZ)
                    ? EXT_FLASH_PAGE_SZ : NumByteToWrite;
    }
    return LFS_ERR_OK;
}

static int
_lfs_deskio_erase(
    const struct lfs_config *c,
    lfs_block_t             block)
{
    uint32_t    Dst_Addr = block * 4096;

//    extfc_erase(EXTFC_ERASE_SECTOR, Dst_Addr, 1);
    g_filesys_dev.cb_sec_erase(Dst_Addr, 1);
    return LFS_ERR_OK;
}

static int _lfs_deskio_sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}

static const struct lfs_config  g_lfs_cfg =
{
    // block device operations
    .read  = _lfs_deskio_read,
    .prog  = _lfs_deskio_prog,
    .erase = _lfs_deskio_erase,
    .sync  = _lfs_deskio_sync,

    // block device configuration
    .read_size         = sizeof(g_read_buf),
    .prog_size         = sizeof(g_prog_buf),
    .block_size        = 4096,
    .block_count       = 64,
    .cache_size        = CONFIG_AFILE_CACHE_SZ, //sizeof(g_file_cache),
    .lookahead_size    = sizeof(g_lookahead_buf),
    .block_cycles      = 500,

    /**
     *  static buffer for cache
     */
    .read_buffer      = (void*)&g_read_buf,
    .prog_buffer      = (void*)&g_prog_buf,
    .lookahead_buffer = (void*)&g_lookahead_buf,
};

static filesys_err_t
_lfs_init(filesys_handle_t *pHFilesys, filesys_init_cfg_t *pCfg)
{
    int     rval = 0;

    do {
        if( !pHFilesys )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        memset(pHFilesys, 0x0, sizeof(filesys_handle_t));
        memset(&g_lfs_flash, 0x0, sizeof(g_lfs_flash));

        memset(&g_hfile, 0x0, sizeof(g_hfile));
        for(int i = 0; i < CONFIG_FILE_CNT; i++)
        {
            g_hfile[i].fd.id             = -1;
            g_hfile[i].fd_cfg.buffer     = (void*)&g_hfile[i].file_cache;
            g_hfile[i].fd_cfg.attr_count = 0;
        }

        pHFilesys->pHFS = (void*)&g_lfs_flash;

        // mount the file system
        rval = lfs_mount((lfs_t*)pHFilesys->pHFS, &g_lfs_cfg);

        /**
         *  reformat if we can't mount the file system
         *  this should only happen on the first boot
         */
        if( rval )
        {
            rval = lfs_format((lfs_t*)pHFilesys->pHFS, &g_lfs_cfg);
            if( rval )
            {
                _lfs_err_string((lfs_error_t)rval);
                break;
            }

            rval = lfs_mount((lfs_t*)pHFilesys->pHFS, &g_lfs_cfg);
            if( rval )
            {
                _lfs_err_string((lfs_error_t)rval);
            }
        }
    } while(0);
    return rval;
}

static filesys_err_t
_lfs_deinit(filesys_handle_t *pHFilesys)
{
    if( !pHFilesys )
        return FILESYS_ERR_NULL_POINTER;

    lfs_unmount((lfs_t*)pHFilesys->pHFS);
    return 0;
}

static filesys_err_t
_lfs_format(filesys_handle_t *pHFilesys)
{
    int     rval = 0;

    if( !pHFilesys )
        return FILESYS_ERR_NULL_POINTER;

    do {
        rval = lfs_format((lfs_t*)pHFilesys->pHFS, &g_lfs_cfg);
        if( rval )
        {
            rval = FILESYS_ERR_FAIL;
            break;
        }

        rval = lfs_mount((lfs_t*)pHFilesys->pHFS, &g_lfs_cfg);
        if( rval )
        {
            rval = FILESYS_ERR_MOUNT_FAIL;
            break;
        }
    } while(0);
    return rval;
}


static filesys_err_t
_lfs_ls(filesys_handle_t *pHFilesys, char *pDir_name)
{
    int     rval = FILESYS_ERR_OK;

    if( !pHFilesys || !pHFilesys->pHFS || !pHFilesys->cb_file_ls )
        return FILESYS_ERR_NULL_POINTER;

    do {
        lfs_t       *pHLfs = (lfs_t*)pHFilesys->pHFS;
        lfs_dir_t   root_dir = {0};
        lfs_info_t  finfo = {0};

        rval = lfs_dir_open(pHLfs, &root_dir, pDir_name);
        if( rval )  break;

        while( 1 )
        {
            rval = lfs_dir_read(pHLfs, &root_dir, &finfo);
            if( rval <= 0 )  break;

        #if 1
            if( finfo.type == LFS_TYPE_REG )
                pHFilesys->cb_file_ls((char*)finfo.name, finfo.size);
        #else
            switch( finfo.type )
            {
                case LFS_TYPE_REG: printf("reg "); break;
                case LFS_TYPE_DIR: printf("dir "); break;
                default:           printf("?   "); break;
            }

            static const char   *prefixes[] = { "", "K", "M", "G" };
            for(int i = sizeof(prefixes)/sizeof(prefixes[0])-1; i >= 0; i--)
            {
                if( finfo.size >= ((1 << 10*i) - 1) )
                {
                    printf("%*u%sBytes ", 4 - (i != 0), finfo.size >> (10 * i), prefixes[i]);
                    break;
                }
            }

            printf("%s\n", finfo.name);
        #endif
        }

        rval = lfs_dir_close(pHLfs, &root_dir);

    } while(0);

    return rval;
}

static filesys_err_t
_lfs_stat(filesys_handle_t *pHFilesys, char *path, filesys_stat_t *pStat)
{
    int     rval = FILESYS_ERR_OK;

    if( !pHFilesys || !path || !pStat )
        return FILESYS_ERR_NULL_POINTER;

    {
        lfs_info_t      finfo = {0};

        rval = lfs_stat((lfs_t*)pHFilesys->pHFS, path, &finfo);

        pStat->size = finfo.size;
        pStat->type = (finfo.type == LFS_TYPE_REG) ? FILESYS_FTYPE_FILE
                    : (finfo.type == LFS_TYPE_DIR)  ? FILESYS_FTYPE_DIR
                    : FILESYS_FTYPE_UNKNOWN;

        #if 1
        int             len = 0;
        len = strlen((char*)finfo.name) + 1;
        len = (len < sizeof(pStat->name)) ? len : sizeof(pStat->name);
        memcpy(pStat->name, (char*)finfo.name, len);
        #else
        snprintf(pStat->name, sizeof(pStat->name), "%s", (char*)stat.name);
        #endif
    }
    return rval;
}

static HAFILE
_lfs_open(filesys_handle_t *pHFilesys, char *path, filesys_mode_t mode)
{
    hfile_lfs_t         *pHFile_cur = 0;

    if( !pHFilesys || !path )
        return 0;

    do {
        int     rval = 0;
        int     file_mode = LFS_O_CREAT;//LFS_O_RDWR;

        if( mode & FILESYS_MODE_RD )
            file_mode |= LFS_O_RDONLY;
        if( mode & FILESYS_MODE_WR )
            file_mode |= LFS_O_WRONLY;

        if(mode & FILESYS_MODE_APPEND)
            file_mode |= LFS_O_APPEND;

        for(int i = 0; i < CONFIG_FILE_CNT; i++)
        {
            if( g_hfile[i].fd.id != (uint16_t)-1 )
                continue;

            pHFile_cur = &g_hfile[i];
            break;
        }

        if( !pHFile_cur )
        {
            err("Count of opened files is max \n");
            break;
        }

        pHFile_cur->pHLfs = (lfs_t*)pHFilesys->pHFS;

        rval = lfs_file_opencfg((lfs_t*)pHFilesys->pHFS, &pHFile_cur->fd, path, file_mode, &pHFile_cur->fd_cfg);
        if( rval )
        {
            pHFile_cur = 0;
            _lfs_err_string((lfs_error_t)rval);
            break;
        }
    } while(0);

    return pHFile_cur;
}

static int
_lfs_close(HAFILE hFile)
{
    int             rval = 0;
    hfile_lfs_t     *pHFile_cur = (hfile_lfs_t*)hFile;

    do {
        rval = lfs_file_close((lfs_t*)pHFile_cur->pHLfs, &pHFile_cur->fd);
        if( rval )      break;

        memset(pHFile_cur, 0x0, sizeof(hfile_lfs_t));

        pHFile_cur->fd.id         = -1;
        pHFile_cur->fd_cfg.buffer = (void*)pHFile_cur->file_cache;
    } while(0);

    return rval;
}

static int
_lfs_read(
    uint8_t *pBuf,
    int     size,
    int     nmemb,
    HAFILE  hFile)
{
    hfile_lfs_t     *pHFile_cur = (hfile_lfs_t*)hFile;
    int             rval = 0;

    rval = lfs_file_read((lfs_t*)pHFile_cur->pHLfs, &pHFile_cur->fd, pBuf, size * nmemb);
    return rval;
}

static int
_lfs_write(
    uint8_t *pBuf,
    int     size,
    int     nmemb,
    HAFILE  hFile)
{
    hfile_lfs_t     *pHFile_cur = (hfile_lfs_t*)hFile;
    int             rval = 0;

    rval = lfs_file_write((lfs_t*)pHFile_cur->pHLfs, &pHFile_cur->fd, pBuf, size * nmemb);
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
filesys_fs_desc_t       g_filesys_littlefs =
{
    .init    = _lfs_init,
    .deinit  = _lfs_deinit,
    .format  = _lfs_format,
    .ls      = _lfs_ls,
    .stat    = _lfs_stat,
    .open    = _lfs_open,
    .close   = _lfs_close,
    .read    = _lfs_read,
    .write   = _lfs_write,
};

#if 0 // reference example

int lfs_ls(lfs_t *lfs, const char *path)
{
    lfs_dir_t dir;
    int err = lfs_dir_open(lfs, &dir, path);
    if (err) {
        return err;
    }

    struct lfs_info info;
    while (true) {
        int res = lfs_dir_read(lfs, &dir, &info);
        if (res < 0) {
            return res;
        }

        if (res == 0) {
            break;
        }

        switch (info.type) {
            case LFS_TYPE_REG: printf("reg "); break;
            case LFS_TYPE_DIR: printf("dir "); break;
            default:           printf("?   "); break;
        }

        static const char *prefixes[] = {"", "K", "M", "G"};
        for (int i = sizeof(prefixes)/sizeof(prefixes[0])-1; i >= 0; i--) {
            if (info.size >= (1 << 10*i)-1) {
                printf("%*u%sB ", 4-(i != 0), info.size >> 10*i, prefixes[i]);
                break;
            }
        }

        printf("%s\n", info.name);
    }

    err = lfs_dir_close(lfs, &dir);
    if (err) {
        return err;
    }

    return 0;
}

lfs_ls(&lfs, "/");
#endif

