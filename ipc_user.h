/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ipc_user.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/29
 * @license
 * @description
 */

#ifndef __ipc_user_H_wzrTNvUA_lUMd_HYIS_s1Wt_uCLgLlCy7LPc__
#define __ipc_user_H_wzrTNvUA_lUMd_HYIS_s1Wt_uCLgLlCy7LPc__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define NODEQ_NUM               4
#define NODEQ_DEPTH             8

//=============================================================================
//                  Macro Definition
//=============================================================================
#define GET_NODEQ_HDR(pBase, nodeq_id)  \
    ((nodeq_header_t*)(((uintptr_t)(pBase)) + (nodeq_id) * sizeof(nodeq_header_t)))

#define GET_NEXT_RD_NODE(pBase, nodeq_id)                                                                                        \
    (((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->wr_index == ((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->rd_index) \
    ? 0                                                                                                                          \
    : (node_t*)(((uintptr_t)(pBase) + sizeof(nodeq_header_t) * NODEQ_NUM + (nodeq_id) * NODEQ_DEPTH * sizeof(node_t)) +          \
        ((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->rd_index * sizeof(node_t))

#define GET_NEXT_WR_NODE(pBase, nodeq_id)                                                                                            \
    ((((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->wr_index == ((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->rd_index) && \
     (((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->wr_cnt != ((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->rd_cnt))       \
    ? 0                                                                                                                              \
    : (node_t*)(((uintptr_t)(pBase) + sizeof(nodeq_header_t) * NODEQ_NUM + (nodeq_id) * NODEQ_DEPTH * sizeof(node_t)) +              \
        ((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->wr_index * sizeof(node_t))

#define UPDATE_NODE_INDEX(pBase, nodeq_id, rw_type)                                                 \
    do {((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->rw_type##_index++;                       \
        ((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->rw_type##_index &= (NODEQ_DEPTH - 1);    \
        ((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->rw_type##_cnt++;                         \
        ((nodeq_header_t*)GET_NODEQ_HDR(pBase, nodeq_id))->rw_type##_cnt &= 0xFFFF;                 \
    } while(0)

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct nodeq_header
{
    uint16_t    rd_index;
    uint16_t    rd_cnt;
    uint16_t    wr_index;
    uint16_t    wr_cnt;
} nodeq_header_t;


typedef struct node
{
    uint32_t    is_used;
    uint32_t    length;
    char        data[56];
} node_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
