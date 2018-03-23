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
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum avi_codec
{
    AVI_CODEC_NONE    = 0,
    AVI_CODEC_MJPG,
    AVI_CODEC_PCM,

} avi_codec_t;

typedef enum avi_frm_type
{
    AVI_FRM_UNKNOWN     = 0,
    AVI_FRM_VIDEO,
    AVI_FRM_AUDIO,

} avi_frm_type_t;

typedef enum avi_frame_state
{
    AVI_FRAME_NONE       = 0,
    AVI_FRAME_PARTIAL,
    AVI_FRAME_END,

} avi_frame_state_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct avi_video_cfg
{
    avi_codec_t     vcodec;
    uint32_t        width;
    uint32_t        height;
    uint32_t        fps;

} avi_video_cfg_t;


typedef struct avi_audio_cfg
{
    avi_codec_t     acodec;
    uint32_t        sample_rate;

} avi_audio_cfg_t;

typedef struct avi_update_info
{
    uint32_t    total_frames;
    uint32_t    total_file_size;

    uint32_t    media_data_size;  // media data size (audio + video data)
} avi_update_info_t;


typedef struct avi_media_info
{
    avi_codec_t     codec;

    union {
        struct {
            float           fps;
        } vid;

        struct {
            uint32_t        sample_rate;
        } aud;
    };


} avi_media_info_t;

typedef struct avi_frame_info
{
    avi_frame_state_t   frm_state;
    uint8_t             *pFrame_addr;
    int                 frame_len;

} avi_frame_info_t;

struct avi_ctrl_info;

/**
 *  \brief  CB_MISC_PROC
 *
 *  \param [in] pCtrl_info      pass control info from avi_demux_media_data()
 *  \return                     0: ok, other: leave braking in avi_demux_media_data() and return from avi_demux_media_data()
 *
 *  \details
 *      Because avi_demux_media_data() will brake flow, use callback to do something which user wants.
 */
typedef int (*CB_MISC_PROC)(struct avi_ctrl_info *pCtrl_info);

/**
 *  \brief  CB_FILL_BUF
 *
 *  \param [in] pCtrl_info      pass control info from avi_demux_media_data()
 *  \param [in] pBuf            the buffer address which need to fill
 *  \param [in] pLen            the length of buffer and user need to re-assign the real length of filling
 *  \return                     0: ok, other: leave braking in avi_demux_media_data() and return from avi_demux_media_data()
 *
 *  \details
 */
typedef int (*CB_FILL_BUF)(struct avi_ctrl_info *pCtrl_info, uint8_t *pBuf, uint32_t *pLen);

/**
 *  \brief  CB_FRAME_STATE
 *
 *  \param [in] pCtrl_info      pass control info from avi_demux_media_data()
 *  \param [in] pMedia_info     the media info of current avi file
 *  \param [in] frm_info        current frame parsing info
 *                                  frame state:
 *                                      AVI_FRAME_NONE      => no frame
 *                                      AVI_FRAME_PARTIAL   => get parts of a frame
 *                                      AVI_FRAME_END       => the last part of a frame
 *
 *  \return                     0: ok, other: leave braking in avi_demux_media_data() and return from avi_demux_media_data()
 *
 *  \details
 */
typedef int (*CB_FRAME_STATE)(struct avi_ctrl_info *pCtrl_info, avi_media_info_t *pMedia_info, avi_frame_info_t *pFrm_info);

typedef struct avi_ctrl_info
{
    CB_MISC_PROC        cb_misc_proc;
    CB_FRAME_STATE      cb_frame_state;
    CB_FILL_BUF         cb_fill_buf;

    // TODO: Need to give a suggest value (more than 1 frame size)
    uint8_t             *pRing_buf;
    int                 ring_buf_size;

    void    *pPrivate_data;

} avi_ctrl_info_t;



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
avi_mux_reset_header(
    avi_video_cfg_t   *pVid_cfg,
    avi_audio_cfg_t   *pAud_cfg,
    uint32_t          align_pow2_num);


int
avi_mux_update_info(
    avi_update_info_t   *pUpdate_info);


uint32_t
avi_mux_get_header_size(void);


int
avi_mux_gen_header(
    uint8_t     *pHeader_buf,
    uint32_t    *pHeader_buf_len);


int
avi_parse_header(
    uint32_t    *pHeader_buf,
    uint32_t    header_buf_len,
    uint32_t    *pMovi_offset);


int
avi_demux_media_data(
    avi_ctrl_info_t     *pCtrl_info,
    uint32_t            *pIs_braking);


#ifdef __cplusplus
}
#endif

#endif
