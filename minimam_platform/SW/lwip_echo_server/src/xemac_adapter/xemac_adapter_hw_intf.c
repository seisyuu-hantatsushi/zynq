
#include "netif/xemacpsif.h"
#include "xemac_adapter.h"
#include "xemac_adapter_phy.h"
#include "xemac_adapter_hw_intf.h"

extern XEmacPs_Config XEmacPs_ConfigTable[];
extern u32_t phymapemac0[32];
extern u32_t phymapemac1[32];
extern u32_t phyaddrforemac;

void hw_intf_init_emacps(xemacpsif_s *xemacps, struct netif *netif)
{
    struct xemac_adapter_context *adapter_context = (struct xemac_adapter_context *)(netif->state);
  
    XEmacPs *xemacpsp;
    s32_t status = XST_SUCCESS;
    u32_t i;
    u32_t phyfoundforemac0 = FALSE;
    u32_t phyfoundforemac1 = FALSE;
    u32_t gigeversion;
    u32_t Reg;
    
    xemacpsp = &xemacps->emacps;
    
    gigeversion = ((Xil_In32(xemacpsp->Config.BaseAddress + 0xFC)) >> 16) & 0xFFF;
    /* Get the number of queues */
    xemacpsp->MaxQueues = 1;
    if (gigeversion > 2) {
        Reg = XEmacPs_ReadReg(xemacpsp->Config.BaseAddress,
                              XEMACPS_DCFG6_OFFSET);
        xemacpsp->MaxQueues += get_num_set_bits(Reg & 0xFF);
    }
    
#ifdef ZYNQMP_USE_JUMBO
    XEmacPs_SetOptions(xemacpsp, XEMACPS_JUMBO_ENABLE_OPTION);
#endif
    
#ifdef LWIP_IGMP
    XEmacPs_SetOptions(xemacpsp, XEMACPS_MULTICAST_OPTION);
#endif
    
#ifdef SGMII_FIXED_LINK
    XEmacPs_SetOptions(xemacpsp, XEMACPS_SGMII_ENABLE_OPTION);
    status = XEmacPs_ReadReg(xemacpsp->Config.BaseAddress, XEMACPS_PCS_CONTROL_OFFSET);
    status &= ~XEMACPS_PCS_CON_AUTO_NEG_MASK;
    XEmacPs_WriteReg(xemacps->emacps.Config.BaseAddress, XEMACPS_PCS_CONTROL_OFFSET, status);
#endif
    
    /* set mac address */
    status = XEmacPs_SetMacAddress(xemacpsp, (void*)(netif->hwaddr), 1);
    if (status != XST_SUCCESS) {
        LWIP_DEBUGF(NETIF_DEBUG, ("In %s:Emac Mac Address set failed...\r\n",__func__));
    }
    
    XEmacPs_SetMdioDivisor(xemacpsp, MDC_DIV_224);
    
    /*  Please refer to file header comments for the file xemacpsif_physpeed.c
     *  to know more about the PHY programming sequence.
     *  For PCS PMA core, phy_setup_emacps is called with the predefined PHY address
     *  exposed through xaparemeters.h
     *  For RGMII case, assuming multiple PHYs can be present on the MDIO bus,
     *  detect_phy is called to get the addresses of the PHY present on
     *  a particular MDIO bus (emac0 or emac1). This address map is populated
     *  in phymapemac0 or phymapemac1.
     *  phy_setup_emacps is then called for each PHY present on the MDIO bus.
     */
#ifndef SGMII_FIXED_LINK
    adapter_context->phy.phyaddr = xemac_adapter_detect_phy(xemacpsp);
    if (xemacpsp->Config.BaseAddress == XPAR_XEMACPS_0_BASEADDR) {
        if (phymapemac0[adapter_context->phy.phyaddr] == TRUE) {
            MacConfig_SgmiiPcs(xemacpsp,adapter_context->phy.phyaddr);
            adapter_context->phy.link_speed = phy_setup_emacps(xemacpsp, adapter_context->phy.phyaddr);
            phyfoundforemac0 = TRUE;
            phyaddrforemac = adapter_context->phy.phyaddr;
            }
    } else {
        if (phymapemac1[adapter_context->phy.phyaddr] == TRUE) {
            MacConfig_SgmiiPcs(xemacpsp,adapter_context->phy.phyaddr);
            adapter_context->phy.link_speed = phy_setup_emacps(xemacpsp, adapter_context->phy.phyaddr);
            phyfoundforemac1 = TRUE;
            phyaddrforemac = adapter_context->phy.phyaddr;
        }
    }
    /* If no PHY was detected, use broadcast PHY address of 0 */
    if (xemacpsp->Config.BaseAddress == XPAR_XEMACPS_0_BASEADDR) {
        if (phyfoundforemac0 == FALSE)
            adapter_context->phy.link_speed = phy_setup_emacps(xemacpsp, 0);
    } else {
        if (phyfoundforemac1 == FALSE)
            adapter_context->phy.link_speed = phy_setup_emacps(xemacpsp, 0);
    }
#else
    link_speed = pcs_setup_emacps(xemacpsp);
#endif
    
    if (adapter_context->phy.link_speed == XST_FAILURE) {
        xemacps->eth_link_status = ETH_LINK_DOWN;
        xil_printf("Phy setup failure %s \n\r",__func__);
        return;
    } else {
        xemacps->eth_link_status = ETH_LINK_UP;
    }
    
    XEmacPs_SetOperatingSpeed(xemacpsp, adapter_context->phy.link_speed);
    /* Setting the operating speed of the MAC needs a delay. */
    {
        volatile s32_t wait;
        for (wait=0; wait < 20000; wait++);
    }
}

void hw_intf_init_emacps_on_error (xemacpsif_s *xemacps, struct netif *netif)
{
    XEmacPs *xemacpsp;
    s32_t status = XST_SUCCESS;
    struct xemac_adapter_context *adapter_context =
        (struct xemac_adapter_context *)(netif->state);
    
    xemacpsp = &xemacps->emacps;
    
    /* set mac address */
    status = XEmacPs_SetMacAddress(xemacpsp, (void*)(netif->hwaddr), 1);
    if (status != XST_SUCCESS) {
        LWIP_DEBUGF(NETIF_DEBUG, ("In %s:Emac Mac Address set failed...\r\n",__func__));
    }
    
    XEmacPs_SetOperatingSpeed(xemacpsp, adapter_context->phy.link_speed);
    
    /* Setting the operating speed of the MAC needs a delay. */
    {
        volatile s32_t wait;
        for (wait=0; wait < 20000; wait++);
    }
}
