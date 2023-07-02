
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "sys_sim.h"
#include "sys_udev.h"

#include "usbh_core.h"


DWORD   g_th_uhost_id;
DWORD   g_th_udev_id;

UINT    g_msg_uhost_tx = 0;
UINT    g_msg_uhost_rx = 0;

uint32_t __usbh_class_info_start__;
uint32_t __usbh_class_info_end__;

struct usbh_class_info      g_class_info_pool[10] =
{
    0
};

static HANDLE       g_hTh;
static HANDLE       g_hEventUDevReady;

extern void USBH_IRQHandler(void);
extern void USBD_IRQHandler(void);


extern sys_udev_t      g_udev_cdc;


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
        USBD_IRQHandler();

        sys_udev_proc();
    }

    return 0;
}


int main()
{
    int         has_exit = false;

    sys_udev_register(&g_udev_cdc);

    __usbh_class_info_start__ = (uint32_t)&g_class_info_pool[0];
    __usbh_class_info_end__   = (uint32_t)&g_class_info_pool[9];

    g_th_uhost_id = GetCurrentThreadId();

    g_msg_uhost_tx = SYS_UMSG_HOST_TX;
    g_msg_uhost_rx = SYS_UMSG_HOST_RX;

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

    usbh_initialize();
//	usbh_class_test();

    while(1)
    {
        USBH_IRQHandler();
    }

    return 0;
}
