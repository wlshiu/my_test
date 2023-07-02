/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file sys_uhost.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/07/02
 * @license
 * @description
 */


#include "sys_uhost.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_UHOST_CLASS_MAX_NUM      3
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct sys_uhost_mgr
{
    int                 uhost_class_cnt;
    sys_uhost_class_t   *pUHClass[CONFIG_UHOST_CLASS_MAX_NUM];
} sys_uhost_mgr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static sys_uhost_mgr_t       g_uhost_mgr = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int sys_uhost_register(sys_uhost_class_t *pUHClass)
{
    int     rval = SYS_UHOST_ERR_OK;

    if( !pUHClass )
        return SYS_UHOST_ERR_NULL_POINTER;

    if( g_uhost_mgr.uhost_class_cnt == CONFIG_UHOST_CLASS_MAX_NUM )
        return SYS_UHOST_ERR_DEV_FULL;

    g_uhost_mgr.pUHClass[g_uhost_mgr.uhost_class_cnt++] = pUHClass;
    return rval;
}

void sys_uhost_init(void)
{
    int     rval = 0;

    for(int i = 0; i < CONFIG_UHOST_CLASS_MAX_NUM; i++)
    {
        sys_uhost_class_t  *pUHClass = g_uhost_mgr.pUHClass[i];

        if( !pUHClass || !pUHClass->uhost_class_init )
            continue;

        rval = pUHClass->uhost_class_init();
        if( rval && pUHClass->uhost_err_callback )
        {
            pUHClass->uhost_err_callback(pUHClass, rval);
        }
    }

    return;
}

void sys_uhost_proc(void)
{
    int     rval = SYS_UHOST_ERR_OK;

    for(int i = 0; i < CONFIG_UHOST_CLASS_MAX_NUM; i++)
    {
        sys_uhost_class_t  *pUHClass = g_uhost_mgr.pUHClass[i];

        if( !pUHClass || !pUHClass->uhost_class_proc )
            continue;

        rval = pUHClass->uhost_class_proc();
        if( rval && pUHClass->uhost_err_callback )
        {
            pUHClass->uhost_err_callback(pUHClass, rval);
        }
    }

    return;
}
