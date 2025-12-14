
#include <stdbool.h>
//#include "lwipopts.h"
#include "lwip/debug.h"
#include "xemac_adapter_phy.h"

#define PHY_BMCR				0x0000
#define PHY_DETECT_REG  						1
#define PHY_IDENTIFIER_1_REG					2
#define PHY_IDENTIFIER_2_REG					3
#define PHY_DETECT_MASK 					0x1808
#define PHY_MARVELL_IDENTIFIER				0x0141
#define PHY_TI_IDENTIFIER					0x2000
#define PHY_ADI_IDENTIFIER				0x0283
#define PHY_REALTEK_IDENTIFIER				0x001c
#define PHY_XILINX_PCS_PMA_ID1			0x0174
#define PHY_XILINX_PCS_PMA_ID2			0x0C00

#define XEMACPS_GMII2RGMII_SPEED1000_FD		0x140
#define XEMACPS_GMII2RGMII_SPEED100_FD		0x2100
#define XEMACPS_GMII2RGMII_SPEED10_FD		0x100
#define XEMACPS_GMII2RGMII_REG_NUM			0x10

#define PHY_REGCR		0x0D
#define PHY_ADDAR		0x0E
#define PHY_RGMIIDCTL	0x86
#define PHY_RGMIICTL	0x32
#define PHY_STS			0x11
#define PHY_TI_CR		0x10
#define PHY_TI_CTRL		0x1F
#define PHY_TI_CFG4		0x31

#define PHY_REGCR_ADDR	0x001F
#define PHY_REGCR_DATA	0x401F
#define PHY_TI_CRVAL	0x5048
#define PHY_TI_CFG4RESVDBIT7	0x80
#define PHY_TI_CFG4RESVDBIT8		0x100
#define PHY_TI_CFG4_AUTONEG_TIMER	0x60

#define PHY_TI_CTRL_SW_RESTART		0x4000
#define PHY_TI_BMCR_SW_RESET		0x8000

#define PHY_TI_PHYSTS_SPEED_SELECTION	0xC000
#define PHY_TI_PHYSTS_1000MBPS		0x8000
#define PHY_TI_PHYSTS_100MBPS		0x4000
#define SPEED_1000MBPS			1000
#define SPEED_100MBPS			100
#define SPEED_10MBPS			10

#define TI_PHY_CR_SGMII_EN		0x0800

/* Frequency setting */
#define SLCR_LOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR	(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR	(XPS_SYS_CTRL_BASEADDR + 0x144)
#define SLCR_GEM_SRCSEL_EMIO	0x40
#define SLCR_LOCK_KEY_VALUE 	0x767B
#define SLCR_UNLOCK_KEY_VALUE	0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL	(XPS_SYS_CTRL_BASEADDR + 0x214)
#define EMACPS_SLCR_DIV_MASK	0xFC0FC0FF

#define IEEE_CTRL_ISOLATE_DISABLE               0xFBFF

extern uint32_t phymapemac0[];
extern uint32_t phymapemac1[];

static bool phy_identify(XEmacPs *xemacpsp, uint32_t phy_addr, uint32_t emacnum)
{
    uint16_t phy_reg;
    uint16_t phy_id;
    bool find = false;
    
    XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_DETECT_REG,
                    &phy_reg);
    XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_IDENTIFIER_1_REG,
                    &phy_id);

    if (((phy_reg != 0xFFFF) &&
         ((phy_reg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) ||
        (phy_id == PHY_XILINX_PCS_PMA_ID1)) {
        /* Found a valid PHY address */
        LWIP_DEBUGF(NETIF_DEBUG,
                    ("XEmacPs detect_phy: PHY detected at address %d.\r\n", phy_addr));
        find = true;
        if (emacnum == 0) {
            phymapemac0[phy_addr] = TRUE;
        } else {
            phymapemac1[phy_addr] = TRUE;
        }
        
        XEmacPs_PhyRead(xemacpsp, phy_addr, PHY_IDENTIFIER_1_REG,
                        &phy_reg);
        if ((phy_reg != PHY_MARVELL_IDENTIFIER) &&
            (phy_reg != PHY_TI_IDENTIFIER) &&
            (phy_reg != PHY_REALTEK_IDENTIFIER) &&
            (phy_reg != PHY_ADI_IDENTIFIER)) {
            xil_printf("WARNING: Not a Marvell or TI or Realtek or Xilinx PCS PMA Ethernet PHY or ADI Ethernet PHY. Please verify the initialization sequence\r\n");
        }
    }
    return find;    
}

uint32_t xemac_adapter_detect_phy(XEmacPs *xemacpsp) {
    uint32_t phy_addr = 0;
    uint32_t emacnum;
    bool find = false;
    if (xemacpsp->Config.BaseAddress == XPAR_XEMACPS_0_BASEADDR)
        emacnum = 0;
    else
        emacnum = 1;
    
#ifdef SDT
    phy_addr = xemacpsp->Config.PhyAddr;
#endif

    if (phy_addr != 0) {
        find = phy_identify(xemacpsp, phy_addr, emacnum);
    } else {
        for (phy_addr = 31; phy_addr > 0; phy_addr--) {
            find = phy_identify(xemacpsp, phy_addr, emacnum);
            if (find) {
                break;
            }              
        }
    }

    if (find) {
        return phy_addr;
    }
    
    return UINT32_MAX;
}  
