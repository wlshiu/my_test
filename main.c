/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file main.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/04/30
 * @license
 * @description
 */


#include <stdint.h>
#include <windows.h>
#include <winsock2.h>
#include <pthread.h>


#include "common.h"
#include "gen_data.h"
#include "upgrade_client.h"
#include "upgrade_server.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_PORT_NUM        1234
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
pthread_mutex_t             g_log_mtx;


static pthread_cond_t       g_usr_cond;
static pthread_mutex_t      g_usr_mtx;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void*
_task_server(void *argv)
{
    uint32_t    *pIs_running = (uint32_t*)argv;
    SOCKET      srvSocket;
    do {
        struct sockaddr_in     srvAddr;

        srvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if( srvSocket == INVALID_SOCKET )
        {
            log_msg("socket error !");
            break;
        }

        srvAddr.sin_family           = AF_INET;
        srvAddr.sin_port             = htons(CONFIG_PORT_NUM);
        srvAddr.sin_addr.S_un.S_addr = INADDR_ANY;
        if( bind(srvSocket, (struct sockaddr*)&srvAddr, sizeof(srvAddr)) == SOCKET_ERROR )
        {
            log_msg("bind error !");

            break;
        }

        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_signal(&g_usr_cond);
        pthread_mutex_unlock(&g_usr_mtx);

        while( *pIs_running )
        {
            int                 rval = 0;
            struct sockaddr_in  remoteAddr;
            int                 addrLen = sizeof(remoteAddr);
            char                *pSendData = "Hey I'm Server\n";
            char                recvData[256] = {0};

            rval = recvfrom(srvSocket, recvData, sizeof(recvData), 0, (struct sockaddr *)&remoteAddr, &addrLen);
            if( rval > 0 )
            {
                recvData[rval] = 0x00;
                log_msg("[SRV] from(%s) '%s' \n", inet_ntoa(remoteAddr.sin_addr), recvData);
            }

            sendto(srvSocket, pSendData, strlen(pSendData), 0, (struct sockaddr*)&remoteAddr, addrLen);

            Sleep(10);
        }
    } while(0);

    closesocket(srvSocket);

    pthread_exit(0);
    return 0;
}

static void*
_task_client(void *argv)
{
    uint32_t    *pIs_running = (uint32_t*)argv;
    SOCKET      clntSocket;
    do {
        struct sockaddr_in      sin;
        int                     len = 0;

        clntSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        sin.sin_family           = AF_INET;
        sin.sin_port             = htons(CONFIG_PORT_NUM);
        sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

        len = sizeof(sin);

        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_signal(&g_usr_cond);
        pthread_mutex_unlock(&g_usr_mtx);

        char    *pSendData = "Hey I am client\n";

        while( *pIs_running )
        {
            int     rval = 0;
            char    recvData[256] = {0};

            sendto(clntSocket, pSendData, strlen(pSendData), 0, (struct sockaddr*)&sin, len);

            rval = recvfrom(clntSocket, recvData, sizeof(recvData), 0, (struct sockaddr*)&sin, &len);
            if( rval > 0 )
            {
                recvData[rval] = 0x00;
                log_msg("[CLNT] from(%s) '%s'\n", inet_ntoa(sin.sin_addr), recvData);
            }

            Sleep(30);
        }

    } while(0);

    closesocket(clntSocket);

    pthread_exit(0);
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(int argc, char **argv)
{
    int         rval = 0;
    pthread_t   t_srv, t_clnt;
    do {
        uint32_t    is_running = 1;

        WSADATA     w;
        WORD        sockVersion = MAKEWORD(2,2);
        if( WSAStartup(sockVersion, &w) != 0 )
        {
            log_msg("Could not open Windows connection.\n");
            break;;
        }

        rval = pthread_mutex_init(&g_log_mtx, 0);
        if( rval )   break;

        rval = pthread_mutex_init(&g_usr_mtx, 0);
        if( rval )   break;

        rval = pthread_cond_init(&g_usr_cond, 0);
        if( rval )   break;

        pthread_create(&t_srv, 0, _task_server, &is_running);

        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
        pthread_mutex_unlock(&g_usr_mtx);

        pthread_create(&t_clnt, 0, _task_client, &is_running);

        pthread_mutex_lock(&g_usr_mtx);
        pthread_cond_wait(&g_usr_cond, &g_usr_mtx);
        pthread_mutex_unlock(&g_usr_mtx);

        Sleep(100);
        is_running = 0;
        pthread_join(t_srv, 0);
        pthread_join(t_clnt, 0);
    } while(0);

    WSACleanup();

    system("pause");
    return 0;
}
