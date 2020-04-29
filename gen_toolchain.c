/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file gen_toolchain.c
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
#include "gen_toolchain.h"

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
gen_toolchain(char *pOut_dir, item_t *pItems, int item_cnt)
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

        snprintf(pOut_path, 1024, "%s/Kconfig.toolchain", pOut_dir);

        //------------------------------
        // fit script
        snprintf(g_pScript_buf, SCRIPT_BUF_SIZE,
                 "#!/bin/bash\n"
                 "args=(\"$@\")\n"
                 "echo -e \"#\\n# Automatically generated file; DO NOT EDIT.\\n#\" > %s\n\n"
                 "Red='\\e[0;31m'\nYellow='\\e[1;33m'\nGreen='\\e[0;32m'\nNC='\\e[0m'\nset -e\n"
                 "if uname | grep -iq 'linux'; then\n\tPLATFORM='linux'\nelse\n\tPLATFORM='win32'\nfi\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"menu \\\"Tool-Chain Configuration\\\"\\n\\tchoice TARGET_TOOLCHAIN_PATH\\n\\t\\tprompt \\\"Target Tool Chain\\\"\\n\\t\\t---help---\\n\\t\\t\\tSelect the target tool-chain.\\n\\n\\t\\tconfig TOOLCHAIN_HOST_ENVIRONMENT\\n\\t\\t\\tbool \\\"Host Environment\\\"\\n\" >> %s\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "for ((i = 0 ; i < $# ; i++));\ndo\n\tif ! echo \"${args[$i]}\" | grep -iq ${PLATFORM}; then\n\t\tcontinue;\n\tfi\n"
                 "\tfilename=$(basename -- \"${args[$i]}\")\n"
                 "\tdescription=${filename}\n"
                 "\textension=${filename##*.}\n"
                 "\tfilename=${filename%%.*}\n"
                 "\tfilename=$(echo ${filename} | sed 's:-:_:g')\n"
                 "\tfilename=$(echo ${filename} | sed 's:\\.:_:g')\n"
                 "\techo -e \"config TOOLCHAIN_${filename^^}\\n\\tbool \\\"${description}\\\"\\n\" >> %s\ndone\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"\\t\\tconfig TOOLCHAIN_CUSTOMER\\n\\t\\t\\tbool \\\"Custom tool-chain\\\"\\n\\n\\tendchoice\\n\\nconfig TARGET_CUSTOMER_TOOLCHAIN\\n\\tstring \\\"Custom tool-chain path\\\" if TOOLCHAIN_CUSTOMER\\n\\tdefault \\\"\\\"\\n\\t---help---\\n\\t\\tpath of tool-chain\\n\\nconfig TARGET_TOOLCHAIN_PATH\\n\\tstring\\n\\tdefault \\\"\\\" if TOOLCHAIN_HOST_ENVIRONMENT\" >> %s\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "for ((i = 0 ; i < $# ; i++));\ndo\n\tfilename=$(basename -- \"${args[$i]}\")\n\tfilename=${filename%%.*}\n\tfilename=$(echo ${filename} | sed 's:-:_:g')\n\tfilename=$(echo ${filename} | sed 's:\\.:_:g')\n\t"
                 "echo -e \"\\tdefault \\\"${args[$i]}\\\" if TOOLCHAIN_${filename^^}\" >> %s\ndone\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"\\tdefault TARGET_CUSTOMER_TOOLCHAIN if TOOLCHAIN_CUSTOMER\\n\\nendmenu\\n\" >> %s\n",
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

        memset(g_pScript_buf, 0x0, SCRIPT_BUF_SIZE);
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
