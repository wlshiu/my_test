#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avi.h"

#define err(str, args...)       do{printf("%s[%u] " str, __func__, __LINE__, ##args);}while(0)


int main()
{
    FILE                *fout = 0;
    avi_video_cfg_t     vid_cfg = {0};

    vid_cfg.vid_type = AVI_VID_MJPG;
    vid_cfg.width    = 800;
    vid_cfg.height   = 600;
    vid_cfg.fps      = 10;
    avi_mux_init_header(&vid_cfg, 0, 0);

    do {
        uint8_t     buf[512] = {0};
        uint32_t    hdr_size = 512;

        memset(buf, 0xFF, hdr_size);
        avi_mux_get_header(buf, &hdr_size);

        if( !(fout = fopen("./head.avi", "wb")) )
        {
            err("open %s fail\n", "./head.avi");
            break;
        }

        fwrite(buf, 1, hdr_size, fout);

    } while(0);

    do { // insert media data

    } while(0);

    if( fout )      fclose(fout);

    return 0;
}
