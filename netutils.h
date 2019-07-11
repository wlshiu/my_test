/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file netutils.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/07/11
 * @license
 * @description
 */

#ifndef __netutils_H_wWsTmEWv_l7K1_Hy8j_sCn6_uR2qinaOS0t2__
#define __netutils_H_wWsTmEWv_l7K1_Hy8j_sCn6_uR2qinaOS0t2__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (WIN32)
    #include <winsock2.h>
    #include<Ws2tcpip.h>
    #ifndef INET6_ADDRSTRLEN
        #define INET6_ADDRSTRLEN 46
    #endif
#else
    #include <netinet/in.h>
    #include<arpa/inet.h>
#endif

#include "errno.h"
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

//=============================================================================
//                  Public Function Definition
//=============================================================================
const char *
inet_ntop(int af, const void *src, char *dst, size_t size);

int inet_pton(int af, const char *src, void *dst);

#ifdef __cplusplus
}
#endif

#endif
