#ifndef _XEMAC_ADAPTER_H_
#define _XEMAC_ADAPTER_H_

#include <lwip/err.h>
#include <lwip/netif.h>
#include "netif/xemacpsif.h"

struct xemac_adapter_phy {
    uint32_t phyaddr;
    uint32_t link_speed;
    err_t (*phy_setup)(XEmacPs *xemacpsp, uint32_t phyaddr);
    uint32_t (*phy_link_speed)(XEmacPs *xemacpsp, uint32_t phyaddr);
};

struct xemac_adapter_context {
    struct netif *netif;
    struct xemac_adapter_phy phy;
    uint8_t macaddr[6];
    void *mac_regaddr;
    xemacpsif_s xemacpsif;
#if !NO_SYS
    sys_sem_t sem_rx_data_available;
#if defined(__arm__) && !defined(ARMR5)
    TimerHandle_t xTimer;
#endif
#endif
    XEmacPs_Config *mac_config;
};

err_t xemac_adapter_init(struct netif *netif);

#endif /* _XEMAC_ADAPTER_H_ */
