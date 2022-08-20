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

#define _tolowcase(chr)      (((chr) >='A' && (chr) <='Z') ? ((chr) + 32) : (chr))

static int strncasecmp(const char *s1, const char *s2, size_t n)
{
    if( n == 0 )    return 0;

    while( n-- != 0 && _tolowcase(*s1) == _tolowcase(*s2) )
    {
        if( n == 0 || *s1 == '\0' || *s2 == '\0' )
            break;
        s1++;
        s2++;
    }

    return _tolowcase(*(unsigned char*)s1) - _tolowcase(*(unsigned char*)s2);
}

uint32_t
ARM_cpu_run(armsimvariables *var, int count)
{
    static uint32_t     clock = 0;
    uint32_t    val = 0;

    while( count-- )
    {
        dbg("\n--------------------------------------------\n");
        dbg("Clock Cycle #: %d\n", clock++);
        dbg("--------------------------------------------\n");

        val = fetch(var);
        if (!val)   break;

        decode(var);
        val = execute(var);
        if (!val)   break;

        mem(var);
        write_back(var);
    }
    return val;
}

void run_armsim(armsimvariables *var)
{
    uint32_t    val;

#if 1
    char        cmd_line[50];

    while( 1 )
    {
        printf("\nARMSim> ");
        scanf("%s", cmd_line);
        if( strncasecmp(cmd_line, "Q", 1) == 0 )
        {
            printf("ARMSim: Simulation Stopped\n");
            break;
        }
        else if( strncasecmp(cmd_line, "n", strlen("n")) == 0 ||
                 strncasecmp(cmd_line, "next", strlen("next")) == 0 )
        {
            val = ARM_cpu_run(var, 1);
            if (!val)   break;
        }
        else if( strncasecmp(cmd_line, "sim", strlen("sim")) == 0 ||
                 strncasecmp(cmd_line, "simulate", strlen("simulate")) == 0 )
        {
            int     count = 0;
            printf("\n  Set cycles> ");
            scanf("%d", &count);

            val = ARM_cpu_run(var, count);
            if (!val)   break;
        }
        else
        {
            printf("\n Invalid Command: [ %s ]\n", cmd_line);
            printf("--------------------------------------------------------------------\n");
            printf("cmds supported:\n");
            printf("   [sim | Simulate <count>]  - to simulate <count> cycles\n"
//                   "   [d | Display]             - to display stage contents\n"
//                   "   [mem <address>]           - to show contents in memory <address>\n"
                   "   [n | next]                - proceed by one cycle\n");
            printf("--------------------------------------------------------------------\n");

        }

    }
#else
    while(var->R[REG_PC] < 4000)
    {
        val = fetch(var);
        if (!val)
            return;

        decode(var);
        val = execute(var);
        if (!val)
            return;

        mem(var);
        write_back(var);
    }
#endif
    return;
}



