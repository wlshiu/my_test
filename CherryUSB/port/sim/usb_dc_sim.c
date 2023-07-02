/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file usb_dc_sim.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/06/30
 * @license
 * @description
 */


#include <windows.h>
#include "usbd_core.h"

#include "sys_sim.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#ifndef USB_NUM_BIDIR_ENDPOINTS
#define USB_NUM_BIDIR_ENDPOINTS         5
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/* Endpoint state */
struct usb_dc_ep_state {
    uint16_t ep_mps;    /* Endpoint max packet size */
    uint8_t ep_type;    /* Endpoint type */
    uint8_t ep_stalled; /* Endpoint stall flag */
    uint8_t *xfer_buf;
    uint32_t xfer_len;
    uint32_t actual_xfer_len;
};

/* Driver state */
struct xxx_udc {
    volatile uint8_t dev_addr;
    struct usb_dc_ep_state in_ep[USB_NUM_BIDIR_ENDPOINTS];  /*!< IN endpoint parameters*/
    struct usb_dc_ep_state out_ep[USB_NUM_BIDIR_ENDPOINTS]; /*!< OUT endpoint parameters */
} g_xxx_udc;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern DWORD   g_th_uhost_id;
extern DWORD   g_th_udev_id;

static uint8_t      g_udev_rxbuf[1024] = {0};
static uint8_t      g_udev_txbuf[1024] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================

__WEAK void usb_dc_low_level_init(void)
{
}

__WEAK void usb_dc_low_level_deinit(void)
{
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int usb_dc_init(void)
{
    memset(&g_xxx_udc, 0, sizeof(struct xxx_udc));

    usb_dc_low_level_init();
    return 0;
}

int usb_dc_deinit(void)
{
    return 0;
}

int usbd_set_address(const uint8_t addr)
{
    return 0;
}

int usbd_ep_open(const struct usbd_endpoint_cfg *ep_cfg)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep_cfg->ep_addr);

    if (USB_EP_DIR_IS_OUT(ep_cfg->ep_addr)) {
        g_xxx_udc.out_ep[ep_idx].ep_mps = ep_cfg->ep_mps;
        g_xxx_udc.out_ep[ep_idx].ep_type = ep_cfg->ep_type;
    } else {
        g_xxx_udc.in_ep[ep_idx].ep_mps = ep_cfg->ep_mps;
        g_xxx_udc.in_ep[ep_idx].ep_type = ep_cfg->ep_type;
    }
    return 0;
}

int usbd_ep_close(const uint8_t ep)
{
    return 0;
}

int usbd_ep_set_stall(const uint8_t ep)
{
    return 0;
}

int usbd_ep_clear_stall(const uint8_t ep)
{
    return 0;
}

int usbd_ep_is_stalled(const uint8_t ep, uint8_t *stalled)
{
    return 0;
}

int usbd_ep_start_write(const uint8_t ep, const uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint32_t tmp;

    if (!data && data_len) {
        return -1;
    }

    g_xxx_udc.in_ep[ep_idx].xfer_buf = (uint8_t *)data;
    g_xxx_udc.in_ep[ep_idx].xfer_len = data_len;
    g_xxx_udc.in_ep[ep_idx].actual_xfer_len = 0;

    return 0;
}

int usbd_ep_start_read(const uint8_t ep, uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);

    if (!data && data_len) {
        return -1;
    }

    g_xxx_udc.out_ep[ep_idx].xfer_buf = (uint8_t *)data;
    g_xxx_udc.out_ep[ep_idx].xfer_len = data_len;
    g_xxx_udc.out_ep[ep_idx].actual_xfer_len = 0;

    return 0;
}

void USBD_IRQHandler(void)
{
    static sys_msg_info_t   msg_udev;
    int                     rxbuf_size = sizeof(g_udev_rxbuf);

    sys_wait_msg((uint8_t*)g_udev_rxbuf, &rxbuf_size);

        #if 1
        printf("===== uDev RX ====\n");
        if( rxbuf_size )
        {
            for(int i = 0; i < rxbuf_size; i++)
            {
                printf("%c", g_udev_rxbuf[i]);
            }

            printf("\n");
        }
        #endif

    snprintf(g_udev_txbuf, sizeof(g_udev_txbuf), "D -> H");

    msg_udev.pMsg_buf = g_udev_txbuf;
    msg_udev.msg_len  = strlen(g_udev_txbuf) + 1;
    sys_send_msg(g_th_uhost_id, &msg_udev);

    return;
}
