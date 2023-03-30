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

#define CONFIG_FILE_NAME_LEN    22 // (CONFIG_FILE_NAME_LEN + 2) for align4
#define CONFIG_FILE_CNT         10

typedef enum filesys_err
{
    FILESYS_ERR_OK                  = 0,
    FILESYS_ERR_FAIL                = -1,
    FILESYS_ERR_NOT_FOUND           = -2,
    FILESYS_ERR_TIMEOUT             = -3,
    FILESYS_ERR_WRONG_PARAM         = -4,
    FILESYS_ERR_NULL_POINTER        = -5,
    FILESYS_ERR_MOUNT_FAIL          = -6,
    FILESYS_ERR_FLASH_INIT_FAIL     = -7,

} filesys_err_t;

typedef enum filesys_sys_type
{
    FILESYS_SYS_TYPE_SPIFFS,
    FILESYS_SYS_TYPE_LFS,
} filesys_sys_type_t;

typedef enum filesys_mode
{
    FILESYS_MODE_RD     = 0x1 << 1,
    FILESYS_MODE_WR     = 0x1 << 2,
    FILESYS_MODE_APPEND = 0x1 << 3,
    FILESYS_MODE_RDWR   = (FILESYS_MODE_RD | FILESYS_MODE_WR),
    FILESYS_MODE_TRUNC  = 0x1 << 4,  // over-write file

} filesys_mode_t;

typedef enum filesys_ftype
{
    FILESYS_FTYPE_UNKNOWN  = 0,
    FILESYS_FTYPE_FILE,
    FILESYS_FTYPE_DIR,
} filesys_ftype_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef void*   HAFILE;

/**
 *  \brief  list files
 *              Callback to user level
 *
 *  \param [in] name        file name
 *  \param [in] size        file size
 *  \return
 *      error code, @ref filesys_err_t
 */
typedef filesys_err_t (*cb_file_ls_t)(char *name, int size);

/**
 *  \brief  Low Level device initialize
 *              Callback to user level
 *
 *  \return
 *      error code, @ref filesys_err_t
 */
typedef filesys_err_t (*cb_ll_init_t)(void);

typedef filesys_err_t (*cb_ll_get_jedec_id_t)(uint32_t *pJID);

/**
 *  \brief  flash sector erase operation
 *              Callback to user level
 *
 *  \param [in] flash_addr      the physical address of a flash
 *  \param [in] sector_cnt      sector count
 *  \return
 *      error code, @ref filesys_err_t
 */
typedef filesys_err_t (*cb_flash_sector_erase_t)(uint32_t flash_addr, int sector_cnt);

/**
 *  \brief  flash read operation
 *              Callback to user level
 *
 *  \param [in] pSys_buf        Pointer to a system buffer
 *  \param [in] flash_addr      the physical address of a flash
 *  \param [in] nbytes          data size with bytes
 *  \return
 *      error code, @ref filesys_err_t
 */
typedef filesys_err_t (*cb_flash_read_t)(uint8_t *pSys_buf, uint32_t flash_addr, int nbytes);

/**
 *  \brief  flash program operation
 *              Callback to user level
 *
 *  \param [in] pSys_buf        Pointer to a system buffer
 *  \param [in] flash_addr      the physical address of a flash
 *  \param [in] nbytes          data size with bytes
 *  \return
 *      error code, @ref filesys_err_t
 */
typedef filesys_err_t (*cb_flash_prog_t)(uint8_t *pSys_buf, uint32_t flash_addr, int nbytes);

typedef struct filesys_ll_dev
{
    cb_flash_sector_erase_t     cb_sec_erase;
    cb_flash_read_t             cb_flash_read;
    cb_flash_prog_t             cb_flash_prog;
} filesys_ll_dev_t;

typedef struct filesys_handle
{
    void            *pHFS;
    
    uint32_t        jedec_id;

    cb_file_ls_t    cb_file_ls;

} filesys_handle_t;

typedef struct filesys_init_cfg
{
    filesys_sys_type_t          sys_type;

    cb_ll_init_t                cb_ll_init;
    cb_ll_get_jedec_id_t        cb_ll_get_jedec_id;
    cb_flash_sector_erase_t     cb_sec_erase;
    cb_flash_read_t             cb_flash_read;
    cb_flash_prog_t             cb_flash_prog;

} filesys_init_cfg_t;

typedef struct filesys_stat
{
    uint32_t            size;
    char                name[CONFIG_FILE_NAME_LEN + 1];
    filesys_ftype_t     type;
} filesys_stat_t;

typedef struct filesys_fs_desc
{
    filesys_err_t   (*init)(filesys_handle_t *pHFilesys, filesys_init_cfg_t *pCfg);
    filesys_err_t   (*deinit)(filesys_handle_t *pHFilesys);
    filesys_err_t   (*format)(filesys_handle_t *pHFilesys);
    filesys_err_t   (*ls)(filesys_handle_t *pHFilesys, char *pDir_name);
    filesys_err_t   (*stat)(filesys_handle_t *pHFilesys, char *path, filesys_stat_t *pStat);
    filesys_err_t   (*capacity)(filesys_handle_t *pHFilesys, uint32_t *pTotal_bytes, uint32_t *pUsed_bytes);

    HAFILE  (*open)(filesys_handle_t *pHFilesys, char *path, filesys_mode_t mode);
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

filesys_err_t
filesys_ls(filesys_handle_t *pHFilesys, char *pDir_name);

filesys_err_t
filesys_stat(filesys_handle_t *pHFilesys, char *path, filesys_stat_t *pStat);

filesys_err_t
filesys_capacity(filesys_handle_t *pHFilesys, uint32_t *pTotal_bytes, uint32_t *pUsed_bytes);

HAFILE
filesys_open(filesys_handle_t *pHFilesys, char *path, filesys_mode_t mode);

int
filesys_close(HAFILE hFile);

int
filesys_read(uint8_t *pBuf, int size, int nmemb, HAFILE hFile);

int
filesys_write(uint8_t *pBuf, int size, int nmemb, HAFILE hFile);

#ifdef __cplusplus
}
#endif

#endif
