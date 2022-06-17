
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include "ymodem.h"
#include "rbuf.h"
#include "my_timer.h"


#define err(str, ...)       do{ printf("[%s:%d]" str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)

static rbi_t                g_rbi_tx = {0};
static rbi_t                g_rbi_rx = {0};
static pthread_cond_t       g_cond;
static pthread_mutex_t      g_mtx;

static uint8_t              *g_pBuf_tx = 0;
static uint8_t              g_buf_rx[1 << 20] = {0};
static uint32_t             g_tick_ms = 0;


int tx_inbyte(unsigned short timeout) // msec timeout
{
    int         rval = -1;
    uint8_t     value = 0;
    int         len = 1;

    uint32_t    start_ms = g_tick_ms;

    while( (g_tick_ms - start_ms) < timeout )
    {
        rval = rbi_pop(&g_rbi_rx, &value, &len);
        if( rval == 0 )
            break;
    }

    return (rval < 0) ? -1 : (int)value;
}

void tx_outbyte(int c)
{
    uint8_t     ch = c & 0xFF;
    rbi_push(&g_rbi_tx, &ch, 1);
    return;
}

int rx_inbyte(unsigned short timeout) // msec timeout
{
    int         rval = -1;
    uint8_t     value = 0;
    int         len = 1;

    uint32_t    start_ms = g_tick_ms;

    while( (g_tick_ms - start_ms) < timeout )
    {
        rval = rbi_pop(&g_rbi_tx, &value, &len);
        if( rval == 0 )
            break;
    }

    return (rval < 0) ? -1 : (int)value;
}

void rx_outbyte(int c)
{
    uint8_t     ch = c & 0xFF;
    rbi_push(&g_rbi_rx, &ch, 1);
    return;
}

static void
_usage(char **argv)
{
    printf("usage: %s [xfer data path]\n", argv[0]);
    system("pause");
}

static void
_timer_handler(void)
{
    g_tick_ms++;
    return;
}

static void*
_thread_ymodem_receive(void* data)
{
    int     *pHas_exit = (int*)data;
    int     nbytes = 0;
    int     is_done = false;

    printf ("Send data using the Ymodem protocol from your terminal emulator now...\n");

    pthread_mutex_lock(&g_mtx);
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mtx);

    while( *pHas_exit == false )
    {
        if( is_done == true )
            continue;

        /* the following should be changed for your environment:
         *  1. the download address,
         *  2. the maximum size to be written at this address
         */
        nbytes = YmodemReceive((uint8_t*)g_buf_rx, sizeof(g_buf_rx));
        if (nbytes < 0)
            printf ("Ymodem receive error: status: %d\n", nbytes);
        else
            printf ("Ymodem successfully received %d bytes\n", nbytes);

        {
            FILE  *fout = 0;
            if( (fout = fopen("rx_data.bin", "wb")) )
            {
                fwrite(g_buf_rx, 1, nbytes, fout);
                fclose(fout);
                fout = 0;
            }
        }

        is_done = true;
    }

    pthread_exit(0);
    return 0;
}

int main(int argc, char **argv)
{
    int         rval = 0;
    int         has_exit = false;
    int         nbytes = 0;
    FILE        *fin = 0;
    uint32_t    filesize = 0;
    pthread_t   t1;

    g_tick_ms = 0;

    if( argc < 2 )
    {
        _usage(argv);
        return -1;
    }

    if( !(fin = fopen(argv[1], "rb")) )
    {
        err("open %s fail \n", argv[1]);
    }

    fseek(fin, 0, SEEK_END);
    filesize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    if( !(g_pBuf_tx = malloc(filesize)) )
    {
        err("allocate %d bytes fail \n", filesize);
    }
    memset(g_pBuf_tx, 0x0, filesize);
    fread(g_pBuf_tx, 1, filesize, fin);
    fclose(fin);
    fin = 0;

    rval = pthread_mutex_init(&g_mtx, 0);
    if( rval )
    {
        err("mutex init fail\n");
    }

    rval = pthread_cond_init(&g_cond, 0);
    if( rval )
    {
        err("cond init fail\n");
    }

    rbi_init(&g_rbi_tx);
    rbi_init(&g_rbi_rx);

    if( timer_start(1, &_timer_handler) )
    {
        err("create timer fail\n");
    }

    pthread_create(&t1, NULL, _thread_ymodem_receive, &has_exit);
    printf("create rx thread...\n");

    pthread_mutex_lock(&g_mtx);
    pthread_cond_wait(&g_cond, &g_mtx);
    pthread_mutex_unlock(&g_mtx);

    printf ("Prepare your terminal emulator to receive data now...\n");
    /* the following should be changed for your environment:
       0x30000 is the download address,
       12000 is the maximum size to be send from this address
     */
    nbytes = YmodemTransmit((uint8_t*)g_pBuf_tx, filesize, "test.raw");
    if( nbytes < 0 )
    {
        printf ("Ymodem transmit error: status: %d\n", nbytes);
    }
    else
    {
        printf ("Ymodem successfully transmitted %d bytes\n", nbytes);
    }

    if( g_pBuf_tx ) free(g_pBuf_tx);
    if( fin )       fclose(fin);

    has_exit = true;
    pthread_join(t1, 0);

    pthread_mutex_destroy(&g_mtx);
    pthread_cond_destroy(&g_cond);

    system("pause");
    return rval;
}
