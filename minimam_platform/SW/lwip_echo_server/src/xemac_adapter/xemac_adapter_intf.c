
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

err_t xemac_adapter_intf_init(struct netif *netif) {
    struct xemac_adapter_context *adapter_context =
        (struct xemac_adapter_context *)netif->state;
    UINTPTR mac_baseaddr = (UINTPTR)(adapter_context->mac_regaddr);
    xemacpsif_s *xemacpsif = &adapter_context->xemacpsif;
    s32_t status = XST_SUCCESS;
    UINTPTR dmacrreg;
    
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
    return ERR_OK;
}

struct pbuf* xemac_adapter_intf_input(struct netif *netif)
{
    struct xemac_s *xemac = (struct xemac_s *)(netif->state);
    xemacpsif_s *xemacpsif = (xemacpsif_s *)(xemac->state);
    struct pbuf *p;
    
    /* see if there is data to process */
    if (pq_qlength(xemacpsif->recv_q) == 0)
        return NULL;
    
    /* return one packet from receive q */
    p = (struct pbuf *)pq_dequeue(xemacpsif->recv_q);
    return p;
}
