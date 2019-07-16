/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file net_app.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/07/11
 * @license
 * @description
 */


#ifndef __net_app_H_09a7b70e_5ddd_4d70_9109_f44353ac4507__
#define __net_app_H_09a7b70e_5ddd_4d70_9109_f44353ac4507__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
//#define CONFIG_CALLBACK_TRACE

typedef void (*cb_net_app)(void);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct uip_tcp_appstate_t
{
    unsigned long       reserved;
} uip_tcp_appstate_t;

typedef struct uip_udp_appstate_t
{
    unsigned long       reserved;
} uip_udp_appstate_t;
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
net_app__set_callback(
    cb_net_app  pfunc);


cb_net_app
net_app__get_callback(void);

#if defined(CONFIG_CALLBACK_TRACE)
void
net_app__call_callback(const char* caller, int line);
#else
void
net_app__call_callback(void);
#endif

#ifdef __cplusplus
}
#endif

#endif


