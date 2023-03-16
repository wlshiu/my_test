/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file filesys.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/03/16
 * @license
 * @description
 */


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
#if defined(CONFIG_USE_LITTLEFS)
extern filesys_fs_desc_t    g_filesys_littlefs;
static filesys_fs_desc_t    *g_filesys_desc = &g_filesys_littlefs;
#elif defined(CONFIG_USE_SPIFFS)
extern filesys_fs_desc_t    g_filesys_spiffs;
static filesys_fs_desc_t    *g_filesys_desc = &g_filesys_spiffs;
#endif

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
filesys_err_t
filesys_init(filesys_handle_t *pHFilesys, filesys_init_cfg_t *pCfg)
{
    filesys_err_t       rval = FILESYS_ERR_OK;
    do {
        if( !pHFilesys )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        if( g_filesys_desc && g_filesys_desc->init )
            rval = g_filesys_desc->init(pHFilesys, pCfg);

    } while(0);
    return rval;
}

filesys_err_t
filesys_deinit(filesys_handle_t *pHFilesys)
{
    filesys_err_t       rval = FILESYS_ERR_OK;
    do {
        if( !pHFilesys )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        if( g_filesys_desc && g_filesys_desc->deinit )
            rval = g_filesys_desc->deinit(pHFilesys);

    } while(0);
    return rval;
}

filesys_err_t
filesys_format(filesys_handle_t *pHFilesys)
{
    filesys_err_t       rval = FILESYS_ERR_OK;
    do {
        if( !pHFilesys )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        if( g_filesys_desc && g_filesys_desc->format )
            rval = g_filesys_desc->format(pHFilesys);

    } while(0);
    return rval;
}

HAFILE
filesys_open(filesys_handle_t *pHFilesys, char *path, filesys_mode_t mode)
{
    HAFILE              hAFile = 0;
    do {
        if( !pHFilesys || !path )
            break;

        if( g_filesys_desc && g_filesys_desc->open )
            hAFile = g_filesys_desc->open(pHFilesys, path, mode);

    } while(0);

    return hAFile;
}

filesys_err_t
filesys_close(HAFILE hFile)
{
    filesys_err_t       rval = FILESYS_ERR_OK;
    do {
        if( !hFile )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        if( g_filesys_desc && g_filesys_desc->close )
            rval = g_filesys_desc->close(hFile);

    } while(0);
    return rval;
}

int
filesys_read(uint8_t *pBuf, int size, int nmemb, HAFILE hFile)
{
    filesys_err_t       rval = FILESYS_ERR_OK;
    do {
        if( !hFile )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        if( g_filesys_desc && g_filesys_desc->read )
            rval = g_filesys_desc->read(pBuf, size, nmemb, hFile);

    } while(0);
    return rval;
}

int
filesys_write(uint8_t *pBuf, int size, int nmemb, HAFILE hFile)
{
    filesys_err_t       rval = FILESYS_ERR_OK;
    do {
        if( !hFile )
        {
            rval = FILESYS_ERR_NULL_POINTER;
            break;
        }

        if( g_filesys_desc && g_filesys_desc->write )
            rval = g_filesys_desc->write(pBuf, size, nmemb, hFile);

    } while(0);
    return rval;
}
