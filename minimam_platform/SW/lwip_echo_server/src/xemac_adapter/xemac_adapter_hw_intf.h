
#ifndef __XEMAC_ADAPTER_HW_INTF_H__
#define __XEMAC_ADAPTER_HW_INTF_H__

#include <lwip/netif.h>
#include "xemac_adapter.h"
#include "netif/xemacpsif.h"

void hw_intf_init_emacps(xemacpsif_s *xemacps, struct netif *netif);
void hw_intf_init_emacps_on_error(xemacpsif_s *xemacps, struct netif *netif);
void hw_intf_setup_isr(struct xemac_adapter_context *adapter);
void hw_intf_error_handler(void *arg, u8 Direction, u32 ErrorWord);

struct pbuf* hw_intf_input(struct xemac_adapter_context *adapter);

#endif /* __XEMAC_ADAPTER_HW_INTF_H__ */
