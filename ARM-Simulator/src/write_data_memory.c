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

//writes the data memory in "data_out.mem" file

void write_data_memory(armsim_cpu *var)
{
    FILE *fp;
    uint32_t i;
    fp = fopen("data_out.mem", "w");
    if(fp == NULL)
    {
        printf("Error opening dataout.mem file for writing\n");
        return;
    }

    for(i = 0; i < 4000; i = i + 4)
    {
        fprintf(fp, "%x %x\n", i, read_word(var->MEM_HEAP, i));
    }
    fclose(fp);
}
