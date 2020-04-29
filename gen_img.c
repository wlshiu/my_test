/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file gen_img.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/13
 * @license
 * @description
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gen_img.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static char     *g_pScript_buf = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
gen_img(char *pOut_dir, item_t *pItems, int item_cnt)
{
    int     rval = 0;
    char    *pOut_path = 0;
    char    *pAct_script = "act_doing.sh";
    char    path[1024] = {0};

    do {
        if( !(pOut_path = malloc(1024)) )
        {
            rval = -__LINE__;
            break;
        }
        memset(pOut_path, 0x0, 1024);

        if( !(g_pScript_buf = malloc(SCRIPT_BUF_SIZE)) )
        {
            rval = -__LINE__;
            break;
        }
        memset(g_pScript_buf, 0x0, SCRIPT_BUF_SIZE);

        snprintf(pOut_path, 1024, "%s/Kconfig.imgs", pOut_dir);

        //------------------------------
        // fit script
        snprintf(g_pScript_buf, SCRIPT_BUF_SIZE,
                 "#!/bin/bash\n"
                 "args=(\"$@\")\n"
                 "echo -e \"#\\n# Automatically generated file; DO NOT EDIT.\\n#\\n\\nconfig CORE_IMG_NUM\\n\\tint\\n\\tdefault $(($#))\\n\\n\" > %s\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "for ((i = 1 ; i < $(($# + 1)) ; i++));\ndo\n\t"
                 "relative_path=$(echo ${args[$(($i - 1))]} | sed 's:'\"%s/\"'::g')\n\t"
                 "echo -e \"config INSERT_CORE_IMAGE_$i\\n\\tbool \\\"Insert core image $i - ${relative_path}\\\"\\n\\tdefault n\\n\\n\" >> %s\n",
                 pOut_dir, pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "\techo -e \"\\tconfig IMAGE_NAME_$i\\n\\t\\tstring \\\"image path\\\"\\n\\t\\tdepends on INSERT_CORE_IMAGE_$i\\n\\t\\tdefault \\\"${relative_path}\\\"\\n\" >> %s\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "\techo -e \"\\tconfig SECTION_NAME_$i\\n\\t\\tstring \\\"section name\\\"\\n\\t\\tdepends on INSERT_CORE_IMAGE_$i\\n\\t\\tdefault \\\".img_$i\\\"\\n\\n\" >> %s\ndone\n",
                 pOut_path);
        //-----------------------------
        // output
        {
            FILE    *fout = 0;

            snprintf(path, sizeof(path), "%s/%s", pOut_dir, pAct_script);
            if( !(fout = fopen(path, "wb")) )
            {
                break;
            }

            fwrite(g_pScript_buf, 1, strlen(g_pScript_buf), fout);
            fclose(fout);
        }

        snprintf(g_pScript_buf, SCRIPT_BUF_SIZE, "bash %s", path);
        for(int i = 0; i < item_cnt; i++)
        {
            snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf), " %s", pItems[i].item_name);
        }

       system(g_pScript_buf);

       remove(path);
    } while(0);

    if( pOut_path )         free(pOut_path);

    if( g_pScript_buf )     free(g_pScript_buf);
    g_pScript_buf = 0;

    return rval;
}
