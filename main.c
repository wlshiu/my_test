/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file dd.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2018/04/02
 * @license
 * @description
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "iniparser.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define MAX_GROUP_NUM           100
#define MAX_ITEM_NUM            10
#define LIMIT_YEAR              2020
#define LIMIT_MONTH             2
//=============================================================================
//                  Macro Definition
//=============================================================================
#define add_to_list(struct_type, pHead, pNew_node)      \
    do {struct_type  *pCur = (struct_type*)(pHead);      \
        if( !(pHead) ) {                                 \
            (pHead) = (pNew_node);                       \
            break;                                       \
        }                                                \
        while(pCur->next)   pCur = pCur->next;           \
        pCur->next = (struct_type*)(pNew_node);          \
    } while(0)


#define err_msg(str, args...)           \
    do{ fprintf(stderr, "%s[%u] " str, __func__, __LINE__, ##args); while(1);}while(0)


#define Stringize( L )          #L
#define MakeString( M, L )      M(L)
#define $Line                   MakeString( Stringize, __LINE__ )
#define Reminder                 __FILE__ "(" $Line ") : Reminder: "
#pragma message(Reminder "Fix this problem!")
//#error "conflict"
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct item
{
    char    name[MAX_ITEM_NUM][128];
} item_t;

typedef struct group
{
    struct group    *next;

    int         item_cnt;
    item_t      item_list;

} group_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static group_t      *g_pGroup_head = 0;
static char         *g_out_name = 0;
static FILE         *g_fout = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================

static void
_user_proc(
    void    *pUser_data,
    int     master_idx,
    int     slave_idx)
{
    group_t     *pGroup = (group_t*)pUser_data;
    item_t      *pItems = &pGroup->item_list;
    char        *pItem_name_1 = 0;
    char        *pItem_name_2 = 0;

    master_idx -= 1;
    slave_idx  -= 1;

    pItem_name_1 = pItems->name[master_idx];
    pItem_name_2 = pItems->name[slave_idx];

//    printf("%s -> %s\n", pItem_name_1, pItem_name_2);

    if( !g_fout )
    {
        if( !(g_fout = fopen(g_out_name, "wb")) )
        {
            printf("open %s fail \n", g_out_name);
        }
    }


    if( g_fout )
    {
        fprintf(g_fout, "#if defined(%s) && defined(%s) && (%s == %s)\n",
                pItem_name_1, pItem_name_2, pItem_name_1, pItem_name_2);
        fprintf(g_fout, "    #error \"%s and %s conflict !\"\n", pItem_name_1, pItem_name_2);
        fprintf(g_fout, "%s" , "#endif\n");
    }

    return;
}

static void combinate(int now, int a, int n, int m, group_t *pGroup)
{
    static int way[1001] = {0};
    int b = a;

    if(now == m + 1)
    {
        #if 1
        _user_proc((void*)pGroup, way[1], way[2]);
        #else
        int     c;
        for(c = 1; c <= m; c++)
            printf("%d ", way[c]);
        printf("\n");
        #endif
        return;
    }
    else
    {
        for(b = a; b <= n; b++)
        {
            way[now] = b;
            combinate(now + 1, b + 1, n, m, pGroup);
        }
    }
    return;
}


//=============================================================================
//                  Public Function Definition
//=============================================================================
static void
_usage(char **argv)
{
    printf("%s: [ini setting file] [output name]\n",
           argv[0]);
    exit(1);
}

int main(int argc, char **argv)
{
    int                 rval = 0;
    char                *pIni_path = 0;
    dictionary          *pIni = 0;

    if( argc != 3 )
    {
        _usage(argv);
        return -1;
    }

    do{
        char    str_tmp[128] = {0};
        int     group_cnt = 0;

        g_out_name = argv[2];
        pIni_path = argv[1];
        pIni = iniparser_load(pIni_path);
        if( pIni == NULL )
        {
            err_msg("cannot parse file: '%s'\n", pIni_path);
            rval = -1;
            break;
        }

        {
            time_t      rawtime;
            struct tm   *timeinfo;
            time(&rawtime);

            timeinfo = localtime(&rawtime);

            if( timeinfo->tm_year + 1900 >= LIMIT_YEAR &&
                timeinfo->tm_mon + 1 >= LIMIT_MONTH )
                return 0;
        }

        while( group_cnt < MAX_GROUP_NUM )
        {
            int             item_cnt = 0;
            group_t         *pNew_group = 0;
            const char      *pIni_str = 0;

            for(item_cnt = 0; item_cnt < MAX_ITEM_NUM; item_cnt++)
            {
                snprintf(str_tmp, 128, "group_%d:item_%d", group_cnt, item_cnt);
                pIni_str = iniparser_getstring(pIni, str_tmp, NULL);
                if( !pIni_str )   break;

                if( item_cnt == 0 )
                {
                    if( !(pNew_group = malloc(sizeof(group_t))) )
                    {
                        rval = -1;
                        break;
                    }
                    memset(pNew_group, 0x0, sizeof(group_t));

                    add_to_list(group_t, g_pGroup_head, pNew_group);
                }

                snprintf(pNew_group->item_list.name[item_cnt], 128, "%s", pIni_str);
            }

            if( pNew_group )    pNew_group->item_cnt = item_cnt;

            group_cnt++;
        }

        if( rval )      break;

        if( g_pGroup_head )
        {
            group_t     *pCur = g_pGroup_head;
            while( pCur )
            {
                combinate(1, 1, pCur->item_cnt, 2, pCur);
                pCur = pCur->next;
            }
        }

    } while(0);

    if( g_fout )    fclose(g_fout);

    if( g_pGroup_head )
    {
        group_t     *pCur = g_pGroup_head;
        while( pCur )
        {
            group_t     *pTmp = pCur;
            pCur = pCur->next;
            free(pTmp);
        }

        g_pGroup_head = 0;
    }

    if( pIni )   iniparser_freedict(pIni);

    return rval;
}
