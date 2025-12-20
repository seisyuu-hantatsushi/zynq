
#include <stdio.h>
#include <string.h>

#include <xparameters.h>
#include "lwipopts.h"
#include "xlwipconfig.h"
#include "lwip/opt.h"

#include "lwip/pbuf.h"

#include "netif/etharp.h"
#include "netif/xadapter.h"
#include "netif/xpqueue.h"

#include "xemac_dma.h"
#include "xemac_adapter_hw_intf.h"
#include "xemac_adapter.h"

/*
 * this function is always called with interrupts off
 * this function also assumes that there are available BD's
 */
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
static err_t _unbuffered_low_level_output(xemacpsif_s *xemacpsif,
                                          struct pbuf *p, u32_t block_till_tx_complete, u32_t *to_block_index )
#else
static err_t _unbuffered_low_level_output(xemacpsif_s *xemacpsif,
                                          struct pbuf *p)
#endif
{
    XStatus status = 0;
    err_t err = ERR_MEM;
    
#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE);	/* drop the padding word */
#endif
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
    if (block_till_tx_complete == 1) {
        status = emacps_sgsend(xemacpsif, p, 1, to_block_index);
    } else {
        status = emacps_sgsend(xemacpsif, p, 0, to_block_index);
    }
#else
    status = emacps_sgsend(xemacpsif, p);
#endif
    if (status != XST_SUCCESS) {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
    } else {
        err = ERR_OK;
    }
    
#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE);	/* reclaim the padding word */
#endif
    
#if LINK_STATS
    lwip_stats.link.xmit++;
#endif /* LINK_STATS */
    
    return err;
    
}

err_t xemac_adapter_intf_init(struct netif *netif) {
    struct xemac_adapter_context *adapter_context =
        (struct xemac_adapter_context *)netif->state;
    UINTPTR mac_baseaddr = (UINTPTR)(adapter_context->mac_regaddr);
    xemacpsif_s *xemacpsif = &adapter_context->xemacpsif;
    s32_t status = XST_SUCCESS;
    UINTPTR dmacrreg;

    adapter_context->emac_type = xemac_type_emacps;
    memcpy(&netif->hwaddr[0], &adapter_context->macaddr[0], 6);
    
    xemacpsif->send_q = NULL;
    xemacpsif->recv_q = pq_create_queue();
    if (xemacpsif->recv_q == NULL) {
        return ERR_MEM;
    }

    /* maximum transfer unit */
#ifdef ZYNQMP_USE_JUMBO
    netif->mtu = XEMACPS_MTU_JUMBO - XEMACPS_HDR_SIZE;
#else
    netif->mtu = XEMACPS_MTU - XEMACPS_HDR_SIZE;
#endif
    
#if LWIP_IGMP
    netif->igmp_mac_filter = xemacpsif_mac_filter_update;
#endif
    
#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif->mld_mac_filter = xemacpsif_mld6_mac_filter_update;
#endif
    
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    
#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif
        
#if LWIP_IGMP
    netif->flags |= NETIF_FLAG_IGMP;
#endif

#if !NO_SYS
    sys_sem_new(&adapter_context->sem_rx_data_available, 0);
#endif

    /* obtain config of this emac */
    adapter_context->mac_config = xemacps_lookup_config(mac_baseaddr);

#if defined (__aarch64__) && (EL1_NONSECURE == 1)
    /* Request device to indicate that this library is using it */
    if (mac_config->BaseAddress == VERSAL_EMACPS_0_BASEADDR) {
        Xil_Smc(PM_REQUEST_DEVICE_SMC_FID, DEV_GEM_0, 1, 0, 100, 1, 0, 0);
    }
    if (mac_config->BaseAddress == VERSAL_EMACPS_0_BASEADDR) {
        Xil_Smc(PM_REQUEST_DEVICE_SMC_FID, DEV_GEM_1, 1, 0, 100, 1, 0, 0);
    }
#endif

    status = XEmacPs_CfgInitialize(&xemacpsif->emacps, adapter_context->mac_config,
                                   adapter_context->mac_config->BaseAddress);
    if (status != XST_SUCCESS) {
        LWIP_DEBUGF(NETIF_DEBUG, ("In %s:EmacPs Configuration Failed....\r\n", __func__));
    }
    
    /* initialize the mac */
    hw_intf_init_emacps(xemacpsif, netif);

    dmacrreg = XEmacPs_ReadReg(xemacpsif->emacps.Config.BaseAddress,
                               XEMACPS_DMACR_OFFSET);
    dmacrreg = dmacrreg | (0x00000010UL);
    XEmacPs_WriteReg(xemacpsif->emacps.Config.BaseAddress,
                     XEMACPS_DMACR_OFFSET, dmacrreg);
#if !NO_SYS
#if defined(__arm__) && !defined(ARMR5)
    /* Freertos tick is 10ms by default; set period to the same */
    xemac->xTimer = xTimerCreate("Timer", 10, pdTRUE, ( void * ) 1, vTimerCallback);
    if (xemac->xTimer == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("In %s:Timer creation failed....\r\n", __func__));
    } else {
        if(xTimerStart(xemac->xTimer, 0) != pdPASS) {
            LWIP_DEBUGF(NETIF_DEBUG, ("In %s:Timer start failed....\r\n", __func__));
        }
    }
#endif
#endif

    hw_intf_setup_isr(adapter_context);
    hw_intf_init_dma(adapter_context);
    start_emacps(xemacpsif);

    adapter_context->netif = netif;
    
    return ERR_OK;
}

err_t xemac_adapter_intf_output(struct netif *netif, struct pbuf *p) {
    struct xemac_adapter_context *adapter_context = (struct xemac_adapter_context *)netif->state;
    err_t err = ERR_MEM;
    s32_t freecnt;
    XEmacPs_BdRing *txring;
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
    u32_t notfifyblocksleepcntr;
    u32_t to_block_index;
#endif
    xil_printf(" xemac_adapter_intf_output\r\n");

    SYS_ARCH_DECL_PROTECT(lev);
    xemacpsif_s *xemacpsif = (xemacpsif_s *)(&adapter_context->xemacpsif);
    freecnt = xemacps_is_tx_space_available(xemacpsif);
    SYS_ARCH_PROTECT(lev);
    if (freecnt <= 5) {
	txring = &(XEmacPs_GetTxRing(&xemacpsif->emacps));
        xemacps_process_sent_bds(xemacpsif, txring);
    }
    if (xemacps_is_tx_space_available(xemacpsif)) {    
#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
        if (netif_is_opt_block_tx_set(netif, NETIF_ENABLE_BLOCKING_TX_FOR_PACKET)) {
            err = _unbuffered_low_level_output(xemacpsif, p, 1, &to_block_index);
        } else {
            err = _unbuffered_low_level_output(xemacpsif, p, 0, &to_block_index);
        }
#else
        err = _unbuffered_low_level_output(xemacpsif, p);
#endif
    } else {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
        xil_printf("pack dropped, no space\r\n");
        SYS_ARCH_UNPROTECT(lev);
        goto return_pack_dropped;
    }
    SYS_ARCH_UNPROTECT(lev);

#if LWIP_UDP_OPT_BLOCK_TX_TILL_COMPLETE
    if (netif_is_opt_block_tx_set(netif, NETIF_ENABLE_BLOCKING_TX_FOR_PACKET)) {
        /* Wait for approx 1 second before timing out */
        notfifyblocksleepcntr = 900000;
        while(notifyinfo[to_block_index] == 1) {
            usleep(1);
            notfifyblocksleepcntr--;
            if (notfifyblocksleepcntr <= 0) {
                err = ERR_TIMEOUT;
                break;
            }
        }
    }
    netif_clear_opt_block_tx(netif, NETIF_ENABLE_BLOCKING_TX_FOR_PACKET);
#endif
    
return_pack_dropped:
    return err;    
}

int32_t xemac_adapter_intf_input(struct netif *netif){
    struct xemac_adapter_context *adapter_context =
        (struct xemac_adapter_context *)netif->state;
    struct eth_hdr *ethhdr;
    struct pbuf *p;
    SYS_ARCH_DECL_PROTECT(lev);

#if !NO_SYS
    while (1)
#endif
        {
            /* move received packet into a new pbuf */
            SYS_ARCH_PROTECT(lev);
            p = hw_intf_input(adapter_context);
            SYS_ARCH_UNPROTECT(lev);
            
            /* no packet could be read, silently ignore this */
            if (p == NULL) {
                return 0;
            }
            
            /* points to packet payload, which starts with an Ethernet header */
            ethhdr = p->payload;

#if LINK_STATS
            lwip_stats.link.recv++;
#endif /* LINK_STATS */

            switch (htons(ethhdr->type)) {
                /* IP or ARP packet? */
            case ETHTYPE_IP:
            case ETHTYPE_ARP:
#if LWIP_IPV6
                /*IPv6 Packet?*/
            case ETHTYPE_IPV6:
#endif
#if PPPOE_SUPPORT
g                /* PPPoE packet? */
            case ETHTYPE_PPPOEDISC:
            case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
                /* full packet send to tcpip_thread to process */
                if (netif->input(p, netif) != ERR_OK) {
                    LWIP_DEBUGF(NETIF_DEBUG, ("xemacpsif_input: IP input error\r\n"));
                    pbuf_free(p);
                    p = NULL;
                }
                break;
                
            default:
                pbuf_free(p);
                p = NULL;
                break;
            }
	}
    
    return 1;

}

