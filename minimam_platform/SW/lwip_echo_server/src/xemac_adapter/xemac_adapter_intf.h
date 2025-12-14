#ifndef _XEMAC_ADAPTER_INTF_H_
#define _XEMAC_ADAPTER_INTF_H_

#include <lwip/err.h>
#include <lwip/netif.h>

err_t xemac_adapter_intf_init(struct netif *netif);
err_t xemac_adapter_intf_output(struct netif *netif, struct pbuf *p);

#endif /* _XEMAC_ADAPTER_INTF_H_ */
