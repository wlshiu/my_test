/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file net_app.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/07/11
 * @license
 * @description
 */


#include "net_app.h"
#include "util.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static cb_net_app   g_net_callback_func = 0;
//=============================================================================
//                  Public Function Definition
//=============================================================================
void
net_app__set_callback(
    cb_net_app  pfunc)
{
    g_net_callback_func = pfunc;
    return;
}


cb_net_app
net_app__get_callback(void)
{
    return g_net_callback_func;
}


#if defined(CONFIG_CALLBACK_TRACE)
void
net_app__call_callback(const char* caller, int line)
#else
void
net_app__call_callback(void)
#endif // 1
{
    do {
        if( !g_net_callback_func )
        {
            // err("NO app callback...\n");
            break;
        }

        #if defined(CONFIG_CALLBACK_TRACE)
        printf("[%s] caller= %s, %u\n", __func__, caller, line);
        #endif // defined
        g_net_callback_func();
    } while(0);

    return;
}



