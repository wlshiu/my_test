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

void execute_data_trans(armsim_cpu *cpu)
{
    cpu->register1 = ((cpu->instruction_word & 0x000F0000) >> 16);          // base address

    cpu->register_dest = ((cpu->instruction_word & 0x0000F000) >> 12);      // in case of STR, value to be stored

    // in case of LDR, designation register
    dbg("Execute    :       Nothing to execute\n");
    return;
}
