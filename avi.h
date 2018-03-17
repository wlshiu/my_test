/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file avi.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/03/16
 * @license
 * @description
 */

#ifndef __avi_mux_H_woTq1j4y_ljZ6_HUmN_s74U_uB4coInby18y__
#define __avi_mux_H_woTq1j4y_ljZ6_HUmN_s74U_uB4coInby18y__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum avi_vid
{
    AVI_VID_NONE    = 0,
    AVI_VID_MJPG,

} avi_vid_t;

typedef enum avi_aud
{
    AVI_AUD_NONE    = 0,
    AVI_AUD_PCM,

} avi_aud_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct avi_video_cfg
{
    avi_vid_t       vid_type;
    uint32_t        width;
    uint32_t        height;
    uint32_t        fps;

} avi_video_cfg_t;


typedef struct avi_audio_cfg
{
    avi_aud_t       aud_type;
    uint32_t        sample_rate;

} avi_audio_cfg_t;

/**
 *  I/O handler
 */
typedef struct avi_io_desc
{
    int     (*cb_open)(struct avi_io_desc *pDesc, const char *pFilename, const char *pMode);
    int     (*cb_close)(struct avi_io_desc *pDesc);
    int     (*cb_read)(struct avi_io_desc *pDesc, uint8_t *pBuf, uint32_t length);
    int     (*cb_write)(struct avi_io_desc *pDesc, uint8_t *pBuf, uint32_t length);

    void    *pFile;
    void    *pPrivate;

} avi_io_desc_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
avi_mux_init_header(
    avi_video_cfg_t   *pVid_cfg,
    avi_audio_cfg_t   *pAud_cfg,
    uint32_t          align_num);



int
avi_mux_get_header(
    uint8_t     *pHeader_buf,
    uint32_t    *pHeader_buf_len);


#ifdef __cplusplus
}
#endif

#endif
