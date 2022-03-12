/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file rbuf.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/03/12
 * @license
 * @description
 */

#ifndef __rbuf_H_w991mzUJ_lWsS_HncW_slju_uWKwfxTrdWhI__
#define __rbuf_H_w991mzUJ_lWsS_HncW_slju_uWKwfxTrdWhI__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define RBI_BUF_ITEM_LEN    8
#define RBI_SLOT_MAX_LEN    256
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct rbi
{
    uint32_t        head;
    uint32_t        tail;

    uint32_t        buf[RBI_SLOT_MAX_LEN][RBI_BUF_ITEM_LEN >> 2];
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
rbi_init(rbi_t *pRB);

int
rbi_pop(rbi_t *pRB, uint8_t *pData, int *pLen);

int
rbi_push(rbi_t *pRB, uint8_t *pData, int len);

#ifdef __cplusplus
}
#endif

#endif
