/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smm.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/05/27
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "smm.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_SMM_NODE_ACT_TAG         0xbeefc001ul
#define CONFIG_SMM_NODE_FREE_TAG        0xbeefdeadul

#define CONFIG_SMM_ADDR_ALIGN_NUM       0x3ul   // 4-align
//#define CONFIG_SMM_ADDR_ALIGN_NUM       0x7ul   // 8-align
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct smm_node
{
    unsigned int  tag;
    unsigned int  len;

} smm_node_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int smm_init(smm_handle_t *pHSmm)
{
    if( !pHSmm || !pHSmm->pool_addr || !pHSmm->pool_size )
        return SMM_ERR_NULL_POINTER;

    if( pHSmm->pool_addr & CONFIG_SMM_ADDR_ALIGN_NUM )
        return SMM_ERR_ADDR_NOT_ALIGN;

    pHSmm->offset    = 0ul;
    pHSmm->usage     = 0ul;
    pHSmm->max_usage = -1;

    memset((void*)pHSmm->pool_addr, 0x0, pHSmm->pool_size);
    return SMM_ERR_OK;
}

void smm_deinit(smm_handle_t *pHSmm)
{
    if( pHSmm )
    {
        pHSmm->offset    = 0ul;
        pHSmm->usage     = 0;
        pHSmm->max_usage = 0;
    }

    return;
}

void* smm_malloc(smm_handle_t *pHSmm, unsigned long len)
{
    smm_node_t      *pNode = 0;

    if( !pHSmm || !pHSmm->pool_addr || !pHSmm->pool_size )
        return (void*)SMM_ERR_NULL_POINTER;

    if( (pHSmm->offset + len) >= pHSmm->pool_size )
        return (void*)SMM_ERR_OVER_POOL;

    /**
     *  ToDo: re-use the wild memory slot if slot size is more than the request
     */

    pNode = (smm_node_t*)(pHSmm->pool_addr + pHSmm->offset);
    pNode->tag = CONFIG_SMM_NODE_ACT_TAG;
    pNode->len = (len + CONFIG_SMM_ADDR_ALIGN_NUM) & ~CONFIG_SMM_ADDR_ALIGN_NUM;

    pHSmm->offset += (pNode->len + sizeof(smm_node_t));
    pHSmm->usage += pNode->len;
    return (void*)(pNode + 1);
}

int smm_free(smm_handle_t *pHSmm, void* p)
{
    smm_node_t      *pNode = 0;

    if( !pHSmm )
        return SMM_ERR_NULL_POINTER;

    pNode = ((smm_node_t*)p) - 1;

    if( pNode->tag == CONFIG_SMM_NODE_ACT_TAG )
    {
        pNode->tag = CONFIG_SMM_NODE_FREE_TAG;

        pHSmm->max_usage = (pHSmm->max_usage < pHSmm->usage)
                         ? pHSmm->usage : pHSmm->max_usage;

        pHSmm->usage -= pNode->len;
    }
    else if( pNode->tag != CONFIG_SMM_NODE_FREE_TAG )
        return SMM_ERR_OVERFLOW;

    return SMM_ERR_OK;
}

int smm_usage(smm_handle_t *pHSmm)
{
    smm_node_t      *pNode = 0;
    int             i = 0;

    if( !pHSmm || !pHSmm->pool_addr || !pHSmm->pool_size )
        return SMM_ERR_NULL_POINTER;

    printf("memory usage:\n");
    printf("    state    address    size(bytes)\n");

    pNode = (smm_node_t*)pHSmm->pool_addr;
    while( (unsigned long)pNode < (pHSmm->pool_addr + pHSmm->offset) &&
           (unsigned long)pNode < (pHSmm->pool_addr + pHSmm->pool_size) )
    {
        switch( pNode->tag )
        {
            default:
                printf("memory overflow happened !\n");
                break;
            case CONFIG_SMM_NODE_ACT_TAG:
                printf("%02d. used     0x%08lX   %d\n", i, (long)(pNode + 1), pNode->len);
                break;
            case CONFIG_SMM_NODE_FREE_TAG:
                printf("%02d. wild     0x%08lX   %d\n", i, (long)(pNode + 1), pNode->len);
                break;
        }
        pNode = (smm_node_t*)(((unsigned long)(pNode + 1)) + pNode->len);
        i++;
    }

    printf("The max usage: %d nbytes\n", pHSmm->max_usage);

    return SMM_ERR_OK;
}
