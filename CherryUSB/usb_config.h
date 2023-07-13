/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CHERRYUSB_CONFIG_H
#define CHERRYUSB_CONFIG_H

//======================================================================================
//          USB common Configuration
//======================================================================================
#define CHERRYUSB_VERSION       0x000900

#define CONFIG_USB_PRINTF(...)  printf(__VA_ARGS__)

/**
 *  ToDo: implement memory pool
 */
#include <stdlib.h>
#define usb_malloc(size)        usb_osal_malloc(size)   //malloc(size)
#define usb_free(ptr)           usb_osal_free(ptr)      //free(ptr)

/* Enable print with color */
//#define CONFIG_USB_PRINTF_COLOR_ENABLE

/* data align size when use dma */
#ifndef CONFIG_USB_ALIGN_SIZE
    #define CONFIG_USB_ALIGN_SIZE   4
#endif

/* attribute data into no cache ram */
#define USB_NOCACHE_RAM_SECTION     __attribute__((section(".noncacheable")))

/**
 *  Dubug Configuration
 */
#ifndef CONFIG_USB_DBG_LEVEL
    #define CONFIG_USB_DBG_LEVEL    USB_DBG_LOG //USB_DBG_INFO
#endif

/* Setup packet log for debug */
#define CONFIG_USBDEV_SETUP_LOG_PRINT

/* Check if the input descriptor is correct */
// #define CONFIG_USBDEV_DESC_CHECK

/* Enable test mode */
// #define CONFIG_USBDEV_TEST_MODE



//======================================================================================
//          USB DEVICE Configuration
//======================================================================================

/* Ep0 max transfer buffer, specially for receiving data from ep0 out */
#ifndef CONFIG_USBDEV_REQUEST_BUFFER_LEN
    #define CONFIG_USBDEV_REQUEST_BUFFER_LEN    256
#endif

/**
 *  Configure classes of USB Device
 */
#define CONFIG_ENABLE_USBD_CDC          1
#define CONFIG_ENABLE_USBD_MSC          0
#define CONFIG_ENABLE_USBD_DFU          0
#define CONFIG_ENABLE_USBD_HID          0
#define CONFIG_ENABLE_USBD_AUDIO        0
#define CONFIG_ENABLE_USBD_VIDIO        0


/* cdc class */
#if defined(CONFIG_ENABLE_USBD_CDC) && (CONFIG_ENABLE_USBD_CDC)
    // #define CONFIG_USBDEV_CDC_ACM_UART
#endif  /* CONFIG_ENABLE_USBD_CDC */

/* msc class */
#if defined(CONFIG_ENABLE_USBD_MSC) &&  (CONFIG_ENABLE_USBD_MSC)
    #ifndef CONFIG_USBDEV_MSC_BLOCK_SIZE
        #define CONFIG_USBDEV_MSC_BLOCK_SIZE            512
    #endif

    #ifndef CONFIG_USBDEV_MSC_MANUFACTURER_STRING
        #define CONFIG_USBDEV_MSC_MANUFACTURER_STRING   ""
    #endif

    #ifndef CONFIG_USBDEV_MSC_PRODUCT_STRING
        #define CONFIG_USBDEV_MSC_PRODUCT_STRING        ""
    #endif

    #ifndef CONFIG_USBDEV_MSC_VERSION_STRING
        #define CONFIG_USBDEV_MSC_VERSION_STRING        "0.1"
    #endif

    // #define CONFIG_USBDEV_MSC_THREAD

    #ifdef CONFIG_USBDEV_MSC_THREAD
        #ifndef CONFIG_USBDEV_MSC_STACKSIZE
            #define CONFIG_USBDEV_MSC_STACKSIZE         2048
        #endif

        #ifndef CONFIG_USBDEV_MSC_PRIO
            #define CONFIG_USBDEV_MSC_PRIO              4
        #endif
    #endif
#endif  /* CONFIG_ENABLE_USBD_MSC */

/* dfu class */
#if defined(CONFIG_ENABLE_USBD_DFU) && (CONFIG_ENABLE_USBD_DFU)
    #define USBD_DFU_XFER_SIZE          1024
#endif  /* CONFIG_ENABLE_USBD_DFU */


/* audio class */
#if defined(CONFIG_ENABLE_USBD_AUDIO) && (CONFIG_ENABLE_USBD_AUDIO)

    #ifndef CONFIG_USBDEV_AUDIO_VERSION
        #define CONFIG_USBDEV_AUDIO_VERSION             0x0100
    #endif

    #ifndef CONFIG_USBDEV_AUDIO_MAX_CHANNEL
        #define CONFIG_USBDEV_AUDIO_MAX_CHANNEL         2
    #endif
#endif  /* CONFIG_ENABLE_USBD_AUDIO */

/* HID class */
#if defined(CONFIG_ENABLE_USBD_HID) && (CONFIG_ENABLE_USBD_HID)

#endif

//======================================================================================
//          USB Host Configuration
//======================================================================================

#define CONFIG_USBHOST_WITH_BARE_METAL

#define CONFIG_USBHOST_MAX_RHPORTS              1
#define CONFIG_USBHOST_MAX_EXTHUBS              1
#define CONFIG_USBHOST_MAX_EHPORTS              4
#define CONFIG_USBHOST_MAX_INTERFACES           6
#define CONFIG_USBHOST_MAX_INTF_ALTSETTINGS     1
#define CONFIG_USBHOST_MAX_ENDPOINTS            4

#define CONFIG_USBHOST_DEV_NAMELEN              16

#ifndef CONFIG_USBHOST_PSC_PRIO
    #define CONFIG_USBHOST_PSC_PRIO                 4
#endif

#ifndef CONFIG_USBHOST_PSC_STACKSIZE
    #define CONFIG_USBHOST_PSC_STACKSIZE            2048
#endif

#define CONFIG_USBHOST_GET_STRING_DESC

/* Ep0 max transfer buffer */
#define CONFIG_USBHOST_REQUEST_BUFFER_LEN       512

#ifndef CONFIG_USBHOST_CONTROL_TRANSFER_TIMEOUT
    #define CONFIG_USBHOST_CONTROL_TRANSFER_TIMEOUT     500
#endif

#ifndef CONFIG_USBHOST_MSC_TIMEOUT
    #define CONFIG_USBHOST_MSC_TIMEOUT                  5000
#endif


#define CONFIG_USBHOST_PIPE_NUM                 4


/**
 *  Configure classes of USB Host
 */
#define CONFIG_ENABLE_USBH_CDC          1
#define CONFIG_ENABLE_USBH_MSC          0
#define CONFIG_ENABLE_USBH_DFU          0
#define CONFIG_ENABLE_USBH_HID          0
#define CONFIG_ENABLE_USBH_AUDIO        0
#define CONFIG_ENABLE_USBH_VIDEO        0


#endif  /* CHERRYUSB_CONFIG_H */
