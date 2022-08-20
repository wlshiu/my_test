/*
    GDB RSP and ARM Simulator

    Copyright (C) 2015 Wong Yan Yin, <jet_wong@hotmail.com>,
    Jackson Teh Ka Sing, <jackson_dmc69@hotmail.com>

    This file is part of GDB RSP and ARM Simulator.

    This program is free software, you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with This program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "MLA.h"
#include <assert.h>
#include "ARMRegisters.h"
#include "getAndSetBits.h"
#include "StatusRegisters.h"
#include "ModifiedImmediateConstant.h"
#include "ITandHints.h"
#include "ConditionalExecution.h"
#include <stdio.h>


/*Multiply Accumulate Encoding T1
    MLA<c> <Rd>,<Rn>,<Rm>,<Ra>

   31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
  |1   1  1  1  1| 0  1  1  0| 0  0  0|     Rn    |     Ra    |    Rd   |0 0 0 0|   Rm  |

where:
        <c><q>        See Standard assembler syntax fields on page A6-7.

        <Rd>          Specifies the destination register.

        <Rn>          Specifies the register that contains the first operand.

        <Rm>          Specifies the register that contains the second operand.

        <Ra>          Specifies the register containing the accumulate value.
*/
void MLAT1(uint32_t instruction)
{
    uint32_t Rm = getBits(instruction, 3, 0);
    uint32_t Rd = getBits(instruction, 11, 8);
    uint32_t Rn = getBits(instruction, 19, 16);
    uint32_t Ra = getBits(instruction, 15, 12);

    if(inITBlock())
    {
        if( checkCondition(cond) )
            writeToCoreRegisters(Rd, coreReg[Rn] * coreReg[Rm] + coreReg[Ra] );

        shiftITState();
    }
    else
        writeToCoreRegisters(Rd, coreReg[Rn] * coreReg[Rm] + coreReg[Ra] );

    coreReg[PC] += 4;
}


