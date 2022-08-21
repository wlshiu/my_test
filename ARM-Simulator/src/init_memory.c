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

void init_memory(armsim_cpu *cpu)
{
    int i;

    for (i = 0; i < 16; i++)        //Resting registers
        cpu->R[i] = 0;

     cpu->R[REG_PC] = -1ul;

    for (i = 0; i < 4000; i++)      //Reseting instruction memory
        cpu->MEM_INST[i] = 0;

    for (i = 0; i < 4000; i++)      //Reseting heap memory
        cpu->MEM_HEAP[i] = 0;

    cpu->instruction_word = 0;
    cpu->operand1 = 0xFFFFFFFFul;
    cpu->operand2 = 0xFFFFFFFFul;
    cpu->answer = 0;
    cpu->register1 = 0;
    cpu->register2 = 0;
    cpu->register_dest = 0;
    cpu->condition = 0;
    cpu->is_dataproc = 0;
    cpu->is_branch = 0;
    cpu->opcode = 0;
    cpu->immediate = 0;
    cpu->branch_true = 0;
    cpu->is_datatrans = 0;
    cpu->load_true = 0;
    cpu->store_true = 0;
    cpu->swi_exit = 0;
    return;
}
