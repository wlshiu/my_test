/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file virtual_phy.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/06
 * @license
 * @description
 */

#include <stdio.h>
#include <windows.h>
#include <fcntl.h>
#include <time.h>
#include "virtual_phy.h"
#include "rbi.h"
#include "log.h"
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
static pthread_t        g_task_vphy;

#if 0
static char             *g_pVPhy_qname = "/vphy_q";
static mqd_t            g_vphy_q;
#else
static rbi_t            g_vphy_q;
#endif

static vphy_dataq_t     *g_pDataq_pool = 0;

static uint32_t         g_vphy_buf[CONFIG_VPHY_MSG_SIZE >> 2] = {0};
static int              g_vphy_is_running = 0;

static pthread_cond_t   g_vphy_cond;
static pthread_mutex_t  g_vphy_mtx;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_task_vphy_routine(void *argv)
{
    int             rval = 0;
    vphy_dataq_t    **ppDataq = 0;

    if( !(ppDataq = malloc(sizeof(vphy_dataq_t*) * CONFIG_VPHY_NODE_MAX)) )
    {
        pthread_exit(0);
        return 0;
    }

    memset(ppDataq, 0x0, sizeof(vphy_dataq_t*) * CONFIG_VPHY_NODE_MAX);
    for(int i = 0; i < CONFIG_VPHY_NODE_MAX; i++)
    {
        ppDataq[i] = &g_pDataq_pool[i];
    }

    pthread_mutex_lock(&g_vphy_mtx);
    pthread_cond_signal(&g_vphy_cond);
    pthread_mutex_unlock(&g_vphy_mtx);

    while( g_vphy_is_running )
    {
        vphy_package_t      *pPackage = 0;

        rval = rbi_pop(g_vphy_q, (uint8_t*)g_vphy_buf, sizeof(g_vphy_buf));
        if( !rval )  continue;

        pPackage = (vphy_package_t*)&g_vphy_buf;

    #if 1
        pthread_mutex_lock(&g_vphy_mtx);

        for(int i = 0; i < CONFIG_VPHY_NODE_MAX; i++)
        {
            int    package_size = 0;

            if( !g_pDataq_pool[i].cb_isr ||
                pPackage->src_tid == g_pDataq_pool[i].tid )
                continue;

            package_size = sizeof(vphy_package_t) + pPackage->data_len;
            rval = rbi_push(g_pDataq_pool[i].dataq, (uint8_t*)pPackage, package_size);
            if( rval )
            {
                printf("rbi send fail (tid = %08x)\n", g_pDataq_pool[i].tid);
            }
        }

        pthread_mutex_unlock(&g_vphy_mtx);

        // shuffle
        srand(time(NULL));
        for(int i = 0; i < CONFIG_VPHY_NODE_MAX; i++)
        {
            int         j = rand() % CONFIG_VPHY_NODE_MAX;
            uint32_t    a = (uint32_t)ppDataq[j];

            ppDataq[j] = ppDataq[i];
            ppDataq[i] = (vphy_dataq_t*)a;
        }

        // trigger ISR
        pthread_mutex_lock(&g_vphy_mtx);

        for(int i = 0; i < CONFIG_VPHY_NODE_MAX; i++)
        {
            if( !ppDataq[i]->cb_isr )
                continue;

            ppDataq[i]->cb_isr(ppDataq[i]);
        }

        pthread_mutex_unlock(&g_vphy_mtx);
    #endif
    }

    if( g_pDataq_pool )
        free(g_pDataq_pool);
    g_pDataq_pool = 0;

    pthread_exit(0);
    return 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
vphy_init(void)
{
    int     rval = 0;

    do {
        if( !(g_pDataq_pool = malloc(sizeof(vphy_dataq_t) * CONFIG_VPHY_NODE_MAX)) )
        {
            rval = -1;
            break;
        }

        memset(g_pDataq_pool, 0x0, sizeof(vphy_dataq_t) * CONFIG_VPHY_NODE_MAX);

        g_vphy_q = rbi_init(10, CONFIG_VPHY_MSG_SIZE);
        if( !g_vphy_q )
        {
            rval = -2;
            break;
        }

        g_vphy_is_running = 1;

        rval = pthread_mutex_init(&g_vphy_mtx, 0);
        if( rval )   break;

        rval = pthread_cond_init(&g_vphy_cond, 0);
        if( rval )   break;

        pthread_create(&g_task_vphy, 0, _task_vphy_routine, 0);

        pthread_mutex_lock(&g_vphy_mtx);
        pthread_cond_wait(&g_vphy_cond, &g_vphy_mtx);
        pthread_mutex_unlock(&g_vphy_mtx);
    } while(0);

    if( rval )
    {
        if( g_pDataq_pool )     free(g_pDataq_pool);
        g_pDataq_pool = 0;

        if( g_vphy_q )  rbi_deinit(g_vphy_q);
    }

    return rval;
}

int
vphy_deinit(void)
{
    int     rval = 0;

    g_vphy_is_running = 0;

    pthread_join(g_task_vphy, 0);

    if( g_pDataq_pool )     free(g_pDataq_pool);
    g_pDataq_pool = 0;

    if( g_vphy_q )  rbi_deinit(g_vphy_q);

    return rval;
}

int
vphy_register_event_callback(
    cb_vphy_isr_t   cb_isr,
    void            *pUser_data)
{
    int     rval = 0;

    do {
        int                 i = 0;

        pthread_mutex_lock(&g_vphy_mtx);

        for(i = 0; i < CONFIG_VPHY_NODE_MAX; ++i)
        {
            if( g_pDataq_pool[i].tid )
                continue;

            g_pDataq_pool[i].tid        = pthread_self();
            g_pDataq_pool[i].cb_isr     = cb_isr;
            g_pDataq_pool[i].pUser_data = pUser_data;

            g_pDataq_pool[i].dataq = rbi_init(5, CONFIG_VPHY_MSG_SIZE);
            if( !g_pDataq_pool[i].dataq )
            {
                rval = -1;
                break;
            }

            break;
        }

        pthread_mutex_unlock(&g_vphy_mtx);

        if( i == CONFIG_VPHY_NODE_MAX )
        {
            rval = -2;
            break;
        }
    } while(0);
    return rval;
}

int
vphy_send(
    uint8_t     *pData,
    uint32_t    length)
{
    int             rval = 0;
    vphy_package_t  *pPackage = 0;

    do {
        uint32_t    package_size = 0;

        package_size = sizeof(vphy_package_t) + length;
//        package_size = (package_size + 0x3) & ~0x3;
        if( package_size > CONFIG_VPHY_MSG_SIZE )
        {
            rval = -1;
            break;
        }

        if( !(pPackage = (vphy_package_t*)malloc(package_size)) )
        {
            rval = -2;
            break;
        }

        memset(pPackage, 0x0, package_size);

        pPackage->src_tid  = pthread_self();
        pPackage->data_len = length;
        memcpy(pPackage->data, pData, length);

        rval = rbi_push(g_vphy_q, (uint8_t*)pPackage, (int)package_size);
        if( rval )
        {
            rval = -3;
            break;
        }
    } while(0);

    if( pPackage )
        free(pPackage);

    return rval;
}




