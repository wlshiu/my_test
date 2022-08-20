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

    You should have received a copy of the GNU General Public License
    along with FLINT; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2015 Kushagra Singh ; kushagra14056@iiitd.ac.in
    Copyright (C) 2015 Lohitaksh Parmar ; lohitaksh14059@iiitd.ac.in

******************************************************************************/

#include <myARMSim.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    char *prog_mem_file;

    if(argc < 2)
    {
        printf("Incorrect number of arguments. Please invoke the simulator with the mem file as an argument\n");
        system("pause");
        exit(1);
    }

    armsimvariables *var = (armsimvariables *) malloc(sizeof(armsimvariables));

    init_memory(var);
    load_program_memory(argv[1], var);
    run_armsim(var);
//    write_data_memory(var);
//    system("pause");
    return 0;
}

