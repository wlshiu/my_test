/**
 * Copyright (c) 2022 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ymodem.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2022/06/17
 * @license
 * @description
 */

#ifndef __ymodem_H_whJOxi2G_lZio_H42C_sNeE_uHIF4YyaGQbj__
#define __ymodem_H_whJOxi2G_lZio_H42C_sNeE_uHIF4YyaGQbj__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_DOWNLOAD_TIMEOUT            ((unsigned int)1000) /* 1 second retry delay */

typedef enum ymodem_state_t
{
    YMODEM_STATE_OK       = 0,
    YMODEM_STATE_ERROR    = -1,
    YMODEM_STATE_ABORT    = -2,
    YMODEM_STATE_TIMEOUT  = -3,
    YMODEM_STATE_DATA     = -4,
    YMODEM_STATE_LIMIT    = -5,
} ymodem_state_t;


//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int YmodemReceive(unsigned char *dest, int destsz);

int YmodemTransmit(unsigned char *src, int srcsz, char *name);

#ifdef __cplusplus
}
#endif

#endif
