
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PPMPixel
{
    uint8_t     red;
    uint8_t     green;
    uint8_t     blue;
} PPMPixel;

typedef struct
{
    int         width;
    int         hight;
    PPMPixel    *data;
} PPMImage;

#define CREATOR                 "RPFELGUEIRAS"
#define RGB_COMPONENT_COLOR     255

static PPMImage *ppm_read(const char *filename)
{
    int         rval = 0;
    FILE        *fp = 0;
    PPMImage    *pImg = 0;

    do {
        char    buff[16] = {0};

        // open PPM file for reading
        if( !(fp = fopen(filename, "rb")) )
        {
            rval = -__LINE__;
            printf("Unable to open file '%s'\n", filename);
            break;
        }

        // read image format
        if( !fgets(buff, sizeof(buff), fp) )
        {
            rval = -__LINE__;
            break;
        }

        // check the image format
        if (buff[0] != 'P' || buff[1] != '6')
        {
            printf("Invalid image format (must be 'P6')\n");
            rval = -__LINE__;
            break;
        }

        if( !(pImg = (PPMImage*)malloc(sizeof(PPMImage))) )
        {
            printf("Unable to allocate memory\n");
            rval = -__LINE__;
            break;
        }

        memset(pImg, 0x0, sizeof(PPMImage));

        {
            int     c = 0;

            c = getc(fp);
            while( c == '#' )
            {
                while( getc(fp) != '\n' ) {}

                c = getc(fp);
            }

            ungetc(c, fp);
        }

        // read image resolution information
        if( fscanf(fp, "%d %d", &pImg->width, &pImg->hight) != 2 )
        {
            printf("Invalid image size (error loading '%s')\n", filename);
            rval = -__LINE__;
            break;
        }

        {
            int     color_depth = 0;

            // read RGB component
            if( fscanf(fp, "%d", &color_depth) != 1 )
            {
                printf("Invalid RGB components (error loading '%s')\n", filename);
                rval = -__LINE__;
                break;
            }

            // check color component depth
            if( color_depth != RGB_COMPONENT_COLOR )
            {
                printf("'%s' does not have 8-bits components\n", filename);
                rval = -__LINE__;
                break;
            }
        }


        while( fgetc(fp) != '\n' ) {}

        // memory allocation for pixel data
        if( !(pImg->data = (PPMPixel*)malloc(pImg->width * pImg->hight * sizeof(PPMPixel))) )
        {
            printf("Unable to allocate memory\n");
            rval = -__LINE__;
            break;
        }

        // read pixel data from file
        fread(pImg->data, 1, 3 * pImg->width * pImg->hight, fp);

    } while(0);

    if( fp )    fclose(fp);

    if( rval )
    {
        if( pImg )
        {
            if( pImg->data )    free(pImg->data);
            free(pImg);
        }

        pImg = 0;
    }

    return pImg;
}

static void ppm_write(const char *filename, PPMImage *img)
{
    FILE *fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P6\n");

    //comments
    fprintf(fp, "# Created by %s\n", CREATOR);

    //image size
    fprintf(fp, "%d %d\n", img->width, img->hight);

    // rgb component depth
    fprintf(fp, "%d\n", RGB_COMPONENT_COLOR);

    // pixel data
    fwrite(img->data, 3 * img->width, img->hight, fp);
    fclose(fp);
    return;
}

static void inverse_color(PPMImage *img)
{
    do {
        if( !img )  break;

        for(int i = 0; i < img->width * img->hight; i++)
        {
            img->data[i].red   = RGB_COMPONENT_COLOR - img->data[i].red;
            img->data[i].green = RGB_COMPONENT_COLOR - img->data[i].green;
            img->data[i].blue  = RGB_COMPONENT_COLOR - img->data[i].blue;
        }
    } while(0);


    return;
}

#define RGB_888_TO_565(r, g, b)    \
            ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))

static void convert_rgb565(char *pOut_filename, PPMImage *pImg)
{
    FILE    *fout = 0;
    do {
        if( !pImg || !pOut_filename )
            break;

        // open file for output
        if( !(fout = fopen(pOut_filename, "wb")) )
        {
            printf("Unable to open file '%s'\n", pOut_filename);
            break;
        }

        for(int i = 0; i < pImg->width * pImg->hight; i++)
        {
            uint16_t    rgb565_value = 0;

            rgb565_value = RGB_888_TO_565(pImg->data[i].red, pImg->data[i].green, pImg->data[i].blue);

            fwrite((void*)&rgb565_value, 1, sizeof(rgb565_value), fout);
        }

    } while(0);

    if( fout )  fclose(fout);

    return;
}

static void
_usage(char **argv)
{
    printf("usage: %s <ppm path>\n", argv[0]);
    return;
}

int main(int argc, char **argv)
{
    int         rval = 0;
    PPMImage    *pImage = 0;

    do {
        char    out_path[256] = {0};
        char    *pExt = 0;

        if( argc != 2 )
        {
            _usage(argv);
            rval = -1;
            break;
        }

        pExt = strrchr(argv[1], '.');

        memcpy(out_path, argv[1], pExt - argv[1]);
        strcat(out_path, "_out");

        pImage = ppm_read(argv[1]);
        if( !pImage )   break;

        #if 0
        strcat(out_path, pExt);

        inverse_color(pImage);
        ppm_write(out_path, pImage);
        #else
        snprintf(out_path, sizeof(out_path),
                 "%s_%dx%d_rgb565.raw", out_path, pImage->width, pImage->hight);
        convert_rgb565(out_path, pImage);
        #endif
    } while(0);

    if( pImage )
    {
        if( pImage->data )    free(pImage->data);

        free(pImage);
    }

    if( rval == 0 )     printf("done ~~~\n");

    system("pause");
    return rval;
}
