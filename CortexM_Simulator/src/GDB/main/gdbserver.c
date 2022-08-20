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
#include <unistd.h>
#include "ServeRSP.h"
#include "ARMRegisters.h"
#include "MemoryBlock.h"
#include "gdbserver.h"
// #include "dll/gdbserverDLL.h"
#include "RemoteSerialProtocol.h"
#include "ARMSimulator.h"
#include "State.h"
#include "stateRSP.h"
#include "ConfigurationDetail.h"
#include "LoadElf.h"

#ifdef  __MINGW32__
/****************Initialize Winsock.****************/
void winsockInit()
{
    printf( "\n1. Initialising Winsock..............." );
    WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if ( iResult != NO_ERROR )
    {
        displayErrorMsg("WSAStartup()");
        WSACleanup();
        return;
    }
    else
        printf( "Initialised\n" );
}

#endif

/****************Create a socket.****************/
void createSocket(SOCKET *sock)
{
    printf( "2. Creating socket...................." );
    *sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( *sock == INVALID_SOCKET )
    {
        displayErrorMsg("socket()");
#ifdef  __MINGW32__
        WSACleanup();
#endif
        return;
    }
    else
        printf( "Socket created\n" );
}

/****************Bind the socket.****************/
void bindSocket(SOCKET *sock, int port)
{
    printf( "3. Binding socket....................." );
    struct sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr( LOCAL_HOST_ADD );
    service.sin_port = htons( port );
    if ( bind( *sock, (SOCKADDR *) &service, sizeof(service) ) == SOCKET_ERROR )
    {
        displayErrorMsg("bind()");
#ifdef  __MINGW32__
        closesocket(*sock);
#elif  __linux__
        close(*sock);
#endif
        return;
    }
    else
        printf( "Bind done\n" );
}

/****************Listen on the socket.****************/
void listenSocket(SOCKET *sock)
{
    printf( "4. Listening to socket................" );
    if ( listen( *sock, 1 ) == SOCKET_ERROR )
        printf( ">>>Error listening on socket\n");
    else
        printf( "Listening...\n" );
}

/****************Accept connections.****************/
void waitingForConnection(SOCKET *sock, int port)
{
#ifdef  __MINGW32__
    winsockInit();
#endif
    createSocket(sock);
    bindSocket(sock, port);
    listenSocket(sock);

    printf( "5. Waiting on %s:%d..........", LOCAL_HOST_ADD, port );
    SOCKET acceptSocket;
    while (1)
    {
        acceptSocket = SOCKET_ERROR;
        while ( acceptSocket == SOCKET_ERROR )
            acceptSocket = accept( *sock, NULL, NULL );

        if ( acceptSocket == INVALID_SOCKET )
            displayErrorMsg("accept()");
        else
            printf( "Connection accepted\n" );
        *sock = acceptSocket;
        break;
    }
}

/****************Send data.****************/
int sendBuffer(SOCKET *sock, char *sendbuf)
{
    int bytesSent;

    bytesSent = send( *sock, sendbuf, strlen(sendbuf), 0 );
    printf( "\nBytes Sent: %ld\n", bytesSent );
    printf( "Reply: %s\n", sendbuf );

    return bytesSent;
}

/****************Receive data.****************/
int receiveBuffer(SOCKET *sock, char *recvbuf)
{
    int bytesRecv = SOCKET_ERROR;

    bytesRecv = recv( *sock, recvbuf, PACKET_SIZE, 0 );
    printf( "\nBytes Recv: %ld\n", bytesRecv );

    return bytesRecv;
}

void initSimulator()
{
    initializeSimulator();
    initializeWatchpoint();
}

void displayErrorMsg(char *errorMsg)
{
#ifdef  __MINGW32__
    printf( ">>>Error at %s: %ld\n", errorMsg, WSAGetLastError() );
#elif  __linux__
    printf( ">>>%s\n", errorMsg );
#endif
}

void *retrieveData(char *dir, TypeOfDataToRetrieve dataToRetrieve)
{
    char *dirName, *str, elfPath[100], device[100];
    GdbServerInfo *gdbServerInfo;
    ConfigInfo configInfo = {.flashOrigin = 0,
                             .flashSize = 0,
                             .ramOrigin = 0,
                             .ramSize = 0
                            };

    if(dataToRetrieve == GDB_SERVER_INFO)
    {
        dirName = appendString(dir, "/GDBServerConfig.ini");
        gdbServerInfo = readGdbServerConfigFile(dirName, "r");
        destroyStr(dirName);

        return gdbServerInfo;
    }
    else if(dataToRetrieve == ELF_PATH)
    {
        dirName = appendString(dir, "/ElfLocation.txt");
        str = readFile(dirName, "r");
        destroyStr(dirName);

        return str;
    }
    else if(dataToRetrieve == ELF_DATA)
    {
        // Retrieve location of elf file
        str = (char *)retrieveData(dir, ELF_PATH);
        sscanf(str, "%s %s", elfPath, device);

        dirName = appendString(dir, "/config");
        readConfigfile(dirName, "r", &configInfo, device);

        loadElf(elfPath, configInfo.flashOrigin, configInfo.flashSize);
        destroyStr(dirName);
        destroyStr(str);

        return NULL;
    }
}

int main(int argc, const char *argv[])
{
    int i, portNumber = DEFAULT_PORT;
    char *str, *parentDirName;
    SOCKET sock;
    RspData rspData = {INITIAL, sock};
    GdbServerInfo *gdbServerInfo;

    for(i = 0; i < argc; i++)
    {
        if(argv[i][0] == ':')
            sscanf(argv[i], ":%d", &portNumber);
    }

    initSimulator();

    // Get the parent directory name
    parentDirName = getDirectoryName((char *)argv[0]);

    // Retrieve the host and port from a self created gdb config file
    gdbServerInfo = (GdbServerInfo *)retrieveData(parentDirName, GDB_SERVER_INFO);

    if(gdbServerInfo->port != -1)
        portNumber = gdbServerInfo->port;        // use default port if port unable to retrieve from GDBServerConfig

    // Connection from client
    waitingForConnection(&rspData.sock, portNumber);

    // Retrieve the data from elf file
    retrieveData(parentDirName, ELF_DATA);

    destroyStr(parentDirName);
    destroyGdbServerInfo(gdbServerInfo);

    int bytesSent;
    int bytesRecv;
    char *reply = NULL;
    char recvbuf[PACKET_SIZE] = "";
    rspData.state = INITIAL;

    while(1)
    {
        bytesRecv = receiveBuffer(&rspData.sock, recvbuf);

        if(bytesRecv != -1)
        {
            recvbuf[bytesRecv] = '\0';
            printf( "recvbuf: %s\n", recvbuf );
        }
        else
        {
            rspData.state = NACK;
            recvbuf[0] = '\0';
        }

        rspState(&rspData, recvbuf);
        // gdbserverMain(&rspData, recvbuf);

        if(rspData.state == KILL)
        {
            break;
        }
    }

#ifdef  __MINGW32__
    /****************Close our socket entirely****************/
    closesocket(rspData.sock);

    /****************Cleanup Winsock****************/
    WSACleanup();
#elif  __linux__
    close(rspData.sock);
#endif
}
