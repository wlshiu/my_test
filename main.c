#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <IPHlpApi.h>


extern int tftp_client(int argc, char **argv);
extern int tftp_server(int argc, char **argv);

int main(int argc, char **argv)
{
#if defined(WIN32)
    // Initialize Winsock
    WSADATA     wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif

    tftp_server(argc, argv);
//    tftp_client(argc, argv);

    WSACleanup();

    return 0;
}
