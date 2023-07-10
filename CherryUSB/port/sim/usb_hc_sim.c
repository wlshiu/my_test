/**
 * Copyright (c) 2023 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file usb_hc_sim.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2023/06/30
 * @license
 * @description
 */

#include <stdint.h>
#include <stdbool.h>

#include "usbh_core.h"
#include "usbh_hub.h"

#include "usb_musb_reg.h"

#include "sys_sim.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

#define MUSB_FADDR_OFFSET 0x00
#define MUSB_POWER_OFFSET 0x01
#define MUSB_TXIS_OFFSET  0x02
#define MUSB_RXIS_OFFSET  0x04
#define MUSB_TXIE_OFFSET  0x06
#define MUSB_RXIE_OFFSET  0x08
#define MUSB_IS_OFFSET    0x0A
#define MUSB_IE_OFFSET    0x0B

#define MUSB_EPIDX_OFFSET 0x0E

#define MUSB_IND_TXMAP_OFFSET      0x10
#define MUSB_IND_TXCSRL_OFFSET     0x12
#define MUSB_IND_TXCSRH_OFFSET     0x13
#define MUSB_IND_RXMAP_OFFSET      0x14
#define MUSB_IND_RXCSRL_OFFSET     0x16
#define MUSB_IND_RXCSRH_OFFSET     0x17
#define MUSB_IND_RXCOUNT_OFFSET    0x18
#define MUSB_IND_TXTYPE_OFFSET     0x1A
#define MUSB_IND_TXINTERVAL_OFFSET 0x1B
#define MUSB_IND_RXTYPE_OFFSET     0x1C
#define MUSB_IND_RXINTERVAL_OFFSET 0x1D

#define MUSB_FIFO_OFFSET 0x20

#define MUSB_DEVCTL_OFFSET 0x60

#define MUSB_TXFIFOSZ_OFFSET  0x62
#define MUSB_RXFIFOSZ_OFFSET  0x63
#define MUSB_TXFIFOADD_OFFSET 0x64
#define MUSB_RXFIFOADD_OFFSET 0x66

#define MUSB_TXFUNCADDR0_OFFSET 0x80
#define MUSB_TXHUBADDR0_OFFSET  0x82
#define MUSB_TXHUBPORT0_OFFSET  0x83
#define MUSB_TXFUNCADDRx_OFFSET 0x88
#define MUSB_TXHUBADDRx_OFFSET  0x8A
#define MUSB_TXHUBPORTx_OFFSET  0x8B
#define MUSB_RXFUNCADDRx_OFFSET 0x8C
#define MUSB_RXHUBADDRx_OFFSET  0x8E
#define MUSB_RXHUBPORTx_OFFSET  0x8F

#ifndef CONIFG_USB_MUSB_PIPE_NUM
#define CONIFG_USB_MUSB_PIPE_NUM 2
#endif

#define USB_BASE    ((uint32_t)(&g_USBH))
#define USB         ((USB0_Type*)USB_BASE)

typedef enum
{
    USB_EP0_STATE_SETUP = 0x0, /**< SETUP DATA */
    USB_EP0_STATE_IN_DATA,     /**< IN DATA */
    USB_EP0_STATE_IN_STATUS,   /**< IN status*/
    USB_EP0_STATE_OUT_DATA,    /**< OUT DATA */
    USB_EP0_STATE_OUT_STATUS,  /**< OUT status */
} ep0_state_t;

#define musb_write_packet       musb_hc_write_packet
#define musb_read_packet        musb_hc_read_packet
//=============================================================================
//                  Macro Definition
//=============================================================================
#define HWREG(x)    (*((volatile uint32_t *)(x)))
#define HWREGH(x)   (*((volatile uint16_t *)(x)))
#define HWREGB(x)   (*((volatile uint8_t *)(x)))

#define USB_TXADDR_BASE(ep_idx)    (USB_BASE + MUSB_TXFUNCADDR0_OFFSET + 0x8 * ep_idx)
#define USB_TXHUBADDR_BASE(ep_idx) (USB_BASE + MUSB_TXFUNCADDR0_OFFSET + 0x8 * ep_idx + 2)
#define USB_TXHUBPORT_BASE(ep_idx) (USB_BASE + MUSB_TXFUNCADDR0_OFFSET + 0x8 * ep_idx + 3)
#define USB_RXADDR_BASE(ep_idx)    (USB_BASE + MUSB_TXFUNCADDR0_OFFSET + 0x8 * ep_idx + 4)
#define USB_RXHUBADDR_BASE(ep_idx) (USB_BASE + MUSB_TXFUNCADDR0_OFFSET + 0x8 * ep_idx + 6)
#define USB_RXHUBPORT_BASE(ep_idx) (USB_BASE + MUSB_TXFUNCADDR0_OFFSET + 0x8 * ep_idx + 7)

#define USB_FIFO_BASE(ep_idx)   (USB_BASE + MUSB_FIFO_OFFSET + 0x4 * (ep_idx))
//=============================================================================
//                  Structure Definition
//=============================================================================
struct musb_pipe
{
    uint8_t dev_addr;
    uint8_t ep_addr;
    uint8_t ep_type;
    uint8_t ep_interval;
    uint8_t speed;
    uint16_t ep_mps;
    bool inuse;
    uint32_t xfrd;
    volatile bool waiter;
    usb_osal_sem_t waitsem;
    struct usbh_hubport *hport;
    struct usbh_urb *urb;
};

struct musb_hcd
{
    volatile bool port_csc;
    volatile bool port_pec;
    volatile bool port_pe;
    struct musb_pipe pipe_pool[CONFIG_USBHOST_PIPE_NUM][2]; /* Support Bidirectional ep */
} g_musb_hcd;

//=============================================================================
//                  Global Data Definition
//=============================================================================
USB0_Type           g_USBH = {0};

static volatile uint8_t usb_ep0_state = USB_EP0_STATE_SETUP;

sys_vmsgq_t     g_hc_vmsgq = {0};

uint32_t        g_uhost_tx_buf[1024 >> 2] = {0};
uint32_t        g_uhost_rx_buf[1024 >> 2] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
__WEAK void usb_hc_low_level_init(void)
{
    g_USBH.TXIS |= USB_TXIE_EP0;
    g_USBH.RXCOUNT = 8;
    g_USBH.IS = USB_IS_CONN;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================

/* get current active ep */
static uint8_t musb_get_active_ep(void)
{
    return HWREGB(USB_BASE + MUSB_EPIDX_OFFSET);
}

/* set the active ep */
static void musb_set_active_ep(uint8_t ep_index)
{
    HWREGB(USB_BASE + MUSB_EPIDX_OFFSET) = ep_index;
}

static void musb_fifo_flush(uint8_t ep)
{
    uint8_t ep_idx = ep & 0x7f;
    if (ep_idx == 0)
    {
        if ((HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) & (USB_CSRL0_RXRDY | USB_CSRL0_TXRDY)) != 0)
            HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) |= USB_CSRH0_FLUSH;
    }
    else
    {
        if (ep & 0x80)
        {
            if (HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) & USB_TXCSRL1_TXRDY)
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) |= USB_TXCSRL1_FLUSH;
        }
        else
        {
            if (HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) & USB_RXCSRL1_RXRDY)
                HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) |= USB_RXCSRL1_FLUSH;
        }
    }
}

static void musb_write_packet(uint8_t ep_idx, uint8_t *buffer, uint16_t len)
{
    uint32_t *buf32;
    uint8_t *buf8;
    uint32_t count32;
    uint32_t count8;
    int i;

    if ((uint32_t)buffer & 0x03)
    {
        buf8 = buffer;
        for (i = 0; i < len; i++)
        {
            printf("[%s:%d] %p, %p\n", __func__, __LINE__, USB_FIFO_BASE(ep_idx), &g_USBH.FIFO0);
            g_USBH.FIFO0.FIFO0_BYTE = *buf8++;
//            HWREGB(USB_FIFO_BASE(ep_idx)) = *buf8++;
        }

        printf("\n");
    }
    else
    {
        count32 = len >> 2;
        count8 = len & 0x03;

        buf32 = (uint32_t *)buffer;

        while (count32--)
        {
            HWREG(USB_FIFO_BASE(ep_idx)) = *buf32++;
        }

        buf8 = (uint8_t *)buf32;

        while (count8--)
        {
            HWREGB(USB_FIFO_BASE(ep_idx)) = *buf8++;
        }
    }

    #if defined(CONFIG_USB_SIM)
    static sys_msg_info_t   msg_uhost;
    int                     txbuf_size = sizeof(g_uhost_tx_buf);
    int                     rxbuf_size = sizeof(g_uhost_rx_buf);

    g_USBD.RXCOUNT = len;
    g_USBD.TXCSR = USB_CSRL0_RXRDY;
    g_USBD.TXIS = USB_TXIE_EP0 | (0x1 << ep_idx);
    g_USBD.RXIS = 0x1 << ep_idx;
    #if 0
    memcpy(g_uhost_tx_buf, buffer, len);

    msg_uhost.pMsg_buf = g_uhost_tx_buf;
    msg_uhost.msg_len  = len;
    sys_send_msg(g_th_udev_id, &msg_uhost);
    #elif 1
    sys_packet_hdr_t        *pHdr = (sys_packet_hdr_t*)&g_udev_rxbuf;

    sys_mutex_lock((void*)g_hMutexDC);
    pHdr->len = (len < sizeof(g_udev_rxbuf)) ? len : sizeof(g_udev_rxbuf);
    memcpy(pHdr->raw, buffer, pHdr->len);

    sys_dump_mem(buffer, len, __func__, __LINE__);

    sys_mutex_unlock((void*)g_hMutexDC);
    #else
    sys_vmsg_node_t     *pNode = 0;

    pNode = malloc(sizeof(sys_vmsg_node_t));
    memset(pNode, 0x0, sizeof(sys_vmsg_node_t));

    memcpy(&pNode->usb_regs, &g_USBH, sizeof(pNode->usb_regs));
    memcpy(&pNode->raw, buffer, len);

    sys_vmsgq_send(&g_hc_vmsgq, pNode);

    #endif // 0

//    trace("\n");
    #endif  /* CONFIG_USB_SIM */

    return;
}

static void musb_read_packet(uint8_t ep_idx, uint8_t *buffer, uint16_t len)
{
    uint32_t *buf32;
    uint8_t *buf8;
    uint32_t count32;
    uint32_t count8;
    int i;

    if ((uint32_t)buffer & 0x03)
    {
        buf8 = buffer;
        for (i = 0; i < len; i++)
        {
            *buf8++ = HWREGB(USB_FIFO_BASE(ep_idx));
        }
    }
    else
    {
        count32 = len >> 2;
        count8 = len & 0x03;

        buf32 = (uint32_t *)buffer;

        while (count32--)
        {
            *buf32++ = HWREG(USB_FIFO_BASE(ep_idx));
        }

        buf8 = (uint8_t *)buf32;

        while (count8--)
        {
            *buf8++ = HWREGB(USB_FIFO_BASE(ep_idx));
        }
    }

    #if defined(CONFIG_USB_SIM)
    static sys_msg_info_t   msg_uhost;
    int                     txbuf_size = sizeof(g_uhost_tx_buf);
    int                     rxbuf_size = sizeof(g_uhost_rx_buf);

    #if 0
    sys_wait_msg((uint8_t*)g_uhost_rx_buf, &rxbuf_size);
    memcpy(buffer, g_uhost_rx_buf, (len < rxbuf_size) ? len : rxbuf_size);
    #elif 1
    sys_packet_hdr_t        *pHdr = (sys_packet_hdr_t*)&g_uhost_rx_buf;

    sys_mutex_lock((void*)g_hMutexHC);
    if( len == 0 )
        trace(" len = %d\n", len);

    memcpy(buffer, pHdr->raw, (len < pHdr->len) ? len : pHdr->len);
//    memset(g_uhost_rx_buf, 0x0, sizeof(g_uhost_rx_buf));

    sys_dump_mem(buffer, len, __func__, __LINE__);
    sys_mutex_unlock((void*)g_hMutexHC);
    #else
    memcpy(buffer, g_uhost_rx_buf, len);
    #endif

//    trace("\n");
    #endif  /* CONFIG_USB_SIM */

    return;
}

void musb_control_pipe_init(struct musb_pipe *pipe, struct usb_setup_packet *setup, uint8_t *buffer, uint32_t buflen)
{
    uint8_t old_ep_index;

    old_ep_index = musb_get_active_ep();
    musb_set_active_ep(0);

    HWREGB(USB_TXADDR_BASE(0)) = pipe->dev_addr;
    HWREGB(USB_BASE + MUSB_IND_TXTYPE_OFFSET) = pipe->speed;
    HWREGB(USB_TXHUBADDR_BASE(0)) = 0;
    HWREGB(USB_TXHUBPORT_BASE(0)) = 0;

    musb_write_packet(0, (uint8_t *)setup, 8);
    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_TXRDY | USB_CSRL0_SETUP;
    musb_set_active_ep(old_ep_index);
}

void musb_bulk_pipe_init(struct musb_pipe *pipe, uint8_t *buffer, uint32_t buflen)
{
    uint8_t ep_idx;
    uint8_t old_ep_index;

    ep_idx = pipe->ep_addr & 0x7f;
    old_ep_index = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    if (pipe->ep_addr & 0x80)
    {
        HWREGB(USB_RXADDR_BASE(ep_idx)) = pipe->dev_addr;
        HWREGB(USB_BASE + MUSB_IND_RXTYPE_OFFSET) = ep_idx | pipe->speed | USB_TXTYPE1_PROTO_BULK;
        HWREGB(USB_BASE + MUSB_IND_RXINTERVAL_OFFSET) = 0;
        HWREGB(USB_RXHUBADDR_BASE(ep_idx)) = 0;
        HWREGB(USB_RXHUBPORT_BASE(ep_idx)) = 0;
        HWREGB(USB_BASE + MUSB_IND_TXCSRH_OFFSET) &= ~USB_TXCSRH1_MODE;
        HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) = USB_RXCSRL1_REQPKT;
    }
    else
    {
        HWREGB(USB_TXADDR_BASE(ep_idx)) = pipe->dev_addr;
        HWREGB(USB_BASE + MUSB_IND_TXTYPE_OFFSET) = ep_idx | pipe->speed | USB_TXTYPE1_PROTO_BULK;
        HWREGB(USB_BASE + MUSB_IND_TXINTERVAL_OFFSET) = 0;
        HWREGB(USB_TXHUBADDR_BASE(ep_idx)) = 0;
        HWREGB(USB_TXHUBPORT_BASE(ep_idx)) = 0;

        if (buflen > pipe->ep_mps)
        {
            buflen = pipe->ep_mps;
        }

        musb_write_packet(ep_idx, buffer, buflen);
        HWREGB(USB_BASE + MUSB_IND_TXCSRH_OFFSET) &= ~USB_TXCSRH1_MODE;
        HWREGB(USB_BASE + MUSB_IND_TXCSRH_OFFSET) |= USB_TXCSRH1_MODE;
        HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_TXCSRL1_TXRDY;
    }
    musb_set_active_ep(old_ep_index);
}

void musb_intr_pipe_init(struct musb_pipe *pipe, uint8_t *buffer, uint32_t buflen)
{
    uint8_t ep_idx;
    uint8_t old_ep_index;

    ep_idx = pipe->ep_addr & 0x7f;
    old_ep_index = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    if (pipe->ep_addr & 0x80)
    {
        HWREGB(USB_RXADDR_BASE(ep_idx)) = pipe->dev_addr;
        HWREGB(USB_BASE + MUSB_IND_RXTYPE_OFFSET) = ep_idx | pipe->speed | USB_TXTYPE1_PROTO_INT;
        HWREGB(USB_BASE + MUSB_IND_RXINTERVAL_OFFSET) = pipe->ep_interval;
        HWREGB(USB_RXHUBADDR_BASE(ep_idx)) = 0;
        HWREGB(USB_RXHUBPORT_BASE(ep_idx)) = 0;
        HWREGB(USB_BASE + MUSB_IND_TXCSRH_OFFSET) &= ~USB_TXCSRH1_MODE;
        HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) = USB_RXCSRL1_REQPKT;
    }
    else
    {
        HWREGB(USB_TXADDR_BASE(ep_idx)) = pipe->dev_addr;
        HWREGB(USB_BASE + MUSB_IND_TXTYPE_OFFSET) = ep_idx | pipe->speed | USB_TXTYPE1_PROTO_INT;
        HWREGB(USB_BASE + MUSB_IND_TXINTERVAL_OFFSET) = pipe->ep_interval;
        HWREGB(USB_TXHUBADDR_BASE(ep_idx)) = 0;
        HWREGB(USB_TXHUBPORT_BASE(ep_idx)) = 0;

        if (buflen > pipe->ep_mps)
        {
            buflen = pipe->ep_mps;
        }

        musb_write_packet(ep_idx, buffer, buflen);
        HWREGB(USB_BASE + MUSB_IND_TXCSRH_OFFSET) &= ~USB_TXCSRH1_MODE;
        HWREGB(USB_BASE + MUSB_IND_TXCSRH_OFFSET) |= USB_TXCSRH1_MODE;
        HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_TXCSRL1_TXRDY;
    }
    musb_set_active_ep(old_ep_index);
}

static int usbh_reset_port(const uint8_t port)
{
    g_musb_hcd.port_pe = 0;
    HWREGB(USB_BASE + MUSB_POWER_OFFSET) |= USB_POWER_RESET;
    usb_osal_msleep(20);
    HWREGB(USB_BASE + MUSB_POWER_OFFSET) &= ~(USB_POWER_RESET);
    usb_osal_msleep(20);
    g_musb_hcd.port_pe = 1;
    return 0;
}

static uint8_t usbh_get_port_speed(const uint8_t port)
{
    uint8_t speed = USB_SPEED_UNKNOWN;

    if (HWREGB(USB_BASE + MUSB_POWER_OFFSET) & USB_POWER_HSMODE)
        speed = USB_SPEED_HIGH;
    else if (HWREGB(USB_BASE + MUSB_DEVCTL_OFFSET) & USB_DEVCTL_FSDEV)
        speed = USB_SPEED_FULL;
    else if (HWREGB(USB_BASE + MUSB_DEVCTL_OFFSET) & USB_DEVCTL_LSDEV)
        speed = USB_SPEED_LOW;

    return speed;
}


int usb_hc_init(void)
{
    uint8_t regval;
    uint32_t fifo_offset = 0;

    memset(&g_musb_hcd, 0, sizeof(struct musb_hcd));

    for (uint8_t i = 0; i < CONFIG_USBHOST_PIPE_NUM; i++)
    {
        g_musb_hcd.pipe_pool[i][0].waitsem = usb_osal_sem_create(0);
        g_musb_hcd.pipe_pool[i][1].waitsem = usb_osal_sem_create(0);
    }

    usb_hc_low_level_init();

    musb_set_active_ep(0);
    HWREGB(USB_BASE + MUSB_IND_TXINTERVAL_OFFSET) = 0;
    HWREGB(USB_BASE + MUSB_TXFIFOSZ_OFFSET) = USB_TXFIFOSZ_SIZE_64;
    HWREGH(USB_BASE + MUSB_TXFIFOADD_OFFSET) = 0;
    HWREGB(USB_BASE + MUSB_RXFIFOSZ_OFFSET) = USB_TXFIFOSZ_SIZE_64;
    HWREGH(USB_BASE + MUSB_RXFIFOADD_OFFSET) = 0;
    fifo_offset += 64;

    for (uint8_t i = 1; i < CONIFG_USB_MUSB_PIPE_NUM; i++)
    {
        musb_set_active_ep(i);
        HWREGB(USB_BASE + MUSB_TXFIFOSZ_OFFSET) = USB_TXFIFOSZ_SIZE_512;
        HWREGH(USB_BASE + MUSB_TXFIFOADD_OFFSET) = fifo_offset;
        HWREGB(USB_BASE + MUSB_RXFIFOSZ_OFFSET) = USB_TXFIFOSZ_SIZE_512;
        HWREGH(USB_BASE + MUSB_RXFIFOADD_OFFSET) = fifo_offset;
        fifo_offset += 512;
    }

    /* Enable USB interrupts */
    regval = USB_IE_RESET | USB_IE_CONN | USB_IE_DISCON |
             USB_IE_RESUME | USB_IE_SUSPND |
             USB_IE_BABBLE | USB_IE_SESREQ | USB_IE_VBUSERR;

    HWREGB(USB_BASE + MUSB_IE_OFFSET) = regval;
    HWREGH(USB_BASE + MUSB_TXIE_OFFSET) = USB_TXIE_EP0;
    HWREGH(USB_BASE + MUSB_RXIE_OFFSET) = 0;

    HWREGB(USB_BASE + MUSB_POWER_OFFSET) |= USB_POWER_HSENAB;

    HWREGB(USB_BASE + MUSB_DEVCTL_OFFSET) |= USB_DEVCTL_SESSION;

#ifdef CONFIG_USB_MUSB_SUNXI
    musb_set_active_ep(0);
    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_TXRDY;
#endif

#if defined(CONFIG_USB_SIM)

    #if 1
    usb_hc_low_level_init();
    #else
    {
        g_USBH.IS   = USB_IS_CONN;
        g_USBH.TXIS = USB_TXIE_EP0;

        sys_vmsg_node_t     *pNode = 0;

        pNode = malloc(sizeof(sys_vmsg_node_t));
        memset(pNode, 0x0, sizeof(sys_vmsg_node_t));

        memcpy(&pNode->usb_regs, &g_USBH, sizeof(pNode->usb_regs));

        sys_vmsgq_send(&g_dc_vmsgq, pNode);
    }
    #endif
#endif  /* CONFIG_USB_SIM */
    return 0;
}

int usbh_roothub_control(struct usb_setup_packet *setup, uint8_t *buf)
{
    uint8_t nports;
    uint8_t port;
    uint32_t status;

    nports = CONFIG_USBHOST_MAX_RHPORTS;
    port = setup->wIndex;
    if (setup->bmRequestType & USB_REQUEST_RECIPIENT_DEVICE)
    {
        switch (setup->bRequest)
        {
            case HUB_REQUEST_CLEAR_FEATURE:
                switch (setup->wValue)
                {
                    case HUB_FEATURE_HUB_C_LOCALPOWER:
                        break;
                    case HUB_FEATURE_HUB_C_OVERCURRENT:
                        break;
                    default:
                        return -EPIPE;
                }
                break;
            case HUB_REQUEST_SET_FEATURE:
                switch (setup->wValue)
                {
                    case HUB_FEATURE_HUB_C_LOCALPOWER:
                        break;
                    case HUB_FEATURE_HUB_C_OVERCURRENT:
                        break;
                    default:
                        return -EPIPE;
                }
                break;
            case HUB_REQUEST_GET_DESCRIPTOR:
                break;
            case HUB_REQUEST_GET_STATUS:
                memset(buf, 0, 4);
                break;
            default:
                break;
        }
    }
    else if (setup->bmRequestType & USB_REQUEST_RECIPIENT_OTHER)
    {
        switch (setup->bRequest)
        {
            case HUB_REQUEST_CLEAR_FEATURE:
                if (!port || port > nports)
                {
                    return -EPIPE;
                }

                switch (setup->wValue)
                {
                    case HUB_PORT_FEATURE_ENABLE:
                        break;
                    case HUB_PORT_FEATURE_SUSPEND:
                    case HUB_PORT_FEATURE_C_SUSPEND:
                        break;
                    case HUB_PORT_FEATURE_POWER:
                        break;
                    case HUB_PORT_FEATURE_C_CONNECTION:
                        g_musb_hcd.port_csc = 0;
                        break;
                    case HUB_PORT_FEATURE_C_ENABLE:
                        g_musb_hcd.port_pec = 0;
                        break;
                    case HUB_PORT_FEATURE_C_OVER_CURREN:
                        break;
                    case HUB_PORT_FEATURE_C_RESET:
                        break;
                    default:
                        return -EPIPE;
                }
                break;
            case HUB_REQUEST_SET_FEATURE:
                if (!port || port > nports)
                {
                    return -EPIPE;
                }

                switch (setup->wValue)
                {
                    case HUB_PORT_FEATURE_SUSPEND:
                        break;
                    case HUB_PORT_FEATURE_POWER:
                        break;
                    case HUB_PORT_FEATURE_RESET:
                        usbh_reset_port(port);
                        break;

                    default:
                        return -EPIPE;
                }
                break;
            case HUB_REQUEST_GET_STATUS:
                if (!port || port > nports)
                {
                    return -EPIPE;
                }

                status = 0;
                if (g_musb_hcd.port_csc)
                {
                    status |= (1 << HUB_PORT_FEATURE_C_CONNECTION);
                }
                if (g_musb_hcd.port_pec)
                {
                    status |= (1 << HUB_PORT_FEATURE_C_ENABLE);
                }

                if (g_musb_hcd.port_pe)
                {
                    status |= (1 << HUB_PORT_FEATURE_CONNECTION);
                    status |= (1 << HUB_PORT_FEATURE_ENABLE);
                    if (usbh_get_port_speed(port) == USB_SPEED_LOW)
                    {
                        status |= (1 << HUB_PORT_FEATURE_LOWSPEED);
                    }
                    else if (usbh_get_port_speed(port) == USB_SPEED_HIGH)
                    {
                        status |= (1 << HUB_PORT_FEATURE_HIGHSPEED);
                    }
                }

                memcpy(buf, &status, 4);
                break;
            default:
                break;
        }
    }
    return 0;
}

int usbh_ep_pipe_reconfigure(usbh_pipe_t pipe, uint8_t dev_addr, uint8_t ep_mps, uint8_t mult)
{
    struct musb_pipe *ppipe = (struct musb_pipe *)pipe;

    ppipe->dev_addr = dev_addr;
    ppipe->ep_mps = ep_mps;

    return 0;
}

int usbh_pipe_alloc(usbh_pipe_t *pipe, const struct usbh_endpoint_cfg *ep_cfg)
{
    struct musb_pipe *ppipe;
    uint8_t old_ep_index;
    uint8_t ep_idx;
    usb_osal_sem_t waitsem;

    ep_idx = ep_cfg->ep_addr & 0x7f;

    if (ep_idx > CONIFG_USB_MUSB_PIPE_NUM)
    {
        return -ENOMEM;
    }

    old_ep_index = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    if (ep_cfg->ep_addr & 0x80)
    {
        ppipe = &g_musb_hcd.pipe_pool[ep_idx][1];
    }
    else
    {
        ppipe = &g_musb_hcd.pipe_pool[ep_idx][0];
    }

    /* store variables */
    waitsem = ppipe->waitsem;

    memset(ppipe, 0, sizeof(struct musb_pipe));

    ppipe->ep_addr = ep_cfg->ep_addr;
    ppipe->ep_type = ep_cfg->ep_type;
    ppipe->ep_mps = ep_cfg->ep_mps;
    ppipe->ep_interval = ep_cfg->ep_interval;
    ppipe->speed = ep_cfg->hport->speed;
    ppipe->dev_addr = ep_cfg->hport->dev_addr;
    ppipe->hport = ep_cfg->hport;

    if (ep_cfg->ep_type == USB_ENDPOINT_TYPE_CONTROL)
    {
        if (ppipe->speed == USB_SPEED_HIGH)
        {
            ppipe->speed = USB_TYPE0_SPEED_HIGH;
        }
        else if (ppipe->speed == USB_SPEED_FULL)
        {
            ppipe->speed = USB_TYPE0_SPEED_FULL;
        }
        else if (ppipe->speed == USB_SPEED_LOW)
        {
            ppipe->speed = USB_TYPE0_SPEED_LOW;
        }
    }
    else
    {
        if (ppipe->speed == USB_SPEED_HIGH)
        {
            ppipe->speed = USB_TXTYPE1_SPEED_HIGH;
        }
        else if (ppipe->speed == USB_SPEED_FULL)
        {
            ppipe->speed = USB_TXTYPE1_SPEED_FULL;
        }
        else if (ppipe->speed == USB_SPEED_LOW)
        {
            ppipe->speed = USB_TXTYPE1_SPEED_LOW;
        }

        if (ppipe->ep_addr & 0x80)
        {
            HWREGH(USB_BASE + MUSB_RXIE_OFFSET) |= (1 << ep_idx);
        }
        else
        {
            HWREGH(USB_BASE + MUSB_TXIE_OFFSET) |= (1 << ep_idx);
        }
    }
    /* restore variable */
    ppipe->inuse = true;
    ppipe->waitsem = waitsem;

    musb_set_active_ep(old_ep_index);
    *pipe = (usbh_pipe_t)ppipe;
    return 0;
}

int usbh_pipe_free(usbh_pipe_t pipe)
{
    return 0;
}

int usbh_submit_urb(struct usbh_urb *urb)
{
    struct musb_pipe *pipe;
    size_t flags;
    int ret = 0;

    if (!urb)
    {
        return -EINVAL;
    }

    pipe = urb->pipe;

    if (!pipe)
    {
        return -EINVAL;
    }

    if (!pipe->hport->connected)
    {
        return -ENODEV;
    }

    if (pipe->urb)
    {
        return -EBUSY;
    }

    flags = usb_osal_enter_critical_section();

    pipe->waiter = false;
    pipe->xfrd = 0;
    pipe->urb = urb;
    urb->errorcode = -EBUSY;
    urb->actual_length = 0;

    if (urb->timeout > 0)
    {
        pipe->waiter = true;
    }
    usb_osal_leave_critical_section(flags);

    switch (pipe->ep_type)
    {
        case USB_ENDPOINT_TYPE_CONTROL:
            usb_ep0_state = USB_EP0_STATE_SETUP;
            musb_control_pipe_init(pipe, urb->setup, urb->transfer_buffer, urb->transfer_buffer_length);
            break;
        case USB_ENDPOINT_TYPE_BULK:
            musb_bulk_pipe_init(pipe, urb->transfer_buffer, urb->transfer_buffer_length);
            break;
        case USB_ENDPOINT_TYPE_INTERRUPT:
            musb_intr_pipe_init(pipe, urb->transfer_buffer, urb->transfer_buffer_length);
            break;
        case USB_ENDPOINT_TYPE_ISOCHRONOUS:
            break;
        default:
            break;
    }
    if (urb->timeout > 0)
    {
        /* wait until timeout or sem give */
        ret = usb_osal_sem_take(pipe->waitsem, urb->timeout);
        if (ret < 0)
        {
            goto errout_timeout;
        }

        ret = urb->errorcode;
    }
    return ret;
errout_timeout:
    pipe->waiter = false;
    usbh_kill_urb(urb);
    return ret;
}

int usbh_kill_urb(struct usbh_urb *urb)
{
    return 0;
}

static inline void musb_pipe_waitup(struct musb_pipe *pipe)
{
    struct usbh_urb *urb;

    urb = pipe->urb;
    pipe->urb = NULL;

    if (pipe->waiter)
    {
        pipe->waiter = false;
        usb_osal_sem_give(pipe->waitsem);
    }

    if (urb->complete)
    {
        if (urb->errorcode < 0)
        {
            urb->complete(urb->arg, urb->errorcode);
        }
        else
        {
            urb->complete(urb->arg, urb->actual_length);
        }
    }
}

void hc_handle_ep0(void)
{
    uint8_t ep0_status;
    struct musb_pipe *pipe;
    struct usbh_urb *urb;
    uint32_t size;

    pipe = (struct musb_pipe *)&g_musb_hcd.pipe_pool[0][0];
    urb = pipe->urb;
    if (urb == NULL)
    {
        return;
    }

    musb_set_active_ep(0);
    ep0_status = HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET);
    if (ep0_status & USB_CSRL0_STALLED)
    {
        HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_CSRL0_STALLED;
        usb_ep0_state = USB_EP0_STATE_SETUP;
        urb->errorcode = -EPERM;
        musb_pipe_waitup(pipe);
        return;
    }
    if (ep0_status & USB_CSRL0_ERROR)
    {
        HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_CSRL0_ERROR;
        musb_fifo_flush(0);
        usb_ep0_state = USB_EP0_STATE_SETUP;
        urb->errorcode = -EIO;
        musb_pipe_waitup(pipe);
        return;
    }
    if (ep0_status & USB_CSRL0_STALL)
    {
        HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_CSRL0_STALL;
        usb_ep0_state = USB_EP0_STATE_SETUP;
        urb->errorcode = -EPERM;
        musb_pipe_waitup(pipe);
        return;
    }

    switch (usb_ep0_state)
    {
        case USB_EP0_STATE_SETUP:
            urb->actual_length += 8;
            if (urb->transfer_buffer_length)
            {
                if (urb->setup->bmRequestType & 0x80)
                {
                    usb_ep0_state = USB_EP0_STATE_IN_DATA;

                    #if defined(CONFIG_USB_SIM)
//                    g_USBH.CSR = 0;
//                    g_USBD.CSR |= USB_CSRL0_RXRDY;
                    g_USBD.RXCOUNT = 0;
                    #if 0

                    /* usb_dc send ACK packet */
                    sys_vmsg_node_t     *pNode = 0;

                    pNode = malloc(sizeof(sys_vmsg_node_t));
                    memset(pNode, 0x0, sizeof(sys_vmsg_node_t));

                    memcpy(&pNode->usb_regs, &g_USBH, sizeof(pNode->usb_regs));

                    sys_vmsgq_send(&g_dc_vmsgq, pNode);
                    #endif

                    #else

                    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_REQPKT;

                    #endif  /* CONFIG_USB_SIM */
                }
                else
                {
                    usb_ep0_state = USB_EP0_STATE_OUT_DATA;
                    size = urb->transfer_buffer_length;
                    if (size > pipe->ep_mps)
                    {
                        size = pipe->ep_mps;
                    }

                    musb_write_packet(0, urb->transfer_buffer, size);
                    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_TXRDY;

                    urb->transfer_buffer += size;
                    urb->transfer_buffer_length -= size;
                    urb->actual_length += size;
                }
            }
            else
            {
                usb_ep0_state = USB_EP0_STATE_IN_STATUS;
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = (USB_CSRL0_REQPKT | USB_CSRL0_STATUS);

                #if defined(CONFIG_USB_SIM)
                g_USBH.CSR = USB_CSRL0_STATUS;
                #endif /* CONFIG_USB_SIM */
            }
            break;
        case USB_EP0_STATE_IN_DATA:
            if (ep0_status & USB_CSRL0_RXRDY)
            {
                size = urb->transfer_buffer_length;
                if (size > pipe->ep_mps)
                {
                    size = pipe->ep_mps;
                }

                size = MIN(size, HWREGH(USB_BASE + MUSB_IND_RXCOUNT_OFFSET));
                musb_read_packet(0, urb->transfer_buffer, size);
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_CSRL0_RXRDY;
                urb->transfer_buffer += size;
                urb->transfer_buffer_length -= size;
                urb->actual_length += size;

                if ((size < pipe->ep_mps) || (urb->transfer_buffer_length == 0))
                {
                    usb_ep0_state = USB_EP0_STATE_OUT_STATUS;
                    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = (USB_CSRL0_TXRDY | USB_CSRL0_STATUS);
                }
                else
                {
                    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_REQPKT;
                    #if defined(CONFIG_USB_SIM)
                    g_USBH.CSR = 0;
                    #endif  /* CONFIG_USB_SIM */
                }
            }
            break;
        case USB_EP0_STATE_OUT_DATA:
            if (urb->transfer_buffer_length > 0)
            {
                size = urb->transfer_buffer_length;
                if (size > pipe->ep_mps)
                {
                    size = pipe->ep_mps;
                }

                musb_write_packet(0, urb->transfer_buffer, size);
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_TXRDY;

                urb->transfer_buffer += size;
                urb->transfer_buffer_length -= size;
                urb->actual_length += size;
            }
            else
            {
                usb_ep0_state = USB_EP0_STATE_IN_STATUS;
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = (USB_CSRL0_REQPKT | USB_CSRL0_STATUS);

                #if defined(CONFIG_USB_SIM)
                g_USBH.CSR = USB_CSRL0_STATUS;
                #endif  /* CONFIG_USB_SIM */
            }
            break;
        case USB_EP0_STATE_OUT_STATUS:
            urb->errorcode = 0;
            musb_pipe_waitup(pipe);
            break;
        case USB_EP0_STATE_IN_STATUS:
            #if !defined(CONFIG_USB_SIM)
            if (ep0_status & (USB_CSRL0_RXRDY | USB_CSRL0_STATUS))
            #endif /* CONFIG_USB_SIM == false */
            {
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~(USB_CSRL0_RXRDY | USB_CSRL0_STATUS);
                urb->errorcode = 0;
                musb_pipe_waitup(pipe);
            }
            break;
    }
    return;
}

#if 1
void USBH_IRQHandler(void)
{
    uint32_t is;
    uint32_t txis;
    uint32_t rxis;
    uint8_t ep_csrl_status;
    // uint8_t ep_csrh_status;
    struct musb_pipe *pipe;
    struct usbh_urb *urb;
    uint8_t ep_idx;
    uint8_t old_ep_idx;

    #if 0//defined(CONFIG_USB_SIM)
    sys_vmsg_node_t     *pNode = 0;
    sys_vmsgq_recv(&g_dc_vmsgq, &pNode);
    if( pNode == 0 )
        return;

    memcpy(&g_USBH, &pNode->usb_regs, sizeof(g_USBH));
    memcpy(&g_uhost_rx_buf, &pNode->raw, sizeof(pNode->raw));
    free(pNode);
    #endif  /* CONFIG_USB_SIM */

    is = HWREGB(USB_BASE + MUSB_IS_OFFSET);
    txis = HWREGH(USB_BASE + MUSB_TXIS_OFFSET);
    rxis = HWREGH(USB_BASE + MUSB_RXIS_OFFSET);

    HWREGB(USB_BASE + MUSB_IS_OFFSET) = is;

    #if defined(CONFIG_USB_SIM)
    g_USBH.IS = 0;
    g_USBH.TXIS = 0;
    #endif  /* CONFIG_USB_SIM */

    old_ep_idx = musb_get_active_ep();

    if (is & USB_IS_CONN)
    {
        g_musb_hcd.port_csc = 1;
        g_musb_hcd.port_pec = 1;
        g_musb_hcd.port_pe = 1;
        usbh_roothub_thread_wakeup(1);
    }

    if (is & USB_IS_DISCON)
    {
        g_musb_hcd.port_csc = 1;
        g_musb_hcd.port_pec = 1;
        g_musb_hcd.port_pe = 0;
        for (uint8_t index = 0; index < CONFIG_USBHOST_PIPE_NUM; index++)
        {
            for (uint8_t j = 0; j < 2; j++)
            {
                struct musb_pipe *pipe = &g_musb_hcd.pipe_pool[index][j];
                struct usbh_urb *urb = pipe->urb;
                if (pipe->waiter)
                {
                    pipe->waiter = false;
                    urb->errorcode = -ESHUTDOWN;
                    usb_osal_sem_give(pipe->waitsem);
                }
            }
        }
        usbh_roothub_thread_wakeup(1);
    }

    if (is & USB_IS_SOF)
    {
    }

    if (is & USB_IS_RESUME)
    {
    }

    if (is & USB_IS_SUSPEND)
    {
    }

    if (is & USB_IS_VBUSERR)
    {
    }

    if (is & USB_IS_SESREQ)
    {
    }

    if (is & USB_IS_BABBLE)
    {
    }

    txis &= HWREGH(USB_BASE + MUSB_TXIE_OFFSET);
    /* Handle EP0 interrupt */
    if (txis & USB_TXIE_EP0)
    {
        txis &= ~USB_TXIE_EP0;
//        HWREGH(USB_BASE + MUSB_TXIS_OFFSET) = USB_TXIE_EP0;

        #if defined(CONFIG_USB_SIM)
        g_USBH.TXIS |= USB_TXIE_EP0;
        #endif  /* CONFIG_USB_SIM */

        hc_handle_ep0();
    }

    for (ep_idx = 1; ep_idx < CONIFG_USB_MUSB_PIPE_NUM; ep_idx++)
    {
        if (txis & (1 << ep_idx))
        {
//            HWREGH(USB_BASE + MUSB_TXIS_OFFSET) = (1 << ep_idx);

            pipe = &g_musb_hcd.pipe_pool[ep_idx][0];
            urb = pipe->urb;
            musb_set_active_ep(ep_idx);

            ep_csrl_status = HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET);

            if (ep_csrl_status & USB_TXCSRL1_ERROR)
            {
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_TXCSRL1_ERROR;
                urb->errorcode = -EIO;
                goto pipe_wait;
            }
            else if (ep_csrl_status & USB_TXCSRL1_NAKTO)
            {
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_TXCSRL1_NAKTO;
                urb->errorcode = -EBUSY;
                goto pipe_wait;
            }
            else if (ep_csrl_status & USB_TXCSRL1_STALL)
            {
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_TXCSRL1_STALL;
                urb->errorcode = -EPERM;
                goto pipe_wait;
            }
            else
            {
                uint32_t size = urb->transfer_buffer_length;

                if (size > pipe->ep_mps)
                {
                    size = pipe->ep_mps;
                }

                urb->transfer_buffer += size;
                urb->transfer_buffer_length -= size;
                urb->actual_length += size;

                if (urb->transfer_buffer_length == 0)
                {
                    urb->errorcode = 0;
                    goto pipe_wait;
                }
                else
                {
                    musb_write_packet(ep_idx, urb->transfer_buffer, size);
                    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_TXCSRL1_TXRDY;
                }
            }
        }
    }

    rxis &= HWREGH(USB_BASE + MUSB_RXIE_OFFSET);
    for (ep_idx = 1; ep_idx < CONIFG_USB_MUSB_PIPE_NUM; ep_idx++)
    {
        if (rxis & (1 << ep_idx))
        {
//            HWREGH(USB_BASE + MUSB_RXIS_OFFSET) = (1 << ep_idx); // clear isr flag

            pipe = &g_musb_hcd.pipe_pool[ep_idx][1];
            urb = pipe->urb;
            musb_set_active_ep(ep_idx);

            ep_csrl_status = HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET);
            //ep_csrh_status = HWREGB(USB_BASE + MUSB_IND_RXCSRH_OFFSET); // todo:for iso transfer

            if (ep_csrl_status & USB_RXCSRL1_ERROR)
            {
                HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) &= ~USB_RXCSRL1_ERROR;
                urb->errorcode = -EIO;
                goto pipe_wait;
            }
            else if (ep_csrl_status & USB_RXCSRL1_NAKTO)
            {
                HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) &= ~USB_RXCSRL1_NAKTO;
                urb->errorcode = -EBUSY;
                goto pipe_wait;
            }
            else if (ep_csrl_status & USB_RXCSRL1_STALL)
            {
                HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) &= ~USB_RXCSRL1_STALL;
                urb->errorcode = -EPERM;
                goto pipe_wait;
            }
            else if (ep_csrl_status & USB_RXCSRL1_RXRDY)
            {
                uint32_t size = urb->transfer_buffer_length;
                if (size > pipe->ep_mps)
                {
                    size = pipe->ep_mps;
                }
                size = MIN(size, HWREGH(USB_BASE + MUSB_IND_RXCOUNT_OFFSET));

                musb_read_packet(ep_idx, urb->transfer_buffer, size);

                HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) &= ~USB_RXCSRL1_RXRDY;

                urb->transfer_buffer += size;
                urb->transfer_buffer_length -= size;
                urb->actual_length += size;

                if ((size < pipe->ep_mps) || (urb->transfer_buffer_length == 0))
                {
                    urb->errorcode = 0;
                    goto pipe_wait;
                }
                else
                {
                    HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) = USB_RXCSRL1_REQPKT;
                }
            }
        }
    }
    musb_set_active_ep(old_ep_idx);
    return;
pipe_wait:
    musb_set_active_ep(old_ep_idx);
    musb_pipe_waitup(pipe);
    return;
}

#else
void USBH_IRQHandler(void)
{
    static sys_msg_info_t   msg_uhost;
    int                     txbuf_size = sizeof(g_uhost_tx_buf);
    int                     rxbuf_size = sizeof(g_uhost_rx_buf);

    snprintf(g_uhost_tx_buf, sizeof(g_uhost_tx_buf), "H -> D");

    msg_uhost.pMsg_buf = g_uhost_tx_buf;
    msg_uhost.msg_len  = strlen(g_uhost_tx_buf);
    sys_send_msg(g_th_udev_id, &msg_uhost);

    sys_wait_msg((uint8_t*)g_uhost_rx_buf, &rxbuf_size);

    #if 1
    printf("===== uhost RX ====\n");
    if( rxbuf_size )
    {
        for(int i = 0; i < rxbuf_size; i++)
        {
            printf("%c", g_uhost_rx_buf[i]);
        }
        printf("\n");
    }
    #endif // 1


    return;
}
#endif
