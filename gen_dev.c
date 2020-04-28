/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file gen_dev.c
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
#include "gen_dev.h"

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
gen_dev(char *pOut_dir, item_t *pItems, int item_cnt)
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

        snprintf(pOut_path, 1024, "%s/Kconfig.devices", pOut_dir);

        //------------------------------
        // fit script
        snprintf(g_pScript_buf, SCRIPT_BUF_SIZE,
                 "#!/bin/bash\n"
                 "args=(\"$@\")\n"
                 "echo -e \"#\\n# Automatically generated file; DO NOT EDIT.\\n#\" > %s\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "echo -e \"config ENABLE_CMSIS\\n\\tbool \\\"CMSIS\\\"\\n\\tdefault y if (CPU_ARM_CM4 || CPU_ARM_CM0)\\n\\tdefault n\\n\\t---help---\\n\\t\\t"
                 "The CMSIS is a vendor-independent hardware abstraction layer for\\n\\t\\tmicrocontrollers that are based on Arm Cortex processors.\\n\" >> %s\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "if [ $# != 1 ]; then\n"
                 "\techo -e \"choice\\n\\tprompt \\\"Target device\\\"\\n\\t---help---\\n\\t\\tSelect the target device.\\n \" >> %s\n\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "\tfor ((i = 0 ; i < $# ; i++));\n\tdo\n\t\tif [ ${args[$i]} = \"CMSIS\" ]; then\n\t\t\tcontinue\n\t\tfi\n"
                 "\techo -e \"\\tconfig ENABLE_${args[$i]^^}\\n\\t\\tbool \\\"${args[$i]}\\\"\\n\" >> %s\n\tdone\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "\t\techo -e \"endchoice\\n\\nconfig TARGET_DEVICE\\n\\tstring \" >> %s\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "\tfor ((i = 0 ; i < $# ; i++));\n\tdo\n\t\tif [ ${args[$i]} = \"CMSIS\" ]; then\n\t\t\tcontinue\n\t\tfi\n"
                 "\t\techo -e \"\\tdefault ${args[$i]} if ENABLE_${args[$i]^^} \" >> %s\n\tdone\n",
                 pOut_path);

        snprintf(&g_pScript_buf[strlen(g_pScript_buf)], SCRIPT_BUF_SIZE - strlen(g_pScript_buf),
                 "\tfor ((i = 0 ; i < $# ; i++));\n\tdo\n\t\tif [ ${args[$i]} = \"CMSIS\" ]; then\n\t\t\tcontinue\n\t\tfi\n"
                 "\t\tkconfig_path=$(find %s/${args[$i]} -maxdepth 1 -name \"Kconfig\")\n\t\tif [ ! -z ${kconfig_path} ]; then\n"
                 "\t\t\techo -e \"\\n\\nif ENABLE_${args[$i]^^}\\n\\tsource ${kconfig_path}\\nendif\" >> %s\n\t\tfi\n\tdone\nfi\n\n ",
                 pOut_dir, pOut_path);

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