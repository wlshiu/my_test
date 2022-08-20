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

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "ServeRSP.h"
#include "Packet.h"
#include "RemoteSerialProtocol.h"
#include "CException.h"
#include "ErrorSignal.h"

/***************************************************************************
 * Serve remote serial protocol to handle every specific packet sent by gdb
 *
 * Input:
 *      data        string of data receive from gdb client
 *
 * Return:
 *      packet      string of data reply to gdb
 ***************************************************************************/
char *serveRSP(char *data)
{
    CEXCEPTION_T errorSignal;
    char *packet;

    Try
    {
        switch(data[1])
        {
            case 'q':   /* General query */
                packet = handleQueryPacket(data);
                break;
            case 'v':   /*  */
                packet = gdbCreateMsgPacket("");
                break;
            case '?':   /* Indicate the reason the target halted */
                packet = handleException();
                // packet = gdbCreateMsgPacket("S05");
                break;
            case 'c':   /* Continue */
                packet = cont(data);
                break;
            case 's':   /* Step */
                packet = step(data);
                break;
            case 'g':   /* Read all register */
                packet = readAllRegister(data);
                break;
            case 'p':   /* Read single register */
                packet = readSingleRegister(data);
                break;
            case 'P':   /* Write single register */
                packet = writeSingleRegister(data);
                break;
            case 'G':   /* Write all register */
                packet = writeAllRegister(data);
                break;
            case 'm':   /* Read memory */
                packet = readMemory(data);
                break;
            case 'M':   /* Write memory */
                packet = writeMemory(data);
                break;
            // case 'X':   /* Write data to memory */
            /* writeMemory(data);
            packet = gdbCreateMsgPacket("OK");
        break; */   //not supported right now
            case 'z':   /* Remove breakpoint or watchpoint */
                packet = removeBreakpointOrWatchpoint(data);
                break;
            case 'Z':   /* Insert breakpoint or watchpoint */
                packet = insertBreakpointOrWatchpoint(data);
                break;
            case 'H':   /* Set thread */
                packet = gdbCreateMsgPacket("");
                break;
            case 'R':   /* Reset */
                break;
            case 'k':   /* Kill request */
                packet = gdbCreateMsgPacket("");
                break;
            default:
                packet = gdbCreateMsgPacket("");
                break;
        }
    }
    Catch(errorSignal)
    {
        char asciiString[4] = "";

        printf("Error signal: %d\n", errorSignal);
        sprintf(asciiString, "E%02d", errorSignal);
        packet = gdbCreateMsgPacket(asciiString);
    }

    return packet;
}