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
    UT_TCASE_PING       = 3,

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

/**
 *  uart pin
 */
typedef enum uart_pin
{
    UART_PIN_TX = UT_UID_UART | 0x1,
    UART_PIN_RX = UT_UID_UART | 0x2,
} uart_pin_t;


/**
 *  i2c pin
 */
typedef enum i2c_pin
{
    I2C_PIN_SCL = UT_UID_I2C | 0x1,
    I2C_PIN_SDA = UT_UID_I2C | 0x2,
} i2c_pin_t;

/**
 *  spi pin
 */
typedef enum spi_pin
{
    SPI_PIN_CS    = UT_UID_SPI | 0x1,
    SPI_PIN_CLK   = UT_UID_SPI | 0x2,
    SPI_PIN_MISO  = UT_UID_SPI | 0x3,
    SPI_PIN_MOSI  = UT_UID_SPI | 0x4,
} spi_pin_t;

/**
 *  timer pin
 */
typedef enum tim_pin
{
    TIM_PIN_PWM      = UT_UID_TIM | 0x1,
    TIM_PIN_CAPTURE  = UT_UID_TIM | 0x2,
} tim_pin_t;

/**
 *  gpio pin
 */
typedef enum gpio_pin
{
    GPIO_IO_PIN      = UT_UID_MISC | 0x1,
} gpio_pin_t;
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
