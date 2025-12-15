#ifndef _XEMAC_ADAPTER_PHY_H_
#define _XEMAC_ADAPTER_PHY_H_

#include "xemacps.h"
#include "xemac_adapter.h"

uint32_t xemac_adapter_detect_phy(XEmacPs *xemacpsp);
uint32_t xemac_adapter_setup_phy(XEmacPs *xemacpsp, uint32_t phy_addr,
                                 struct xemac_adapter_context *adapter_context);

#endif /* _XEMAC_ADAPTER_PHY_H_ */
