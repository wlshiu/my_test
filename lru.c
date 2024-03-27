/**
 * Copyright (c) 2024 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file lru.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2024/03/27
 * @license
 * @description
 */


#include "lru.h"
#include <stdlib.h>
#include <string.h>
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
static inline int _is_lru_empty(lru_list_t *pHLRU)
{
    return (pHLRU->tail == 0);
}

static inline int _is_lru_full(lru_list_t *pHLRU)
{
    return (pHLRU->cnt == pHLRU->max_nodes);
}

static lru_node_t* _lru_create_node(int key, uint32_t value)
{
    lru_node_t  *pNode = 0;

    if( (pNode = malloc(sizeof(lru_node_t))) )
    {
        pNode->key   = key;
        pNode->value = value;
        pNode->next  = pNode->prev = 0;
    }
    return pNode;
}

static void _lru_dequeue(lru_list_t *pHLRU)
{
    if( _is_lru_empty(pHLRU) )
        return;

    if( pHLRU->head == pHLRU->tail )
        pHLRU->head = 0;

    do {
        lru_node_t  *pNode = pHLRU->tail;

        pHLRU->tail = pHLRU->tail->prev;

        if( pHLRU->tail )
            pHLRU->tail->next = 0;

        free(pNode);

        pHLRU->cnt--;
    } while(0);

    return;
}

static void _lru_enqueue(lru_list_t *pHLRU, int key, uint32_t value)
{
    lru_hash_t      *pHash = &pHLRU->htable;

    if( _is_lru_full(pHLRU) )
    {
        pHash->htable[pHLRU->tail->key % pHLRU->htable.total_nodes] = 0;
        _lru_dequeue(pHLRU);
    }

    do {
        lru_node_t      *pNode = 0;

        pNode = _lru_create_node(key, value);
        if( !pNode )    break;

        // the new node is inserted to head
        pNode->next = pHLRU->head;

        if( _is_lru_empty(pHLRU) )
        {
            pHLRU->tail = pHLRU->head = pNode;
        }
        else
        {
            pHLRU->head->prev = pNode;
            pHLRU->head       = pNode;
        }

        pHash->htable[key % pHLRU->htable.total_nodes] = pNode;

        pHLRU->cnt++;

    } while(0);

    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
lru_list_t* lru_create(int list_node_size, int hash_node_size)
{
    lru_list_t      *pHLRU = 0;

    do {
        if( !(pHLRU = (lru_list_t*)malloc(sizeof(lru_list_t))) )
            break;

        memset(pHLRU, 0x0, sizeof(lru_list_t));

        pHLRU->max_nodes = list_node_size;

        pHLRU->htable.total_nodes = hash_node_size;
        if( !(pHLRU->htable.htable = (lru_node_t**)malloc(pHLRU->htable.total_nodes * sizeof(lru_node_t*))) )
            break;

        memset(pHLRU->htable.htable, 0x0, pHLRU->htable.total_nodes * sizeof(lru_node_t*));

    } while(0);

    if( pHLRU && !pHLRU->htable.htable )
    {
        free(pHLRU);
        pHLRU = 0;
    }

    return pHLRU;
}

void lru_destroy(lru_list_t **ppHLRU)
{
    do {
        lru_list_t  *pHLRU = *ppHLRU;

        if( !ppHLRU || !pHLRU )
            break;

        if( pHLRU->htable.htable )
            free(pHLRU->htable.htable);

        free(pHLRU);

        *ppHLRU = 0;
    } while(0);

    return;
}

int lru_push_node(lru_list_t *pHLRU, int key, uint32_t value)
{
    lru_err_t       rval = LRU_ERR_OK;

    do {
        lru_node_t      *pNode = 0;

        if( !pHLRU )
        {
            rval = LRU_ERR_NULL_POINTER;
            break;
        }

        #if 0
        if( key >= pHLRU->htable.total_nodes )
        {
            rval = LRU_ERR_OUT_HASH_SIZE;
            break;
        }
        #endif

        pNode = pHLRU->htable.htable[key % pHLRU->htable.total_nodes];

        if( pNode == 0 )
            _lru_enqueue(pHLRU, key, value);
        else if( pNode != pHLRU->head )
        {
            pNode->prev->next = pNode->next;
            if( pNode->next )
                pNode->next->prev = pNode->prev;

            if( pNode == pHLRU->tail )
            {
                pHLRU->tail       = pNode->prev;
                pHLRU->tail->next = 0;
            }

            pNode->next         = pHLRU->head;
            pNode->prev         = 0;
            pNode->next->prev   = pNode;
            pNode->value        = value;

            pHLRU->head = pNode;
        }
    } while(0);

    return rval;
}

int lru_get_node_info(lru_list_t *pHLRU, int key, uint32_t *pValue)
{
    lru_err_t       rval = LRU_ERR_OK;

    do {
        lru_hash_t      *pHash = &pHLRU->htable;
        lru_node_t      *pNode = 0;

        if( !pHLRU || !pValue )
        {
            rval = LRU_ERR_NULL_POINTER;
            break;
        }

        *pValue = LRU_NO_DATA;

        #if 0
        if( key >= pHLRU->htable.total_nodes )
        {
            rval = LRU_ERR_OUT_HASH_SIZE;
            break;
        }
        #endif

        pNode = pHash->htable[key % pHLRU->htable.total_nodes];
        if( !pNode )
        {
            rval = LRU_ERR_NO_NODE;
            break;
        }

        *pValue = pNode->value;

    } while(0);

    return rval;
}

