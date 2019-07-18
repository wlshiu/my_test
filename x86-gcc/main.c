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
#if 0
    #define MY_IP1        127
    #define MY_IP2        0
    #define MY_IP3        0
    #define MY_IP4        2
#else
    #define MY_IP1        192
    #define MY_IP2        168
    #define MY_IP3        56
    #define MY_IP4        1
#endif

#if 0
    #define MY_MAC1        0xaa
    #define MY_MAC2        0xbb
    #define MY_MAC3        0xcc
    #define MY_MAC4        0xdd
    #define MY_MAC5        0x11
    #define MY_MAC6        0x22
#else
    #define MY_MAC1        0x0a
    #define MY_MAC2        0x00
    #define MY_MAC3        0x27
    #define MY_MAC4        0x00
    #define MY_MAC5        0x00
    #define MY_MAC6        0x0e
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================
#define IS_ETH_TYPE(eth_type)       (((struct uip_eth_hdr *)&uip_buf[0])->type == uip_htons(eth_type))

#define IS_BROADCASE_PACKET()       (((struct uip_eth_hdr*)&uip_buf[0])->dest.addr[0] == 0xFF && \
                                     ((struct uip_eth_hdr*)&uip_buf[0])->dest.addr[1] == 0xFF && \
                                     ((struct uip_eth_hdr*)&uip_buf[0])->dest.addr[2] == 0xFF && \
                                     ((struct uip_eth_hdr*)&uip_buf[0])->dest.addr[3] == 0xFF && \
                                     ((struct uip_eth_hdr*)&uip_buf[0])->dest.addr[4] == 0xFF && \
                                     ((struct uip_eth_hdr*)&uip_buf[0])->dest.addr[5] == 0xFF)

#define debug(str, ...)         printf("[%s:%u] "str, __func__, __LINE__, ## __VA_ARGS__);

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
extern uint8_t  uip_buf[UIP_BUFSIZE + 2] __attribute__ ((aligned(4)));

//static struct timer    g_periodic_timer;
static struct timer    g_arp_timer;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(void)
{
    uip_ipaddr_t    ip;

//    timer_set(&g_periodic_timer, CLOCK_SECOND / 2);   /* 0.5s */
    timer_set(&g_arp_timer, CLOCK_SECOND * 10);	    /* 10s */

    /* Initialize the device driver. */
    pcapdev_init();

    /* Initialize the uIP TCP/IP stack. */
    uip_init();

    net_init();

    while(1) {
        net_proc();

        /* Let the pcapdev network device driver read an entire IP packet
           into the uip_buf. If it must wait for more than 0.5 seconds, it
           will return with the return value 0. If so, we know that it is
           time to call upon the uip_periodic(). Otherwise, the pcapdev has
           received an IP packet that is to be processed by uIP. */
        uip_len = pcapdev_read();

        if( uip_len == 0 ) {
            // send packet
        #if 0
            // TCP
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
        #endif

            #if UIP_UDP
            // UDP
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
        } else {
            // receive packet
            if( IS_ETH_TYPE(UIP_ETHTYPE_IP) ) {
                uip_arp_ipin();

                if( IS_BROADCASE_PACKET() )
                    debug("get broadcast packet\n");

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
