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

//writes the results back to register file
void write_back(armsim_cpu *cpu)
{
    if( cpu->is_dataproc && !(cpu->store_true || cpu->load_true) )
    {
        cpu->R[cpu->register_dest] = cpu->answer;

        dbg("Writeback             :       Write 0x%x to R%u\n", cpu->answer, cpu->register_dest);
    }
    else
    {
        dbg("Writeback             :       Nothing to Write-Back\n");
    }

//    dbg("\n");
    cpu->is_datatrans = 0;
    cpu->is_dataproc = 0;
    cpu->is_branch = 0;
    cpu->swi_exit = 0;

    cpu->load_true = 0;
    cpu->store_true = 0;
    cpu->branch_true = 0;
    return;
}

