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

void execute_data_proc(armsim_cpu *cpu)
{
    if (cpu->opcode == 0)
    {
        cpu->answer = cpu->operand1 & cpu->operand2;

        dbg("Execute    : AND 0x%x and 0x%x\n", cpu->operand1, cpu->operand2);
    }
    else if (cpu->opcode == 1)
    {
        cpu->answer = cpu->operand1 ^ cpu->operand2;

        dbg("Execute    : XOR 0x%x and 0x%x\n", cpu->operand1, cpu->operand2);
    }
    else if (cpu->opcode == 2)
    {
        cpu->answer = cpu->operand1 - cpu->operand2;

        dbg("Execute    : SUBTRACT 0x%x and 0x%x\n", cpu->operand1, cpu->operand2);
    }
    else if (cpu->opcode == 4)
    {
        cpu->answer = cpu->operand1 + cpu->operand2;

        dbg("Execute    : ADD 0x%x and 0x%x\n", cpu->operand1, cpu->operand2);
    }
    else if (cpu->opcode == 5)
    {
        cpu->answer = cpu->operand1 + cpu->operand2 + 1;

        dbg("Execute    : ADD with Carry 0x%x and 0x%x\n", cpu->operand1, cpu->operand2);
    }
    else if (cpu->opcode == 10)
    {
        cpu->answer = cpu->operand1 - cpu->operand2;
        update_flags(cpu);

//        dbg("EXECUTE : Operation is : SUBTRACT with SET FLAG\n");
        dbg("Execute    : SUBTRACT 0x%x and 0x%x\n", cpu->operand1, cpu->operand2);
    }
    else if (cpu->opcode == 12)
    {
        cpu->answer = cpu->operand1 | cpu->operand2;

        dbg("Execute    : OR 0x%x and 0x%x\n", cpu->operand1, cpu->operand2);
    }
    else if (cpu->opcode == 13)
    {
        cpu->answer = cpu->operand2;

        dbg("Execute    : MOVE 0x%x to R%u\n", cpu->operand1, cpu->register_dest);

    }
    else if (cpu->opcode == 15)
    {
        cpu->answer = ~(cpu->operand2);

        dbg("Execute    : NOT 0x%x\n", cpu->operand2);
    }
    else
    {
        dbg("Opcode wrong in case of execute_data_proc, given is %d\n", cpu->opcode);
    }

    return;
}
