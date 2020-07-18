/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/07/18
 * @license
 * @description
 */


#include <stdlib.h>
#include <stdio.h>
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

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_malloc(int type, uint32_t len)
{
    uint8_t     *pBuf = 0;
    pBuf = malloc(len);
    return pBuf;
}

static void
_free(int type, void *p)
{
    if( p ) free(p);
    return;
}


//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    {
        skb_conf_t      config = {0};
        config.pf_malloc = _malloc;
        config.pf_free   = _free;
        skb_init(&config);
    }

    do {
        skb_t   *pSkb = 0;
        pSkb = skb_create(100);
        if( !pSkb ) break;

        skb_ref(pSkb);


        skb_destroy(pSkb);

        skb_destroy(pSkb);
    } while(0);



    skb_deinit();
    getc(stdin);
    return 0;
}
