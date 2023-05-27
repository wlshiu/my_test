/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file smm.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/05/27
 * @license
 * @description
 *
 *  SMM (Static Memory Management) is implemented to manage a static memory.
 *  This lib only support to allocate memory and record the pulse of memory usage.
 *  ps. free memory and merge memory leakage are NOT supported,
 *      and the memory fragmentation MUST be happened.
 */

#ifndef __smm_H_wO61d75c_l2zP_HIv8_sl1s_uEdfo0FxNd2g__
#define __smm_H_wO61d75c_l2zP_HIv8_sl1s_uEdfo0FxNd2g__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum smm_err
{
    SMM_ERR_OK              = 0,
    SMM_ERR_NULL_POINTER    = -1,
    SMM_ERR_ADDR_NOT_ALIGN  = -2,
    SMM_ERR_OVER_POOL       = -3,
    SMM_ERR_OVERFLOW        = -4,
} smm_err_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct smm_handle
{
    unsigned long   pool_addr;
    int             pool_size;
    int             offset;
    int             usage;      // bytes
    int             max_usage;  // bytes

} smm_handle_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int smm_init(smm_handle_t *pHSmm);

void smm_deinit(smm_handle_t *pHSmm);

void* smm_malloc(smm_handle_t *pHSmm, unsigned long len);

int smm_free(smm_handle_t *pHSmm, void *p);

int smm_usage(smm_handle_t *pHSmm);

#ifdef __cplusplus
}
#endif

#endif
