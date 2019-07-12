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


#include <string.h>
#include "uip.h"
#include "tftp.h"
#include "net.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define TFTP_IP1        127
#define TFTP_IP2        0
#define TFTP_IP3        0
#define TFTP_IP4        1

#define STATE_CLOSED        0
#define STATE_CONNECTING    1
#define STATE_SENDING       2
#define STATE_DATA          3
#define STATE_ACKED         4
#define STATE_ERROR         5

#define OP_RRQ      1
#define OP_WRQ      2
#define OP_DATA     3
#define OP_ACK      4
#define OP_ERROR    5


#define RW_MODE     "octet"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define log(str, ...)       printf("[%s:%d] " str, __func__, __LINE__, ##__VA_ARGS__)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct _tftp_msg
{
    uint16_t   opcode;
    union {
        struct _req {
            char    file_mode[256];
        } request;

        struct _data {
            uint16_t   blocknum;
            uint8_t    data[512];
        } data;

        struct _ack {
            uint16_t   blocknum;
        } ack;

        struct _error {
            uint16_t   error_code;
            char    error_msg[256];
        } error;

    } packet;
} tftp_msg_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================
static tftpc_t      g_tftpc;

static uint8_t      g_busy_flag;
static uint8_t      g_result_code;

#define BUF_SIZE (512)
static uint8_t     block_buf[BUF_SIZE] __attribute__ ((aligned (4)));

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
write_block()
{
    uint8_t     *data_ptr = 0;
    tftp_msg_t  *msg  = (tftp_msg_t *)uip_appdata;

    data_ptr = msg->packet.data.data;

    printf("\n\n========= wr %d bytes\n", g_tftpc.datalen);
    for(int i = 0; i < g_tftpc.datalen; i++)
    {
        if( i && !(i & 0xF) )
            printf("\n");

        printf("%02X ", data_ptr[i]);
    }
    printf("\n\n");

#if 0
    for(i = 0; i < g_tftpc.datalen - 4; i++) {
        block_buf[i] = data_ptr[i];
    }
    if(flash_write_block((uint32_t *)block_buf) != 0) {
        g_tftpc.flash_error ++;
    }
#endif
}

static void
send_ack(void)
{
    tftp_msg_t  *msg = (tftp_msg_t*)uip_appdata;

    msg->opcode              = UIP_HTONS(OP_ACK);
    msg->packet.ack.blocknum = UIP_HTONS(g_tftpc.blocknum);
    uip_send(uip_appdata, 4);
    return;
}

static void
send_request(void)
{
    tftp_msg_t  *msg = (tftp_msg_t *)uip_appdata;
    char        *pdata;
    uint16_t    datalen = 0;
    static char mode[] = RW_MODE;

    pdata = (char *)msg->packet.request.file_mode;

    msg->opcode = UIP_HTONS(OP_RRQ);
    datalen = 2;
    for(int i = 0; i < strlen(g_tftpc.fname); i++)
    {
        *pdata++ = g_tftpc.fname[i];
        datalen++;
    }

    *pdata++ = 0;
    datalen++;

    for(int i = 0; i < strlen(mode); i++)
    {
        *pdata++ = mode[i];
        datalen++;
    }
    *pdata++ = 0;
    datalen++;

    uip_send(uip_appdata, datalen);
    return;
}


static void
parse_msg(void)
{
    tftp_msg_t *m = (tftp_msg_t *)uip_appdata;
    uint16_t blocknum;

    switch(UIP_HTONS(m->opcode))
    {
        case OP_DATA:
            blocknum = UIP_HTONS(m->packet.data.blocknum);
            g_tftpc.duplicate = (g_tftpc.blocknum == blocknum ? 1 : 0);
            g_tftpc.blocknum = blocknum;
            g_tftpc.datalen = uip_datalen();
            g_tftpc.state = STATE_DATA;
            log("\n-----> op data\n");
            break;

        case OP_ACK:
            g_tftpc.blocknum = UIP_HTONS(m->packet.ack.blocknum);
            g_tftpc.state    = STATE_ACKED;
            log("\n-----> op ack\n");
            break;

        case OP_ERROR:
            g_tftpc.tftp_error = UIP_HTONS(m->packet.error.error_code);
            g_tftpc.state      = STATE_ERROR;
            log("\n-----> op err\n");
            break;
        default:
            log("\n-----> op %d\n", m->opcode);
            break;
    }
    return;
}


static
PT_THREAD(handle_tftp(void))
{
    PT_BEGIN(&g_tftpc.pt);
    g_tftpc.state = STATE_SENDING;
    g_tftpc.ticks = CLOCK_SECOND * 3;
    g_tftpc.retries = 3;

    do {
        log("send req\n");
        send_request();
        timer_set(&g_tftpc.timer, g_tftpc.ticks);
        PT_WAIT_UNTIL(&g_tftpc.pt, uip_newdata() || timer_expired(&g_tftpc.timer));
        if(uip_newdata())
        {
            log("... income data\n");
            parse_msg();
            break;
        }

    } while (--g_tftpc.retries > 0);

    if(g_tftpc.state == STATE_DATA)
    {
        // do something with the first block of data
        write_block();
        while(g_tftpc.datalen == 516)
        {
            g_tftpc.state = STATE_SENDING;
            g_tftpc.retries = 3;
            do {
                send_ack();
                timer_set(&g_tftpc.timer, g_tftpc.ticks);
                PT_YIELD_UNTIL(&g_tftpc.pt, uip_newdata() || timer_expired(&g_tftpc.timer));
                if(uip_newdata())
                {
                    parse_msg();
                    break;
                }
            } while(--g_tftpc.retries > 0);

            if(g_tftpc.state == STATE_DATA)
            {
                if(!g_tftpc.duplicate)
                {
                    // do something with next block of data
                    write_block();
                }
            }
            else
            {
                // error or timeout
                uip_close();
                uip_udp_remove(g_tftpc.conn);

                g_tftpc.state = STATE_CLOSED;
                g_busy_flag   = 0;
                g_result_code = TFTPC_ERROR;

                PT_RESTART(&g_tftpc.pt);
            }

        }
        // send final ack
        send_ack();
        timer_set(&g_tftpc.timer, g_tftpc.ticks);
        PT_WAIT_UNTIL(&g_tftpc.pt, timer_expired(&g_tftpc.timer));
        uip_close();
        uip_udp_remove(g_tftpc.conn);
        g_tftpc.state = STATE_CLOSED;

        g_result_code = (g_tftpc.flash_error > 0) ? TFTPC_FLASH_ERROR : TFTPC_SUCCESS;

        g_busy_flag = 0;

    }
    else if(g_tftpc.state == STATE_ACKED)
    {
        // only for write operations
    }
    else
    {
        // error, or server down
        uip_abort();
        uip_udp_remove(g_tftpc.conn);
        g_tftpc.state = STATE_CLOSED;

        g_result_code = (g_tftpc.tftp_error == 1) ? TFTPC_FILE_NOT_FOUND : TFTPC_SERVER_DOWN;

        g_busy_flag = 0;
    }

    PT_END(&g_tftpc.pt);
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
uint8_t tftpc_busy()
{
    return g_busy_flag;
}

uint8_t tftpc_result()
{
    return g_result_code;
}

void tftpc_init(void)
{
    memset(&g_tftpc, 0x0, sizeof(g_tftpc));
    g_tftpc.state = STATE_CLOSED;

    PT_INIT(&g_tftpc.pt);
    return;
}

uint8_t tftpc_get(char *fname)
{
    uint8_t    rval = 0;
    do {
        uip_ipaddr_t addr;

        if(g_tftpc.state != STATE_CLOSED)
        {
            log("Client is busy\n");
            rval = -1;  // client is busy
            break;
        }

        log("request '%s'\n", fname);

        strncpy(g_tftpc.fname, fname, MAX_FNAME_LEN - 1);
        uip_ipaddr(&addr, TFTP_IP1, TFTP_IP2, TFTP_IP3, TFTP_IP4);
        g_tftpc.conn = uip_udp_new(&addr, UIP_HTONS(TFTP_PORT));
        if( g_tftpc.conn == NULL )
        {
            log("No available connections");
            rval = -2;  // no available connections
            break;
        }

//        uip_udp_bind(g_tftpc.conn, UIP_HTONS(TFTP_PORT));

        g_tftpc.state = STATE_CONNECTING;
        g_busy_flag = 1;

    } while(0);
    return rval;
}


void tftpc_appcall(void)
{
    handle_tftp();
    return;
}


