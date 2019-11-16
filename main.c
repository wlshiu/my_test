#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "uart_dev.h"
#include "log.h"

#include "pthread.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_IN_BUF_LEN       4096
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct reader
{
    union {
        int             fd;
        uart_handle_t   hUart;
    };

    uint8_t         *pBuf;
    long            buf_size;
    long            buf_remain_data;

    uint8_t         *pCur;
    uint8_t         *pEnd;

    uint32_t        is_restart;

//    int             alignment;
//    unsigned long   is_big_endian;

} reader_t;

typedef struct reader_cfg
{
    char    *pName;

    union {
        struct {
            int     port;
            int     baudrate;
        } uart;

        struct {
            int     reserved;
        } def;
    };
} reader_cfg_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
unsigned long       gLog_flags = 0x1;

pthread_mutex_t     g_log_mtx;
//static uint32_t     g_in_buf[(CONFIG_IN_BUF_LEN + 20) >> 2] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void _usage(char *pProg)
{
    printf("usage: %s [dev name] [baud rate]",
           pProg);
    exit(1);
    return;
}


static reader_t*
_reader_create(reader_cfg_t *pCfg)
{
    int         rval = -1;
    reader_t    *pReader = 0;
    do {
        uart_handle_t   hUart = 0;
        uart_cfg_t      uart_cfg = {0};

        if( !pCfg )  break;

        uart_cfg.pDev_name       = pCfg->pName;
        /* uart_cfg.act_dev_type    = UART_DEV_TYPE_SIM; */
        uart_cfg.act_dev_type    = UART_DEV_TYPE_RASP_PI;
        uart_cfg.uart.baud_rate  = pCfg->uart.baudrate;
        uart_cfg.uart.port       = pCfg->uart.port;
        hUart = uart_dev_init(&uart_cfg, 0);
        if( !hUart )
        {
            err("communication device initial fail \n");
            break;
        }

        if( !(pReader = malloc(sizeof(reader_t))) )
        {
            err("malloc reader fail \n");
            break;
        }

        if( !(pReader->pBuf = malloc(CONFIG_IN_BUF_LEN)) )
        {
            err("malloc '%d' fail\n", CONFIG_IN_BUF_LEN);
            break;
        }
        memset(pReader->pBuf, 0x0, CONFIG_IN_BUF_LEN);

        pReader->buf_size   = CONFIG_IN_BUF_LEN;
        pReader->pCur       = pReader->pBuf;
        pReader->pEnd       = pReader->pBuf;
        pReader->hUart      = hUart;

        rval = 0;
    } while(0);

    return pReader;
}

static int
_reader_destory(reader_t *pReader)
{
    int     rval = -1;
    do {
        if( !pReader )  break;

        uart_dev_deinit(pReader->hUart);

        if( pReader->pBuf )
            free(pReader->pBuf);

        free(pReader);
        rval = 0;
    } while(0);

    return rval;
}

static int
_reader_full_buf(
    reader_t    *pReader,
    int (*cb_post_read)(unsigned char *pBuf, int buf_size))
{
    int         rval = 0;
    do {
        long        remain_data = 0;

        if( !pReader )      break;

        if( pReader->is_restart )
        {
            pReader->pCur = pReader->pBuf;
            pReader->pEnd = pReader->pBuf;
            pReader->is_restart = 0;
        }

        remain_data = pReader->pEnd - pReader->pCur;
        remain_data = (remain_data < 0) ? 0 : remain_data;
        if( remain_data < (pReader->buf_size >> 2) )
        {
            int     nbytes = 0;

            if( remain_data > 0 && pReader->pBuf != pReader->pCur )
                memmove(pReader->pBuf, pReader->pCur, remain_data);

            #if 0
            nbytes = pReader->buf_size - remain_data;
            #else
            nbytes = pReader->buf_size - remain_data - 2;
            #endif
            uart_dev_recv_bytes(pReader->hUart, pReader->pBuf + remain_data, &nbytes);

            pReader->pCur = pReader->pBuf;
            pReader->pEnd = pReader->pBuf + remain_data + nbytes;

            if( nbytes )
            {
                *pReader->pEnd       = '\\';
                *(pReader->pEnd + 1) = '\0';

                if( cb_post_read &&
                    (rval = cb_post_read(pReader->pBuf + remain_data, nbytes)) )
                    break;
            }
        }

        pReader->buf_remain_data = pReader->pEnd - pReader->pCur;

    } while(0);
    return rval;
}

static int
_post_read(unsigned char *pBuf, int buf_size)
{
    int     rval = 0;
    do {
        if( !buf_size )     break;

        for(int i = 0; i < buf_size; ++i)
        {
            putchar((char)pBuf[i]);
            pBuf[i] = (pBuf[i] == '\n' || pBuf[i] == '\r')
                    ? '\0' : pBuf[i];
        }
    } while(0);

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    reader_t        *pReader = 0;

    do {
        reader_cfg_t    cfg = {0};
        int             item_num = -1;
        int             is_sent = 0;

        if( argc < 2 )
        {
            _usage(argv[0]);
            break;
        }

        cfg.pName         = argv[1];
        cfg.uart.baudrate = atol(argv[2]);
        pReader = _reader_create(&cfg);
        if( !pReader )      break;

        pthread_mutex_init(&g_log_mtx, 0);

        while(1)
        {
            int     rval = 0;

        #if 0
            if( item_num == -1 )
            {
                char    ch = 'l';
                uart_dev_send_bytes(pReader->hUart, (uint8_t*)&ch, 1);
            }
        #endif

            _reader_full_buf(pReader, _post_read);
            while( pReader->pCur < pReader->pEnd )
            {
                if( _reader_full_buf(pReader, _post_read) )
                    break;

                {   // start parsing a line
                    char    *pAct_str = 0;
                    size_t  offset = 0;

                    pAct_str = (char*)pReader->pCur;
                    offset  = (strlen(pAct_str) + 1);
                    if( pAct_str[offset - 2] != '\\' )
                    {
                        pReader->pCur += offset;
                        rval = sscanf(pAct_str, "--- total %d items", &item_num);
                    }
                    else        rval = 0;

                    if( rval != 1 )
                        continue;

                    printf("\n==== %d\n", item_num);

                    if( is_sent == 0 )
                    {
                        char    ch = '2';
                        rval = uart_dev_send_bytes(pReader->hUart, (uint8_t*)&ch, 1);
                        if( rval == 0 )
                            is_sent = 1;
                        else
                            fprintf(stdout, "send fail %d\n", rval);
                    }
                }
            }
        }

    } while(0);

    _reader_destory(pReader);

    return 0;
}
