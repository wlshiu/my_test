/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/05/27
 * @license
 * @description
 */


#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static HANDLE       g_hComm;
static int          g_baud_rate = 0;
static char         g_com_name[64] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_task_listener(void *argv)
{
    while(1)
    {
        BOOL    Status;             // Status of the various operations
        char    TempChar = 0;
        DWORD   NoBytesRead = 0;    // Bytes read by ReadFile()

        Status = ReadFile(g_hComm,
                          &TempChar,
                          sizeof(TempChar),
                          &NoBytesRead,
                          NULL);
        if( NoBytesRead > 0 )
        {
            putc(TempChar, stdout);
        }
    }

    pthread_exit(0);
    return 0;
}

static void
_usage(char *pPorg)
{
    printf("usage: %s [COM1] [11520] [path]\n", pPorg);

    exit(-1);
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    int         rval = 0;
    FILE        *fin = 0;
    uint8_t     *pBuf = 0;

    do {
        pthread_t   t;
        BOOL        Status;

        if( argc < 4 )
        {
            _usage(argv[0]);
            break;
        }

        snprintf(g_com_name, sizeof(g_com_name), "\\\\.\\%s", argv[1]);
        g_hComm = CreateFile(g_com_name,                       // Name of the Port to be Opened
                             GENERIC_READ | GENERIC_WRITE,      // Read/Write Access
                             0,                                 // No Sharing, ports cant be shared
                             NULL,                              // No Security
                             OPEN_EXISTING,                     // Open existing port only
                             0,                                 // Non Overlapped I/O
                             NULL);                             // Null for Comm Devices
        if( g_hComm == INVALID_HANDLE_VALUE )
        {
            printf("\n   Error! - Port %s can't be opened", g_com_name);
            rval = -1;
            break;
        }

        {
            DCB             dcbSerialParams = {0};          // Initializing DCB structure
            COMMTIMEOUTS    timeouts = {0};

            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

            g_baud_rate = atoi(argv[2]);
            g_baud_rate = (g_baud_rate == 9600) ? CBR_9600 : g_baud_rate;
            g_baud_rate = (g_baud_rate == 115200) ? CBR_115200 : g_baud_rate;

            dcbSerialParams.BaudRate = g_baud_rate;   // Setting BaudRate = 9600
            dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
            dcbSerialParams.StopBits = ONESTOPBIT;    // Setting StopBits = 1
            dcbSerialParams.Parity   = NOPARITY;      // Setting Parity = None

            Status = SetCommState(g_hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB
            if( Status == FALSE )
            {
                printf("\n   Error! in Setting DCB Structure");
                rval = -1;
                break;
            }


            timeouts.ReadIntervalTimeout         = -1;
            timeouts.ReadTotalTimeoutConstant    = 50;
            timeouts.ReadTotalTimeoutMultiplier  = 10;
            timeouts.WriteTotalTimeoutConstant   = 500;
            timeouts.WriteTotalTimeoutMultiplier = 0;
            if( SetCommTimeouts(g_hComm, &timeouts) == FALSE )
            {
                printf("\n   Error! in Setting Time Outs");
                rval = -1;
                break;
            }
        }

        pthread_create(&t, 0, _task_listener, 0);

        {
            int         filesize = 0;
            DWORD       dNoOFBytestoWrite;              // No of bytes to write into the port
            DWORD       dNoOfBytesWritten = 0;          // No of bytes written to the port

            if( !(fin = fopen(argv[3], "rb")) )
            {
                printf("open %s fail \n", argv[3]);
                rval = -1;
                break;
            }

            fseek(fin, 0ul, SEEK_END);
            filesize = ftell(fin);
            fseek(fin, 0ul, SEEK_SET);

            if( !(pBuf = malloc(filesize)) )
            {
                printf("malloc %u fail \n", filesize);
                rval = -1;
                break;
            }
            fread(fin, 1, filesize, fin);

            fclose(fin);
            fin = 0;

            dNoOFBytestoWrite = filesize;

            Status = WriteFile(g_hComm,             // Handle to the Serialport
                               pBuf,                // Data to be written to the port
                               dNoOFBytestoWrite,   // No of bytes to write into the port
                               &dNoOfBytesWritten,  // No of bytes written to the port
                               NULL);
            if( Status == FALSE || dNoOfBytesWritten != dNoOFBytestoWrite )
            {
                printf("send data (%lu/%lu) from %s fail \n",
                       dNoOfBytesWritten, dNoOFBytestoWrite, g_com_name);
                rval = -1;
                break;
            }
        }
    } while(0);

    if( fin )   fclose(fin);
    if( pBuf )  free(pBuf);

    CloseHandle(g_hComm);

    _getch();
    return rval;
}
