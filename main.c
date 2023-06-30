
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "usbh_core.h"

#define MSG_MST_TX          (WM_USER + 100)
#define MSG_MST_RX          (WM_USER + 200)


#define err(str, ...)       do{ printf("[%s:%d][Error] " str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)

typedef struct my_msg_info
{
    uint8_t     *pMsg_buf;
    int         msg_len;
} my_msg_info_t;


DWORD   g_th_uhost_id;
DWORD   g_th_udev_id;

uint32_t __usbh_class_info_start__;
uint32_t __usbh_class_info_end__;

struct usbh_class_info      g_class_info_pool[10] =
{
    0
};

static HANDLE       g_hTh;
static HANDLE       g_hEventUDevReady;

static uint8_t      g_uhost_tx_buf[1024] = {0};
static uint8_t      g_uhost_rx_buf[1024] = {0};

static uint8_t      g_udev_rxbuf[1024] = {0};
static uint8_t      g_udev_txbuf[1024] = {0};

extern void USBH_IRQHandler(uint8_t *pBuf_rx, int RxBytes, uint8_t *pBuf_tx, int *pTxBytes);
extern void USBD_IRQHandler(uint8_t *pBuf_rx, int RxBytes, uint8_t *pBuf_tx, int *pTxBytes);

static int
_wait_msg(uint8_t *pBuf, int *pBuf_size)
{
    MSG     msg;

    while(true)
    {
        if( !GetMessage(&msg, 0, 0, 0) ) //get msg from message queue
        {
            printf(" th %d wait\n", GetCurrentThreadId());
            Sleep(1);
            continue;
        }

        switch( msg.message )
        {
            case MSG_MST_TX:
            case MSG_MST_RX:
                {
                    int             len = *pBuf_size;
                    my_msg_info_t   *pMsg_info = (my_msg_info_t*)msg.wParam;

                    len = (pMsg_info->msg_len > len) ? len : pMsg_info->msg_len;
                    memcpy(pBuf, pMsg_info->pMsg_buf, len);
                    *pBuf_size = len;
                }
                return 0;

            default:
                break;
        }
    }
    return 0;
}

static int
_send_msg(DWORD th_id, uint8_t *pBuf)
{
    if( !PostThreadMessage(th_id, MSG_MST_TX, (WPARAM)pBuf, 0) ) //post thread msg
    {
        // fail
    }
    return 0;
}

static uint32_t __stdcall
_thread_udev(PVOID pM)
{
    printf(" @ %s\n", __func__);

    if( !SetEvent(g_hEventUDevReady) ) //set thread start event
    {
        err("set event fail \n");
    }

    while(1)
    {
        static my_msg_info_t    msg_udev_tx;
        int                     rxbuf_size = sizeof(g_udev_rxbuf);
        int                     txbuf_size = sizeof(g_udev_txbuf);

        _wait_msg((uint8_t*)g_udev_rxbuf, &rxbuf_size);

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

        USBD_IRQHandler((uint8_t*)g_udev_rxbuf, rxbuf_size,
                        (uint8_t*)g_udev_txbuf, &txbuf_size);

        msg_udev_tx.pMsg_buf = g_udev_txbuf;
        msg_udev_tx.msg_len  = txbuf_size;
        _send_msg(g_th_uhost_id, &msg_udev_tx);
    }

    return 0;
}


int main()
{
    int         has_exit = false;

    __usbh_class_info_start__ = (uint32_t)&g_class_info_pool[0];
    __usbh_class_info_end__   = (uint32_t)&g_class_info_pool[9];

    g_th_uhost_id = GetCurrentThreadId();

    g_hEventUDevReady = CreateEvent(0, FALSE, FALSE, 0); //create thread start event
    if( g_hEventUDevReady == 0 )
    {
        err("create event fail \n");
    }

    g_hTh = CreateThread(NULL, 0 /* use default stack size */,
                       (LPTHREAD_START_ROUTINE)_thread_udev,
                       (void*)&has_exit, // argument to thread function
                       0,               // use default creation flags
                       &g_th_udev_id);    // returns the th
    if( g_hTh == NULL )
    {
        err("CreateThread fail\n");
    }

    WaitForSingleObject(g_hEventUDevReady, INFINITE);

    while(1)
    {
        static my_msg_info_t    msg_uhost_tx;
        int                     txbuf_size = sizeof(g_uhost_tx_buf);
        int                     rxbuf_size = sizeof(g_uhost_rx_buf);

        USBD_IRQHandler((uint8_t*)g_uhost_rx_buf, rxbuf_size,
                        (uint8_t*)g_uhost_tx_buf, &txbuf_size);

        msg_uhost_tx.pMsg_buf = g_uhost_tx_buf;
        msg_uhost_tx.msg_len  = strlen(g_uhost_tx_buf);
        _send_msg(g_th_udev_id, &msg_uhost_tx);

        _wait_msg((uint8_t*)g_uhost_rx_buf, &rxbuf_size);

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

    }

    return 0;
}
