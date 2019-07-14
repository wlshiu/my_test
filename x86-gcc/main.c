#include "uip-conf.h"
#include "uip.h"
#include "uip_arp.h"
#include "uip_timer.h"
#include "pcapdev.h"

#include "net.h"

#include <stdio.h>
#include <stdarg.h>

//=============================================================================
//                  Constant Definition
//=============================================================================
#if 1
    #define MY_IP1        127
    #define MY_IP2        0
    #define MY_IP3        0
    #define MY_IP4        2
#else
    #define MY_IP1        192
    #define MY_IP2        168
    #define MY_IP3        0
    #define MY_IP4        12
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================
#define IS_ETH_TYPE(eth_type)       (((struct uip_eth_hdr *)&uip_buf[0])->type == uip_htons(eth_type))


#define debug(str, ...)         printf("[%s:%u] "str, __func__, __LINE__, ## __VA_ARGS__);

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern uint8_t  uip_buf[UIP_BUFSIZE + 2] __attribute__ ((aligned(4)));

static struct timer    g_periodic_timer;
static struct timer    g_arp_timer;
//=============================================================================
//                  Private Function Definition
//=============================================================================
//#include "pkg.h"
//static void
//_sim_decode_pkg_prac()
//{
//    uip_ipaddr_t addr;
//    struct uip_udp_conn *conn;
//
//    uip_ipaddr(&addr, 127, 0, 0, 1);
//    conn = uip_udp_new(&addr, UIP_HTONS(69));
//
//    uip_len = __tftp_pkg_bin_len;
//    memcpy(uip_buf, __tftp_pkg_bin, uip_len);
//    uip_process(UIP_TIMER);
//    return;
//}
//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(void)
{
    uint8_t    i, arptimer;

    timer_set(&g_periodic_timer, CLOCK_SECOND / 2);   /* 0.5s */
    timer_set(&g_arp_timer, CLOCK_SECOND * 10);	    /* 10s */

    /* Initialize the device driver. */
    pcapdev_init();

    /* Initialize the uIP TCP/IP stack. */
    uip_init();
    uip_ipaddr_t    ip;

    uip_ipaddr(&ip, MY_IP1, MY_IP2, MY_IP3, MY_IP4);
    uip_sethostaddr(&ip);

    uip_ipaddr(&ip, 255, 255, 255, 0);
    uip_setnetmask(&ip);

//    uip_ipaddr(&ip, 192, 168, 99, 1);
//    uip_setdraddr(&ip);

#if 1
//    const uint8_t mac[] = {0xED, 0xAA, 0xE5, 0xB0, 0x8A, 0x77};
    const uint8_t           mac[] = {0xaa, 0xbb, 0xcc, 0xdd, 0x11, 0x22};
    struct uip_eth_addr     xAddr;
    xAddr.addr[0] = mac[0];
    xAddr.addr[1] = mac[1];
    xAddr.addr[2] = mac[2];
    xAddr.addr[3] = mac[3];
    xAddr.addr[4] = mac[4];
    xAddr.addr[5] = mac[5];

    uip_setethaddr(xAddr);
#endif // 0

    net_init();

//    uint16_t    port = 80;
//    uip_listen(UIP_HTONS(port));
//    uip_listen(port);

    arptimer = 0;
    while(1) {
        net_proc();

        /* Let the pcapdev network device driver read an entire IP packet
           into the uip_buf. If it must wait for more than 0.5 seconds, it
           will return with the return value 0. If so, we know that it is
           time to call upon the uip_periodic(). Otherwise, the pcapdev has
           received an IP packet that is to be processed by uIP. */
        #if 1
        uip_len = pcapdev_read();
        #else
        uip_len = __tftp_pkg_bin_len;
        memcpy(uip_buf, __tftp_pkg_bin, uip_len);
        #endif


#if 1
        if(uip_len > 0) {   /* received packet */
            struct uip_eth_hdr      *pEth_hdr = (struct uip_eth_hdr*)&uip_buf[0];
            if( IS_ETH_TYPE(UIP_ETHTYPE_IP) ) {    /* IP packet */
                uip_arp_ipin();
                if( pEth_hdr->dest.addr[0] == 0xff &&
                    pEth_hdr->dest.addr[1] == 0xff &&
                    pEth_hdr->dest.addr[2] == 0xff &&
                    pEth_hdr->dest.addr[3] == 0xff &&
                    pEth_hdr->dest.addr[4] == 0xff &&
                    pEth_hdr->dest.addr[5] == 0xff )
                    printf("get broadcast packet\n");

                uip_input();

                /**
                 *  If the above function invocation resulted in data that
                 *   should be sent out on the network, the global variable
                 *   uip_len is set to a value > 0.
                 */
                if(uip_len > 0) {
                    uip_arp_out();
                    pcapdev_send();
                }
            } else if( IS_ETH_TYPE(UIP_ETHTYPE_ARP) ) {    /*ARP packet */
                uip_arp_arpin();
                /* If the above function invocation resulted in data that
                    should be sent out on the network, the global variable
                    uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    pcapdev_send();  /* ARP ack*/
                }
            }
        } else if(timer_expired(&g_periodic_timer)) { /* no packet but periodic_timer time out (0.5s)*/
            timer_reset(&g_periodic_timer);

            for(int i = 0; i < UIP_CONNS; i++) {
                uip_periodic(i);
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    uip_arp_out();
                    pcapdev_send();
                }
            }
            #if UIP_UDP
            for(int i = 0; i < UIP_UDP_CONNS; i++) {
                uip_udp_periodic(i);
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    uip_arp_out();
                    pcapdev_send();
                }
            }
            #endif /* UIP_UDP */

            /* Call the ARP timer function every 10 seconds. */
            if(timer_expired(&g_arp_timer)) {
                timer_reset(&g_arp_timer);
                uip_arp_timer();
            }
        }
#else

        if(uip_len == 0) {
            for(i = 0; i < UIP_CONNS; i++) {
                uip_periodic(i);
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    uip_arp_out();
                    pcapdev_send();
                }
            }

            #if UIP_UDP
            for(i = 0; i < UIP_UDP_CONNS; i++) {
                uip_udp_periodic(i);
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    uip_arp_out();
                    pcapdev_send();
                }
            }
            #endif /* UIP_UDP */

            /* Call the ARP timer function every 10 seconds. */
            if(++arptimer == 20) {
                uip_arp_timer();
                arptimer = 0;
            }

        } else {
            if( IS_ETH_TYPE(UIP_ETHTYPE_IP) ) {
                uip_arp_ipin();
                uip_input();
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    uip_arp_out();
                    pcapdev_send();
                }
            } else if( IS_ETH_TYPE(UIP_ETHTYPE_ARP) ) {
                uip_arp_arpin();
                /* If the above function invocation resulted in data that
                   should be sent out on the network, the global variable
                   uip_len is set to a value > 0. */
                if(uip_len > 0) {
                    pcapdev_send();
                }
            }
        }
#endif // 1
    }

    return 0;
}
/*-----------------------------------------------------------------------------------*/
void
uip_log(char *m, const char *func, int line)
{
    printf("[%s:%u] uIP log: %s\n", func, line, m);
}
/*-----------------------------------------------------------------------------------*/

#if 0
int             g_udp_spoof = 0;
uip_ipaddr_t    g_udp_spoof_ip;
void enda_appcall(void)
{
    debug("appcall\n");
    uip_send("hello\n", 6);
}

void enda_udp_appcall(void)
{
    debug("udp appcall\n");
}
#endif
