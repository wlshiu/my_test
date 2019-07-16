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
#include "uip.h"


#define TFTP_PORT               69
#define TFTP_MAX_FNAME_LEN      128


typedef enum tftpc_err
{
    TFTPC_ERR_SYS_BUSY          = 2,
    TFTPC_ERR_NO_INCOME         = 1,
    TFTPC_ERR_OK                = 0,
    TFTPC_ERR_SERVER_DOWN       = -1,
    TFTPC_ERR_FILE_NOT_FOUND    = -2,
    TFTPC_ERR_FAIL              = -3,
    TFTPC_ERR_FLASH_FAIL        = -4,
    TFTPC_ERR_UNKNOWN_OPCODE    = -5,
    TFTPC_ERR_CONNECTION_FULL   = -6,

} tftpc_err_t;


typedef enum tftp_errcode
{
    TFTP_ERRCODE_FILE_NOT_FOUND     = 1,    /**< File not found */
    TFTP_ERRCODE_ACCESS_DENIED      = 2,    /**< Access violation */
    TFTP_ERRCODE_DISK_FULL          = 3,    /**< Disk full or allocation exceeded */
    TFTP_ERRCODE_ILLEGAL_OP         = 4,    /**< Illegal TFTP operation */
    TFTP_ERRCODE_UNKNOWN_TID        = 5,    /**< Unknown transfer ID */
    TFTP_ERRCODE_FILE_EXISTS        = 6,    /**< File already exists */
    TFTP_ERRCODE_UNKNOWN_USER       = 7,    /**< No such user */
} tftp_errcode_t;


void
tftpc_init(void);


tftpc_err_t
tftpc_get(
    uip_ipaddr_t    *pSvr_ipaddr,
    char            *fname);


uint32_t
tftpc_is_busy(void);


tftpc_err_t
tftpc_get_result(void);


void tftpc_appcall(void);


#endif /* TFTP_H_ */
