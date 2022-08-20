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

#include "MOVImmediate.h"
#include "ITandHints.h"
#include "ConditionalExecution.h"
#include <stdio.h>


/*Move Immediate Encoding T1
        MOVS <Rd>,#<imm8>               Outside IT block.
        MOV<c> <Rd>,#<imm8>             Inside IT block.

  Note : This instruction can never move any negative value

   31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
  |0   0  1| 0  0|   Rd   |         imm8          |               unused                |

where:
        S         If present, specifies that the instruction updates the flags. Otherwise, the instruction does not
                  update the flags.

        <c><q>    See Standard assembler syntax fields on page A6-7.

        <Rd>      Specifies the destination register. It can only cover until R7 because of 3 bits

        <const>   Specifies the immediate value to be placed in <Rd>. The range of allowed values is 0-255 for
                  encoding T1

*/
void MOVImmediateT1(uint32_t instruction)
{
    uint32_t imm8 = getBits(instruction, 23, 16);
    uint32_t destinationRegister = getBits(instruction, 26, 24);

    if(inITBlock())
    {
        if( checkCondition(cond) )
            executeMOVImmediate(imm8, destinationRegister, 0);
        shiftITState();
    }
    else
        executeMOVImmediate(imm8, destinationRegister, 1);

    coreReg[PC] += 2;
}




/* Move Immediate Encoding T2

MOV{S}<c>.W <Rd>,#<const>

31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
|1  1  1  1 0 |i| 0  0  0  1  0 |S| 1  1  1  1  0 |  imm3  |    Rd   |     imm8      |

where:
          S         If present, specifies that the instruction updates the flags. Otherwise, the instruction does not
                    update the flags.

          <c><q>    See Standard assembler syntax fields on page A6-7.

          <Rd>      Specifies the destination register.

          <const>   Specifies the immediate value to be placed in <Rd>. The range of allowed values is 0-255 for
                    encoding T1 and 0-65535 for encoding T3. See Modified immediate constants in Thumb
                    instructions on page A5-15 for the range of allowed values for encoding T2.

          When both 32-bit encodings are available for an instruction, encoding T2 is preferred to
          encoding T3 (if encoding T3 is required, use the MOVW syntax)
*/
void MOVImmediateT2(uint32_t instruction)
{
    uint32_t imm8 = getBits(instruction, 7, 0);
    uint32_t Rd = getBits(instruction, 11, 8);
    uint32_t imm3 = getBits(instruction, 14, 12);
    uint32_t statusFlag = getBits(instruction, 20, 20);

    uint32_t i = getBits(instruction, 26, 26);
    uint32_t bit7 = getBits(instruction, 7, 7);
    uint32_t temp = (i << 3 ) | imm3;
    uint32_t modifyControl = (temp << 1) | bit7;

    uint32_t ModifiedConstant = ModifyImmediateConstant(modifyControl, imm8, 1);

    if(inITBlock())
    {
        if( checkCondition(cond) )
            executeMOVImmediate(ModifiedConstant, Rd, statusFlag);
        shiftITState();
    }
    else
        executeMOVImmediate(ModifiedConstant, Rd, statusFlag);

    coreReg[PC] += 4;
}



/* Move Immediate Encoding T3

MOVW<c> <Rd>,#<imm16>

31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
|1  1  1  1 0 |i| 1  0  0  1  0  0 |   imm4    |0|  imm3   |    Rd   |     imm8      |

This instruction will not affect the status register
largest value for the constant is 65536 which means 16 bits value

16 bits = imm4 | i | imm3 | imm8

where:
          S         If present, specifies that the instruction updates the flags. Otherwise, the instruction does not
                    update the flags.

          <c><q>    See Standard assembler syntax fields on page A6-7.

          <Rd>      Specifies the destination register.

          <const>   Specifies the immediate value to be placed in <Rd>. The range of allowed values is 0-255 for
                    encoding T1 and 0-65535 for encoding T3. See Modified immediate constants in Thumb
                    instructions on page A5-15 for the range of allowed values for encoding T2.

          When both 32-bit encodings are available for an instruction, encoding T2 is preferred to
          encoding T3 (if encoding T3 is required, use the MOVW syntax)
*/
void MOVImmediateT3(uint32_t instruction)
{
    uint32_t imm8 = getBits(instruction, 7, 0);
    uint32_t Rd = getBits(instruction, 11, 8);
    uint32_t imm3 = getBits(instruction, 14, 12);
    uint32_t imm4 = getBits(instruction, 19, 16);
    uint32_t i = getBits(instruction, 26, 26);

    uint32_t constant;

    constant = ( imm3 << 8 ) | imm8;
    constant = ( i << 11 ) | constant;
    constant = ( imm4 << 12) | constant;

    if(inITBlock())
    {
        if( checkCondition(cond) )
            executeMOVImmediate(constant, Rd, 0);
        shiftITState();
    }
    else
        executeMOVImmediate(constant, Rd, 0);

}



/*  This function will perform the move immediate

    Input:  immediate       the immediate going to move into Rd
            Rd              destination register
            S               indicator for affecting the flag or not

*/
void executeMOVImmediate(uint32_t immediate, uint32_t Rd, uint32_t S)
{
    writeToCoreRegisters(Rd, immediate);

    if(S == 1)
    {
        updateZeroFlag(coreReg[Rd]);
        updateNegativeFlag(coreReg[Rd]);
    }
}


