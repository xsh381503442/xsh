#ifndef TASK_BLE_ACCESSARY_H
#define TASK_BLE_ACCESSARY_H

#include <stdint.h>
#include <stdbool.h>
#include "ble_srv_common.h"


#define TASK_BLE_ACCESSORY_EVT_ADV_REPORT                      1
#define TASK_BLE_ACCESSORY_EVT_SCAN                            2
#define TASK_BLE_ACCESSORY_EVT_SCAN_ENABLE                     3
#define TASK_BLE_ACCESSORY_EVT_HEARTRATE_SRV_DISC              4
#define TASK_BLE_ACCESSORY_EVT_RUNNING_SRV_DISC                5
#define TASK_BLE_ACCESSORY_EVT_CYCLING_SRV_DISC                6
#define TASK_BLE_ACCESSORY_EVT_SCAN_DISABLE                    7
#define TASK_BLE_ACCESSORY_EVT_DATA_TIMER_START                8
#define TASK_BLE_ACCESSORY_EVT_DATA_TIMER_STOP                 9


#define ACCESSORY_NAME_MAX       10

typedef struct
{
	uint16_t conn_handle;
	ble_gap_addr_t addr;
	uint8_t name[ACCESSORY_NAME_MAX+1];
	uint16_t uuid;
	bool uuid_found;
	bool name_found;
}_accessory_info;


typedef struct
{
	_accessory_info info;
	bool data_valid;
	bool     is_inst_stride_len_present;             /**< True if Instantaneous Stride Length is present in the measurement. */
	bool     is_total_distance_present;              /**< True if Total Distance is present in the measurement. */
	bool     is_running;                             /**< True if running, False if walking. */
	uint16_t inst_speed;                             /**< Instantaneous Speed. */
	uint8_t  inst_cadence;                           /**< Instantaneous Cadence. */
	uint16_t inst_stride_length;                     /**< Instantaneous Stride Length. */
	uint32_t total_distance;                         /**< Total Distance. */	
	
}_accessory_running;

typedef struct
{
	_accessory_info info;
	bool data_valid;
	uint16_t hr_value; 
	
}_accessory_heartrate;

typedef struct
{
	_accessory_info info;
	bool data_valid;
	bool     is_wheel_revolution_data_present; 
	bool     is_crank_revolution_data_present; 
	uint32_t cumulative_wheel_revolutions;
	uint16_t last_wheel_event_time;
	uint16_t cumulative_crank_revolutions;
	uint16_t last_crank_event_time;		
	
	
}_accessory_cycling;



typedef struct __attribute__((packed))
{
	uint32_t command;
	bool hr_data_valid;
	uint16_t hr_value; 	
	
	bool run_data_valid;
	bool     run_is_inst_stride_len_present;             /**< True if Instantaneous Stride Length is present in the measurement. */
	bool     run_is_total_distance_present;              /**< True if Total Distance is present in the measurement. */
	bool     run_is_running;                             /**< True if running, False if walking. */
	uint16_t run_inst_speed;                             /**< Instantaneous Speed. */
	uint8_t  run_inst_cadence;                           /**< Instantaneous Cadence. */
	uint16_t run_inst_stride_length;                     /**< Instantaneous Stride Length. */
	uint32_t run_total_distance;                         /**< Total Distance. */		
	
	bool cycle_data_valid;
	bool     cycle_is_wheel_revolution_data_present; 
	bool     cycle_is_crank_revolution_data_present; 
	uint32_t cycle_cumulative_wheel_revolutions;
	uint16_t cycle_last_wheel_event_time;
	uint16_t cycle_cumulative_crank_revolutions;
	uint16_t cycle_last_crank_event_time;		
	
	
	
}_task_ble_accessory_data;


void task_ble_accessory_scan_start(void);

void task_ble_accessary_handle(uint32_t evt, uint8_t *p_data, uint32_t length);
void task_ble_accessary_init(void * p_context);

#endif
