/**********************************************
 * Author: ideawu(www.ideawu.net)
 * Date: 2007-04
 * File: server.c
 *********************************************/

#include "tftpx.h"
#include "work_thread.h"

void config()
{
    conf_document_root = ".";//"/home/ideawu/books";
}


int tftp_server(int argc, char **argv)
{
    int sock;
    int done = 0;   // Server exit.
    char *local_ip = 0;
    socklen_t addr_len;
    pthread_t t_id;
    struct sockaddr_in server;
    unsigned short port = SERVER_PORT;

    if(argc < 2)
    {
        printf("Usage: %s [ip] [port]\n", argv[0]);
        printf("    port - default %d\n", SERVER_PORT);
        return 0;
    }

    local_ip = argv[1];
    if(argc > 2)
    {
        port = (unsigned short)atoi(argv[2]);
    }

    config();

    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("Server socket could not be created.\n");
        return 0;
    }

    server.sin_family   = AF_INET;
    server.sin_port     = htons(port);
    inet_pton(AF_INET, local_ip, &(server.sin_addr.s_addr));

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Server bind failed. Server already running? Proper permissions?\n");
        return 2;
    }

    printf("Server started at %s:%d.\n", local_ip, port);

    struct tftpx_request *request;
    addr_len = sizeof(struct sockaddr_in);
    while(!done)
    {
        request = (struct tftpx_request *)malloc(sizeof(struct tftpx_request));
        memset(request, 0, sizeof(struct tftpx_request));
        request->size = recvfrom(
                            sock, &(request->packet), MAX_REQUEST_SIZE, 0,
                            (struct sockaddr *) &(request->client),
                            &addr_len);
        request->packet.cmd = ntohs(request->packet.cmd);
        printf("Receive request.\n");
        if(pthread_create(&t_id, NULL, work_thread, request) != 0)
        {
            printf("Can't create thread.\n");
        }
    }

    return 0;
}
