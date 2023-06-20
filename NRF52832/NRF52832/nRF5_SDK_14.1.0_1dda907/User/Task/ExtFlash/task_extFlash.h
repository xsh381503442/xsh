#ifndef __TASK_EXTFLASH_H__
#define __TASK_EXTFLASH_H__

#include <stdint.h>

#define TASK_EXTFLASH_EVT_HWT                       				1
#define TASK_EXTFLASH_EVT_READ_RAW                  				2
#define TASK_EXTFLASH_EVT_WRITE_RAW                 				3
#define TASK_EXTFLASH_EVT_ERASE_RAW                 				4
#define TASK_EXTFLASH_EVT_BLE_DFU_BLE_POSTVALIDATE   				5
#define TASK_EXTFLASH_EVT_BLE_DFU_APOLLO_POSTVALIDATE       6
#define TASK_EXTFLASH_EVT_BLE_DFU_APOLLO_PREVALIDATE        7
#define TASK_EXTFLASH_EVT_BLE_DFU_BLE_PREVALIDATE           8
#ifdef COD 
#define TASK_EXTFLASH_EVT_OTA_ERASE_RAW								9
#define TASK_EXTFLASH_EVT_OTA_WRITE_RAW								10


#define TASK_EXTFLASH_EVT_COD_CMD_ON_GET_DATA_FRAME_NUM 			11
#define TASK_EXTFLASH_EVT_COD_CMD_ON_GET_DATA						12
void cod_sport_data_copy_send(uint8_t* buffer, uint32_t output_size);

#endif

void task_extFlash_init(void * p_context);
void task_extFlash_handle(uint32_t evt, uint32_t addr, uint8_t *p_data, uint32_t length);





#endif //__TASK_EXTFLASH_H__
