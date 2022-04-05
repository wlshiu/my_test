
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>


#define MSG_MST_TX          (WM_USER + 100)
#define MSG_MST_RX          (WM_USER + 200)


#define err(str, ...)       do{ printf("[%s:%d][Error] " str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)

typedef struct my_msg_info
{
    uint8_t     *pMsg_buf;
    int         msg_len;
} my_msg_info_t;


static HANDLE       g_hTh;
static DWORD        g_th_mst_id;
static DWORD        g_th_slv_id;

static HANDLE           g_hEventSlvReady;

static uint32_t              g_mst_tx_buf[1024 >> 4] = {0};
static uint32_t              g_mst_rx_buf[1024 >> 4] = {0};


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

//                    printf("thread %d:\n  ", GetCurrentThreadId());
//                    for(int i = 0; i < pMsg_info->msg_len; i++)
//                        printf("%c", pMsg_info->pMsg_buf[i]);
//
//                    printf("\n");
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
_thread_slv(PVOID pM)
{
    printf(" @ %s\n", __func__);

    if( !SetEvent(g_hEventSlvReady) ) //set thread start event
    {
        err("set event fail \n");
    }

    while(1)
    {
        static my_msg_info_t    msg_mst_rx;
        uint8_t                 buf[512] = {0};
        int                     buf_size = sizeof(buf);

        _wait_msg((uint8_t*)buf, &buf_size);

        printf("===== %s RX ====\n", __func__);
        for(int i = 0; i < buf_size; i++)
        {
            printf("%c", buf[i]);
        }

        printf("\n");

        snprintf(g_mst_rx_buf, sizeof(g_mst_rx_buf), "  from %s", __func__);

        msg_mst_rx.pMsg_buf = g_mst_rx_buf;
        msg_mst_rx.msg_len  = strlen(g_mst_rx_buf);
        _send_msg(g_th_mst_id, &msg_mst_rx);
    }

    return 0;
}


int main()
{
    int         has_exit = false;

    g_th_mst_id = GetCurrentThreadId();

    g_hEventSlvReady = CreateEvent(0, FALSE, FALSE, 0); //create thread start event
    if( g_hEventSlvReady == 0 )
    {
        err("create event fail \n");
    }

    g_hTh = CreateThread(NULL, 0 /* use default stack size */,
                       (LPTHREAD_START_ROUTINE)_thread_slv,
                       (void*)&has_exit, // argument to thread function
                       0,               // use default creation flags
                       &g_th_slv_id);    // returns the th
    if( g_hTh == NULL )
    {
        err("CreateThread fail\n");
    }

    WaitForSingleObject(g_hEventSlvReady, INFINITE);

    while(1)
    {
        static my_msg_info_t    msg_mst_tx;
        uint8_t                 buf[512] = {0};
        int                     buf_size = sizeof(buf);

        snprintf(g_mst_tx_buf, sizeof(g_mst_tx_buf), "  from %s", __func__);

        msg_mst_tx.pMsg_buf = g_mst_tx_buf;
        msg_mst_tx.msg_len  = strlen(g_mst_tx_buf);
        _send_msg(g_th_slv_id, &msg_mst_tx);

        _wait_msg((uint8_t*)buf, &buf_size);

        printf("===== %s RX ====\n", __func__);
        for(int i = 0; i < buf_size; i++)
        {
            printf("%c", buf[i]);
        }
        printf("\n");
    }

    return 0;
}
