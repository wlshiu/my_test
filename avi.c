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
#define stringize(s)    #s
#define to_str(s)       stringize(s)

#define SET_LIST(pStruct, fourCC)   \
        do{ (pStruct)->tag_list = (uint32_t)AVI_FCC_LIST; \
            (pStruct)->fcc = (fourCC);                    \
        }while(0)


#define _assert(expression)  \
    ((void)((expression) ? 0 : printf("%s[%u] err '%s'\n", __func__, __LINE__, to_str(expression))))

#define log_fcc(tagfcc) \
    printf("%c%c%c%c\n", ((tagfcc) & 0xFF), ((tagfcc) & 0xFF00) >> 8, ((tagfcc) & 0xFF0000) >> 16, ((tagfcc) & 0xFF000000) >> 24);
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct avi_list_odml
{
    avi_list_t              list_odml;
    uint32_t                reserved;
} avi_list_odml_t;

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

    uint32_t                has_junk;
    avi_chunk_t             junk;

    uint32_t                header_size;
    uint32_t                align_pow2_num;
} avi_mux_ctxt_t;

typedef struct avi_comm_header
{
    avi_riff_t              riff;
    avi_list_t              list_hdrl;
    avi_avih_t              avih;
} avi_comm_header_t;
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
/**
 *  \brief      new a avi header
 *
 *  \param [in] pVid_cfg            video configuration
 *  \param [in] pAud_cfg            audio configuration
 *  \param [in] align_pow2_num      alignment number (2 power align_pow2_num)
 *  \return
 *
 *  \details
 */
int
avi_mux_reset_header(
    avi_video_cfg_t   *pVid_cfg,
    avi_audio_cfg_t   *pAud_cfg,
    uint32_t          align_pow2_num)
{
    int     rval = 0;

    // reset header
    memset(&g_avi_ctxt, 0x0, sizeof(avi_mux_ctxt_t));

    align_pow2_num = (align_pow2_num < 8) ? 8 : align_pow2_num;
    g_avi_ctxt.align_pow2_num = align_pow2_num;

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

            header_size = sizeof(avi_riff_t) + 8 + pList->size;

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

                // update info after stopping:
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

                    // update info after stopping:
                    // pStrm_hdr->dwLength = total frames

                    if( pVid_cfg->vcodec == AVI_CODEC_MJPG )
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
                    pBmp_info->biSizeImage      = ((pBmp_info->biWidth * (pBmp_info->biBitCount >> 3) + 3) & ~0x3) * pBmp_info->biHeight;

                    if( pVid_cfg->vcodec == AVI_CODEC_MJPG )
                    {
                        pBmp_info->biCompression = (uint32_t)AVI_FCC_MJPG;
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

        pList = &g_avi_ctxt.list_movi;
        SET_LIST(pList, (uint32_t)AVI_FCC_MOVI);
        pList->size = 4;

        header_size += sizeof(avi_list_t);

        {   // check data size alignment, add AVI_FCC_JUNK or not
            uint32_t    alignment = (1 << align_pow2_num) - 1;
            uint32_t    padding_size = alignment + 1 - (header_size & alignment) - 8;

            if( (g_avi_ctxt.has_junk = padding_size) )
            {
                avi_chunk_t     *pJunk = &g_avi_ctxt.junk;

                pJunk->fcc  = (uint32_t)AVI_FCC_JUNK;
                pJunk->size = padding_size;

                header_size += (sizeof(avi_chunk_t) + padding_size);
            }
        }

        pRiff->size = header_size - 8;
        g_avi_ctxt.header_size = header_size;

        g_avi_ctxt.is_initialized = true;
    } while(0);

    return rval;
}


int
avi_mux_update_info(
    avi_update_info_t   *pUpdate_info)
{
    _assert(pUpdate_info != 0);

    if( !g_avi_ctxt.is_initialized )
        return -1;

    g_avi_ctxt.riff.size                                            = pUpdate_info->total_file_size;
    g_avi_ctxt.list_hdrl_box.avih.main_hdr.dwTotalFrames            = pUpdate_info->total_frames;
    g_avi_ctxt.list_hdrl_box.list_strl_vid.strh.stream_hdr.dwLength = pUpdate_info->total_frames;
    g_avi_ctxt.list_movi.size                                       = pUpdate_info->media_data_size;
    return 0;
}

int
avi_mux_add_frame(
    avi_frm_type_t  frm_type,
    uint8_t         *pFrm_buf,
    uint32_t        frm_len)
{
    int         rval = 0;

    // callback to write frame ???

    if( frm_type == AVI_FRM_VIDEO )
    {   // video frame AVI_FCC_00DC

    }
    else
    {   // audio frame AVI_FCC_00WB

    }
    return rval;
}

uint32_t
avi_mux_get_header_size()
{
    if( !g_avi_ctxt.is_initialized )
        return (uint32_t)(-1);

    return g_avi_ctxt.header_size;
}

/**
 *  \brief      Generate a binary row of avi header
 *
 *  \param [in] pHeader_buf         buffer for output binary avi header
 *  \param [in] pHeader_buf_len     buffer length
 *  \return
 *
 *  \details
 */
int
avi_mux_gen_header(
    uint8_t     *pHeader_buf,
    uint32_t    *pHeader_buf_len)
{
    int         rval = -1;

    _assert(pHeader_buf != 0);
    _assert(pHeader_buf_len != 0);
    _assert(*pHeader_buf_len != 0);

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

        if( g_avi_ctxt.has_junk )
        {
            memcpy(pCur, &g_avi_ctxt.junk, sizeof(avi_chunk_t));
            pCur += sizeof(avi_chunk_t);

            memset(pCur, 0xAA, g_avi_ctxt.junk.size);
            pCur += g_avi_ctxt.junk.size;
        }

        memcpy(pCur, &g_avi_ctxt.list_movi, sizeof(avi_list_t));
        pCur += sizeof(avi_list_t);

        *pHeader_buf_len = pCur - pHeader_buf;
        rval = 0;
    } while(0);

    return rval;
}

/**
 *  \brief      parse a binary row of avi header
 *
 *  \param [in] pHeader_buf         input buffer of binary row of avi header
 *  \param [in] header_buf_len      input buffer length (it MUST more than 512 bytes)
 *  \return
 *
 *  \details
 */
int
avi_parse_header(
    uint32_t    *pHeader_buf,
    uint32_t    header_buf_len,
    uint32_t    *pMovi_offset)
{
    int     rval = 0;

    _assert(pHeader_buf != 0);
    _assert(header_buf_len >= 512);

    do {
        uint32_t            align_pow2_num = 0;
        uint8_t             *pCur = (uint8_t*)pHeader_buf;
        // uint32_t            *pBuf_end = pHeader_buf + header_buf_len;
        avi_comm_header_t   avi_comm_hdr = {0};
        avi_comm_header_t   *pAvi_comm_hdr = 0;
        avi_list_t          *pList = 0;
        avi_main_hdr_t      *pMain_hdr = 0;
        uint32_t            verified_size = 0;
        uint32_t            movi_offset = 0;

        #if 1
        memcpy(&avi_comm_hdr, pHeader_buf, sizeof(avi_comm_header_t));
        pAvi_comm_hdr = &avi_comm_hdr;
        #else
        pAvi_comm_hdr = pHeader_buf;
        #endif

        {   // RIFF
            avi_riff_t          *pRiff = &pAvi_comm_hdr->riff;
            if( pRiff->tag_riff != (uint32_t)AVI_FCC_RIFF ||
                pRiff->fcc != (uint32_t)AVI_FCC_AVI )
            {
                rval = -1;
                break;
            }

            verified_size += sizeof(avi_riff_t);
        }

        pList = &pAvi_comm_hdr->list_hdrl;
        if( pList->tag_list != (uint32_t)AVI_FCC_LIST ||
            pList->fcc != (uint32_t)AVI_FCC_HDRL )
        {
            rval = -1;
            break;
        }
        verified_size += sizeof(avi_list_t);

        {   // avih
            avi_avih_t          *pAvih = &pAvi_comm_hdr->avih;

            pMain_hdr = &pAvih->main_hdr;
            if( pAvih->tag_avih != (uint32_t)AVI_FCC_AVIH ||
                pMain_hdr->dwStreams == 0 )
            {
                rval = -1;
                break;
            }

            verified_size += sizeof(avi_avih_t);
        }

        pCur += sizeof(avi_comm_header_t);

        {   // check strh/strf exist or not
            int             stream_cnt = 0;
            avi_list_t      *pCur_list = (avi_list_t*)pCur;

            do {
                avi_stream_hdr_box_t    *pStrh = (avi_stream_hdr_box_t*)((uint32_t)pCur_list + sizeof(avi_list_t));
                avi_stream_hdr_t        *pStrm_hdr = &pStrh->stream_hdr;

                if( pCur_list->tag_list != (uint32_t)AVI_FCC_LIST ||
                    pCur_list->fcc != (uint32_t)AVI_FCC_STRL )
                {
                    rval = -1;
                    break;
                }

                if( pStrh->tag_strh != (uint32_t)AVI_FCC_STRH &&
                    (pStrm_hdr->fccType != (uint32_t)AVI_FCC_AUDS &&
                     pStrm_hdr->fccType != (uint32_t)AVI_FCC_VIDS) )
                {
                    rval = -1;
                    break;
                }

                if( *((uint32_t*)((uint32_t)pStrm_hdr + pStrh->size)) != (uint32_t)AVI_FCC_STRF )
                {
                    rval = -1;
                    break;
                }

                pCur_list = (avi_list_t*)((uint32_t)pCur_list + 8 + pCur_list->size);

            } while( ++stream_cnt < pMain_hdr->dwStreams );

            if( rval )      break;
        }

        align_pow2_num = g_avi_ctxt.align_pow2_num;
        memset(&g_avi_ctxt, 0x0, sizeof(avi_mux_ctxt_t));
        g_avi_ctxt.align_pow2_num = align_pow2_num;

        memcpy(&g_avi_ctxt, pHeader_buf, verified_size);

        {   // strl
            int             stream_cnt = 0;
            avi_list_t      *pCur_list = (avi_list_t*)pCur;

            do {
                avi_stream_hdr_box_t    *pStrh = (avi_stream_hdr_box_t*)((uint32_t)pCur_list + sizeof(avi_list_t));
                avi_stream_hdr_t        *pStrm_hdr = &pStrh->stream_hdr;

                if( pStrm_hdr->fccType == (uint32_t)AVI_FCC_VIDS )
                {
                    avi_bmp_info_hdr_box_t  *pStrf = &g_avi_ctxt.list_hdrl_box.list_strl_vid.strf;
                    avi_list_strl_vid_t     *pList_strl_v = &g_avi_ctxt.list_hdrl_box.list_strl_vid;

                    g_avi_ctxt.has_video = true;
                    memcpy(pList_strl_v, pCur_list, sizeof(avi_list_t) + sizeof(avi_stream_hdr_box_t));

                    // update list size
                    pList_strl_v->list_strl.size = sizeof(avi_list_strl_vid_t) - 8;

                    memcpy(pStrf, (void*)((uint32_t)pStrm_hdr + pStrh->size), sizeof(avi_bmp_info_hdr_box_t));

                    verified_size += (sizeof(avi_list_t) + sizeof(avi_stream_hdr_box_t) + sizeof(avi_bmp_info_hdr_box_t));
                }
                else if( pStrm_hdr->fccType == (uint32_t)AVI_FCC_AUDS )
                {
                    avi_wave_fmt_ex_box_t   *pStrf = &g_avi_ctxt.list_hdrl_box.list_strl_aud.strf;
                    avi_list_strl_aud_t     *pList_strl_a = &g_avi_ctxt.list_hdrl_box.list_strl_aud;

                    g_avi_ctxt.has_audio = true;
                    memcpy(pList_strl_a, pCur_list, sizeof(avi_list_t) + sizeof(avi_stream_hdr_box_t));

                    // update list size
                    pList_strl_a->list_strl.size = sizeof(avi_list_strl_aud_t) - 8;

                    memcpy(pStrf, (void*)((uint32_t)pStrm_hdr + pStrh->size), sizeof(avi_wave_fmt_ex_box_t));
                    pStrf->size = sizeof(avi_wave_fmt_ex_t);

                    verified_size += (sizeof(avi_list_t) + sizeof(avi_stream_hdr_box_t) + sizeof(avi_wave_fmt_ex_box_t));
                }
                else
                {
                    // un-support type
                }

                pCur_list = (avi_list_t*)((uint32_t)pCur_list + 8 + pCur_list->size);

            } while( ++stream_cnt < pMain_hdr->dwStreams );

            pCur = (uint8_t*)pCur_list;
        }

        // update hdrl size
        pList = &g_avi_ctxt.list_hdrl_box.list_hdrl;
        pList->size = 4 + sizeof(avi_avih_t);
        if( g_avi_ctxt.has_video )  pList->size += sizeof(avi_list_strl_vid_t);
        if( g_avi_ctxt.has_audio )  pList->size += sizeof(avi_list_strl_aud_t);


        {   // find movi list
            #if 0
            avi_list_t      *pCur_list = &pAvi_comm_hdr->list_hdrl;
            #else
            avi_list_t      *pCur_list = (avi_list_t*)((uint32_t)pHeader_buf + sizeof(avi_riff_t));
            #endif // 0

            pList = &g_avi_ctxt.list_movi;

            do {
                if( pCur_list->tag_list == (uint32_t)AVI_FCC_JUNK )
                {
                    pCur_list = (avi_list_t*)((uint32_t)pCur_list + 8 + pCur_list->size);
                    continue;
                }

                if( pCur_list->tag_list != (uint32_t)AVI_FCC_LIST )
                {
                    // not avi list structure
                    SET_LIST(pList, (uint32_t)AVI_FCC_MOVI);
                    pList->size = 4;
                    break;
                }

                if( pCur_list->fcc == (uint32_t)AVI_FCC_MOVI )
                {
                    movi_offset = (uint32_t)pCur_list - (uint32_t)pHeader_buf + sizeof(avi_list_t);
                    memcpy(pList, pCur_list, sizeof(avi_list_t));
                    break;
                }

                pCur_list = (avi_list_t*)((uint32_t)pCur_list + 8 + pCur_list->size);

            } while(1);

            verified_size += sizeof(avi_list_t);
        }

        {   // check data size alignment, add AVI_FCC_JUNK or not
            uint32_t    alignment = (1 << align_pow2_num) - 1;
            uint32_t    padding_size = alignment + 1 - (verified_size & alignment) - 8;

            if( (g_avi_ctxt.has_junk = padding_size) )
            {
                avi_chunk_t     *pJunk = &g_avi_ctxt.junk;

                pJunk->fcc  = (uint32_t)AVI_FCC_JUNK;
                pJunk->size = padding_size;

                verified_size += (sizeof(avi_chunk_t) + padding_size);
            }
        }

        g_avi_ctxt.header_size    = verified_size;
        g_avi_ctxt.is_initialized = true;

        if( pMovi_offset )     *pMovi_offset = movi_offset;

    } while(0);

    return rval;
}


int
avi_demux_media_data()
{
    return 0;
}

