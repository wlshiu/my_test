/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/09/01
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_INODE_MAX_NUM            10
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct in_node
{
    int     offset;
    char    path[256];
} in_node_t;

typedef struct info_mgr
{
    uint32_t    default_value;
    uint32_t    bin_size;

    char        out_path[256];

    int         in_node_cnt;
    in_node_t   in_nodes[CONFIG_INODE_MAX_NUM];

} info_mgr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static info_mgr_t       g_info_mgr = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void _usage(char **argv)
{
    printf("usage: %s [options]\n"
            "options:\n"
            "  --def_val    the default value (uint8_t type)\n"
            "  --size       the bytes of this bin\n"
            "  --out        the output path\n\n"
            "  --in_file    the inserted file path\n"
            "  --offset     this offset bytes of the inserted file\n"
            "\n", argv[0]);

    exit(-1);
    while(1) {}
}

static int
_parse_argv(int argc, char **argv)
{
    int     mark = 0;

    if( argc < 2 ) {
        _usage(argv);
        return -1;
    }

    argv++; argc--;
    while( argc ) {
        in_node_t   *pIn_node = 0;

        if (!strncmp(argv[0], "--def_val", strlen("--def_val"))) {
            argv++; argc--;
            g_info_mgr.default_value = (*(argv[0] + 1) == 'x')
                                     ? strtol(argv[0], 0, 16)
                                     : strtol(argv[0], 0, 10);

            g_info_mgr.default_value &= 0xFF;

        } else if (!strncmp(argv[0], "--size", strlen("--size"))) {
            argv++; argc--;
            g_info_mgr.bin_size = (*(argv[0] + 1) == 'x')
                                     ? strtol(argv[0], 0, 16)
                                     : strtol(argv[0], 0, 10);

        } else if (!strncmp(argv[0], "--out", strlen("--out"))) {
            argv++; argc--;
            snprintf((char*)&g_info_mgr.out_path, sizeof(g_info_mgr.out_path), "%s", argv[0]);

        } else if (!strncmp(argv[0], "--in_file", strlen("--in_file"))) {
            argv++; argc--;

            pIn_node = &g_info_mgr.in_nodes[g_info_mgr.in_node_cnt];

            snprintf(pIn_node->path, sizeof(pIn_node->path), "%s", argv[0]);

            if( mark == 0 || mark == 1 )
                mark++;

        } else if (!strncmp(argv[0], "--offset", strlen("--offset"))) {
            argv++; argc--;

            pIn_node = &g_info_mgr.in_nodes[g_info_mgr.in_node_cnt];

            pIn_node->offset = (*(argv[0] + 1) == 'x')
                             ? strtol(argv[0], 0, 16)
                             : strtol(argv[0], 0, 10);

            if( mark == 0 || mark == 1 )
                mark++;

        } else {
            mark = 0;
        }

        if( mark == 2 )
        {
            g_info_mgr.in_node_cnt++;
            mark = 0;
        }

        argv++; argc--;
    }

    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    int         rval = 0;
    uint8_t     *pData = 0;
    FILE        *fout = 0;
    FILE        *fin = 0;

    do {
        memset(&g_info_mgr, 0x0, sizeof(g_info_mgr));

        rval = _parse_argv(argc, argv);
        if( rval )  break;

        if( !g_info_mgr.bin_size )
        {
            rval = -1;
            printf("No Size !\n");
            break;
        }

        if( !(pData = malloc(g_info_mgr.bin_size)) )
        {
            rval = -2;
            printf("malloc %d fail\n", g_info_mgr.bin_size);
            break;
        }

        if( !(fout = fopen(g_info_mgr.out_path, "wb")) )
        {
            rval = -3;
            printf("open %s fail \n", g_info_mgr.out_path);
            break;
        }

        memset(pData, g_info_mgr.default_value & 0xFF, g_info_mgr.bin_size);

        for(int i = 0; i < g_info_mgr.in_node_cnt; i++)
        {
            in_node_t   *pIn_node = 0;
            int         fsize = 0;

            pIn_node = &g_info_mgr.in_nodes[i];

            if( !(fin = fopen(pIn_node->path, "rb")) )
            {
                rval = -4;
                printf("open %s fail \n", pIn_node->path);
                break;
            }

            fseek(fin, 0, SEEK_END);
            fsize = ftell(fin);
            fseek(fin, 0, SEEK_SET);

            if( (pIn_node->offset + fsize) > g_info_mgr.bin_size )
            {
                rval = -5;
                printf("Out the expected bin size\n");
                break;
            }

            fread(&pData[pIn_node->offset], 1, fsize, fin);

            fclose(fin);
            fin = 0;
        }

        if( rval < 0 )  break;

        fwrite(pData, 1, g_info_mgr.bin_size, fout);

    } while(0);

    if( pData )     free(pData);
    if( fout )      fclose(fout);
    if( fin )       fclose(fin);

    system("pause");
    return rval;
}
