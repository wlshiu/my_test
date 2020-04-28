/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file gen_app.c
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
#include "gen_app.h"

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
gen_app(char *pOut_dir, item_t *pItems, int item_cnt)
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

        snprintf(pOut_path, 1024, "%s/Kconfig.app", pOut_dir);

        snprintf(g_pScript_buf, SCRIPT_BUF_SIZE,
                 "#!/bin/bash\n"
                 "args=(\"$@\")\n"
                 "echo -e \"#\\n# Automatically generated file; DO NOT EDIT.\\n#\" > %s\n"
                 "echo -e \"choice\\n\tprompt \\\"Target App\\\"\\n\\tdefault APP_HELLO_WORLD\\n\\t---help---\\n\\t\\tSelect the target application.\\n\" >> %s\n",
                 pOut_path, pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "for ((i = 0 ; i < $# ; i++));\ndo\n\techo -e \"\\tconfig APP_${args[$i]^^}\\n\\t\\tbool \\\"${args[$i]}\\\"\\n\" >> %s\ndone\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"endchoice\\n\" >> %s\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"config TARGET_APP_PROJECT\\n\\tstring\" >> %s\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "for ((i = 0 ; i < $# ; i++));\ndo\n\techo -e \"\\tdefault ${args[$i]} if APP_${args[$i]^^}\" >> %s\ndone\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"\\n\\n\" >> %s\n"
                 "for ((i = 0 ; i < $# ; i++));\ndo\n\tkconfig_path=$(find %s/${args[$i]} -name \"Kconfig\")\n"
                 "\tif [ ! -z ${kconfig_path} ]; then\n"
                 "\t\techo -e \"if APP_${args[$i]^^}\\n\\tsource ${kconfig_path}\\nendif\\n\" >> %s\n\tfi\ndone\n",
                 pOut_path, pOut_dir, pOut_path);

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
