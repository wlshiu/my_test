/**
 * Copyright (c) 2021 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2021/08/04
 * @license
 * @description
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "comm_dev.h"
#include <windows.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ARG_MAX_NUM          3

#define CONFIG_RX_TIMEOUT_SEC       8

#define NC          "\033[m"
#define RED         "\033[0;32;31m"
#define GREEN       "\033[0;32;32m"
//=============================================================================
//                  Macro Definition
//=============================================================================
#define comm_dev_init(pCfg)                         g_pComm_dev->init(pCfg)
#define comm_dev_send(pHandle, pData, length)       g_pComm_dev->send_bytes(pHandle, pData, length)
#define comm_dev_recv(pHandle, pData, pLength)      g_pComm_dev->recv_bytes(pHandle, pData, pLength)
#define comm_dev_get_state(pHandle, stype)          g_pComm_dev->get_state(pHandle, stype)
#define comm_dev_reset_buf(pHandle, pbuf, len)      g_pComm_dev->reset_buf(pHandle, pbuf, len)
#define comm_dev_deinit(pHandle)                    g_pComm_dev->deinit(pHandle)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct usr_argv
{
    bool        is_running;
} usr_argv_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern comm_dev_desc_t      g_comm_dev_comport;
static comm_dev_desc_t      *g_pComm_dev = &g_comm_dev_comport;


static char                 g_line_buf[1024] = {0};
static char                 g_rx_buf[1024] = {0};
static char                 *g_pFile_name = 0;
static char                 *g_comport = 0;
static uint32_t             g_baudrate = 0;
static comm_handle_t        g_hComm = 0;
static HANDLE               g_hRecv;
static bool                 g_is_rx_idle = false;

static HANDLE               g_Mutex;

static usr_argv_t           g_usr_argv = {};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_get_params(int argc, char **argv)
{
    if (argc < 2) {
        return -1;
    }

    argv++; argc--;
    while(argc) {
        if (!strcmp(argv[0], "--file")) {
            g_pFile_name = argv[1];
        } else if (!strcmp(argv[0], "--comport")) {
            argv++; argc--;
            g_comport = argv[0];
        } else if (!strcmp(argv[0], "--baudrate")) {
            argv++; argc--;
            g_baudrate = atol(argv[0]);
        }
        else {

        }
        argv++; argc--;
    }

    return 0;
}

static void
_trim_leading_spaces(char* str)
{
    char    *pSrc = 0, *pDest = 0;

    pSrc = pDest = str;

    while( *pSrc == ' ' )
        pSrc++;

    if( pSrc != pDest )
    {
        uint32_t    len = strlen(str) - (int)(pSrc - pDest);
        memmove(pDest, pSrc, len);

        pDest[len] = '\0';
    }

    return;
}

static int
_strncasecmp(char *s1, char *s2, size_t n)
{
	if (n == 0)
		return 0;

	do {
		unsigned char c1 = (unsigned char) *s1++;
		unsigned char c2 = (unsigned char) *s2++;

		if (c1 != c2) {
			if (c1 >= 'A' && c1 <= 'Z' &&
			    c2 >= 'a' && c2 <= 'z')
				c1 += 'a' - 'A';
			else if (c1 >= 'a' && c1 <= 'z' &&
				 c2 >= 'A' && c2 <= 'Z')
				c2 += 'a' - 'A';
			if (c1 != c2)
				return c1 - c2;
		}
		if (c1 == 0)
			break;
	} while (--n != 0);

	return 0;
}

static uint32_t __stdcall
_comport_recv(PVOID pM)
{
    usr_argv_t      *pArgv = (usr_argv_t*)pM;
    uint32_t        cnt = 0;

    while( pArgv->is_running )
    {
        int     sleep_ms = 5;
        int     length = sizeof(g_rx_buf);

        memset(g_rx_buf, 0x0, length);
        comm_dev_recv(g_hComm, (uint8_t*)g_rx_buf, &length);
        if( length )
        {
            WaitForSingleObject(g_Mutex,   // handle to mutex
                                INFINITE); // no time-out interval

            fprintf(stdout, "%s", g_rx_buf);
//            printf("%s", g_rx_buf);

            fflush(stdout);
            g_is_rx_idle = false;
            cnt = 0;

            ReleaseMutex(g_Mutex);
        }
        else
        {
            Sleep(sleep_ms);
            cnt++;

            if( sleep_ms * cnt > CONFIG_RX_TIMEOUT_SEC )
                g_is_rx_idle = true;
        }
    }

    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void usage(char *pProg)
{
    printf("%s [option] [value]\n"
           "\n", pProg);

    system("pause");
    exit(-1);
}

int main(int argc, char **argv)
{
    int     rval = 0;
    FILE    *fin = 0;

    do {
        comm_cfg_t  com_cft = {};

        rval = _get_params(argc, argv);
        if( rval )
        {
            usage(argv[0]);
            break;
        }

        g_Mutex = CreateMutex(NULL,  // default security attributes
                              FALSE, // initially not owned
                              NULL); // unnamed mutex
        if( g_Mutex == NULL )
        {
            printf("CreateMutex() error %u\n", (unsigned int)GetLastError());
            break;
        }

        if( !(fin = fopen(g_pFile_name, "r")) )
        {
            printf("open %s fail !\n", g_pFile_name);
            break;
        }

        com_cft.comport.pComport_name = g_comport;
        com_cft.comport.baudrate      = g_baudrate;
        g_hComm = comm_dev_init(&com_cft);
        if( !g_hComm )
        {
            printf("open COM Port (%s) fail \n", com_cft.comport.pComport_name);
            break;
        }

        printf("\n\n=========================\n");

        g_usr_argv.is_running = true;
        g_hRecv = CreateThread(NULL, 0 /* use default stack size */,
                               (LPTHREAD_START_ROUTINE)_comport_recv,
                               (void*)&g_usr_argv, // argument to thread function
                               0,          // use default creation flags
                               NULL);      // returns the th
        if( g_hRecv == NULL )
        {
            printf("CreateThread fail\n");
            break;
        }

        while( fgets(g_line_buf, sizeof(g_line_buf), fin) != NULL )
        {
            char    ch = 0;
            _trim_leading_spaces(g_line_buf);

            #if 0
            WaitForSingleObject(g_Mutex, INFINITE);

            printf(GREEN "%s" NC, g_line_buf);

            ReleaseMutex(g_Mutex);
            #endif

            ch = g_line_buf[0];
            if( ch == ';' || ch == '//' || ch == '#' ||
                ch == '\r' || ch == '\n'  )
                continue;

//            printf(g_line_buf);

            if( !_strncasecmp(g_line_buf, "break", strlen("break")) ||
                !_strncasecmp(g_line_buf, "exit", strlen("exit")) ||
                !_strncasecmp(g_line_buf, "return", strlen("return")) )
            {
                break;
            }

            if( !_strncasecmp(g_line_buf, "wait", strlen("wait")) ||
                !_strncasecmp(g_line_buf, "delay", strlen("delay")) )
            {
                int         arg_cnt = 0;
                char        *pCmd_args[CONFIG_ARG_MAX_NUM] = {0};
                char        *pCur = g_line_buf;
                char        *pEnd = g_line_buf + strlen(g_line_buf);
                uint32_t    is_arg_head = 0;

                is_arg_head = 1;
                while( pCur < pEnd )
                {
                    char    c = *pCur;

                    if( c == ' ' || c == '\t' || c == '\n' )
                    {
                        *pCur++ = '\0';
                        is_arg_head = 1;
                        continue;
                    }

                    if( is_arg_head && arg_cnt < CONFIG_ARG_MAX_NUM &&
                        (((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') || c == '_' || c == '?') || c == '-') )
                    {
                        pCmd_args[arg_cnt++] = pCur;
                        is_arg_head = 0;
                    }

                    pCur++;
                }

                Sleep(atol(pCmd_args[1]));

                continue;
            }

            comm_dev_send(g_hComm, (uint8_t*)g_line_buf, strlen(g_line_buf));
            memset(g_line_buf, 0x0, sizeof(g_line_buf));

            Sleep(20);
        }
    } while(0);

    if( fin )   fclose(fin);

//    getch();

    while( g_is_rx_idle == false )
        Sleep(1);

    g_usr_argv.is_running = false;

    // Wait for the thread to finish.
//    WaitForSingleObject(g_hRecv, INFINITE);

    Sleep(200);

    // Close the thread handle and free memory allocation.
    CloseHandle(g_hRecv);

    comm_dev_deinit(g_hComm);
    g_hComm = 0;

    return 0;
}
