/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file gen_ld.c
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
#include "gen_ld.h"

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
gen_ld(char *pOut_dir, item_t *pItems, int item_cnt)
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

        snprintf(pOut_path, 1024, "%s/Kconfig.linkscript", pOut_dir);

        //------------------------------
        // fit script
        snprintf(g_pScript_buf, SCRIPT_BUF_SIZE,
                 "#!/bin/bash\n"
                 "args=(\"$@\")\n"
                 "srctree=$(pwd)\n"
                 "echo -e \"#\\n# Automatically generated file; DO NOT EDIT.\\n#\\n\\n\" > %s\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"menu \\\"Link-Script Configuration\\\"\\n\\n\\tchoice TARGET_LD_FILE\\n\\t\\tprompt \\\"Target Link Script\\\"\\n\\t\\tdefault LD_FILE_CMSIS_GCC_ARM\\n\\t\\t---help---\\n\\t\\t\\tSelect the target link script.\\n\" >> %s\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "for ((i = 0 ; i < $# ; i++));\ndo\n\tparentdir=$(dirname ${args[$i]} | sed 's,^\\(.*/\\)\\?\\([^/]*\\),\\2,')\n\t"
                 "filename=$(basename -- \"${args[$i]}\")\n\t"
                 "filename=${filename%%.*}\n\t"
                 "filename=$(echo ${filename} | sed 's:\\.:_:g')\n\t"
                 "description=$(echo ${args[$i]} | sed 's:'\"${srctree}\"/'::g')\n\t"
                 "echo -e \"\\t\\tconfig LD_FILE_${parentdir^^}_${filename^^}\\n\\t\\t\\tbool \\\"${description}\\\"\\n\" >> %s\ndone\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"\\t\\tconfig LD_FILE_CUSTOMER\\n\\t\\t\\tbool \\\"Custom link script\\\"\\n\\n\\tendchoice\\n\\n\\tconfig TARGET_CUSTOMER_LD_FILE\\n\\t\\tstring \\\"Custom link script file\\\" if LD_FILE_CUSTOMER\\n\\t\\tdefault \\\"\\\"\\n\\t\\t---help---\\n\\t\\t\\te.g. xx/xxx.lds.S\\n\\n\\tconfig TARGET_LD_FILE\\n\\t\\tstring\" >> %s\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "for ((i = 0 ; i < $# ; i++));\ndo\n\tparentdir=$(dirname ${args[$i]} | sed 's,^\\(.*/\\)\\?\\([^/]*\\),\\2,')\n\tfilename=$(basename -- \"${args[$i]}\")\n\tfilename=${filename%%.*}\n\tfilename=$(echo ${filename} | sed 's:\\.:_:g')\n\t"
                 "echo -e \"\\t\\tdefault \\\"${args[$i]}\\\" if LD_FILE_${parentdir^^}_${filename^^}\" >> %s\ndone\n\n"
                 "echo -e \"\\t\\tdefault TARGET_CUSTOMER_LD_FILE if LD_FILE_CUSTOMER\\n\\nendmenu\\n\" >> %s",
                 pOut_path, pOut_path);
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
