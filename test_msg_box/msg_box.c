/**
 * Copyright (c) 2015 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file msg_box.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2015/08/21
 * @license
 * @description
 */


#include "msg_box.h"

#if (_MSC_VER)
    #define _CRTDBG_MAP_ALLOC
#endif

#include "util_def.h"
#include <string.h>

#if (_MSC_VER)
    #include <crtdbg.h>
#endif

#if 0
    #include <pthread.h>
#else
    typedef int     pthread_mutex_t;
    #define PTHREAD_MUTEX_INITIALIZER   0
    #define pthread_mutex_lock(m)
    #define pthread_mutex_unlock(m)
#endif
//=============================================================================
//                Constant Definition
//=============================================================================
#define MAX_MSG_QUEUE_NUM       10
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 *  message box
 */
typedef struct msg_box
{
    struct msg_box      *next;

    uint32_t    type;
    msg_node_t  *pMsg_node;

    int     (*destroy_node)(msg_node_t **ppMsg_node);
    int     (*handle_node)(msg_node_t *pMsg_node);

    void    *pTunnel_info[2];
} msg_box_t;
//=============================================================================
//                Global Data Definition
//=============================================================================
static msg_box_t       *g_pHead_msg_box = 0;
static msg_box_t       *g_pCur_msg_box = 0;
static int              g_msg_box_cnt = 0;

static pthread_mutex_t  g_mutex_msgbox = PTHREAD_MUTEX_INITIALIZER;
//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
int
msg_box_post_node(
    msg_info_t  *pMsg_info)
{
    int     result = -1;

    do {
        msg_box_t   *pCur_msg_box = 0;

        if( pMsg_info->create_node )
            pMsg_info->create_node(pMsg_info);

        if( !pMsg_info->pMsg_node )
            break;

        if( !(pCur_msg_box = malloc(sizeof(msg_box_t))) )
        {
            err_msg("malloc fail !!\n");
            break;
        }
        memset(pCur_msg_box, 0x0, sizeof(msg_arg_t));

        pCur_msg_box->type         = pMsg_info->type;
        pCur_msg_box->pMsg_node    = pMsg_info->pMsg_node;
        pCur_msg_box->destroy_node = pMsg_info->destroy_node;
        pCur_msg_box->handle_node  = pMsg_info->handle_node;

        pthread_mutex_lock(&g_mutex_msgbox);
        if( g_pHead_msg_box == 0 )
        {
            g_pHead_msg_box = g_pCur_msg_box = pCur_msg_box;
        }
        else
        {
            g_pCur_msg_box->next = pCur_msg_box;
            g_pCur_msg_box = g_pCur_msg_box->next;
        }
        g_msg_box_cnt++;
        result = 0;
        pthread_mutex_unlock(&g_mutex_msgbox);
    } while(0);

    return result;
}


int
msg_box_fetch_node(
    msg_node_t   **ppMsg_node)
{
    int     result = -1;

    do {
        msg_box_t   *pCur_msg_box = 0;

        if( ppMsg_node == 0 )
        {
            dbg_msg("Null Pointer !!\n");
            break;
        }

        *ppMsg_node = 0;

        pthread_mutex_lock(&g_mutex_msgbox);
        pCur_msg_box = g_pHead_msg_box;
        if( g_msg_box_cnt > 0 && pCur_msg_box )
        {
            *ppMsg_node = pCur_msg_box->pMsg_node;
            g_pHead_msg_box = pCur_msg_box->next;
            g_pCur_msg_box = (g_pHead_msg_box) ? g_pCur_msg_box : 0;

            free(pCur_msg_box);
            g_msg_box_cnt--;
        }

        result = 0;
        pthread_mutex_unlock(&g_mutex_msgbox);
    } while(0);

    return result;
}

int
msg_box_erase_all(void)
{
    int     result = -1;

    do {
        msg_box_t  *pCur_box = 0;

        pthread_mutex_lock(&g_mutex_msgbox);
        pCur_box = g_pHead_msg_box;
        while( pCur_box )
        {
            msg_box_t   *pTmp_box = pCur_box;
            if( pCur_box->destroy_node )
                pCur_box->destroy_node(&pCur_box->pMsg_node);

            pCur_box = pCur_box->next;
            free(pTmp_box);
        }

        g_msg_box_cnt = 0;
        g_pHead_msg_box = g_pCur_msg_box = 0;

        result = 0;
        pthread_mutex_unlock(&g_mutex_msgbox);
    } while(0);

    return result;
}


