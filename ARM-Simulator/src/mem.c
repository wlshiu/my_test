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

void mem(armsim_cpu *cpu)
{
    if (cpu->is_datatrans)
    {
        uint8_t immediate = 1 - ((cpu->instruction_word & 0x02000000) >> 25);
        uint16_t offset;

        if (immediate)
        {
            offset = (cpu->instruction_word & 0x0FFF);
        }
        else
        {
            cpu->register2 = (cpu->instruction_word & 0x0000000F);
            cpu->operand2 = cpu->R[cpu->register2];
            shift_operand2(cpu);
            offset = cpu->operand2;
        }

        if (cpu->store_true)         // STR instruction
        {
            write_word(cpu->MEM_HEAP, cpu->R[cpu->register1] + offset, cpu->R[cpu->register_dest]);

            dbg("Memory                :       Store to address[0x%08x] = 0x%x\n", cpu->R[cpu->register1] + offset, cpu->R[cpu->register_dest]);
        }
        else if (cpu->load_true)     // LDR instruction
        {
            cpu->R[cpu->register_dest] = read_word(cpu->MEM_HEAP, cpu->R[cpu->register1] + offset);

            dbg("Memory                :       Load to R%u = 0x%x (from mem 0x%08x)\n",
                cpu->register_dest,
                cpu->R[cpu->register_dest],
                cpu->R[cpu->register1] + offset);
        }
    }
    else
    {
        dbg("Memory                :       No memory operation\n");
    }
    return;
}
