
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "sys_sim.h"
#include "sys_udev.h"
#include "sys_uhost.h"

#include "usbh_core.h"


DWORD   g_th_uhost_id;
DWORD   g_th_udev_id;

UINT    g_msg_uhost_tx = 0;
UINT    g_msg_uhost_rx = 0;

uint32_t __usbh_class_info_start__;
uint32_t __usbh_class_info_end__;

struct usbh_class_info      *g_class_info_pool[10] =
{
    0
};

static HANDLE       g_hTh;
static HANDLE       g_hEventUDevReady;
static HANDLE       g_hEventHCIncoming;
static HANDLE       g_hEventHCAppEnd;

HANDLE  g_hMutexHC;
HANDLE  g_hMutexDC;

extern void USBH_IRQHandler(void);
extern void USBD_IRQHandler(void);


extern sys_udev_t      g_udev_cdc;

/**
 *  sys usb host
 */
extern sys_uhost_class_t   g_uhost_class_cdc;
extern sys_uhost_class_t   g_uhost_vthreads;

/**
 *  class info of USB Host
 */
extern const struct usbh_class_info     cdc_acm_class_info;
extern const struct usbh_class_info     cdc_data_class_info;
extern const struct usbh_class_info     hub_class_info;
extern const struct usbh_class_info     hid_custom_class_info;
extern const struct usbh_class_info     msc_class_info;

bool     g_is_hc_end = false;
bool     g_is_dc_end = false;

static uint32_t __stdcall
_thread_udev(PVOID pM)
{
    printf(" @ %s\n", __func__);

    if( !SetEvent(g_hEventUDevReady) ) //set thread start event
    {
        err("set event fail \n");
    }

    sys_udev_init();

    while(1)
    {
        sys_msg_info_t   msg_udev;
        uint32_t         msg = 0;
        uint32_t         msg_len = sizeof(uint32_t);

        sys_uhost_proc();

//        sys_udev_proc();

        Sleep(1000);

    }

    return 0;
}

int main()
{
    int         has_exit = false;

    /* USB device */
    sys_udev_register(&g_udev_cdc);

    /* classes of USB Host */
//    sys_uhost_register(&g_uhost_class_cdc);
    sys_uhost_register(&g_uhost_vthreads);

    /* Class info */
    int     cnt = 0;
    __usbh_class_info_start__ = (uint32_t)&g_class_info_pool[0];
    g_class_info_pool[cnt++] = &cdc_acm_class_info;
    g_class_info_pool[cnt++] = &cdc_data_class_info;
    g_class_info_pool[cnt++] = &hub_class_info;
    g_class_info_pool[cnt++] = &msc_class_info;
    g_class_info_pool[cnt++] = &hid_custom_class_info;
    __usbh_class_info_end__   = (uint32_t)&g_class_info_pool[cnt];

#if 1
    /* Windows API */
    g_th_uhost_id = GetCurrentThreadId();

    g_msg_uhost_tx = SYS_UMSG_HOST_TX;
    g_msg_uhost_rx = SYS_UMSG_HOST_RX;

    g_hMutexHC = CreateMutex(NULL, TRUE, NULL);
    if( g_hMutexHC == NULL )
    {
        err("Create mutex HC fail \n");
    }

    g_hMutexDC = CreateMutex(NULL, TRUE, NULL);
    if( g_hMutexDC == NULL )
    {
        err("Create mutex DC fail \n");
    }

    ReleaseMutex(g_hMutexHC);
    ReleaseMutex(g_hMutexDC);


    g_hEventHCIncoming = CreateEvent(0, FALSE, FALSE, 0); //create thread start event
    if( g_hEventHCIncoming == 0 )
    {
        err("create event fail \n");
    }

    g_hEventHCAppEnd   = CreateEvent(0, FALSE, FALSE, 0); //create thread start event
    if( g_hEventHCAppEnd == 0 )
    {
        err("create event fail \n");
    }

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

#endif


    usbh_initialize();
//	usbh_class_test();

    sys_uhost_init();

    g_is_hc_end = false;
    g_is_dc_end = true;

    while(1)
    {
        sys_msg_info_t   msg_uhost;
        uint32_t         msg = 0;
        uint32_t         msg_len = sizeof(uint32_t);

        USBH_IRQHandler();

        Sleep(1);

        USBD_IRQHandler();

    }

    return 0;
}
