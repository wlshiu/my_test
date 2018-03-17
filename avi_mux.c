/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file avi_mux.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/03/16
 * @license
 * @description
 */


#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "riff.h"
#include "avi.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum avi_media_track
{
    AVI_MEDIA_TRACK_AUDIO,
    AVI_MEDIA_TRACK_VIDEO,
    AVI_MEDIA_TRACK_TOTAL

} avi_media_track_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define SET_LIST(pStruct, fourCC)   \
        do{ (pStruct)->tag_list = (uint32_t)AVI_FCC_LIST; \
            (pStruct)->fcc = (fourCC);                    \
        }while(0)


//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct avi_list_strl_vid
{
    avi_list_t              list_strl;
    avi_stream_hdr_box_t    strh;
    avi_bmp_info_hdr_box_t  strf;

} avi_list_strl_vid_t;

typedef struct avi_list_strl_aud
{
    avi_list_t              list_strl;
    avi_stream_hdr_box_t    strh;
    avi_wave_fmt_ex_box_t   strf;

} avi_list_strl_aud_t;

typedef struct avi_list_hdrl
{
    avi_list_t              list_hdrl;
    avi_avih_t              avih;

    avi_list_strl_vid_t     list_strl_vid;
    avi_list_strl_aud_t     list_strl_aud;

} avi_list_hdrl_t;

/**
 *  avi context
 */
typedef struct avi_mux_ctxt
{
    avi_riff_t              riff;
    avi_list_hdrl_t         list_hdrl_box;

    avi_list_t              list_movi;  // only list region, no involve media data

    bool                    is_initialized;

    bool                    has_video;
    bool                    has_audio;

    uint32_t                header_size;
} avi_mux_ctxt_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

static avi_mux_ctxt_t   g_avi_ctxt = {.is_initialized = false,};
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
    uint32_t          align_num)
{
    int     rval = 0;

    // reset header
    memset(&g_avi_ctxt, 0x0, sizeof(avi_mux_ctxt_t));

    do {
        uint32_t            header_size = 0;
        avi_riff_t          *pRiff = &g_avi_ctxt.riff;
        avi_list_t          *pList = 0;

        pRiff->tag_riff = (uint32_t)AVI_FCC_RIFF;
        pRiff->fcc      = (uint32_t)AVI_FCC_AVI;

        {   // LIST:hdrl
            avi_list_hdrl_t     *pList_hdrl = &g_avi_ctxt.list_hdrl_box;
            avi_avih_t          *pAvih = &g_avi_ctxt.list_hdrl_box.avih;
            avi_main_hdr_t      *pMain_hdr = &pAvih->main_hdr;

            pList = &pList_hdrl->list_hdrl;
            SET_LIST(pList, (uint32_t)AVI_FCC_HDRL);
            pList->size = 4 + sizeof(avi_avih_t);
            if( pVid_cfg )  pList->size += sizeof(avi_list_strl_vid_t);
            if( pAud_cfg )  pList->size += sizeof(avi_list_strl_aud_t);

            header_size = sizeof(avi_riff_t) + 8 + pList->size + sizeof(g_avi_ctxt.list_movi);

            pAvih->tag_avih = (uint32_t)AVI_FCC_AVIH;
            pAvih->size     = sizeof(avi_main_hdr_t);

            pMain_hdr->dwMaxBytesPerSec   = 7000;
            pMain_hdr->dwFlags            = 16;

            if( pVid_cfg )
            {
                avi_list_strl_vid_t     *pList_strl_vid = &g_avi_ctxt.list_hdrl_box.list_strl_vid;

                g_avi_ctxt.has_video = true;
                pMain_hdr->dwStreams++;

                pMain_hdr->dwMicroSecPerFrame = 1000000 / pVid_cfg->fps;

                pMain_hdr->dwWidth  = pVid_cfg->width;
                pMain_hdr->dwHeight = pVid_cfg->height;
                // pMain_hdr->dwTotalFrames = ;

                pList = &pList_strl_vid->list_strl;
                SET_LIST(pList, (uint32_t)AVI_FCC_STRL);
                pList->size = sizeof(avi_list_strl_vid_t) - 8;

                {   // strh
                    avi_stream_hdr_box_t    *pStrm_hdr_box = &pList_strl_vid->strh;
                    avi_stream_hdr_t        *pStrm_hdr = &pStrm_hdr_box->stream_hdr;

                    pStrm_hdr_box->tag_strh = (uint32_t)AVI_FCC_STRH;
                    pStrm_hdr_box->size     = sizeof(avi_stream_hdr_t);

                    pStrm_hdr->fccType = (uint32_t)AVI_FCC_VIDS;
                    pStrm_hdr->dwScale = 1;
                    pStrm_hdr->dwRate  = pVid_cfg->fps;
                    // pStrm_hdr->dwLength = total frames
                    if( pVid_cfg->vid_type == AVI_VID_MJPG )
                    {
                        pStrm_hdr->fccHandler = (uint32_t)AVI_FCC_MJPG;
                    }
                }

                {   // strf
                    avi_bmp_info_hdr_box_t      *pStrf = &pList_strl_vid->strf;
                    avi_bmp_info_hdr_t          *pBmp_info = &pStrf->bmp_hdr;

                    pStrf->tag_strf = (uint32_t)AVI_FCC_STRF;
                    pStrf->size     = sizeof(avi_bmp_info_hdr_t);

                    pBmp_info->biSize           = sizeof(avi_bmp_info_hdr_t);
                    pBmp_info->biWidth          = pMain_hdr->dwWidth;
                    pBmp_info->biHeight         = pMain_hdr->dwHeight;
                    pBmp_info->biPlanes         = 1;
                    pBmp_info->biBitCount       = 24;
                    // pBmp_info->biSizeImage      =

                    if( pVid_cfg->vid_type == AVI_VID_MJPG )
                    {
                        pBmp_info->biCompression = BI_JPEG;
                    }

                }
            }

            if( pAud_cfg )
            {
                avi_list_strl_aud_t     *pList_strl_aud = &g_avi_ctxt.list_hdrl_box.list_strl_aud;

                g_avi_ctxt.has_audio = true;
                pMain_hdr->dwStreams++;

                pList = &pList_strl_aud->list_strl;
                SET_LIST(pList, (uint32_t)AVI_FCC_STRL);
                pList->size = sizeof(avi_list_strl_aud_t) - 8;

                {   // strh
                    avi_stream_hdr_box_t    *pStrm_hdr_box = &pList_strl_aud->strh;
                    avi_stream_hdr_t        *pStrm_hdr = &pStrm_hdr_box->stream_hdr;

                    pStrm_hdr_box->tag_strh = (uint32_t)AVI_FCC_STRH;
                    pStrm_hdr_box->size     = sizeof(avi_stream_hdr_t);

                    pStrm_hdr->fccType = (uint32_t)AVI_FCC_AUDS;
                }

                {   // strf
                    avi_wave_fmt_ex_box_t   *pStrf = &pList_strl_aud->strf;
                    avi_wave_fmt_ex_t       *pWav_info = &pStrf->wave_hdr;

                    pStrf->tag_strf = (uint32_t)AVI_FCC_STRF;
                    pStrf->size     = sizeof(avi_wave_fmt_ex_t);
                }
            }

        }

        {   // check data size alignment
            // add AVI_FCC_JUNK or not

            // header_size += ailgnment JUNK size
        }

        pList = &g_avi_ctxt.list_movi;
        SET_LIST(pList, (uint32_t)AVI_FCC_MOVI);

        pRiff->size = header_size - 8;
        g_avi_ctxt.header_size = header_size;

        g_avi_ctxt.is_initialized = true;
    } while(0);

    return rval;
}


int
avi_update_info(uint32_t file_size, uint32_t total_vframe)
{
    return 0;
}

uint32_t
avi_mux_get_header_size()
{
    if( !g_avi_ctxt.is_initialized )
        return (uint32_t)(-1);

    return g_avi_ctxt.header_size;
}

int
avi_mux_get_header(
    uint8_t     *pHeader_buf,
    uint32_t    *pHeader_buf_len)
{
    int         rval = -1;

    if( !g_avi_ctxt.is_initialized )
        return rval;

    do {
        uint32_t    remain_size = *pHeader_buf_len;
        uint8_t     *pCur = pHeader_buf;

        if( g_avi_ctxt.header_size > remain_size )
        {
            break;
        }

        memcpy(pCur, &g_avi_ctxt.riff, sizeof(avi_riff_t));
        pCur += sizeof(avi_riff_t);

        {
            avi_list_hdrl_t     *pList_hdrl = &g_avi_ctxt.list_hdrl_box;

            memcpy(pCur, &pList_hdrl->list_hdrl, sizeof(avi_list_t));
            pCur += sizeof(avi_list_t);

            memcpy(pCur, &pList_hdrl->avih, sizeof(avi_avih_t));
            pCur += sizeof(avi_avih_t);

            if( g_avi_ctxt.has_video )
            {
                memcpy(pCur, &pList_hdrl->list_strl_vid, sizeof(avi_list_strl_vid_t));
                pCur += sizeof(avi_list_strl_vid_t);
            }

            if( g_avi_ctxt.has_audio )
            {
                memcpy(pCur, &pList_hdrl->list_strl_aud, sizeof(avi_list_strl_aud_t));
                pCur += sizeof(avi_list_strl_aud_t);
            }
        }

        // memcpy(pCur, JUNK, alignment);

        g_avi_ctxt.list_movi.size = 10;
        memcpy(pCur, &g_avi_ctxt.list_movi, sizeof(avi_list_t));

        *pHeader_buf_len = pCur - pHeader_buf;
        rval = 0;
    } while(0);

    return rval;
}



