/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ascript.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/09/06
 * @license
 * @description
 */

#include <stdio.h>
#include "ascript.h"

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
void ascript_delay(uint32_t ticks)
{
    printf("for(int i = 0; i < ticks; i++);\n");

    return;
}

void ascript_reg_write(uint32_t addr, uint32_t value)
{
    printf("*((volatile uint32_t*)0x%08X) = 0x%08X\n", addr, value);

    return;
}

void ascript_reg_write_msk(uint32_t addr, uint32_t mask, uint32_t value)
{
    printf("*((volatile uint32_t*)0x%08X) =\n"
           "    (*((volatile uint32_t*)0x%08X) & ~0x%08X) | (0x%08X & 0x%08X);\n",
           addr, addr, mask, value, mask);


    return;
}

uint32_t ascript_reg_read_msk(uint32_t addr, uint32_t mask)
{
    printf("return *((volatile uint32_t*)0x%08X) & 0x%08X\n", addr, mask);
    return 0;
}

void ascript_wait_flags(uint32_t addr, uint32_t flags)
{
    printf("while( (*((volatile uint32_t*)0x%08X) & 0x%08X ) != 0x%08X );\n",
           addr, flags, flags);

    return;
}

void ascript_set_flags(uint32_t addr, uint32_t flags)
{
    printf("*((volatile uint32_t*)0x%08X) |= 0x%08X;\n", addr, flags);
    return;
}

void ascript_clr_flags(uint32_t addr, uint32_t flags)
{
    printf("*((volatile uint32_t*)0x%08X) &= ~0x%08X;\n", addr, flags);

    return;
}

