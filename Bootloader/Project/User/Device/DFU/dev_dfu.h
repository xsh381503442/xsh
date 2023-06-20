#ifndef __DEV_DFU_H__
#define __DEV_DFU_H__





#include <stdint.h>
#include <stdbool.h>
#include "board.h"

#define DEV_BLE_SYS_OFF_PIN            BSP_APOLLO2_DEV_BLE_SYS_OFF_PIN	




#ifdef LAYER_BOOTLOADER  

#define DEV_DFU_PROGRESS_STATUS_UPDATE_APOLLO2       1
#define DEV_DFU_PROGRESS_STATUS_UPDATE_NRF52832      2

typedef struct {
	uint8_t percentage;
	uint8_t status;
}_dev_dfu_progress;


#endif 


void dev_dfu_event_handle(uint32_t *p_data, uint32_t length);


#ifdef LAYER_UPDATEALGORITHM  
bool dev_dfu_update_algorithm_start(void);
#endif

#endif //__DEV_DFU_H__
