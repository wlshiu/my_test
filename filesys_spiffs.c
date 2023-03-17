/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file filesys_spiffs.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/03/16
 * @license
 * @description
 */


#include "filesys.h"
#include "spiffs.h"

#include "extfc.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_EXT_FLASH_PHYS_SZ        (512ul << 10)//(512ul << 10)
#define CONFIG_EXT_FLASH_BLOCK_SZ       (32ul << 10)//(64ul << 10)
#define CONFIG_EXT_FLASH_PHYS_ADDR      (0)
#define CONFIG_EXT_FLASH_LOG_PAGE_SZ    (256)

#define EXT_FLASH_SECTOR_SZ             (4ul << 10)
#define EXT_FLASH_PAGE_SZ               256
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct hfile_spiffs
{
    spiffs              *pHSpiffs;
    spiffs_file         fd;
} hfile_spiffs_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern filesys_ll_dev_t     g_filesys_dev;

static spiffs   g_hSpiffs = {0};

static u32_t     g_spiffs_work_buf[(CONFIG_EXT_FLASH_LOG_PAGE_SZ * 2) >> 2];
static u32_t     g_spiffs_fds[(32 * 4) >> 2];
static u32_t     g_spiffs_cache_buf[((CONFIG_EXT_FLASH_LOG_PAGE_SZ + 32) * 4) >> 2];

static hfile_spiffs_t       g_hfile[CONFIG_FILE_CNT] = {0};
//static uint32_t             g_file_cnt = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================

static s32_t _spiffs_ll_erase(uint32_t addr, uint32_t len)
{
    int     rval = 0;
    int     count = (len + EXT_FLASH_SECTOR_SZ - 1) >> 12;

    for(int i = 0; i < count; i++)
    {
        g_filesys_dev.cb_sec_erase(addr + i * EXT_FLASH_SECTOR_SZ, 1);
    }

    return rval;
}

static s32_t _spiffs_ll_read(uint32_t addr, uint32_t size, uint8_t *dst)
{
    int     rval = 0;

    g_filesys_dev.cb_flash_read(dst, addr, size);

    return rval;
}

static s32_t _spiffs_ll_write(uint32_t addr, uint32_t size, uint8_t *src)
{
    int         rval = 0;
    uint32_t    WriteAddr = addr;
    uint32_t    addr_end = 0;
    int         length = 0;
    uint8_t     *pData = src;
    uint16_t    NBytes = size;


    addr_end = (WriteAddr + EXT_FLASH_PAGE_SZ) & ~(EXT_FLASH_PAGE_SZ - 1);

    length = addr_end - addr;
    length = (length > NBytes) ? NBytes : length;

    addr_end = WriteAddr + NBytes;
    do {
        g_filesys_dev.cb_flash_prog(pData, WriteAddr, NBytes);

        WriteAddr += length;
        pData     += length;

        length = ((WriteAddr + EXT_FLASH_PAGE_SZ) > addr_end)
               ? (addr_end - WriteAddr) : EXT_FLASH_PAGE_SZ;

    } while( WriteAddr < addr_end );

    return rval;
}


static const spiffs_config        g_spiffs_def_cfg =
{
    .hal_erase_f        = _spiffs_ll_erase,
    .hal_read_f         = _spiffs_ll_read,
    .hal_write_f        = _spiffs_ll_write,

#if (SPIFFS_SINGLETON == 0)
    .phys_size          = CONFIG_EXT_FLASH_PHYS_SZ,
    .phys_addr          = CONFIG_EXT_FLASH_PHYS_ADDR,
    .phys_erase_block   = CONFIG_EXT_FLASH_BLOCK_SZ,
    .log_block_size     = CONFIG_EXT_FLASH_BLOCK_SZ,
    .log_page_size      = CONFIG_EXT_FLASH_LOG_PAGE_SZ,

    #if SPIFFS_FILEHDL_OFFSET
    .fh_ix_offset = 1000,
    #endif

#endif
};


static const char* _spiffs_errstr(s32_t err)
{
    if( err > 0 )   return "";

    switch (err)
    {
        case SPIFFS_OK                        :     return " OK";
        case SPIFFS_ERR_NOT_MOUNTED           :     return " not mounted";
        case SPIFFS_ERR_FULL                  :     return " full";
        case SPIFFS_ERR_NOT_FOUND             :     return " not found";
        case SPIFFS_ERR_END_OF_OBJECT         :     return " end of object";
        case SPIFFS_ERR_DELETED               :     return " deleted";
        case SPIFFS_ERR_NOT_FINALIZED         :     return " not finalized";
        case SPIFFS_ERR_NOT_INDEX             :     return " not index";
        case SPIFFS_ERR_OUT_OF_FILE_DESCS     :     return " out of file descs";
        case SPIFFS_ERR_FILE_CLOSED           :     return " file closed";
        case SPIFFS_ERR_FILE_DELETED          :     return " file deleted";
        case SPIFFS_ERR_BAD_DESCRIPTOR        :     return " bad descriptor";
        case SPIFFS_ERR_IS_INDEX              :     return " is index";
        case SPIFFS_ERR_IS_FREE               :     return " is free";
        case SPIFFS_ERR_INDEX_SPAN_MISMATCH   :     return " index span mismatch";
        case SPIFFS_ERR_DATA_SPAN_MISMATCH    :     return " data span mismatch";
        case SPIFFS_ERR_INDEX_REF_FREE        :     return " index ref free";
        case SPIFFS_ERR_INDEX_REF_LU          :     return " index ref lu";
        case SPIFFS_ERR_INDEX_REF_INVALID     :     return " index ref invalid";
        case SPIFFS_ERR_INDEX_FREE            :     return " index free";
        case SPIFFS_ERR_INDEX_LU              :     return " index lu";
        case SPIFFS_ERR_INDEX_INVALID         :     return " index invalid";
        case SPIFFS_ERR_NOT_WRITABLE          :     return " not writable";
        case SPIFFS_ERR_NOT_READABLE          :     return " not readable";
        case SPIFFS_ERR_CONFLICTING_NAME      :     return " conflicting name";
        case SPIFFS_ERR_NOT_CONFIGURED        :     return " not configured";

        case SPIFFS_ERR_NOT_A_FS              :     return " not a fs";
        case SPIFFS_ERR_MOUNTED               :     return " mounted";
        case SPIFFS_ERR_ERASE_FAIL            :     return " erase fail";
        case SPIFFS_ERR_MAGIC_NOT_POSSIBLE    :     return " magic not possible";

        case SPIFFS_ERR_NO_DELETED_BLOCKS     :     return " no deleted blocks";

        case SPIFFS_ERR_FILE_EXISTS           :     return " file exists";

        case SPIFFS_ERR_NOT_A_FILE            :     return " not a file";
        case SPIFFS_ERR_RO_NOT_IMPL           :     return " ro not impl";
        case SPIFFS_ERR_RO_ABORTED_OPERATION  :     return " ro aborted operation";
        case SPIFFS_ERR_PROBE_TOO_FEW_BLOCKS  :     return " probe too few blocks";
        case SPIFFS_ERR_PROBE_NOT_A_FS        :     return " probe not a fs";
        case SPIFFS_ERR_NAME_TOO_LONG         :     return " name too long";

        case SPIFFS_ERR_IX_MAP_UNMAPPED       :     return " ix map unmapped";
        case SPIFFS_ERR_IX_MAP_MAPPED         :     return " ix map mapped";
        case SPIFFS_ERR_IX_MAP_BAD_RANGE      :     return " ix map bad range";

        default                               :     return " <unknown>";
    }
}
static void
_spiffs_err_log(uint32_t err_code)
{
    printf("%s\n", _spiffs_errstr(err_code));
    return;
}


static void
_spiffs_check_cb_f(
    spiffs_check_type   type,
    spiffs_check_report report,
    u32_t               arg1,
    u32_t               arg2)
{
    static u32_t    old_perc = 999;

    u32_t   perc = arg1 * 100 / 256;
    if( report == SPIFFS_CHECK_PROGRESS && old_perc != perc )
    {
        old_perc = perc;
        printf("CHECK REPORT: ");
        switch(type)
        {
            case SPIFFS_CHECK_LOOKUP:   printf("LU ");    break;
            case SPIFFS_CHECK_INDEX:    printf("IX ");    break;
            case SPIFFS_CHECK_PAGE:     printf("PA ");    break;
        }
        printf("%i%%\n", perc);
    }

    if( report != SPIFFS_CHECK_PROGRESS )
    {
        printf("   check: ");
        switch (type)
        {
            case SPIFFS_CHECK_INDEX:    printf("INDEX  "); break;
            case SPIFFS_CHECK_LOOKUP:   printf("LOOKUP "); break;
            case SPIFFS_CHECK_PAGE:     printf("PAGE   "); break;
            default:
                printf("????   ");
                break;
        }

        switch( report )
        {
            case SPIFFS_CHECK_ERROR:                    printf("ERROR %i", arg1);                   break;
            case SPIFFS_CHECK_DELETE_BAD_FILE:          printf("DELETE BAD FILE %04x", arg1);       break;
            case SPIFFS_CHECK_DELETE_ORPHANED_INDEX:    printf("DELETE ORPHANED INDEX %04x", arg1); break;
            case SPIFFS_CHECK_DELETE_PAGE:              printf("DELETE PAGE %04x", arg1);           break;
            case SPIFFS_CHECK_FIX_INDEX:                printf("FIX INDEX %04x:%04x", arg1, arg2);  break;
            case SPIFFS_CHECK_FIX_LOOKUP:               printf("FIX INDEX %04x:%04x", arg1, arg2);  break;
            default:                                    printf("??");                               break;
        }
        printf("\n");
    }
    return;
}

static filesys_err_t
_spiffs_init(filesys_handle_t *pHFilesys, filesys_init_cfg_t *pCfg)
{
    int     rval = FILESYS_ERR_OK;

    do {
        if( !pHFilesys )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        memset(pHFilesys, 0x0, sizeof(filesys_handle_t));
        memset(&g_hSpiffs, 0x0, sizeof(g_hSpiffs));

        memset(&g_hfile, 0x0, sizeof(g_hfile));
        for(int i = 0; i < CONFIG_FILE_CNT; i++)
            g_hfile[i].fd = -1;

//        g_file_cnt = 0;

        pHFilesys->pHFS = (void*)&g_hSpiffs;

        rval = SPIFFS_mount((spiffs*)pHFilesys->pHFS,
                            (spiffs_config*)&g_spiffs_def_cfg,
                            (u8_t*)&g_spiffs_work_buf,
                            (u8_t*)&g_spiffs_fds, sizeof(g_spiffs_fds),
                            g_spiffs_cache_buf, sizeof(g_spiffs_cache_buf),
                            _spiffs_check_cb_f);

        if( rval != SPIFFS_OK && SPIFFS_errno((spiffs*)pHFilesys->pHFS) == SPIFFS_ERR_NOT_A_FS )
        {
            _spiffs_err_log(SPIFFS_errno((spiffs*)pHFilesys->pHFS));
            msg("formatting file system ...\n");
            rval = SPIFFS_format((spiffs*)pHFilesys->pHFS);
            if( rval != SPIFFS_OK )
            {
                rval = FILESYS_ERR_FAIL;
                msg("SPIFFS format failed: %i\n", SPIFFS_errno((spiffs*)pHFilesys->pHFS));
                _spiffs_err_log(SPIFFS_errno((spiffs*)pHFilesys->pHFS));
                break;
            }

            rval = SPIFFS_mount((spiffs*)pHFilesys->pHFS,
                                (spiffs_config*)&g_spiffs_def_cfg,
                                (u8_t*)&g_spiffs_work_buf,
                                (u8_t*)&g_spiffs_fds, sizeof(g_spiffs_fds),
                                g_spiffs_cache_buf, sizeof(g_spiffs_cache_buf),
                                _spiffs_check_cb_f);
        }

        if( rval != SPIFFS_OK )
        {
            rval = FILESYS_ERR_FAIL;
            msg("SPIFFS mount failed: %i\n", SPIFFS_errno((spiffs*)pHFilesys->pHFS));
            _spiffs_err_log(SPIFFS_errno((spiffs*)pHFilesys->pHFS));
            break;
        }

        msg("SPIFFS mounted\n");
    } while(0);
    return rval;
}

static filesys_err_t
_spiffs_deinit(filesys_handle_t *pHFilesys)
{
    if( !pHFilesys )
        return FILESYS_ERR_NULL_POINTER;

    SPIFFS_unmount((spiffs*)pHFilesys->pHFS);
    return 0;
}


static filesys_err_t
_spiffs_format(filesys_handle_t *pHFilesys)
{
    int     rval = FILESYS_ERR_OK;

    if( !pHFilesys )
        return FILESYS_ERR_NULL_POINTER;

    do {
        SPIFFS_unmount((spiffs*)pHFilesys->pHFS);
        rval = SPIFFS_format((spiffs*)pHFilesys->pHFS);
        if( rval )
        {
            rval = FILESYS_ERR_FAIL;
            break;
        }

        rval = SPIFFS_mount((spiffs*)pHFilesys->pHFS,
                            (spiffs_config*)&g_spiffs_def_cfg,
                            (u8_t*)&g_spiffs_work_buf,
                            (u8_t*)&g_spiffs_fds, sizeof(g_spiffs_fds),
                            g_spiffs_cache_buf, sizeof(g_spiffs_cache_buf),
                            _spiffs_check_cb_f);
        if( rval )
        {
            rval = FILESYS_ERR_MOUNT_FAIL;
            break;
        }

    } while(0);
    return rval;
}

static filesys_err_t
_spiffs_ls(filesys_handle_t *pHFilesys, char *pDir_name)
{
    int                 rval = FILESYS_ERR_OK;
    spiffs_DIR          root_dir = {0};
    spiffs_dirent_t     dir_cur = {0};
    spiffs_dirent_t     *pdir_cur = &dir_cur;

    if( !pHFilesys || !pHFilesys->cb_file_ls )
        return FILESYS_ERR_NULL_POINTER;

    SPIFFS_opendir((spiffs*)pHFilesys->pHFS, (pDir_name) ? pDir_name : "/", &root_dir);

    while( (pdir_cur = SPIFFS_readdir(&root_dir, pdir_cur)) )
    {
        if( pHFilesys->cb_file_ls((char*)pdir_cur->name, pdir_cur->size) )
            break;
    }
    SPIFFS_closedir(&root_dir);

    return rval;
}

static filesys_err_t
_spiffs_stat(filesys_handle_t *pHFilesys, char *path, filesys_stat_t *pStat)
{
    int     rval = FILESYS_ERR_OK;

    if( !pHFilesys || !path || !pStat )
        return FILESYS_ERR_NULL_POINTER;

    {
        spiffs_stat     stat = {0};

        rval = SPIFFS_stat((spiffs*)pHFilesys->pHFS, path, &stat);
        pStat->size = stat.size;
        pStat->type = (stat.type == SPIFFS_TYPE_FILE) ? FILESYS_FTYPE_FILE
                    : (stat.type == SPIFFS_TYPE_DIR)  ? FILESYS_FTYPE_DIR
                    : FILESYS_FTYPE_UNKNOWN;

        #if 1
        int             len = 0;
        len = strlen((char*)stat.name) + 1;
        len = (len < sizeof(pStat->name)) ? len : sizeof(pStat->name);
        memcpy(pStat->name, (char*)stat.name, len);
        #else
        snprintf(pStat->name, sizeof(pStat->name), "%s", (char*)stat.name);
        #endif
    }

    return rval;
}

static HAFILE
_spiffs_open(filesys_handle_t *pHFilesys, char *path, filesys_mode_t mode)
{
    hfile_spiffs_t  *pHFile_cur = 0;

    if( !pHFilesys || !path )
        return 0;

    do {
        spiffs_mode     file_mode = SPIFFS_O_CREAT;
        spiffs_file     fd = -1;

        if( mode & FILESYS_MODE_RD )
            file_mode |= SPIFFS_O_RDONLY;
        if( mode & FILESYS_MODE_WR )
            file_mode |= SPIFFS_O_WRONLY;

        if(mode & FILESYS_MODE_APPEND)
            file_mode |= SPIFFS_O_APPEND;

        fd = SPIFFS_open((spiffs*)pHFilesys->pHFS, path, file_mode, 0);
        if( fd < 0 )
        {
            printf("errno %i\n", SPIFFS_errno((spiffs*)pHFilesys->pHFS));
            printf(" %s\n", _spiffs_errstr(SPIFFS_errno((spiffs*)pHFilesys->pHFS)));
            break;
        }

        for(int i = 0; i < CONFIG_FILE_CNT; i++)
        {
            if( g_hfile[i].fd != fd )
                continue;

            // file exist
            pHFile_cur = &g_hfile[i];
            break;
        }

        if( !pHFile_cur )
        {
            for(int i = 0; i < CONFIG_FILE_CNT; i++)
            {
                // new file
                if( g_hfile[i].fd == -1 )
                {
                    g_hfile[i].fd       = fd;
                    g_hfile[i].pHSpiffs = (spiffs*)pHFilesys->pHFS;

                    pHFile_cur = &g_hfile[i];
                    break;
                }
            }
        }

    } while(0);


    return (HAFILE)pHFile_cur;
}

static int
_spiffs_close(HAFILE hFile)
{
    int             rval = 0;
    hfile_spiffs_t  *pHFile_cur = (hfile_spiffs_t*)hFile;

    rval = SPIFFS_close((spiffs*)pHFile_cur->pHSpiffs, pHFile_cur->fd);

    memset(pHFile_cur, 0x0, sizeof(hfile_spiffs_t));
    pHFile_cur->fd = -1;

    return rval;
}

static int
_spiffs_read(
    uint8_t *pBuf,
    int     size,
    int     nmemb,
    HAFILE  hFile)
{
    hfile_spiffs_t  *pHFile_cur = (hfile_spiffs_t*)hFile;
    int             rval = 0;

    rval = SPIFFS_read(pHFile_cur->pHSpiffs, pHFile_cur->fd, pBuf, size * nmemb);

    return rval;
}

static int
_spiffs_write(
    uint8_t *pBuf,
    int     size,
    int     nmemb,
    HAFILE  hFile)
{
    hfile_spiffs_t  *pHFile_cur = (hfile_spiffs_t*)hFile;
    int             rval = 0;

    do {
        rval = SPIFFS_write((spiffs*)pHFile_cur->pHSpiffs, pHFile_cur->fd, pBuf, size * nmemb);
        if( rval )  break;

        rval = SPIFFS_fflush((spiffs*)pHFile_cur->pHSpiffs, pHFile_cur->fd);
        if( rval )  break;
    } while(0);

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
filesys_fs_desc_t       g_filesys_spiffs =
{
    .init    = _spiffs_init,
    .deinit  = _spiffs_deinit,
    .format  = _spiffs_format,
    .ls      = _spiffs_ls,
    .stat    = _spiffs_stat,
    .open    = _spiffs_open,
    .close   = _spiffs_close,
    .read    = _spiffs_read,
    .write   = _spiffs_write,
};




