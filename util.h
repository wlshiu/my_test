/**
 * Copyright (c) 2019, Vango Technologies, Inc. - http://www.vangotech.com/tw/
 * All rights reserved.
 */
/** @file util.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/03/12
 * @license
 * @description
 */


#ifndef __util_H_w413deaa_lf73_hedd_sd2b_uf627f708e66__
#define __util_H_w413deaa_lf73_hedd_sd2b_uf627f708e66__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define UTIL_REG_MASK_WRITE(pReg, u32_data, u32_mask)   do{ *(pReg) &= ~(u32_mask); \
                                                            *(pReg) |= ((u32_data) & (u32_mask)); \
                                                        }while(0)

#define UTIL_SET_BIT(u32_data, bit_order)           ((u32_data) |= (0x1 << (bit_order)))
#define UTIL_CLR_BIT(u32_data, bit_order)           ((u32_data) &= ~(0x1 << (bit_order)))
#define UTIL_IS_BIT_SET(u32_data, bit_order)        ((u32_data) & (0x1 << (bit_order)))


#define util_delay(timeout_cnt)                 do{ for(int k = 0; k < timeout_cnt; k++) __asm("nop"); }while(0)

#ifndef MIN
    #define MIN(a, b)                           (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
    #define MAX(a, b)                           (((a) > (b)) ? (a) : (b))
#endif


#define stringize(s)    #s
#define _toStr(a)       stringize(a)

#define FOUR_CC(a, b, c, d)         (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))

#ifndef __unused
#define __unused            __attribute__ ((unused))
#endif

#ifndef __align
#define __align(num)        __attribute__ ((aligned(num)))
#endif

/**
 *  debug mode macro
 */
#if defined(CONFIG_DEBUG_MODE)
    #include <stdio.h>
    #define _assert(expression)                                         \
        do{ if(expression) break;                                       \
            printf("%s: %s[#%u]\n", #expression, __FILE__, __LINE__);   \
            while(1);                                                   \
        }while(0)

    #ifndef __pause
        #if defined(__GNUC__)
        #define __pause()       do{ __asm volatile("BKPT #01"); }while(0)
        #else
        #define __pause()
        #endif
    #endif

    #define log(str, ...)           printf("[%s:%d] " str, __func__, __LINE__, ##__VA_ARGS__)
    #define err(str, ...)           printf("[%s:%u] " str, __func__, __LINE__, ##__VA_ARGS__)
    #define trace(str, ...)         printf("[%s:%d] " str, __func__, __LINE__, ##__VA_ARGS__)
#else
    #define __pause()
    #define _assert(x)
    #define log(str, ...)
    #define err(str, ...)
    #define trace(str, ...)
#endif

/**
 *  memory layout info
 */
#if defined(__GNUC__)
extern char     __text_start__;
extern char     __text_end__;

extern char     __bss_start__;
extern char     __bss_end__;

extern char     __data_start__;
extern char     __data_end__;

extern char     __HeapBase;
extern char     __HeapLimit;

extern char     __StackTop;
extern char     __StackLimit;

#define GET_STACK_START(ppAddr)      (*(ppAddr) = &__StackTop)
#define GET_STACK_END(ppAddr)        (*(ppAddr) = &__StackLimit)
#define GET_HEAP_START(ppAddr)       (*(ppAddr) = &__HeapBase)
#define GET_HEAP_END(ppAddr)         (*(ppAddr) = &__HeapLimit)

#endif


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


