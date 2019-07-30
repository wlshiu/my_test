/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * @(#)$Id: dhcpc.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "uip.h"
#include "dhcpc.h"
#include "uip_timer.h"
#include "pt.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_MEASURE_TIME


#define DHCP_MSG_FILENAME_SIZE  128

#define STATE_INITIAL           0
#define STATE_SENDING           1
#define STATE_OFFER_RECEIVED    2
#define STATE_CONFIG_RECEIVED   3

#define BOOTP_BROADCAST         0x8000

#define DHCP_OP_REQUEST         1
#define DHCP_OP_REPLY           2
#define DHCP_HTYPE_ETHERNET     1
#define DHCP_HLEN_ETHERNET      6
#define DHCP_MSG_LEN            236

#define DHCPC_SERVER_PORT       67
#define DHCPC_CLIENT_PORT       68

#define DHCP_MSG_DISCOVER       1
#define DHCP_MSG_OFFER          2
#define DHCP_MSG_REQUEST        3
#define DHCP_MSG_DECLINE        4
#define DHCP_MSG_ACK            5
#define DHCP_MSG_NAK            6
#define DHCP_MSG_RELEASE        7

#define DHCP_OPTION_SUBNET_MASK     1
#define DHCP_OPTION_ROUTER          3
#define DHCP_OPTION_DNS_SERVER      6
#define DHCP_OPTION_REQ_IPADDR      50
#define DHCP_OPTION_LEASE_TIME      51
#define DHCP_OPTION_MSG_TYPE        53
#define DHCP_OPTION_SERVER_ID       54
#define DHCP_OPTION_REQ_LIST        55
#define DHCP_OPTION_TFTPD_NAME      66
#define DHCP_OPTION_TFTPD_IP        150
#define DHCP_OPTION_END             255


#define DHCP_MAGIC_COOKIE           0x63538263u

typedef enum dhcpc_phase
{
    DHCPC_PHASE_IP_ALLOC,
    DHCPC_PHASE_IP_FREE,
    DHCPC_PHASE_REQ,

} dhcpc_phase_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct dhcpc
{
    struct pt               pt;
    struct uip_udp_conn     *conn;
    struct timer            timer;
    uint32_t                xid;

    struct uip_eth_addr     mac_addr;
    uint16_t                ticks;

    uint32_t                state;

    uint8_t                 serverid[4];

    uint16_t                lease_time[2];
    uip_ipaddr_t            ipaddr;
    uip_ipaddr_t            next_ipaddr;
    uip_ipaddr_t            tftp_ipaddr;
    uip_ipaddr_t            netmask;
    uip_ipaddr_t            dnsaddr;
    uip_ipaddr_t            default_router;

    char                    tftpd_name[64];
    char                    boot_filename[DHCP_MSG_FILENAME_SIZE];
} dhcpc_t;

#pragma pack(1)
typedef struct dhcp_msg
{
    uint8_t     op;             /* message type */
    uint8_t     htype;          /* hardware address type */
    uint8_t     hlen;           /* hardware address length */
    uint8_t     hops;           /* should be zero in client message */
    uint32_t    xid;            /* transaction id */
    uint16_t    secs;           /* elapsed time in sec. from boot */
    uint16_t    flags;
    uint32_t    ciaddr;         /* (previously allocated) client IP */
    uint32_t    yiaddr;         /* 'your' client IP address */
    uint32_t    siaddr;         /* should be zero in client's messages */
    uint32_t    giaddr;         /* should be zero in client's messages */
    uint8_t     chaddr[16];     /* client's hardware address */

    uint8_t     sname[64];      /* server host name */
    uint8_t     filename[DHCP_MSG_FILENAME_SIZE];  /* boot file name */

    uint32_t    cookie;
    uint8_t     options[308];   /* message options - cookie */
} dhcp_msg_t;
#pragma pack()

//=============================================================================
//                  Global Data Definition
//=============================================================================
static const uint8_t    magic_cookie[4] = {99, 130, 83, 99};

static dhcpc_t          g_dhcpc;
static uint32_t         g_dhcpc_is_busy = 0;
static dhcpc_phase_t    g_dhcpc_phase = DHCPC_PHASE_IP_ALLOC;

#if defined(CONFIG_MEASURE_TIME)
#include <stdio.h>
static clock_time_t     g_tm_start = 0;
#endif // defined
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t
_dhcpc_gen_xid(void)
{
    srand(time(0));
    return rand() & 0xFFFFFFFF;
}

static uint8_t*
_dhcpc_add_msg_type(uint8_t *optptr, uint8_t type)
{
    *optptr++ = DHCP_OPTION_MSG_TYPE;
    *optptr++ = 1;
    *optptr++ = type;
    return optptr;
}


static uint8_t*
_dhcpc_add_server_id(uint8_t *optptr)
{
    *optptr++ = DHCP_OPTION_SERVER_ID;
    *optptr++ = 4;
    memcpy(optptr, g_dhcpc.serverid, 4);
    return optptr + 4;
}


static uint8_t*
_dhcpc_add_req_ipaddr(uint8_t *optptr)
{
    *optptr++ = DHCP_OPTION_REQ_IPADDR;
    *optptr++ = 4;
    memcpy(optptr, g_dhcpc.ipaddr, sizeof(g_dhcpc.ipaddr));
    return optptr + 4;
}

#if 0
static uint8_t*
_dhcpc_add_req_options(uint8_t *optptr)
{
    *optptr++ = DHCP_OPTION_REQ_LIST;
    *optptr++ = 3;
    *optptr++ = DHCP_OPTION_SUBNET_MASK;
    *optptr++ = DHCP_OPTION_ROUTER;
    *optptr++ = DHCP_OPTION_DNS_SERVER;
    return optptr;
}
#endif

static uint8_t*
_dhcpc_add_end_tag(uint8_t *optptr)
{
    *optptr++ = DHCP_OPTION_END;
    return optptr;
}


static void
_dhcpc_create_msg(dhcp_msg_t *m)
{
    memset(m, 0x0, sizeof(dhcp_msg_t));

    m->op     = DHCP_OP_REQUEST;
    m->htype  = DHCP_HTYPE_ETHERNET;
    m->hlen   = sizeof(struct uip_eth_addr);
    m->xid    = g_dhcpc.xid;
    m->flags  = UIP_HTONS(BOOTP_BROADCAST); /*  Broadcast bit. */
    m->cookie = DHCP_MAGIC_COOKIE;

    memcpy(m->chaddr, &g_dhcpc.mac_addr, sizeof(g_dhcpc.mac_addr));
    return;
}


static void
_dhcpc_send_discover(void)
{
    uint8_t         *pCur = 0;
    dhcp_msg_t      *m = (dhcp_msg_t*)uip_appdata;

    _dhcpc_create_msg(m);

    pCur = _dhcpc_add_msg_type(m->options, DHCP_MSG_DISCOVER);
    pCur = _dhcpc_add_end_tag(pCur);

    uip_send(uip_appdata, pCur - (uint8_t *)uip_appdata);
    return;
}

static void
_dhcpc_send_release(void)
{
    uint8_t         *pCur = 0;
    dhcp_msg_t      *m = (dhcp_msg_t*)uip_appdata;

    _dhcpc_create_msg(m);

    pCur = _dhcpc_add_msg_type(m->options, DHCP_MSG_RELEASE);
    pCur = _dhcpc_add_end_tag(pCur);

    uip_send(uip_appdata, pCur - (uint8_t *)uip_appdata);
    return;
}

static void
_dhcpc_send_request(void)
{
    uint8_t         *pCur = 0;
    dhcp_msg_t      *m = (dhcp_msg_t*)uip_appdata;

    _dhcpc_create_msg(m);

    pCur = _dhcpc_add_msg_type(m->options, DHCP_MSG_REQUEST);
    pCur = _dhcpc_add_server_id(pCur);
    pCur = _dhcpc_add_req_ipaddr(pCur);
    pCur = _dhcpc_add_end_tag(pCur);

    uip_send(uip_appdata, pCur - (uint8_t *)uip_appdata);
}


static int
_dhcpc_parse_options(uint8_t *optptr, int len)
{
    uint8_t     *pEnd = optptr + len;
    int         type = 0;

    while( optptr < pEnd )
    {
        switch( *optptr )
        {
            case DHCP_OPTION_SUBNET_MASK:
                memcpy(g_dhcpc.netmask, optptr + 2, 4);
                break;
            case DHCP_OPTION_ROUTER:
                memcpy(g_dhcpc.default_router, optptr + 2, 4);
                break;
            case DHCP_OPTION_DNS_SERVER:
                memcpy(g_dhcpc.dnsaddr, optptr + 2, 4);
                break;
            case DHCP_OPTION_MSG_TYPE:
                type = *(optptr + 2);
                break;
            case DHCP_OPTION_SERVER_ID:
                memcpy(g_dhcpc.serverid, optptr + 2, 4);
                break;
            case DHCP_OPTION_LEASE_TIME:
                memcpy(g_dhcpc.lease_time, optptr + 2, 4);
                break;
            case DHCP_OPTION_END:
                return type;

            case DHCP_OPTION_TFTPD_NAME:
                memcpy(g_dhcpc.tftpd_name, optptr + 2, *(optptr + 1));
                break;
            case DHCP_OPTION_TFTPD_IP:
                memcpy(g_dhcpc.tftp_ipaddr, optptr + 2, 4);
                break;
            default:    break;
        }

        optptr += optptr[1] + 2;
    }
    return type;
}


static int
_dhcpc_parse_msg(void)
{
    do {
        dhcp_msg_t  *m = (struct dhcp_msg *)uip_appdata;

        if( !uip_newdata() )
            break;

        if( m->op == DHCP_OP_REPLY &&
            m->xid == g_dhcpc.xid &&
            (m->chaddr[0] == g_dhcpc.mac_addr.addr[0] &&
             m->chaddr[1] == g_dhcpc.mac_addr.addr[1] &&
             m->chaddr[2] == g_dhcpc.mac_addr.addr[2] &&
             m->chaddr[3] == g_dhcpc.mac_addr.addr[3] &&
             m->chaddr[4] == g_dhcpc.mac_addr.addr[4] &&
             m->chaddr[5] == g_dhcpc.mac_addr.addr[5]) )
        {
            *((uint32_t*)&g_dhcpc.ipaddr)      = m->yiaddr;
            *((uint32_t*)&g_dhcpc.next_ipaddr) = m->siaddr;
            memcpy(g_dhcpc.boot_filename, m->filename, DHCP_MSG_FILENAME_SIZE - 1);
            return _dhcpc_parse_options(m->options, uip_datalen());
        }
    } while(0);
    return 0;
}



static
PT_THREAD(handle_dhcp(void))
{
    PT_BEGIN(&g_dhcpc.pt);

    g_dhcpc.state = STATE_SENDING;
    g_dhcpc.ticks = CLOCK_SECOND;

    if( g_dhcpc_phase == DHCPC_PHASE_IP_ALLOC )
    {
        #if defined(CONFIG_MEASURE_TIME)
        g_tm_start = clock_time();
        #endif // defined

        do {
            _dhcpc_send_discover();

            timer_set(&g_dhcpc.timer, g_dhcpc.ticks);
            PT_WAIT_UNTIL(&g_dhcpc.pt, uip_newdata() || timer_expired(&g_dhcpc.timer));

            if( _dhcpc_parse_msg() == DHCP_MSG_OFFER ) {
                g_dhcpc.state = STATE_OFFER_RECEIVED;
                break;
            }

            if( g_dhcpc.ticks < CLOCK_SECOND * 30 ) {
                g_dhcpc.ticks *= 2;
            }
        } while( g_dhcpc.state != STATE_OFFER_RECEIVED );

        #if defined(CONFIG_MEASURE_TIME)
        log("time: %d ms\n", clock_time() - g_tm_start);
        #endif // defined

        #if 0
        log_ip("\n\nGot IP address : ", g_dhcpc.ipaddr, "(ln. %d)\n", __LINE__);
        log_ip("Got netmask    : ", g_dhcpc.netmask, "(ln. %d)\n", __LINE__);
        log_ip("Got DNS        : ", g_dhcpc.dnsaddr, "(ln. %d)\n", __LINE__);
        log_ip("Got def-router : ", g_dhcpc.default_router, "(ln. %d)\n", __LINE__);
        log_ip("Got tftp-svr   : ", g_dhcpc.tftp_ipaddr, "(name= '%s')\n", g_dhcpc.tftpd_name);
        log("Got boot file: %s\n", g_dhcpc.boot_filename);
        log("Lease expires in %ld seconds\n\n",
            uip_ntohs(g_dhcpc.lease_time[0]) * 65536ul + uip_ntohs(g_dhcpc.lease_time[1]));
        #endif

        #if defined(CONFIG_MEASURE_TIME)
        g_tm_start = clock_time();
        #endif // defined

        g_dhcpc.ticks = CLOCK_SECOND;

        do {
            _dhcpc_send_request();
            timer_set(&g_dhcpc.timer, g_dhcpc.ticks);
            PT_YIELD_UNTIL(&g_dhcpc.pt, uip_newdata() || timer_expired(&g_dhcpc.timer));

            if( _dhcpc_parse_msg() == DHCP_MSG_ACK ) {
                g_dhcpc.state = STATE_CONFIG_RECEIVED;
                break;
            }

            if( g_dhcpc.ticks <= CLOCK_SECOND * 10 )
                g_dhcpc.ticks += CLOCK_SECOND;
            else
            {
                PT_RESTART(&g_dhcpc.pt);
            }
        } while( g_dhcpc.state != STATE_CONFIG_RECEIVED );

        #if defined(CONFIG_MEASURE_TIME)
        log("time: %d ms\n", clock_time() - g_tm_start);
        #endif // defined
    }
    else if( g_dhcpc_phase == DHCPC_PHASE_IP_FREE )
    {
        #if defined(CONFIG_MEASURE_TIME)
        g_tm_start = clock_time();
        #endif // defined

        g_dhcpc.ticks = CLOCK_SECOND;

        _dhcpc_send_release();

        timer_set(&g_dhcpc.timer, g_dhcpc.ticks);
        PT_YIELD_UNTIL(&g_dhcpc.pt, uip_newdata() || timer_expired(&g_dhcpc.timer));

        log("@@@ sent DHCPRelease message\n");

        uip_udp_remove(g_dhcpc.conn);

        #if defined(CONFIG_MEASURE_TIME)
        log("time: %d ms\n", clock_time() - g_tm_start);
        #endif // defined
    }

    /**
     *  Configure net setting
     */
    uip_sethostaddr(g_dhcpc.ipaddr);
    uip_setnetmask(g_dhcpc.netmask);
    uip_setdraddr(g_dhcpc.default_router);

    g_dhcpc_is_busy = 0;

    /*
     * PT_END restarts the thread so we do this instead. Eventually we
     * should reacquire expired leases here.
     */
    while(1) {
        PT_YIELD(&g_dhcpc.pt);
    }

    PT_END(&g_dhcpc.pt);
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
dhcpc_err_t
dhcpc_init(struct uip_eth_addr  *pMac_addr)
{
    dhcpc_err_t     rval = DHCPC_ERR_OK;
    do {
        uip_ipaddr_t    broadcast_ipaddr;

        memset(&g_dhcpc, 0x0, sizeof(g_dhcpc));

        g_dhcpc.xid   = _dhcpc_gen_xid();
        g_dhcpc.state = STATE_INITIAL;

        PT_INIT(&g_dhcpc.pt);

        memcpy(&g_dhcpc.mac_addr, pMac_addr, sizeof(struct uip_eth_addr));

        uip_ipaddr(broadcast_ipaddr, 255, 255, 255, 255);
        g_dhcpc.conn = uip_udp_new(&broadcast_ipaddr, UIP_HTONS(DHCPC_SERVER_PORT));
        if( !g_dhcpc.conn )
        {
            rval = DHCPC_ERR_CONNECTION_FULL;
            break;
        }

        uip_udp_bind(g_dhcpc.conn, UIP_HTONS(DHCPC_CLIENT_PORT));

        g_dhcpc_phase   = DHCPC_PHASE_IP_ALLOC;
        g_dhcpc_is_busy = 1;

    } while(0);
    return rval;
}

void
dhcpc_appcall(void)
{
    handle_dhcp();
}

uint32_t
dhcpc_is_busy(void)
{
    return g_dhcpc_is_busy;
}

dhcpc_err_t
dhcpc_get_tftp_server(uip_ipaddr_t *pAddr)
{
    dhcpc_err_t     rval = DHCPC_ERR_OK;
    do {
        uip_ipaddr_t    mask;
        uip_ipaddr(&mask, 255, 255, 0, 0);

        // tftp server and received IP MUST be in the same LAN
        if( !uip_ipaddr_maskcmp(g_dhcpc.tftp_ipaddr, g_dhcpc.ipaddr, mask) )
        {
            rval = DHCPC_ERR_NO_TFTP_SVR;
            break;
        }

        memcpy(pAddr, g_dhcpc.tftp_ipaddr, sizeof(uip_ipaddr_t));
    } while(0);
    return rval;
}

char*
dhcpc_get_boot_filename(void)
{
    return g_dhcpc.boot_filename;
}

void
dhcpc_release_ip(void)
{
    PT_INIT(&g_dhcpc.pt);
    memset(g_dhcpc.ipaddr, 0x0, sizeof(g_dhcpc.ipaddr));
    memset(g_dhcpc.netmask, 0x0, sizeof(g_dhcpc.netmask));
    memset(g_dhcpc.default_router, 0x0, sizeof(g_dhcpc.default_router));

    g_dhcpc_phase   = DHCPC_PHASE_IP_FREE;
    g_dhcpc_is_busy = 1;
    return;
}
