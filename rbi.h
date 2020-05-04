/**
 * Copyright (c) 2018 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file rbi.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @license
 * @description
 */

#ifndef __rbi_H_wZZsqgW2_l3RS_Hrer_sH5Q_ushqu1NsbXds__
#define __rbi_H_wZZsqgW2_l3RS_Hrer_sH5Q_ushqu1NsbXds__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define RBI_CALC_POOL_SIZE(slot_size)       ((slot_size) * sizeof(void*))
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct rbi
{
//#define RB_SIZE_MASK    0x1
    uint32_t        head;
    uint32_t        pick;
    uint32_t        tail;

    uint32_t        slot_size;
    uint32_t        unit_size;
    uint32_t        *pBuf;

} rbi_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void
rbi_init(rbi_t *pRB, uint32_t *pArray, uint32_t slot_size);


uint32_t
rbi_is_empty(rbi_t *pRB);


uint32_t
rbi_is_full(rbi_t *pRB);


uint32_t
rbi_pick(rbi_t *pRB);


uint32_t
rbi_pop(rbi_t *pRB);


int
rbi_push(rbi_t *pRB, uint32_t data);


#ifdef __cplusplus
}
#endif

#endif
