#ifndef __TASK_CENTRAL_H__
#define __TASK_CENTRAL_H__





#include <stdint.h>
#include <stdbool.h>
#include "ble_gap.h"



#define SCAN_DEV_NAME_LEN_MAX             10
#define SCAN_DEV_MAX                      3


typedef struct
{
  uint32_t interval;            /**< Scan interval between 0x0004 and 0x4000 in 0.625 ms units (2.5 ms to 10.24 s). */
  uint32_t window;              /**< Scan window between 0x0004 and 0x4000 in 0.625 ms units (2.5 ms to 10.24 s). */
  uint32_t timeout;             /**< Scan timeout between 0x0001 and 0xFFFF in seconds, 0x0000 disables timeout. */
	uint16_t uuid;
} _scan_param;



typedef struct{
	uint8_t name[SCAN_DEV_NAME_LEN_MAX + 1];
	uint8_t length;
} _scan_dev_name;


typedef struct{
	bool found;
	_scan_dev_name name;
	ble_gap_addr_t addr;	
	int8_t rssi;
} _scan_dev;


struct s_central_controller {
	uint32_t	command;
	_scan_param param;
	_scan_dev scan_dev[SCAN_DEV_MAX];
} ;


typedef struct s_central_controller _central_controller;

void task_central_handle(uint32_t evt, uint8_t *p_data, uint32_t length);
void task_central_init(void * p_context);



#endif //__TASK_CENTRAL_H__



