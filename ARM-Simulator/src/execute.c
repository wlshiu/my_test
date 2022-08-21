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

//executes the ALU operation based on ALUop
uint8_t execute(armsim_cpu *cpu)
{
    if (cpu->swi_exit == 1)
        return 0;

    if( cpu->instruction_word == 0 )
    {
        dbg("Execute               : wait\n");
        return 1;
    }

    else if (cpu->is_dataproc)
        execute_data_proc(cpu);
    else if (cpu->is_datatrans)
        execute_data_trans(cpu);
    else if (cpu->is_branch && cpu->branch_true)
        execute_branch(cpu);
    else
    {
        dbg("Execute               : wait\n");
    }

    return 1;
}
