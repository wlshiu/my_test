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


#include "skb.h"

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
static cb_malloc_t      g_malloc = 0;
static cb_free_t        g_free = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================

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
int
skb_init(skb_conf_t *pConf)
{
    int     rval = 0;
    return rval;
}

void
skb_deinit()
{
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
        if( !g_malloc || !g_free || !length )
            break;

        pSkb = g_malloc(sizeof(skb_t));
        if( !pSkb ) break;

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

        // do free

    } while(0);
    return;
}

int
skb_reserve(skb_t *pSkb)
{
    int     rval = 0;
    return rval;
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












