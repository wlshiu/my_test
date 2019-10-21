/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/09
 * @license
 * @description
 */



#include "cm_backtrace.h"
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
static void fault_test_by_unalign(void)
{
    volatile int * SCB_CCR = (volatile int *) 0xE000ED14; // SCB->CCR
    volatile int * p;
    volatile int value;

    *SCB_CCR |= (1 << 3); /* bit3: UNALIGN_TRP. */

    p = (int *) 0x00;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

    p = (int *) 0x04;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

    p = (int *) 0x03;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);
}

static void fault_test_by_div0(void)
{
    volatile int * SCB_CCR = (volatile int *) 0xE000ED14; // SCB->CCR
    int x, y, z;

    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */

    x = 10;
    y = 0;
    z = x / y;
    printf("z:%d\n", z);
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
void HardFault_Handler(void)
{
#ifdef __ARM__
    __asm("MOV     r0, lr               \n" /* get lr */
          "MOV     r1, sp               \n" /* get stack pointer (current is MSP) */
          "BL      cm_backtrace_fault   \n");

    __asm volatile("BKPT #01");
#endif
}

int main(void)
{
    cm_backtrace_init("CmBacktrace", "v0.0.0", "v1.3.0");

    fault_test_by_div0();
    return 0;
}
