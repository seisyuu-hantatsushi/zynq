#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "xparameters.h"
#include "ff.h"
#include "xdevcfg.h"
#include "xiicps.h"

#include "xil_printf.h"
#include "echo_server.h"

#define I2C_SCLK_RATE (400000) // 400KHz
#define I2C_RTC_SLAVE_ADDRESS (0x51)

XIicPs s_I2c0Instance;
struct time {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

static int32_t read_rtc(struct time *now){

    XIicPs_Config *pI2c0Cfg = NULL;
    uint8_t buf[10] = { 0 };
    int32_t status;
    
    pI2c0Cfg = XIicPs_LookupConfig(XPAR_XIICPS_0_BASEADDR);
    if(pI2c0Cfg == NULL){
        return -1;
    }

    status = XIicPs_CfgInitialize(&s_I2c0Instance, pI2c0Cfg, pI2c0Cfg->BaseAddress); 
    if(status != XST_SUCCESS){
        return -1;
    }

    status = XIicPs_SelfTest(&s_I2c0Instance);
    if(status != XST_SUCCESS){
        return -1;
    }

    status = XIicPs_SetSClk(&s_I2c0Instance, I2C_SCLK_RATE);
    if(status != XST_SUCCESS){
        return -1;
    }

    buf[0] = 0x02;
    status = XIicPs_MasterSendPolled(&s_I2c0Instance, &buf[0], 1, I2C_RTC_SLAVE_ADDRESS);
    if(status != XST_SUCCESS){
        return -1;
    }
    /* Wait until bus is idle to start another transfer */
    while (XIicPs_BusIsBusy(&s_I2c0Instance));

    status = XIicPs_MasterRecvPolled(&s_I2c0Instance, &buf[0], 7, I2C_RTC_SLAVE_ADDRESS);
    if(status != XST_SUCCESS){
        return -1;
    }
    
    now->sec   = ((buf[0] & 0x70) >> 4)*10 + (buf[0] & 0x0F);
    now->min   = ((buf[1] & 0x70) >> 4)*10 + (buf[1] & 0x0F);
    now->hour  = ((buf[2] & 0x70) >> 4)*10 + (buf[2] & 0x0F);
    now->day   = ((buf[3] & 0x70) >> 4)*10 + (buf[3] & 0x0F);
    now->month = ((buf[5] & 0x10) >> 4)*10 + (buf[5] & 0x0F);
    now->year  = ((buf[6] & 0xF0) >> 4)*10 + (buf[6] & 0x0F);
    xil_printf("now, %d:%d:%d:%d:%d:%d\r\n", now->year, now->month, now->day, now->hour, now->min, now->sec);
        
    return 0;
}

static const TCHAR* s_pSDDev="0:/";

static int init_fatds(FATFS *pFatfs){

    FRESULT rc;
    
    rc = f_mount(pFatfs, s_pSDDev, 0);
    if(rc != FR_OK){
        printf("failed to mount sd: %d\n", rc);
        return rc;
    }
    
    return FR_OK;
}

int SDFfunctuionTest(struct time *now){
    FATFS fatfs;
    FRESULT rc;
    char test_buf[256] = {'\0'};
    int write_size;
    
    rc = init_fatds(&fatfs);
    if(rc != FR_OK){
        return XST_FAILURE;
    }
    
    {
        DIR dir;
        FILINFO fno;
        rc = f_opendir(&dir, s_pSDDev);
        if(rc != FR_OK){
            goto error_exit;
        }
        
        while(FR_OK==f_readdir(&dir, &fno)){
            if(fno.fname[0] == '\0'){
                break;
            }
            printf("%s\n",fno.fname);	
        }
        f_closedir(&dir);
    }
    
    write_size = snprintf(test_buf, sizeof(test_buf)-1, "%d/%d/%d, %d:%d:%d", 2000+now->year, now->month, now->day, now->hour, now->min, now->sec);

    {
        FIL fil;
        UINT bw;
        f_open(&fil, "nowtime.txt", FA_CREATE_ALWAYS|FA_WRITE);
        f_lseek(&fil, 0);
        f_write(&fil, &test_buf[0],  write_size, &bw);
        f_close(&fil);
    }
    memset(test_buf, 0x00, sizeof(test_buf));
    
    {
        FIL fil;
        UINT bw;
        rc = f_open(&fil, "nowtime.txt", FA_READ);
        if(rc == FR_OK){
            f_lseek(&fil, 0);
            f_read(&fil, &test_buf[0], write_size, &bw);
            f_close(&fil);
            xil_printf("%s\r\n", &test_buf[0]);
        } else {
            xil_printf("failed to open nowtime.txt\r\n");
        }
    }

    
error_exit:

    f_unmount(s_pSDDev);
    return XST_SUCCESS;	
}

int main()
{
    struct time now = { 0 };
    read_rtc(&now);
    SDFfunctuionTest(&now);
    echo_server();    
    return 0;
}
