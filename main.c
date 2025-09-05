/**
 * Copyright (c) 2025 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2025/09/06
 * @license
 * @description
 */


#include "main.h"
#include "ascript.h"

#include "ARM_Example.h"
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
int main(void)
{
    AScript_Reg_Write(TIMER0->MATCH, 0x123);
    AScript_Reg_Write_Msk(TIMER0->PRESCALE_WR, 0x0F00, 0x0500);
    AScript_Set_Flags(TIMER0->CR, TIMER0_CR_RST_Msk);
    AScript_Set_Flags(TIMER0->CR, TIMER0_CR_S_Msk | TIMER0_CR_IDR_Msk);

    AScript_Wait_Flags(TIMER0->SR, TIMER0_SR_UN_Msk);
    AScript_Clr_Flags(TIMER0->SR, TIMER0_SR_UN_Msk);
    return 0;
}
