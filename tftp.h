/*
 * tftp.h
 *
 *  Created on: Feb 1, 2011
 *      Author: James Harwood
 *
 * This module is free software and there is NO WARRANTY.
 * No restrictions on use. You can use, modify and redistribute it for
   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
 */

#ifndef TFTP_H_
#define TFTP_H_

#include "uip_timer.h"
#include "pt.h"
#include "uip-conf.h"

#define TFTP_PORT           69
#define MAX_FNAME_LEN       128

#define TFTPC_SUCCESS			0
#define TFTPC_SERVER_DOWN		1
#define TFTPC_FILE_NOT_FOUND	2
#define TFTPC_ERROR				3
#define TFTPC_FLASH_ERROR		4

typedef struct tftpc_state
{
    struct pt pt;
    char state;
    struct uip_udp_conn *conn;
    struct timer timer;
    uint16_t ticks;
    uint16_t retries;
    uint16_t blocknum;
    uint16_t tftp_error;
    uint16_t flash_error;
    uint16_t datalen;
    uint8_t duplicate;
    char fname[MAX_FNAME_LEN];
} tftpc_t;

void tftpc_init(void);
uint8_t tftpc_get(char *fname);
uint8_t tftpc_busy();
uint8_t tftpc_result();

void tftpc_appcall(void);

//typedef struct tftpc_state uip_udp_appstate_t;
//#define UIP_UDP_APPCALL tftpc_appcall

#endif /* TFTP_H_ */
