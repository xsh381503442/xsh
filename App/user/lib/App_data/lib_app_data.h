#ifndef __LIB_APP_DATA_H__
#define __LIB_APP_DATA_H__




#include <stdint.h>
#include <stdbool.h>



#define LIB_APP_DATA_RESET_STATUS_DFU_FINISH       1

typedef struct __attribute__ ((aligned(4))){
	uint32_t setting_version;
	uint32_t bootloader_version;
	uint32_t application_version;
	uint32_t softdevice_version;
	uint32_t pcb_version;
	uint32_t mac_address_f4b;
	uint32_t mac_address_l2b;
	uint8_t name[10+1];
	uint8_t name_len;	
	uint32_t build_date;
	uint32_t build_time;
	
}_lib_app_data_ble;

typedef struct __attribute__ ((aligned(4))){
	uint32_t crc;
	uint32_t reset_status;
	_lib_app_data_ble ble;
	uint32_t bat_adc;

}_lib_app_data;


void lib_app_data_erase(void);
void lib_app_data_write(_lib_app_data *lib_app_data);
void lib_app_data_init(void);
void lib_app_data_read(_lib_app_data *lib_app_data);
bool lib_app_data_crc_check_bootsetting(_lib_app_data *lib_app_data);
#endif //__LIB_APP_DATA_H__


