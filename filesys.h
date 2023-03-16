/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file filesys.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/03/16
 * @license
 * @description
 */

#ifndef __filesys_H_wFyTY8ql_lda8_H6qf_szCw_uzobxXVz0J9X__
#define __filesys_H_wFyTY8ql_lda8_H6qf_szCw_uzobxXVz0J9X__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
//#define CONFIG_USE_LITTLEFS
#define CONFIG_USE_SPIFFS

#if defined(CONFIG_USE_LITTLEFS)
#include "lfs.h"
#elif defined(CONFIG_USE_SPIFFS)
#include "spiffs.h"
#endif

#define CONFIG_FILE_NAME_LEN    32
#define CONFIG_FILE_CNT         10

typedef enum filesys_err
{
    FILESYS_ERR_OK              = 0,
    FILESYS_ERR_FAIL            = -1,
    FILESYS_ERR_NOT_FOUND       = -2,
    FILESYS_ERR_TIMEOUT         = -3,
    FILESYS_ERR_WRONG_PARAM     = -4,
    FILESYS_ERR_NULL_POINTER    = -5,

} filesys_err_t;

typedef enum filesys_mode
{
    FILESYS_MODE_RD     = 0x1 << 1,
    FILESYS_MODE_WR     = 0x1 << 2,
    FILESYS_MODE_APPEND = 0x1 << 3,
    FILESYS_MODE_RDWR   = (FILESYS_MODE_RD | FILESYS_MODE_WR),

} filesys_mode_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef void*   HAFILE;

typedef struct filesys_handle
{
    void      *pHFS;

} filesys_handle_t;

typedef struct filesys_init_cfg
{
    #if defined(CONFIG_USE_LITTLEFS)
    lfs_config       init_cfg;
    #elif defined(CONFIG_USE_SPIFFS)
    spiffs_config    init_cfg;
    #endif

} filesys_init_cfg_t;

typedef struct filesys_stat
{
    uint32_t    size;
    char        name[CONFIG_FILE_NAME_LEN];
} filesys_stat_t;

typedef struct filesys_fs_desc
{
    filesys_err_t   (*init)(filesys_handle_t *hFilesys, filesys_init_cfg_t *pCfg);
    filesys_err_t   (*deinit)(filesys_handle_t *hFilesys);
    filesys_err_t   (*format)(filesys_handle_t *hFilesys);
    filesys_err_t   (*ls)(filesys_handle_t *hFilesys);

    HAFILE  (*open)(filesys_handle_t *hFilesys, char *path, filesys_mode_t mode);
    int     (*close)(HAFILE hFile);

    int   (*read)(uint8_t *pBuf, int size, int nmemb, HAFILE hFile);
    int   (*write)(uint8_t *pBuf, int size, int nmemb, HAFILE hFile);

} filesys_fs_desc_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
filesys_err_t
filesys_init(filesys_handle_t *pHFilesys, filesys_init_cfg_t *pCfg);

filesys_err_t
filesys_deinit(filesys_handle_t *pHFilesys);

filesys_err_t
filesys_format(filesys_handle_t *pHFilesys);

HAFILE
filesys_open(filesys_handle_t *pHFilesys, char *path, filesys_mode_t mode);

filesys_err_t
filesys_close(HAFILE hFile);

int
filesys_read(uint8_t *pBuf, int size, int nmemb, HAFILE hFile);

int
filesys_write(uint8_t *pBuf, int size, int nmemb, HAFILE hFile);

#ifdef __cplusplus
}
#endif

#endif
