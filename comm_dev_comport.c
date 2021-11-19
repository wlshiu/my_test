/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file comm_dev_comport.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/06/09
 * @license
 * @description
 */

#if defined(WIN32)
#include <stdbool.h>
#include "comm_dev.h"
#include <windows.h>
#include <unistd.h>         //Used for UART
#include "log.h"

#include <pthread.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_UART_RX_BUF_POW2             20
#define CONFIG_UART_RX_BUF_SIZE             (0x1ul << CONFIG_UART_RX_BUF_POW2)

/**
 *  CBR_9600, CBR_19200
 */
#define CONFIG_BAUD_RATE                    CBR_19200//CBR_9600

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct uart_dev
{
    HANDLE              fd_uart;
    volatile uint32_t   rd_idx;
    volatile uint32_t   wr_idx;

    int                 is_rx_running;
    uint8_t             *pRx_buf;
    long                rx_buf_len;

} uart_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t      g_rx_buf[CONFIG_UART_RX_BUF_SIZE] = {0};

static uart_dev_t   g_uart_dev = {};
static uint32_t     g_baudrate = CBR_19200;

static HANDLE       g_Mutex;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t __stdcall
_uart_data_listener(PVOID pM)
{
    uart_dev_t   *pDev = (uart_dev_t*)pM;

    msg("COMPort listener start\n\n");

    pDev->is_rx_running = true;

    while( pDev->is_rx_running )
    {
        DWORD       len = 0;
        DWORD       dwWaitResult;
        uint32_t    rd_idx = 0;
        uint32_t    wr_idx = 0;
        uint32_t    pos = 0;

        Sleep(1);

        dwWaitResult = WaitForSingleObject(g_Mutex,   // handle to mutex
                                           INFINITE); // no time-out interval
        if( dwWaitResult != WAIT_OBJECT_0 )
            continue;

        rd_idx = pDev->rd_idx;
        wr_idx = pDev->wr_idx;

        pos = (wr_idx + 1) & (pDev->rx_buf_len - 1);
        if( pos == rd_idx )
        {
            Sleep(5);
            continue;
        }

        ReadFile(pDev->fd_uart,               // Handle of the Serial port
                 &pDev->pRx_buf[wr_idx],      // Temporary character
                 (pDev->rx_buf_len - wr_idx), // Size of TempChar
                 &len,                        // Number of bytes read
                 NULL);

        if( len < 0 )
        {
            err("uart read fail\n");
        }
        else if( len )
        {
            wr_idx += len;
            pDev->wr_idx = (uint32_t)wr_idx;
        }

        ReleaseMutex(g_Mutex);
    }

    return 0;
}

static void
_usleep(unsigned int usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * (__int64)usec);

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

static comm_handle_t
_comport_init(comm_cfg_t *pCfg)
{
    uart_dev_t     *pHDev = 0;
    do {
        DCB         dcbSerialParams = {}; // Initializing DCB structure
        HANDLE      uart_rx_handler;
        COMMTIMEOUTS CommTimeouts = {};
        char        UartName[100];
        CHAR        *pcCommPort;
        BOOL        fSuccess;

        sprintf((char *)UartName, "\\\\.\\%s", (const char *)pCfg->comport.pComport_name);

        switch( pCfg->comport.baudrate )
        {
            case 9600:   g_baudrate = CBR_9600;      break;
            case 19200:  g_baudrate = CBR_19200;     break;
            case 57600:  g_baudrate = CBR_57600;     break;
            default:
            case 115200: g_baudrate = CBR_115200;    break;
        }

        g_Mutex = CreateMutex(NULL,  // default security attributes
                              FALSE, // initially not owned
                              NULL); // unnamed mutex
        if( g_Mutex == NULL )
        {
            msg("CreateMutex() error %u\n", (unsigned int)GetLastError());
            break;
        }

        msg("\ncomport '%s' (%d) init\n", UartName, pCfg->comport.baudrate);
        pcCommPort = UartName;
        g_uart_dev.fd_uart = CreateFile(pcCommPort,                     //port name
                                        GENERIC_READ | GENERIC_WRITE,   //Read/Write
                                        0,                              // No Sharing
                                        NULL,                           // No Security
                                        OPEN_EXISTING,                  // Open existing port only
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);                          // Null for Comm Devices

        if( g_uart_dev.fd_uart == INVALID_HANDLE_VALUE )
        {
            msg("comport create fail\n");
            break;
        }

        g_uart_dev.is_rx_running = false;
        g_uart_dev.pRx_buf       = g_rx_buf;
        g_uart_dev.rx_buf_len    = sizeof(g_rx_buf);
        uart_rx_handler = CreateThread(NULL,       // default security attributes
                                       0,          // use default stack size
                                       (LPTHREAD_START_ROUTINE)_uart_data_listener, // thread function name
                                       (void*)&g_uart_dev,       // argument to thread function
                                       0,          // use default creation flags
                                       NULL);      // returns the th

        if (uart_rx_handler == NULL)
        {
            msg("CreateThread fail\n");
            break;
        }

        dcbSerialParams.DCBlength = sizeof(DCB);

        fSuccess = GetCommState(g_uart_dev.fd_uart, &dcbSerialParams);

        if (!fSuccess)
        {
            //  Handle the error.
            msg("GetCommState failed with error %ld.\n", GetLastError());
            break;
        }

        //  Fill in some DCB values and set the com state:
        //  57,600 bps, 8 data bits, no parity, and 1 stop bit.
        dcbSerialParams.BaudRate = g_baudrate;  //  baud rate
        dcbSerialParams.ByteSize = 8;           //  data size, xmit and rcv
        dcbSerialParams.Parity = NOPARITY;      //  parity bit
        dcbSerialParams.StopBits = ONESTOPBIT;  //  stop bit
        dcbSerialParams.fBinary = 1;            // force binary mode
        dcbSerialParams.fParity = 0;            // no parity check

        dcbSerialParams.fOutxCtsFlow = 0;    // Disable CTS monitoring
        dcbSerialParams.fOutxDsrFlow = 0;    // Disable DSR monitoring
        dcbSerialParams.fDtrControl = 0;     // Disable DTR monitoring
        dcbSerialParams.fOutX = 0;           // Disable XON/XOFF for transmission
        dcbSerialParams.fInX = 0;            // Disable XON/XOFF for receiving
        dcbSerialParams.fRtsControl = 0;     // Disable RTS (Ready To Send)
        dcbSerialParams.fDsrSensitivity = 0;
        dcbSerialParams.fNull = 0;

        fSuccess = SetCommState(g_uart_dev.fd_uart, &dcbSerialParams);
        if (!fSuccess)
        {
            //  Handle the error.
            msg("SetCommState failed with error %ld.\n", GetLastError());
            break;
        }

        //  Get the comm config again.
        fSuccess = GetCommState(g_uart_dev.fd_uart, &dcbSerialParams);

        if (!fSuccess)
        {
            //  Handle the error.
            msg("GetCommState failed with error %ld.\n", GetLastError());
            break;
        }

        fSuccess = GetCommTimeouts(g_uart_dev.fd_uart, &CommTimeouts);
        if (!fSuccess)
        {
            //  Handle the error.
            msg("GetCommTimeouts failed with error %ld.\n", GetLastError());
            break;
        }

        CommTimeouts.ReadIntervalTimeout = -1;
        CommTimeouts.ReadTotalTimeoutConstant = 0;
        CommTimeouts.ReadTotalTimeoutMultiplier = 0;
        CommTimeouts.WriteTotalTimeoutConstant = 500;
        CommTimeouts.WriteTotalTimeoutMultiplier = 0;

        fSuccess = SetCommTimeouts(g_uart_dev.fd_uart, &CommTimeouts);
        if (!fSuccess)
        {
            //  Handle the error.
            msg("SetCommTimeouts failed with error %ld.\n", GetLastError());
            break;
        }

        pHDev = &g_uart_dev;
    } while(0);

    return (comm_handle_t)pHDev;
}

static int
_comport_send_bytes(
    comm_handle_t   pHandle,
    uint8_t         *pData,
    int             data_len)
{
    int     rval = 0;
    do {
        uart_dev_t     *pHDev = (uart_dev_t*)pHandle;
        int             count = 0;
        DWORD           dNoOfBytesWritten = 0;     // No of bytes written to the port

        if( !pHDev )   break;

        while( data_len-- )
        {
            count = WriteFile(pHDev->fd_uart,       // Handle to the Serial port
                              pData++,                // Data to be written to the port
                              1,             //No of bytes to write
                              &dNoOfBytesWritten,   //Bytes written
                              NULL);
            if( dNoOfBytesWritten < 0 || (dNoOfBytesWritten != 1) )
            {
                rval = -1;
                break;
            }

        #if 1
            _usleep(300);
        #else
            Sleep(1);
        #endif
        }
    } while(0);

    return rval;
}

static int
_comport_recv_bytes(
    comm_handle_t   pHandle,
    uint8_t         *pData,
    int             *pData_len)
{
    int     rval = -1;
    do {
        uart_dev_t     *pHDev = (uart_dev_t*)pHandle;
        int             len = 0;
        int             buf_len = 0;
        int             rd_idx = 0;
        int             wr_idx = 0;

        DWORD       dwWaitResult;

        buf_len = *pData_len;
        *pData_len = 0;

        dwWaitResult = WaitForSingleObject(g_Mutex,   // handle to mutex
                                           INFINITE); // no time-out interval

        if( dwWaitResult != WAIT_OBJECT_0 )
            break;

        rd_idx = pHDev->rd_idx;
        wr_idx = pHDev->wr_idx;

        if( !pHDev )   break;

        while( 1 )
        {
            if( rd_idx == wr_idx )      break;
            if( buf_len == len )        break;

            *pData++ = pHDev->pRx_buf[rd_idx];
            rd_idx = (rd_idx + 1) & (pHDev->rx_buf_len - 1);

            len++ ;
        }

        pHDev->rd_idx = rd_idx;

        ReleaseMutex(g_Mutex);

        *pData_len = len;
        rval = 0;
    } while(0);
    return rval;
}

static int
_comport_reset_buf(
    comm_handle_t   pHandle,
    uint8_t         *pBuf,
    uint32_t        buf_len)
{
    int             rval = 0;
    do {
        uart_dev_t     *pHDev = (uart_dev_t*)pHandle;

        if( !pHDev )   break;

        pHDev->rd_idx = 0;
        pHDev->wr_idx = 0;

    } while(0);
    return rval;
}

static int
_comport_get_state(
    comm_handle_t   pHandle,
    comm_state_t    state)
{
    int             rval = 0;
    do {
        uart_dev_t     *pHDev = (uart_dev_t*)pHandle;
        uint32_t        rd_idx = pHDev->rd_idx;
        uint32_t        wr_idx = pHDev->wr_idx;

        if( !pHDev )   break;

        if( state == COMM_STATE_GET_RX_EVENT )
        {
            // if empty, no rx_event
            rval = !(rd_idx == wr_idx);
        }
    } while(0);
    return rval;
}

static int
_comport_deinit(
    comm_handle_t pHandle)
{
    int     rval = 0;
    do {
        uart_dev_t     *pHDev = (uart_dev_t*)pHandle;

        if( !pHDev )   break;

        pHDev->is_rx_running = false;

        Sleep(200);

        CloseHandle(pHDev->fd_uart);    // Closing the Serial Port
        memset(&g_uart_dev, 0x0, sizeof(g_uart_dev));
    } while(0);

    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
comm_dev_desc_t      g_comm_dev_comport =
{
    .init          = _comport_init,
    .send_bytes    = _comport_send_bytes,
    .recv_bytes    = _comport_recv_bytes,
    .reset_buf     = _comport_reset_buf,
    .get_state     = _comport_get_state,
    .deinit        = _comport_deinit,
};
#endif
