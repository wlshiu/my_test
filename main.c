
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <pthread.h>

#define CONFIG_MAX_RX_SIZE      128
#define CONFIG_TARGET_PORT      1234

#define err(str, ...)       do{ printf("[%s:%d]" str, __func__, __LINE__, ##__VA_ARGS__); while(1); }while(0)

static pthread_cond_t       g_cond;
static pthread_mutex_t      g_mtx;


static void* _server(void *pArgv)
{
    int                     *pHas_exit = (int*)pArgv;
    SOCKET                  servSock;
    struct sockaddr_in      sockAddr;

    servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&sockAddr, 0, sizeof(sockAddr));

    sockAddr.sin_family      = PF_INET;  // use IPv4
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port        = htons(CONFIG_TARGET_PORT);  // port

    if( bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) == SOCKET_ERROR )
    {
        err("sock bind fail: %d\n", WSAGetLastError());
    }

    pthread_mutex_lock(&g_mtx);
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mtx);

    while( *pHas_exit == false )
    {
        SOCKET      clntSock;
        SOCKADDR    clntAddr;
        int         nSize = sizeof(SOCKADDR);

        listen(servSock, 20);

        if( (clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize)) != INVALID_SOCKET )
        {
            int     nbytes = 0;
            char    msg[128] = {0};

            nbytes = recv(clntSock, msg, sizeof(msg), 0);
            if( nbytes && nbytes != SOCKET_ERROR )
            {
                printf("Svr Rx: %s\n", msg);
            }

            snprintf(msg, sizeof(msg), "Hello World");

            nbytes = send(clntSock, msg, strlen(msg) + 1, 0);
            if( nbytes == 0 || nbytes == SOCKET_ERROR )
            {
                err("Srv tx fail\n");
            }

            closesocket(clntSock);
        }
    }

    closesocket(servSock);

    pthread_exit(0);
    return 0;
}

int main()
{
    int         rval = 0;
    int         has_exit = false;
    pthread_t   t1;

    WSADATA     wsaData;
	printf("\nInitialising Winsock...\n\n");
	if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0 )
	{
		err("Failed. Error Code : %d\n", WSAGetLastError());
	}

    rval = pthread_mutex_init(&g_mtx, 0);
    if( rval )
    {
        err("mutex init fail\n");
    }

    rval = pthread_cond_init(&g_cond, 0);
    if( rval )
    {
        err("cond init fail\n");
    }

    pthread_create(&t1, NULL, _server, &has_exit);
    printf("create server thread...\n\n");

    pthread_mutex_lock(&g_mtx);
    pthread_cond_wait(&g_cond, &g_mtx);
    pthread_mutex_unlock(&g_mtx);

    {
        // Clint
        SOCKET              sock;
        struct sockaddr_in  sockAddr;

        if( (sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET )
        {
            err("Could not create socket : %d\n", WSAGetLastError());
        }

        memset(&sockAddr, 0, sizeof(sockAddr));

        sockAddr.sin_family      = PF_INET;  // IPv4
        sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        sockAddr.sin_port        = htons(CONFIG_TARGET_PORT); // port

        if( connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) < 0 )
        {
            err("client connect fail\n");
        }

        int     nbytes = 0;
        char    buffer[CONFIG_MAX_RX_SIZE] = {0};

        snprintf(buffer, sizeof(buffer), "Hey Server ~~\n");

        nbytes = send(sock, buffer, strlen(buffer) + 1, 0);
        if( nbytes == 0 || nbytes == SOCKET_ERROR )
        {
            err("Clnt tx fail\n");
        }


        nbytes = recv(sock, buffer, CONFIG_MAX_RX_SIZE, NULL);
        if( nbytes == 0 || nbytes == SOCKET_ERROR )
        {
            err("Clnt Rx fail\n");
        }

        printf("Clnt Rx: %s\n", buffer);

        has_exit = true;
        closesocket(sock);
    }

    printf("done~~~\n\n");

    pthread_mutex_destroy(&g_mtx);
    pthread_cond_destroy(&g_cond);
    pthread_join(t1, 0);

    WSACleanup();   // disable win socket
    system("pause");
    return 0;
}
