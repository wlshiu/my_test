/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_dev.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/07/02
 * @license
 * @description
 */


#include "sys_udev.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_UDEV_MAX_NUM             3
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sys_udev_mgr
{
    int             udev_cnt;
    sys_udev_t      *pUDev[CONFIG_UDEV_MAX_NUM];
} sys_udev_mgr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static sys_udev_mgr_t       g_udev_mgr = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int sys_udev_register(sys_udev_t *pDev)
{
    int     rval = SYS_UDEV_ERR_OK;

    if( !pDev )
        return SYS_UDEV_ERR_NULL_POINTER;

    if( g_udev_mgr.udev_cnt == CONFIG_UDEV_MAX_NUM )
        return SYS_UDEV_ERR_DEV_FULL;

    g_udev_mgr.pUDev[g_udev_mgr.udev_cnt++] = pDev;
    return rval;
}

void sys_udev_init(void)
{
    int     rval = 0;

    for(int i = 0; i < CONFIG_UDEV_MAX_NUM; i++)
    {
        sys_udev_t  *pUDev = g_udev_mgr.pUDev[i];

        if( !pUDev || !pUDev->udev_init )
            continue;

        rval = pUDev->udev_init();
        if( rval && pUDev->udev_err_callback )
        {
            pUDev->udev_err_callback(pUDev, rval);
        }
    }

    return;
}

void sys_udev_proc(void)
{
    int     rval = SYS_UDEV_ERR_OK;

    for(int i = 0; i < CONFIG_UDEV_MAX_NUM; i++)
    {
        sys_udev_t  *pUDev = g_udev_mgr.pUDev[i];

        if( !pUDev || !pUDev->udev_proc )
            continue;

        rval = pUDev->udev_proc();
        if( rval && pUDev->udev_err_callback )
        {
            pUDev->udev_err_callback(pUDev, rval);
        }
    }

    return;
}
