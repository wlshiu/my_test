#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <errno.h>
//#include <iostream>
#include "libusb.h"

#define TEST_SIZE 50

#define STOPBITS_1 0
#define STOPBITS_1_5 1
#define STOPBITS_2 2

#define VCP_PARITY_NONE 0
#define VCP_PARITY_ODD 1
#define VCP_PARITY_EVEN 2
#define VCP_PARITY_MARK 3
#define VCP_PARITY_SPACE 4

typedef struct _vcp_config_setting
{
    unsigned int BaudRate;
    unsigned char StopBits;
    unsigned char Parity;
    unsigned char DataBits;
} vcp_config_setting;

static struct libusb_device_handle *devh = NULL;

#define PACK_SIZE 64
uint8_t endpoint_in = 0x82;
uint8_t endpoint_out = 0x02; // default IN and OUT endpoints
unsigned char tx_data_tmp[TEST_SIZE];
unsigned char rx_data_tmp[TEST_SIZE * 2];
int size = 0;
uint8_t endpoint_in_array[1] = {0x82};
uint8_t endpoint_out_array[1] = {0x02};
unsigned char tx_data[] = "this is a test string sent through usb vcp\r\n";

static unsigned long num_bytes = 0, num_xfer = 0;
unsigned int wr_cnt = 0;
unsigned int rd_cnt = 0;
vcp_config_setting vcp_config;
vcp_config_setting vcp_config_back;

//异步接收的回调函数，当接收到数据后，以字符串的形式输出到控制台
void LIBUSB_CALL cb_xfr_in(struct libusb_transfer *xfr)
{
    int i;
    rd_cnt++;
    if (xfr->status != LIBUSB_TRANSFER_COMPLETED)
    {
        fprintf(stderr, "transfer status %d\n", xfr->status);
        libusb_free_transfer(xfr);
        exit(3);
    }
    for (i = 0; i < xfr->actual_length; i++)
    {
        printf("%c", xfr->buffer[i]);
    }

// 本次接收数据完成后，重新提交传输请求，并等待下次接收事件，若提交失败，则释放该段内存
    if(libusb_submit_transfer(xfr) < 0)
    {
        libusb_free_transfer(xfr);
    }
}

static int benchmark_in(uint8_t ep)
{
    static uint8_t buf[PACK_SIZE];
    static struct libusb_transfer *xfr;

    printf("(SUBMIT FUNC) Starting a new async bulk transfer, READ\n\n");
    xfr = libusb_alloc_transfer(0);
    if (!xfr)
        return -ENOMEM;

    libusb_fill_bulk_transfer (
        xfr,
        devh,
        ep,
        buf,
        sizeof(buf),
        cb_xfr_in,
        NULL,
        0
    );
    return libusb_submit_transfer(xfr);
}

int main(int argc, char *argv[])
{
    int rc;
    int i = 0;

    libusb_context  *ctx = NULL;

    rc = libusb_init(&ctx);
    if (rc < 0)
        return rc;

    /**
     *  + CH340 (USB to Uart)
     *      > USB\VID_1A86&PID_7523
     *  + STM32 VCP
     *      > VID_1A0A&PID_3B90
     */
    devh = libusb_open_device_with_vid_pid(ctx, 0x1A0A, 0x3B90);
//    devh = libusb_open_device_with_vid_pid(ctx, 0x1A86, 0x7523);
    if(NULL == devh)
    {
        printf("Open USB device failed\n");
        goto out;
    }

    rc = libusb_claim_interface(devh, 0);
    if (rc < 0)
    {
        printf("Error claiming interface: %s\n",
               libusb_error_name(rc));
        goto out;
    }

    printf("DEVICE READY...\n\n");
    vcp_config.BaudRate = 115200;
    vcp_config.Parity = 0;
    vcp_config.StopBits = 0;
    vcp_config.DataBits = 8;
    rc = libusb_control_transfer (
            devh, 0b00100001, 0x20, 0, 0,
            (unsigned char *)(&vcp_config), sizeof(vcp_config), 0);
    if(rc < 0)
    {
        printf("Error: %s", libusb_strerror(rc));
        return -1;
    }

    rc = libusb_bulk_transfer(
            devh, endpoint_out, tx_data, sizeof(tx_data),
            &size, 1000);
    if(rc < 0)
    {
        printf("Error: %s", libusb_strerror(rc));
    }
    else
    {
        printf("%d bytes written, ret: %d\n", size, rc);
        size = 0;
    }

    benchmark_in(endpoint_in);
    while(1)
    {
        rc = libusb_handle_events(ctx);
    }

    libusb_release_interface(devh, 0);

out:
    if(devh)
        libusb_close(devh);

    libusb_exit(ctx);
    return 0;
}
