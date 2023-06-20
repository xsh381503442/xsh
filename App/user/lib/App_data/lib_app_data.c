#include "lib_app_data.h"
#include "drv_intFlash.h"
#include "am_bootloader.h"
#include <string.h>
#include "lib_boot_setting.h"




void lib_app_data_init(void)
{
	_lib_app_data lib_app_data;
	memset(&lib_app_data,0xFF,sizeof(_lib_app_data));
	lib_app_data_write(&lib_app_data);
}

void lib_app_data_read(_lib_app_data *lib_app_data)
{
	memcpy((void*)lib_app_data, (uint8_t *)APP_DATA_ADDRESS, sizeof(_lib_app_data));
}
void lib_app_data_erase(void)
{
	drv_intFlash_erase_page(APP_DATA_ADDRESS);
}

void lib_app_data_write(_lib_app_data *lib_app_data)
{
	drv_intFlash_erase_page(APP_DATA_ADDRESS);
	drv_intFlash_write_page(APP_DATA_ADDRESS,(uint32_t *)lib_app_data,sizeof(_lib_app_data)/4);
}
bool lib_app_data_crc_check_bootsetting(_lib_app_data *lib_app_data)
{
	uint32_t crc = am_bootloader_fast_crc32((uint8_t*)(&lib_app_data->crc)+sizeof(lib_app_data->crc),sizeof(_lib_app_data)-sizeof(lib_app_data->crc));

	if(crc == lib_app_data->crc)
	{
		return true;
	}
	else
	{
		return false;
	}	
	
}



