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

void decode_dataproc(armsim_cpu *cpu)
{
    cpu->opcode    = (cpu->instruction_word & 0x1E00000) >> 21;    // 24, 23, 22, 21
    cpu->immediate = (cpu->instruction_word & 0x02000000) >> 25;   // 25

    cpu->register1 = (cpu->instruction_word & 0x000F0000) >> 16;      // 19, 18, 17, 16
    cpu->operand1  = cpu->R[cpu->register1];
    cpu->register_dest = (cpu->instruction_word & 0x0000F000) >> 12;  // 15, 14, 13, 12

    if( cpu->immediate )         // 11, 10, 9, 8 ->shift; 7 - 0 -> value
    {
        int shift = (cpu->instruction_word & 0x00000F00) >> 8;

        cpu->operand2 = (cpu->instruction_word & 0x000000FF);
        cpu->operand2 <<= shift;
    }
    else if( !cpu->immediate )   //
    {
        cpu->register2 = (cpu->instruction_word & 0x0000000F);
        cpu->operand2 = cpu->R[cpu->register2];
        shift_operand2(cpu);
    }

    dbg("Decode                : Rn= R%u, Rm= R%u, Rd= R%u\n", cpu->register1, cpu->register2, cpu->register_dest);
    return;
}
