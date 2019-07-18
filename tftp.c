/*
 * tftp.c
 *
 *  Created on: Feb 2, 2011
 *      Author: James Harwood
 *
 * This module is free software and there is NO WARRANTY.
 * No restrictions on use. You can use, modify and redistribute it for
 * personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
 */

#include <stdio.h>
#include <string.h>
#include "uip.h"
#include "tftp.h"
#include "net.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ENABLE_TFTP_RX_DUMP


#define TFTP_RETRY_CNT      5

#define STATE_CLOSED        0
#define STATE_CONNECTING    1
#define STATE_SENDING       2
#define STATE_DATA          3
#define STATE_ACKED         4
#define STATE_ERROR         5
#define STATE_OACKED        6


#define OP_RRQ          1 /**< Read request opcode */
#define OP_WRQ          2 /**< Write request opcode */
#define OP_DATA         3 /**< Data block opcode */
#define OP_ACK          4 /**< Data block acknowledgement opcode */
#define OP_ERROR        5 /**< Error opcode */
#define OP_OACK         6 /**< Options acknowledgement opcode */

#define TFTP_RW_MODE        "octet"

/*
 * options
 */
#define TFTP_OPT_TSIZE      "tsize"
#define TFTP_OPT_TIMEOUT    "timeout"
#define TFTP_OPT_BLKSIZE    "blksize"
#define TFTP_OPT_MCAST      "multicast"
#define TFTP_OPT_PORT       "udpport"

#define TFTP_SEGSIZE        512     /* data segment size */

//=============================================================================
//                  Macro Definition
//=============================================================================
#define stringize(s)        #s
#define _toStr(a)           stringize(a)

/**
 *  @brief get_srcport_from_pkg
 *      get source port number of received package
 *  @param [in] new_port
 */
#define get_srcport_from_pkg()      ((struct uip_udpip_hdr*)&uip_buf[UIP_LLH_LEN])->srcport

#define log(str, ...)               printf("[%s:%d] " str, __func__, __LINE__, ##__VA_ARGS__)


#ifndef vt_memcpy
#warning "TODO: include util.h for vt_memcpy !!!!"
#define vt_memcpy   memcpy
#define vt_strlen   strlen
#define vt_strncmp  strncmp
#endif // vt_memcpy
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct tftpc
{
    struct uip_udp_conn *conn[2];
    struct pt       pt;
    struct timer    timer;
    uint16_t        ticks;
    uint16_t        retries;
    uint16_t        blocknum;
    uint16_t        tftp_error;
    uint16_t        flash_error;
    uint16_t        datalen;
    uint16_t        local_port;
    uint8_t         duplicate;
    uint8_t         state;
    char            fname[TFTP_MAX_FNAME_LEN];
} tftpc_t;

#pragma pack(1)
typedef struct _tftp_msg
{
    uint16_t   opcode;
    union {
        struct _req {
            char        file_mode[256];
        } request;

        struct _data {
            uint16_t    blocknum;
            uint8_t     data[512];
        } data;

        struct _ack {
            uint16_t    blocknum;
        } ack;

        struct _error {
            uint16_t    error_code;
            char        error_msg[256];
        } error;

        struct _oack {
            uint8_t     data[0];
        } oack;

    } packet;
} tftp_msg_t;
#pragma pack()

//typedef struct oack_opt
//{
//    char    *pOpt;
//    int     opt_strlen;
//} oack_opt_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static tftpc_t          g_tftpc;

static uint32_t         g_tfpt_is_busy = 0;
static tftpc_err_t      g_result_code = TFTPC_ERR_OK;

//static uint8_t          block_buf[TFTP_SEGSIZE] __attribute__ ((aligned (4)));

//static oack_opt_t       g_opt_table[] =
//{
//    { .pOpt = TFTP_OPT_TSIZE,   .opt_strlen = 6, },
//    { .pOpt = TFTP_OPT_BLKSIZE, .opt_strlen = 8, },
//    { .pOpt = TFTP_OPT_PORT,    .opt_strlen = 8, },
//    { .pOpt = TFTP_OPT_TIMEOUT, .opt_strlen = 8, },
//    { .pOpt = TFTP_OPT_MCAST,   .opt_strlen = 10, },
//    { .pOpt = 0,                .opt_strlen = 0 }
//};

#if defined(CONFIG_ENABLE_TFTP_RX_DUMP)
static FILE     *fout = 0;
#endif
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_tftp_write_data_block(void)
{
    uint8_t     *pData = 0;
    tftp_msg_t  *msg = (tftp_msg_t*)uip_appdata;

    pData = msg->packet.data.data;


#if defined(CONFIG_ENABLE_TFTP_RX_DUMP)
    if( fout )  fwrite(pData, 1, g_tftpc.datalen - 4, fout);
#endif
    return;
}

static void
_tftp_send_ack(void)
{
    tftp_msg_t  *msg = (tftp_msg_t*)uip_appdata;

    msg->opcode              = UIP_HTONS(OP_ACK);
    msg->packet.ack.blocknum = UIP_HTONS(g_tftpc.blocknum);

    uip_udp_send(4);
    return;
}

static void
_tftp_send_rrq(void)
{
    tftp_msg_t  *msg = (tftp_msg_t *)uip_appdata;
    char        *pData;
    int         len = 0;
    uint16_t    datalen = 0;
    static char mode[] = TFTP_RW_MODE;

    log("send RRQ\n");

    pData = (char *)msg->packet.request.file_mode;

    // op-code
    msg->opcode = UIP_HTONS(OP_RRQ);
    datalen = 2;

    // filename
    len = vt_strlen(g_tftpc.fname) + 1;
    vt_memcpy(pData, g_tftpc.fname, len);
    pData   += len;
    datalen += len;

    // mode
    len = vt_strlen(mode) + 1;
    vt_memcpy(pData, mode, len);
    pData   += len;
    datalen += len;

    // tsize
    *pData++ = 't';
    *pData++ = 's';
    *pData++ = 'i';
    *pData++ = 'z';
    *pData++ = 'e';
    *pData++ = '\0';
    *pData++ = '0';
    *pData++ = '\0';
    datalen += 8;

    log("push %d bytes for send \n", datalen);

    // TODO: multi-cast
    uip_udp_send(datalen);
    return;
}


static tftpc_err_t
_tftp_parse_msg(void)
{
    tftpc_err_t     rval = TFTPC_ERR_OK;
    tftp_msg_t      *m = (tftp_msg_t *)uip_appdata;
    uint16_t        blocknum;

    if( !uip_newdata() )
        return TFTPC_ERR_NO_INCOME;

    switch( UIP_HTONS(m->opcode) )
    {
        case OP_DATA:
            blocknum = UIP_HTONS(m->packet.data.blocknum);

            log("Recv 'DATA', blknum #%02d\n", blocknum);

            g_tftpc.duplicate = (g_tftpc.blocknum == blocknum ? 1 : 0);
            g_tftpc.blocknum  = blocknum;
            g_tftpc.datalen   = uip_datalen();
            g_tftpc.state     = STATE_DATA;
            break;

        case OP_ACK:
            log("Recv 'ACK'\n");
            g_tftpc.blocknum = UIP_HTONS(m->packet.ack.blocknum);
            g_tftpc.state    = STATE_ACKED;
            break;

        case OP_ERROR:
            log("Recv 'ERROR'\n");
            g_tftpc.tftp_error = UIP_HTONS(m->packet.error.error_code);
            g_tftpc.state      = STATE_ERROR;
            break;

        case OP_OACK:
            log("Recv 'OARK'\n");
            g_tftpc.state = STATE_OACKED;
            break;

        default:
            log("-----> op %d\n", UIP_HTONS(m->opcode));
            rval = TFTPC_ERR_UNKNOWN_OPCODE;
            break;
    }
    return rval;
}


static
PT_THREAD(handle_tftp(void))
{
    tftpc_err_t     rval = TFTPC_ERR_OK;

    PT_BEGIN(&g_tftpc.pt);
    g_tftpc.state   = STATE_SENDING;
    g_tftpc.ticks   = CLOCK_SECOND >> 8;// CLOCK_SECOND * 3;
    g_tftpc.retries = TFTP_RETRY_CNT;

    do {
        _tftp_send_rrq();
        timer_set(&g_tftpc.timer, g_tftpc.ticks);
        PT_WAIT_UNTIL(&g_tftpc.pt, uip_newdata() || timer_expired(&g_tftpc.timer));

        rval = _tftp_parse_msg();
        if( rval == TFTPC_ERR_OK )
            break;
    } while( --g_tftpc.retries );


    if( g_tftpc.state == STATE_OACKED )
    {
        g_tftpc.retries = TFTP_RETRY_CNT;
        do {
            _tftp_send_ack();
            timer_set(&g_tftpc.timer, g_tftpc.ticks);
            PT_YIELD_UNTIL(&g_tftpc.pt, uip_newdata() || timer_expired(&g_tftpc.timer));

            rval = _tftp_parse_msg();
            if( g_tftpc.state == STATE_DATA )
                break;

        } while(--g_tftpc.retries > 0);
    }

    if( g_tftpc.state == STATE_DATA )
    {
        _tftp_write_data_block();

        while( g_tftpc.datalen == 516 )
        {
            g_tftpc.state   = STATE_SENDING;
            g_tftpc.retries = TFTP_RETRY_CNT;
            do {
                _tftp_send_ack();
                timer_set(&g_tftpc.timer, g_tftpc.ticks);
                PT_YIELD_UNTIL(&g_tftpc.pt, uip_newdata() || timer_expired(&g_tftpc.timer));

                rval = _tftp_parse_msg();
                if( rval == TFTPC_ERR_OK )
                    break;

            } while( --g_tftpc.retries );

            if( g_tftpc.state == STATE_DATA )
            {
                if( !g_tftpc.duplicate )
                {
                    _tftp_write_data_block();
                }
            }
            else
            {
                // error or timeout
                uip_close();

                uip_udp_remove(g_tftpc.conn[0]);
                uip_udp_remove(g_tftpc.conn[1]);

                g_tftpc.state  = STATE_CLOSED;
                g_tfpt_is_busy = 0;
                g_result_code  = TFTPC_ERR_FAIL;

                PT_RESTART(&g_tftpc.pt);
            }
        }

        // send final ack
        _tftp_send_ack();
        timer_set(&g_tftpc.timer, g_tftpc.ticks);
        PT_WAIT_UNTIL(&g_tftpc.pt, timer_expired(&g_tftpc.timer));

        uip_close();

        uip_udp_remove(g_tftpc.conn[0]);
        uip_udp_remove(g_tftpc.conn[1]);
        g_tftpc.state = STATE_CLOSED;

        #if defined(CONFIG_ENABLE_TFTP_RX_DUMP)
        if( fout )
        {
            fclose(fout);
            fout = 0;
        }
        #endif

        g_result_code = (g_tftpc.flash_error)
                      ? TFTPC_ERR_FLASH_FAIL : TFTPC_ERR_OK;

        g_tfpt_is_busy = 0;
    }
    else if(g_tftpc.state == STATE_ACKED)
    {
        // only for write operations
    }
    else
    {
        // error, or server down
        uip_abort();
        uip_udp_remove(g_tftpc.conn[0]);
        uip_udp_remove(g_tftpc.conn[1]);
        g_tftpc.state = STATE_CLOSED;

        #if 1
        switch( g_tftpc.tftp_error )
        {
            case TFTP_ERRCODE_FILE_NOT_FOUND:    printf("error! %s\n", _toStr(TFTP_ERRCODE_FILE_NOT_FOUND));    break;
            case TFTP_ERRCODE_ACCESS_DENIED:     printf("error! %s\n", _toStr(TFTP_ERRCODE_ACCESS_DENIED));     break;
            case TFTP_ERRCODE_DISK_FULL:         printf("error! %s\n", _toStr(TFTP_ERRCODE_DISK_FULL));         break;
            case TFTP_ERRCODE_ILLEGAL_OP:        printf("error! %s\n", _toStr(TFTP_ERRCODE_ILLEGAL_OP));        break;
            case TFTP_ERRCODE_UNKNOWN_TID:       printf("error! %s\n", _toStr(TFTP_ERRCODE_UNKNOWN_TID));       break;
            case TFTP_ERRCODE_FILE_EXISTS:       printf("error! %s\n", _toStr(TFTP_ERRCODE_FILE_EXISTS));       break;
            case TFTP_ERRCODE_UNKNOWN_USER:      printf("error! %s\n", _toStr(TFTP_ERRCODE_UNKNOWN_USER));      break;
            default:    break;
        }
        #endif

        g_result_code = g_tftpc.tftp_error;

        g_tfpt_is_busy = 0;
    }

    PT_END(&g_tftpc.pt);
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
uint32_t
tftpc_is_busy(void)
{
    return g_tfpt_is_busy;
}

tftpc_err_t
tftpc_get_result(void)
{
    return g_result_code;
}

void tftpc_init(void)
{
    memset(&g_tftpc, 0x0, sizeof(g_tftpc));
    g_tftpc.state = STATE_CLOSED;

    PT_INIT(&g_tftpc.pt);

    #if defined(CONFIG_ENABLE_TFTP_RX_DUMP)
    if( !fout )     fout = fopen("dump.bin", "wb");
    #endif
    return;
}

tftpc_err_t
tftpc_get(
    uip_ipaddr_t    *pSvr_ipaddr,
    char            *fname)
{
    tftpc_err_t     rval = TFTPC_ERR_OK;
    do {
        if( g_tftpc.state != STATE_CLOSED )
        {
            log("Client is busy\n");
            rval = TFTPC_ERR_SYS_BUSY;  // client is busy
            break;
        }

        log("request '%s'\n", fname);

        strncpy(g_tftpc.fname, fname, vt_strlen(fname));

        g_tftpc.conn[0] = uip_udp_new(pSvr_ipaddr, UIP_HTONS(TFTP_PORT));
        g_tftpc.conn[1] = uip_udp_new(pSvr_ipaddr, UIP_HTONS(0));
        if( !g_tftpc.conn[0] || !g_tftpc.conn[1] )
        {
            log("No available udp connections\n");
            rval = TFTPC_ERR_CONNECTION_FULL;  // no available connections
            break;
        }

        g_tftpc.local_port = uip_ntohs(g_tftpc.conn[0]->lport);
        uip_udp_bind(g_tftpc.conn[1], UIP_HTONS(g_tftpc.local_port));

        g_tftpc.state = STATE_CONNECTING;
        g_tfpt_is_busy = 1;

    } while(0);
    return rval;
}


void tftpc_appcall(void)
{
    if( uip_udp_conn->lport == UIP_HTONS(g_tftpc.local_port) )
    {
        handle_tftp();
    }
    return;
}


