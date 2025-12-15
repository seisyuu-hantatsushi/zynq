
#include "sleep.h"
#include "xemac_ieee_reg.h"
#include "rtl8211f_phy.h"

//RTL8211FDI
#undef IEEE_SPECIFIC_STATUS_REG
#undef IEEE_SPEED_MASK
#undef IEEE_SPEED_1000
#undef IEEE_SPEED_100

#define IEEE_SPECIFIC_STATUS_REG        0X1A
#define IEEE_SPEED_MASK                 0x30
#define IEEE_SPEED_1000                 0x20
#define IEEE_SPEED_100                  0x10
//RTL8211FID

err_t rtl8211f_phy_setup(XEmacPs *xemacpsp, uint32_t phy_addr) {

#if 0 //bx71
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x1F, 0x0D08);
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x11, 0x0009);
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x1F, 0x0000);
#endif
#if 1
    // EEE LED Control function
    // switch to eth page 0xd04
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x1F, 0x0d04);
    
    // Read current EELCR value.
    uint16_t eeelcr_value;
    XEmacPs_PhyRead(xemacpsp, phy_addr, 0x11, &eeelcr_value);
    
    // Modify settings (e.g., disable the EEE indicator for LED1 and LED2).
    eeelcr_value &= ~(3 << 2);  // Clear the EEE Enable bit for LED1

    // Write back the modified value.
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x11, eeelcr_value);

    // Switch back to the default page.
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 31, 0x0000);
#endif


#if 1
    // Control whether LED1 and LED2 blink or stay on continuously.
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x1F, 0x0D04);
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x10, 0x617F);
    XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x1F, 0x0000);
#endif

    return ERR_OK;
}

uint32_t rtl8211f_phy_link_speed(XEmacPs *xemacpsp, uint32_t phy_addr) {
    uint16_t control;
    uint16_t status;
    uint16_t status_speed;
    uint32_t timeout_counter = 0;
    uint32_t temp_speed;

    xil_printf("Start PHY autonegotiation \r\n");

    XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
    control |= IEEE_ASYMMETRIC_PAUSE_MASK;
    control |= IEEE_PAUSE_MASK;
    control |= ADVERTISE_100;
    control |= ADVERTISE_10;
    XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);
    
    XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
                    &control);
    control |= ADVERTISE_1000;
    XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,
                     control);
    
    XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
    control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
    XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);
    
    XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    control |= IEEE_CTRL_RESET_MASK;
    XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);
    
    while (1) {
        XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
        if (control & IEEE_CTRL_RESET_MASK)
            continue;
        else
            break;
    }
    
    XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
    
    xil_printf("Waiting for PHY to complete autonegotiation.\r\n");
    
    while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
        sleep(1);
        timeout_counter++;
        
        if (timeout_counter == 5) {
            xil_printf("Auto negotiation error \r\n");
            return XST_FAILURE;
        }
        XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
    }
    xil_printf("autonegotiation complete \r\n");
    XEmacPs_PhyRead(xemacpsp, phy_addr,IEEE_SPECIFIC_STATUS_REG,
                    &status_speed);
    if (status_speed & 0x4) {
        temp_speed = status_speed & IEEE_SPEED_MASK;
        
        if (temp_speed == IEEE_SPEED_1000)
            return 1000;
        else if(temp_speed == IEEE_SPEED_100)
            return 100;
        else
            return 10;
    }
    
    return XST_FAILURE;  
}  
