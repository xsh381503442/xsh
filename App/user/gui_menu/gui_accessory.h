#ifndef __GUI_ACCESSORY_H__
#define __GUI_ACCESSORY_H__



#include <stdint.h>
#include <stdbool.h>

#define ACCESSORY_STATUS_DISCONNECT       1
#define ACCESSORY_STATUS_CONNECT          2


typedef struct __attribute__((packed))
{
	uint8_t status;
	uint8_t mac[6];
	
	
}_ble_accessory_running;
typedef struct __attribute__((packed))
{
	uint8_t status;
	uint8_t mac[6];
	
	
}_ble_accessory_heartrate;
typedef struct __attribute__((packed))
{
	uint8_t status;
	uint8_t mac[6];
	
	
}_ble_accessory_cycling;



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
	
	
	
}_ble_accessory_data;

typedef struct __attribute__((packed))
{
	bool enable;
	_ble_accessory_running running;
	_ble_accessory_heartrate heartrate;
	_ble_accessory_cycling cycling;
	
	_ble_accessory_data data;
	
	
}_ble_accessory;








void gui_accessory_paint(void);
void gui_accessory_btn_evt(uint32_t evt);

void gui_accessory_heartrate_btn_evt(uint32_t evt);
void gui_accessory_heartrate_paint(void);


void gui_accessory_running_btn_evt(uint32_t evt);
void gui_accessory_running_paint(void);

void gui_accessory_cycling_wheel_paint(void);
void gui_accessory_cycling_wheel_btn_evt(uint32_t evt);

void gui_accessory_cycling_cadence_paint(void);
void gui_accessory_cycling_cadence_btn_evt(uint32_t evt);





#endif //__GUI_ACCESSORY_H__


