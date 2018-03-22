/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/03/22
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avi.h"

#include "riff.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define MAX_VFRAME_NUM              30

#define BUF_VFRAME_MAX              (50 << 10)

#define FILENAME_MOVI_PAYLAOD       "movi_payload.tmp"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, args...)           do{printf("%s[%u] " str, __func__, __LINE__, ##args);}while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct chunk
{
    uint32_t    fcc;
    uint32_t    length;
} chunk_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t     g_bs_buf[BUF_VFRAME_MAX] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

static int
_test_reload()
{
    FILE        *fin = 0;
    uint32_t    file_size = 0;
    uint8_t     buf[1 << 20] = {0};

    if( !(fin = fopen("./test.avi", "rb")) )
    {
        err("open %s fail \n", "./test.avi");
        return -1;
    }
    fseek(fin, 0l, SEEK_END);
    file_size = ftell(fin);
    fseek(fin, 0l, SEEK_SET);

    fread(buf, 1, file_size, fin);
    fclose(fin);

    avi_parse_header((uint32_t*)buf, file_size, 0);
    return 0;
}

static int
_test_gen_header()
{
    FILE        *fout = 0;
    uint8_t     buf[512] = {0};
    uint32_t    hdr_size = 512;

    memset(buf, 0xFF, hdr_size);
    avi_mux_gen_header(buf, &hdr_size);

    if( !(fout = fopen("./head.avi", "wb")) )
    {
        err("open %s fail\n", "./head.avi");
        return -1;
    }

    fwrite(buf, 1, hdr_size, fout);
    fclose(fout);
    fout = 0;

    #if 0
    avi_parse_header((uint32_t*)buf, hdr_size, 0);
    memset(buf, 0xFF, hdr_size);
    avi_mux_gen_header(buf, &hdr_size);
    if( !(fout = fopen("./head_reload.avi", "wb")) )
    {
        err("open %s fail\n", "./head.avi");
        break;
    }

    fwrite(buf, 1, hdr_size, fout);
    fclose(fout);
    fout = 0;
    #endif

    return 0;
}


static int
_get_video_source(
    uint8_t     **ppBuf,
    uint32_t    *pBuf_len)
{
    *ppBuf = (uint8_t*)0x878787;
    *pBuf_len = 1111;
    return 0;
}

static int
_get_audio_source(
    uint8_t     **ppBuf,
    uint32_t    *pBuf_len)
{
    *ppBuf = (uint8_t*)0x787878;
    *pBuf_len = 0;
    return 0;
}

static int
_encode_one_vframe(
    uint8_t     *pSrc_buf,
    uint32_t    src_buf_len,
    uint8_t     *pDest_buf,
    uint32_t    *pDest_buf_len)
{
    static uint32_t     frame_cnt = 11;

    int         rval = 0;
    FILE        *fin = 0;

    do{
        char        jpg_name[128] = {0};
        uint32_t    jpg_len = 0;

        // printf("src_buf= x%x, len= %u\n", (uint32_t)pSrc_buf, src_buf_len);

        snprintf(jpg_name, 128, "./jpgs/%u.jpeg", frame_cnt++);
        if( !(fin = fopen(jpg_name, "rb")) )
        {
            rval = -1;
            err("open %s fail \n", jpg_name);
            return rval;
        }

        fseek(fin, 0, SEEK_END);
        jpg_len = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        if( *pDest_buf_len < jpg_len )
        {
            rval = -1;
            break;
        }

        *pDest_buf_len = jpg_len;
        fread(pDest_buf, 1, jpg_len, fin);

    } while(0);

    if( fin )       fclose(fin);

    return rval;
}

static int
_encode_one_aframe(
    uint8_t     *pSrc_buf,
    uint32_t    src_buf_len,
    uint8_t     *pDest_buf,
    uint32_t    *pDest_buf_len)
{
    *pDest_buf_len = 0;
    return 0;
}

static int
_output_avi_file(
    char    *pPath)
{
    int         rval = 0;
    FILE        *fout = 0;

    do {
        int         cnt = 0, out_size = 0;
        FILE        *fin = 0;
        uint8_t     buf[512] = {0};
        uint32_t    length = 512;

        memset(buf, 0xFF, length);
        avi_mux_gen_header(buf, &length);

        if( !(fout = fopen(pPath, "wb")) )
        {
            rval = -1;
            err("open %s fail\n", pPath);
            break;
        }

        fwrite(buf, 1, length, fout);

        if( !(fin = fopen(FILENAME_MOVI_PAYLAOD, "rb")) )
        {
            rval = -1;
            break;
        }

        while( (length = fread(buf, 1, 512, fin)) )
        {
            out_size += length;
            // printf("%d-th, len= %d, %u\n", cnt, length, out_size);
            fwrite(buf, 1, length, fout);
            cnt++;
        }

        fclose(fin);

    } while(0);

    if( fout )      fclose(fout);
    fout = 0;
    return rval;
}

static int
_decode_one_vframe(
    uint8_t     *pSrc_buf,
    uint32_t    src_buf_len,
    uint8_t     *pDest_buf,
    uint32_t    *pDest_buf_len)
{
    int     rval = 0;

    return rval;
}

static void
_test_mux(
    char    *pOut_path)
{
    FILE                *fout = 0;
    avi_video_cfg_t     vid_cfg = {0};

    vid_cfg.vcodec   = AVI_CODEC_MJPG;
    vid_cfg.width    = 800;
    vid_cfg.height   = 600;
    vid_cfg.fps      = 10;
    avi_mux_reset_header(&vid_cfg, 0, 9);

    do {
        uint8_t     *pFrm_buf = 0;
        uint32_t    frm_buf_len = 0;
        uint32_t    bs_buf_len = BUF_VFRAME_MAX;
        uint32_t    vframe_cnt = 0;
        uint32_t    media_total_size = 0;

        if( !(fout = fopen(FILENAME_MOVI_PAYLAOD, "wb")) )
        {
            err("open %s fail \n", FILENAME_MOVI_PAYLAOD);
            break;
        }

        do {
            int         rval = 0;
            chunk_t     *pChunk = 0;
            _get_video_source(&pFrm_buf, &frm_buf_len);

            if( frm_buf_len )
            {
                // video frame
                bs_buf_len = BUF_VFRAME_MAX - 8;
                *((uint32_t*)g_bs_buf) = AVI_FCC_00DB;
                rval = _encode_one_vframe(pFrm_buf, frm_buf_len, &g_bs_buf[8], &bs_buf_len);
                if( rval )  break;

                *((uint32_t*)g_bs_buf + 1) = bs_buf_len;

                if( bs_buf_len & 0x1 )
                {
                    g_bs_buf[8 + bs_buf_len] = 0;
                    bs_buf_len += 1;
                }

                fwrite(g_bs_buf, 1, bs_buf_len + 8, fout);

                media_total_size += bs_buf_len;
            }

            _get_audio_source(&pFrm_buf, &frm_buf_len);
            if( frm_buf_len )
            {
                // audio frame
                bs_buf_len = BUF_VFRAME_MAX - 8;
                *((uint32_t*)g_bs_buf) = AVI_FCC_01WB;
                rval = _encode_one_aframe(pFrm_buf, frm_buf_len, &g_bs_buf[8], &bs_buf_len);
                if( rval )  break;

                *((uint32_t*)g_bs_buf + 1) = bs_buf_len;

                if( bs_buf_len & 0x1 )
                {
                    g_bs_buf[8 + bs_buf_len] = 0;
                    bs_buf_len += 1;
                }

                fwrite(g_bs_buf, 1, bs_buf_len + 8, fout);

                media_total_size += bs_buf_len;
            }

        } while( vframe_cnt++ < MAX_VFRAME_NUM );

        fclose(fout);

        do {
            avi_update_info_t   info = {0};

            if( avi_mux_get_header_size() == (-1) )
            {
                err("%s", "wrong avi header size \n");
                break;
            }

            info.media_data_size = media_total_size;
            info.total_file_size = media_total_size + avi_mux_get_header_size();
            info.total_frames    = vframe_cnt;
            avi_mux_update_info(&info);

            _output_avi_file(pOut_path);

        } while(0);


    } while(0);

    if( fout )      fclose(fout);

    return;
}

static void
_test_demux(
    char    *pFile_path)
{
#define AVI_CACHE_SIZE      512
    FILE    *fin = 0;

    do {
        int         rval = 0;
        uint32_t    bs_buf_len = BUF_VFRAME_MAX;
        uint32_t    section_len = AVI_CACHE_SIZE;
        uint32_t    media_data_offset = 0;

        if( !(fin = fopen(pFile_path, "rb")) )
        {
            err("open %s fail \n", pFile_path);
            break;
        }

        section_len = 10 << 10;
        fread(g_bs_buf, 1, section_len, fin);

        avi_parse_header((uint32_t*)g_bs_buf, section_len, &media_data_offset);
        printf("media offset= x%x\n", media_data_offset);

        // fill bs buffer
        fseek(fin, media_data_offset, SEEK_SET);


//        _decode_one_vframe();

    } while(0);

    if( fin )   fclose(fin);

    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int main()
{
    char    *name = "./out.avi";
    // _test_mux(name);
    _test_demux("./test.avi");

    return 0;
}

