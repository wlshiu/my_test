/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file skb.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/07/15
 * @license
 * @description
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "skb.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
//#define CONFIG_TEST_SBK_QUEUE

#define CONFIG_SKB_SLOT_NUM         3//16
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct skb_list
{
    skb_t   *head;
    skb_t   *tail;
} skb_list_t;

typedef struct skb_dev
{
    cb_malloc_t     cb_malloc;
    cb_free_t       cb_free;

#if defined(CONFIG_TEST_SBK_QUEUE)
    skb_list_t      skb_used;
#endif

    skb_list_t      skb_wild;

} skb_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static skb_t            g_skb_pool[CONFIG_SKB_SLOT_NUM] = {0};
static skb_dev_t        g_skb_dev = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_skb_enqueue(skb_list_t *pSkb_list, skb_t *pSkb)
{
    int     rval = 0;
    do {
        if( !pSkb )
        {
            rval = -1;
            break;
        }
        pSkb->next = 0;

        if( pSkb_list->head )
            pSkb_list->tail->next = pSkb;
        else
            pSkb_list->head = pSkb;

        pSkb_list->tail = pSkb;

    } while(0);

    return rval;
}

static skb_t*
_skb_dequeue(skb_list_t *pSkb_list, skb_t *pSkb)
{
    do {
        skb_t   *pCur = 0;
        if( !pSkb )
        {
            // dequeue the first item
            pSkb = pSkb_list->head;
            pSkb_list->head = (pSkb) ? pSkb->next : 0;
            break;
        }

        pCur = pSkb_list->head;

        while( pCur )
        {
            if( pCur->next == pSkb )
            {
                pCur->next = pSkb->next;
                break;
            }

            pCur = pCur->next;
        }

    } while(0);
    return pSkb;
}

static void
_skb_list_dump(skb_list_t *pSkb_list, char *prefix)
{
    skb_t   *pSkb = pSkb_list->head;
    while( pSkb )
    {
        printf("%s: %p\n", prefix, pSkb);
        pSkb = pSkb->next;
    }
    return;
}


static void
_test_skb_queue()
{
#if defined(CONFIG_TEST_SBK_QUEUE)
    int     cnt = 10;
    skb_t   *pSkb = 0;

    srand(time(0));

    for(int i = 0; i < CONFIG_SKB_SLOT_NUM; i++)
    {
        printf("%d-th: %08x\n", i, &g_skb_pool[i]);
    }

    printf("=========\n");
    _skb_list_dump(&g_skb_dev.skb_wild, "wild");
    _skb_list_dump(&g_skb_dev.skb_used, "used");

    while( cnt-- )
    {
        switch( (rand() % 33) & 0x1 )
        {
            case 1:
                printf("---%d: used(enq), wild(deq)\n", __LINE__);
                pSkb = _skb_dequeue(&g_skb_dev.skb_wild, 0);
                _skb_enqueue(&g_skb_dev.skb_used, pSkb);

                break;
            default:
                printf("---%d: wild(enq), used(deq)\n", __LINE__);
                pSkb = _skb_dequeue(&g_skb_dev.skb_used, 0);
                _skb_enqueue(&g_skb_dev.skb_wild, pSkb);
                break;
        }

        _skb_list_dump(&g_skb_dev.skb_wild, "wild");
        _skb_list_dump(&g_skb_dev.skb_used, "used");
    }
#endif
    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  @brief  skb_init
 *              initialize sk buffer mechanism
 *
 *  @param [in] pConf   the configuration of sk buffer mechanism
 *  @return
 *      0: ok, others: fail
 */
skb_err_t
skb_init(skb_conf_t *pConf)
{
    skb_err_t   rval = SKB_ERR_OK;

    do {
        if( !pConf || !pConf->pf_malloc || !pConf->pf_free )
        {
            rval = SKB_ERR_WRONG_PARAM;
            break;
        }

        memset(&g_skb_dev, 0x0, sizeof(g_skb_dev));

        g_skb_dev.cb_malloc = pConf->pf_malloc;
        g_skb_dev.cb_free   = pConf->pf_free;

        {
            skb_list_t      *pSkb_list = &g_skb_dev.skb_wild;

            pSkb_list->head = &g_skb_pool[0];
            pSkb_list->tail = pSkb_list->head;

            for(int i = 1; i < CONFIG_SKB_SLOT_NUM; i++)
            {
                pSkb_list->tail->next = &g_skb_pool[i];
                pSkb_list->tail = pSkb_list->tail->next;
            }
        }

        _test_skb_queue();
    } while(0);

    return rval;
}

void
skb_deinit(void)
{
    for(int i = 0; i < CONFIG_SKB_SLOT_NUM; i++)
    {
        skb_t   *pSkb = &g_skb_pool[i];

        if( pSkb->head )
            g_skb_dev.cb_free(0, pSkb->head);
    }

    memset(&g_skb_pool, 0x0, sizeof(g_skb_pool));
    return;
}

/**
 *  @brief  skb_create
 *              create a skb item
 *  @return
 *      NULL or pointer of a skb item
 */
skb_t*
skb_create(int length)
{
    skb_t   *pSkb = 0;
    do {
        skb_err_t   rval = SKB_ERR_OK;
        if( !length )
        {
            rval = SKB_ERR_WRONG_PARAM;
            break;
        }

        pSkb = _skb_dequeue(&g_skb_dev.skb_wild, 0);
        if( !pSkb )
        {
            rval = SKB_ERR_FULL;
            break;
        }

        memset(pSkb, 0x0, sizeof(skb_t));

        pSkb->head = g_skb_dev.cb_malloc(0, length);
        if( !pSkb->head )
        {
            _skb_enqueue(&g_skb_dev.skb_wild, pSkb);

            pSkb = 0;
            rval = SKB_ERR_NO_DATA_BUFFER;
            break;
        }

        pSkb->end = length;

        pSkb->transport_hdr = -1;
        pSkb->network_hdr   = -1;
        pSkb->mac_hdr       = -1;

    #if defined(CONFIG_TEST_SBK_QUEUE)
        _skb_enqueue(&g_skb_dev.skb_used, pSkb);
    #endif

        pSkb->ref_cnt = 1;

    } while(0);

    return pSkb;
}

void
skb_destroy(skb_t *pSkb)
{
    do {
        if( !pSkb || --pSkb->ref_cnt )
            break;

        if( pSkb->head )
            g_skb_dev.cb_free(0, pSkb->head);

    #if defined(CONFIG_TEST_SBK_QUEUE)
        pSkb = _skb_dequeue(&g_skb_dev.skb_used, pSkb);
    #endif

        _skb_enqueue(&g_skb_dev.skb_wild, pSkb);

    } while(0);
    return;
}


void*
skb_push(
    skb_t       *pSkb,
    uint8_t     *pData,
    uint32_t    length)
{

}

int
skb_align(
    skb_t       *pSkb,
    uint32_t    align_num)
{
    int     rval = 0;
    return rval;
}












