/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file overlay.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/06/21
 * @license
 * @description
 */

#ifndef __overlay_H_wFkzExD3_lQAm_HZ4D_soO5_uRbIeocZrLKm__
#define __overlay_H_wFkzExD3_lQAm_HZ4D_soO5_uRbIeocZrLKm__

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
#if defined ( __CC_ARM )
#define DECLARE_OVERLAY_AREA(__section_name__, __var_name__)             \
    extern unsigned char    Load$$##__section_name__##$$Base;            \
    extern unsigned char    Image$$##__section_name__##$$Base;           \
    extern unsigned char    Image$$##__section_name__##$$Length;         \
    static overlay_info_t       __var_name__ = {                         \
        .pLoad_base = (uint32_t*)&Load$$##__section_name__##$$Base,      \
        .pExec_base = (uint32_t*)&Image$$##__section_name__##$$Base,     \
        .length     = ((uint32_t*)&Image$$##__section_name__##$$Length), \
    }
#else
    #error "Overlay syntax not support !!!"
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct overlay_info
{
    uint32_t    *pLoad_base;
    uint32_t    *pExec_base;
    uint32_t    *length;
} overlay_info_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void overlay_load(overlay_info_t *pInfo);


#ifdef __cplusplus
}
#endif

#endif
