/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file log.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/04/17
 * @license
 * @description
 */


#ifndef __log_H_645a1707_cf81_4d18_8200_47192020d9eb__
#define __log_H_645a1707_cf81_4d18_8200_47192020d9eb__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#if defined(WIN32)
    #define NC
    #define RED
    #define LIGHT_RED
    #define GREEN
    #define LIGHT_GREEN
    #define BLUE
    #define LIGHT_BLUE
    #define DARY_GRAY
    #define CYAN
    #define LIGHT_CYAN
    #define PURPLE
    #define LIGHT_PURPLE
    #define BROWN
    #define YELLOW
    #define LIGHT_GRAY
    #define WHITE

#else
    #define NC          "\033[m"
    #define RED         "\033[0;32;31m"
    #define LIGHT_RED   "\033[1;31m"
    #define GREEN       "\033[0;32;32m"
    #define LIGHT_GREEN "\033[1;32m"
    #define BLUE        "\033[0;32;34m"
    #define LIGHT_BLUE  "\033[1;34m"
    #define DARY_GRAY   "\033[1;30m"
    #define CYAN        "\033[0;36m"
    #define LIGHT_CYAN  "\033[1;36m"
    #define PURPLE      "\033[0;35m"
    #define LIGHT_PURPLE "\033[1;35m"
    #define BROWN       "\033[0;33m"
    #define YELLOW      "\033[1;33m"
    #define LIGHT_GRAY  "\033[0;37m"
    #define WHITE       "\033[1;37m"
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================
#if 0
#define err(str, ...)                   do{ printf(RED"[%s:%d:error] " str NC, __func__, __LINE__, ##__VA_ARGS__); fflush(stderr); }while(0)
#else
#define err(str, ...)                   do{ printf(RED"[error] " str NC, ##__VA_ARGS__); fflush(stderr); }while(0)
#endif
#define msg(str, ...)                   do{ printf(str, ##__VA_ARGS__); fflush(stdout); } while(0)
#define msg_color(color, str, ...)      do{ printf(color str NC, ##__VA_ARGS__); fflush(stdout); }while(0)

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

#ifdef __cplusplus
}
#endif

#endif


