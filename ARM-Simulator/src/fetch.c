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

// reads from the instruction memory and updates the instruction register
// R[15] is pc, contains address

uint32_t fetch(armsim_cpu *cpu)
{
    cpu->R[REG_PC] = (cpu->R[REG_PC] == -1ul) ? 0 : cpu->R[REG_PC];
    cpu->instruction_word = read_word(cpu->MEM_INST, cpu->R[REG_PC]);
    cpu->R[REG_PC] += 4;

    dbg("Fetch (pc= 0x%08X): 0x%X (new)\n", cpu->R[REG_PC] - 4, cpu->instruction_word);
    dbg("\n\n");

    return cpu->instruction_word;
}
