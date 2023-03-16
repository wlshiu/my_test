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
#define CONFIG_FS_CACHE_SZ              512

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

//=============================================================================
//                  Global Data Definition
//=============================================================================
uint32_t    g_read_buf[CONFIG_FS_CACHE_SZ >> 2];
uint32_t    g_prog_buf[CONFIG_FS_CACHE_SZ >> 2];
uint32_t    g_file_cache[CONFIG_FS_CACHE_SZ >> 2];
uint32_t    g_lookahead_buf[CONFIG_FS_CACHE_SZ >> 2];

static lfs_t        g_lfs_flash = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

static int
_lfs_deskio_read(
    const struct lfs_config *c,
    lfs_block_t             block,
    lfs_off_t               off,
    void                    *buffer,
    lfs_size_t              size)
{
    extfc_read((uint8_t *)buffer, c->block_size * block + off, size);
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
        extfc_program(pBuffer, WriteAddr, page_remain);

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

    extfc_erase(EXTFC_ERASE_SECTOR, Dst_Addr, 1);
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
    .cache_size        = sizeof(g_file_cache),
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
_lfs_init(filesys_handle_t *hFilesys, filesys_init_cfg_t *pCfg)
{
    int     rval = 0;

    do {
        if( !hFilesys )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        memset(hFilesys, 0x0, sizeof(filesys_handle_t));
        memset(&g_lfs_flash, 0x0, sizeof(g_lfs_flash));

        hFilesys->pHFS = (void*)&g_lfs_flash;

        // mount the file system
        rval = lfs_mount((lfs_t*)hFilesys->pHFS, &g_lfs_cfg);

        /**
         *  reformat if we can't mount the file system
         *  this should only happen on the first boot
         */
        if( rval )
        {
            lfs_format((lfs_t*)hFilesys->pHFS, &g_lfs_cfg);
            lfs_mount((lfs_t*)hFilesys->pHFS, &g_lfs_cfg);
        }
    } while(0);
    return rval;
}

static filesys_err_t
_lfs_deinit(filesys_handle_t *hFilesys)
{
    if( !hFilesys )
        return FILESYS_ERR_NULL_POINTER;

    lfs_unmount((lfs_t*)hFilesys->pHFS);
    return 0;
}

static filesys_err_t
_lfs_format(filesys_handle_t *hFilesys)
{
    if( !hFilesys )
        return FILESYS_ERR_NULL_POINTER;

    do {
        lfs_format((lfs_t*)hFilesys->pHFS, &g_lfs_cfg);
        lfs_mount((lfs_t*)hFilesys->pHFS, &g_lfs_cfg);
    } while(0);
    return 0;
}

static HAFILE
_lfs_open(filesys_handle_t *hFilesys, char *path, filesys_mode_t mode)
{
    return 0;
}

static int
_lfs_close(HAFILE hFile)
{
    return 0;
}

static int
_lfs_read(
    uint8_t *pBuf,
    int     size,
    int     nmemb,
    HAFILE  hFile)
{
    return 0;
}

static int
_lfs_write(
    uint8_t *pBuf,
    int     size,
    int     nmemb,
    HAFILE  hFile)
{
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
filesys_fs_desc_t       g_filesys_littlefs =
{
    .init    = _lfs_init,
    .deinit  = _lfs_deinit,
    .format  = _lfs_format,
    .open    = _lfs_open,
    .close   = _lfs_close,
    .read    = _lfs_read,
    .write   = _lfs_write,
};

