#include "am_mcu_apollo.h"
#include "am_util.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"
#include <string.h>

#include "drv_config.h"
#include "drv_ublox.h"
#include "drv_ble.h"
#include "drv_pc.h"
#include "drv_battery.h"
#include "drv_extFlash.h"
#include "drv_light.h"


#include "cmd_pc.h"
#include "cmd_ble.h"

#include "lib_boot_setting.h"
#include "lib_boot.h"
#include "lib_app_data.h"
#include "lib_error.h"
#include "lib_utf8_gbk.h"
#include "lib_error.h"

#include "com_sport.h"
#include "com_data.h"

#include "time_notify.h"

#include "gui_tool_calendar.h"
#include "gui_accessory.h"
#include "gui_tool_findphone.h"
#include "com_dial.h"
#include "gui_theme.h"
#include "gui_notify_warning.h"


#include "algo_time_calibration.h"

#include "task_ble.h"
#include "task.h"
#include "task_config.h"
#include "task_tool.h"
#include "watch_config.h"
#include "task_sport.h"
#include "task_step.h"
#include "mode_power_off.h"
#include "gui_dfu.h"


#if DEBUG_ENABLED == 1 && TASK_BLE_LOG_ENABLED == 1
	#define TASK_BLE_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_BLE_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define TASK_BLE_LOG_WRITESTRING(...)
	#define TASK_BLE_LOG_PRINTF(...)		        
#endif



#define DELAY_US(X)                 am_util_delay_us(X)
//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

#ifdef COD
#include "timer_app.h"
#include "drv_lsm6dsl.h"
#include "task_hrt.h"
#include "com_ringbuffer.h"

extern _drv_bat_event   g_bat_evt;
cod_ble_sport_cast cod_user_sport_cast;
_cod_user_training_cast cod_user_training_cast;
_cod_user_sensor_req cod_user_sensor_req = {0};
extern void delete_sport_data(void);

extern uint8_t cod_uart_normal_open;

static uint8_t cod_sensor_acc_buffer[128];   //存储咕咚六轴数据缓存
struct ring_buffer_header cod_sensor_acc_rb; //存储咕咚六轴数据环型队列

static uint8_t cod_sensor_gyro_buffer[128];   //存储咕咚六轴数据缓存
struct ring_buffer_header cod_sensor_gyro_rb; //存储咕咚六轴数据环型队列

uint8_t cod_sensor_first_flag = 0;
uint32_t acc_num;
uint32_t gyro_num;

uint8_t cod_sport_cast_flag = 0;//投屏状态标志

#endif
extern _ble_accessory g_ble_accessory;
extern SetValueStr SetValue;
extern am_hal_rtc_time_t RTC_time;
static TaskHandle_t task_handle;
EventGroupHandle_t evt_handle_ble;
static bool m_initialized = false;
#define JUMP_BOOTLOADER               1
static uint8_t m_event = 0;
uint8_t get_action_name[10];
uint16_t get_action_name_len;
volatile ScreenState_t save_vice_detail_index;
extern uint8_t sport_cast_pause_index;

#ifdef COD 
uint8_t vice_status = VICE_SPORT_ING;

uint8_t vice_train_status = VICE_TRAIN_ING;
uint32_t real_time_log_flag =0;
#endif
	
typedef struct{
	uint32_t *p_data;
	uint32_t length;
} _task_buffer;
static _task_buffer m_task_buffer = {NULL,0};

typedef struct __attribute__ ((packed))
{
	uint32_t command;
	uint8_t addr[BLE_GAP_ADDR_LEN];	
} _task_ble_conn_evt;
uint8_t g_peer_addr[BLE_GAP_ADDR_LEN];	

typedef struct __attribute__ ((packed))
{
	uint32_t command;
	uint8_t disconnect_reason;

} _task_ble_conn_evt_disconnect;
uint8_t g_ble_disconnect_reason;	

typedef struct __attribute__ ((packed))
{
	uint32_t command;
	uint8_t name[BLE_CONFIG_DEVICE_NAME_LEN_MAX+1];
	uint8_t len;	
} _task_ble_dev_name_update;




typedef struct __attribute__ ((packed)){
	uint32_t command;
	uint32_t crc;
	uint8_t year;
	uint8_t month;
	uint8_t date;
	uint8_t day_of_week;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	int8_t time_zone;
}_ble_current_time;
typedef struct __attribute__ ((packed))
{
	uint32_t  command;
	uint32_t  crc;
	uint8_t		Sex;						//性别  0女  1 男 //APP设置
	uint8_t		Age;						//年龄 //APP设置
	uint8_t		Height;						//身高  单位：cm //APP设置
	uint8_t		Weight;						//体重  单位：kg //APP设置
	uint8_t		Stride;						//步长(日常徒步步长) 单位：cm //APP设置
	uint16_t	VO2Max;						//最大摄氧量 单位：ml/kg/min //APP设置
	uint16_t  LactateThresholdSpeed;    //乳酸阈速度	 //APP设置
	uint8_t   LactateThresholdHeartrate; //乳酸阈心率值 //APP设置
	uint8_t   htr_zone_one_min;       //心率区间1低值 //APP设置
	uint8_t   htr_zone_one_max;       //心率区间1高值 //APP设置
	uint8_t   htr_zone_two_min;       //心率区间2低值 //APP设置
	uint8_t   htr_zone_two_max;       //心率区间2低值 //APP设置
	uint8_t   htr_zone_three_min;     //心率区间3低值 //APP设置
	uint8_t   htr_zone_three_max;     //心率区间3低值 //APP设置
	uint8_t   htr_zone_four_min;      //心率区间4低值 //APP设置
	uint8_t   htr_zone_four_max;      //心率区间4低值 //APP设置
	uint8_t   htr_zone_five_min;      //心率区间5低值 //APP设置
	uint8_t   htr_zone_five_max;      //心率区间5低值 //APP设置
}_ble_personal_info;
typedef union __attribute__ ((packed))
{ 
		uint8_t value;
		struct __attribute__ ((packed))
		{
			uint8_t htr_waining          :     1;   
			uint8_t pace_alert           :     1; 
			uint8_t distance_alert       :     1; 
			uint8_t auto_count_lap       :     1; 
			uint8_t goal_lap_alert       :     1; 
			uint8_t goal_cal_alert    	 :     1; 
			uint8_t goal_time_alert   	 :     1;
			uint8_t altitude_alert   	 	 :     1;
		}bits;
}_sports_switch;
typedef struct __attribute__ ((packed))
{
	uint32_t  command;
	uint32_t  crc;
	union __attribute__ ((packed))
	{ 
		uint32_t value;
		struct __attribute__ ((packed))
		{
			uint32_t daily          :     1;   
			uint32_t run            :     1; 
			uint32_t marithon       :     1; 
			uint32_t country_race   :     1; 
			uint32_t indoor_swim    :     1; 
			uint32_t cycling        :     1; 
			uint32_t mountain_climb :     1; 
			uint32_t walking        :     1; 
			uint32_t country_walk   :     1; 
			uint32_t indoor_run     :     1; 
			uint32_t triathonal     :     1; 
		}bits;
	}sports_type_mask;
	struct __attribute__ ((packed))
	{
		uint16_t goal_step;
		uint16_t goal_calory;
		uint16_t goal_distance;
	}daily;
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint16_t goal_pace;
		uint16_t goal_distance;
		uint16_t lap_count_distance;
	}run;	
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint16_t goal_pace;
		uint16_t goal_distance;
		uint16_t lap_count_distance;
	}marithon;	
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint16_t goal_pace;
		uint16_t goal_distance;
		uint16_t lap_count_distance;
	}country_race;		
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint8_t goal_lap;
		uint16_t goal_time_sec;
		uint8_t pool_len;
	}indoor_swim;		
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint16_t goal_pace;
		uint16_t goal_distance;
		uint16_t lap_count_distance;
		uint16_t wheel_radius; //寸（2.54cm）数字除以10
	}cycling;			
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint16_t goal_alivation;
		uint16_t goal_distance;
		uint16_t lap_count_distance;
	}mountain_climb;		
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint16_t goal_calory;
		uint16_t goal_time_sec;
		uint16_t lap_count_distance;
	}walking;	
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint16_t goal_calory;
		uint16_t lap_count_distance;
	}country_walk;		
	struct __attribute__ ((packed))
	{
		_sports_switch swtch;
		uint16_t goal_pace;
		uint16_t goal_distance;
		uint16_t lap_count_distance;
	}indoor_run;	
	struct __attribute__ ((packed))
	{
		struct __attribute__ ((packed))
		{
			_sports_switch swtch;
			uint16_t goal_time_sec;
			uint16_t lap_count_distance;
		}outdoor_swim;	
		struct __attribute__ ((packed))
		{
			_sports_switch swtch;
			uint16_t goal_pace;
			uint16_t goal_distance;
			uint16_t lap_count_distance;
			uint16_t wheel_radius; //寸（2.54cm）数字除以10
		}cycling;
		struct __attribute__ ((packed))
		{
			_sports_switch swtch;
			uint16_t goal_pace;
			uint16_t goal_distance;
			uint16_t lap_count_distance;
		}run;			
	}triathonal;	
}_ble_sports_param;

typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t step;
	uint32_t calory;
	uint32_t distance;
}_wechat_sport_data;


_ble_passkey g_ble_passkey;
extern _app_notification_msg g_app_notification_msg;




static void ble_uart_presonse_application_ble_info(bool is_app_layer);
static void ble_uart_ccommand_application_info_apollo2(void);
#ifdef COD
static void ble_uart_ccommand_application_info_bat(void);
static void ble_uart_ccommand_application_current_time_read(void);
static void ble_uart_ccommand_application_location_info(void);
static void ble_uart_ccommand_application_phone_call_status(void);
static void ble_uart_ccommand_application_phone_call_delay(void);
static void ble_uart_ccommand_application_on_bind_device(void);
static void ble_uart_ccommand_application_bind_status(void);
static void ble_uart_ccommand_application_sport_cast(void);
static void ble_uart_ccommand_application_training_cast(void);
static void ble_uart_ccommand_application_rt_data_trans(void);
static void ble_uart_ccommand_application_sensor_data_trans(void);
static void ble_uart_ccommand_application_rest_heartrate(void);
static void ble_uart_ccommand_application_data_frame_num(void);
static void ble_uart_ccommand_application_delete_data(void);
static void ble_uart_ccommand_application_mac_read(void);
static void ble_uart_ccommand_application_mac_write(void);
static void ble_uart_ccommand_application_update_apgs_time(void);
static void ble_uart_ccommand_application_update_bp_start(void);
static void ble_uart_ccommand_application_update_bp_finish(void);
static void ble_uart_ccommand_application_real_time_log_trans(void);

#endif
static void ble_uart_ccommand_application_ble_connect(void);
static void ble_uart_ccommand_application_ble_disconnect(void);
static void ble_uart_ccommand_application_ble_bond(void);
static void ble_uart_ccommand_application_ble_passkey(void);
static void ble_uart_response_application_jump_bootloader(void);
static void ble_uart_response_application_reset(void);
static void ble_uart_response_bootloader_bootsetting(void);
static void ble_uart_response_bootloader_prevalidate(void);
static void ble_uart_response_bootloader_postvalidate(void);
static void ble_uart_response_bootloader_reset_n_activate(void);
static void pc_uart_ccommand_application_jump_bootloader(void);
static void pc_uart_command_hardware_test_ble_info_read_response(void);
static void pc_uart_command_hardware_test_ble_rssi_read_response(void);
static void pc_uart_command_hardware_test_ble_rssi_read_stop_response(void);
static void pc_uart_command_hardware_test_ble_extflash_read_response(void);
static void ble_uart_ccommand_application_dev_name_update(void);
static void ble_uart_ccommand_application_weather_info_request(void);
static void ble_uart_ccommand_application_current_time_write(void);
static void ble_uart_ccommand_application_personal_info_read(void);
static void ble_uart_ccommand_application_personal_info_write(void);
static void ble_uart_ccommand_application_sports_param_read(void);
static void ble_uart_ccommand_application_sports_param_write(void);
static void ble_uart_ccommand_application_app_message_ios(void);	
static void ble_uart_ccommand_application_app_message_android(void);
static void ble_uart_ccommand_application_wechat_sport_data_request(void);
static void ble_uart_ccommand_application_accessory_heartrate_connect(void);	
static void ble_uart_ccommand_application_accessory_running_connect(void);	
static void ble_uart_ccommand_application_accessory_cycling_connect(void);
static void ble_uart_ccommand_application_accessory_heartrate_disconnect(void);
static void ble_uart_ccommand_application_accessory_running_disconnect(void);	
static void ble_uart_ccommand_application_accessory_cycling_disconnect(void);	
static void ble_uart_ccommand_application_accessory_data_upload(void);	
static void ble_uart_ccommand_application_extflash_config_astro_sync_request(void);
static void ble_uart_ccommand_application_extflash_config_main_day_data_request(void);
static void ble_uart_ccommand_application_extflash_config_activity_address_request(void);
static void ble_uart_ccommand_application_extflash_config_activity_address_boundary_request(void);
static void ble_uart_ccommand_application_extflash_config_shard_track_address_request(void);
static void ble_uart_ccommand_application_extflash_config_shard_track_header_write(void);
static void ble_uart_response_application_find_phone(void);
static void ble_uart_ccommand_application_find_watch(void);
static void ble_uart_ccommand_application_get_watch_default_sport_mode(void);

static void ble_uart_command_application_extflash_config_train_plan_request_uuid(void);
static void ble_uart_command_application_extflash_config_train_plan_read_address_request(void);
static void ble_uart_command_application_extflash_config_train_plan_write_address_request(void);
static void ble_uart_command_application_extflash_config_train_plan_result_request_uuid(void);
static void ble_uart_command_application_extflash_config_train_plan_result_read_address_request(void);
static void ble_uart_command_application_extflash_config_train_plan_result_write_address_request(void);
static void ble_uart_command_application_extflash_config_train_notify_switch(void);	
static void ble_uart_ccommand_application_extflash_config_train_plan_erase_address_request(void);
static void ble_uart_ccommand_application_extflash_config_train_plan_result_erase_address_request(void);
static void ble_uart_ccommand_application_get_watch_calculate_param_write(void);
static void ble_uart_ccommand_application_get_watch_calculate_param_read(void);
static void ble_uart_ccommand_application_reset(void);


static void ble_uart_ccommand_application_get_watch_face_info(void);
static void ble_uart_ccommand_application_get_watch_face_info_2(void);
static void ble_uart_ccommand_application_get_watch_face_finish(void);
static void ble_uart_ccommand_application_get_watch_face_activate(void);



static void ble_uart_ccommand_app_ble_dfu_start(void);
static void ble_uart_ccommand_app_ble_dfu_finish(void);
static void ble_uart_ccommand_app_ble_dfu_apollo_bootsetting(void);
static void ble_uart_ccommand_app_ble_dfu_apollo_prevalidate(void);
static void ble_uart_ccommand_app_ble_dfu_apollo_postvalidate(void);
static void ble_uart_ccommand_app_ble_dfu_apollo_reset_n_activate(void);



static void task_cb(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	
//	if(evt == DRV_BLE_EVT_UART_DATA)
//	{
//		TASK_BLE_LOG_PRINTF("[TASK_BLE]:task_cb\n");
//		for(uint32_t i=0;i<32;i++)
//		{
//			TASK_BLE_LOG_PRINTF("%02X ",p_data[i]);
//		}
//		TASK_BLE_LOG_PRINTF("\n");
//		
//	}
	
	
	BaseType_t xHigherPriorityTaskWoken, xResult;
	xHigherPriorityTaskWoken = pdFALSE;
	m_task_buffer.p_data = (uint32_t *)p_data;
	m_task_buffer.length = length;
	xResult = xEventGroupSetBitsFromISR(evt_handle_ble, (evt),
																			&xHigherPriorityTaskWoken);
	if (xResult == pdPASS)
	{
		 portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
//	else
//	{
//		ERR_HANDLER(ERR_FORBIDDEN);
//	}
}

static void task(void *pvParameters)
{

	uint32_t bitSet;
	

	while (1)
	{
		bitSet = xEventGroupWaitBits(evt_handle_ble, 0xFF, pdTRUE,
												pdFALSE, portMAX_DELAY);
			switch(bitSet)
			{
				case DRV_BLE_EVT_UART_DATA:{
					TASK_BLE_LOG_PRINTF("[TASK_BLE]:DRV_BLE_EVT_UART_DATA\r\n");	
//				for(uint32_t i=0;i<12;i++)
//				{
//					TASK_BLE_LOG_PRINTF("%08X ",m_task_buffer.p_data[i]);
//				}
//				TASK_BLE_LOG_PRINTF("\n");	
					
				if(m_task_buffer.p_data[0] & ((1<<BLE_UART_CCOMMAND_MASK_APPLICATION_POSITION)<<8))
				{
					TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_MASK_APPLICATION\n");
						switch(m_task_buffer.p_data[0]&0x0000FF00)
						{
							case BLE_UART_RESPONSE_APPLICATION_BLE_INFO:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_APPLICATION_BLE_INFO\n");
								ble_uart_presonse_application_ble_info(true);
							}break;
							case BLE_UART_CCOMMAND_APPLICATION_INFO_APOLLO2:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_INFO_APOLLO2\n");
								ble_uart_ccommand_application_info_apollo2();
							}break;	
#ifdef COD
							case BLE_UART_CCOMMAND_APPLICATION_INFO_BAT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_INFO_BAT\n");
								ble_uart_ccommand_application_info_bat();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_CURRENT_TIME_READ:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_CURRENT_TIME_READ\n");
								ble_uart_ccommand_application_current_time_read();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_LOCATION_INFO:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_LOCATION_INFO\n");
								ble_uart_ccommand_application_location_info();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_STATUS:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_STATUS\n");
								ble_uart_ccommand_application_phone_call_status();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_DELAY:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_DELAY\n");
								ble_uart_ccommand_application_phone_call_delay();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_ON_BIND_DEVICE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ON_BIND_DEVICE\n");
								ble_uart_ccommand_application_on_bind_device();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_SEND_BIND_STATUS:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_SEND_BIND_STATUS\n");
								ble_uart_ccommand_application_bind_status();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_ON_SPORT_CAST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ON_SPORT_CAST\n");
								ble_uart_ccommand_application_sport_cast();
								}break;	
							case BLE_UART_CCOMMAND_APPLICATION_ON_TRAINING_CAST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ON_TRAINING_CAST\n");
								ble_uart_ccommand_application_training_cast();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_RT_DATA_TRANS:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_RT_DATA_TRANS\n");
								ble_uart_ccommand_application_rt_data_trans();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_SENSOR_DATA_TRANS:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_SENSOR_DATA_TRANS\n");
								ble_uart_ccommand_application_sensor_data_trans();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_REST_HEARTRATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_DELAY\n");
								ble_uart_ccommand_application_rest_heartrate();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_DATA_FRAME_NUM:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_DATA_FRAME_NUM\n");
								ble_uart_ccommand_application_data_frame_num();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_DELETE_DATA:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_DELETE_DATA\n");
								ble_uart_ccommand_application_delete_data();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_MAC_READ:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_MAC_READ\n");
								ble_uart_ccommand_application_mac_read();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_MAC_WRITE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_MAC_WRITE\n");
								ble_uart_ccommand_application_mac_write();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_UPDATA_AGPS_TIME:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_UPDATA_AGPS_TIME\n");
								ble_uart_ccommand_application_update_apgs_time();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_UPDATA_BP_START:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_UPDATA_BP_START\n");
								ble_uart_ccommand_application_update_bp_start();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_UPDATA_BP_FINISH:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_UPDATA_BP_START\n");
								ble_uart_ccommand_application_update_bp_finish();
								}break;
							case BLE_UART_CCOMMAND_APPLICATION_REAL_TIME_LOG_TRANS:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_RT_DATA_TRANS\n");
								ble_uart_ccommand_application_real_time_log_trans();
								}break;
#endif
							case BLE_UART_CCOMMAND_APPLICATION_BLE_CONNECT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_BLE_CONNECT\n");
								ble_uart_ccommand_application_ble_connect();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_BLE_DISCONNECT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_BLE_DISCONNECT\n");
								ble_uart_ccommand_application_ble_disconnect();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_BLE_BOND:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_BLE_BOND\n");
								ble_uart_ccommand_application_ble_bond();
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_BLE_PASSKEY:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_BLE_PASSKEY\n");
								ble_uart_ccommand_application_ble_passkey();
							}break;	
							case BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER\n");
								ble_uart_response_application_jump_bootloader();
							}break;	
							case BLE_UART_RESPONSE_APPLICATION_RESET:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_APPLICATION_RESET\n");
								ble_uart_response_application_reset();
							}break;								
							case BLE_UART_RESPONSE_BOOTLOADER_BOOTSETTING:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_BOOTLOADER_BOOTSETTING\n");
								ble_uart_response_bootloader_bootsetting();
							}break;							
							case BLE_UART_RESPONSE_BOOTLOADER_PREVALIDATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_BOOTLOADER_PREVALIDATE\n");
								ble_uart_response_bootloader_prevalidate();
							}break;										
							case BLE_UART_RESPONSE_BOOTLOADER_POSTVALIDATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_BOOTLOADER_POSTVALIDATE\n");
								ble_uart_response_bootloader_postvalidate();
							}break;			
							case BLE_UART_RESPONSE_BOOTLOADER_RESET_N_ACTIVATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_BOOTLOADER_RESET_N_ACTIVATE\n");
								ble_uart_response_bootloader_reset_n_activate();
							}break;			
							case PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER\n");
								pc_uart_ccommand_application_jump_bootloader();
							}break;
						
							case BLE_UART_CCOMMAND_APPLICATION_DEV_NAME_UPDATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_DEV_NAME_UPDATE\n");
								ble_uart_ccommand_application_dev_name_update();
							}break;							
							case BLE_UART_CCOMMAND_APPLICATION_CURRENT_TIME_WRITE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_CURRENT_TIME_WRITE\n");
								ble_uart_ccommand_application_current_time_write();
							}break;								
							case BLE_UART_CCOMMAND_APPLICATION_WEATHER_INFO_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_WEATHER_INFO_REQUEST\n");
								ble_uart_ccommand_application_weather_info_request();
							}break;								
							case BLE_UART_CCOMMAND_APPLICATION_PERSONAL_INFO_READ:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_PERSONAL_INFO_READ\n");
								ble_uart_ccommand_application_personal_info_read();
							}break;							
							case BLE_UART_CCOMMAND_APPLICATION_PERSONAL_INFO_WRITE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_PERSONAL_INFO_WRITE\n");
								ble_uart_ccommand_application_personal_info_write();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_SPORTS_PARAM_READ:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_SPORTS_PARAM_READ\n");
								ble_uart_ccommand_application_sports_param_read();
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_SPORTS_PARAM_WRITE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_SPORTS_PARAM_WRITE\n");
								ble_uart_ccommand_application_sports_param_write();
							}break;								
							case BLE_UART_CCOMMAND_APPLICATION_APP_MESSAGE_IOS:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_APP_MESSAGE_IOS\n");
								ble_uart_ccommand_application_app_message_ios();
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_APP_MESSAGE_ANDROID:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_APP_MESSAGE_ANDROID\n");
								ble_uart_ccommand_application_app_message_android();
							}break;							
							case BLE_UART_CCOMMAND_APPLICATION_WECHAT_SPORT_DATA_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_WECHAT_SPORT_DATA_REQUEST\n");
								ble_uart_ccommand_application_wechat_sport_data_request();
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_HEARTRATE_CONNECT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_HEARTRATE_CONNECT\n");
								ble_uart_ccommand_application_accessory_heartrate_connect();
							}break;								
							case BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_RUNNING_CONNECT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_RUNNING_CONNECT\n");
								ble_uart_ccommand_application_accessory_running_connect();
							}break;			
							case BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_CYCLING_CONNECT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_CYCLING_CONNECT\n");
								ble_uart_ccommand_application_accessory_cycling_connect();
							}break;			
							case BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_HEARTRATE_DISCONNECT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_HEARTRATE_DISCONNECT\n");
								ble_uart_ccommand_application_accessory_heartrate_disconnect();
							}break;			
							case BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_RUNNING_DISCONNECT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_RUNNING_DISCONNECT\n");
								ble_uart_ccommand_application_accessory_running_disconnect();
							}break;			
							case BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_CYCLING_DISCONNECT:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_CYCLING_DISCONNECT\n");
								ble_uart_ccommand_application_accessory_cycling_disconnect();
							}break;										
							case BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_DATA_UPLOAD:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_DATA_UPLOAD\n");
								ble_uart_ccommand_application_accessory_data_upload();
							}break;							
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST\n");
								ble_uart_ccommand_application_extflash_config_astro_sync_request();
							}break;				
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_MAIN_DAY_DATA_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_MAIN_DAY_DATA_REQUEST\n");
								ble_uart_ccommand_application_extflash_config_main_day_data_request();
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST\n");
								ble_uart_ccommand_application_extflash_config_activity_address_request();
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_REQUEST\n");
								ble_uart_ccommand_application_extflash_config_activity_address_boundary_request();
							}break;															
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_ADDRESS_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_ADDRESS_REQUEST\n");
								ble_uart_ccommand_application_extflash_config_shard_track_address_request();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_HEADER_WRITE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_HEADER_WRITE\n");
								ble_uart_ccommand_application_extflash_config_shard_track_header_write();
							}break;								
							case BLE_UART_RESPONSE_APPLICATION_FIND_PHONE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_APPLICATION_FIND_PHONE\n");
								ble_uart_response_application_find_phone();
							}break;					
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID\n");
								ble_uart_command_application_extflash_config_train_plan_request_uuid();
							}break;									
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST\n");
								ble_uart_command_application_extflash_config_train_plan_read_address_request();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST\n");
								ble_uart_command_application_extflash_config_train_plan_write_address_request();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID\n");
								ble_uart_command_application_extflash_config_train_plan_result_request_uuid();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST\n");
								ble_uart_command_application_extflash_config_train_plan_result_read_address_request();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST\n");
								ble_uart_command_application_extflash_config_train_plan_result_write_address_request();
							}break;		
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_NOTIFY_SWITCH:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_NOTIFY_SWITCH\n");
								ble_uart_command_application_extflash_config_train_notify_switch();
							}break;									
							
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST\n");
								ble_uart_ccommand_application_extflash_config_train_plan_erase_address_request();
							}break;									
							case BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST\n");
								ble_uart_ccommand_application_extflash_config_train_plan_result_erase_address_request();
							}break;																
							case BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_CALCULATE_PARAM_WRITE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_CALCULATE_PARAM_WRITE\n");
								ble_uart_ccommand_application_get_watch_calculate_param_write();
							}break;									
							case BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_CALCULATE_PARAM_READ:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_CALCULATE_PARAM_READ\n");
								ble_uart_ccommand_application_get_watch_calculate_param_read();
							}break;		

							case BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO\n");
								ble_uart_ccommand_application_get_watch_face_info();
							}break;	
							case BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO_2:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO_2\n");
								ble_uart_ccommand_application_get_watch_face_info_2();
							}break;								
							case BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_FINISH:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_FINISH\n");
								ble_uart_ccommand_application_get_watch_face_finish();
							}break;								
							case BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_ACTIVATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_ACTIVATE\n");
								ble_uart_ccommand_application_get_watch_face_activate();
							}break;	
							

							case BLE_UART_CCOMMAND_APPLICATION_RESET:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_RESET\n");
								ble_uart_ccommand_application_reset();
							}break;	
							
							
							case BLE_UART_CCOMMAND_APPLICATION_APP_ERROR:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_APP_ERROR\n");
								BLE_ERR_HANDLER((_ble_app_error *)m_task_buffer.p_data);
							}break;	

							case BLE_UART_CCOMMAND_APPLICATION_FIND_WATCH:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_FIND_WATCH\n");
								ble_uart_ccommand_application_find_watch();
							}break;
							
							case BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_DEFAULT_SPORT_MODE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_DEFAULT_SPORT_MODE\n");
								ble_uart_ccommand_application_get_watch_default_sport_mode();
							}break;
							
							default:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:UNSPPORTED COMMAND\n");
								drv_ble_uart_close();
							}break;
						}		 			
				}
				else if(m_task_buffer.p_data[0] & ((1<<BLE_UART_CCOMMAND_MASK_BOOTLOADER_POSITION)<<8))
				{
					switch(m_task_buffer.p_data[0]&0x0000FF00)
					{
						case BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER:{
							TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER\n");
							ble_uart_response_application_jump_bootloader();
						}break;	
						case BLE_UART_RESPONSE_APPLICATION_BLE_INFO:{
							TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_RESPONSE_APPLICATION_BLE_INFO\n");
							ble_uart_presonse_application_ble_info(false);
						}break;
						
						default:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:UNSPPORTED COMMAND\n");
								drv_ble_uart_close();
						}break;
					}
				}
				else if(m_task_buffer.p_data[0] & ((1<<BLE_UART_CCOMMAND_MASK_APP_POSITION)<<8))
				{
					TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_MASK_APP\n");
						switch(m_task_buffer.p_data[0]&0x0000FF00)
						{
							case BLE_UART_CCOMMAND_APP_BLE_DFU_START:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APP_BLE_DFU_START\n");
								ble_uart_ccommand_app_ble_dfu_start();
							}break;
							case BLE_UART_CCOMMAND_APP_BLE_DFU_FINISH:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APP_BLE_DFU_FINISH\n");
								ble_uart_ccommand_app_ble_dfu_finish();
							}break;
							case BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_BOOTSETTING:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_BOOTSETTING\n");
								ble_uart_ccommand_app_ble_dfu_apollo_bootsetting();
							}break;
							case BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_PREVALIDATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_PREVALIDATE\n");
								ble_uart_ccommand_app_ble_dfu_apollo_prevalidate();
							}break;							
							case BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_POSTVALIDATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_POSTVALIDATE\n");
								ble_uart_ccommand_app_ble_dfu_apollo_postvalidate();
							}break;							
							case BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE\n");
								ble_uart_ccommand_app_ble_dfu_apollo_reset_n_activate();
							}break;								
							
							
							default:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:UNSPPORTED COMMAND\n");
								drv_ble_uart_close();
							}break;
						}							
					
				}		
				else if(m_task_buffer.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_HARDWARE_TEST_POSITION)<<8))
				{
					TASK_BLE_LOG_PRINTF("[TASK_BLE]:PC_UART_CCOMMAND_MASK_HARDWARE_TEST_POSITION\n");
						switch(m_task_buffer.p_data[0]&0x0000FF00)
						{
							case PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ_RESPONSE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ_RESPONSE\n");
								pc_uart_command_hardware_test_ble_info_read_response();
							}break;
							case PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_RESPONSE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_RESPONSE\n");
								pc_uart_command_hardware_test_ble_rssi_read_response();
							}break;							
							case PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_STOP_RESPONSE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_STOP_RESPONSE\n");
								pc_uart_command_hardware_test_ble_rssi_read_stop_response();
							}break;			
							case PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ_RESPONSE:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ_RESPONSE\n");
								pc_uart_command_hardware_test_ble_extflash_read_response();
							}break;	


							
							default:{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:UNSPPORTED COMMAND\n");
								drv_ble_uart_close();
							}break;
						}							
					
				}	
				else{
					TASK_BLE_LOG_PRINTF("[TASK_BLE]:UNSPPORTED MASK\n");
					drv_ble_uart_close();
				}
					


				}break;				
				case DRV_BLE_EVT_UART_CTS_INTERRUPT:{
//					DELAY_US(10);
//					vTaskDelay(10);
					if (am_hal_gpio_input_bit_read(DRV_BLE_UART_CTS_PIN))
					{
							drv_ble_on_cts_high();				
					}
					else
					{
							drv_ble_on_cts_low();
						switch(m_event)
						{
							case JUMP_BOOTLOADER:{
								lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_DFU_BLE);
							}break;
						
						}
					}						
				}break;				
				
				
				default:
					break;
			}		
	
	}
}




void task_ble_init(void)
{
	if(m_initialized)
	{
		return;
	}
	
	m_initialized = true;		
	

	drv_ble_init(task_cb);


	
	
	evt_handle_ble = xEventGroupCreate();
	if(evt_handle_ble == NULL)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TASK_BLE_LOG_PRINTF("[TASK_BLE]:xEventGroupCreate fail\n");
	}
	BaseType_t xResult = xTaskCreate(task, "BLE", TaskBLE_StackDepth, NULL, TaskBLE_Priority, &task_handle);
	if (xResult == pdFAIL)
	{
		ERR_HANDLER(ERR_NO_MEM);
		TASK_BLE_LOG_PRINTF("[TASK_BLE]:xTaskCreate fail\n");
	}
	
	//g_ble_accessory.enable = false;
	g_ble_accessory.heartrate.status = ACCESSORY_STATUS_DISCONNECT;
	g_ble_accessory.running.status = ACCESSORY_STATUS_DISCONNECT;
	g_ble_accessory.cycling.status = ACCESSORY_STATUS_DISCONNECT;
	g_ble_accessory.data.cycle_data_valid = false;
	g_ble_accessory.data.hr_data_valid = false;
	g_ble_accessory.data.run_data_valid = false;

	if(g_ble_accessory.enable == true)
	{
		DELAY_MS(3000);
		//发送扫描指令
        TASK_BLE_LOG_PRINTF("[TASK_BLE]:xTaskCreate task_ble_init send scan enable command\n");
		uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_SCAN_ENABLE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		_uart_data_param param;
		param.user_callback = NULL;
		param.p_data = (uint8_t *)command;
		param.length = sizeof(command);
		uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	#ifndef WATCH_HAS_NO_BLE 
		while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
		{
			DELAY_MS(TASK_UART_DELAY);
		}	
	#endif
	}
	
	if(ScreenState == DISPLAY_SCREEN_ACCESSORY)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_ACCESSORY;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
	}	

	
}

void task_ble_uninit(void)
{
	if(!m_initialized)
	{
		return;
	}	
	m_initialized = false;	

	drv_ble_uart_close_on_ble_sys_off();

	
	
	vTaskDelete(task_handle);
	vEventGroupDelete(evt_handle_ble);

}



void dev_ble_init(uint8_t ble_switch)
{
	if(ble_switch == 0)
	{

		am_hal_gpio_out_bit_set(DEV_BLE_SYS_OFF_PIN);
		am_hal_gpio_pin_config(DEV_BLE_SYS_OFF_PIN,AM_HAL_GPIO_OUTPUT);	
	
	}
	else
	{

		am_hal_gpio_pin_config(DEV_BLE_SYS_OFF_PIN,AM_HAL_GPIO_OUTPUT);

	    am_hal_gpio_out_bit_clear(DEV_BLE_SYS_OFF_PIN);

	}

}


void dev_ble_system_on(void)
{

	if(am_hal_gpio_out_bit_read(DEV_BLE_SYS_OFF_PIN))
	{
		am_hal_gpio_out_bit_clear(DEV_BLE_SYS_OFF_PIN);				
		//DELAY_MS(100);
		task_ble_init();

	}
}

void dev_ble_system_off(void)
{

	if(!am_hal_gpio_out_bit_read(DEV_BLE_SYS_OFF_PIN))
	{
		am_hal_gpio_out_bit_set(DEV_BLE_SYS_OFF_PIN);	

		task_ble_uninit();
	}
}



static void ble_uart_presonse_application_ble_info(bool is_app_layer) 
{
	_task_ble_info task_ble_info;
	/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
	memcpy((uint8_t *)&task_ble_info,(uint8_t *)m_task_buffer.p_data,sizeof(_task_ble_info));
	drv_ble_uart_close();
    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)&task_ble_info, sizeof(_task_ble_info));
	}
	
	if(is_app_layer)
	{
		_lib_app_data lib_app_data;
		lib_app_data_read(&lib_app_data);					
		if(!lib_app_data_crc_check_bootsetting(&lib_app_data))
		{
			lib_app_data.ble.setting_version = task_ble_info.setting_version;
			lib_app_data.ble.bootloader_version = task_ble_info.bootloader_version;
			lib_app_data.ble.application_version = task_ble_info.application_version;
			lib_app_data.ble.softdevice_version = task_ble_info.softdevice_version;
			lib_app_data.ble.pcb_version = task_ble_info.pcb_version;
			lib_app_data.ble.mac_address_f4b = task_ble_info.mac_address_f4b;
			lib_app_data.ble.mac_address_l2b = task_ble_info.mac_address_l2b;
			lib_app_data.ble.name_len = task_ble_info.dev_name.len;
			if(task_ble_info.dev_name.len>10 || task_ble_info.dev_name.len==0)
			{
				return;
			}
			if(task_ble_info.dev_name.name[10] != 0)
			{
				return;
			}
			memset(lib_app_data.ble.name,0,sizeof(lib_app_data.ble.name));
			memcpy(lib_app_data.ble.name,task_ble_info.dev_name.name,task_ble_info.dev_name.len);		
			lib_app_data.ble.build_date = task_ble_info.build_date;
			lib_app_data.ble.build_time = task_ble_info.build_time;
			lib_app_data.crc = am_bootloader_fast_crc32((uint8_t*)(&lib_app_data.crc)+sizeof(lib_app_data.crc),sizeof(_lib_app_data)-sizeof(lib_app_data.crc));
			lib_app_data_write(&lib_app_data);	

		}
		else{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]:app data area already exist\n");
		}


		if(ScreenState == DISPLAY_SCREEN_MENU)
		{
			DISPLAY_MSG  msg = {0,0};

			ScreenState = DISPLAY_SCREEN_WATCH_INFO;		
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
		}			
	}
	


}
static void ble_uart_ccommand_application_info_apollo2(void)
{
	_boot_setting boot_setting;
	lib_boot_setting_read(&boot_setting);
	_info_apollo2 info_apollo2;
	info_apollo2.command = BLE_UART_RESPONSE_APPLICATION_INFO_APOLLO2 | BLE_UART_CCOMMAND_MASK_APPLICATION;
	info_apollo2.bootloader_version = boot_setting.bootloader_version;
	info_apollo2.application_version = boot_setting.application_version;
	info_apollo2.update_version = boot_setting.update_version;
	if (rtc_hw_flag == 1)
	{
		info_apollo2.pcb_version = boot_setting.pcb_version | 0x0100;
	}
	else
	{
		info_apollo2.pcb_version = boot_setting.pcb_version;
	}
	info_apollo2.chip_id_0 = AM_REG(MCUCTRL, CHIPID0);
	info_apollo2.chip_id_1 = AM_REG(MCUCTRL, CHIPID1);
	memcpy(info_apollo2.model,boot_setting.model,sizeof(info_apollo2.model));
	info_apollo2.protocal_version = BLE_PROTOCAL_VERSION_CODE;
	
	drv_ble_uart_put((uint8_t *)&info_apollo2, sizeof(_info_apollo2));	
}
#ifdef COD
extern SemaphoreHandle_t StepSemaphore;

void cod_user_sensor_open(void)
{
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:cod_user_sensor_open start \n");
	ring_buffer_init(&cod_sensor_acc_rb, cod_sensor_acc_buffer, 128);
	ring_buffer_init(&cod_sensor_gyro_rb, cod_sensor_gyro_buffer, 128);

	drv_lsm6dsl_cod_open(cod_user_sensor_req.acc_flag,cod_user_sensor_req.gyro_flag,cod_user_sensor_req.six_freq);
	cod_sensor_first_flag = 1;
	 acc_num = 0;
     gyro_num = 0;
	xSemaphoreGive(StepSemaphore);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:cod_user_sensor_open end \n");
	timer_cod_sensor_start();
	if (cod_user_sensor_req.heart_flag)
	{
		 task_hrt_start();	
	}
}

void cod_user_sensor_close(void)
{
	timer_cod_sensor_stop(false);
	if((cod_user_sensor_req.acc_flag) ||(cod_user_sensor_req.gyro_flag))
	{
		drv_lsm6dsl_cod_close();
	}
	if (cod_user_sensor_req.heart_flag)
	{
		task_hrt_close();
	}
	cod_sensor_first_flag = 0;
	memset(&cod_user_sensor_req,0,sizeof(_cod_user_sensor_req));
	cod_uart_normal_open = 0;
	drv_ble_uart_close();
}

static void ble_uart_ccommand_application_info_bat(void)
{
	uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_INFO_BAT | BLE_UART_CCOMMAND_MASK_APPLICATION};
			if (g_bat_evt.type == DRV_BAT_EVENT_CHARGING)
			{

			  if (g_bat_evt.level_percent ==100)
			  {
				command[1] =( 0x01 | g_bat_evt.level_percent <<8); //充满
			  }
			  else
			  {
				command[1] =( 0x02 | g_bat_evt.level_percent <<8); //充电
			  }
			
			}
			else{

				command[1] = (0x00 | g_bat_evt.level_percent <<8); //放电
			}

	drv_ble_uart_put((uint8_t *)command, sizeof(command));

}
static void ble_uart_ccommand_application_current_time_read(void)
{
	_ble_current_time ble_app_time;

	am_hal_rtc_time_get(&RTC_time);
	ble_app_time.command = BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_READ | BLE_UART_CCOMMAND_MASK_APPLICATION;
	ble_app_time.year = RTC_time.ui32Year;
	ble_app_time.month = RTC_time.ui32Month;
	ble_app_time.date = RTC_time.ui32DayOfMonth;
	ble_app_time.day_of_week = RTC_time.ui32Weekday;
	ble_app_time.hour = RTC_time.ui32Hour;
	ble_app_time.minute = RTC_time.ui32Minute;
	ble_app_time.second = RTC_time.ui32Second;
	ble_app_time.time_zone = 8;
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:ble_app_time.command %08x\n",ble_app_time.command);
	drv_ble_uart_put((uint8_t *)&ble_app_time, sizeof(ble_app_time));	

}

static void ble_uart_ccommand_application_location_info(void)
{
	CodWeatherSync WeatherInfo;
	uint8_t city[10],city_gbk[10];
	uint16_t city_len;
	dev_extFlash_enable();			
	dev_extFlash_read(WEATHER_DATA_START_ADDRESS, (uint8_t *)&WeatherInfo, sizeof(CodWeatherSync));			
	dev_extFlash_disable();
	
	memcpy((uint8_t *)city,(uint8_t *)(m_task_buffer.p_data + 1), 10);
	lib_utf16_gbk(city,10,city_gbk,&city_len);
	memset((uint8_t *)&WeatherInfo.city,0 ,10);
	memcpy((uint8_t *)&WeatherInfo.city,(uint8_t *)city_gbk, city_len);
	uint32_t response[1] = {BLE_UART_RESPONSE_APPLICATION_LOCATION_INFO | BLE_UART_CCOMMAND_MASK_APPLICATION};
	drv_ble_uart_put((uint8_t *)response, sizeof(response));

	dev_extFlash_enable();		
	dev_extFlash_erase(WEATHER_DATA_START_ADDRESS,sizeof(CodWeatherSync));
	dev_extFlash_write(WEATHER_DATA_START_ADDRESS, (uint8_t *)&WeatherInfo, sizeof(CodWeatherSync));			
	dev_extFlash_disable();


}

static void ble_uart_ccommand_application_phone_call_status(void)
{	
	uint32_t response[1] = {BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_STATUS | BLE_UART_CCOMMAND_MASK_APPLICATION};
	drv_ble_uart_put((uint8_t *)response, sizeof(response));

	if (ScreenState ==DISPLAY_SCREEN_NOTIFY_APP_MSG)
	{
		timer_notify_display_stop(false);
		timer_notify_motor_stop(false);
		timer_notify_buzzer_stop(false);
		DISPLAY_MSG  msg = {0,0};
		ScreenState = ScreenStateSave;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	

	}
}

static void ble_uart_ccommand_application_phone_call_delay(void)
{
	uint32_t command[2];
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));			
	SetValue.phone_call_delay = command[1];
	
	uint32_t response[1] = {BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_DELAY | BLE_UART_CCOMMAND_MASK_APPLICATION};
	
	drv_ble_uart_put((uint8_t *)response, sizeof(response));
	
}

void ble_uart_ccommand_application_phone_call_ctrl(cod_ble_phone_call_ctrl ctrl)
{
	
	uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_CTRL | BLE_UART_CCOMMAND_MASK_APPLICATION};
	command[1] = (uint32_t)ctrl;
	
	_uart_data_param param;	
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}

}

static void ble_uart_ccommand_application_on_bind_device(void)
{
	drv_ble_uart_close();
	if((ScreenState != DISPLAY_SCREEN_LOGO)
			&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
			&& time_notify_is_ok())
		{
			if(ScreenState < DISPLAY_SCREEN_NOTIFY)
			{
				ScreenStateSave = ScreenState;
			}
			timer_notify_display_start(60000,1,false);
			
			timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
			timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
			
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_NOTIFY_ON_BIND_DEVICE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
		}			
	
}

void ble_uart_response_application_on_bind_device(bool is_success)
{
	
	uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_ON_BIND_DEVICE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	command[1] = (uint32_t)is_success;
	
	_uart_data_param param;	
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}

}

static void ble_uart_ccommand_application_bind_status(void)
{
	cod_bind_status bind_status;
	uint32_t command[2];
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));			
	bind_status= (cod_bind_status)command[1];

	uint32_t response[1] = {BLE_UART_RESPONSE_APPLICATION_SEND_BIND_STATUS | BLE_UART_CCOMMAND_MASK_APPLICATION};
	
	drv_ble_uart_put((uint8_t *)response, sizeof(response));

	if((ScreenState != DISPLAY_SCREEN_LOGO)
			&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
			&& time_notify_is_ok())
		{
			if(ScreenState < DISPLAY_SCREEN_NOTIFY)
			{
				ScreenStateSave = ScreenState;
			}
			timer_notify_display_start(5000,1,false);
			
			timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
			timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
			
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_NOTIFY_BIND_STATUS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			msg.value =(uint32_t) bind_status;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
		}	
}	

static void ble_uart_ccommand_application_sport_cast(void)
{
 
 DISPLAY_MSG  msg = {0,0}; 
   uint8_t vice_screen_sport_type=0;
	memcpy((uint8_t *)&cod_user_sport_cast,(uint8_t *)(m_task_buffer.p_data+1), sizeof(cod_ble_sport_cast));			

	uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_CMD_CAST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	
	TASK_BLE_LOG_PRINTF("sport_type:%d;sport_status:%d\r\n",cod_user_sport_cast.sport_type,cod_user_sport_cast.sport_status);

	if(Get_TaskSportSta()==true)
	{
		  response[1] = COD_BLE_CAST_RESULT_SPORTING;
		  drv_ble_uart_put((uint8_t *)response, sizeof(response));
          return;
	}
	else
		{


	   mode_close();


	   }
	response[1] = COD_BLE_CAST_RESULT_OK;
	drv_ble_uart_put((uint8_t *)response, sizeof(response));
	if(cod_user_sport_cast.sport_status == COD_SPORT_STATUS_STOP)
	{
		cod_sport_cast_flag = 0;
	}
	else
	{
		cod_sport_cast_flag = 1;
	}

	if(cod_user_sport_cast.sport_status == COD_SPORT_STATUS_STOP)
	{
		if (SetValue.rt_heart)
		{
			SetValue.rt_heart = 0;
			task_hrt_close();
		}
	}
	else if (cod_user_sport_cast.sport_status == COD_SPORT_STATUS_START)
	{
		if (SetValue.rt_heart == 0)
		{
			SetValue.rt_heart =1;
			task_hrt_start();
		}
	}
  vice_screen_sport_type = cod_user_sport_cast.sport_type;
  

	
   switch(vice_screen_sport_type)
   	{
    case SPORT_TYPE_OUTDOOR_RUN:
    ScreenStateSave = DISPLAY_VICE_SCREEN_OUTDOOR_RUN; 
	break;
		
	case SPORT_TYPE_OUTDOOR_WALK:
	ScreenStateSave = DISPLAY_VICE_SCREEN_OUTDOOR_WALK; 
	break;

	case SPORT_TYPE_RIDE:
	ScreenStateSave = DISPLAY_VICE_SCREEN_RIDE; 	
	break;

	case SPORT_TYPE_CLIMB:
    ScreenStateSave = DISPLAY_VICE_SCREEN_CLIMB; 
	break;

   }

 if(cod_user_sport_cast.sport_status==COD_SPORT_STATUS_START)//开始
	{
	sport_cast_pause_index = 0;
		if((ScreenState != DISPLAY_SCREEN_LOGO) &&(ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG) && time_notify_is_ok())  
		 {
		 
		 vice_status = VICE_SPORT_ING;
		  timer_notify_display_start(4000,1,false);  
		//      timer_notify_display_start(14000,1,false);  
			ScreenState = DISPLAY_VICE_SCREEN_SPORT_HINT;
			msg.cmd = MSG_DISPLAY_SCREEN; 
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			timer_notify_motor_start(2000,2000,1,false,NOTIFY_MODE_NORMAL);
			timer_notify_buzzer_start(2000,2000,1,false,NOTIFY_MODE_NORMAL);
		 	}
    }
 else if(cod_user_sport_cast.sport_status==COD_SPORT_STATUS_SPORTING)//运动中&继续
 	     {
         
          if(vice_status == VICE_SPORT_PAUSE)
         	{
            
			vice_status = VICE_SPORT_ING;
			save_vice_detail_index = ScreenStateSave;
		    timer_app_vice_pasue_start(500,0);//暂停&继续运动转换到运动投屏界面
            timer_notify_motor_start(500,500,1,false,NOTIFY_MODE_NORMAL);
    		timer_notify_buzzer_start(500,500,1,false,NOTIFY_MODE_NORMAL);//一次短震动
		    }
		  else
		  	{
               ScreenState = ScreenStateSave;
			}
		     msg.cmd = MSG_DISPLAY_SCREEN; 
		     xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
		  }
 else if(cod_user_sport_cast.sport_status==COD_SPORT_STATUS_PAUSE)
		{
	
		 	   
		if(vice_status != VICE_SPORT_PAUSE)
		 {
            vice_status = VICE_SPORT_PAUSE;
		    timer_app_vice_pasue_start(300,1);//暂停5分钟计时
		    ScreenStateSave = DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND;
		   timer_notify_display_start(500,1,false);  
		    ScreenState = DISPLAY_SCREEN_REMIND_VICE_PAUSE_CONTINUE;
		    msg.cmd = MSG_DISPLAY_SCREEN; 
		    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		    timer_notify_motor_start(500,500,1,false,NOTIFY_MODE_NORMAL);
			timer_notify_buzzer_start(500,500,1,false,NOTIFY_MODE_NORMAL);

		 }
		else
		 {
            // ScreenState = DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND;
		     // msg.cmd = MSG_DISPLAY_SCREEN; 
		     // xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
                       
           }
			  
			    
       	}
	else if(cod_user_sport_cast.sport_status==COD_SPORT_STATUS_STOP)
	{
	   
	  // timer_app_vice_pasue_start(300,1);//暂停5分钟计时
	   timer_app_vice_pasue_stop(false);
	   set_train_pause_time(); 
       ScreenState = DISPLAY_VICE_SCREEN_SPORT_DATA;
	   msg.cmd = MSG_DISPLAY_SCREEN;
       xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

	}
	
	
}	

static void ble_uart_ccommand_application_training_cast(void)
{
   
   DISPLAY_MSG	msg = {0,0};
	memcpy((uint8_t *)&cod_user_training_cast,(uint8_t *)(m_task_buffer.p_data+1), sizeof(_cod_user_training_cast));			

	uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_CMD_CAST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	
	if(Get_TaskSportSta()==true)
	{
		  response[1] = COD_BLE_CAST_RESULT_SPORTING;
		  drv_ble_uart_put((uint8_t *)response, sizeof(response));
          return;
	}
	else
		{


	   mode_close();


	   }
	response[1] = COD_BLE_CAST_RESULT_OK;
	drv_ble_uart_put((uint8_t *)response, sizeof(response));
	
	
	TASK_BLE_LOG_PRINTF("training_type:%d;training_status:%d;action_type:%d\r\n",cod_user_training_cast.training_type,cod_user_training_cast.training_status,cod_user_training_cast.action_type);
	cod_user_training_cast.action_name_len = (cod_user_training_cast.action_name_len - (cod_user_training_cast.action_name_len%2));
	lib_utf16_gbk((uint8_t *)cod_user_training_cast.action_name,cod_user_training_cast.action_name_len,get_action_name,&get_action_name_len);


	if (cod_user_training_cast.training_status == TRAINING_STATUS_START)//开始
	{
		if((ScreenState != DISPLAY_SCREEN_LOGO)&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)&& time_notify_is_ok())
				{
					
					ScreenStateSave = DISPLAY_SCREEN_TRAIN_CAST;	
					timer_notify_display_start(3000,1,false);
			 		ScreenState = DISPLAY_SCREEN_TRAIN_CAST_START_REMIND;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					timer_notify_motor_start(2000,2000,1,false,NOTIFY_MODE_NORMAL);
		            timer_notify_buzzer_start(2000,2000,1,false,NOTIFY_MODE_NORMAL);
				}
	
		
	}
	else if (cod_user_training_cast.training_status == TRAINING_STATUS_PAUSE)//暂停
	{
    if(vice_train_status != VICE_TRAIN_PAUSE)
    	{
    	 vice_train_status = VICE_TRAIN_PAUSE;
		ScreenState = DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND;
		
		timer_app_vice_pasue_start(300,2);//暂停只计时
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
		timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
    	}
	else;
		
		
	}
	else if (cod_user_training_cast.training_status == TRAINING_STATUS_STOP)//结束
	{
	          timer_app_vice_pasue_stop(false);
			  set_train_pause_time();
			if((ScreenState != DISPLAY_SCREEN_LOGO)
				&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
				&& time_notify_is_ok())
				{	
					ScreenStateSave = DISPLAY_SCREEN_HOME;
					
					timer_notify_display_start(12000,1,false);
		             
					 ScreenState = DISPLAY_SCREEN_TRAIN_CAST_END_REMIND;
					DISPLAY_MSG  msg = {0,0};
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
				}
		  timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
		  timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
		 
		  //cod_user_sensor_close();	
	}
	/*训练和恢复训练*/
	else 
	{   /*恢复*/
		if (cod_user_training_cast.training_status == TRAINING_STATUS_RESUME)
		{
		    
			vice_train_status = VICE_TRAIN_ING;
			timer_app_vice_pasue_stop(false);
			set_train_pause_time();
			timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
		  	timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
		
		}
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_TRAIN_CAST;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	}

}	

static void ble_uart_ccommand_application_rt_data_trans(void)
{
	uint32_t command;
	memcpy((uint8_t *)&command,(uint8_t *)(m_task_buffer.p_data+1), sizeof(command));			
 
	if((command &0xFFFF)== 2)
	{
		SetValue.rt_today_summary = (command >> 16);
		if (SetValue.rt_today_summary)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: timer_rt_cod_start \n");	
			timer_rt_cod_start();
		}
		else {
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: timer_rt_cod_stop \n");	
			timer_rt_cod_stop(false);
		}
		
	}
	else if ((command &0xFFFF)== 3)
	{		
		SetValue.rt_heart = (command >> 16);
		cod_uart_normal_open = SetValue.rt_heart;
		if (SetValue.rt_heart)
		{
			task_hrt_start();
		}
		else
		{
			task_hrt_close();
		}
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: rt_heart %d\n",SetValue.rt_heart);
	}
	drv_ble_uart_close();
	
}


static void ble_uart_ccommand_application_sensor_data_trans(void)
{
	uint8_t start,req_num;
	uint32_t command;
	cod_ble_sensor_req cod_sensor_req[6];
	memset(cod_sensor_req,0,sizeof(cod_sensor_req));
	memset(&cod_user_sensor_req,0,sizeof(_cod_user_sensor_req));
	memcpy((uint8_t *)&command,(uint8_t *)(m_task_buffer.p_data+1), sizeof(command));
	req_num = command &0xFF;
	start = command >> 16;
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: sensor_data_trans %x,%x,%x\n",command,req_num,start);
	memcpy((uint8_t *)cod_sensor_req,(uint8_t *)(m_task_buffer.p_data+2), sizeof(cod_ble_sensor_req)*6);
	//drv_ble_uart_close();
	
	cod_user_sensor_req.sensor_start = start;
	cod_uart_normal_open = start;

	for (uint8_t i = 0 ;i<6;i++ )
	{
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: cod_sensor_req %d,%d\n",cod_sensor_req[i].type,cod_sensor_req[i].freq);
		if (cod_sensor_req[i].type == SENSOR_TYPE_ACC )
		{
			cod_user_sensor_req.acc_flag = 1;
			cod_user_sensor_req.six_freq = cod_sensor_req[i].freq;
		}

		if (cod_sensor_req[i].type == SENSOR_TYPE_GYRO )
		{
			cod_user_sensor_req.gyro_flag = 1;
			cod_user_sensor_req.six_freq = cod_sensor_req[i].freq;
		}

		if (cod_sensor_req[i].type == SENSOR_TYPE_HEART )
		{
			cod_user_sensor_req.heart_flag = 1;
			cod_user_sensor_req.heart_freq = cod_sensor_req[i].freq;
		}

	}

	if (cod_user_sensor_req.sensor_start)
	{
		cod_user_sensor_open();
		cod_user_sensor_req.sensor_open = 1;
		
	}
	else
	{
		cod_user_sensor_req.sensor_open = 0;
		cod_user_sensor_close();
	}
 
}	
void ble_uart_response_application_sport_ctrl(cod_ble_sport_ctrl ctrl)
{
	
	uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_SPORT_CTRL | BLE_UART_CCOMMAND_MASK_APPLICATION};
	command[1] = (uint32_t)ctrl;
	
	_uart_data_param param;	
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}

}

void ble_uart_response_application_sensor_data_trains(uint8_t *data,uint16_t len)
{
	uint32_t command[64];
	if (len > 252)
	{
		len = 252;
	}
	command[0] = BLE_UART_RESPONSE_APPLICATION_SENSOR_DATA_TRAINS | BLE_UART_CCOMMAND_MASK_APPLICATION |(len << 24);
	
	memcpy((uint8_t *)(command+1),(uint8_t *)data,len);
	//TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_uart_response_application_sensor_data_trains %08x,len= %d \r\n",command[0],len);
	#if 0
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:data:");
	for (uint8_t i= 0;i< len;i++)
	{
		TASK_BLE_LOG_PRINTF("%x,",data[i]);
	}
	TASK_BLE_LOG_PRINTF("\r\n");
	#endif
	_uart_data_param param;	
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = len + 4;
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(30);
	}

}
//acc_len,gyro_len,heart_len3¤?è°üà¨type,lenght 2×??úμ?3¤?è￡???óDêy?Yê??a0￡?
void ble_cod_sensor_data_trains(uint8_t *acc_data,uint8_t acc_len ,uint8_t *gyro_data,uint8_t gyro_len,uint8_t *heart_data,uint8_t heart_len)
{

	uint8_t data[252];
	uint8_t type;
	uint8_t acc_data_len,gyro_data_len,heart_data_len;
	uint16_t temp_seuq_id;
	temp_seuq_id = (cod_user_sensor_req.sequ_id << 8) |(cod_user_sensor_req.sequ_id >> 8);//大小端转换
	
    memcpy((uint8_t *)data,(uint8_t *)&temp_seuq_id,2); //
    
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: sequ_id %x,%x,%x,%x,%x \r\n",cod_user_sensor_req.sequ_id,data[0],data[1],(uint8_t)cod_user_sensor_req.sequ_id,(uint8_t)(cod_user_sensor_req.sequ_id>> 8));
	
	if (acc_data != NULL)
	{
		 type = SENSOR_TYPE_ACC;
		 if ((2 + acc_len) < 252)//·à?1????
		 {
		 	acc_data_len = acc_len - 2;
		 	memcpy((uint8_t *)data +2,(uint8_t *)&type,1);	
		 	memcpy((uint8_t *)data +2+1,(uint8_t *)&acc_data_len,1);
		 	memcpy((uint8_t *)data +2+1+1,(uint8_t *)acc_data,acc_data_len);
			//TASK_BLE_LOG_PRINTF("[TASK_BLE]: acc_len %d,%d\n",acc_len,acc_len-2);
		 }
	}

	if (gyro_data != NULL)
	{
		 type = SENSOR_TYPE_GYRO;
		 if ((2 + acc_len +gyro_len) < 252)//·à?1????
		 {
		 	gyro_data_len = gyro_len - 2;
			memcpy((uint8_t *)data +2 + acc_len,(uint8_t *)&type,1);	
		 	memcpy((uint8_t *)data +2 + acc_len +1,(uint8_t *)&gyro_data_len,1);	
		 	memcpy((uint8_t *)data +2 + acc_len +1+1,(uint8_t *)gyro_data,gyro_data_len);
			//TASK_BLE_LOG_PRINTF("[TASK_BLE]: gyro_len %d,%d\n",gyro_len,gyro_len-2);
		 }
	}

	if (heart_data != NULL)
	{
		 type = SENSOR_TYPE_HEART;
		 if ((2 + acc_len +gyro_len+heart_len) < 252)//·à?1????
		 {
		 	heart_data_len = heart_len - 2;
			memcpy((uint8_t *)data + 2 + acc_len+gyro_len,(uint8_t *)&type,1);	
		 	memcpy((uint8_t *)data + 2 + acc_len+gyro_len+1,(uint8_t *)&heart_data_len,1);	
		 	memcpy((uint8_t *)data + 2 + acc_len+gyro_len+1+1,(uint8_t *)heart_data,heart_data_len);
			//TASK_BLE_LOG_PRINTF("[TASK_BLE]: heart_len %d,%d\n",heart_len,heart_len-2);
		 }
	}
	
	ble_uart_response_application_sensor_data_trains(data,2+acc_len + gyro_len+heart_len);
	cod_user_sensor_req.sequ_id++;
	

}

//uint16_t log_times = 0; 
//uint16_t send_times = 0; 


void ble_uart_response_application_real_time_log_trains(uint8_t *data,uint16_t len)
{
	uint32_t command[64];
	if (len > 252)
	{
		len = 252;
	}
	//log_times++;
	command[0] = BLE_UART_RESPONSE_APPLICATION_REAL_TIME_LOG_TRAINS | BLE_UART_CCOMMAND_MASK_APPLICATION |(len << 24);
	
	memcpy((uint8_t *)(command+1),(uint8_t *)data,len);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_uart_response_application_real_time_log_trains %08x,len= %d\r\n",command[0],len);
	
	_uart_data_param param;	
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = len + 4;
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(30);
	}
}


static void ble_uart_ccommand_application_rest_heartrate(void)
{
	uint32_t command[2];
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));			
	if (command[1] <  get_hdr_max())
	{
		SetValue.rest_heart = command[1];
	}
	else
	{
		SetValue.rest_heart = 0.5 * get_hdr_max();
	}
	
	uint32_t response[1] = {BLE_UART_RESPONSE_APPLICATION_REST_HEARTRATE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	drv_ble_uart_put((uint8_t *)response, sizeof(response));

}	

static void ble_uart_ccommand_application_data_frame_num(void)
{

	uint32_t command[2];
	cod_ble_data_type type;

	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));			
	
	type = (cod_ble_data_type)command[1];
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:cod_ble_data_type %d\r\n",type);
	if (type == DATA_TYPE_STEP)
	{
		read_all_day_steps();
		
		if (last_sync_com_step < last_store_com_step)
		{
			last_sync_com_step = last_store_com_step;
		}
		if (last_sync_run_step < last_store_run_step)
		{
			last_sync_run_step = last_store_run_step;
		}
		if (last_sync_walk_step < last_store_walk_step)
		{
			last_sync_walk_step = last_store_walk_step;
		}
		if (last_sync_cal < last_store_cal)
		{
			last_sync_cal = last_store_cal;
		}
		if (last_sync_dis < last_store_dis)
		{
			last_sync_dis = last_store_dis;
		}
		
		uint16_t tmp[5];
		if (get_filt_stepcount() >last_sync_com_step)
		{
			tmp[0] = get_filt_stepcount() - last_sync_com_step;
		}
		else
		{
			tmp[0] = 0;
		}
		if (SetValue.run_step > last_sync_run_step)
		{
			tmp[1] = SetValue.run_step  - last_sync_run_step;
		}
		else
		{
			tmp[1] = 0;
		}
		if (SetValue.walk_step > last_sync_walk_step)
		{
			tmp[2] = SetValue.walk_step  - last_sync_walk_step;
		}
		else
		{
			tmp[2] = 0;
		}
		if ((Get_TotalEnergy()/100) > last_sync_cal)
		{
			tmp[3] = Get_TotalEnergy()/100 - last_sync_cal;
		}
		else
		{
			tmp[3] = 0;
		}
		if ( (Get_TotalDistance()/10) > last_sync_dis)
		{
			tmp[4] = Get_TotalDistance()/10 - last_sync_dis;
		}
		else
		{
			tmp[4] = 0;
		}
		//大小端转换
		tmp[0] = (tmp[0] << 8) |(tmp[0] >> 8);
		tmp[1] = (tmp[1] << 8) |(tmp[1] >> 8);
		tmp[2] = (tmp[2] << 8) |(tmp[2] >> 8);
		tmp[3] = (tmp[3] << 8) |(tmp[3] >> 8);
		tmp[4] = (tmp[4] << 8) |(tmp[4] >> 8);
		
		
		uint32_t response[64];
		uint16_t len;
	
		response[0]= BLE_UART_RESPONSE_APPLICATION_DATA_FRAME_NUM | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
		memcpy((uint8_t *)response + 8 ,(uint8_t *)all_day_steps,sizeof(all_day_steps));
		memcpy((uint8_t *)response + 8+ sizeof(all_day_steps) ,(uint8_t *)tmp,sizeof(tmp));

		len = 8+  sizeof(all_day_steps) + sizeof(tmp);
		response[1] = (len << 16) | (uint16_t)type;
		
		memcpy((uint8_t *)(response +1) ,(uint8_t *)&response[1],4);
		
		drv_ble_uart_put((uint8_t *)response,len);
		
	    last_sync_com_step =  get_filt_stepcount();
		last_sync_run_step = SetValue.run_step;
		last_sync_walk_step = SetValue.walk_step;
		last_sync_cal = Get_TotalEnergy()/100;
		last_sync_dis = Get_TotalDistance()/10;
		
	}

}	

static void ble_uart_ccommand_application_delete_data(void)
{

	uint32_t command[2];
	cod_ble_data_type type;

	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));			
	
	type = (cod_ble_data_type)command[1];

	if (type == DATA_TYPE_STEP)
	{
		delete_all_day_steps();

	}
	//if (type == DATA_TYPE_SPORT)
	//{
	//	delete_sport_data();
	//}

	uint32_t response[2]= {BLE_UART_RESPONSE_APPLICATION_DELETE_DATA | BLE_UART_CCOMMAND_MASK_APPLICATION};
	response[1] = type;	
	drv_ble_uart_put((uint8_t *)response, sizeof(response));
}	


static void ble_uart_ccommand_application_mac_read(void)
{
	_ble_mac ble_mac;
	memcpy((uint8_t *)&ble_mac,(uint8_t *)m_task_buffer.p_data, sizeof(_ble_mac));
	drv_ble_uart_close();
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_mac %x:%x:%x:%x:%x:%x\r\n",ble_mac.mac[0],ble_mac.mac[1],ble_mac.mac[2],ble_mac.mac[3],ble_mac.mac[4],ble_mac.mac[5]);
	ble_mac.cmd = PC_UART_RESPONSE_APPLICATION_MAC_READ | PC_UART_COMMAND_MASK_APPLICATION;
	drv_pc_send_data((uint8_t *)&ble_mac, sizeof(_ble_mac));	
	
	
}	


static void ble_uart_ccommand_application_mac_write(void)
{
	_ble_mac ble_mac;
	memcpy((uint8_t *)&ble_mac,(uint8_t *)m_task_buffer.p_data, sizeof(_ble_mac));
	drv_ble_uart_close();
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_mac %x:%x:%x:%x:%x:%x\r\n",ble_mac.mac[0],ble_mac.mac[1],ble_mac.mac[2],ble_mac.mac[3],ble_mac.mac[4],ble_mac.mac[5]);
	ble_mac.cmd = PC_UART_RESPONSE_APPLICATION_MAC_WRITE | PC_UART_COMMAND_MASK_APPLICATION;
	drv_pc_send_data((uint8_t *)&ble_mac, sizeof(_ble_mac));	

}	
 
static void ble_uart_ccommand_application_update_apgs_time(void)
{
	uint32_t agps_size;
	memcpy((uint8_t *)&agps_size,(uint8_t *)(m_task_buffer.p_data+1), sizeof(agps_size));
	drv_ble_uart_close();
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: %s,agps_size %d\r\n",__func__,agps_size);
	am_hal_rtc_time_get(&RTC_time);
	SetValue.agps_update_time.Year = RTC_time.ui32Year;
	SetValue.agps_update_time.Month= RTC_time.ui32Month;
	SetValue.agps_update_time.Day= RTC_time.ui32DayOfMonth;
	SetValue.agps_update_time.Hour= RTC_time.ui32Hour;
	SetValue.agps_update_time.Minute= RTC_time.ui32Minute;
	SetValue.agps_update_time.Second= RTC_time.ui32Second;

        #ifdef WATCH_SONY_GNSS 
          SetValue.agps_update_flag = 1;//需要更新
          TASK_BLE_LOG_PRINTF("[AGPS UPDATE FLAG] %d\r\n",SetValue.agps_update_flag);
        #endif  
}	

static void ble_uart_ccommand_application_update_bp_start(void)
{
	uint32_t response[1]= {BLE_UART_RESPONSE_APPLICATION_UPDATA_BP_START | BLE_UART_CCOMMAND_MASK_APPLICATION};
	drv_ble_uart_put((uint8_t *)response, sizeof(response));

	mode_close();
	DISPLAY_MSG  msg = {0,0};
	ScreenState = DISPLAY_SCREEN_UPDATE_BP;
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	SetValue.Theme = 0;
	com_dial_get(SetValue.Theme, &DialData);//默认表盘
}	

static void ble_uart_ccommand_application_update_bp_finish(void)
{
	drv_ble_uart_close();
	gui_dfu_timer_stop();
	
	if (com_dial_is_valid(4))
	{
		SetValue.Theme = 4;
	}
	else
	{
		SetValue.Theme = 0;
	}
	
	//清除数据
	memset(&DialData, 0, sizeof(DialDataStr));
	//获取表盘数据
	com_dial_get(SetValue.Theme, &DialData);

	DISPLAY_MSG  msg = {0,0};
	ScreenState = DISPLAY_SCREEN_HOME;
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

}	

static void ble_uart_ccommand_application_real_time_log_trans(void)
{
	uint32_t command;
	memcpy((uint8_t *)&command,(uint8_t *)(m_task_buffer.p_data+1), sizeof(command));			

 	real_time_log_flag = command;
	
	cod_uart_normal_open = real_time_log_flag;
	TASK_BLE_LOG_PRINTF("%s,%d\r\n",__func__,real_time_log_flag);
	drv_ble_uart_close();
	
}

#endif



static void ble_uart_ccommand_application_ble_connect(void)								
{

		drv_ble_uart_close();

	
//	_task_ble_conn_evt *task_ble_conn_evt = (_task_ble_conn_evt *)m_task_buffer.p_data;
//	memcpy(g_peer_addr,task_ble_conn_evt->addr,BLE_GAP_ADDR_LEN);
	
	SetValue.SwBle = BLE_CONNECT;
	if((ScreenState != DISPLAY_SCREEN_LOGO)
		&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
	  && (ScreenState != DISPLAY_SCREEN_BLE_DFU)
		&& time_notify_is_ok())
	{

	  // if(((ScreenStateSave<=DISPLAY_VICE_SCREEN_SPORT_HINT)&&(ScreenStateSave>=DISPLAY_VICE_SCREEN_OUTDOOR_RUN))||((ScreenStateSave<=DISPLAY_SCREEN_TRAIN_CAST_PRESS_REMIND)&&(ScreenStateSave>=DISPLAY_SCREEN_TRAIN_CAST_START_REMIND)))
    /* 	if(((ScreenStateSave<=DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND)&&(ScreenStateSave>=DISPLAY_SCREEN_TRAIN_CAST))
		||((ScreenStateSave<=DISPLAY_SCREEN_TRAIN_CAST_END_REMIND)&&(ScreenStateSave>=DISPLAY_SCREEN_TRAIN_CAST_START_REMIND)))
	  {


	   mode_close();


		    DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
		     ScreenState = ScreenStateSave;
			 xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}*/
		if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		{
			ScreenStateSave = ScreenState;
			
									
		timer_notify_display_start(3000,1,false);
		
		timer_notify_motor_stop(false);
		timer_notify_buzzer_stop(false);
		
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_NOTIFY_BLE_CONNECT;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			}
	}
}
static void ble_uart_ccommand_application_ble_disconnect(void)
{
#ifdef COD 
		if (SetValue.rt_heart)
		{
			task_hrt_close();
			SetValue.rt_heart = 0;
		}
		cod_sport_cast_flag = 0;
		cod_user_sensor_close();		

//  _task_ble_conn_evt_disconnect *task_ble_conn_evt_disconnect = (_task_ble_conn_evt_disconnect *)m_task_buffer.p_data;
//  g_ble_disconnect_reason = task_ble_conn_evt_disconnect->disconnect_reason;		

	SetValue.SwBle = BLE_DISCONNECT;
	if((ScreenState != DISPLAY_SCREEN_LOGO)
		&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
		&& (ScreenState != DISPLAY_SCREEN_BLE_DFU)
		&& time_notify_is_ok())
	{
    // if(((ScreenStateSave<=DISPLAY_VICE_SCREEN_SPORT_HINT)&&(ScreenStateSave>=DISPLAY_VICE_SCREEN_OUTDOOR_RUN))||((ScreenStateSave<=DISPLAY_SCREEN_TRAIN_CAST_PRESS_REMIND)&&(ScreenStateSave>=DISPLAY_SCREEN_TRAIN_CAST_START_REMIND)))

		if(((ScreenState<=DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND)&&(ScreenState>=DISPLAY_SCREEN_TRAIN_CAST))
			||((ScreenState<=DISPLAY_SCREEN_TRAIN_CAST_END_REMIND)&&(ScreenState>=DISPLAY_SCREEN_TRAIN_CAST_START_REMIND)))

		{

		if((ScreenState==DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND)||(ScreenState==DISPLAY_SCREEN_REMIND_VICE_PAUSE_CONTINUE)||(ScreenState==DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND))
			{
              timer_app_vice_pasue_stop(false);
	          set_train_pause_time(); 

		   }
			DISPLAY_MSG  msg = {0,0};
		    msg.cmd = MSG_DISPLAY_SCREEN;
			ScreenState = DISPLAY_SCREEN_HOME;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
		else if (ScreenState == DISPLAY_SCREEN_UPDATE_BP)
	 	{
			ScreenStateSave = DISPLAY_SCREEN_HOME;
			gui_dfu_timer_stop();
			timer_notify_display_start(3000,1,false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			ScreenState = DISPLAY_SCREEN_NOTIFY_BLE_DISCONNECT;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	 	}
		else if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		{
			ScreenStateSave = ScreenState;
			timer_notify_display_start(3000,1,false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			ScreenState = DISPLAY_SCREEN_NOTIFY_BLE_DISCONNECT;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
							
	}
	else if (ScreenState == DISPLAY_SCREEN_BLE_DFU)
	{
			ScreenStateSave = DISPLAY_SCREEN_HOME;
			gui_dfu_timer_stop();
			timer_notify_display_start(3000,1,false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			ScreenState = DISPLAY_SCREEN_NOTIFY_BLE_DISCONNECT;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}
#else
	drv_ble_uart_close();
	SetValue.SwBle = BLE_DISCONNECT;

	if((ScreenState != DISPLAY_SCREEN_LOGO)
		&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
		&& (ScreenState != DISPLAY_SCREEN_BLE_DFU)
		&& time_notify_is_ok())
	{
		if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		{
			ScreenStateSave = ScreenState;
		}
		timer_notify_display_start(3000,1,false);
		
//		timer_notify_motor_start(1000,500,1,false,2);
//		timer_notify_buzzer_start(1000,500,1,false,2);
		timer_notify_motor_stop(false);
		timer_notify_buzzer_stop(false);
		
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_NOTIFY_BLE_DISCONNECT;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	}

#endif

}

static void ble_uart_ccommand_application_ble_bond(void)
{
	drv_ble_uart_close();	
	SetValue.SwBle = BLE_BONDED;
	if((ScreenState != DISPLAY_SCREEN_LOGO)
		&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
		&& (ScreenState != DISPLAY_SCREEN_BLE_DFU)
		&& time_notify_is_ok())
	{
		if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		{
			ScreenStateSave = ScreenState;
		}

		timer_notify_display_start(3000,1,false);
		
//		timer_notify_motor_start(1000,500,1,false,2);
//		timer_notify_buzzer_start(1000,500,1,false,2);
		timer_notify_motor_stop(false);
		timer_notify_buzzer_stop(false);
		
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_NOTIFY_BLE_BOND;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
	}	
}
static void ble_uart_ccommand_application_ble_passkey(void)								
{
	
	memcpy((uint8_t *)&g_ble_passkey,(uint8_t *)m_task_buffer.p_data,sizeof(_ble_passkey));
	drv_ble_uart_close();
	if((ScreenState != DISPLAY_SCREEN_LOGO)
		&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
		&& (ScreenState != DISPLAY_SCREEN_BLE_DFU)
		&& time_notify_is_ok())
	{
		if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		{
			ScreenStateSave = ScreenState;
		}
		timer_notify_display_start(10000,1,false);
		
//		timer_notify_motor_start(1000,500,1,false,2);
//		timer_notify_buzzer_start(1000,500,1,false,2);
		timer_notify_motor_stop(false);
		timer_notify_buzzer_stop(false);
		
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_NOTIFY_BLE_PASSKEY;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
	}	
}

static void ble_uart_response_application_jump_bootloader(void)
{
	/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
 
     if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
	}
}	

static void ble_uart_response_application_reset(void)
{
	/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/

     if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
	}
}

static void ble_uart_response_bootloader_bootsetting(void)
{
	/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/

     if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
	}
}	

static void ble_uart_response_bootloader_prevalidate(void)
{
	/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/

    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
	}
}	
static void ble_uart_response_bootloader_postvalidate(void)
{
	/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/

    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
	}
}	

static void ble_uart_response_bootloader_reset_n_activate(void)
{
	/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/

    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)m_task_buffer.p_data, m_task_buffer.length);
	}
}	
static void pc_uart_ccommand_application_jump_bootloader(void)
{
	uint32_t command[1] = {PC_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER | PC_UART_COMMAND_MASK_APPLICATION};
	drv_ble_uart_put((uint8_t *)&command, sizeof(command));	
	m_event = JUMP_BOOTLOADER;	
}	

static void pc_uart_command_hardware_test_ble_info_read_response(void)
{
	
	_task_ble_info task_ble_info;
	/**<Check if the USB dectect pin is active, the ble info should forward to pc>*/
	memcpy((uint8_t *)&task_ble_info,(uint8_t *)m_task_buffer.p_data,sizeof(_task_ble_info));
	drv_ble_uart_close();
	

    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)&task_ble_info, sizeof(_task_ble_info));
	}
	_lib_app_data lib_app_data;
	lib_app_data.ble.setting_version = task_ble_info.setting_version;
	lib_app_data.ble.bootloader_version = task_ble_info.bootloader_version;
	lib_app_data.ble.application_version = task_ble_info.application_version;
	lib_app_data.ble.softdevice_version = task_ble_info.softdevice_version;
	lib_app_data.ble.pcb_version = task_ble_info.pcb_version;
	lib_app_data.ble.mac_address_f4b = task_ble_info.mac_address_f4b;
	lib_app_data.ble.mac_address_l2b = task_ble_info.mac_address_l2b;
	memset(lib_app_data.ble.name,0,sizeof(lib_app_data.ble.name));
	memcpy(lib_app_data.ble.name,task_ble_info.dev_name.name,task_ble_info.dev_name.len);		
	lib_app_data.ble.name_len = task_ble_info.dev_name.len;
	lib_app_data.crc = am_bootloader_fast_crc32((uint8_t*)(&lib_app_data.crc)+sizeof(lib_app_data.crc),sizeof(_lib_app_data)-sizeof(lib_app_data.crc));
	lib_app_data_write(&lib_app_data);	
}	

static void pc_uart_command_hardware_test_ble_rssi_read_response(void)
{
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));	
	drv_ble_uart_close();
    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
	}
	
	
}

static void pc_uart_command_hardware_test_ble_rssi_read_stop_response(void)
{
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));		
	drv_ble_uart_close();

    if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
	{
		drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
	}
}	

static void pc_uart_command_hardware_test_ble_extflash_read_response(void)
{
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));		
	drv_ble_uart_close();

   if(am_hal_gpio_input_bit_read(DRV_BAT_USB_DETECT_PIN))
    {
		drv_pc_send_data((uint8_t *)buffer, sizeof(buffer));
	}
}
static void ble_uart_ccommand_application_dev_name_update(void)
{
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));	
	drv_ble_uart_close();
	_task_ble_dev_name_update *dev_name_update = (_task_ble_dev_name_update *)buffer;
	_lib_app_data lib_app_data;
	lib_app_data_read(&lib_app_data);
	memset(lib_app_data.ble.name,0,sizeof(lib_app_data.ble.name));
	memcpy(lib_app_data.ble.name,dev_name_update->name,dev_name_update->len);		
	lib_app_data.ble.name_len = dev_name_update->len;
	lib_app_data.crc = am_bootloader_fast_crc32((uint8_t*)(&lib_app_data.crc)+sizeof(lib_app_data.crc),sizeof(_lib_app_data)-sizeof(lib_app_data.crc));
	lib_app_data_write(&lib_app_data);
	if(ScreenState == DISPLAY_SCREEN_WATCH_INFO)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_WATCH_INFO;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);									
	}
}	

static void ble_uart_ccommand_application_current_time_write(void)								
{
	am_hal_rtc_time_t m_before_time,m_now_time; //授时前RTC时间和要设置到RTC的时间
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
	rtc_time_t AppTime,WatchTime;
#endif
	_ble_current_time *ble_app_time = (_ble_current_time *)(m_task_buffer.p_data);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc 0x%08X\n",ble_app_time->crc);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: year 20%d\n",ble_app_time->year);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: month %d\n",ble_app_time->month);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: date %d\n",ble_app_time->date);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: day_of_week %d\n",ble_app_time->day_of_week);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: hour %d\n",ble_app_time->hour);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: minute %d\n",ble_app_time->minute);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: second %d\n",ble_app_time->second);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: time_zone %d\n",ble_app_time->time_zone);
	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&ble_app_time->crc)+sizeof(ble_app_time->crc),  
														sizeof(_ble_current_time)-sizeof(ble_app_time->crc)-sizeof(ble_app_time->command));		
	if(crc_dest == ble_app_time->crc)
	{
		#if !defined COD
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x00;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));
		#endif
	}
	else{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		#ifdef COD
		response[1] = 0x00;
		#else
		response[1] = 0x01;
		#endif
		drv_ble_uart_put((uint8_t *)response, sizeof(response));
		return;
	}
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
	AppTime.Year = ble_app_time->year;
	AppTime.Month = ble_app_time->month;
	AppTime.Day = ble_app_time->date;
	AppTime.Hour = ble_app_time->hour;
	AppTime.Minute = ble_app_time->minute;
	AppTime.Second = ble_app_time->second;
	
	WatchTime.Year = RTC_time.ui32Year;
	WatchTime.Month = RTC_time.ui32Month;
	WatchTime.Day = RTC_time.ui32DayOfMonth;
	WatchTime.Hour = RTC_time.ui32Hour;
	WatchTime.Minute = RTC_time.ui32Minute;
	WatchTime.Second = RTC_time.ui32Second;

	uint8_t m_timecalibration_status = ReadCalibrationValueAndConfigAdjValue();
	Set_TimeCalibrationStatus(m_timecalibration_status);//设置精密校准状态
	if(IsApptoWatchTime(AppTime,WatchTime) && Get_TimeCalibrationStatus() == STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION)
#else
	if(IsApptoWatchTime())
#endif
	{
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
		TASK_BLE_LOG_PRINTF("[TASK_BLE]:App can change watch time.Watchtime->%d-%d-%d %d:%d:%d Get_TimeCalibrationStatus=%d\n",
			RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,RTC_time.ui32Hour,
			RTC_time.ui32Minute,RTC_time.ui32Second,Get_TimeCalibrationStatus());
#else
		TASK_BLE_LOG_PRINTF("[TASK_BLE]:App can change watch time.Watchtime->%d-%d-%d %d:%d:%d ScreenState=%d\n",
			RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,RTC_time.ui32Hour,
			RTC_time.ui32Minute,RTC_time.ui32Second,ScreenState);
#endif
		am_hal_rtc_time_get(&m_before_time);

		RTC_time.ui32Hour = ble_app_time->hour;
		RTC_time.ui32Minute = ble_app_time->minute;
		RTC_time.ui32Second = ble_app_time->second;
		RTC_time.ui32DayOfMonth = ble_app_time->date;
		RTC_time.ui32Month = ble_app_time->month;
		RTC_time.ui32Year = ble_app_time->year;
		RTC_time.ui32Weekday = ble_app_time->day_of_week;
		RTC_time.ui32Century = 0;
		RTC_time.ui32Hundredths = 0;	

		am_hal_rtc_time_set(&RTC_time);
		m_now_time = RTC_time;
		RenewOneDay( m_before_time,m_now_time,true );
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
		set_s_set_time_resource(SET_TIME_RESOURCE_BY_BLE);//设置RTC时间来源--蓝牙
#endif
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
		am_hal_rtc_time_get(&RTC_time);
		if(Get_TimeCalibrationStatus() == STATUS_FLASH_HAS_ADJ_HAS_TCALIBRATION)
		{//修改的时间 如果精密时间校准ok 才更新区域2中时间为了下次补偿用
			TimeCalibrationStr m_calibration_time;
			
			m_calibration_time = Get_TimeCalibrationStr();
			
			m_calibration_time.CalibrationSaveTime.Year = RTC_time.ui32Year;
			m_calibration_time.CalibrationSaveTime.Month = RTC_time.ui32Month;
			m_calibration_time.CalibrationSaveTime.Day = RTC_time.ui32DayOfMonth;
			m_calibration_time.CalibrationSaveTime.Hour = RTC_time.ui32Hour;
			m_calibration_time.CalibrationSaveTime.Minute = RTC_time.ui32Minute;
			m_calibration_time.CalibrationSaveTime.Second = RTC_time.ui32Second;
			m_calibration_time.CalibrationSaveTime.Msecond = RTC_time.ui32Hundredths;
			Set_TimeCalibrationStr(m_calibration_time);
			SaveCalibrationData();
		}
#endif
#ifdef COD
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x01;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));
#endif

		if(ScreenState == DISPLAY_SCREEN_HOME)
		{
			DISPLAY_MSG  msg = {0};
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);									
		}

	}
	else
	{
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
		TASK_BLE_LOG_PRINTF("[TASK_BLE]:App can not change watch time!Watchtime->%d-%d-%d %d:%d:%d Get_TimeCalibrationStatus=%d\n",
			RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,RTC_time.ui32Hour,
			RTC_time.ui32Minute,RTC_time.ui32Second,Get_TimeCalibrationStatus());
#else
		TASK_BLE_LOG_PRINTF("[TASK_BLE]:App can not change watch time!Watchtime->%d-%d-%d %d:%d:%d ScreenState=%d\n",
			RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,RTC_time.ui32Hour,
			RTC_time.ui32Minute,RTC_time.ui32Second,ScreenState);
#ifdef COD
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x00;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));
#endif		
#endif
	}
}

static void ble_uart_ccommand_application_weather_info_request(void)
{
#ifdef COD
	CodWeatherSync WeatherInfo;

	CodWeatherSync WeatherSave;

	dev_extFlash_enable();			
	dev_extFlash_read(WEATHER_DATA_START_ADDRESS, (uint8_t *)&WeatherInfo, sizeof(CodWeatherSync));			
	dev_extFlash_disable();
	
	memcpy((uint8_t *)WeatherInfo.cod_weather,(uint8_t *)(m_task_buffer.p_data + 1), sizeof(cod_ble_weather)*24);
	uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_WEATHER_INFO_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	response[1] = 0x00;
	drv_ble_uart_put((uint8_t *)response, sizeof(response));

	
	am_hal_rtc_time_get(&RTC_time);
	WeatherInfo.date[0]= RTC_time.ui32Year;
	WeatherInfo.date[1]= RTC_time.ui32Month;
	WeatherInfo.date[2]= RTC_time.ui32DayOfMonth;
	WeatherInfo.date[3]= RTC_time.ui32Hour;

	//先读取flash天气数据，有更新时才会去写flash
	dev_extFlash_enable();			
	dev_extFlash_read(WEATHER_DATA_START_ADDRESS, (uint8_t *)&WeatherSave, sizeof(CodWeatherSync));			
	dev_extFlash_disable();
	
	TASK_BLE_LOG_PRINTF("y:%d;m:%d,d:%d,h:%d;t:%d;w:%d\r\n",WeatherSave.date[0],WeatherSave.date[1],WeatherSave.date[2],WeatherSave.date[3],WeatherSave.cod_weather[0].temperature,WeatherSave.cod_weather[0].weather);
	if ((WeatherInfo.date[0]!= WeatherSave.date[0])||(WeatherInfo.date[1] != WeatherSave.date[1]) ||(WeatherInfo.date[2] != WeatherSave.date[2])  ||(WeatherInfo.date[3] != WeatherSave.date[3]))
	{
		dev_extFlash_enable();		
		dev_extFlash_erase(WEATHER_DATA_START_ADDRESS,sizeof(CodWeatherSync));
		dev_extFlash_write(WEATHER_DATA_START_ADDRESS, (uint8_t *)&WeatherInfo, sizeof(CodWeatherSync));			
		dev_extFlash_disable();
		
		gui_tool_calendar_weather_info_init(); 
	
		if(ScreenState == DISPLAY_SCREEN_CALENDAR)
		{
			DISPLAY_MSG  msg = {0};
			ScreenState = DISPLAY_SCREEN_CALENDAR;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);									
		}	
		
	}
#else
	AppWeatherSync *WeatherSync = (AppWeatherSync *)(m_task_buffer.p_data + 1);
	AppWeatherSync WeatherInfo;


	if(WeatherSync->day_count == 0)
	{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_WEATHER_INFO_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x02;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));		
		return;		
	}
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&WeatherSync->crc)+sizeof(WeatherSync->crc),  
	sizeof(AppWeatherSync)-sizeof(WeatherSync->crc)-sizeof(AppWeatherSyncDaily)*((sizeof(WeatherSync->weather_array)/sizeof(WeatherSync->weather_array[0])) - WeatherSync->day_count));	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:crc_dest --> 0x%08X\n",crc_dest);
	
	if(crc_dest == WeatherSync->crc)
	{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_WEATHER_INFO_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x00;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));
		//先读取flash天气数据，有更新时才会去写flash
		dev_extFlash_enable();			
		dev_extFlash_read(WEATHER_DATA_START_ADDRESS, (uint8_t *)&WeatherInfo, sizeof(AppWeatherSync));			
		dev_extFlash_disable();
		
		if (WeatherInfo.crc != crc_dest)
		{
			dev_extFlash_enable();		
			dev_extFlash_erase(WEATHER_DATA_START_ADDRESS,sizeof(AppWeatherSync));
			dev_extFlash_write(WEATHER_DATA_START_ADDRESS, (uint8_t *)WeatherSync, sizeof(AppWeatherSync));			
			dev_extFlash_disable();
		}

		gui_tool_calendar_weather_info_init();
		
		if(ScreenState == DISPLAY_SCREEN_CALENDAR)
		{
			DISPLAY_MSG  msg = {0};
			ScreenState = DISPLAY_SCREEN_CALENDAR;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);									
		}		
	}
	else{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_WEATHER_INFO_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x01;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));	
	}
#endif
}	

static void ble_uart_ccommand_application_personal_info_read(void)
{
#ifdef COD	
	_ble_user_info	cod_user_info;
	 cod_user_info.command = BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_READ | BLE_UART_CCOMMAND_MASK_APPLICATION;;
     cod_user_info.gender =   SetValue.AppSet.Sex;
	 cod_user_info.age = SetValue.AppSet.Age ;
	 cod_user_info.height= SetValue.AppSet.Height;
	 cod_user_info.weight = SetValue.AppSet.Weight;
	 drv_ble_uart_put((uint8_t *)&cod_user_info, sizeof(cod_user_info));
#else
	_ble_personal_info ble_personal_info;
	ble_personal_info.command = BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_READ | BLE_UART_CCOMMAND_MASK_APPLICATION;
	ble_personal_info.Sex = SetValue.AppSet.Sex;
	ble_personal_info.Age = SetValue.AppSet.Age;
	ble_personal_info.Height = SetValue.AppSet.Height;
	ble_personal_info.Weight = SetValue.AppSet.Weight;
	ble_personal_info.Stride = SetValue.AppSet.Stride;
	ble_personal_info.VO2Max = SetValue.AppSet.VO2Max;
	ble_personal_info.LactateThresholdSpeed = SetValue.AppSet.LactateThresholdSpeed;
	ble_personal_info.LactateThresholdHeartrate = SetValue.AppSet.LactateThresholdHeartrate;	
	ble_personal_info.htr_zone_one_min = SetValue.AppSet.htr_zone_one_min;
	ble_personal_info.htr_zone_one_max = SetValue.AppSet.htr_zone_one_max;
	ble_personal_info.htr_zone_two_min = SetValue.AppSet.htr_zone_two_min;
	ble_personal_info.htr_zone_two_max = SetValue.AppSet.htr_zone_two_max;
	ble_personal_info.htr_zone_three_min = SetValue.AppSet.htr_zone_three_min;
	ble_personal_info.htr_zone_three_max = SetValue.AppSet.htr_zone_three_max;
	ble_personal_info.htr_zone_four_min = SetValue.AppSet.htr_zone_four_min;	
	ble_personal_info.htr_zone_four_max = SetValue.AppSet.htr_zone_four_max;
	ble_personal_info.htr_zone_five_min = SetValue.AppSet.htr_zone_five_min;
	ble_personal_info.htr_zone_five_max = SetValue.AppSet.htr_zone_five_max;
	ble_personal_info.crc = am_bootloader_fast_crc32((uint8_t*)(&ble_personal_info.crc)+sizeof(ble_personal_info.crc),  
														sizeof(_ble_personal_info)-sizeof(ble_personal_info.crc)-sizeof(ble_personal_info.command));		
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc 0x%08X\n",ble_personal_info.crc);	
	drv_ble_uart_put((uint8_t *)&ble_personal_info, sizeof(ble_personal_info));
#endif	
}

static void ble_uart_ccommand_application_personal_info_write(void)
{
#ifdef COD 
	_ble_user_info 	cod_user_info;
	memcpy((uint8_t *)&cod_user_info,(uint8_t *)m_task_buffer.p_data, sizeof(cod_user_info));
	SetValue.AppSet.Sex = cod_user_info.gender;
	SetValue.AppSet.Age = cod_user_info.age;
	SetValue.AppSet.Height = cod_user_info.height;
	SetValue.AppSet.Weight = cod_user_info.weight;

	uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	response[1] = 0x00;
	drv_ble_uart_put((uint8_t *)response, sizeof(response));
	
#else
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));		
	_ble_personal_info *ble_personal_info = (_ble_personal_info *)(buffer);

	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&ble_personal_info->crc)+sizeof(ble_personal_info->crc),  
														sizeof(_ble_personal_info)-sizeof(ble_personal_info->crc)-sizeof(ble_personal_info->command));	

	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc_dest 0x%08X\n",crc_dest);	
	if(crc_dest == ble_personal_info->crc)
	{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x00;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));	
	}
	else{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x01;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));
		return;
	}	
	
	SetValue.AppSet.Sex = ble_personal_info->Sex;
	SetValue.AppSet.Age = ble_personal_info->Age;
	SetValue.AppSet.Height = ble_personal_info->Height;
	SetValue.AppSet.Weight = ble_personal_info->Weight;
	SetValue.AppSet.Stride = ble_personal_info->Stride;
	SetValue.AppSet.VO2Max = ble_personal_info->VO2Max;
	SetValue.AppSet.LactateThresholdSpeed = ble_personal_info->LactateThresholdSpeed;
	SetValue.AppSet.LactateThresholdHeartrate = ble_personal_info->LactateThresholdHeartrate;
	SetValue.AppSet.htr_zone_one_min = ble_personal_info->htr_zone_one_min;
	SetValue.AppSet.htr_zone_one_max = ble_personal_info->htr_zone_one_max;
	SetValue.AppSet.htr_zone_two_min = ble_personal_info->htr_zone_two_min;
	SetValue.AppSet.htr_zone_two_max = ble_personal_info->htr_zone_two_max;
	SetValue.AppSet.htr_zone_three_min = ble_personal_info->htr_zone_three_min;
	SetValue.AppSet.htr_zone_three_max = ble_personal_info->htr_zone_three_max;
	SetValue.AppSet.htr_zone_four_min = ble_personal_info->htr_zone_four_min;
	SetValue.AppSet.htr_zone_four_max = ble_personal_info->htr_zone_four_max;
#if defined	(WATCH_SIM_SPORT)
	if (SetValue.HeartRateSet != 1)
	{
		SetValue.AppSet.htr_zone_five_min = ble_personal_info->htr_zone_five_min;
	}
#else
	SetValue.AppSet.htr_zone_five_min = ble_personal_info->htr_zone_five_min;
#endif
	SetValue.AppSet.htr_zone_five_max = ble_personal_info->htr_zone_five_max;

	//运动步长根据身高调整，如增加校准后再根据是否校准确定是否同步
	if(SetValue.AppSet.Height > 59)
	{
		SetValue.AppGet.StepLengthRun = SetValue.AppSet.Height*60/100 ;//cm
		SetValue.AppGet.StepLengthCountryRace = SetValue.AppSet.Height*60/100;
		SetValue.AppGet.StepLengthMarathon = SetValue.AppSet.Height*60/100;
		SetValue.AppGet.StepLengthWalking = SetValue.AppSet.Height*45/100 ;
		SetValue.AppGet.StepLengthClimbing = SetValue.AppSet.Height*45/100 ;
#if defined	(WATCH_SIM_SPORT) || defined	(WATCH_COM_SPORT)
		if (SetValue.IndoorLengSet != 1)
		{
			SetValue.AppGet.StepLengthIndoorRun = SetValue.AppSet.Height*60/100;
		}
#else
		SetValue.AppGet.StepLengthIndoorRun = SetValue.AppSet.Height*60/100;
#endif
		SetValue.AppGet.StepLengthHike = SetValue.AppSet.Height*45/100 ;
	}
	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: Height %d,StepLengthRun %d,StepLengthWalking %d\n",SetValue.AppSet.Height,SetValue.AppGet.StepLengthRun,SetValue.AppGet.StepLengthWalking);	
#endif
}

static void ble_uart_ccommand_application_sports_param_read(void)
{
	_ble_sports_param ble_sports_param;
	memset(&ble_sports_param,0,sizeof(ble_sports_param));
	ble_sports_param.command = BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_READ | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
	ble_sports_param.sports_type_mask.bits.daily = 1;
	ble_sports_param.daily.goal_step =  SetValue.AppSet.Goal_Steps;
	ble_sports_param.daily.goal_calory = SetValue.AppSet.Goal_Energy ;
	ble_sports_param.daily.goal_distance = SetValue.AppSet.Goal_Distance;

	ble_sports_param.run.swtch.value = SetValue.SportSet.SwRun;
    ble_sports_param.run.goal_pace = SetValue.SportSet.DefHintSpeedRun;
    ble_sports_param.run.goal_distance = SetValue.SportSet.DefHintDistanceRun;
    ble_sports_param.run.lap_count_distance = SetValue.SportSet.DefCircleDistanceRun;

	ble_sports_param.marithon.swtch.value  = SetValue.SportSet.SwMarathon;
    ble_sports_param.marithon.goal_pace = SetValue.SportSet.DefHintSpeedMarathon;
    ble_sports_param.marithon.goal_distance =   SetValue.SportSet.DefHintDistanceMarathon;
    ble_sports_param.marithon.lap_count_distance = SetValue.SportSet.DefCircleDistanceMarathon;

	ble_sports_param.country_race.swtch.value = SetValue.SportSet.SwCountryRace;
    ble_sports_param.country_race.goal_pace = SetValue.SportSet.DefHintSpeedCountryRace;
    ble_sports_param.country_race.goal_distance = SetValue.SportSet.DefHintDistanceCountryRace;
    ble_sports_param.country_race.lap_count_distance = SetValue.SportSet.DefCircleDistanceCountryRace;

	ble_sports_param.indoor_swim.swtch.value = SetValue.SportSet.SwSwimmg;
    ble_sports_param.indoor_swim.goal_lap =  SetValue.SportSet.DefHintCircleSwimming ;
    ble_sports_param.indoor_swim.goal_time_sec = SetValue.SportSet.DefHintTimeSwimming;
	ble_sports_param.indoor_swim.pool_len = SetValue.AppGet.LengthSwimmingPool;

	ble_sports_param.cycling.swtch.value = SetValue.SportSet.SwCycling;
    ble_sports_param.cycling.goal_distance =  SetValue.SportSet.DefHintDistanceCycling;
    ble_sports_param.cycling.goal_pace = SetValue.SportSet.DefHintSpeedCycling;
    ble_sports_param.cycling.lap_count_distance =  SetValue.SportSet.DefCircleDistanceCycling;
  //  ble_sports_param.cycling.wheel_radius= SetValue.SportSet.WheelDiameter;

	ble_sports_param.mountain_climb.swtch.value =SetValue.SportSet.SwClimbing;
    ble_sports_param.mountain_climb.goal_alivation = SetValue.SportSet.DefHintClimbingHeight;
    ble_sports_param.mountain_climb.goal_distance = SetValue.SportSet.DefHintDistanceClimbing;
    ble_sports_param.mountain_climb.lap_count_distance = SetValue.SportSet.DefCircleDistanceClimbing;

	ble_sports_param.walking.swtch.value = SetValue.SportSet.SwWalking;
    ble_sports_param.walking.goal_calory = SetValue.SportSet.DefWalkingFatBurnGoal;
    ble_sports_param.walking.goal_time_sec =  SetValue.SportSet.DefHintTimeWalking;
    ble_sports_param.walking.lap_count_distance =  SetValue.SportSet.DefCircleDistanceWalk;

	ble_sports_param.country_walk.swtch.value = SetValue.SportSet.SwHike;
    ble_sports_param.country_walk.goal_calory = SetValue.SportSet.DefHikeFatBurnGoal;
    ble_sports_param.country_walk.lap_count_distance = SetValue.SportSet.DefCircleDistanceHike;

	ble_sports_param.indoor_run.swtch.value = SetValue.SportSet.SwIndoorRun;
    ble_sports_param.indoor_run.goal_pace = SetValue.SportSet.DefHintSpeedIndoorRun;
    ble_sports_param.indoor_run.goal_distance = SetValue.SportSet.DefHintDistanceIndoorRun;
    ble_sports_param.indoor_run.lap_count_distance = SetValue.SportSet.DefCircleDistanceIndoorRun;

	ble_sports_param.triathonal.outdoor_swim.swtch.value = SetValue.SportSet.SwOutdoorSwimmg ;
    ble_sports_param.triathonal.outdoor_swim.goal_time_sec =  SetValue.SportSet.DefHintTimeOutdoorSwimmg ;
    ble_sports_param.triathonal.outdoor_swim.lap_count_distance = SetValue.SportSet.DefCircleDistancOutdoorSwimmg;
	
	ble_sports_param.crc = am_bootloader_fast_crc32((uint8_t*)(&ble_sports_param.crc)+sizeof(ble_sports_param.crc),  
														sizeof(_ble_sports_param)-sizeof(ble_sports_param.crc)-sizeof(ble_sports_param.command));		

	drv_ble_uart_put((uint8_t *)&ble_sports_param, sizeof(ble_sports_param));	
}	



static void ble_uart_ccommand_application_sports_param_write(void)
{
#ifdef COD 
	uint32_t command[2];
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));			
	
	SetValue.AppSet.Goal_Steps = command[1];
	
	uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	response[1] = 0x00;
	drv_ble_uart_put((uint8_t *)response, sizeof(response));
	
#else
	_ble_sports_param *ble_sports_param = (_ble_sports_param *)(m_task_buffer.p_data);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param crc 0x%08X\n",ble_sports_param->crc);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: m_task_buffer.p_data %x\n",m_task_buffer.p_data);	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: size of m_task_buffer.p_data %d\n",sizeof(m_task_buffer.p_data));
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: length of m_task_buffer %d\n",m_task_buffer.length);	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&ble_sports_param->crc)+sizeof(ble_sports_param->crc),  
														sizeof(_ble_sports_param)-sizeof(ble_sports_param->crc)-sizeof(ble_sports_param->command));	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc_dest %x\n",crc_dest);		
	if(crc_dest == ble_sports_param->crc)
	{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x00;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));	

		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.daily --> %d\n",ble_sports_param->sports_type_mask.bits.daily);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.run --> %d\n",ble_sports_param->sports_type_mask.bits.run);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.marithon --> %d\n",ble_sports_param->sports_type_mask.bits.marithon);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.country_race --> %d\n",ble_sports_param->sports_type_mask.bits.country_race);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.indoor_swim --> %d\n",ble_sports_param->sports_type_mask.bits.indoor_swim);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.cycling --> %d\n",ble_sports_param->sports_type_mask.bits.cycling);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.mountain_climb --> %d\n",ble_sports_param->sports_type_mask.bits.mountain_climb);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.walking --> %d\n",ble_sports_param->sports_type_mask.bits.walking);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.country_walk --> %d\n",ble_sports_param->sports_type_mask.bits.country_walk);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.indoor_run --> %d\n",ble_sports_param->sports_type_mask.bits.indoor_run);	
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->sports_type_mask.bits.triathonal --> %d\n",ble_sports_param->sports_type_mask.bits.triathonal);	
		
		
		if(ble_sports_param->sports_type_mask.bits.daily)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: daily\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->daily.goal_step --> %d\n",ble_sports_param->daily.goal_step);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->daily.goal_calory --> %d\n",ble_sports_param->daily.goal_calory);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->daily.goal_distance --> %d\n",ble_sports_param->daily.goal_distance);
      SetValue.AppSet.Goal_Steps = ble_sports_param->daily.goal_step;
	    SetValue.AppSet.Goal_Energy = ble_sports_param->daily.goal_calory;
	    SetValue.AppSet.Goal_Distance = ble_sports_param->daily.goal_distance;//??
		}
		if(ble_sports_param->sports_type_mask.bits.run)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: run\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->run.swtch --> 0x%02X\n",ble_sports_param->run.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->run.goal_pace --> %d\n",ble_sports_param->run.goal_pace);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->run.goal_distance --> %d\n",ble_sports_param->run.goal_distance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->run.lap_count_distance --> %d\n",ble_sports_param->run.lap_count_distance);
      SetValue.SportSet.SwRun = ble_sports_param->run.swtch.value;
      SetValue.SportSet.DefHintSpeedRun = ble_sports_param->run.goal_pace;
      SetValue.SportSet.DefHintDistanceRun = ble_sports_param->run.goal_distance;
      SetValue.SportSet.DefCircleDistanceRun = ble_sports_param->run.lap_count_distance;
		}		
		if(ble_sports_param->sports_type_mask.bits.marithon)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: marithon\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->marithon.swtch --> 0x%02X\n",ble_sports_param->marithon.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->marithon.goal_pace --> %d\n",ble_sports_param->marithon.goal_pace);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->marithon.goal_distance --> %d\n",ble_sports_param->marithon.goal_distance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->marithon.lap_count_distance --> %d\n",ble_sports_param->marithon.lap_count_distance);
      SetValue.SportSet.SwMarathon = ble_sports_param->marithon.swtch.value;
      SetValue.SportSet.DefHintSpeedMarathon = ble_sports_param->marithon.goal_pace;
      SetValue.SportSet.DefHintDistanceMarathon = ble_sports_param->marithon.goal_distance;
      SetValue.SportSet.DefCircleDistanceMarathon = ble_sports_param->marithon.lap_count_distance;
		}		
		if(ble_sports_param->sports_type_mask.bits.country_race)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: country_race\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->country_race.swtch --> 0x%02X\n",ble_sports_param->country_race.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->country_race.goal_pace --> %d\n",ble_sports_param->country_race.goal_pace);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->country_race.goal_distance --> %d\n",ble_sports_param->country_race.goal_distance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->country_race.lap_count_distance --> %d\n",ble_sports_param->country_race.lap_count_distance);
      SetValue.SportSet.SwCountryRace = ble_sports_param->country_race.swtch.value;
      SetValue.SportSet.DefHintSpeedCountryRace = ble_sports_param->country_race.goal_pace;
      SetValue.SportSet.DefHintDistanceCountryRace = ble_sports_param->country_race.goal_distance;
      SetValue.SportSet.DefCircleDistanceCountryRace = ble_sports_param->country_race.lap_count_distance;
		}			
		if(ble_sports_param->sports_type_mask.bits.indoor_swim)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: indoor_swim\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->indoor_swim.swtch --> 0x%02X\n",ble_sports_param->indoor_swim.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->indoor_swim.goal_lap --> %d\n",ble_sports_param->indoor_swim.goal_lap);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->indoor_swim.goal_time_sec --> %d\n",ble_sports_param->indoor_swim.goal_time_sec);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->indoor_swim.pool_len --> %d\n",ble_sports_param->indoor_swim.pool_len);
      SetValue.SportSet.SwSwimmg =ble_sports_param->indoor_swim.swtch.value;
      SetValue.SportSet.DefHintCircleSwimming = ble_sports_param->indoor_swim.goal_lap;
      SetValue.SportSet.DefHintTimeSwimming = ble_sports_param->indoor_swim.goal_time_sec;
//      SetValue.AppGet.LengthSwimmingPool = ble_sports_param->indoor_swim.pool_len;//
		}			
		if(ble_sports_param->sports_type_mask.bits.cycling)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: cycling\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->cycling.swtch --> 0x%02X\n",ble_sports_param->cycling.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->cycling.goal_distance --> %d\n",ble_sports_param->cycling.goal_distance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->cycling.goal_pace --> %d\n",ble_sports_param->cycling.goal_pace);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->cycling.lap_count_distance --> %d\n",ble_sports_param->cycling.lap_count_distance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->cycling.wheel_radius --> %d\n",ble_sports_param->cycling.wheel_radius);	
      SetValue.SportSet.SwCycling =ble_sports_param->cycling.swtch.value;
      SetValue.SportSet.DefHintDistanceCycling = ble_sports_param->cycling.goal_distance;
      SetValue.SportSet.DefHintSpeedCycling = ble_sports_param->cycling.goal_pace;
      SetValue.SportSet.DefCircleDistanceCycling = ble_sports_param->cycling.lap_count_distance;
      SetValue.SportSet.WheelDiameter = ble_sports_param->cycling.wheel_radius;
            
		}				
		if(ble_sports_param->sports_type_mask.bits.mountain_climb)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: mountain_climb\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->mountain_climb.swtch --> 0x%02X\n",ble_sports_param->mountain_climb.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->mountain_climb.goal_alivation --> %d\n",ble_sports_param->mountain_climb.goal_alivation);
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->mountain_climb.goal_distance --> %d\n",ble_sports_param->mountain_climb.goal_distance);
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->mountain_climb.lap_count_distance --> %d\n",ble_sports_param->mountain_climb.lap_count_distance);
      SetValue.SportSet.SwClimbing =ble_sports_param->mountain_climb.swtch.value;
      SetValue.SportSet.DefHintClimbingHeight =ble_sports_param->mountain_climb.goal_alivation;
      SetValue.SportSet.DefHintDistanceClimbing =ble_sports_param->mountain_climb.goal_distance;
      SetValue.SportSet.DefCircleDistanceClimbing =ble_sports_param->mountain_climb.lap_count_distance;
            
		}			
		if(ble_sports_param->sports_type_mask.bits.walking)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: walking\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->walking.swtch --> 0x%02X\n",ble_sports_param->walking.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->walking.goal_calory --> %d\n",ble_sports_param->walking.goal_calory);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->walking.goal_time_sec --> %d\n",ble_sports_param->walking.goal_time_sec);
      SetValue.SportSet.SwWalking =ble_sports_param->walking.swtch.value;
      SetValue.SportSet.DefWalkingFatBurnGoal =ble_sports_param->walking.goal_calory;
      SetValue.SportSet.DefHintTimeWalking =ble_sports_param->walking.goal_time_sec;
      SetValue.SportSet.DefCircleDistanceWalk = ble_sports_param->walking.lap_count_distance;
            
		}				
		if(ble_sports_param->sports_type_mask.bits.country_walk)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: country_walk\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->country_walk.swtch --> 0x%02X\n",ble_sports_param->country_walk.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->country_walk.goal_calory --> %d\n",ble_sports_param->country_walk.goal_calory);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->country_walk.lap_count_distance --> %d\n",ble_sports_param->country_walk.lap_count_distance);
      SetValue.SportSet.SwHike =ble_sports_param->country_walk.swtch.value;
      SetValue.SportSet.DefHikeFatBurnGoal =ble_sports_param->country_walk.goal_calory;
      SetValue.SportSet.DefCircleDistanceHike =ble_sports_param->country_walk.lap_count_distance;
		}				
		if(ble_sports_param->sports_type_mask.bits.indoor_run)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: indoor_run\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->indoor_run.swtch --> 0x%02X\n",ble_sports_param->indoor_run.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->indoor_run.goal_distance --> %d\n",ble_sports_param->indoor_run.goal_distance);		
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->indoor_run.goal_pace --> %d\n",ble_sports_param->indoor_run.goal_pace);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->indoor_run.lap_count_distance --> %d\n",ble_sports_param->indoor_run.lap_count_distance);
      SetValue.SportSet.SwIndoorRun = ble_sports_param->indoor_run.swtch.value;
      SetValue.SportSet.DefHintSpeedIndoorRun = ble_sports_param->indoor_run.goal_pace;
      SetValue.SportSet.DefHintDistanceIndoorRun = ble_sports_param->indoor_run.goal_distance;
      SetValue.SportSet.DefCircleDistanceIndoorRun = ble_sports_param->indoor_run.lap_count_distance;
		}			
		if(ble_sports_param->sports_type_mask.bits.triathonal)
		{
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: triathonal\n");	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.outdoor_swim.swtch --> 0x%02X\n",ble_sports_param->triathonal.outdoor_swim.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.outdoor_swim.goal_time_sec --> %d\n",ble_sports_param->triathonal.outdoor_swim.goal_time_sec);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.outdoor_swim.lap_count_distance --> %d\n",ble_sports_param->triathonal.outdoor_swim.lap_count_distance);
      SetValue.SportSet.SwOutdoorSwimmg = ble_sports_param->triathonal.outdoor_swim.swtch.value;
      SetValue.SportSet.DefHintTimeOutdoorSwimmg = ble_sports_param->triathonal.outdoor_swim.goal_time_sec;
      SetValue.SportSet.DefCircleDistancOutdoorSwimmg = ble_sports_param->triathonal.outdoor_swim.lap_count_distance;


			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.cycling.swtch --> 0x%02X\n",ble_sports_param->triathonal.cycling.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.cycling.goal_distance --> %d\n",ble_sports_param->triathonal.cycling.goal_distance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.cycling.goal_pace --> %d\n",ble_sports_param->triathonal.cycling.goal_pace);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.cycling.lap_count_distance --> %d\n",ble_sports_param->triathonal.cycling.lap_count_distance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.cycling.wheel_radius --> %d\n",ble_sports_param->triathonal.cycling.wheel_radius);			


			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.run.swtch --> 0x%02X\n",ble_sports_param->triathonal.run.swtch);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.run.goal_distance --> %d\n",ble_sports_param->triathonal.run.goal_distance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.run.goal_pace --> %d\n",ble_sports_param->triathonal.run.goal_pace);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]: ble_sports_param->triathonal.run.lap_count_distance --> %d\n",ble_sports_param->triathonal.run.lap_count_distance);	

		}	
	
	}
	else{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x01;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));
		return;
	}	
	
#endif
	
}	



static void ble_uart_ccommand_application_app_message_ios(void)
{
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));			
	drv_ble_uart_close();
	uint8_t gbk_char[ATTR_MESSAGE_SIZE];
	uint16_t gbk_length;		
	_app_srv_msg *app_srv_ancs_msg = (_app_srv_msg *)(buffer);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc --> 0x%08X\n",app_srv_ancs_msg->crc);	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&app_srv_ancs_msg->crc)+sizeof(app_srv_ancs_msg->crc),  
														sizeof(_app_srv_msg)-sizeof(app_srv_ancs_msg->crc)-sizeof(app_srv_ancs_msg->command));	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc_dest --> 0x%08X\n",crc_dest);
	
	if(crc_dest == app_srv_ancs_msg->crc)
	{
		TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc match success!!!\n");
		memset(&g_app_notification_msg,0,sizeof(g_app_notification_msg));
		g_app_notification_msg.status = APP_NOTIFY_MSG_STATUS_UNREAD;
		g_app_notification_msg.type = app_srv_ancs_msg->type;
		memcpy(g_app_notification_msg.date_time,app_srv_ancs_msg->date_time,sizeof(g_app_notification_msg.date_time));
		memcpy(g_app_notification_msg.message_size,app_srv_ancs_msg->message_size,sizeof(g_app_notification_msg.message_size));
		
		lib_utf8_gbk(app_srv_ancs_msg->title,gbk_char,&gbk_length);		
		/*
			表情UTF-8通过lib_utf8_gbk字库芯片提供的接口转出来的是0，先用0x20空格代替
		*/
		for(uint16_t i=0;i<gbk_length;i++)
		{
			if(gbk_char[i] == 0)
			{
				gbk_char[i] = 0x20;
			}
		}			
		memcpy(g_app_notification_msg.title,gbk_char,gbk_length);
		
			
		
		lib_utf8_gbk(app_srv_ancs_msg->subtitle,gbk_char,&gbk_length);
		/*
			表情UTF-8通过lib_utf8_gbk字库芯片提供的接口转出来的是0，先用0x20空格代替
		*/
		for(uint16_t i=0;i<gbk_length;i++)
		{
			if(gbk_char[i] == 0)
			{
				gbk_char[i] = 0x20;
			}
		}			
		memcpy(g_app_notification_msg.subtitle,gbk_char,gbk_length);
		lib_utf8_gbk(app_srv_ancs_msg->message,gbk_char,&gbk_length);		
		
		/*
			表情UTF-8通过lib_utf8_gbk字库芯片提供的接口转出来的是0，先用0x20空格代替
		*/
		for(uint16_t i=0;i<gbk_length;i++)
		{
			if(gbk_char[i] == 0)
			{
				gbk_char[i] = 0x20;
			}
		}		
		memcpy(g_app_notification_msg.message,gbk_char,gbk_length);
		uint8_t status[1] = {0};
		uint32_t i = 0;
		for(i=0;i<APP_NOTIFY_MSG_NUM_MAX;i++)
		{
			dev_extFlash_enable();			
			dev_extFlash_read(NOTIFY_DATA_START_ADDRESS+i*APP_NOTIFY_MSG_SIZE_MAX, status, sizeof(status));			
			dev_extFlash_disable();			
			if(status[0] == APP_NOTIFY_MSG_STATUS_ERASED)
			{
				break;
			}
		}
		
		if(i < APP_NOTIFY_MSG_NUM_MAX)
		{
			dev_extFlash_enable();			
			dev_extFlash_write(NOTIFY_DATA_START_ADDRESS+i*APP_NOTIFY_MSG_SIZE_MAX, (uint8_t *)&g_app_notification_msg, sizeof(g_app_notification_msg));			
			dev_extFlash_disable();			
		}
		else
		{
		
			dev_extFlash_enable();			
			dev_extFlash_erase(NOTIFY_DATA_START_ADDRESS, APP_NOTIFY_MSG_SIZE_MAX*APP_NOTIFY_MSG_NUM_MAX);	
			dev_extFlash_write(NOTIFY_DATA_START_ADDRESS, (uint8_t *)&g_app_notification_msg, sizeof(g_app_notification_msg));			
			dev_extFlash_disable();	
			
			
		}
		
		
		
		if((ScreenState != DISPLAY_SCREEN_LOGO)
			&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
			&& time_notify_is_ok())
		{
			if(ScreenState < DISPLAY_SCREEN_NOTIFY)
			{
				ScreenStateSave = ScreenState;
			}
			timer_notify_display_start(10000,1,false);
			if (g_app_notification_msg.type ==APP_MSG_TYPE_CALL_IOS)
			{
			 #ifdef COD 
			 	timer_notify_motor_start(1000,500,SetValue.phone_call_delay,false,NOTIFY_MODE_PHONE_CALL);
				timer_notify_buzzer_start(1000,500,SetValue.phone_call_delay,false,NOTIFY_MODE_PHONE_CALL);
			 #else
				timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_NORMAL);
				timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_NORMAL);
			#endif
			}
			else
			{
				#ifdef COD 
				timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_PHONE_CALL);
				#else
				timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
				timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
				#endif
			}
			
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_NOTIFY_APP_MSG;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
		}			
		
	}
	
}

static void ble_uart_ccommand_application_app_message_android(void)
{	
	uint8_t gbk_char[ATTR_MESSAGE_SIZE];
	uint16_t gbk_length;
	_app_srv_msg *app_srv_msg = (_app_srv_msg *)(m_task_buffer.p_data);
	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc --> 0x%08X\n",app_srv_msg->crc);	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&app_srv_msg->crc)+sizeof(app_srv_msg->crc),  
														sizeof(_app_srv_msg)-sizeof(app_srv_msg->crc)-sizeof(app_srv_msg->command));	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc_dest --> 0x%08X\n",crc_dest);
	if(crc_dest == app_srv_msg->crc)
	{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_APP_MESSAGE_ANDROID | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x00;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));

		memset(&g_app_notification_msg,0,sizeof(g_app_notification_msg));
		g_app_notification_msg.status = APP_NOTIFY_MSG_STATUS_UNREAD;		
		memcpy((uint8_t *)&g_app_notification_msg.type,(uint8_t *)&app_srv_msg->type,sizeof(_app_msg_type));
		memcpy(&g_app_notification_msg.message_size,app_srv_msg->message_size,sizeof(g_app_notification_msg.message_size));

			
		lib_utf16_gbk(app_srv_msg->title,ATTR_TITLE_SIZE,gbk_char,&gbk_length);		
		
		memcpy(g_app_notification_msg.title,gbk_char,gbk_length);
		lib_utf16_gbk(app_srv_msg->message,ATTR_MESSAGE_SIZE,gbk_char,&gbk_length);		
		
		memcpy(g_app_notification_msg.message,gbk_char,gbk_length);
		
		
		uint8_t status[1] = {0};
		uint32_t i = 0;
		for(i=0;i<APP_NOTIFY_MSG_NUM_MAX;i++)
		{
			dev_extFlash_enable();			
			dev_extFlash_read(NOTIFY_DATA_START_ADDRESS+i*APP_NOTIFY_MSG_SIZE_MAX, status, sizeof(status));			
			dev_extFlash_disable();			
			if(status[0] == APP_NOTIFY_MSG_STATUS_ERASED)
			{
				break;
			}
		}
		
		if(i < APP_NOTIFY_MSG_NUM_MAX)
		{
			dev_extFlash_enable();			
			dev_extFlash_write(NOTIFY_DATA_START_ADDRESS+i*APP_NOTIFY_MSG_SIZE_MAX, (uint8_t *)&g_app_notification_msg, sizeof(g_app_notification_msg));			
			dev_extFlash_disable();			
		}
		else
		{
			dev_extFlash_enable();			
			dev_extFlash_erase(NOTIFY_DATA_START_ADDRESS, APP_NOTIFY_MSG_SIZE_MAX*APP_NOTIFY_MSG_NUM_MAX);	
			dev_extFlash_write(NOTIFY_DATA_START_ADDRESS, (uint8_t *)&g_app_notification_msg, sizeof(g_app_notification_msg));			
			dev_extFlash_disable();	
		}
		
		
		
		if((ScreenState != DISPLAY_SCREEN_LOGO)
			&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
			&& time_notify_is_ok())
		{
			if(ScreenState < DISPLAY_SCREEN_NOTIFY)
			{
				ScreenStateSave = ScreenState;
			}
			timer_notify_display_start(10000,1,false);
			if (g_app_notification_msg.type ==APP_MSG_TYPE_CALL)
			{
			#ifdef COD 
			 	timer_notify_motor_start(1000,500,SetValue.phone_call_delay,false,NOTIFY_MODE_PHONE_CALL);
				timer_notify_buzzer_start(1000,500,SetValue.phone_call_delay,false,NOTIFY_MODE_PHONE_CALL);
			#else
				timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_NORMAL);
				timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_NORMAL);
			#endif
			}
			else
			{
				#ifdef COD 
				timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_PHONE_CALL);
				#else
				timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
				timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_NORMAL);
				#endif
			}
			
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_NOTIFY_APP_MSG;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);								
		}			
		
	}else{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_APP_MESSAGE_ANDROID | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x01;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));
		return;	
	}
}	



static void ble_uart_ccommand_application_wechat_sport_data_request(void)							
{
	_wechat_sport_data wechat_sport_data;
	wechat_sport_data.command = BLE_UART_RESPONSE_APPLICATION_WECHAT_SPORT_DATA_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	wechat_sport_data.step = Get_TotalStep();
	wechat_sport_data.calory = Get_TotalEnergy();
	wechat_sport_data.distance = Get_TotalDistance();	
	drv_ble_uart_put((uint8_t *)&wechat_sport_data, sizeof(wechat_sport_data));	
}


static void ble_uart_ccommand_application_accessory_heartrate_connect(void)
{

	memcpy(g_ble_accessory.heartrate.mac,(uint8_t *)(m_task_buffer.p_data + 1),6);
	g_ble_accessory.heartrate.status = ACCESSORY_STATUS_CONNECT;
	drv_ble_uart_close();
	if(ScreenState == DISPLAY_SCREEN_ACCESSORY)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_ACCESSORY;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
	}
}	
static void ble_uart_ccommand_application_accessory_running_connect(void)
{

	memcpy(g_ble_accessory.running.mac,(uint8_t *)(m_task_buffer.p_data + 1),6);
	g_ble_accessory.running.status = ACCESSORY_STATUS_CONNECT;
	drv_ble_uart_close();
	if(ScreenState == DISPLAY_SCREEN_ACCESSORY)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_ACCESSORY;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
	}
}	
static void ble_uart_ccommand_application_accessory_cycling_connect(void)
{	
	memcpy(g_ble_accessory.cycling.mac,(uint8_t *)(m_task_buffer.p_data + 1),6);
	g_ble_accessory.cycling.status = ACCESSORY_STATUS_CONNECT;
	drv_ble_uart_close();
	if(ScreenState == DISPLAY_SCREEN_ACCESSORY)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_ACCESSORY;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
	}
}		
static void ble_uart_ccommand_application_accessory_heartrate_disconnect(void)
{
	memset(g_ble_accessory.heartrate.mac,0,6);
	g_ble_accessory.heartrate.status = ACCESSORY_STATUS_DISCONNECT;
	drv_ble_uart_close();
	if(ScreenState == DISPLAY_SCREEN_ACCESSORY)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_ACCESSORY;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
	}
}	
static void ble_uart_ccommand_application_accessory_running_disconnect(void)
{
	memset(g_ble_accessory.running.mac,0,6);
	g_ble_accessory.running.status = ACCESSORY_STATUS_DISCONNECT;
	drv_ble_uart_close();
	if(ScreenState == DISPLAY_SCREEN_ACCESSORY)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_ACCESSORY;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
	}
}	
static void ble_uart_ccommand_application_accessory_cycling_disconnect(void)
{
	memset(g_ble_accessory.cycling.mac,0,6);
	g_ble_accessory.cycling.status = ACCESSORY_STATUS_DISCONNECT;
	drv_ble_uart_close();
	if(ScreenState == DISPLAY_SCREEN_ACCESSORY)
	{
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_ACCESSORY;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
	}
}
static void ble_uart_ccommand_application_accessory_data_upload(void)
{
	
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));		
	if(g_ble_accessory.enable == true){
		drv_ble_uart_close();

	}
	else{
			//回复停止扫描指令
	        TASK_BLE_LOG_PRINTF("[TASK_BLE]:xTaskCreate task_ble_init send scan enable command\n");
			uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_SCAN_DISABLE | BLE_UART_CCOMMAND_MASK_APPLICATION};
			_uart_data_param param;
			param.user_callback = NULL;
			param.p_data = (uint8_t *)command;
			param.length = sizeof(command);
			uint8_t count = TASK_UART_DELAY_COUNT_OUT;
		#ifndef WATCH_HAS_NO_BLE 
			while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
			{
				DELAY_MS(TASK_UART_DELAY);
			}	
		#endif
	}

	
	g_ble_accessory.data = *(_ble_accessory_data *)buffer;
	
}
#ifdef WATCH_GPS_SNR_TEST
typedef struct{	
	uint8_t num;
	uint8_t sn;
}_task_ble_gps_snr_data;

void ble_uart_ccommand_application_send_gps_test_data(GPSMode *gps_data)
{
	_task_ble_gps_snr_data ble_snr_data[12];	
	uint32_t i;

	if (SetValue.SwBle != BLE_CONNECT){
		TASK_BLE_LOG_PRINTF("ble is disconnect");
		return;
	}
	for(i=0;i<12;i++)
	{
		ble_snr_data[i].num = (uint8_t)gps_data->slmsg[i].num;
		ble_snr_data[i].sn = (uint8_t)gps_data->slmsg[i].sn;
		TASK_BLE_LOG_PRINTF("[TASK_BLE]:gps_data --> %02X%02X\n",gps_data->slmsg[i].num,gps_data->slmsg[i].sn);
	}
	uint32_t command[7] = {BLE_UART_CCOMMAND_APPLICATION_SEND_GPS_TEST_DATA | BLE_UART_CCOMMAND_MASK_APPLICATION, 0,0,0,0,0,0};
	memcpy((uint8_t *)(command+1), (uint8_t *)&ble_snr_data, sizeof(ble_snr_data));
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:command --> 0x%08X\n",command[0]);
	TASK_BLE_LOG_PRINTF("%08X\n",command[1]);
	TASK_BLE_LOG_PRINTF("%08X\n",command[2]);
	TASK_BLE_LOG_PRINTF("%08X\n",command[3]);
	TASK_BLE_LOG_PRINTF("%08X\n",command[4]);
	TASK_BLE_LOG_PRINTF("%08X\n",command[5]);
	TASK_BLE_LOG_PRINTF("%08X\n",command[6]);
	
	_uart_data_param param;	
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	#ifndef WATCH_HAS_NO_BLE 
		while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
		{
			DELAY_MS(TASK_UART_DELAY);
		}
	#endif
	TASK_BLE_LOG_PRINTF("drv_ble_send_data sucess!!!!\n");
}
#endif
static void ble_uart_ccommand_application_find_watch(void)								
{
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer)); 		
	//drv_ble_uart_close();
	uint32_t operation_code = *(uint32_t *)((uint32_t *)buffer+1);
		
	if(ScreenState != DISPLAY_SCREEN_LOGO)		
	{
		if(ScreenState < DISPLAY_SCREEN_NOTIFY)
		{
			ScreenStateSave = ScreenState;
			
		}		
		if(operation_code == FIND_WATCH_PARAM_START)
		{
			timer_notify_display_start(0xffffffff,1,false);
		 	timer_notify_motor_start(1000,500,0xffffffff,false,NOTIFY_MODE_SPECIFY_FUNC);
		 	timer_notify_buzzer_start(1000,500,0xffffffff,false,NOTIFY_MODE_SPECIFY_FUNC);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_NOTIFY_FIND_WATCH;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			drv_light_enable();
			
		}
		else if (operation_code == FIND_WATCH_PARAM_END)
		{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			drv_light_disable();
			gui_notify_findwatch_timer_stop();
		}

		uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_FIND_WATCH | BLE_UART_CCOMMAND_MASK_APPLICATION, 0};
		command[1] = operation_code;
		drv_ble_uart_put((uint8_t *)&command, sizeof(command));
		
	}
}

typedef struct
{
	uint32_t command;
	uint32_t sport_mode;
	uint32_t reserve;	
}_task_ble_watch_default_sport_mode_data;
static void ble_uart_ccommand_application_get_watch_default_sport_mode(void)
{	
	_task_ble_watch_default_sport_mode_data command;
	command.command = BLE_UART_RESPONSE_APPLICATION_GET_WATCH_DEFAULT_SPORT_MODE | BLE_UART_CCOMMAND_MASK_APPLICATION;
	//此赋值和com_data 1672行中的赋值一致
	
	command.sport_mode = DEFAULT_SPORT_MODE;

	command.reserve = 0;	
	drv_ble_uart_put((uint8_t *)&command, sizeof(_task_ble_watch_default_sport_mode_data));
}


typedef struct __attribute__((packed))
{
	uint32_t command;
	uint8_t error;
	uint32_t address;
}_task_ble_extflash_config_astro_sync_request;
static void ble_uart_ccommand_application_extflash_config_astro_sync_request(void)
{
#ifdef COD
	uint32_t command[2] = {0};
	bool has_agps;
	uint16_t update_time;
	rtc_time_t Ymdhms;
	am_hal_rtc_time_get(&RTC_time);
	Ymdhms.Year = RTC_time.ui32Year;
	Ymdhms.Month = RTC_time.ui32Month;
	Ymdhms.Day = RTC_time.ui32DayOfMonth;
	Ymdhms.Hour = RTC_time.ui32Hour;
	Ymdhms.Minute = RTC_time.ui32Minute;
	Ymdhms.Second = RTC_time.ui32Second;

	command[0] = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	has_agps = ublox_is_offline_data_in_flash_uptodate();
	update_time = Get_DiffTime(Ymdhms,SetValue.agps_update_time)/3600;
	command[1] = has_agps << 16 | update_time;
	drv_ble_uart_put((uint8_t *)&command, sizeof(command));
#else
	_task_ble_extflash_config_astro_sync_request command;
	command.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	#if defined WATCH_GPS_ONLINE
	command.error = ublox_online_offline_data_in_flash_uptodata();
	#else
	command.error = ublox_is_offline_data_in_flash_uptodate();
	#endif
	command.address = SATELLITE_DATA_START_ADDRESS;
	

	drv_ble_uart_put((uint8_t *)&command, sizeof(_task_ble_extflash_config_astro_sync_request));
#endif

}

typedef struct __attribute__((packed))
{
	uint8_t date[3];
	uint8_t actvty_cnt;
	uint32_t address;
	uint32_t size;
}_main_day_data_data;


typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t total_step;
	uint32_t total_energy;
	uint32_t total_distance;
	uint16_t total_VO2MaxMeasure;    //测定最大摄氧量 单位：ml/kg/min
	uint16_t total_LTSpeedMeasure;    //测定乳酸阈速度 s/km
	uint8_t  total_LTBmpMeasure;     //测定乳酸阈心率值 
	uint8_t  total_heartrate;
	uint8_t  reserved[12];
	uint8_t day_count;
	_main_day_data_data data[7];

}_task_ble_extflash_config_main_day_data;

static void ble_uart_ccommand_application_extflash_config_main_day_data_request(void)
{
	uint8_t year, month, date;
	uint8_t hour, minute, second,flag;
	uint8_t last_activity_index;
	year = *((uint8_t *)m_task_buffer.p_data+4);
	month = *((uint8_t *)m_task_buffer.p_data+5);
	date = *((uint8_t *)m_task_buffer.p_data+6);
	last_activity_index = *((uint8_t *)m_task_buffer.p_data+7);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:year request --> %d\r\n",year);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:month request --> %d\r\n",month);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:date request --> %d\r\n",date);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:last_activity_index --> %d\r\n",last_activity_index);	
	flag = *((uint8_t *)m_task_buffer.p_data+8);
	hour = *((uint8_t *)m_task_buffer.p_data+9);
	minute = *((uint8_t *)m_task_buffer.p_data+10);
	second = *((uint8_t *)m_task_buffer.p_data+11);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:flag request --> %d\r\n",flag);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:hour request --> %d\r\n",hour);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:minute request --> %d\r\n",minute);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:second request --> %d\r\n",second);
	
	_task_ble_extflash_config_main_day_data config;
	memset((uint8_t *)&config,0,sizeof(_task_ble_extflash_config_main_day_data));
	
	config.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_MAIN_DAY_DATA_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	config.day_count = 0;
	
	DayMainDataStr dayMainDataStr;
	uint8_t activity_index = 0, m_last_activity_index = 0;
	uint8_t j = 0,k = 0;	
	uint32_t date_request = year<<16 | month<<8 | date;
	uint32_t date_flash = 0;
	uint32_t time_requet = hour<<16 |minute<<8 | second;
	uint32_t time_flash = 0;
	
	for(uint8_t i=0;i<7;i++)
	{
		dev_extFlash_enable();
		dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i,(uint8_t *)&dayMainDataStr,3);
		dev_extFlash_disable();
		if(dayMainDataStr.Year != 0xFF &&
			 dayMainDataStr.Month != 0xFF &&
			 dayMainDataStr.Date != 0xFF
			)
		{
			
			date_flash = dayMainDataStr.Year<<16 | dayMainDataStr.Month<<8 | dayMainDataStr.Date;
			
			if(date_flash >= date_request)
			{
				config.data[config.day_count].date[0] = dayMainDataStr.Year;//year;
				config.data[config.day_count].date[1] = dayMainDataStr.Month;//month;
				config.data[config.day_count].date[2] = dayMainDataStr.Date;//date;		
				config.data[config.day_count].address = DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i;//address;
				config.data[config.day_count].size = DAY_COMMON_DATA_SIZE;//size;		
				
				TASK_BLE_LOG_PRINTF("[TASK_BLE]:date --> %d.%d.%d\n",config.data[config.day_count].date[0],
																														 config.data[config.day_count].date[1],
																														 config.data[config.day_count].date[2]);
				TASK_BLE_LOG_PRINTF("[TASK_BLE]:address --> 0x%08X\n",config.data[config.day_count].address);
				TASK_BLE_LOG_PRINTF("[TASK_BLE]:size --> %d\n",config.data[config.day_count].size);
				
				
				if(date_flash == date_request)
				{
					m_last_activity_index = last_activity_index;

					uint8_t m_time_data[3] = {0,0,0};
					if(flag == 0xFF) {
						TASK_BLE_LOG_PRINTF("[TASK_BLE]:flag == 0xFF\n");
						for(k=0;k<DAY_ACTIVITY_MAX_NUM;k++){
							dev_extFlash_enable();
							dev_extFlash_read(DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_DATA_SIZE+sizeof(uint8_t)*5+sizeof(ActivityDataStr)*k, m_time_data, 3);
							dev_extFlash_disable();
							TASK_BLE_LOG_PRINTF("[TASK_BLE]:m_time_hour --> %d\r\n",m_time_data[0]);
							TASK_BLE_LOG_PRINTF("[TASK_BLE]:m_time_minute --> %d\r\n",m_time_data[1]);
							TASK_BLE_LOG_PRINTF("[TASK_BLE]:m_time_second --> %d\r\n",m_time_data[2]);
							if(m_time_data[0] != 0xFF &&
								 m_time_data[1] != 0xFF &&
								 m_time_data[2] != 0xFF
								){
									time_flash = m_time_data[0]<<16 | m_time_data[1]<<8 | m_time_data[2];
									if(time_flash > time_requet){
										m_last_activity_index = k;
										TASK_BLE_LOG_PRINTF("[TASK_BLE]:m_last_activity_index --> %d\r\n",m_last_activity_index);
										break;
									}
								}
								else{
									m_last_activity_index = 0;
									TASK_BLE_LOG_PRINTF("[TASK_BLE]:m_last_activity_index = 0 \r\n");
									TASK_BLE_LOG_PRINTF("[TASK_BLE]:m_time_data is all 0xff \r\n");
									break;
								}
						}
					}
				}
				else{
					m_last_activity_index = 0;
				}
				TASK_BLE_LOG_PRINTF("[TASK_BLE]:Sport is Triathlon now!IsTriathlonSport=%d,RTC_time%d-%d-%d,TriathlonSportTime:%d-%d-%d\n"
				,SetValue.TriathlonSport.IsTriathlonSport,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth
				,SetValue.TriathlonSport.year,SetValue.TriathlonSport.month,SetValue.TriathlonSport.day);
				if(SetValue.TriathlonSport.IsTriathlonSport == 1 
				   && dayMainDataStr.Year == RTC_time.ui32Year
				   && dayMainDataStr.Month == RTC_time.ui32Month
				   && dayMainDataStr.Date == RTC_time.ui32DayOfMonth)
				{//现在处于铁人三项运动中 该铁人三项已保存的数据不上传
					rtc_time_t TriathlonTime;
					//确认正在运动的铁人三项属于该空间的,避免两个相同日期空间
					dev_extFlash_enable();
					dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_DATA_SIZE+sizeof(ActivityDataStr)*(SetValue.TriathlonSport.IndexTriathlon-1)
						                +sizeof(uint8_t)*2,(uint8_t *)&TriathlonTime,sizeof(rtc_time_t));
					dev_extFlash_disable();
					TASK_BLE_LOG_PRINTF("[TASK_BLE]:Sport is Triathlon now!+++++++TriathlonTime%d-%d-%d %d:%d:%d\n",
					 TriathlonTime.Year,TriathlonTime.Month,TriathlonTime.Day,TriathlonTime.Hour,TriathlonTime.Minute,TriathlonTime.Second);
					if(SetValue.TriathlonSport.year == TriathlonTime.Year && SetValue.TriathlonSport.month == TriathlonTime.Month
						&& SetValue.TriathlonSport.day == TriathlonTime.Day && SetValue.TriathlonSport.hour == TriathlonTime.Hour
					  && SetValue.TriathlonSport.minute == TriathlonTime.Minute && SetValue.TriathlonSport.second == TriathlonTime.Second)
					{
						for(j=0;j<DAY_ACTIVITY_MAX_NUM;j++)
						{
							dev_extFlash_enable();
							dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_DATA_SIZE+sizeof(ActivityDataStr)*j+sizeof(uint8_t),&activity_index,sizeof(activity_index));
							dev_extFlash_disable();
							if(activity_index < (DAY_ACTIVITY_MAX_NUM+1) && activity_index > m_last_activity_index 
								 && activity_index < SetValue.TriathlonSport.IndexTriathlon)
							{
								TASK_BLE_LOG_PRINTF("[TASK_BLE]:activity_index --> %d\n",activity_index);
								config.data[config.day_count].actvty_cnt++;
							}			
						}
					}

				}
				else
				{//现在不处于铁人三项运动中
					TASK_BLE_LOG_PRINTF("[TASK_BLE]:Sport is not Triathlon now!!!!!\n");
					for(j=0;j<DAY_ACTIVITY_MAX_NUM;j++)
					{
						dev_extFlash_enable();
						dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*i+DAY_COMMON_DATA_SIZE+sizeof(ActivityDataStr)*j+sizeof(uint8_t),&activity_index,sizeof(activity_index));
						dev_extFlash_disable();
						if(activity_index < (DAY_ACTIVITY_MAX_NUM+1) && activity_index > m_last_activity_index)
						{
							TASK_BLE_LOG_PRINTF("[TASK_BLE]:activity_index --> %d\n",activity_index);
							config.data[config.day_count].actvty_cnt++;
						}			
					}
				}

				TASK_BLE_LOG_PRINTF("[TASK_BLE]:actvty_cnt --> %d\n",config.data[config.day_count].actvty_cnt);
				config.day_count++;	
			}
		}
	}		
	
	config.total_step = Get_TotalStep();
	config.total_energy = Get_TotalEnergy();
	config.total_distance = Get_TotalDistance();
	config.total_VO2MaxMeasure = DayMainData.VO2MaxMeasuring;
	config.total_LTSpeedMeasure = DayMainData.LTSpeedMeasuring;
	config.total_LTBmpMeasure = DayMainData.LTBmpMeasuring;
	config.total_heartrate = DayMainData.BasedHeartrate;
	memset(config.reserved,0,sizeof(config.reserved));
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:config.total_step --> %d\n",config.total_step);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:config.total_energy --> %d\n",config.total_energy);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:config.total_distance --> %d\n",config.total_distance);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:config.total_VO2MaxMeasure --> %d\n",config.total_VO2MaxMeasure);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:config.total_LTSpeedMeasure --> %d\n",config.total_LTSpeedMeasure);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:config.total_LTBmpMeasure --> %d\n",config.total_LTBmpMeasure);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:config.total_heartrate --> %d\n",config.total_heartrate);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:config.day_count --> %d\n",config.day_count);
	
	drv_ble_uart_put((uint8_t *)&config, sizeof(config));
}

typedef struct
{
	uint32_t command;
	ActivityDataStr activityDataStr;
	
}_task_ble_extflash_config_activity_address;

static void ble_uart_ccommand_application_extflash_config_activity_address_request(void)
{
	uint8_t year, month, date;
	uint8_t hour, minute, second,flag;
	uint8_t last_activity_index;
	uint32_t main_day_data_address = 0;
	year = *((uint8_t *)m_task_buffer.p_data+4);
	month = *((uint8_t *)m_task_buffer.p_data+5);
	date = *((uint8_t *)m_task_buffer.p_data+6);
	last_activity_index = *((uint8_t *)m_task_buffer.p_data+7);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:main_day_data_address --> 0x%02X,0x%02X,0x%02X,0x%02X\n",
																														*((uint8_t *)m_task_buffer.p_data+8),
																													*((uint8_t *)m_task_buffer.p_data+9),
																														*((uint8_t *)m_task_buffer.p_data+10),
																														*((uint8_t *)m_task_buffer.p_data+11));
	main_day_data_address = *(uint32_t *)((uint8_t *)m_task_buffer.p_data+8);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:year request --> %d\n",year);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:month request --> %d\n",month);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:date request --> %d\n",date);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:last_activity_index --> %d\n",last_activity_index);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:main_day_data_address --> 0x%08X\n",main_day_data_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:sizeof ActivityDataStr --> %d\n",sizeof(ActivityDataStr));	
	DayMainDataStr dayMainDataStr;

	flag = *((uint8_t *)m_task_buffer.p_data+12);
	hour = *((uint8_t *)m_task_buffer.p_data+13);
	minute = *((uint8_t *)m_task_buffer.p_data+14);
	second = *((uint8_t *)m_task_buffer.p_data+15);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:flag request --> %d\n",flag);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:hour request --> %d\n",hour);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:minute request --> %d\n",minute);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:second request --> %d\n",second);

	uint32_t time_requet = hour<<16 |minute<<8 | second;
	uint32_t time_flash = 0;

	dev_extFlash_enable();
	dev_extFlash_read(main_day_data_address,(uint8_t *)&dayMainDataStr,3);
	dev_extFlash_disable();	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:year --> %d\n",dayMainDataStr.Year);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:month --> %d\n",dayMainDataStr.Month);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:date --> %d\n",dayMainDataStr.Date);		
	
	
	if(dayMainDataStr.Year == year && dayMainDataStr.Month == month && dayMainDataStr.Date == date)
	{
		uint8_t j = 0;uint8_t activity_index = 0;uint8_t k = 0;
		uint8_t m_time_data[3] = {0,0,0};
		if(flag == 0xFF) {
			TASK_BLE_LOG_PRINTF("[TASK_BLE]:flag == 0xFF\n");
			for(k=0;k<DAY_ACTIVITY_MAX_NUM;k++){
				dev_extFlash_enable();
				dev_extFlash_read(main_day_data_address+DAY_COMMON_DATA_SIZE+sizeof(uint8_t)*5+sizeof(ActivityDataStr)*k, m_time_data, 3);
				dev_extFlash_disable();
				if(m_time_data[0] != 0xFF &&
					 m_time_data[1] != 0xFF &&
					 m_time_data[2] != 0xFF
					){
						time_flash = m_time_data[0]<<16 | m_time_data[1]<<8 | m_time_data[2];
						if(time_flash > time_requet){
							last_activity_index = k;
							TASK_BLE_LOG_PRINTF("[TASK_BLE]:last_activity_index --> %d\n",last_activity_index);
							break;
						}
					}
					else{
						last_activity_index = 0;
						TASK_BLE_LOG_PRINTF("[TASK_BLE]:m_time_data is all 0xff");
						break;
					}
			}
		}
		
		for(j=0;j<DAY_ACTIVITY_MAX_NUM;j++)
		{
			dev_extFlash_enable();
			dev_extFlash_read(main_day_data_address+DAY_COMMON_DATA_SIZE+sizeof(ActivityDataStr)*j+sizeof(uint8_t),&activity_index,sizeof(activity_index));
			dev_extFlash_disable();
			if(activity_index < (DAY_ACTIVITY_MAX_NUM+1) && activity_index > last_activity_index)
			{
				TASK_BLE_LOG_PRINTF("[TASK_BLE]:activity_index --> %d\n",activity_index);
				break;
			}			
		}
					
		if(j < DAY_ACTIVITY_MAX_NUM)
		{
			_task_ble_extflash_config_activity_address activity_data;
			activity_data.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
			
			dev_extFlash_enable();
			dev_extFlash_read(main_day_data_address+DAY_COMMON_DATA_SIZE+sizeof(ActivityDataStr)*j,(uint8_t *)&activity_data.activityDataStr,sizeof(activity_data.activityDataStr));
			dev_extFlash_disable();					
			
			TASK_BLE_LOG_PRINTF("[TASK_BLE]:Activity_Type --> 0x%02X\n",activity_data.activityDataStr.Activity_Type);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]:Activity_Index --> %d\n",activity_data.activityDataStr.Activity_Index);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]:Act_Start_Time --> %d.%d.%d.%d.%d.%d\n",activity_data.activityDataStr.Act_Start_Time.Year,activity_data.activityDataStr.Act_Start_Time.Month,activity_data.activityDataStr.Act_Start_Time.Day,activity_data.activityDataStr.Act_Start_Time.Hour,activity_data.activityDataStr.Act_Start_Time.Minute,activity_data.activityDataStr.Act_Start_Time.Second);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]:Act_Stop_Time --> %d.%d.%d.%d.%d.%d\n",activity_data.activityDataStr.Act_Stop_Time.Year,activity_data.activityDataStr.Act_Stop_Time.Month,activity_data.activityDataStr.Act_Stop_Time.Day,activity_data.activityDataStr.Act_Stop_Time.Hour,activity_data.activityDataStr.Act_Stop_Time.Minute,activity_data.activityDataStr.Act_Stop_Time.Second);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]:CircleDistance --> 0x%02X\n",activity_data.activityDataStr.CircleDistance);	
			TASK_BLE_LOG_PRINTF("[TASK_BLE]:Steps_Start_Address --> 0x%08X\n",activity_data.activityDataStr.Steps_Start_Address);	
			
			drv_ble_uart_put((uint8_t *)&activity_data, sizeof(activity_data));	
		}
		else{
			uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
			command[1] = 0x02;
			drv_ble_uart_put((uint8_t *)command, sizeof(command));				
		}		
	}else
	{
		uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
		command[1] = 0x01;
		drv_ble_uart_put((uint8_t *)command, sizeof(command));
	}	
}


static void ble_uart_ccommand_application_extflash_config_activity_address_boundary_request(void)
{
	uint32_t command[25] = {BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	command[1] = STEP_DATA_START_ADDRESS;
	command[2] = STEP_DATA_STOP_ADDRESS;	
	command[3] = HEARTRATE_DATA_START_ADDRESS;
	command[4] = HEARTRATE_DATA_STOP_ADDRESS;	
	command[5] = PRESSURE_DATA_START_ADDRESS;
	command[6] = PRESSURE_DATA_STOP_ADDRESS;	
	command[7] = GPS_DATA_START_ADDRESS;
	command[8] = GPS_DATA_STOP_ADDRESS;		
	command[9] = DISTANCE_DATA_START_ADDRESS;
	command[10] = DISTANCE_DATA_STOP_ADDRESS;	
	command[11] = ENERGY_DATA_START_ADDRESS;
	command[12] = ENERGY_DATA_STOP_ADDRESS;		
	command[13] = SPEED_DATA_START_ADDRESS;
	command[14] = SPEED_DATA_STOP_ADDRESS;	
	command[15] = DISTANCEPOINT_START_ADDRESS;
	command[16] = DISTANCEPOINT_STOP_ADDRESS;			
	command[17] = CIRCLETIME_START_ADDRESS;
	command[18] = CIRCLETIME_STOP_ADDRESS;
	#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
	command[19] = VERTICALSIZE_START_ADDRESS;
	command[20] = VERTICALSIZE_STOP_ADDRESS;
	#endif
	command[21] = TOUCHDOWN_START_ADDRESS;	
	command[22] = TOUCHDOWN_STOP_ADDRESS;			
	command[23] = PAUSE_DATA_START_ADDRESS;
	command[24] = PAUSE_DATA_STOP_ADDRESS;	
	drv_ble_uart_put((uint8_t *)command, sizeof(command));
}




static void ble_uart_ccommand_application_extflash_config_shard_track_address_request(void)
{
	uint32_t command[5] = {BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};
	ShareTrackStr shareTrackStr;
	uint8_t i=0;
	for(i=0;i<SHARED_TRACK_NUM_MAX;i++)
	{
		dev_extFlash_enable();
		dev_extFlash_read( GPSTRACK_DATA_START_ADDRESS+ONE_SHARED_TRACK_SIZE*i,(uint8_t *)&shareTrackStr,1);
		dev_extFlash_disable();
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		if(shareTrackStr.Number == 0xFF && shareTrackStr.new_sign_value != SPORT_NEW_CLOUD_DISTINGUISH_SIGN_VALUE)
		{
			break;
		}
#else
		if(shareTrackStr.Number == 0xFF)
		{
			break;
		}			
#endif
	}
	if(i < SHARED_TRACK_NUM_MAX)
	{
		command[1] = 0x01;	//有空间
		command[2] = i+0xB0; //共享轨迹编号
		command[3] = GPSTRACK_DATA_START_ADDRESS + i*ONE_SHARED_TRACK_SIZE; //擦除地址
		command[4] = GPSTRACK_DATA_START_ADDRESS + i*ONE_SHARED_TRACK_SIZE + sizeof(ShareTrackStr); //轨迹点写入首地址
	}
	else{
		command[1] = 0x02;		//无空间		
	}			
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:command[0] --> 0x%08X\n",command[0]);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:command[1] --> 0x%08X\n",command[1]);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:command[2] --> 0x%08X\n",command[2]);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:command[3] --> 0x%08X\n",command[3]);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:command[4] --> 0x%08X\n",command[4]);
	
	drv_ble_uart_put((uint8_t *)command, sizeof(command));
}
static void ble_uart_ccommand_application_extflash_config_shard_track_header_write(void)
{
	ShareTrackStr *shareTrackStr = (ShareTrackStr *)((uint8_t *)m_task_buffer.p_data+4);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:Number --> 0x%02X\n",shareTrackStr->Number);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:Name --> %s\n",shareTrackStr->Name);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:point_start_address --> 0x%08X\n",shareTrackStr->point_start_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:point_end_address --> 0x%08X\n",shareTrackStr->point_end_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:gps_start_address --> 0x%08X\n",shareTrackStr->gps_start_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:gps_end_address --> 0x%08X\n",shareTrackStr->gps_end_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:Activity_Type --> 0x%02X\n",shareTrackStr->Activity_Type);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:diatance --> %d\n",shareTrackStr->diatance);
	dev_extFlash_enable();
	uint32_t address = shareTrackStr->point_start_address - sizeof(ShareTrackStr);
	dev_extFlash_write( address,(uint8_t *)shareTrackStr,sizeof(ShareTrackStr));
	dev_extFlash_disable();	
	uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_HEADER_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	command[1] = 0x01;
	drv_ble_uart_put((uint8_t *)command, sizeof(command));
}
static void ble_uart_response_application_find_phone(void)
{
	uint8_t buffer[256];
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));			
	drv_ble_uart_close();
	uint32_t operation_code = *(uint32_t *)((uint32_t *)buffer+1);
	if(operation_code == FIND_PHONE_PARAM_FOUND)
	{
		gui_tool_findphone_timer_stop();	
		//挂起工具任务
		tool_task_close();
		
		//返回原界面
		timer_notify_rollback(DISPLAY_SCREEN_FINDPHONE, DISPLAY_SCREEN_TOOL);

	}
}


typedef struct __attribute__((packed))
{
	uint32_t command;
	uint8_t md5[32];
	uint8_t uuid[32];
	TrainPlanSport StartTime;     //年/月/日
	TrainPlanSport StopTime;      //年/月/日
	uint8_t SwHint;               //提醒开关
	TrainPlanHint Hint;           //提醒时间 时分(24小时制)
}_extflash_config_train_plan_request_uuid;

//APP获取手表训练计划MD5/UUID/startDate/endDate/提醒开关/提醒时间
static void ble_uart_command_application_extflash_config_train_plan_request_uuid(void)
{
	_extflash_config_train_plan_request_uuid request_uuid;
	TrainPlanDownStr  MD5_UUIDStr;
	
	dev_extFlash_enable();
	//为了获取手表的MD5/UUID/startDate/endDate/提醒开关/提醒时间
	dev_extFlash_read(TRAINING_DATA_START_ADDRESS,(uint8_t*)(&MD5_UUIDStr),sizeof(TrainPlanDownStr));
	dev_extFlash_disable();
	
	memcpy(request_uuid.md5,MD5_UUIDStr.Md5Str,sizeof(MD5_UUIDStr.Md5Str));
	memcpy(request_uuid.uuid,MD5_UUIDStr.UUIDStr,sizeof(MD5_UUIDStr.UUIDStr));
	memcpy(&request_uuid.StartTime,&MD5_UUIDStr.StartTime,sizeof(MD5_UUIDStr.StartTime));
	memcpy(&request_uuid.StopTime,&MD5_UUIDStr.StopTime,sizeof(MD5_UUIDStr.StopTime));
	memcpy(&request_uuid.Hint,&MD5_UUIDStr.Hint,sizeof(MD5_UUIDStr.Hint));
	request_uuid.SwHint = SetValue.TrainPlanNotifyFlag;
	request_uuid.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID | BLE_UART_CCOMMAND_MASK_APPLICATION;

	
	drv_ble_uart_put((uint8_t *)&request_uuid, sizeof(_extflash_config_train_plan_request_uuid));
}


typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t address;
	uint32_t size;
}_extflash_config_train_plan_read_address_request;
//APP获取手表训练计划读地址：APP在读取训练计划闪存数据前询问APOLLO训练计划存储在闪存中的地址，和数据大小
static void ble_uart_command_application_extflash_config_train_plan_read_address_request(void)
{

	_extflash_config_train_plan_read_address_request address_request;
	address_request.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	address_request.address = TRAINING_DATA_START_ADDRESS;
	address_request.size = Get_TrainPlanDownloadDataSize();
	
	drv_ble_uart_put((uint8_t *)&address_request, sizeof(_extflash_config_train_plan_read_address_request));	
	
}

typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t address;
}_extflash_config_train_plan_write_address_request;
//APP获取手表训练计划写地址：APP在将训练计划写入闪存前询问APOLLO需要将训练计划写入闪存什么地址
static void ble_uart_command_application_extflash_config_train_plan_write_address_request(void)
{
	_extflash_config_train_plan_write_address_request address_request;
	address_request.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	address_request.address = TRAINING_DATA_START_ADDRESS;//目前暂定全部重写整个训练计划
	drv_ble_uart_put((uint8_t *)&address_request, sizeof(_extflash_config_train_plan_write_address_request));
}
typedef struct __attribute__((packed))
{
	uint32_t command;
	uint8_t md5[32];
	uint8_t uuid[32];
}_extflash_config_train_plan_result_request_uuid;
//APP获取训练计划结果MD5和UUID命令
static void ble_uart_command_application_extflash_config_train_plan_result_request_uuid(void)
{
	_extflash_config_train_plan_result_request_uuid request_uuid;
	TrainPlanUploadStr  MD5_UUIDStr;
	
	//从上传训练计划数据中取MD5和UUID
	dev_extFlash_enable();
	dev_extFlash_read(TRAINING_DATA_UPLOAD_ADDRESS,(uint8_t*)(&MD5_UUIDStr),sizeof(TrainPlanUploadStr));
	dev_extFlash_disable();
	
	memcpy(request_uuid.md5,MD5_UUIDStr.Md5Str,sizeof(MD5_UUIDStr.Md5Str));
	memcpy(request_uuid.uuid,MD5_UUIDStr.UUIDStr,sizeof(MD5_UUIDStr.UUIDStr));
	
	request_uuid.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID | BLE_UART_CCOMMAND_MASK_APPLICATION;


	drv_ble_uart_put((uint8_t *)&request_uuid, sizeof(_extflash_config_train_plan_result_request_uuid));
}
typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t result;
	uint32_t address;
	uint32_t size;
	uint32_t loopcount;
}_extflash_config_train_plan_result_read_address_request;
//APP获取训练计划结果读地址：APP在读取闪存中训练计划结果前询问APOLLO训练计划结果保存在闪存地址和数据大小 
static void ble_uart_command_application_extflash_config_train_plan_result_read_address_request(void)
{
	uint8_t year = 0,month = 0,day = 0,last_activity_index = 0;//模拟APP下发的开始查找时间
	_extflash_config_train_plan_result_read_address_request address_request;
	TrainPlanUploadBLEData UploadBLEData;
	
	year = *((uint8_t *)m_task_buffer.p_data+4);
	month = *((uint8_t *)m_task_buffer.p_data+5);
	day = *((uint8_t *)m_task_buffer.p_data+6);
	last_activity_index = *((uint8_t *)m_task_buffer.p_data+7);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:year request --> %d\n",year);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:month request --> %d\n",month);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:day request --> %d\n",day);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:last_activity_index --> %d\n",last_activity_index);
	
	UploadBLEData = Get_TrainPlanResult(year,month,day);
	address_request.result = UploadBLEData.Is_HasTodayResult;
	
	if(address_request.result == true)
	{//有数据需上传
		address_request.address = UploadBLEData.Address;
		address_request.size = UploadBLEData.Size;
		address_request.loopcount = UploadBLEData.Days;
	}
	else if(address_request.result == false){
	//无数据上传时  需完善
		address_request.address = 0;
		address_request.size = 0;
		address_request.loopcount = 0;
	}
	address_request.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;

	drv_ble_uart_put((uint8_t *)&address_request, sizeof(_extflash_config_train_plan_result_read_address_request));	
}
typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t address;
}_extflash_config_train_plan_result_write_address_request;
//APP获取训练计划结果写地址：APP在写入训练计划结果前询问APOLLO训练计划结果需要写到闪存哪个地址
static void ble_uart_command_application_extflash_config_train_plan_result_write_address_request(void)
{
	_extflash_config_train_plan_result_write_address_request address_request;
	address_request.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	address_request.address = TRAINING_DATA_UPLOAD_ADDRESS;
	drv_ble_uart_put((uint8_t *)&address_request, sizeof(_extflash_config_train_plan_result_write_address_request));
}


//APP设置手表训练计划通知开关已经训练计划开始开关
static void ble_uart_command_application_extflash_config_train_notify_switch(void)
{
	uint8_t train_plan_notify_switch = *((uint8_t *)m_task_buffer.p_data+4);
	uint8_t train_plan_switch = *((uint8_t *)m_task_buffer.p_data+5);
	
	switch(train_plan_switch)
	{
		case 0x01://训练计划开启
		{
			IsOpen_TrainPlan_Settings(true);
			if(train_plan_notify_switch == 0x02)
			{
				//训练通知开关关
				TASK_BLE_LOG_PRINTF("[TASK_BLE]:train plan switch off\n");
				Set_TrainPlan_Notify_IsOpen(false);
			}
			else if(train_plan_notify_switch == 0x01){
				//训练通知开关开
				TASK_BLE_LOG_PRINTF("[TASK_BLE]:train plan switch on\n");
				Set_TrainPlan_Notify_IsOpen(true);
			}
			else
			{
				//非定义值
				ERR_HANDLER(ERR_NOT_SUPPORTED);
			}							
		}
			break;
		case 0x02://训练计划关闭 通知也关闭
		{
			Set_TrainPlan_Notify_IsOpen(false);
			IsOpen_TrainPlan_Settings(false);
		}
			break;
		default:
			break;
	}
	uint32_t command[1] = {BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_NOTIFY_SWITCH | BLE_UART_CCOMMAND_MASK_APPLICATION};
	drv_ble_uart_put((uint8_t *)command, sizeof(command));	
	if(get_trainplan_valid_flag())
	{//有训练计划同步
		Get_TrainPlanDayRecord();
	}
	if((IsSportMode(ScreenState) != true) && (IsSportReady(ScreenState) != true))
	{
		DISPLAY_MSG  msg = {0,0};
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}
}
typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t address;
	uint32_t size;
}_extflash_config_train_plan_erase_address_request;
static void ble_uart_ccommand_application_extflash_config_train_plan_erase_address_request(void)
{
	_extflash_config_train_plan_erase_address_request address_request;
	address_request.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	address_request.address = TRAINING_DATA_START_ADDRESS;
	address_request.size = sizeof(TrainPlanDownStr)+sizeof(DownloadStr)*TRAIN_PLAN_MAX_DAYS;
	drv_ble_uart_put((uint8_t *)&address_request, sizeof(_extflash_config_train_plan_erase_address_request));
}
typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t address;
	uint32_t size;
}_extflash_config_train_plan_result_erase_address_request;
static void ble_uart_ccommand_application_extflash_config_train_plan_result_erase_address_request(void)
{
	_extflash_config_train_plan_result_erase_address_request address_request;
	address_request.command = BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	address_request.address = TRAINING_DATA_UPLOAD_ADDRESS;
	address_request.size = sizeof(TrainPlanUploadStr)+sizeof(UploadStr)*TRAIN_PLAN_MAX_DAYS;
	drv_ble_uart_put((uint8_t *)&address_request, sizeof(_extflash_config_train_plan_result_erase_address_request));
}

typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t crc;
	AppGetStr appGetStr;
}_get_watch_calculate_param;

static void ble_uart_ccommand_application_get_watch_calculate_param_write(void)
{
	_get_watch_calculate_param *get_watch_calculate_param = (_get_watch_calculate_param *)(m_task_buffer.p_data);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: crc --> 0x%08X\n",get_watch_calculate_param->crc);	
	uint32_t crc_dest = am_bootloader_fast_crc32((uint8_t*)(&get_watch_calculate_param->crc)+sizeof(get_watch_calculate_param->crc),  
														sizeof(AppGetStr));
  if(crc_dest == get_watch_calculate_param->crc)
	{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x00;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));	
		memcpy(&SetValue.AppGet,&get_watch_calculate_param->appGetStr,sizeof(AppGetStr));	
	}		
	else{
		uint32_t response[2] = {BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION};
		response[1] = 0x01;
		drv_ble_uart_put((uint8_t *)response, sizeof(response));		
	}
	
}
static void ble_uart_ccommand_application_get_watch_calculate_param_read(void)
{
	_get_watch_calculate_param get_watch_calculate_param;
	get_watch_calculate_param.command = BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_READ | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: SetValue.AppGet.LengthSwimmingPool --> %d\n",SetValue.AppGet.LengthSwimmingPool);
	memcpy(&get_watch_calculate_param.appGetStr,&SetValue.AppGet,sizeof(AppGetStr));

	TASK_BLE_LOG_PRINTF("[TASK_BLE]: get_watch_calculate_param.appGetStr.LengthSwimmingPool --> %d\n",get_watch_calculate_param.appGetStr.LengthSwimmingPool);	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]: get_watch_calculate_param.appGetStr.CalibrationStride --> %d\n",get_watch_calculate_param.appGetStr.CalibrationStride);
	get_watch_calculate_param.crc  = am_bootloader_fast_crc32((uint8_t*)(&get_watch_calculate_param.crc)+sizeof(get_watch_calculate_param.crc),  
														sizeof(AppGetStr));	
	drv_ble_uart_put((uint8_t *)&get_watch_calculate_param, sizeof(_get_watch_calculate_param));	

}


static void ble_uart_ccommand_application_reset(void)
{	
	SetValue.SwBle = BLE_DISCONNECT;
#ifdef COD 
		if (SetValue.rt_heart)
		{
			task_hrt_close();
			SetValue.rt_heart = 0;
		}
		cod_sport_cast_flag = 0;
		cod_user_sensor_close();

		SetValue.SwBle = BLE_DISCONNECT;
	if((ScreenState != DISPLAY_SCREEN_LOGO)
		&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
		&& (ScreenState != DISPLAY_SCREEN_BLE_DFU)
		&& time_notify_is_ok())
	{
    // if(((ScreenStateSave<=DISPLAY_VICE_SCREEN_SPORT_HINT)&&(ScreenStateSave>=DISPLAY_VICE_SCREEN_OUTDOOR_RUN))||((ScreenStateSave<=DISPLAY_SCREEN_TRAIN_CAST_PRESS_REMIND)&&(ScreenStateSave>=DISPLAY_SCREEN_TRAIN_CAST_START_REMIND)))

		if(((ScreenState<=DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND)&&(ScreenState>=DISPLAY_SCREEN_TRAIN_CAST))
			||((ScreenState<=DISPLAY_SCREEN_TRAIN_CAST_END_REMIND)&&(ScreenState>=DISPLAY_SCREEN_TRAIN_CAST_START_REMIND)))

		{

		if((ScreenState==DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND)||(ScreenState==DISPLAY_SCREEN_REMIND_VICE_PAUSE_CONTINUE)||(ScreenState==DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND))
			{
              timer_app_vice_pasue_stop(false);
	          set_train_pause_time(); 

		   }
			DISPLAY_MSG  msg = {0,0};
		    msg.cmd = MSG_DISPLAY_SCREEN;
			ScreenState = DISPLAY_SCREEN_HOME;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
	}
#else
		drv_ble_uart_close();
#endif
	
	if(ScreenState == DISPLAY_SCREEN_HOME)
	{
		DISPLAY_MSG  msg = {0,0};
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}	
	
}


#include "mode_power_off.h"

static void ble_uart_ccommand_app_ble_dfu_start(void)
{
	
  uint32_t command[1] = {BLE_UART_RESPONSE_APP_BLE_DFU_START | BLE_UART_CCOMMAND_MASK_APP};	
	drv_ble_uart_put((uint8_t *)command, sizeof(command));	
	
	
	
	if(ScreenState != DISPLAY_SCREEN_BLE_DFU)
	{
		mode_close();
		DISPLAY_MSG  msg = {0,0};
		ScreenState = DISPLAY_SCREEN_BLE_DFU;
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}

}
#include "gui_dfu.h"
static void ble_uart_ccommand_app_ble_dfu_finish(void)
{
	gui_dfu_timer_stop();

	
  uint32_t command[1] = {BLE_UART_RESPONSE_APP_BLE_DFU_FINISH | BLE_UART_CCOMMAND_MASK_APP};	
	drv_ble_uart_put((uint8_t *)command, sizeof(command));	

	mode_power_off_store_set_value();
	lib_app_data_erase();		
	
	DELAY_MS(1000);
	taskENTER_CRITICAL();
	am_hal_reset_por();	
	
}



static void ble_uart_ccommand_app_ble_dfu_apollo_bootsetting(void)
{

	
}


static void ble_uart_ccommand_app_ble_dfu_apollo_prevalidate(void)
{

}


static void ble_uart_ccommand_app_ble_dfu_apollo_postvalidate(void)
{

}

static void ble_uart_ccommand_app_ble_dfu_apollo_reset_n_activate(void)
{
	gui_dfu_timer_stop();	

	
  uint32_t command[1] = {BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE | BLE_UART_CCOMMAND_MASK_APP};	
	drv_ble_uart_put((uint8_t *)command, sizeof(command));	


	mode_power_off_store_set_value();
	lib_app_data_erase();	
	
	
	DELAY_MS(1000);
	lib_boot_jump_to_bootloader(BOOT_SETTING_ENTER_BOOT_REASON_DFU_BLE);
}

typedef struct __attribute__ ((packed))
{
	uint32_t cmd;
	
	uint16_t watch_face_0_index;     //手表当前表盘
	uint32_t watch_face_0_address;
	uint32_t watch_face_0_version;	
	uint32_t watch_face_0_valid;	
	uint8_t  watch_face_0_name[20];
	
	uint16_t watch_face_1_index;    //手表FLASH中第一个表盘
	uint32_t watch_face_1_address;
	uint32_t watch_face_1_version;
	uint32_t watch_face_1_valid;
	uint8_t  watch_face_1_name[20];
	
	uint16_t watch_face_2_index;
	uint32_t watch_face_2_address;
	uint32_t watch_face_2_version;
	uint32_t watch_face_2_valid;
	uint8_t  watch_face_2_name[20];

	uint16_t watch_face_3_index;
	uint32_t watch_face_3_address;
	uint32_t watch_face_3_version;	
	uint32_t watch_face_3_valid;
	uint8_t  watch_face_3_name[20];
	
	uint16_t watch_face_4_index;
	uint32_t watch_face_4_address;
	uint32_t watch_face_4_version;	
	uint32_t watch_face_4_valid;
	uint8_t  watch_face_4_name[20];
	
} _watch_face_info;


static void ble_uart_ccommand_application_get_watch_face_info(void)
{
	
	_watch_face_info info = {0};

	info.cmd = BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
	uint8_t buffer[34] = {0};
/*
第一个表盘	
*/		
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS, (uint8_t *)buffer, sizeof(buffer));	
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+34, (uint8_t *)info.watch_face_1_name, sizeof(info.watch_face_1_name));	
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024-4), (uint8_t *)&info.watch_face_1_valid, 4);	
	dev_extFlash_disable();				
	info.watch_face_1_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_1_address = CUSTOMTHEME_START_ADDRESS;
	info.watch_face_1_version = *(uint32_t *)((uint8_t *)buffer+12);

	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_1_index --> 0x%04X\n",info.watch_face_1_index);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_1_address --> 0x%08X\n",info.watch_face_1_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_1_valid --> 0x%08X\n",info.watch_face_1_valid);
	
	
/*
第二个表盘	
*/			
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024), (uint8_t *)buffer, sizeof(buffer));	
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024)+34, (uint8_t *)info.watch_face_2_name, sizeof(info.watch_face_2_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024)+(200*1024-4), (uint8_t *)&info.watch_face_2_valid, 4);
	dev_extFlash_disable();				
	info.watch_face_2_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_2_address = CUSTOMTHEME_START_ADDRESS+(200*1024);
	info.watch_face_2_version = *(uint32_t *)((uint8_t *)buffer+12);	
	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_2_index --> 0x%04X\n",info.watch_face_2_index);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_2_address --> 0x%08X\n",info.watch_face_2_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_2_valid --> 0x%08X\n",info.watch_face_2_valid);	
	
/*
第三个表盘	
*/			
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*2), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*2)+34, (uint8_t *)info.watch_face_3_name, sizeof(info.watch_face_3_name));	
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*2)+(200*1024-4), (uint8_t *)&info.watch_face_3_valid, 4);			
	dev_extFlash_disable();				
	info.watch_face_3_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_3_address = CUSTOMTHEME_START_ADDRESS+(200*1024*2);
	info.watch_face_3_version = *(uint32_t *)((uint8_t *)buffer+12);	

	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_3_index --> 0x%04X\n",info.watch_face_3_index);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_3_address --> 0x%08X\n",info.watch_face_3_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_3_valid --> 0x%08X\n",info.watch_face_3_valid);

/*
第四个表盘	
*/			
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*3), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*3)+34, (uint8_t *)info.watch_face_4_name, sizeof(info.watch_face_4_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*3)+(200*1024-4), (uint8_t *)&info.watch_face_4_valid, 4);			
	dev_extFlash_disable();			
	info.watch_face_4_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_4_address = CUSTOMTHEME_START_ADDRESS+(200*1024*3);
	info.watch_face_4_version = *(uint32_t *)((uint8_t *)buffer+12);	
	
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_4_index --> 0x%04X\n",info.watch_face_4_index);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_4_address --> 0x%08X\n",info.watch_face_4_address);
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_4_valid --> 0x%08X\n",info.watch_face_4_valid);	
	
	
	/*
	当前表盘	
	*/	

	info.watch_face_0_index = DialData.Number;
	info.watch_face_0_address = 0xFFFFFFFF;
	info.watch_face_0_version = 0xFFFFFFFF;	
	info.watch_face_0_valid = 0xFFFFFFFF;	
	memset(info.watch_face_0_name,0xFF,sizeof(info.watch_face_0_name));
		
	TASK_BLE_LOG_PRINTF("[TASK_BLE]:watch_face_0_index --> 0x%04X\n",info.watch_face_0_index);

	
	drv_ble_uart_put((uint8_t *)&info, sizeof(_watch_face_info));	
}
static void ble_uart_ccommand_application_get_watch_face_info_2(void)
{
	_watch_face_info info = {0};

	info.cmd = BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO_2 | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
	uint8_t buffer[34] = {0};
	
	
/*
空表盘	
*/			
	info.watch_face_0_index = 0xFFFF;
	info.watch_face_0_address = 0xFFFFFFFF;
	info.watch_face_0_version = 0xFFFFFFFF;	
	info.watch_face_0_version = 0xFFFFFFFF;	
	memset(info.watch_face_0_name,0xFF,sizeof(info.watch_face_0_name));

/*
空表盘	
*/				
	info.watch_face_1_index = 0xFFFF;
	info.watch_face_1_address = 0xFFFFFFFF;
	info.watch_face_1_version = 0xFFFFFFFF;	
	info.watch_face_1_version = 0xFFFFFFFF;	
	memset(info.watch_face_1_name,0xFF,sizeof(info.watch_face_1_name));	
	
/*
第五个表盘	
*/		
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*4), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*4)+34, (uint8_t *)info.watch_face_2_name, sizeof(info.watch_face_2_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*4)+(200*1024-4), (uint8_t *)&info.watch_face_2_valid, 4);			
	dev_extFlash_disable();			
	info.watch_face_2_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_2_address = CUSTOMTHEME_START_ADDRESS+(200*1024*4);
	info.watch_face_2_version = *(uint32_t *)((uint8_t *)buffer+12);	
		
	
/*
第六个表盘	
*/		
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*5), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*5)+34, (uint8_t *)info.watch_face_3_name, sizeof(info.watch_face_3_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*5)+(200*1024-4), (uint8_t *)&info.watch_face_3_valid, 4);			
	dev_extFlash_disable();			
	info.watch_face_3_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_3_address = CUSTOMTHEME_START_ADDRESS+(200*1024*5);
	info.watch_face_3_version = *(uint32_t *)((uint8_t *)buffer+12);	
	
	
/*
第七个表盘	
*/			
	dev_extFlash_enable();		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*6), (uint8_t *)buffer, sizeof(buffer));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*6)+34, (uint8_t *)info.watch_face_4_name, sizeof(info.watch_face_4_name));		
	dev_extFlash_read(CUSTOMTHEME_START_ADDRESS+(200*1024*6)+(200*1024-4), (uint8_t *)&info.watch_face_4_valid, 4);			
	dev_extFlash_disable();			
	info.watch_face_4_index = *(uint16_t *)((uint8_t *)buffer+32);
	info.watch_face_4_address = CUSTOMTHEME_START_ADDRESS+(200*1024*6);
	info.watch_face_4_version = *(uint32_t *)((uint8_t *)buffer+12);	
	
	drv_ble_uart_put((uint8_t *)&info, sizeof(_watch_face_info));	
}
static void ble_uart_ccommand_application_get_watch_face_finish(void)
{
	uint32_t cmd = BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_FINISH | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
	drv_ble_uart_put((uint8_t *)&cmd, sizeof(cmd));	
	
	if(ScreenState == DISPLAY_SCREEN_THEME)
	{
		//表盘下载成功后，重新读取表盘数据
		gui_theme_init(1);
		
		DISPLAY_MSG  msg = {0,0};
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}
	else if(ScreenState == DISPLAY_SCREEN_THEME_APP_MENU)
	{
		//表盘下载成功后，重新读取表盘数据
		gui_theme_init(0);
	}
	
#if defined WACTH_THEME_APP_DOWNLOAD
	//表盘下载成功后，重新初始化表盘
	com_dial_init();
	
	if(ScreenState == DISPLAY_SCREEN_HOME)
	{
		//在待机界面，刷新表盘
		DISPLAY_MSG  msg = {0,0};
		msg.cmd = MSG_DISPLAY_SCREEN;
		xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
	}
#endif
}
static void ble_uart_ccommand_application_get_watch_face_activate(void)
{
	uint32_t cmd = BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_ACTIVATE | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
	drv_ble_uart_put((uint8_t *)&cmd, sizeof(cmd));	
}


