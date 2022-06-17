/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ymodem.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/06/17
 * @license
 * @description
 */


#include <string.h>
#include "ymodem.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define YPACKET_HEADER_SIZE         ((unsigned int)3)
#define YPACKET_TRAILER_SIZE        ((unsigned int)2)

#define YPACKET_DATA_INDEX          ((unsigned int)4)
#define YPACKET_START_INDEX         ((unsigned int)1)
#define YPACKET_NUMBER_INDEX        ((unsigned int)2)
#define YPACKET_CNUMBER_INDEX       ((unsigned int)3)

#define YPACKET_OVERHEAD_SIZE       (YPACKET_HEADER_SIZE + YPACKET_TRAILER_SIZE - 1)

#define YPACKET_SIZE                ((unsigned int)128)
#define YPACKET_1K_SIZE             ((unsigned int)1024)

/**
 * +-------- Packet in IAP memory ------------------------------------------+
 * | 0      |  1    |  2     |  3   |  4      | ... | n+4     | n+5  | n+6  |
 * |------------------------------------------------------------------------|
 * | unused | start | number | !num | data[0] | ... | data[n] | crc0 | crc1 |
 * +------------------------------------------------------------------------+
 * the first byte is left unused for memory alignment reasons
 */

#define FILE_NAME_LENGTH        ((unsigned int)64)
#define FILE_SIZE_LENGTH        ((unsigned int)16)

#define TAG_SOH                     ((unsigned char)0x01)  /* start of 128-byte data packet */
#define TAG_STX                     ((unsigned char)0x02)  /* start of 1024-byte data packet */
#define TAG_EOT                     ((unsigned char)0x04)  /* end of transmission */
#define TAG_ACK                     ((unsigned char)0x06)  /* acknowledge */
#define TAG_NAK                     ((unsigned char)0x15)  /* negative acknowledge */
#define TAG_CAN                     ((unsigned char)0x18)  /* two of these in succession aborts transfer */
#define TAG_CRC16                   ((unsigned char)0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define NEGATIVE_BYTE               ((unsigned char)0xFF)

#define TAG_ABORT1                  ((unsigned char)0x41)  /* 'A' == 0x41, abort by user */
#define TAG_ABORT2                  ((unsigned char)0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT                 ((unsigned int)0x100000)

#define CONFIG_MAX_ERRORS           ((unsigned int)5)


#define STATE_OK            0
#define STATE_ERROR         -1
#define STATE_BUSY          -2
//=============================================================================
//                  Macro Definition
//=============================================================================
#define IS_VALID_HEX(c)         (((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f') || ((c) >= '0' && (c) <= '9'))
#define IS_VALID_DEC(c)         ((c) >= '0' && (c) <= '9')

#define CONVERT_HEX(c)          (((c) >= '0' && (c) <= '9') ? ((c) - '0') :       \
                                 ((c) >= 'a' && (c) <= 'f') ? ((c) - 'a' + 10) :  \
                                 ((c) - 'A' + 10))

#define CONVERT_DEC(c)          ((c) - '0')

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern int tx_inbyte(unsigned short timeout);
extern void tx_outbyte(int c);

extern int rx_inbyte(unsigned short timeout);
extern void rx_outbyte(int c);

static unsigned char    rxbuf[YPACKET_1K_SIZE + YPACKET_DATA_INDEX + YPACKET_TRAILER_SIZE];
static unsigned char    txbuf[YPACKET_1K_SIZE + YPACKET_DATA_INDEX + YPACKET_TRAILER_SIZE];
static unsigned char    g_FileName[FILE_NAME_LENGTH] = {0};

//=============================================================================
//                  Private Function Definition
//=============================================================================
#if 1
static unsigned short _calc_crc16(const unsigned char *buf, unsigned int count)
{
    unsigned short crc = 0;

    while( count-- )
    {
        crc = crc ^ *buf++ << 8;

        for(int i = 0; i < 8; i++)
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        }
    }
    return crc;
}


#else

static unsigned short _update_crc16(unsigned short crc_in, unsigned char byte)
{
    unsigned int    crc = crc_in;
    unsigned int    in = byte | 0x100;

    do {
        crc <<= 1;
        in <<= 1;
        if( in & 0x100 )
            ++crc;

        if( crc & 0x10000 )
            crc ^= 0x1021;

    } while( !(in & 0x10000) );

    return crc & 0xFFFFu;
}

static unsigned short _calc_crc16(const unsigned char *pData, unsigned int size)
{
    unsigned int            crc = 0;
    const unsigned char     *dataEnd = pData + size;

    while( pData < dataEnd )
        crc = _update_crc16(crc, *pData++);

    crc = _update_crc16(crc, 0);
    crc = _update_crc16(crc, 0);

    return crc & 0xFFFFu;
}

#endif // 1

/**
 *  \brief  Convert an integer to a string
 *
 *  \param [in] pStr        The string output pointer (length MUST be more than 11 bytes.)
 *  \param [in] value       The integer to be converted
 *  \return
 *      None
 */
static void
_int2str(char *pStr, unsigned int value)
{
    /* Maximum number of decimal digits in u32 is 10 */
    int     pos = 10;

    pStr[10] = 0;

    if( value == 0 )
    {
        /* If already zero then just return zero */
        pStr[0] = '0';
        pStr[1] = 0;
        return;
    }

    while( (value != 0) && (pos > 0) )
    {
        pStr[--pos] = (value % 10) + '0';
        value /= 10;
    }

    return;
}

/**
 *  \brief  Convert a string to an integer
 *
 *  \param [in] pInput      The string to be converted
 *  \param [in] pValue      The integer value
 *  \return
 *      0: Correct, -1: Error
 */
static int
_str2int(char *pInput, int *pValue)
{
    int             i = 0;
    int             rval = 0;
    unsigned int    value = 0;

    if( pInput[0] == '0' && (pInput[1] == 'x' || pInput[1] == 'X') )
    {
        i = 2;
        while( i < 11 && pInput[i] != '\0' )
        {
            if( IS_VALID_HEX(pInput[i]) )
            {
                value = (value << 4) + CONVERT_HEX(pInput[i]);
            }
            else
            {
                /* Invalid input */
                rval = -1;
                break;
            }
            i++;
        }

        /* valid result */
        if( pInput[i] == '\0' )
        {
            *pValue = value;
        }
    }
    else /* max 10-digit decimal input */
    {
        while( i < 11 && rval != 1 )
        {
            if( pInput[i] == '\0' )
            {
                *pValue = value;
            }
            else if( (pInput[i] == 'k' || pInput[i] == 'K') && i > 0 )
            {
                value = value << 10;
                *pValue = value;
            }
            else if( (pInput[i] == 'm' || pInput[i] == 'M') && i > 0 )
            {
                value = value << 20;
                *pValue = value;
            }
            else if( IS_VALID_DEC(pInput[i]) )
            {
                value = value * 10 + CONVERT_DEC(pInput[i]);
            }
            else
            {
                /* Invalid input */
                rval = -1;
                break;
            }
            i++;
        }
    }

    return rval;
}


static int
_receive_packet(unsigned char *pData, unsigned int *pLength, unsigned int timeout)
{
    int             rval = STATE_OK;
    unsigned int    crc;
    unsigned int    packet_size = 0;
    int             c;

    *pLength = 0;

    do {
        if( (c = rx_inbyte(timeout)) < 0 )
        {
            rval = STATE_ERROR;
            break;
        }

        switch( c )
        {
            case TAG_SOH:
                packet_size = YPACKET_SIZE;
                break;
            case TAG_STX:
                packet_size = YPACKET_1K_SIZE;
                break;
            case TAG_EOT:
                return STATE_OK;

            case TAG_CAN:
                if( (c = rx_inbyte(timeout)) == TAG_CAN )
                {
                    packet_size = 2;
                    break;
                }
            default:
                return STATE_ERROR;

            case TAG_ABORT1:
            case TAG_ABORT2:
                return STATE_BUSY;
        }

        *pData = c;
        if( packet_size < YPACKET_SIZE )
            break;

        for(int i = 1; i < (packet_size + YPACKET_OVERHEAD_SIZE); i++)
        {
            if( (c = rx_inbyte(timeout)) < 0 )
            {
                packet_size = 0;
                return STATE_ERROR;
            }

            *pData++ = c;
        }

        if( pData[YPACKET_NUMBER_INDEX] != (pData[YPACKET_CNUMBER_INDEX] ^ NEGATIVE_BYTE) )
        {
            packet_size = 0;
            rval = STATE_ERROR;
            break;
        }

        /* Check packet CRC */
        crc = (pData[packet_size + YPACKET_DATA_INDEX] << 8) | pData[packet_size + YPACKET_DATA_INDEX + 1];

        if( _calc_crc16(&pData[YPACKET_DATA_INDEX], packet_size) != crc )
        {
            packet_size = 0;
            rval = STATE_ERROR;
        }

    } while(0);

    *pLength = packet_size;
    return rval;
}

/**
 *  \brief  Prepare the first block
 *
 *  \param [in] pData           output buffer
 *  \param [in] pFileName       name of the file to be sent
 *  \param [in] length          length of the file to be sent in bytes
 *  \return
 *      None
 */
static void _gen_packet0(unsigned char *pData, const char *pFileName, int length)
{
    int     i, j = 0;
    char    str_size[11] = {0};

    /* first 3 bytes are constant */
    pData[YPACKET_START_INDEX]   = TAG_SOH;
    pData[YPACKET_NUMBER_INDEX]  = 0x00;
    pData[YPACKET_CNUMBER_INDEX] = 0xff;

    /* Filename written */
    for(i = 0; pFileName[i] != '\0' && i < FILE_NAME_LENGTH; i++)
    {
        pData[i + YPACKET_DATA_INDEX] = pFileName[i];
    }

    pData[i + YPACKET_DATA_INDEX] = 0x00;

    /* file size written */
    _int2str(str_size, length);
    i = i + YPACKET_DATA_INDEX + 1;

    j = 0;
    while( str_size[j] != '\0' )
    {
        pData[i++] = str_size[j++];
    }

    /* padding with zeros */
    for(j = i; j < YPACKET_SIZE + YPACKET_DATA_INDEX; j++)
    {
        pData[j] = 0;
    }

    return;
}


static int
_send_packet0(unsigned char *pData, int length, unsigned int timeout)
{
    int             rval = YMODEM_STATE_OK;
    char            err_cnt = 0;
    unsigned char   ack_recpt = 0;

    while( ack_recpt == 0 && rval == YMODEM_STATE_OK )
    {
        int             c = 0;
        unsigned short  crc16 = 0;

        for(int i = 0; i < length; i++)
        {
            tx_outbyte(pData[YPACKET_START_INDEX + i]);
        }

        /* Send CRC */
        crc16 = _calc_crc16(&pData[YPACKET_DATA_INDEX], YPACKET_SIZE);

        tx_outbyte(crc16 >> 8);
        tx_outbyte(crc16 & 0xFF);

        /* Wait for Ack and 'C' */
        if( (c = rx_inbyte(timeout)) < 0 )
        {
            if( err_cnt++ > CONFIG_MAX_ERRORS )
            {
                rval = YMODEM_STATE_ERROR;
                break;
            }

            continue;
        }

        if( c == TAG_ACK )
            ack_recpt = 1;
        else if( c == TAG_CAN )
        {
            if( (c = rx_inbyte(timeout)) == TAG_CAN )
            {
                rval = YMODEM_STATE_ABORT;
                break;
            }
        }
    }
    return rval;
}

/**
 *  \brief  Prepare the data packet
 *
 *  \param [in] pSource             pointer to the data to be sent
 *  \param [in] pPkt                pointer to the output buffer
 *  \param [in] pkt_idx             index of the packet
 *  \param [in] size_blk            length of the block to be sent in bytes
 *  \return
 *      size of a packet
 */
static int
_gen_data_packet(unsigned char *pSource, unsigned char *pPkt, int pkt_idx, int size_blk)
{
    unsigned char   *pCur = 0;
    int             pkt_size = 0;
    int             packet_size = 0;

    /* Make first three packet */
    packet_size = (size_blk >= YPACKET_1K_SIZE) ? YPACKET_1K_SIZE : YPACKET_SIZE;
    pkt_size    = (size_blk < packet_size) ? size_blk : packet_size;

    pPkt[YPACKET_START_INDEX]   = (packet_size == YPACKET_1K_SIZE) ? TAG_STX : TAG_SOH;
    pPkt[YPACKET_NUMBER_INDEX]  = pkt_idx;
    pPkt[YPACKET_CNUMBER_INDEX] = (~pkt_idx);

    pCur = pSource;

    /* Filename packet has valid data */
    for(int i = YPACKET_DATA_INDEX; i < pkt_size + YPACKET_DATA_INDEX; i++)
    {
        pPkt[i] = *pCur++;
    }

    if( pkt_size <= packet_size )
    {
        for(int i = pkt_size + YPACKET_DATA_INDEX; i < packet_size + YPACKET_DATA_INDEX; i++)
        {
            pPkt[i] = 0x1A; /* EOF (0x1A) or 0x00 */
        }
    }
    return pkt_size;
}

/**
 *  \brief  Send a data packet
 *
 *  \param [in] pData           pointer to the data to be sent
 *  \param [in] size_blk        total size
 *  \param [in] timeout         the timeout of xfer
 *  \return
 *      size of a packet
 */
static int
_send_data_packet(unsigned char *pData, int size_blk, unsigned int timeout)
{
    int             rval = YMODEM_STATE_OK;
    char            err_cnt = 0;
    unsigned char   ack_recpt = 0;
    int             packet_size = 0;

    /* Resend packet if NAK for few times else end of communication */
    while( ack_recpt == 0 && rval == YMODEM_STATE_OK )
    {
        int             c = 0;
        unsigned short  crc16 = 0;

        /* Send next packet */
        packet_size = (size_blk < YPACKET_1K_SIZE) ? YPACKET_SIZE : YPACKET_1K_SIZE;

        for(int i = 0; i < packet_size + YPACKET_HEADER_SIZE; i++)
        {
            tx_outbyte(pData[YPACKET_START_INDEX + i]);
        }

        /* Send CRC */
        crc16 = _calc_crc16(&pData[YPACKET_DATA_INDEX], packet_size);
        tx_outbyte(crc16 >> 8);
        tx_outbyte(crc16 & 0xFF);

        /* Wait for Ack */
        if( (c = rx_inbyte(timeout)) != TAG_ACK )
        {
            if( err_cnt++ > CONFIG_MAX_ERRORS )
            {
                packet_size = 0;

                rval = YMODEM_STATE_ERROR;
                break;
            }

            continue;
        }

        ack_recpt = 1;
    }

    return packet_size;
}

/**
 *  \brief  Setnd end code
 *
 *  \param [in] timeout     the timeout of xfer
 *  \return
 *
 */
static int
_send_end(unsigned int timeout)
{
    int             rval = YMODEM_STATE_OK;
    char            err_cnt = 0;
    unsigned char   ack_recpt = 0;

    /* Sending End Of Transmission char */
    while( ack_recpt == 0 && rval == YMODEM_STATE_OK )
    {
        int     c = 0;

        tx_outbyte(TAG_EOT);

        /* Wait for Ack */
        if( (c = rx_inbyte(timeout)) < 0 )
        {
            if( err_cnt++ > CONFIG_MAX_ERRORS )
            {
                rval = YMODEM_STATE_ERROR;
                break;
            }

            continue;
        }

        if( c == TAG_ACK )
            ack_recpt = 1;
        else if( c == TAG_CAN )
        {
            if( (c = rx_inbyte(timeout)) == TAG_CAN )
            {
                rval = YMODEM_STATE_ABORT;
                break;
            }
        }
    }

    return rval;
}

static int
_send_empty_packat(unsigned char *pOut_buf, int timeout)
{
    int             rval = YMODEM_STATE_OK;
    int             c;
    unsigned short  crc16 = 0;

    /* Preparing an empty packet */
    pOut_buf[YPACKET_START_INDEX]   = TAG_SOH;
    pOut_buf[YPACKET_NUMBER_INDEX]  = 0;
    pOut_buf[YPACKET_CNUMBER_INDEX] = 0xFF;
    for(int i = YPACKET_DATA_INDEX; i < (YPACKET_SIZE + YPACKET_DATA_INDEX); i++)
    {
        pOut_buf [i] = 0x00;
    }

    /* Send Packet */
    for(int i = YPACKET_START_INDEX; i < YPACKET_SIZE + YPACKET_HEADER_SIZE; i++)
    {
        tx_outbyte(pOut_buf[i]);
    }

    /* Send CRC */
    crc16 = _calc_crc16(&pOut_buf[YPACKET_DATA_INDEX], YPACKET_SIZE);
    tx_outbyte(crc16 >> 8);
    tx_outbyte(crc16 & 0xFF);

    /* Wait for Ack and 'C' */
    if( (c = rx_inbyte(timeout)) == TAG_CAN )
    {
        rval = YMODEM_STATE_ABORT;
    }

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int YmodemReceive(unsigned char *dest, int destsz)
{
    int             rval = YMODEM_STATE_OK;
    int             rbytes = 0;
    unsigned char   session_done = 0;
    unsigned char   file_done = 0;
    unsigned char   session_begin = 0;
    unsigned char   err_cnt = 0;
    unsigned int    rx_pkt_cnt = 0;
    unsigned int    packet_len = 0;

    while( session_done == 0 && rval == YMODEM_STATE_OK )
    {
        rx_pkt_cnt = 0;
        file_done = 0;

        while( file_done == 0 && rval == YMODEM_STATE_OK )
        {
            switch( _receive_packet(rxbuf, &packet_len, CONFIG_DOWNLOAD_TIMEOUT) )
            {
                case STATE_OK: /* ready to receive data */
                    err_cnt = 0;
                    switch( packet_len )
                    {
                        case 2:
                            /* Abort by sender */
                            rx_outbyte(TAG_ACK);
                            rval = YMODEM_STATE_ABORT;
                            break;
                        case 0:
                            /* End of transmission */
                            rx_outbyte(TAG_ACK);
                            file_done = 1;
                            break;
                        default:
                            /* Normal packet */
                            if( rxbuf[YPACKET_NUMBER_INDEX] != (rx_pkt_cnt & 0xFF) )
                            {
                                rx_outbyte(TAG_NAK);
                                break;
                            }

                            if( rx_pkt_cnt == 0 ) /* File name packet */
                            {
                                unsigned char    *pCur = 0;

                                if( rxbuf[YPACKET_DATA_INDEX] == 0 )
                                {
                                    /* File header packet is empty, end session */
                                    rx_outbyte(TAG_ACK);
                                    file_done = 1;
                                    session_done = 1;
                                    break;
                                }

                                /* File name extraction */
                                memset(g_FileName, 0x0, sizeof(g_FileName));

                                pCur = &rxbuf[YPACKET_DATA_INDEX];
                                for(int i = 0; i < sizeof(g_FileName) - 1; i++)
                                {
                                    if( *pCur == 0 )
                                    {
                                        g_FileName[i] = 0;
                                        break;
                                    }

                                    g_FileName[i] = *pCur++;
                                }

                                /* File size extraction */
                                {
                                    unsigned char   file_size[FILE_SIZE_LENGTH] = {0};

                                    pCur++;
                                    for(int i = 0; i < sizeof(file_size); i++)
                                    {
                                        if( *pCur == ' ' )
                                            break;

                                        file_size[i] = *pCur++;
                                    }

                                    _str2int((char*)file_size, &rbytes);
                                }

                                if( rbytes > destsz )
                                {
                                    rx_outbyte(TAG_CAN);
                                    rx_outbyte(TAG_CAN);
                                    rval = YMODEM_STATE_LIMIT;
                                    break;
                                }

                                memset(dest, 0xAA, destsz);

                                rx_outbyte(TAG_ACK);
                                rx_outbyte(TAG_CRC16);
                            }
                            else
                            {
                                /* Data packet: write rx_data to destination address */
                                if( destsz < packet_len )
                                {
                                    rx_outbyte(TAG_CAN);
                                    rx_outbyte(TAG_CAN);
                                    rval = YMODEM_STATE_DATA;
                                    break;
                                }

                                memcpy(dest, &rxbuf[YPACKET_DATA_INDEX], packet_len);

                                destsz -= packet_len;
                                dest   += packet_len;

                                rx_outbyte(TAG_ACK);

                                if( rval != 0 )
                                {
                                    /* if fail, end session */
                                    rx_outbyte(TAG_CAN);
                                    rx_outbyte(TAG_CAN);
                                    rval = YMODEM_STATE_DATA;
                                }
                            }

                            rx_pkt_cnt++;
                            session_begin = 1;
                            break;
                    }
                    break;

                case STATE_BUSY:
                    rx_outbyte(TAG_CAN);
                    rx_outbyte(TAG_CAN);
                    rval = YMODEM_STATE_ABORT;
                    break;

                default: /* error handle */
                    if( session_begin )
                        err_cnt++;

                    if( err_cnt > CONFIG_MAX_ERRORS )
                    {
                        /* Abort communication */
                        rx_outbyte(TAG_CAN);
                        rx_outbyte(TAG_CAN);
                    }
                    else
                    {
                        /* Ask for a packet */
                        rx_outbyte(TAG_CRC16);
                    }
                    break;
            }
        }
    }
    return rval;
}



int YmodemTransmit(unsigned char *src, int srcsz, char *name)
{
    int     rval = 0;
    int     c = 0;

    int     retry = 0;

    do {
        c = tx_inbyte(CONFIG_DOWNLOAD_TIMEOUT);
    } while( retry++ < 6000 && c != TAG_CRC16 );

    do {
        int     blk_index = 1;
        int     blk_index_max = srcsz / YPACKET_1K_SIZE;

        if( c != TAG_CRC16 )
        {
            tx_outbyte(TAG_CAN);
            tx_outbyte(TAG_CAN);
            rval = YMODEM_STATE_TIMEOUT;
            break;
        }

        _gen_packet0(txbuf, name, srcsz);

        /* Send Packet0 */
        rval = _send_packet0(txbuf, YPACKET_SIZE + YPACKET_HEADER_SIZE, NAK_TIMEOUT);
        if( rval != YMODEM_STATE_OK )   break;

        /* Here 1024 bytes length is used to send the packets */
        while( srcsz )
        {
            int     pkt_size = 0;

            pkt_size = _gen_data_packet(&src[0], txbuf, blk_index, srcsz);

            pkt_size = _send_data_packet(txbuf, srcsz, NAK_TIMEOUT);
            if( pkt_size == 0 )
            {
                rval = YMODEM_STATE_ERROR;
                break;
            }

            src += pkt_size;

            if( srcsz > pkt_size )
            {
                srcsz -= pkt_size;

                if( blk_index == blk_index_max )
                {
                    rval = YMODEM_STATE_LIMIT;
                    break;
                }

                blk_index++;
            }
            else
            {
                srcsz = 0;
            }
        }

        if( rval != YMODEM_STATE_OK )
            break;

        /* Sending End Of Transmission char */
        rval = _send_end(NAK_TIMEOUT);
        if( rval != YMODEM_STATE_OK )
            break;

        /* Empty packet sent - some terminal emulators need this to close session */
        _send_empty_packat(txbuf, NAK_TIMEOUT);
    } while(0);

    return rval;
}
