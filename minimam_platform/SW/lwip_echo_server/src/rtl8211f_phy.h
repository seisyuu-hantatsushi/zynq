#ifndef _RTL8211F_PHY_H_
#define _RTL8211F_PHY_H_

#include <stdint.h>
#include <lwip/err.h>
#include "xemacps.h"

err_t rtl8211f_phy_setup(XEmacPs *xemacpsp, uint32_t phy_addr);
uint32_t rtl8211f_phy_link_speed(XEmacPs *xemacpsp, uint32_t phyaddr);

#endif /* _RTL8211F_PHY_H_ */
