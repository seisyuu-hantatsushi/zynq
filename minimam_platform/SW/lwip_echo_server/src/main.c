/*
 * Copyright (C) 2009 - 2022 Xilinx, Inc.
 * Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <stdio.h>

#include "lwip/def.h"
#include "lwip/netif.h"
#include "xparameters.h"

#include "netif/xadapter.h"

#include "platform.h"
#include "platform_config.h"
#if defined (__arm__) || defined(__aarch64__)
#include "xil_printf.h"
#endif

#include "rtl8211f_phy.h"
#include "netif/ethernet.h"
#include "lwip/tcp.h"
#include "xil_cache.h"

#if LWIP_IPV6==1
#include "lwip/ip.h"
#else
#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif
#endif

#include "xemac_adapter/xemac_adapter.h"

/* defined by each RAW mode application */
void print_app_header();
int start_application();
int transfer_data();
void tcp_fasttmr(void);
void tcp_slowtmr(void);

/* missing declaration in lwIP */
void lwip_init();

#if LWIP_IPV6==0
#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *echo_netif;

#if LWIP_IPV6==1
void print_ip6(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf(" %x:%x:%x:%x:%x:%x:%x:%x\n\r",
		   IP6_ADDR_BLOCK1(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK2(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK3(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK4(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK5(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK6(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK7(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK8(&ip->u_addr.ip6));

}
#else
void print_ip(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
		   ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}
#endif

/* the mac address of the board. this should be unique per board */
#define MAC_ADDRESS { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 }
const static uint32_t default_ipv4_addr = LWIP_MAKEU32(192, 168, 10, 91);
const static uint32_t default_ipv4_netmask = LWIP_MAKEU32(255, 255, 255, 0);
const static uint32_t default_ipv4_gw = LWIP_MAKEU32(192, 168, 10, 1);

int main()
{
    struct xemac_adapter_context pseth_context = {
        .netif = NULL,
        .phy = {
            .phyaddr = 0,
            .phy_setup = rtl8211f_phy_setup,
            .phy_link_speed = rtl8211f_phy_link_speed
        },
        .macaddr = MAC_ADDRESS,
        .mac_regaddr = (void *)PLATFORM_EMAC_BASEADDR
    };
    
#if LWIP_IPV6==0
    ip_addr_t ipaddr, netmask, gw;
    
#endif
    echo_netif = &server_netif;
    
    /* Define this board specific macro in order perform PHY reset on ZCU102 */
    
    init_platform();
    
#if LWIP_IPV6==0
#if LWIP_DHCP==1
    ipaddr.addr = 0;
    gw.addr = 0;
    netmask.addr = 0;
#else
    /* initialize IP addresses to be used */
    ip4_addr_set_u32(&ipaddr,  PP_HTONL(default_ipv4_addr));
    ip4_addr_set_u32(&netmask, PP_HTONL(default_ipv4_netmask));
    ip4_addr_set_u32(&gw,      PP_HTONL(default_ipv4_gw));
#endif
#endif
    print_app_header();
    
    lwip_init();
    
#if (LWIP_IPV6 == 0)
    /* Add network interface to the netif_list, and set it as default */
    echo_netif = netif_add(echo_netif, &ipaddr, &netmask, &gw, &pseth_context, xemac_adapter_init, ethernet_input);
#else
    /* Add network interface to the netif_list, and set it as default */
    if (!xemac_add(echo_netif, NULL, NULL, NULL, mac_ethernet_address,
                   PLATFORM_EMAC_BASEADDR)) {
        xil_printf("Error adding N/W interface\n\r");
        return -1;
    }
    echo_netif->ip6_autoconfig_enabled = 1;
        
    netif_create_ip6_linklocal_address(echo_netif, 1);
    netif_ip6_addr_set_state(echo_netif, 0, IP6_ADDR_VALID);
    
    print_ip6("\n\rBoard IPv6 address ", &echo_netif->ip6_addr[0].u_addr.ip6);
    
#endif
    netif_set_default(echo_netif);
    
#ifndef SDT
    /* now enable interrupts */
    platform_enable_interrupts();
#endif
    
    /* specify that the network if is up */
    netif_set_up(echo_netif);
    
#if (LWIP_IPV6 == 0)
#if (LWIP_DHCP==1)
    /* Create a new DHCP client for this interface.
     * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() atxbg
     * the predefined regular intervals after starting the client.
     */
    dhcp_start(echo_netif);
    dhcp_timoutcntr = 240;
    
    while (((echo_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0)) {
      // xemacif_input(echo_netif);
        xemac_adapter_input(echo_netif);
    }
    
    if (dhcp_timoutcntr <= 0) {
      if ((echo_netif->ip_addr.addr) == 0) {
            ip4_addr_set_u32(&ipaddr,  PP_HTONL(default_ipv4_addr));
            ip4_addr_set_u32(&netmask, PP_HTONL(default_ipv4_netmask));
            ip4_addr_set_u32(&gw,      PP_HTONL(default_ipv4_gw));                    
            xil_printf("DHCP Timeout\r\n");
            xil_printf("Configuring default IP of %d.%d.%d.%d.\r\n",
                       ip4_addr1_val(ipaddr), ip4_addr2_val(ipaddr), ip4_addr3_val(ipaddr), ip4_addr4_val(ipaddr));
        }
    }
    
    ipaddr.addr = echo_netif->ip_addr.addr;
    gw.addr = echo_netif->gw.addr;
    netmask.addr = echo_netif->netmask.addr;
#endif
    
    print_ip_settings(&ipaddr, &netmask, &gw);
    
#endif
    /* start the application (web server, rxtest, txtest, etc..) */
    start_application();

    /* receive and process packets */
    while (1) {
        if (TcpFastTmrFlag) {
            tcp_fasttmr();
            TcpFastTmrFlag = 0;
        }
        if (TcpSlowTmrFlag) {
            tcp_slowtmr();
            TcpSlowTmrFlag = 0;
        }
        xemac_adapter_input(echo_netif);
        transfer_data();
    }

    /* never reached */
    cleanup_platform();
    
    return 0;
}
