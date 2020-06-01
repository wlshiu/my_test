/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file serial_port_rx.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/04/17
 * @license
 * @description
 */


#if defined(__linux__)
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_DEFAULT_UART_BAUD_RATE       B115200


#define CONFIG_UART_RX_BUF_POW2             14
#define CONFIG_UART_RX_BUF_SIZE             (0x1 << CONFIG_UART_RX_BUF_POW2)
//=============================================================================
//                  Macro Definition
//=============================================================================
#define msg(str, ...)           printf(str, ## __VA_ARGS__)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct uart_dev
{
    int                 fd_uart;
    volatile uint16_t   rd_idx;
    volatile uint16_t   wr_idx;

    int                 is_rx_running;
    uint8_t             *pRx_buf;
    long                rx_buf_len;
} uart_dev_t;


typedef void*   comm_handle_t;
typedef struct comm_cfg
{
    char                *pDev_name;

    union {
        struct {
            unsigned long    port;
            unsigned long    baud_rate;
        } uart;

        struct {
            void    *pTunnel_info;
        } def;
    };

    uint8_t     *pBuf;
    uint32_t    buf_len;

} comm_cfg_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t      g_rx_buf[CONFIG_UART_RX_BUF_SIZE] = {0};

static uart_dev_t   g_uart_dev = {.fd_uart = -1, };
//=============================================================================
//                  Private Function Definition
//=============================================================================
static comm_handle_t
_uart_init(comm_cfg_t *pCfg)
{
    uart_dev_t     *pHDev = 0;
    do {
        struct termios      options;
        speed_t             baudrate = CONFIG_DEFAULT_UART_BAUD_RATE;

        g_uart_dev.fd_uart = open(pCfg->pDev_name, O_RDWR | O_NOCTTY | O_NDELAY);
        if( g_uart_dev.fd_uart < 0 )
            break;

        memset(&options, 0x0, sizeof(struct termios));

        g_uart_dev.is_rx_running = false;
        g_uart_dev.pRx_buf       = g_rx_buf;
        g_uart_dev.rx_buf_len    = sizeof(g_rx_buf);

        switch( pCfg->uart.baud_rate )
        {
            case 921600:    baudrate = B921600; break;
            case 460800:    baudrate = B460800; break;
            case 230400:    baudrate = B230400; break;
            case 115200:    baudrate = B115200; break;
            case 57600:     baudrate = B57600; break;
            case 9600:      baudrate = B9600; break;

            default:    break;
        }

        options.c_cflag = CS8 | CLOCAL | CREAD;
        options.c_iflag = 0;
        options.c_oflag = 0;
        // options.c_lflag = 0;

        msg("RX: baud rate: %lu\n\n", pCfg->uart.baud_rate);

        cfsetispeed(&options, baudrate);
        cfsetospeed(&options, baudrate);
        tcflush(g_uart_dev.fd_uart, TCIFLUSH);
        tcsetattr(g_uart_dev.fd_uart, TCSANOW, &options);

        pHDev = &g_uart_dev;
    } while(0);

    return (comm_handle_t)pHDev;
}

static int
_uart_deinit(comm_handle_t pHandle)
{
    int     rval = 0;
    do {
        uart_dev_t     *pHDev = (uart_dev_t*)pHandle;

        if( !pHDev )   break;

        pHDev->is_rx_running = false;

        usleep(10000);

        close(pHDev->fd_uart);
        memset(&g_uart_dev, 0x0, sizeof(g_uart_dev));
    } while(0);

    return rval;
}

static int
_uart_send_bytes(
    comm_handle_t   pHandle,
    uint8_t         *pData,
    int             data_len)
{
    int     rval = 0;
    do {
        uart_dev_t     *pHDev = (uart_dev_t*)pHandle;
        int             count = 0;

        if( !pHDev )   break;

        count = write(pHDev->fd_uart, pData, data_len);
        if( count < 0 || (count != data_len) )
            rval = -1;
    } while(0);
    return rval;
}

static int
_uart_recv_bytes(
    comm_handle_t   pHandle,
    uint8_t         *pData,
    int             *pData_len)
{
    int     rval = -1;
    do {
        uart_dev_t     *pHDev = (uart_dev_t*)pHandle;
        int             len = 0;
        int             buf_len = 0;
        int             rd_idx = pHDev->rd_idx;
        int             wr_idx = pHDev->wr_idx;

        buf_len = *pData_len;
        *pData_len = 0;

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

        *pData_len = len;
        rval = 0;
    } while(0);
    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
void usage(char *pProg)
{
    printf("usage: %s [dev_path] [115200] [file path]", pProg);
    exit(-1);
    return;
}

int main(int argc, char** argv)
{
    int             rval = 0;
    FILE            *fin = 0;
    comm_handle_t   hComm = 0;

    do {
        comm_cfg_t      cfg = {0};
        uart_dev_t      *pDev = &g_uart_dev;
        uint32_t        end_word = 0;

        if( argc < 3 )
        {
            usage(argv[0]);
            rval = -1;
            break;
        }

        cfg.pDev_name       = argv[1];
        cfg.uart.baud_rate  = atoi(argv[2]);
        cfg.uart.port       = 1;

        hComm = _uart_init(&cfg);
        if( !hComm )
        {
            printf("init fail \n");
            rval = -1;
            break;
        }

        if( !(fin = fopen(argv[3], "wb")) )
        {
            printf("open %s fail \n", argv[3]);
            rval = -1;
            break;
        }

        while(1)
        {
            int         len = 0;
            uint32_t    rd_idx = pDev->rd_idx;
            uint32_t    wr_idx = pDev->wr_idx;
            uint32_t    pos = 0;

            len = read(pDev->fd_uart, &pDev->pRx_buf[0], pDev->rx_buf_len);
            if( len < 0 )
            {
                printf("uart read fail\n");
            }
            else if( len )
            {
                #if 1
                fwrite(&pDev->pRx_buf[0], 1, len, fin);
                #else
                for(int i = 0; i < len; i++)
                {
                    printf("%02x ", pDev->pRx_buf[i]);
                }
                #endif

                end_word = (end_word << 8) | pDev->pRx_buf[len - 1];

                if( end_word == 0xCCCCCCCC )
                {
                    printf("done~~~~~\n");
                    break;
                }
            }
        }

    } while(0);

    _uart_deinit(hComm);

    if( fin )   fclose(fin);
    return rval;
}
#endif
