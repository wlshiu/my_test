/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_uhost.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/07/02
 * @license
 * @description
 */

#ifndef __sys_uhost_H_w7dccBDl_l9yK_HdKK_sZGM_uqQQ6SnvOaxF__
#define __sys_uhost_H_w7dccBDl_l9yK_HdKK_sZGM_uqQQ6SnvOaxF__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum sys_uhost_err
{
    SYS_UHOST_ERR_OK             = 0,
    SYS_UHOST_ERR_NO_DEV         = -1,
    SYS_UHOST_ERR_DEV_FULL       = -2,
    SYS_UHOST_ERR_NULL_POINTER   = -3,

} sys_uhost_err_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define SYS_UHOST_TAG(a, b, c, d)           ((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((c) & 0xFF) <<8) | ((d) & 0xFF))
//=============================================================================
//                  Structure Definition
//=============================================================================
struct sys_uhost_class;
typedef struct sys_uhost_class
{
    uint32_t    uclass_id;

    void        *pExtra;

    int     (*uhost_class_init)(void);
    int     (*uhost_class_proc)(void);
    void    (*uhost_err_callback)(struct sys_uhost_class *pUHClass, int err_code);
} sys_uhost_class_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int sys_uhost_register(sys_uhost_class_t *pUHClass);

void sys_uhost_init(void);

void sys_uhost_proc(void);

#ifdef __cplusplus
}
#endif

#endif
