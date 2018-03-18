#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avi.h"

#define err(str, args...)       do{printf("%s[%u] " str, __func__, __LINE__, ##args);}while(0)

#define MAX_VFRAME_NUM          30

int main()
{
    FILE                *fout = 0;
    uint32_t            vframe_cnt = 0;
    avi_video_cfg_t     vid_cfg = {0};

    vid_cfg.vcodec   = AVI_CODEC_MJPG;
    vid_cfg.width    = 800;
    vid_cfg.height   = 600;
    vid_cfg.fps      = 10;
    avi_mux_reset_header(&vid_cfg, 0, 9);

    do {
        uint8_t     buf[512] = {0};
        uint32_t    hdr_size = 512;

        memset(buf, 0xFF, hdr_size);
        avi_mux_gen_header(buf, &hdr_size);

        if( !(fout = fopen("./head.avi", "wb")) )
        {
            err("open %s fail\n", "./head.avi");
            break;
        }

        fwrite(buf, 1, hdr_size, fout);
        fclose(fout);
        fout = 0;

        #if 0
        avi_mux_reload_header((uint32_t*)buf, hdr_size);
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
    } while(0);

#if 0
    do { // insert media data
        FILE        *fin = 0;
        char        jpg_name[128] = {0};
        uint32_t    jpg_len = 0;

        snprintf(jpg_name, 128, "./jpgs/%u.jpg", vframe_cnt);
        if( !(fin = fopen(jpg_name, "r")) )
        {
            err("open %s fail \n", jpg_name);
            break;
        }

        fseek(fin, 0, SEEK_END);
        jpg_len = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        if( fin )       fclose(fin);

        vframe_cnt++;
    } while( vframe_cnt < MAX_VFRAME_NUM );
#endif // 0

    if( fout )      fclose(fout);

    return 0;
}
