/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file ut_common.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/09/03
 * @license
 * @description
 */

#ifndef __ut_common_H_wPJH6tY4_l5Wd_H8r8_s6QZ_uG72PEcq0R3Y__
#define __ut_common_H_wPJH6tY4_l5Wd_H8r8_s6QZ_uG72PEcq0R3Y__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_COMM_DEV_BAUDRATE        19200//9600

typedef enum ut_err
{
    UT_ERR_EXIT                 = 5,
    UT_ERR_SEND_DATA            = 4,
    UT_ERR_DATA_READY           = 3,
    UT_ERR_WAIT_DATA            = 2,
    UT_ERR_FINISH               = 1,

    UT_ERR_OK                   = 0,
    UT_ERR_NULL_POINTER         = -1,
    UT_ERR_WRONG_PARAM          = -2,
    UT_ERR_TIMEOUT              = -3,
    UT_ERR_COMM_DEV_OPEN_FAIL   = -4,
    UT_ERR_COMM_DEV_TX_FAIL     = -5,
    UT_ERR_COMM_DEV_RX_FAIL     = -6,
    UT_ERR_UNKNOWN              = -7,
    UT_ERR_CRC_FAIL             = -8,
    UT_ERR_OUT_BUF_LEN          = -9,
    UT_ERR_UT_INIT_FAIL         = -10,
    UT_ERR_UT_READ_FAIL         = -11,
    UT_ERR_UT_WRITE_FAIL        = -12,
    UT_ERR_UT_PROC_FAIL         = -13,
    UT_ERR_UT_ITEM_FULL         = -14,
    UT_ERR_BUF_TOO_SMALL        = -15,
    UT_ERR_HW_IP_INIT_FAIL      = -16,
    UT_ERR_FAIL                 = -17,
    UT_ERR_WRANG_PARAM          = -18,
    UT_ERR_DATA_NOT_MATCH       = -19,

} ut_err_t;

#define UT_UID_MASK            0xFFFFFF00
#define UT_UID_UART            0x75727400
#define UT_UID_SPI             0x73706900
#define UT_UID_I2C             0x69326300
#define UT_UID_TIM             0x74696D00
#define UT_UID_GPIO            0x67706900
#define UT_UID_MISC            0x6D736300

typedef enum ut_hw_type
{
    UT_HW_TYPE_NONE     = 0,
    UT_HW_TYPE_MISC,
    UT_HW_TYPE_UART,
    UT_HW_TYPE_SPI,
    UT_HW_TYPE_I2C,
    UT_HW_TYPE_TIMER,
    UT_HW_TYPE_GPIO,

} ut_hw_type_t;

typedef enum ut_tcase
{
    UT_TCASE_RESET      = 2,
    UT_TCASE_PING,
    UT_TCASE_TIMER_PERIOD,
    UT_TCASE_TIMER_PWM,
    UT_TCASE_TIMER_CAP,

} ut_tcase_t;

typedef enum ut_role
{
    UT_ROLE_MASTER  = 0,
    UT_ROLE_SLAVE,
} ut_role_t;

typedef enum ut_method
{
    UT_METHOD_INIT      = 0,
    UT_METHOD_SELF_ROUNTE,    /* Stand Along test */
    UT_METHOD_READ,
    UT_METHOD_WRITE,
    UT_METHOD_DEINIT

} ut_method_t;


#define PIN_IGNORE          0

/**
 *  uart pin
 */
typedef enum pin_uart
{
    PIN_UART_TX = UT_UID_UART | 0x1,
    PIN_UART_RX = UT_UID_UART | 0x2,
} pin_uart_t;


/**
 *  i2c pin
 */
typedef enum pin_i2c
{
    PIN_I2C_SCL = UT_UID_I2C | 0x1,
    PIN_I2C_SDA = UT_UID_I2C | 0x2,
} pin_i2c_t;

/**
 *  spi pin
 */
typedef enum pin_spi
{
    PIN_SPI_CS    = UT_UID_SPI | 0x1,
    PIN_SPI_CLK   = UT_UID_SPI | 0x2,
    PIN_SPI_MISO  = UT_UID_SPI | 0x3,
    PIN_SPI_MOSI  = UT_UID_SPI | 0x4,
} pin_spi_t;

/**
 *  timer pin
 */
typedef enum pin_tim
{
    PIN_TIM_PWM      = UT_UID_TIM | 0x1,
    PIN_TIM_PWM_NOUT = UT_UID_TIM | 0x2,
    PIN_TIM_CAPTURE  = UT_UID_TIM | 0x3,
    PIN_TIM_EXT      = UT_UID_TIM | 0x4,
    PIN_TIM_GATE     = UT_UID_TIM | 0x5,
    PIN_TIM_TOG      = UT_UID_TIM | 0x6,
    PIN_TIM_TOG_NOUT = UT_UID_TIM | 0x7,
    PIN_TIM_GPIO_OUT = UT_UID_TIM | 0x8,
} pin_tim_t;

/**
 *  gpio pin
 */
typedef enum pin_gpio
{
    PIN_GPIO_IO      = UT_UID_MISC | 0x1,
} pin_gpio_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define UT_FOURCC(a, b, c, d)       (((uint32_t)((d) & 0xFF))       | \
                                     ((uint32_t)((c) & 0xFF) << 8)  | \
                                     ((uint32_t)((b) & 0xFF) << 16) | \
                                     ((uint32_t)((a) & 0xFF) << 24))
//=============================================================================
//                  Structure Definition
//=============================================================================
#pragma anon_unions
typedef struct pin_attr
{
    uint32_t    pin_type;
    uint32_t    index;

    union {
        uint32_t    value;
        struct {
            uint32_t    port    : 8;
            uint32_t    pin     : 8;
            uint32_t    af_mode : 8;
            uint32_t    channel : 4;
        } attr;
    };

} pin_attr_t;

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
