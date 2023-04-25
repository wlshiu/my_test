/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_methods.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/04/24
 * @license
 * @description
 */

#ifndef __sys_method_H_wfWPjcEC_l8NH_Hnfh_sh67_u872wTXIjGtO__
#define __sys_method_H_wfWPjcEC_l8NH_Hnfh_sh67_u872wTXIjGtO__

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
#define FOURCC(a, b, c, d)      ((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((c) & 0xFF) << 8) | ((d) & 0xFF))
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sys_methods
{
    void*   (*cb_malloc)(int size, uint32_t tag, int subtag);
    void    (*cb_free)(void *ptr, uint32_t tag, int subtag);
//    void *calloc(size_t nmemb, size_t size);

    void    (*cb_debug)(void);

    #define SYS_MEM_DUMP_TYPE_FILE      (0x1u << 7)
    #define SYS_MEM_DUMP_TYPE_U8        (0x1u << 0)
    #define SYS_MEM_DUMP_TYPE_U32       (0x1u << 1)
    #define SYS_MEM_DUMP_TYPE_FLOAT     (0x1u << 2)
    void    (*cb_dump_mem)(char *prefix, void *pAddr, int length, int dump_type);


} sys_methods_t;
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
