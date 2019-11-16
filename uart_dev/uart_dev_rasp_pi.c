/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file uart_dev_pi.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/11
 * @license
 * @description
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uart_dev.h"
#include "pthread.h"
#include "log.h"


#if defined(__linux__)

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

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct pi_uart_dev
{
    int                 fd_uart;
    volatile uint16_t   rd_idx;
    volatile uint16_t   wr_idx;

    int                 is_rx_running;
    uint8_t             *pRx_buf;
    long                rx_buf_len;
    uart_dev_type_t     dev_type;
} pi_uart_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t      g_rx_buf[CONFIG_UART_RX_BUF_SIZE] = {0};

static pi_uart_dev_t   g_rasp_pi_uart_dev = {.fd_uart = -1, };

extern pthread_mutex_t     g_log_mtx;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_rasp_pi_uart_data_listener(void *argv)
{
    pi_uart_dev_t   *pDev = (pi_uart_dev_t*)argv;

    pthread_detach(pthread_self());

    pDev->is_rx_running = true;

    while( pDev->is_rx_running )
    {
        int         len = 0;
        uint32_t    rd_idx = pDev->rd_idx;
        uint32_t    wr_idx = pDev->wr_idx;
        uint32_t    pos = 0;

        pos = (wr_idx + 1) & (pDev->rx_buf_len - 1);
        if( pos == rd_idx )
        {
            usleep(5000);
            continue;
        }

        len = read(pDev->fd_uart, &pDev->pRx_buf[wr_idx], (pDev->rx_buf_len - wr_idx));
        if( len < 0 )
        {
            err("uart read fail\n");
        }
        else if( len )
        {
            wr_idx += len;
            pDev->wr_idx = (uint16_t)wr_idx;
        }
        else    usleep(10);
    }

    pthread_exit(NULL);
    return 0;
}

static uart_handle_t
_rasp_pi_uart_init(uart_cfg_t *pCfg)
{
    pi_uart_dev_t     *pHDev = 0;
    do {
        struct termios      options;
        speed_t             baudrate = CONFIG_DEFAULT_UART_BAUD_RATE;
        pthread_t           t1;

        g_rasp_pi_uart_dev.fd_uart = open(pCfg->pDev_name, O_RDWR | O_NOCTTY | O_NDELAY);
        if( g_rasp_pi_uart_dev.fd_uart < 0 )
            break;

        memset(&options, 0x0, sizeof(struct termios));

        g_rasp_pi_uart_dev.is_rx_running = false;
        g_rasp_pi_uart_dev.dev_type      = UART_DEV_TYPE_RASP_PI;
        g_rasp_pi_uart_dev.pRx_buf       = g_rx_buf;
        g_rasp_pi_uart_dev.rx_buf_len    = sizeof(g_rx_buf);
        pthread_create(&t1, 0, _rasp_pi_uart_data_listener, (void*)&g_rasp_pi_uart_dev);

        while( g_rasp_pi_uart_dev.is_rx_running == false )
            usleep(1000);

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

        msg(LOG_LEVEL_INFO, "baud rate: %d\n", pCfg->uart.baud_rate);

        cfsetispeed(&options, baudrate);
        cfsetospeed(&options, baudrate);
        tcflush(g_rasp_pi_uart_dev.fd_uart, TCIFLUSH);
        tcsetattr(g_rasp_pi_uart_dev.fd_uart, TCSANOW, &options);

        pHDev = &g_rasp_pi_uart_dev;
    } while(0);

    return (uart_handle_t)pHDev;
}

static int
_rasp_pi_uart_deinit(uart_handle_t pHandle)
{
    int     rval = 0;
    do {
        pi_uart_dev_t     *pHDev = (pi_uart_dev_t*)pHandle;

        if( !pHDev )   break;

        pHDev->is_rx_running = false;

        usleep(10000);

        close(pHDev->fd_uart);
        memset(&g_rasp_pi_uart_dev, 0x0, sizeof(g_rasp_pi_uart_dev));
    } while(0);

    return rval;
}

static int
_rasp_pi_uart_send_bytes(
    uart_handle_t   pHandle,
    uint8_t         *pData,
    int             data_len)
{
    int     rval = 0;
    do {
        pi_uart_dev_t       *pHDev = (pi_uart_dev_t*)pHandle;
        int                 retry = 2;
        int                 count = 0;

        if( !pHDev )   break;

        do {
            count = write(pHDev->fd_uart, pData, data_len);
        } while( count < 0 && --retry );

        fprintf(stderr, "[%s:%d] %c, cnt= %d, %d\n", __func__, __LINE__, *((char*)pData), count, data_len);

        if( count < 0 || (count != data_len) )
            rval = -1;
    } while(0);
    return rval;
}

static int
_rasp_pi_uart_recv_bytes(
    uart_handle_t   pHandle,
    uint8_t         *pData,
    int             *pData_len)
{
    int     rval = -1;
    do {
        pi_uart_dev_t     *pHDev = (pi_uart_dev_t*)pHandle;
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

static int
_rasp_pi_uart_get_state(
    uart_handle_t   pHandle,
    uart_state_t    state)
{
    int             rval = 0;
    do {
        pi_uart_dev_t     *pHDev = (pi_uart_dev_t*)pHandle;
        uint32_t        rd_idx = pHDev->rd_idx;
        uint32_t        wr_idx = pHDev->wr_idx;

        if( !pHDev )   break;

        if( state == UART_STATE_GET_RX_EVENT )
        {
            // if empty, no rx_event
            rval = !(rd_idx == wr_idx);
        }
    } while(0);
    return rval;
}

static int
_rasp_pi_uart_reset_buf(
    uart_handle_t   pHandle,
    uint8_t         *pBuf,
    uint32_t        buf_len)
{
    int             rval = 0;
    do {
        pi_uart_dev_t     *pHDev = (pi_uart_dev_t*)pHandle;

        if( !pHDev )   break;

        pHDev->rd_idx = 0;
        pHDev->wr_idx = 0;

    } while(0);
    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
uart_ops_t      g_rasp_pi_uart_pi_ops =
{
    .dev_type      = UART_DEV_TYPE_RASP_PI,
    .init          = _rasp_pi_uart_init,
    .deinit        = _rasp_pi_uart_deinit,
    .send_bytes    = _rasp_pi_uart_send_bytes,
    .recv_bytes    = _rasp_pi_uart_recv_bytes,
    .get_state     = _rasp_pi_uart_get_state,
    .reset_buf     = _rasp_pi_uart_reset_buf,

};
#else
uart_ops_t      g_rasp_pi_uart_pi_ops = {0};
#endif
