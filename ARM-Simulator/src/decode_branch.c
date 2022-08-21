/*=============================================================================

    This file is part of the Functional Simulator for ARM Processor. It was
    developed as a part of Computer Architecture Course CS112 offered at
    IIIT Delhi.

    This is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    It is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

=============================================================================*/
/******************************************************************************

    Copyright (C) 2015 Kushagra Singh ; kushagra14056@iiitd.ac.in
    Copyright (C) 2015 Lohitaksh Parmar ; lohitaksh14059@iiitd.ac.in

******************************************************************************/

#include <myARMSim.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void decode_branch(armsim_cpu *cpu)
{
    char    *pInstruction_name = 0;

    cpu->branch_true = 1;
    cpu->condition   = (cpu->instruction_word & 0xF0000000) >> 28;  // 31, 30, 29, 28

    if (cpu->condition == 0 && cpu->Z)          // BEQ, zero flag is true
    {
        pInstruction_name = "BEQ";
    }
    else if (cpu->condition == 1 && !(cpu->Z))  // BNE, zero flag is false
    {
        pInstruction_name = "BNE";
    }
    else if (cpu->condition == 10 && !(cpu->N)) // BGE, not negative
    {
        pInstruction_name = "BGE";
    }
    else if (cpu->condition == 11 && cpu->N)    // BLT, negative flag is true
    {
        pInstruction_name = "BLT";
    }
    else if (cpu->condition == 12 && !(cpu->N) && !(cpu->Z))    // BGT, not negative, not zero
    {
        pInstruction_name = "BGT";
    }
    else if (cpu->condition == 13 && (cpu->N || cpu->Z))        // BLE, either negative or zero
    {
        pInstruction_name = "BLE";
    }
    else if (cpu->condition == 14 )             // Unconditional
    {
        pInstruction_name = "B"; // B, BL, BX, BLX
    }
    else
        cpu->branch_true = 0;

    if( cpu->branch_true )
        dbg("Execute    : %s\n", pInstruction_name);
//    else
//        dbg("Execute    : NOT Taking branch\n");

    return;
}
