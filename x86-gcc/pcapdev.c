

#include "pthread.h"
#include "pcap.h"

#include "uipopt.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define err(str, ...)       printf("[%s:%u] " str, __func__, __LINE__, ##__VA_ARGS__)


#ifndef UIP_CONF_EXTERNAL_BUFFER
	extern uint8_t uip_buf[UIP_BUFSIZE+2];
#else
	extern unsigned char *uip_buf;
#endif

extern uint16_t    uip_len;
extern void     *uip_appdata;

//////////////////////////////////////////////////////////
static char                g_error_buffer[PCAP_ERRBUF_SIZE] = {0};
static pthread_cond_t      g_cond;
static pthread_mutex_t     g_mtx;
static pcap_t               *g_pAd_handle = 0;

/* From tcptraceroute, convert a numeric IP address to a string */
#define IPTOSBUFFERS    12
static char*
iptos(uint32_t in)
{
    static char output[IPTOSBUFFERS][3 * 4 + 3 + 1];
    static short which;
    u_char *p;

    p = (u_char *)&in;
    which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
    snprintf(output[which], sizeof(output[which]), "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output[which];
}

static void
_ifprint(pcap_if_t *d)
{
    pcap_addr_t *a;

    /* Description */
    if (d->description)
        printf("\tDescription: %s\n", d->description);

    /* Loopback Address*/
    printf("\tLoopback: %s\n", (d->flags & PCAP_IF_LOOPBACK) ? "yes" : "no");

    /* IP addresses */
    for(a = d->addresses; a; a = a->next)
    {
        switch(a->addr->sa_family)
        {
            case AF_INET:
                printf("\tAddress Family Name: AF_INET\n");
                if (a->addr)
                    printf("\tAddress: %s\n", iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
                if (a->netmask)
                    printf("\tNetmask: %s\n", iptos(((struct sockaddr_in *)a->netmask)->sin_addr.s_addr));
                if (a->broadaddr)
                    printf("\tBroadcast Address: %s\n", iptos(((struct sockaddr_in *)a->broadaddr)->sin_addr.s_addr));
                if (a->dstaddr)
                    printf("\tDestination Address: %s\n", iptos(((struct sockaddr_in *)a->dstaddr)->sin_addr.s_addr));
                break;

            case AF_INET6:
                printf("\tAddress Family Name: AF_INET6\n");
                break;

            default:
                printf("\tAddress Family Name: Unknown\n");
                break;
        }
    }
    printf("\n");
}

static int
_get_packet(unsigned int* pLen, char** pkt_data)
{
    int     rval = 0;

    do {
        int                 res;
        struct pcap_pkthdr  *header;

        *pLen = 0;
        res = pcap_next_ex(g_pAd_handle, &header, (const u_char**)pkt_data);
        if( res < 0 )
        {
            err("Error reading the packets: %s\n", pcap_geterr(g_pAd_handle));
            rval = -12;
            break;
        }

        *pLen = (res) ? header->len : 0;
    } while(0);
    return rval;
}

int
pcapdev_init(void)
{
    do {
        pcap_if_t   *alldevs;
        pcap_if_t   *d;
        int         inum;
        int         i = 0;

        pthread_mutex_init(&g_mtx, 0);
        pthread_cond_init(&g_cond, 0);

        /* Retrieve the device list */
        if( pcap_findalldevs(&alldevs, g_error_buffer) == -1 )
        {
            err("Error in pcap_findalldevs: %s\n", g_error_buffer);
            break;
        }

        /* Print the list */
        for(d = alldevs; d; d = d->next)
        {
            printf("\n%d. %s\n", ++i, d->name);
            _ifprint(d);
        }

        if( i == 0 )
        {
            err("%s", "\nNo interfaces found! Make sure WinPcap is installed.\n");
            break;
        }

        printf("Enter the interface number (1-%d):", i);
        scanf("%d", &inum);

        if( inum < 1 || inum > i )
        {
            err("%s", "\nInterface number out of range.\n");
            /* Free the device list */
            pcap_freealldevs(alldevs);
            break;;
        }

        /* Jump to the selected adapter */
        for(d = alldevs, i = 0; i < inum - 1; d = d->next, i++);

        /* Open the device */
        /* Open the adapter */
        if( (g_pAd_handle = pcap_open_live(d->name,	// name of the device
                                       65536,   // portion of the packet to capture.
                                                // 65536 grants that the whole packet will be captured on all the MACs.
                                       1,       // promiscuous mode (nonzero means promiscuous)
                                       1000,    // read timeout
                                       g_error_buffer   // error buffer
                                      )) == NULL)
        {
            err("\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
            /* Free the device list */
            pcap_freealldevs(alldevs);
            break;
        }

        printf("\nlistening on %s...\n", d->description);

        /* At this point, we don't need any more the device list. Free it */
        pcap_freealldevs(alldevs);

    } while(0);

	return 0;
}

unsigned int
pcapdev_read(void)
{
    uint32_t    len = 0;
    do {
        char        *pData = 0;

        _get_packet(&len, &pData);
        if( len == 0 )  break;

        if( len > UIP_BUFSIZE )
        {
            err("out buffer %d/%d\n", len, UIP_BUFSIZE);
            len = 0;
            break;
        }

        #if 0
        do {
            static int  g_cnt = 0;
            char        dump_name[64] = {0};
            FILE        *fout = 0;

            if( g_cnt > 3 )     break;

            snprintf(dump_name, sizeof(dump_name), "dump_package_%02d.bin", g_cnt++);
            if( (fout = fopen(dump_name, "wb")) )
            {
                fwrite(pData, 1, len, fout);
                fclose(fout);
            }
        } while(0);
        #endif

        memcpy(uip_buf, pData, len);
    } while(0);
    return len;
}

void
pcapdev_send(void)
{
    uint8_t     tmpbuf[UIP_BUFSIZE];
    int         i;

    for(i = 0; i < 40 + UIP_LLH_LEN; i++) {
        tmpbuf[i] = uip_buf[i];
    }

    for(; i < uip_len; i++) {
        tmpbuf[i] = ((uint8_t*)uip_appdata)[i - 40 - UIP_LLH_LEN];
    }

    printf("\n\n============== tx %d bytes\n", uip_len);
    for(i = 0; i < uip_len; i++)
    {
        if( i && !(i & 0xF) )
            printf("\n");
        printf("%02X ", tmpbuf[i]);
    }
    printf("\n\n");

    if( pcap_sendpacket(g_pAd_handle, tmpbuf, uip_len) != 0 )
    {
        err("\nError sending the packet: errcode = %s\n", pcap_geterr(g_pAd_handle));
//        return -12;
    }
    return;
}
