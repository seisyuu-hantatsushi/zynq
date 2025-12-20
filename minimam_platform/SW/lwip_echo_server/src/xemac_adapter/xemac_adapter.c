
#include <stdio.h>
#include <string.h>

#include <xparameters.h>
#include "lwipopts.h"
#include "xlwipconfig.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/igmp.h"

#include "netif/etharp.h"
#include "netif/xadapter.h"
#include "netif/xpqueue.h"
#include "xscugic.h"

#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif

#include "xil_printf.h"

#include "xemac_adapter_intf.h"
#include "xemac_adapter.h"

/* Define those to better describe your network interface. */
#define IFNAME0 't'
#define IFNAME1 'e'

/*
 * xemacpsif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t  xemac_adapter_output(struct netif *netif, struct pbuf *p,
                                   const ip_addr_t *ipaddr)
{
    /* resolve hardware address, then send (or queue) packet */
     return etharp_output(netif, p, ipaddr);
}

err_t xemac_adapter_init(struct netif *netif){

#if LWIP_SNMP
    /* ifType ethernetCsmacd(6) @see RFC1213 */
    netif->link_type = 6;
    /* your link speed here */
    netif->link_speed = ;
    netif->ts = 0;
    netif->ifinoctets = 0;
    netif->ifinucastpkts = 0;
    netif->ifinnucastpkts = 0;
    netif->ifindiscargds = 0;
    netif->ifoutoctets = 0;
    netif->ifoutucastpkts = 0;
    netif->ifoutnucastpkts = 0;
    netif->ifoutdiscards = 0;
#endif
    
    netif->name[0]    = IFNAME0;
    netif->name[1]    = IFNAME1;
    netif->output     = xemac_adapter_output;
    netif->linkoutput = xemac_adapter_intf_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif
    
    xemac_adapter_intf_init(netif);
    return ERR_OK;  

}

int32_t xemac_adapter_input(struct netif *netif){
    struct xemac_adapter_context *adapter_context =
        (struct xemac_adapter_context *)netif->state;
    int n_packets = 0;
    
    switch (adapter_context->emac_type) {
    case xemac_type_emacps:
        n_packets = xemac_adapter_intf_input(netif);
        break;
    default:
        xil_printf("incorrect configuration: unknown temac type");
        while(1);
        return 0;
    }
    return n_packets;  
}    
