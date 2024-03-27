/**
 * Copyright (c) 2024 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file lru.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2024/03/27
 * @license
 * @description
 */

#ifndef __lru_H_wYixF03b_lUT5_HPqv_s0J2_uTae6210vtga__
#define __lru_H_wYixF03b_lUT5_HPqv_s0J2_uTae6210vtga__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define LRU_NO_DATA         0xdeadbeeful

typedef enum lru_err
{
    LRU_ERR_OK              = 0,
    LRU_ERR_NULL_POINTER    = -10,
    LRU_ERR_FAIL            = -9,
    LRU_ERR_NO_NODE         = -8,
    LRU_ERR_OUT_HASH_SIZE   = -7,
} lru_err_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  a node of LRU (Least Recently Used)
 */
typedef struct lru_node
{
    struct lru_node     *prev, *next;

    int                 key;
    uint32_t            value;
} lru_node_t;

typedef struct lru_hash
{
    int         total_nodes;
    lru_node_t  **htable;
} lru_hash_t;

typedef struct lru_list
{
    lru_node_t  *head, *tail;

    int         max_nodes;
    int         cnt;

    lru_hash_t  htable;
} lru_list_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
lru_list_t* lru_create(int list_node_size, int hash_node_size);

void lru_destroy(lru_list_t **ppHLRU);

int lru_push_node(lru_list_t *pHLRU, int key, uint32_t value);

int lru_get_node_info(lru_list_t *pHLRU, int key, uint32_t *pValue);

#ifdef __cplusplus
}
#endif

#endif
