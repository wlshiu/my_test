#ifndef __net_util_windows_H_wGxFG9m5_lOJJ_HIgt_szPX_ugfUOPkJVYVK__
#define __net_util_windows_H_wGxFG9m5_lOJJ_HIgt_szPX_ugfUOPkJVYVK__

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#define RAND_SEED       (time(NULL))
//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static int _Get_MAC_Addr(char *pIp, char *pMac_addr)
{
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(AdapterInfo);
    char *mac_addr = (char*)malloc(12);

    AdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));

    // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO *) malloc(dwBufLen);
        if (AdapterInfo == NULL)
        {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return -1;
        }
    }

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
    {
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
        do{
            sprintf(pMac_addr, "%02X%02X%02X%02X%02X%02X",
                    pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                    pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                    pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
            if(strcmp(pIp, pAdapterInfo->IpAddressList.IpAddress.String) == 0)
            {
                printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, pMac_addr);
                free(AdapterInfo);
                return 0;
            }
            pAdapterInfo = pAdapterInfo->Next;
        }while(pAdapterInfo);
    }
    free(AdapterInfo);
    return 0;
}

static int _Get_Host_IP(char *pBuf)
{
    //char *Ip;
    WSADATA wsaData;
    struct hostent *pHostEnt;
    struct sockaddr_in tmpSockAddr; //placeholder for the ip address
    char hostname[128];

    // Not needed if it is already taken care by some other part of the application
    WSAStartup(MAKEWORD(2,0),&wsaData);

    gethostname(hostname, sizeof(hostname)) ;
    pHostEnt = gethostbyname(hostname);

    if(pHostEnt == NULL)
    {
        printf("Error occured: %s\n",GetLastError());
        return 0;
    }

    memcpy(&tmpSockAddr.sin_addr, pHostEnt->h_addr, pHostEnt->h_length);

    // Ip = (char *)malloc(17);
    strcpy(pBuf, inet_ntoa(tmpSockAddr.sin_addr));
    printf("Ip Address of the machine %s is %s\n",hostname, pBuf);

    // Not needed if it is already taken care by some other part of the application
    WSACleanup();
    return 0;
}
//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
