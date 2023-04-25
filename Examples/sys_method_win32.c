/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_method.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/04/24
 * @license
 * @description
 */

#include <stdio.h>
#include "sys_methods.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_TAG_LIST_MAX     200
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct tag_list
{
    union {
        uint32_t    tag;
        char        ch[4];
    };

    int         subtag;

    int         sz;
} tag_list_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static tag_list_t       g_tag_list[CONFIG_TAG_LIST_MAX] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_win32_malloc(int size, uint32_t tag, int subtag)
{
    for(int i = 0; i < CONFIG_TAG_LIST_MAX; i++)
    {
        if( g_tag_list[i].tag == tag && g_tag_list[i].subtag == subtag )
        {
            printf("tag the same 0x%08X, %d\n", tag, subtag);
            while(1);
        }
    }

    for(int i = 0; i < CONFIG_TAG_LIST_MAX; i++)
    {
        if( g_tag_list[i].tag != 0 )
            continue;

        g_tag_list[i].tag    = tag;
        g_tag_list[i].subtag = subtag;
        g_tag_list[i].sz     = size;
        break;
    }

    return malloc(size);
}

static void
_win32_free(void *ptr, uint32_t tag, int subtag)
{
    for(int i = 0; i < CONFIG_TAG_LIST_MAX; i++)
    {
        if( g_tag_list[i].tag != tag ||
            g_tag_list[i].subtag != subtag )
            continue;

        g_tag_list[i].tag    = 0;
        g_tag_list[i].subtag = 0;
        g_tag_list[i].sz     = 0;
        break;
    }

    return free(ptr);
}


static void
_win32_debug(void)
{
    int     sz = 0;
    printf("\n\n========\n");

    for(int i = 0; i < CONFIG_TAG_LIST_MAX; i++)
    {
        if( g_tag_list[i].tag != 0 )
        {
            sz += g_tag_list[i].sz;

            printf("-- %dth, 4CC=%C%C%C%C, tag= %d, sz = %d /%d\n",
                   i,
                   g_tag_list[i].ch[3], g_tag_list[i].ch[2],
                   g_tag_list[i].ch[1], g_tag_list[i].ch[0],
                   g_tag_list[i].subtag,
                   g_tag_list[i].sz,
                   sz);
        }
    }
    return;
}

static void
_win32_dump_mem(char *prefix, void *pAddr, int length, int dump_type)
{
    FILE    *fout = 0;

    if( dump_type & SYS_MEM_DUMP_TYPE_FILE )
    {
        char    out_name[64] = {0};
        if( prefix )
        {
            snprintf(out_name, sizeof(out_name), "%s_mem.log", prefix);
            fout = fopen(out_name, "w");
        }
    }

    if( dump_type & SYS_MEM_DUMP_TYPE_U8 )
    {
        uint32_t    addr = (uint32_t)pAddr;
        uint8_t     *pCur = (uint8_t*)pAddr;
        for(int i = 0; i < length; i++)
        {
            if( (i & 0xF) == 0 )
            {
                if( fout )  fprintf(fout, "\n%s%08X |", prefix, addr);

                printf("\n%s%08X |", prefix, addr);
                addr += 16;
            }

            printf(" %02X", pCur[i]);
            if( fout )  fprintf(fout, " %02X", pCur[i]);
        }
    }
    else if( dump_type & SYS_MEM_DUMP_TYPE_U32 )
    {
        uint32_t    addr = (uint32_t)pAddr;
        uint32_t    *pCur = (uint32_t*)pAddr;
        for(int i = 0; i < length; i++)
        {
            if( (i & 0x3) == 0 )
            {
                if( fout )  fprintf(fout, "\n%s%08X |", prefix, addr);

                printf("\n%s%08X |", prefix, addr);
                addr += 16;
            }

            printf(" %08X", pCur[i]);
            if( fout )  fprintf(fout, " %08X", pCur[i]);
        }
    }
    else if( dump_type & SYS_MEM_DUMP_TYPE_FLOAT )
    {
        uint32_t    addr = (uint32_t)pAddr;
        float       *pCur = (float*)pAddr;
        for(int i = 0; i < length; i++)
        {
            if( (i & 0x3) == 0 )
            {
                if( fout )  fprintf(fout, "\n%s%08X |", prefix, addr);

                printf("\n%s%08X |", prefix, addr);
                addr += 16;
            }

            printf(" %4.8f", pCur[i]);
            if( fout )  fprintf(fout, " %4.8f", pCur[i]);
        }
    }

    if( fout )  fclose(fout);
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
sys_methods_t       g_sys_methods_win32 =
{
    .cb_malloc   = _win32_malloc,
    .cb_free     = _win32_free,

    .cb_debug    = _win32_debug,
    .cb_dump_mem = _win32_dump_mem,
};

