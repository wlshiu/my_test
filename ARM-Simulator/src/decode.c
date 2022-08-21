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

void decode(armsim_cpu *cpu)
{
    uint8_t temp;
    uint8_t shift;

    if( cpu->instruction_word == 0 )
    {
        dbg("Decode                : wait\n");
        return;
    }

    temp = (cpu->instruction_word & 0x0C000000) >> 26;    // 27, 26

    if( temp == 0 )
    {
        cpu->is_dataproc = 1;

        /* data processing */
        decode_dataproc(cpu);
    }
    else if( temp == 1 )
    {
        cpu->is_datatrans = 1;

        /* data transfer  */
        decode_datatrans(cpu);
    }
    else if( temp == 2 )
    {
        cpu->is_branch = 1;

        /* branch */
        decode_branch(cpu);
    }
    else if( temp == 3 )
    {
        /* S/w trigger interrupt */
        cpu->swi_exit = 1;
        dbg("Decode     :           => SWI_EXIT\n");
    }

    return;
}
