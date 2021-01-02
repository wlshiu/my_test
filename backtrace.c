/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file backtrace.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/01/02
 * @license
 * @description
 */


#include <stdio.h>
#include "backtrace.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define configSUPPORT_FPU   1
#define configSUPPORT_ZOL   0
#define configSUPPORT_IFC   1

#if ( configSUPPORT_FPU == 1 )
    #if defined(__NDS32_EXT_FPU_CONFIG_0__)
        #define portFPU_REGS    8
    #elif defined(__NDS32_EXT_FPU_CONFIG_1__)
        #define portFPU_REGS    16
    #elif defined(__NDS32_EXT_FPU_CONFIG_2__)
        #define portFPU_REGS    32
    #elif defined(__NDS32_EXT_FPU_CONFIG_3__)
        #define portFPU_REGS    64
    #endif

    #define portFPU_REGS    32
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  context switch
 */
typedef struct stack_nds32
{
#if ( configSUPPORT_FPU == 1 )
    unsigned long       fpu[portFPU_REGS];
#endif

    unsigned long       sp;
    unsigned long       psw;
    unsigned long       ipc;
    unsigned long       ipsw;

#if (configSUPPORT_ZOL)
    unsigned long       zol_lb;
    unsigned long       zol_le;
    unsigned long       zol_lc;
#endif

#if (configSUPPORT_IFC)
    unsigned long       ifc_lp;
#endif

#if (configSUPPORT_D0D1)
    unsigned long       d0_lo;
    unsigned long       d0_lh;
    unsigned long       d1_lo;
    unsigned long       d1_lh;
#endif

    unsigned long       r2;
    unsigned long       r3;
    unsigned long       r4;
    unsigned long       r5;
    unsigned long       r6;
    unsigned long       r7;
    unsigned long       r8;
    unsigned long       r9;
    unsigned long       r10;
#ifdef __NDS32_REDUCE_REGS__
    unsigned long       r15;
#else
    unsigned long       r11;
    unsigned long       r12;
    unsigned long       r13;
    unsigned long       r14;
    unsigned long       r15;
    unsigned long       r16;
    unsigned long       r17;
    unsigned long       r18;
    unsigned long       r19;
    unsigned long       r20;
    unsigned long       r21;
    unsigned long       r22;
    unsigned long       r23;
    unsigned long       r24;
    unsigned long       r25;
    unsigned long       r26;
    unsigned long       r27;
#endif

    unsigned long       fp; // r28;
    unsigned long       gp; // r29;
    unsigned long       lp; // r30;

    unsigned long       r0;
    unsigned long       r1;

} stack_nds32_t;


typedef struct stack_frame
{
    unsigned long       fp; // r28;
    unsigned long       gp; // r29;
    unsigned long       lp; // r30;
} stack_frame_t;

typedef struct backtrace_mgr
{
    backtrace_txt_range_t   *pTxt_range;
    unsigned int            range_num;
} backtrace_mgr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static backtrace_mgr_t      g_backtrace_mgr = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
#define SP_BASE     0x110550
//=============================================================================
//                  Public Function Definition
//=============================================================================
void
backtrace_init(
    backtrace_txt_range_t   *pRange,
    unsigned long           number)
{
    g_backtrace_mgr.pTxt_range = pRange;
    g_backtrace_mgr.range_num  = number;
    return;
}

void
backtrace(
    unsigned long   sp_cur,
    unsigned long   sp_start,
    unsigned long   *pFrame,
    int             frame_level)
{
    stack_nds32_t   *pStack_cntxt_switch = (stack_nds32_t*)sp_cur;

    do {
        stack_frame_t   *pFrame = 0;

        printf("size = 0x%x\n", sizeof(stack_nds32_t));
        printf(" sp   = 0x%08x\n", pStack_cntxt_switch->sp);
        printf(" psw  = 0x%08x\n", pStack_cntxt_switch->psw);
        printf(" ipc  = 0x%08x\n", pStack_cntxt_switch->ipc);
        printf(" ipsw = 0x%08x\n", pStack_cntxt_switch->ipsw);
        printf(" r27 = 0x%08x\n", pStack_cntxt_switch->r27);

        printf(" fp = 0x%08x, 0x%08x\n", pStack_cntxt_switch->fp, pStack_cntxt_switch->fp - SP_BASE);
        printf(" gp = 0x%08x\n", pStack_cntxt_switch->gp);
        printf(" lp = 0x%08x\n", pStack_cntxt_switch->lp);

        frame_level = (frame_level > 0) ? frame_level : 5;

        if( (pStack_cntxt_switch->fp & 0x3) )
            break;

        #if 0
        pFrame = (stack_frame_t*)(pStack_cntxt_switch->fp - sizeof(stack_frame_t));
        #else
        //-------- for test
        pFrame = (stack_frame_t*)(pStack_cntxt_switch->fp - sizeof(stack_frame_t) - SP_BASE + sp_cur);
        #endif


        for(int level = 0; level < frame_level; level++)
        {
            printf("--- frame %d (0x%08x)---\n", level, pFrame);
            printf("fp = 0x%08x\n", pFrame->fp);
            printf("gp = 0x%08x\n", pFrame->gp);
            printf("lp = 0x%08x\n", pFrame->lp);

            if( (pFrame->fp & 0x3) ||
                pFrame->fp > sp_start || pFrame->fp < sp_cur)
                break;

            #if 0
            pFrame = (stack_frame_t*)(pFrame->fp - sizeof(stack_frame_t));
            if( (unsigned long)pFrame > sp_start )
                break;
            #else
            printf("offset= x%x\n", pFrame->fp - sizeof(stack_frame_t) - SP_BASE);
            pFrame = (stack_frame_t*)(pFrame->fp - sizeof(stack_frame_t) - SP_BASE + sp_cur);
            if( (unsigned long)pFrame > sp_start )
                break;
            #endif


        }


    } while(0);

    return;
}

