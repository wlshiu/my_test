/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/13
 * @license
 * @description
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "gen_app.h"
#include "gen_dev.h"
#include "gen_img.h"
#include "gen_ld.h"
#include "gen_prebuild.h"
#include "gen_test.h"
#include "gen_toolchain.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum gen_type
{
    GEN_TYPE_UNKNOWN    = 0,
    GEN_TYPE_APP,
    GEN_TYPE_DEV,
    GEN_TYPE_IMG,
    GEN_TYPE_LDS,
    GEN_TYPE_PREBUILD,
    GEN_TYPE_TEST,
    GEN_TYPE_TOOLCHAIN,

} gen_type_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define msg(str, ...)       do { printf("[%s:%u] " str, __func__, __LINE__, ##__VA_ARGS__); } while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static gen_type_t       g_gen_type = GEN_TYPE_UNKNOWN;
static item_t           g_items[100] = {0};
static int              g_item_cnt = 0;
static char             *g_pOut_path = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
usage(char *pProg)
{
    printf("usage: %s [type]\n"
           "    type:\n"
           "        app/dev/img/lds/prebuild/test/toolchain\n",
           pProg);
    exit(-1);
}

static void
_getparams(int argc, char **argv)
{
#if 0
    if (argc != 1 ) {
        usage(argv[0]);
        exit(-1);
    }
#endif // 0

    argv++; argc--;
    while(argc) {
        if (!strncmp(argv[0], "app", strlen("app"))) {
            argv++; argc--;
            g_gen_type = GEN_TYPE_APP;
            break;
        } else if (!strncmp(argv[0], "dev", strlen("dev"))) {
            argv++; argc--;
            g_gen_type = GEN_TYPE_DEV;
            break;
        } else if (!strncmp(argv[0], "img", strlen("img"))) {
            argv++; argc--;
            g_gen_type = GEN_TYPE_IMG;
            break;
        } else if (!strncmp(argv[0], "lds", strlen("lds"))) {
            argv++; argc--;
            g_gen_type = GEN_TYPE_LDS;
            break;
        } else if (!strncmp(argv[0], "prebuild", strlen("prebuild"))) {
            argv++; argc--;
            g_gen_type = GEN_TYPE_PREBUILD;
            break;
        } else if (!strncmp(argv[0], "test", strlen("test"))) {
            argv++; argc--;
            g_gen_type = GEN_TYPE_TEST;
            break;
        } else if (!strncmp(argv[0], "toolchain", strlen("toolchain"))) {
            argv++; argc--;
            g_gen_type = GEN_TYPE_TOOLCHAIN;
            break;
        } else if (!strncmp(argv[0], "copyright", strlen("copyright"))) {
            argv++; argc--;
            printf("Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.\n");
            exit(0);
        } else {
            msg("other options: %s\n", argv[0]);
            argv++; argc--;
        }
    }

    g_item_cnt  = argc - 1;
    g_pOut_path = argv[0];

    for(int i = 1; i < argc; i++)
    {
        snprintf(g_items[i - 1].item_name, sizeof(g_items[i - 1].item_name), "%s", argv[i]);
    }

    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    _getparams(argc, argv);

    do {
        if( g_gen_type == GEN_TYPE_UNKNOWN )
            break;

        switch( g_gen_type )
        {
            case GEN_TYPE_APP:
                gen_app(g_pOut_path, g_items, g_item_cnt);
                break;
            case GEN_TYPE_DEV:
                gen_dev(g_pOut_path, g_items, g_item_cnt);
                break;
            case GEN_TYPE_IMG:
                gen_img(g_pOut_path, g_items, g_item_cnt);
                break;
            case GEN_TYPE_LDS:
                gen_ld(g_pOut_path, g_items, g_item_cnt);
                break;
            case GEN_TYPE_PREBUILD:
                gen_prebuild(g_pOut_path, g_items, g_item_cnt);
                break;
            case GEN_TYPE_TEST:
                gen_test(g_pOut_path, g_items, g_item_cnt);
                break;
            case GEN_TYPE_TOOLCHAIN:
                gen_toolchain(g_pOut_path, g_items, g_item_cnt);
                break;
            default:
                break;
        }

    } while(0);

    return 0;
}
