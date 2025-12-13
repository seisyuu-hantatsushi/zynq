
#include "xparameters_ps.h"
#include "ff.h"
#include "xdevcfg.h"
#include "xstatus.h"

#include <stdio.h>

static const TCHAR* s_pSDDev="0:/";

int init_fatds(FATFS *pFatfs){

	FRESULT rc;

	rc = f_mount(pFatfs, s_pSDDev, 0);
	if(rc != FR_OK){
		printf("failed to mount sd: %d\n", rc);
		return rc;
	}

	return FR_OK;
}

int SDFfunctuionTest(){
	FATFS fatfs;
	FRESULT rc;
	char test_buf[256] = {'\0'};

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

	{
		
	}
	
error_exit:

	f_unmount(s_pSDDev);
	return XST_SUCCESS;	
}


int main(void)
{

	printf("Start Peripheal test.\r\n");

	SDFfunctuionTest();

	printf("End Peripheal test.\r\n");

	return XST_SUCCESS;
}