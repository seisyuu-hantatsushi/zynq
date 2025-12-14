
#ifndef __XEMAC_ADAPTER_HW_INTF_H__
#define __XEMAC_ADAPTER_HW_INTF_H__

#include <lwip/netif.h>
#include "netif/xemacpsif.h"

void hw_intf_init_emacps(xemacpsif_s *xemacps, struct netif *netif);


#endif /* __XEMAC_ADAPTER_HW_INTF_H__ */
