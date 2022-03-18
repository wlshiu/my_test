/*
 * Copyright 2001-2021 Georges Menie (www.menie.org)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* this code needs standard functions memcpy() and memset()
   and input/output functions _inbyte() and _outbyte().

   the prototypes of the input/output functions are:
     int _inbyte(unsigned short timeout); // msec timeout
     void _outbyte(int c);

 */

#include <stdio.h>
#include <string.h>
#include "crc16.h"
#include "xmodem.h"


#define SOH     0x01
#define STX     0x02
#define EOT     0x04
#define ACK     0x06
#define NAK     0x15
#define CAN     0x18
#define CTRLZ   0x1A

#define DLY_1S      1000
#define MAXRETRANS  25

typedef enum xfer_mode
{
    XFER_MODE_TX,
    XFER_MODE_RX

} xfer_mode_t;

extern int tx_inbyte(unsigned short timeout);
extern void tx_outbyte(int c);

extern int rx_inbyte(unsigned short timeout);
extern void rx_outbyte(int c);

static int check(int crc, const unsigned char *buf, int sz)
{
    if (crc)
    {
        unsigned short crc = crc16_ccitt(buf, sz);
        unsigned short tcrc = (buf[sz] << 8) + buf[sz + 1];
        if (crc == tcrc)
            return 1;
    }
    else
    {
        int i;
        unsigned char cks = 0;
        for (i = 0; i < sz; ++i)
        {
            cks += buf[i];
        }
        if (cks == buf[sz])
            return 1;
    }

    return 0;
}

static void flushinput(xfer_mode_t mode)
{
    if( mode == XFER_MODE_TX )
    {
        while (tx_inbyte(((DLY_1S) * 3) >> 1) >= 0) {}
    }
#if 0
    else if( mode == XFER_MODE_RX )
    {
        while (rx_inbyte(((DLY_1S) * 3) >> 1) >= 0) {}
    }
#endif
    return;
}

#if 0
xmodem_state_t xmodemReceive(unsigned char *dest, int destsz)
{
    unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
    unsigned char *p;
    int bufsz, crc = 0;
    unsigned char trychar = 'C';
    unsigned char packetno = 1;
    int i, c, len = 0;
    int retry, retrans = MAXRETRANS;

    for(;;)
    {
        for( retry = 0; retry < 16; ++retry)
        {
            if (trychar)
                rx_outbyte(trychar);

            if ((c = rx_inbyte((DLY_1S) << 1)) >= 0)
            {
                switch (c)
                {
                    case SOH:
                        bufsz = 128;
                        goto start_recv;
                    case STX:
                        bufsz = 1024;
                        goto start_recv;
                    case EOT:
                        flushinput(XFER_MODE_RX);
                        rx_outbyte(ACK);
                        return len; /* normal end */
                    case CAN:
                        if ((c = rx_inbyte(DLY_1S)) == CAN)
                        {
                            flushinput(XFER_MODE_RX);
                            rx_outbyte(ACK);
                            return XMODEM_STATE_CANCELED_BY_REMOTE; /* canceled by remote */
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        if (trychar == 'C')
        {
            trychar = NAK;
            continue;
        }
        flushinput(XFER_MODE_RX);
        rx_outbyte(CAN);
        rx_outbyte(CAN);
        rx_outbyte(CAN);
        return XMODEM_STATE_SYNC_ERROR; /* sync error */

start_recv:
        if (trychar == 'C')
            crc = 1;

        trychar = 0;
        p = xbuff;
        *p++ = c;
        for (i = 0;  i < (bufsz + (crc ? 1 : 0) + 3); ++i)
        {
            if ((c = rx_inbyte(DLY_1S)) < 0)
                goto reject;

            *p++ = c;
        }

        if( xbuff[1] == (unsigned char)(~xbuff[2]) &&
            (xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno - 1) &&
            check(crc, &xbuff[3], bufsz))
        {
            if (xbuff[1] == packetno)
            {
                register int count = destsz - len;
                if (count > bufsz)
                    count = bufsz;

                if (count > 0)
                {
                    #if 0
                    #else
                    memcpy(&dest[len], &xbuff[3], count);
                    #endif

                    len += count;
                }
                ++packetno;
                retrans = MAXRETRANS + 1;
            }
            if (--retrans <= 0)
            {
                flushinput(XFER_MODE_RX);
                rx_outbyte(CAN);
                rx_outbyte(CAN);
                rx_outbyte(CAN);
                return XMODEM_STATE_OVER_RETRY_TIMES; /* too many retry error */
            }
            rx_outbyte(ACK);
            continue;
        }
reject:
        flushinput(XFER_MODE_RX);
        rx_outbyte(NAK);
    }
}
#endif

xmodem_state_t xmodemTransmit(unsigned char *src, int srcsz)
{
    unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
    int bufsz, crc = -1;
    unsigned char packetno = 1;
    int i, c, len = 0;
    int retry;

    for(;;)
    {
        for( retry = 0; retry < 16; ++retry)
        {
            if ((c = tx_inbyte((DLY_1S) << 1)) >= 0)
            {
                switch (c)
                {
                    case 'C':
                        crc = 1;
                        goto start_trans;
                    case NAK:
                        crc = 0;
                        goto start_trans;
                    case CAN:
                        if ((c = tx_inbyte(DLY_1S)) == CAN)
                        {
                            tx_outbyte(ACK);
                            flushinput(XFER_MODE_TX);
                            return XMODEM_STATE_CANCELED_BY_REMOTE; /* canceled by remote */
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        tx_outbyte(CAN);
        tx_outbyte(CAN);
        tx_outbyte(CAN);
        flushinput(XFER_MODE_TX);
        return XMODEM_STATE_SYNC_ERROR; /* no sync */

        for(;;)
        {
start_trans:
            xbuff[0] = SOH;
            bufsz = 128;
            xbuff[1] = packetno;
            xbuff[2] = ~packetno;
            c = srcsz - len;
            if (c > bufsz)
                c = bufsz;

            if (c >= 0)
            {
                memset(&xbuff[3], 0, bufsz);
                if (c == 0)
                {
                    xbuff[3] = CTRLZ;
                }
                else
                {
                    memcpy(&xbuff[3], &src[len], c);
                    if (c < bufsz)
                        xbuff[3 + c] = CTRLZ;
                }
                if (crc)
                {
                    unsigned short ccrc = crc16_ccitt(&xbuff[3], bufsz);
                    xbuff[bufsz + 3] = (ccrc >> 8) & 0xFF;
                    xbuff[bufsz + 4] = ccrc & 0xFF;
                }
                else
                {
                    unsigned char ccks = 0;
                    for (i = 3; i < bufsz + 3; ++i)
                    {
                        ccks += xbuff[i];
                    }
                    xbuff[bufsz + 3] = ccks;
                }
                for (retry = 0; retry < MAXRETRANS; ++retry)
                {
                    for (i = 0; i < bufsz + 4 + (crc ? 1 : 0); ++i)
                    {
                        tx_outbyte(xbuff[i]);
                    }
                    if ((c = tx_inbyte(DLY_1S)) >= 0 )
                    {
                        switch (c)
                        {
                            case ACK:
                                ++packetno;
                                len += bufsz;
                                goto start_trans;
                            case CAN:
                                if ((c = tx_inbyte(DLY_1S)) == CAN)
                                {
                                    tx_outbyte(ACK);
                                    flushinput(XFER_MODE_TX);
                                    return XMODEM_STATE_CANCELED_BY_REMOTE; /* canceled by remote */
                                }
                                break;
                            case NAK:
                            default:
                                break;
                        }
                    }
                }
                tx_outbyte(CAN);
                tx_outbyte(CAN);
                tx_outbyte(CAN);
                flushinput(XFER_MODE_TX);
                return XMODEM_STATE_XMIT_ERR; /* xmit error */
            }
            else
            {
                for (retry = 0; retry < 10; ++retry)
                {
                    tx_outbyte(EOT);
                    if ((c = tx_inbyte((DLY_1S) << 1)) == ACK)
                        break;
                }
                flushinput(XFER_MODE_TX);
                return (c == ACK) ? len : XMODEM_STATE_FAIL;
            }
        }
    }
}


