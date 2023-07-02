/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_udev.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/07/02
 * @license
 * @description
 */

#ifndef __sys_udev_H_wHJJqK7F_lC8Q_HfsW_sM7g_urdVy6ColYvh__
#define __sys_udev_H_wHJJqK7F_lC8Q_HfsW_sM7g_urdVy6ColYvh__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum sys_udev_err
{
    SYS_UDEV_ERR_OK             = 0,
    SYS_UDEV_ERR_NO_DEV         = -1,
    SYS_UDEV_ERR_DEV_FULL       = -2,
    SYS_UDEV_ERR_NULL_POINTER   = -3,

} sys_udev_err_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define SYS_UDEV_TAG(a, b, c, d)            ((((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | (((c) & 0xFF) <<8) | ((d) & 0xFF))
//=============================================================================
//                  Structure Definition
//=============================================================================
struct sys_udev;
typedef struct sys_udev
{
    uint32_t    udev_id;

    void        *pExtra;

    int     (*udev_init)(void);
    int     (*udev_proc)(void);
    void    (*udev_err_callback)(struct sys_udev *pUDev, int err_code);

} sys_udev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int sys_udev_register(sys_udev_t *pDev);

void sys_udev_init(void);

void sys_udev_proc(void);

#ifdef __cplusplus
}
#endif

#endif
