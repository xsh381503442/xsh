#include "task_apollo2.h"
#include "drv_apollo2.h"
#include "dev_apollo2.h"
#include "cmd_apollo2.h"
#include "app_srv_apollo.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"

#include "nrf_drv_gpiote.h"
#include "nrf_sdm.h"
#include "cmd_gpregret.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_delay.h"
#include "cmd_pc.h"
#include "task_extFlash.h"
#include "task_ble_conn.h"
#include "nrf_power.h"
#include "app_srv_wechat.h"
#include "task_ble_accessary.h"
#include "cmd_nrf52_srv.h"
#include "cmd_apollo_srv.h"
#include "app_srv_nrf52.h"
#include "app_srv_ancs.h"

#include "drv_ble_flash.h"

#include "drv_wdt.h"
#include "watch_config.h"

#if defined (COD)
#include "cod_ble_api.h"
#include "app_srv_cod.h"

#endif

#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 						2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME 					task_apollo2
#define NRF_LOG_LEVEL       					LOG_LEVEL_INFO
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

#ifdef LAYER_BOOTLOADER  
#define DELAY_MS(X)                 nrf_delay_ms(X)
#endif

#ifdef LAYER_APPLICATION 
//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)
#endif






static bool m_ble_jump_bootloader = false;

static TaskHandle_t task_handle;



#define TASK_APOLLO_USE_QUEUE

#ifdef TASK_APOLLO_USE_QUEUE
static QueueHandle_t QueueTask = NULL;
struct s_queue_task{
	uint8_t	evt;
	uint32_t *p_data;
	uint32_t length;
	
};
typedef struct s_queue_task _queue_task;

static _queue_task queue = {0,NULL,0};

#else
typedef struct{
	uint8_t	evt;
	uint32_t *p_data;
	uint32_t length;
} _task_buffer;
static _task_buffer m_task_buffer = {0,NULL,0};

#endif


typedef struct __attribute__((packed))
{
	uint32_t command;
	uint32_t step;
	uint32_t calory;
	uint32_t distance;
}_wechat_sport_data;


extern bool g_is_central_mode;


static void	ble_uart_ccommand_application_ble_info(void);
static void ble_uart_response_application_info_apollo2(void);
#ifdef COD

_all_day_steps all_day_steps[7];
uint8_t all_day_sync_data[10];
static void ble_uart_response_application_info_bat(void);
static void ble_uart_response_application_current_time_read(void);
static void ble_uart_response_application_location_info(void);
static void ble_uart_response_application_phone_call_status(void);
static void ble_uart_response_application_phone_call_delay(void);
static void ble_uart_response_application_phone_call_ctrl(void);
static void ble_uart_response_application_on_bind_device(void);
static void ble_uart_response_application_send_bind_status(void);
static void ble_uart_response_application_cmd_cast(void);
static void ble_uart_response_application_rt_today(void);
static void ble_uart_response_application_rt_heart(void);
static void ble_uart_response_application_sport_ctrl(void);
static void ble_uart_response_application_sensor_data_trains(void);
static void ble_uart_response_application_rest_heartrate(void);
static void ble_uart_response_application_data_frame_num(void);
static void ble_uart_response_application_deletle_data(void);
static void ble_uart_response_application_mac_read(void);
static void ble_uart_response_application_mac_write(void);
static void ble_uart_response_application_updata_bp_start(void);
static void ble_uart_response_application_real_time_log_trains(void);
#endif
#ifdef WATCH_QIWEI_SPORT_DATA
static void ble_uart_response_application_sport_data_upload(void);
#endif

static void ble_uart_ccommand_applciation_jump_bootloader(void);
static void pc_uart_response_application_jump_bootloader(void);
static void pc_uart_command_hardware_test_ble_info_read(void)	;
static void pc_uart_command_hardware_test_ble_rssi_read(void);
static void pc_uart_command_hardware_test_ble_rssi_read_stop(void);
static void pc_uart_command_hardware_test_ble_extflash_read(void);
static void ble_uart_response_application_weather_info_request(void);
static void ble_uart_response_application_current_time_write(void);
static void ble_uart_response_application_personal_info_read(void);
static void ble_uart_response_application_personal_info_write(void);
static void ble_uart_response_application_sports_param_read(void);
static void ble_uart_response_application_sports_param_write(void);
static void ble_uart_response_application_app_message_android(void);
static void ble_uart_response_application_wechat_sport_data_request(void);
static void ble_uart_cdommand_application_scan_enable(void);	
static void ble_uart_cdommand_application_scan_disable(void);		
static void ble_uart_response_application_extflash_config_astro_sync_request(void);
static void ble_uart_response_application_extflash_config_main_day_data_request(void);
static void ble_uart_response_application_extflash_config_activity_address_request(void);
static void ble_uart_response_application_extflash_config_activity_address_boundary_request(void);
static void ble_uart_response_application_extflash_config_shard_track_address_request(void);
static void ble_uart_response_application_extflash_config_shard_track_header_write(void);
static void ble_uart_command_application_find_phone(void);
static void ble_uart_response_application_extflash_config_train_plan_request_uuid(void);
static void ble_uart_response_application_extflash_config_train_plan_read_address_request(void);
static void ble_uart_response_application_extflash_config_train_plan_write_address_request(void);
static void ble_uart_response_application_extflash_config_train_plan_result_request_uuid(void);
static void ble_uart_response_application_extflash_config_train_plan_result_read_address_request(void);
static void ble_uart_response_application_extflash_config_train_plan_result_write_address_request(void);
static void ble_uart_response_application_extflash_config_train_plan_notify_switch(void);
static void ble_uart_response_application_extflash_config_train_plan_erase_address_request(void);			
static void ble_uart_response_application_extflash_config_train_plan_result_erase_address_request(void);	
static void ble_uart_response_application_get_watch_calculate_param_write(void);
static void ble_uart_response_application_get_watch_calculate_param_read(void);

static void ble_uart_responese_application_sensor_data_upload(void);
static void ble_uart_application_sensor_data_upload_transfer(void);

static void ble_uart_response_app_ble_dfu_start(void);
static void ble_uart_response_app_ble_dfu_finish(void);
static void ble_uart_response_app_ble_dfu_apollo_bootsetting(void);
static void ble_uart_response_app_ble_dfu_apollo_prevalidate(void);
static void ble_uart_response_app_ble_dfu_apollo_postvalidate(void);
static void ble_uart_response_app_ble_dfu_apollo_reset_n_activate(void);
static void ble_uart_command_application_notify_msg_action_perform(void);

static void ble_uart_response_application_watch_face_info(void);
static void ble_uart_response_application_watch_face_info_2(void);
static void ble_uart_response_application_watch_face_finish(void);
static void ble_uart_response_application_watch_face_activate(void);



#ifdef SOS_ADVERTISING 
static void ble_uart_command_application_sos_gps(void);					
#endif	
static void ble_uart_response_application_find_watch(void);
static void ble_uart_response_application_get_watch_default_sport_mode(void);
static void ble_uart_response_application_send_gps_test_data(void);


#ifdef WATCH_HAS_HYTERA_FUNC
static void ble_uart_response_application_hytera_hr_collcte_start(void);
static void ble_uart_response_application_hytera_hr_collcte_stop(void);
static void ble_uart_response_application_hytera_sport_data_collcte_start(void);
static void ble_uart_response_application_hytera_sport_data_collcte_stop(void);
static void ble_uart_response_application_hytera_sport_data_read(void);
static void ble_uart_response_application_hytera_hr_data_read(void);

#endif


task_apollo2_ht_rssi_callback g_ht_rssi_callback = NULL;
extern ble_config_params_t     *g_ble_config;
#ifdef COD  
extern uint16_t cod_usr_ble_tx_mtu;
#endif


typedef struct __attribute__((packed))
{
	uint32_t command;
	union __attribute__((packed))
	{
		int32_t rssi;
		struct __attribute__((packed))
		{
			uint32_t interval;            /**< Scan interval between 0x0004 and 0x4000 in 0.625 ms units (2.5 ms to 10.24 s). */
			uint32_t window;              /**< Scan window between 0x0004 and 0x4000 in 0.625 ms units (2.5 ms to 10.24 s). */
			uint32_t timeout;             /**< Scan timeout between 0x0001 and 0xFFFF in seconds, 0x0000 disables timeout. */
			uint32_t uuid;			
		}_param;
		
	}_U;
}_hardware_test_ble_rssi;

_hardware_test_ble_rssi m_hardware_test_ble_rssi;





static void task_cb(uint32_t evt, uint8_t *p_data, uint32_t length)
{
	NRF_LOG_DEBUG("task_cb");
	
	//uart_task_cb++;
	
	#ifdef TASK_APOLLO_USE_QUEUE
	_queue_task  queue = {0};
	queue.evt = evt;
	queue.p_data = (uint32_t *)p_data;
	queue.length = length;
	

	BaseType_t yield_req = pdFALSE;
	xQueueSendFromISR( QueueTask, &queue, &yield_req );

	/* Switch the task if required. */
	portYIELD_FROM_ISR(yield_req);

	#else 
	m_task_buffer.evt = evt;
	m_task_buffer.p_data = (uint32_t *)p_data;
	m_task_buffer.length = length;
	BaseType_t yield_req = pdFALSE;
	vTaskNotifyGiveFromISR(task_handle, &yield_req);

	
	/* Switch the task if required. */
	portYIELD_FROM_ISR(yield_req);
     #endif
	

	
}
extern bool g_app_error_reset_flag;
extern bool g_ble_dfu_finish_flag;
extern bool g_app_other_reset_flag;


static void PC_UART_CCOMMAND_MASK_APPLICATION_POSITION_FUNC(void)
{

							#ifdef TASK_APOLLO_USE_QUEUE
							switch(queue.p_data[0]&0x0000FF00)
							#else
							switch(m_task_buffer.p_data[0]&0x0000FF00)
							#endif 
							{
								case BLE_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER");
									ble_uart_ccommand_applciation_jump_bootloader();
								}break;
								
								
								case BLE_UART_CCOMMAND_APPLICATION_BLE_INFO:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_BLE_INFO");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_ccommand_application_ble_info();
								}break;
								case BLE_UART_RESPONSE_APPLICATION_INFO_APOLLO2:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_INFO_APOLLO2");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_info_apollo2();
								}break;
								#ifdef COD
								case BLE_UART_RESPONSE_APPLICATION_INFO_BAT:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_INFO_BAT");
									ble_uart_response_application_info_bat();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_READ:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_READ");
									ble_uart_response_application_current_time_read();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_LOCATION_INFO:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_LOCATION_INFO");
									ble_uart_response_application_location_info();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_STATUS:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_STATUS");
									ble_uart_response_application_phone_call_status();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_DELAY:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_DELAY");
									ble_uart_response_application_phone_call_delay();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_CTRL:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_CTRL");
									ble_uart_response_application_phone_call_ctrl();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_ON_BIND_DEVICE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_ON_BIND_DEVICE");
									ble_uart_response_application_on_bind_device();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_SEND_BIND_STATUS:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_SEND_BIND_STATUS");
									ble_uart_response_application_send_bind_status();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_CMD_CAST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_CMD_CAST");
									ble_uart_response_application_cmd_cast();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_RT_TODAY:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_RT_TODAY");
									ble_uart_response_application_rt_today();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_RT_HEART:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_RT_HEART");
									ble_uart_response_application_rt_heart();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_SPORT_CTRL:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_SPORT_CTRL");
									ble_uart_response_application_sport_ctrl();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_SENSOR_DATA_TRAINS:{
									//NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_SENSOR_DATA_TRAINS");
									ble_uart_response_application_sensor_data_trains();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_REST_HEARTRATE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_ON_BIND_DEVICE");
									ble_uart_response_application_rest_heartrate();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_DATA_FRAME_NUM:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_DATA_FRAME_NUM");
									ble_uart_response_application_data_frame_num();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_DELETE_DATA:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_DELETE_DATA");
									ble_uart_response_application_deletle_data();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_MAC_READ:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_MAC_READ");
									ble_uart_response_application_mac_read();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_MAC_WRITE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_MAC_READ");
									ble_uart_response_application_mac_write();
									}break;
							    case BLE_UART_RESPONSE_APPLICATION_UPDATA_BP_START:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_UPDATA_BP_START");
									ble_uart_response_application_updata_bp_start();
									}break;
								case BLE_UART_RESPONSE_APPLICATION_REAL_TIME_LOG_TRAINS:{
									//NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_SENSOR_DATA_TRAINS");
									ble_uart_response_application_real_time_log_trains();
									}break;
								#endif
								#ifdef WATCH_QIWEI_SPORT_DATA
								case BLE_UART_RESPONSE_APPLICATION_SPORT_DATA_UPLOAD:{
									//NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_SPORT_DATA_UPLOAD");
									ble_uart_response_application_sport_data_upload();
									}break;
								#endif
								case PC_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER:{
									NRF_LOG_INFO("PC_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									pc_uart_response_application_jump_bootloader();
								}break;						
			
								case BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_WRITE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_WRITE");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_current_time_write();
								}break;									
								case BLE_UART_RESPONSE_APPLICATION_WEATHER_INFO_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_WEATHER_INFO_REQUEST");
									//NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_weather_info_request();
								}break;									
								case BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_READ:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_READ");
									//NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_personal_info_read();
								}break;		
								case BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_WRITE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_WRITE");
									//NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_personal_info_write();
								}break;				
								case BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_READ:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_READ");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_sports_param_read();
								}break;		
								case BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_WRITE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_WRITE");
									//NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_sports_param_write();
								}break;			
								case BLE_UART_RESPONSE_APPLICATION_APP_MESSAGE_ANDROID:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_APP_MESSAGE_ANDROID");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_app_message_android();
								}break;	
								case BLE_UART_RESPONSE_APPLICATION_WECHAT_SPORT_DATA_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_WECHAT_SPORT_DATA_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_wechat_sport_data_request();
								}break;		
								case BLE_UART_CCOMMAND_APPLICATION_SCAN_ENABLE:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_SCAN_ENABLE");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_cdommand_application_scan_enable();
								}break;	
								case BLE_UART_CCOMMAND_APPLICATION_SCAN_DISABLE:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_SCAN_DISABLE");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_cdommand_application_scan_disable();
								}break;
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_astro_sync_request();
								}break;					
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_MAIN_DAY_DATA_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_MAIN_DAY_DATA_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_main_day_data_request();
								}break;		
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_activity_address_request();
								}break;		
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_activity_address_boundary_request();
								}break;									
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_ADDRESS_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_ADDRESS_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_shard_track_address_request();
								}break;			
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_HEADER_WRITE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_HEADER_WRITE");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_shard_track_header_write();
								}break;								
								case BLE_UART_CCOMMAND_APPLICATION_FIND_PHONE:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_FIND_PHONE");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_command_application_find_phone();
								}break;										
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_request_uuid();
								}break;									
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_read_address_request();
								}break;				
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_write_address_request();
								}break;				
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_result_request_uuid();
								}break;				
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_result_read_address_request();
								}break;				
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_result_write_address_request();
								}break;				
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_NOTIFY_SWITCH:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_NOTIFY_SWITCH");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_notify_switch();
								}break;												
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_erase_address_request();
								}break;				
								case BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_extflash_config_train_plan_result_erase_address_request();
								}break;												

								case BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_WRITE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_WRITE");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_get_watch_calculate_param_write();
								}break;	
								case BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_READ:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_READ");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_get_watch_calculate_param_read();
								}break;	
								case BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_watch_face_info();
								}break;		
								case BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO_2:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO_2");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_watch_face_info_2();
								}break;								
								case BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_FINISH:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_FINISH");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_watch_face_finish();
								}break;	
								case BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_ACTIVATE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_ACTIVATE");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_watch_face_activate();
								}break;									
								
								
								case BLE_UART_CCOMMAND_APPLICATION_NOTIFY_MSG_ACTION_PERFORM:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_NOTIFY_MSG_ACTION_PERFORM");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_command_application_notify_msg_action_perform();
								}break;	
								
								
#ifdef SOS_ADVERTISING 
								case BLE_UART_CCOMMAND_APPLICATION_SOS_GPS:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_SOS_GPS");
									ble_uart_command_application_sos_gps();
								}break;							
#endif								


								case BLE_UART_RESPONSE_APPLICATION_FIND_WATCH:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_FIND_WATCH");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_find_watch();
								}break;

								case BLE_UART_RESPONSE_APPLICATION_GET_WATCH_DEFAULT_SPORT_MODE:{
								    NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_GET_WATCH_DEFAULT_SPORT_MODE");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_get_watch_default_sport_mode();
								}break;
								
								case BLE_UART_CCOMMAND_APPLICATION_SEND_GPS_TEST_DATA:{
								    NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_SEND_GPS_TEST_DATA");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_send_gps_test_data();
								}break;

								case BLE_UART_RESPONSE_APPLICATION_SENSOR_DATA_UPLOAD:{
								    NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_SENSOR_DATA_UPLOAD");
									NRF_LOG_HEXDUMP_INFO(queue.p_data, 11);
									ble_uart_responese_application_sensor_data_upload();
								}break;
								case BLE_UART_APPLICATION_SENSOR_DATA_UPLOAD_TRANSFER:{
								    NRF_LOG_INFO("BLE_UART_APPLICATION_SENSOR_DATA_UPLOAD_TRANSFER");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_application_sensor_data_upload_transfer();
								}break;
							#if defined (WATCH_HAS_HYTERA_FUNC)
								case BLE_UART_RESPONSE_APPLICATION_HYTERA_HEARTRATE_COLLECTION_START:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_HYTERA_HEARTRATE_COLLECTION_START");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_hytera_hr_collcte_start();
								}break;
								case BLE_UART_RESPONSE_APPLICATION_HYTERA_HEARTRATE_COLLECTION_STOP:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_HYTERA_HEARTRATE_COLLECTION_STOP");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_hytera_hr_collcte_stop();
								}break;
								case BLE_UART_RESPONSE_APPLICATION_HYTERA_SPORT_DATA_COLLECTION_START:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_HYTERA_SPORT_DATA_COLLECTION_START");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_hytera_sport_data_collcte_start();
								}break;
								case BLE_UART_RESPONSE_APPLICATION_HYTERA_SPORT_DATA_COLLECTION_STOP:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_HYTERA_SPORT_DATA_COLLECTION_STOP");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_hytera_sport_data_collcte_stop();
								}break;
								case BLE_UART_RESPONSE_APPLICATION_HYTERA_SPORT_DATA_READ:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_HYTERA_SPORT_DATA_READ");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_hytera_sport_data_read();					
								}break;
								case BLE_UART_RESPONSE_APPLICATION_HYTERA_HEARTRATE_DATA_READ:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APPLICATION_HYTERA_HEARTRATE_DATA_READ");
									NRF_LOG_HEXDUMP_INFO(g_ble_config->dev_name.name, 11);
									ble_uart_response_application_hytera_hr_data_read();					
								}break;
								
							#endif
								default:{
									NRF_LOG_INFO("UNSPPORTED COMMAND");
									drv_apollo2_uart_close();	
								}break;
							}				

}
static void task(void * pvParameter)
{
    NRF_LOG_DEBUG("task");
	(void)pvParameter;	
	
	if(g_app_error_reset_flag || g_app_other_reset_flag)
	{
		NRF_LOG_INFO("g_app_error_reset_flag");
		uint32_t command = BLE_UART_CCOMMAND_APPLICATION_RESET | BLE_UART_CCOMMAND_MASK_APPLICATION;	
		_uart_data_param param;
		param.user_callback = NULL;
		param.p_data = (uint8_t *)&command;
		param.length = sizeof(command);
		uint8_t count = TASK_UART_DELAY_COUNT_OUT;
		while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
		{
			DELAY_MS(TASK_UART_DELAY);
		}		
	
	}

	if(g_ble_dfu_finish_flag)
	{
		NRF_LOG_INFO("g_ble_dfu_finish_flag");
		uint32_t command = BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE | BLE_UART_CCOMMAND_MASK_APP;	
		_uart_data_param param;
		param.user_callback = NULL;
		param.p_data = (uint8_t *)&command;
		param.length = sizeof(command);
		uint8_t count = TASK_UART_DELAY_COUNT_OUT;
		while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
		{
			DELAY_MS(TASK_UART_DELAY);
		}	
	}
	
	
		while (1)
		{

			#ifdef TASK_APOLLO_USE_QUEUE	
			if (xQueueReceive(QueueTask, &queue, portMAX_DELAY) == pdTRUE)
			{
			
			 // NRF_LOG_ERROR("queue %x\r\n",queue.p_data);
			  switch(queue.evt)
			 
			#else

			(void) ulTaskNotifyTake(pdTRUE,/* Clear the notification value before exiting (equivalent to the binary semaphore). */
                                portMAX_DELAY); /* Block indefinitely (INCLUDE_vTaskSuspend has to be enabled).*/

				
		
				switch(m_task_buffer.evt)
				#endif
	
					

				{
					case APP_UART_TIMEOUT:
						NRF_LOG_DEBUG("APP_UART_TIMEOUT");
					case APP_UART_DATA_READY:{
						//NRF_LOG_DEBUG("APP_UART_DATA_READY");
						//NRF_LOG_HEXDUMP_INFO(m_task_buffer.p_data, 8);
						#ifdef TASK_APOLLO_USE_QUEUE
						if(queue.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_APPLICATION_POSITION)<<8))
						#else
						if(m_task_buffer.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_APPLICATION_POSITION)<<8))
						#endif
						{
							//NRF_LOG_DEBUG("PC_UART_CCOMMAND_MASK_APPLICATION_POSITION");
							PC_UART_CCOMMAND_MASK_APPLICATION_POSITION_FUNC();
						}
						#ifdef TASK_APOLLO_USE_QUEUE
						else if(queue.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_BOOTLOADER_POSITION)<<8))
						{
							switch(queue.p_data[0]&0x0000FF00)
						#else
						else if(m_task_buffer.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_BOOTLOADER_POSITION)<<8))
						{
							switch(m_task_buffer.p_data[0]&0x0000FF00)
					    #endif
							{
								case BLE_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER:{
									NRF_LOG_INFO("BLE_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER");
									ble_uart_ccommand_applciation_jump_bootloader();
								}break;	


							
								default:{
									NRF_LOG_INFO("UNSPPORTED COMMAND");
									drv_apollo2_uart_close();
								}break;
							}							
					
						}
						#ifdef TASK_APOLLO_USE_QUEUE
						else if(queue.p_data[0] & ((1<<BLE_UART_CCOMMAND_MASK_APP_POSITION)<<8))
						{
							switch(queue.p_data[0]&0x0000FF00)
						#else
						else if(m_task_buffer.p_data[0] & ((1<<BLE_UART_CCOMMAND_MASK_APP_POSITION)<<8))
						{
							switch(m_task_buffer.p_data[0]&0x0000FF00)
						#endif
							{
								case BLE_UART_RESPONSE_APP_BLE_DFU_START:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APP_BLE_DFU_START");
									ble_uart_response_app_ble_dfu_start();
								}break;
								case BLE_UART_RESPONSE_APP_BLE_DFU_FINISH:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APP_BLE_DFU_FINISH");
									ble_uart_response_app_ble_dfu_finish();
								}break;
								case BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_BOOTSETTING:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_BOOTSETTING");
									ble_uart_response_app_ble_dfu_apollo_bootsetting();
								}break;
								case BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_PREVALIDATE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_PREVALIDATE");
									ble_uart_response_app_ble_dfu_apollo_prevalidate();
								}break;								
								case BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_POSTVALIDATE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_POSTVALIDATE");
									ble_uart_response_app_ble_dfu_apollo_postvalidate();
								}break;										
								case BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE:{
									NRF_LOG_INFO("BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE");
									ble_uart_response_app_ble_dfu_apollo_reset_n_activate();
								}break;


								
								default:{
									NRF_LOG_INFO("UNSPPORTED COMMAND");
									drv_apollo2_uart_close();
								}break;
							}								
							
						}			
						
						#ifdef TASK_APOLLO_USE_QUEUE
						else if(queue.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_HARDWARE_TEST_POSITION)<<8))
						{
							switch(queue.p_data[0]&0x0000FF00)
						#else
						else if(m_task_buffer.p_data[0] & ((1<<PC_UART_CCOMMAND_MASK_HARDWARE_TEST_POSITION)<<8))
						{
							switch(m_task_buffer.p_data[0]&0x0000FF00)
						#endif
							{
								case PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ:{
									NRF_LOG_INFO("PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ");
									pc_uart_command_hardware_test_ble_info_read();
								}break;			
								case PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ:{
									NRF_LOG_INFO("PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ");
									pc_uart_command_hardware_test_ble_rssi_read();
								}break;	
								case PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_STOP:{
									NRF_LOG_INFO("PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_STOP");
									pc_uart_command_hardware_test_ble_rssi_read_stop();
								}break;	
								case PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ:{
									NRF_LOG_INFO("PC_UART_COMMAND_HARDWARE_TEST_BLE_EXTFLASH_READ");
									pc_uart_command_hardware_test_ble_extflash_read();
								}break;

							
								default:{
									NRF_LOG_INFO("UNSPPORTED COMMAND");
									drv_apollo2_uart_close();
								}break;
							}								
							
						}
						else{
							NRF_LOG_INFO("UNSPPORTED MASK");
							drv_apollo2_uart_close();
						}			
						
				


					}break;	
	
					case APP_UART_TX_EMPTY:{
						NRF_LOG_DEBUG("APP_UART_TX_EMPTY");
					}break;
					case APP_UART_COMMUNICATION_ERROR:{
						NRF_LOG_DEBUG("APP_UART_COMMUNICATION_ERROR");
					}break;
					case APP_UART_FIFO_ERROR:{
						NRF_LOG_DEBUG("APP_UART_FIFO_ERROR");
					}break;		
	
					
					case APP_UART_CTS_INTERRUPT:{
						NRF_LOG_DEBUG("APP_UART_CTS_INTERRUPT");
//						nrf_delay_us(5);
						if (nrf_drv_gpiote_in_is_set(APOLLO2_CTS_PIN_NUMBER))
						{
							drv_apollo2_on_cts_high();
						}
						else
						{
							drv_apollo2_on_cts_low();
							if(m_ble_jump_bootloader)
							{
//								if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
//								{
//									taskENTER_CRITICAL();
//								}
//								uint32_t err_code;
//								uint8_t softdevice_enabled;
//								err_code = sd_softdevice_is_enabled(&softdevice_enabled);
//								APP_ERROR_CHECK(err_code);
//								if(softdevice_enabled)
//								{
//									err_code = sd_softdevice_disable();
//									APP_ERROR_CHECK(err_code);	
//								}			
//								nrf_power_gpregret_set(CMD_GPREGRET_BOOTLOADER_PC_DFU);								
//								NVIC_SystemReset();
								
									
									uint32_t err_code;
									err_code = drv_ble_flash_delete();
									APP_ERROR_CHECK(err_code);
									err_code = sd_power_gpregret_clr(0, 0xffffffff);
									APP_ERROR_CHECK(err_code);
									err_code = sd_power_gpregret_set(0, CMD_GPREGRET_BOOTLOADER_PC_DFU);
									APP_ERROR_CHECK(err_code);		
									drv_wdt_feed();
									nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);						
							}							
						}							
					}break;						
					default:{
						
					}break;
				}
#ifdef TASK_APOLLO_USE_QUEUE
			}
#endif
			
    }
}


void task_apollo2_init(void * p_context)
{
  NRF_LOG_DEBUG("task_apollo2_init");

	

	drv_apollo2_init(task_cb);
		
	
	

	BaseType_t xReturned = xTaskCreate(task,
																		 "APOL",
																		 256+64,
																		 p_context,
																		 _PRIO_APP_HIGH,
																		 &task_handle);
	if (xReturned != pdPASS)
	{
			NRF_LOG_ERROR("task not created.");
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}

    #ifdef TASK_APOLLO_USE_QUEUE
	QueueTask = xQueueCreate( 5,sizeof( _queue_task ));
	#endif

	

}


static void	ble_uart_ccommand_application_ble_info(void)
{
	_dev_apollo2_ble_info ble_info;
	dev_apollo2_ble_info_get(&ble_info);
	drv_apollo2_uart_put((uint8_t *)&ble_info, sizeof(_dev_apollo2_ble_info));
}	

_info_apollo2 g_info_apollo2;
static void ble_uart_response_application_info_apollo2(void)									
{
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	memcpy((uint8_t *)&g_info_apollo2,buffer,sizeof(_info_apollo2));
	#if defined (COD)
	NRF_LOG_INFO("g_info_apollo2.pcb_version --> %x,%x",(uint8_t)(g_info_apollo2.pcb_version >>8),(uint8_t)g_info_apollo2.pcb_version);
	NRF_LOG_INFO("g_info_apollo2.application_version --> %x,%X",(uint8_t)(g_info_apollo2.application_version>>8),(uint8_t)g_info_apollo2.application_version);
	cod_cmd_device_version_resp((uint8_t)(g_info_apollo2.pcb_version >>8),(uint8_t)g_info_apollo2.pcb_version,
				(uint8_t)(g_info_apollo2.application_version>>8),(uint8_t)g_info_apollo2.application_version);
	#else
	NRF_LOG_INFO("g_info_apollo2.bootloader_version --> 0x%08X",g_info_apollo2.bootloader_version);	
	NRF_LOG_INFO("g_info_apollo2.application_version --> 0x%08X",g_info_apollo2.application_version);
	NRF_LOG_INFO("g_info_apollo2.update_version --> 0x%08X",g_info_apollo2.update_version);
	NRF_LOG_INFO("g_info_apollo2.pcb_version --> 0x%08X",g_info_apollo2.pcb_version);
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
	#endif
}
#ifdef COD
extern uint8_t g_battery_level;
static void ble_uart_response_application_info_bat(void)									
{
	uint32_t command[2];
	cod_battery_status ele_status;
	uint8_t ele;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)command,(uint8_t *)queue.p_data, sizeof(command));
	#else
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	#endif
	drv_apollo2_uart_close();
	ele = (uint8_t)(command[1] >>8);
	ele_status = (cod_battery_status)(command[1]&0xFF);
	NRF_LOG_INFO("%s-->%d,%d\r\n",__func__,ele,ele_status);	
	cod_cmd_device_ele_resp(ele, ele_status);
	g_battery_level = ele;

}

static void ble_uart_response_application_current_time_read(void)									
{
	_ble_current_time ble_app_time;
	cod_ble_time time;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)&ble_app_time,(uint8_t *)queue.p_data, sizeof(ble_app_time));
	#else
	memcpy((uint8_t *)&ble_app_time,(uint8_t *)m_task_buffer.p_data, sizeof(ble_app_time));
	#endif
	drv_apollo2_uart_close();

	time.year = ble_app_time.year +2000;
	time.month = ble_app_time.month;
	time.day = ble_app_time.date;          
	time.hour = ble_app_time.hour;         
	time.minute = ble_app_time.minute;         
	time.second = ble_app_time.second; 
	if (ble_app_time.day_of_week == 0){//星期格式转换
		time.week = 6;
	}
	else{
		time.week = ble_app_time.day_of_week -1;
	}
   	time.time_format = 0;   
   	time.time_zone = ble_app_time.time_zone;
	NRF_LOG_INFO("%s-->%d,%d,%d,\r\n",__func__,time.year,time.month,time.day);	
	NRF_LOG_INFO("time %d,%d,%d,%d\r\n",time.hour,time.minute,time.second,time.week);

	cod_cmd_get_time_resp(&time);

}

static void ble_uart_response_application_location_info(void)									
{
	drv_apollo2_uart_close();
	NRF_LOG_INFO("%s \r\n",__func__);	
	cod_cmd_set_location_info_resp();

}

static void ble_uart_response_application_phone_call_status(void)									
{
	drv_apollo2_uart_close();
	NRF_LOG_INFO("%s \r\n",__func__);	
	cod_cmd_sync_phone_call_status_resp();

}

static void ble_uart_response_application_phone_call_delay(void)									
{
	drv_apollo2_uart_close();
	NRF_LOG_INFO("%s \r\n",__func__);	
	cod_cmd_set_phone_call_virb_delay_resp();

}

static void ble_uart_response_application_phone_call_ctrl(void)									
{
	uint32_t command[2];
	cod_ble_phone_call_ctrl ctrl;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)command,(uint8_t *)queue.p_data, sizeof(command));
	#else
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	#endif
	drv_apollo2_uart_close();
	ctrl = (cod_ble_phone_call_ctrl)(command[1]);
	NRF_LOG_INFO("%s-->%d\r\n",__func__,ctrl);	
	cod_cmd_phone_call_ctrl_req(ctrl);

}

static void ble_uart_response_application_on_bind_device(void)									
{
	uint32_t command[2];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)command,(uint8_t *)queue.p_data, sizeof(command));
	#else
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	#endif
	drv_apollo2_uart_close();
	bool is_success = (bool)(command[1]);
	NRF_LOG_INFO("%s-->%d\r\n",__func__,is_success);	
	cod_cmd_bind_device_resp(is_success);

}
static void ble_uart_response_application_send_bind_status(void)									
{

	drv_apollo2_uart_close();
	NRF_LOG_INFO("%s\r\n",__func__);	
	cod_cmd_send_bind_status_resp();

}

static void ble_uart_response_application_cmd_cast(void)									
{
	cod_ble_cast_result result;
	uint32_t command[2];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)command,(uint8_t *)queue.p_data, sizeof(command));
	#else
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	#endif
	drv_apollo2_uart_close();
	result  = (cod_ble_cast_result)(command[1]);;
	NRF_LOG_INFO("%s: %d\r\n",__func__,result);	
	cod_cmd_cast_resp(result);

}

static void ble_uart_response_application_rt_today(void)									
{
	uint32_t command[4];
	float calorie;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)command,(uint8_t *)queue.p_data, sizeof(command));
	#else
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	#endif
	drv_apollo2_uart_close();
	calorie =  command[3]/1000 + 0.1*(command[3]%1000/100) +0.01*(command[3]%100/10)+0.001 *(command[3]%10);
	NRF_LOG_INFO("%s-->%d,%d,%d\r\n",__func__,command[1],command[2],command[3]);//卡路里传输卡,需转换	
	cod_cmd_rt_today_summary_data_resp(command[1],command[2],calorie);

}
static void ble_uart_response_application_rt_heart(void)									
{
	uint32_t command[2];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)command,(uint8_t *)queue.p_data, sizeof(command));
	#else
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	#endif
	drv_apollo2_uart_close();
	uint8_t heart = (command[1]);
	NRF_LOG_INFO("%s-->%d\r\n",__func__,heart);	
	cod_cmd_rt_heart_bp_bo_resp(heart,0,0,0);

}

static void ble_uart_response_application_sport_ctrl(void)									
{
	uint32_t command[2];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)command,(uint8_t *)queue.p_data, sizeof(command));
	#else
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	#endif
	drv_apollo2_uart_close();
	cod_ble_sport_ctrl ctrl = (cod_ble_sport_ctrl)(command[1]);
	NRF_LOG_INFO("%s-->%d\r\n",__func__,ctrl);	
	cod_cmd_sport_ctrl_req(ctrl);

}

static void ble_uart_response_application_sensor_data_trains(void)									
{
	uint8_t data[256];
	uint16_t len;
	uint32_t command;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)&command,(uint8_t *)queue.p_data, sizeof(command));
	len = command >> 24 ;
	if (len > 252)
	{
		len = 252;
	}
	memcpy((uint8_t *)data,(uint8_t *)(queue.p_data +1), len);
	#else
	memcpy((uint8_t *)&command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	len = command >> 24 ;
	if (len > 252)
	{
		len = 252;
	}

	memcpy((uint8_t *)data,(uint8_t *)(m_task_buffer.p_data +1), len);
	#endif
	drv_apollo2_uart_close();
	NRF_LOG_INFO("%d,%d",len,(data[0] >> 8|data[1]));	
	cod_cmd_sensor_data_trans_resp(data,len);

}


extern uint8_t cod_uart_normal_open;
static void ble_uart_response_application_real_time_log_trains(void)									
{
	uint8_t data[256];
	uint16_t len;
	uint32_t command;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)&command,(uint8_t *)queue.p_data, sizeof(command));
	len = command >> 24 ;
	if (len > 252)
	{
		len = 252;
	}
	memcpy((uint8_t *)data,(uint8_t *)(queue.p_data +1), 252);
	#else
	memcpy((uint8_t *)&command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	len = command >> 24 ;
	if (len > 252)
	{
		len = 252;
	}
	memcpy((uint8_t *)data,(uint8_t *)(m_task_buffer.p_data +1), len);
	#endif
	drv_apollo2_uart_close();
	NRF_LOG_ERROR("hrt log %d,%d",len,(uint16_t)(data[250] << 8|data[251]));	
	cod_cmd_rt_heart_log_resp(data,len);

}

static void ble_uart_response_application_rest_heartrate(void)									
{
	drv_apollo2_uart_close();
	NRF_LOG_INFO("%s\r\n",__func__);	
	cod_cmd_set_rest_heart_rate_resp();


}

static void ble_uart_response_application_data_frame_num(void)									
{
	uint8_t data[256];
	uint16_t len;
	uint32_t num = 0;
	uint32_t temp;
	uint16_t frame_len = cod_usr_ble_tx_mtu-8; //减数据头8
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)&temp,(uint8_t *)(queue.p_data +1), sizeof(temp));
	len = temp >> 16;
	memcpy((uint8_t *)data,(uint8_t *)(queue.p_data +2), len);
	#else
	memcpy((uint8_t *)&temp,(uint8_t *)(m_task_buffer.p_data +1), sizeof(temp));
	len = temp >> 16;
	memcpy((uint8_t *)data,(uint8_t *)(m_task_buffer.p_data +2), len);
	#endif
	drv_apollo2_uart_close();

	cod_ble_data_type type = (cod_ble_data_type)(temp&0xFFFF);
	//NRF_LOG_INFO("%s len %d,%d,%x\r\n",__func__,len,type,temp);	
	if( type==DATA_TYPE_STEP)
	{
		memcpy((uint8_t *)all_day_steps,(uint8_t *)data,sizeof(all_day_steps));
		memcpy((uint8_t *)all_day_sync_data,(uint8_t *)data +sizeof(all_day_steps),sizeof(all_day_sync_data));

		for (uint8_t i = 0;i < 7;i++)
		{
			NRF_LOG_INFO("size %d\r\n",all_day_steps[i].size);	
			if (all_day_steps[i].size != 0)
			{
				num += all_day_steps[i].size;
			}
		}
	   num += sizeof(all_day_sync_data);
	   NRF_LOG_WARNING("%s len %d",__func__,num);	
	   if ( (num %frame_len) == 0)
	   {
			num = num /frame_len;
	   }
	   else
	   {
			num = (num /frame_len) +1;
	   }
	
	}
	NRF_LOG_INFO("%s %d,%d\r\n",__func__,type,num);	
	cod_cmd_get_data_frame_num_resp(type,num,true);

}

static void ble_uart_response_application_deletle_data(void)									
{
	uint32_t command[2];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)command,(uint8_t *)queue.p_data, sizeof(command));
	#else
	memcpy((uint8_t *)command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	#endif
	drv_apollo2_uart_close();
	cod_ble_data_type type = (cod_ble_data_type)(command[1]);
	NRF_LOG_INFO("%s-->%d\r\n",__func__,type);	
	cod_cmd_delete_data_resp(type,true);

}

static void ble_uart_response_application_mac_read(void)									
{
	uint32_t	 err_code;
	ble_gap_addr_t device_addr;
	_ble_mac ble_mac;
	memset(&ble_mac,0,sizeof(_ble_mac));
	memset(&device_addr,0,sizeof(ble_gap_addr_t));
	err_code = sd_ble_gap_addr_get(&device_addr);

	for ( uint8_t i = 6; i >0;)
	{	
		i--;
		ble_mac.mac[5-i]= device_addr.addr[i];
	}
	NRF_LOG_INFO("[%s] err %d\r\n",__func__,err_code);
	NRF_LOG_INFO("read ble_mac %x:%x:%x:%x:%x:%x\r\n",ble_mac.mac[0],ble_mac.mac[1],ble_mac.mac[2],ble_mac.mac[3],ble_mac.mac[4],ble_mac.mac[5]);
	ble_mac.cmd = BLE_UART_CCOMMAND_APPLICATION_MAC_READ | BLE_UART_CCOMMAND_MASK_APPLICATION;
	drv_apollo2_uart_put((uint8_t *)&ble_mac, sizeof(_ble_mac));
	
}

static void ble_uart_response_application_mac_write(void)									
{
	_ble_mac ble_mac;
	
	uint32_t err_code;
	ble_gap_addr_t device_addr,device_read;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)&ble_mac,(uint8_t *)queue.p_data, sizeof(_ble_mac));
	#else
	memcpy((uint8_t *)&ble_mac,(uint8_t *)m_task_buffer.p_data, sizeof(_ble_mac));
	#endif
	NRF_LOG_INFO("receive ble_mac %x:%x:%x:%x:%x:%x\r\n",ble_mac.mac[0],ble_mac.mac[1],ble_mac.mac[2],ble_mac.mac[3],ble_mac.mac[4],ble_mac.mac[5]);
	//ble_mac.mac[0] =0xCC;
	//sd_ble_gap_adv_stop()
	err_code = sd_ble_gap_addr_get(&device_addr);
	for ( uint8_t i = 6; i >0;)
	{	
		i--;
		device_addr.addr[5-i]= ble_mac.mac[i];
	}

	device_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
	NRF_LOG_INFO("device_addr %x:%x:%x:%x:%x:%x\r\n",device_addr.addr[0],device_addr.addr[1],device_addr.addr[2],device_addr.addr[3],device_addr.addr[4],device_addr.addr[5]); 
	err_code = drv_ble_flash_cod_mac_write(&device_addr);
	DELAY_MS(200);
	//sd_ble_gap_addr_set(&device_addr);
	NRF_LOG_INFO("[%s] %d,%d err %x\r\n",__func__,device_addr.addr_id_peer,device_addr.addr_type,err_code);

	//读出mac
	memset(&device_read,0,sizeof(ble_gap_addr_t));
	
	err_code = drv_ble_flash_cod_mac_read(&device_read);
	//sd_ble_gap_addr_get(&device_read);
	NRF_LOG_INFO("device_read %x:%x:%x:%x:%x:%x\r\n",device_read.addr[0],device_read.addr[1],device_read.addr[2],device_read.addr[3],device_read.addr[4],device_read.addr[5]);
	for ( uint8_t i = 6; i >0;)
	{	
		i--;
		ble_mac.mac[5-i]= device_read.addr[i];
	}

	ble_mac.cmd = BLE_UART_CCOMMAND_APPLICATION_MAC_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION;
	NRF_LOG_INFO("read ble_mac %x:%x:%x:%x:%x:%x\r\n",ble_mac.mac[0],ble_mac.mac[1],ble_mac.mac[2],ble_mac.mac[3],ble_mac.mac[4],ble_mac.mac[5]);
	drv_apollo2_uart_put((uint8_t *)&ble_mac, sizeof(_ble_mac));
	
}

static void ble_uart_response_application_updata_bp_start(void)									
{
	NRF_LOG_DEBUG("ble_uart_response_app_ble_dfu_start");
	drv_apollo2_uart_close();
	cod_cmd_ota_start_resp(cod_usr_ble_tx_mtu,10,200,FILE_TYPE_BP);;	
	
}


#endif

#ifdef WATCH_QIWEI_SPORT_DATA
static void ble_uart_response_application_sport_data_upload(void)									
{
	uint8_t data[47];
	uint16_t len;
	uint32_t command;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)&command,(uint8_t *)queue.p_data, sizeof(command));
	
	memcpy((uint8_t *)data,(uint8_t *)(queue.p_data +1), 47);
	#else
	memcpy((uint8_t *)&command,(uint8_t *)m_task_buffer.p_data, sizeof(command));
	

	memcpy((uint8_t *)data,(uint8_t *)(m_task_buffer.p_data +1), 47);
	#endif
	drv_apollo2_uart_close();
	NRF_LOG_INFO("ble_uart_response_application_sport_data_upload %x:%x:%x\r\n",data[0],data[2],data[46]);	
	app_srv_apollo_flash_send(data, sizeof(data));

}
#endif
static void ble_uart_ccommand_applciation_jump_bootloader(void)									
{

	//	uint32_t command[1] = {BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER | BLE_UART_CCOMMAND_MASK_APPLICATION};		
//	drv_apollo2_uart_put((uint8_t *)&command, sizeof(command));			
	drv_apollo2_uart_close();
	m_ble_jump_bootloader = true;//????????????λ??????????????bootloader

}
static void pc_uart_response_application_jump_bootloader(void)
{
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
}	
static void pc_uart_command_hardware_test_ble_info_read(void)									
{
	_dev_apollo2_ble_info ble_info;
	dev_apollo2_ble_info_get(&ble_info);
	ble_info.command = PC_UART_COMMAND_HARDWARE_TEST_BLE_INFO_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
	drv_apollo2_uart_put((uint8_t *)&ble_info, sizeof(_dev_apollo2_ble_info));
	

}


static void scan_start(_hardware_test_ble_rssi *param)
{
	ret_code_t err_code;
	ble_gap_scan_params_t scan_params;
	scan_params.active = 1;
	scan_params.interval = param->_U._param.interval;
	scan_params.window = param->_U._param.window;
	scan_params.timeout = param->_U._param.timeout;
	scan_params.use_whitelist = 0;
	
	
	(void) sd_ble_gap_scan_stop();

	err_code = sd_ble_gap_scan_start(&scan_params);
	// It is okay to ignore this error since we are stopping the scan anyway.
	if (err_code != NRF_ERROR_INVALID_STATE)
	{
			APP_ERROR_CHECK(err_code);
	}
}


static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index + 1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index + 2];
            p_typedata->data_len = field_length - 1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
}
static bool find_adv_uuid(const ble_gap_evt_adv_report_t *p_adv_report, const uint16_t uuid_to_find)
{
    ret_code_t err_code;
    data_t     adv_data;
    data_t     type_data;

    // Initialize advertisement report for parsing.
    adv_data.p_data     = (uint8_t *)p_adv_report->data;
    adv_data.data_len   = p_adv_report->dlen;

    err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE,
                                &adv_data,
                                &type_data);

    if (err_code != NRF_SUCCESS)
    {
        // Look for the services in 'complete' if it was not found in 'more available'.
        err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE,
                                    &adv_data,
                                    &type_data);

        if (err_code != NRF_SUCCESS)
        {
            // If we can't parse the data, then exit.
            return false;
        }
    }

    // Verify if any UUID match the given UUID.
    for (uint32_t u_index = 0; u_index < (type_data.data_len / sizeof(uint16_t)); u_index++)
    {
        uint16_t extracted_uuid;

        UUID16_EXTRACT(&extracted_uuid, &type_data.p_data[u_index * sizeof(uint16_t)]);

        if (extracted_uuid == uuid_to_find)
        {
            return true;
        }
    }
    return false;
}
static void ht_rssi_callback(ble_gap_evt_adv_report_t const* p_adv_report)
{
	ble_gap_addr_t p_addr 					= p_adv_report->peer_addr;
	int8_t rssi 										= p_adv_report->rssi;
	
	if (find_adv_uuid(p_adv_report, m_hardware_test_ble_rssi._U._param.uuid))
	{
		NRF_LOG_DEBUG("FOUND TARGET DEVICE");
		m_hardware_test_ble_rssi.command = PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST;
		m_hardware_test_ble_rssi._U.rssi = rssi;
		NRF_LOG_HEXDUMP_DEBUG(p_addr.addr, 6);		
		NRF_LOG_DEBUG("rssi --> %d dbm",m_hardware_test_ble_rssi._U.rssi );
		
		task_ble_conn_handle(TASK_BLE_CONN_HWT_RSSI_READ, (uint8_t *)&m_hardware_test_ble_rssi, 8);	
		
	}	
}


static void pc_uart_command_hardware_test_ble_rssi_read(void)									
{
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));	
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));	
	#endif
	drv_apollo2_uart_close();
	_hardware_test_ble_rssi *hardware_test_ble_rssi = (_hardware_test_ble_rssi *)buffer;
	
	NRF_LOG_DEBUG("interval --> 0x%04X",hardware_test_ble_rssi->_U._param.interval);
	NRF_LOG_DEBUG("window --> 0x%04X",hardware_test_ble_rssi->_U._param.window);
	NRF_LOG_DEBUG("timeout --> 0x%04X",hardware_test_ble_rssi->_U._param.timeout);
	NRF_LOG_DEBUG("uuid --> 0x%04X",hardware_test_ble_rssi->_U._param.uuid);
	
	g_ht_rssi_callback = ht_rssi_callback;
	
	memcpy(&m_hardware_test_ble_rssi,hardware_test_ble_rssi,sizeof(_hardware_test_ble_rssi));
	scan_start(&m_hardware_test_ble_rssi);
}


static void pc_uart_command_hardware_test_ble_rssi_read_stop(void)
{
	uint32_t command[1] = {PC_UART_COMMAND_HARDWARE_TEST_BLE_RSSI_READ_STOP_RESPONSE | PC_UART_COMMAND_MASK_HARDWARE_TEST};
	drv_apollo2_uart_put((uint8_t *)command, sizeof(command));									
	g_ht_rssi_callback = NULL;
	(void) sd_ble_gap_scan_stop();
	

	
}
static void pc_uart_command_hardware_test_ble_extflash_read(void)
{
	drv_apollo2_uart_close();
	uint8_t buffer[256];
	for(uint32_t i=0;i<sizeof(buffer);i++)
	{
		buffer[i] = i;
	}
	task_extFlash_handle(TASK_EXTFLASH_EVT_HWT, 0, buffer, sizeof(buffer));
}	
static void ble_uart_response_application_weather_info_request(void)
{
#ifdef COD 
	drv_apollo2_uart_close();
	NRF_LOG_INFO("%s \r\n",__func__);
	cod_cmd_sync_weather_resp();
#else
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
#endif
}	

static void ble_uart_response_application_current_time_write(void)
{
#ifdef COD
	uint32_t response[2];
	bool success;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)response,(uint8_t *)queue.p_data, sizeof(response));
	#else
	memcpy((uint8_t *)response,(uint8_t *)m_task_buffer.p_data, sizeof(response));
	#endif
	drv_apollo2_uart_close();
	success = (bool)response[1];
	NRF_LOG_INFO("%s ,%d\r\n",__func__,success);	
	cod_cmd_set_time_resp(success);
	
#else
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
#endif
}

static void ble_uart_response_application_personal_info_read(void)
{
#ifdef COD 
	_ble_user_info cod_user_info;
	cod_ble_user_info userInfo;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)&cod_user_info,(uint8_t *)queue.p_data, sizeof(cod_user_info));
	#else
	memcpy((uint8_t *)&cod_user_info,(uint8_t *)m_task_buffer.p_data, sizeof(cod_user_info))
	#endif
	drv_apollo2_uart_close();

	userInfo.gender = (cod_ble_gender)cod_user_info.gender;
	userInfo.age = cod_user_info.age;
	userInfo.height = cod_user_info.height;
	userInfo.weight = cod_user_info.weight;
	NRF_LOG_INFO("%s ,%d,%d,%d,%d\r\n",__func__,userInfo.gender,userInfo.age,userInfo.height,userInfo.weight);
	cod_cmd_get_user_info_resp(&userInfo);
#else
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
#endif
}	

static void ble_uart_response_application_personal_info_write(void)
{
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	#ifdef COD 
	NRF_LOG_INFO("%s \r\n",__func__);
	cod_cmd_set_user_info_resp();
	#else
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
	#endif
}	
static void ble_uart_response_application_sports_param_read(void)
{
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
}	
static void ble_uart_response_application_sports_param_write(void)
{
#ifdef COD
	drv_apollo2_uart_close();
	NRF_LOG_INFO("%s \r\n",__func__);
	cod_cmd_set_step_target_resp();
#else
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
#endif
}	

static void ble_uart_response_application_app_message_android(void)
{
#ifdef COD 
	drv_apollo2_uart_close();
	NRF_LOG_INFO("%s \r\n",__func__);
	cod_cmd_notify_msg_resp();
#else
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
#endif
}	

static void ble_uart_response_application_wechat_sport_data_request(void)
{
	#ifndef COD 
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	
	_wechat_sport_data *wechat_sport_data = (_wechat_sport_data *)buffer;

	NRF_LOG_DEBUG("wechat_sport_data->step --> %d",wechat_sport_data->step);
	NRF_LOG_DEBUG("wechat_sport_data->calory --> %d",wechat_sport_data->calory);
	NRF_LOG_DEBUG("wechat_sport_data->distance --> %d",wechat_sport_data->distance);
	
	uint8_t pedo_data[10];
	pedo_data[0] = (1<<WECHAT_STEP | 1<<WECHAT_CALORY | 1<<WECHAT_DISTANCE);			
	pedo_data[1] = wechat_sport_data->step;
	pedo_data[2] = wechat_sport_data->step>>8;
	pedo_data[3] = wechat_sport_data->step>>16;		
	pedo_data[4] = wechat_sport_data->distance;
	pedo_data[5] = wechat_sport_data->distance>>8;
	pedo_data[6] = wechat_sport_data->distance>>16;		
	pedo_data[7] = wechat_sport_data->calory;
	pedo_data[8] = wechat_sport_data->calory>>8;
	pedo_data[9] = wechat_sport_data->calory>>16;		
	
	app_srv_wechat_pedo_meas_send(pedo_data,sizeof(pedo_data));		
#endif	
}

static void ble_uart_cdommand_application_scan_enable(void)
{
	drv_apollo2_uart_close();	
	task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_SCAN_ENABLE, NULL, 0);	

}


static void ble_uart_cdommand_application_scan_disable(void)
{
	drv_apollo2_uart_close();	
	task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_SCAN_DISABLE, NULL, 0);	

}

static void ble_uart_response_application_extflash_config_astro_sync_request(void)
{
#ifdef COD
	uint32_t command = 0;
	bool has_agps;
	uint16_t update_time;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)&command,(uint8_t *)&queue.p_data[1],sizeof(command));
	#else
	memcpy((uint8_t *)&command,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command));
	#endif
	drv_apollo2_uart_close();	
	has_agps = (bool)(command >> 16);
	update_time = (uint16_t)(command & 0xFFFF);
	NRF_LOG_INFO("%s %d,%d\r\n",__func__,has_agps,update_time);
	cod_cmd_agps_status_resp(has_agps,update_time);
#else
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_RRQUEST_ASTRO_SYNC_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);	
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();	
	app_srv_apollo_flash_send(command, sizeof(command));
#endif
}

static void ble_uart_response_application_extflash_config_main_day_data_request(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_MAIN_DAY_DATA_RRQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();	
	app_srv_apollo_flash_send(command, sizeof(command));	
}

static void ble_uart_response_application_extflash_config_activity_address_request(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_ACTIVITY_ADDRESS_RRQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();	
	app_srv_apollo_flash_send(command, sizeof(command));	
}

static void ble_uart_response_application_extflash_config_activity_address_boundary_request(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_RRQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();	
	app_srv_apollo_flash_send(command, sizeof(command));		
}


static void ble_uart_response_application_extflash_config_shard_track_address_request(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_SHARED_TRACK_ADDRESS_REQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));	
}
static void ble_uart_response_application_extflash_config_shard_track_header_write(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_SHARED_TRACK_HEADER_WRITE_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_command_application_find_phone(void)
{
	
	uint8_t command[20] = {CMD_NRF52_SRV_FIND_PHONE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_nrf52_control_send(command, sizeof(command));
}

static void ble_uart_response_application_extflash_config_train_plan_request_uuid(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_response_application_extflash_config_train_plan_read_address_request(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_response_application_extflash_config_train_plan_write_address_request(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_response_application_extflash_config_train_plan_result_request_uuid(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_response_application_extflash_config_train_plan_result_read_address_request(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_response_application_extflash_config_train_plan_result_write_address_request(void)
{
	
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_response_application_extflash_config_train_plan_notify_switch(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_NOTIFY_SWITCH_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_response_application_extflash_config_train_plan_erase_address_request(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}	
static void ble_uart_response_application_extflash_config_train_plan_result_erase_address_request(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}
static void ble_uart_response_application_get_watch_calculate_param_write(void)
{
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
}
static void ble_uart_response_application_get_watch_calculate_param_read(void)
{
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));
}

static void ble_uart_response_app_ble_dfu_start(void)
{
	#ifndef COD 
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_DFU_START_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
	#else
	NRF_LOG_DEBUG("ble_uart_response_app_ble_dfu_start");
	drv_apollo2_uart_close();

	cod_cmd_ota_start_resp(cod_usr_ble_tx_mtu,10,200,FILE_TYPE_OTA);
  #endif	
}

static void ble_uart_response_app_ble_dfu_finish(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_FINISH_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));
}

static void ble_uart_response_app_ble_dfu_apollo_bootsetting(void)
{

}
static void ble_uart_response_app_ble_dfu_apollo_prevalidate(void)
{

}
static void ble_uart_response_app_ble_dfu_apollo_postvalidate(void)
{

}
static void ble_uart_response_app_ble_dfu_apollo_reset_n_activate(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_DFU_BLE_DFU_APOLLO_RESET_N_ACTIVATE_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	#ifndef COD 
	app_srv_apollo_flash_send(command, sizeof(command));
	#endif

}

static void ble_uart_command_application_notify_msg_action_perform(void)
{
	uint32_t buffer[64];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)buffer,(uint8_t *)queue.p_data, sizeof(buffer));
	#else
	memcpy((uint8_t *)buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));
	#endif
	drv_apollo2_uart_close();


	app_srv_ancs_msg_action_perform(buffer[1]);

	
	
}


static void ble_uart_response_application_watch_face_info(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_INFO_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));	
}
static void ble_uart_response_application_watch_face_info_2(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_INFO_2_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));	
}
static void ble_uart_response_application_watch_face_finish(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_FINISH_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));	
}
static void ble_uart_response_application_watch_face_activate(void)
{
	uint8_t command[180] = {CMD_APOLLO_SRV_FLASH_DFU_BLE_WATCH_FACE_ACTIVATE_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_apollo_flash_send(command, sizeof(command));	
}
#ifdef SOS_ADVERTISING 
#include "ble_advertising.h"
#include "main.h"
extern uint16_t g_conn_handle_peripheral;
static void ble_uart_command_application_sos_gps(void)
{
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));	
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));	
	#endif
	drv_apollo2_uart_close();
	_sos_cmd *sos_cmd = (_sos_cmd *)buffer;
	if(sos_cmd->subcmd == SOS_SUBCMD_START)
	{
		NRF_LOG_INFO("SOS_SUBCMD_START");	
		ble_advertising_sos_gps_update((uint8_t *)&sos_cmd->msg, sizeof(sos_cmd->msg));
		if(g_conn_handle_peripheral != BLE_CONN_HANDLE_INVALID)
		{
			NRF_LOG_INFO("connected");	
			uint8_t response = CMD_NRF52_SRV_SOS_GPS;
			app_srv_nrf52_control_send(&response, sizeof(response));			
		}
	}
	else if(sos_cmd->subcmd == SOS_SUBCMD_FINISH)
	{
		NRF_LOG_INFO("SOS_SUBCMD_FINISH");
		ble_advertising_sos_gps_finish();
		
	}
	
}					
#endif	




static void ble_uart_response_application_find_watch(void)
{	
	uint8_t command[20] = {CMD_NRF52_SRV_FIND_WATCH_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif
	drv_apollo2_uart_close();
	app_srv_nrf52_control_send(command, sizeof(command));
}

static void ble_uart_response_application_get_watch_default_sport_mode(void)
{	
	uint8_t command[20] = {CMD_NRF52_SRV_GET_WATCH_DEFAULT_SPORT_MODE_RESPONSE};
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(command+1,(uint8_t *)&queue.p_data[1],sizeof(command)-1);
	#else
	memcpy(command+1,(uint8_t *)&m_task_buffer.p_data[1],sizeof(command)-1);
	#endif	
	drv_apollo2_uart_close();
	app_srv_nrf52_control_send(command, sizeof(command));
}

static void ble_uart_response_application_send_gps_test_data(void)
{	
	uint8_t buffer[256];
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy(buffer,(uint8_t *)queue.p_data, sizeof(buffer));	
	#else
	memcpy(buffer,(uint8_t *)m_task_buffer.p_data, sizeof(buffer));	
	#endif
	drv_apollo2_uart_close();
	NRF_LOG_DEBUG("ble_uart_response_application_send_gps_test_data");
	app_srv_apollo_uart_send((uint8_t *)buffer, sizeof(buffer));

	//uint32_t command[1] = {BLE_UART_RESPONSE_APPLICATION_SEND_GPS_TES_DATA | PC_UART_COMMAND_MASK_HARDWARE_TEST};	
	//drv_apollo2_uart_put((uint8_t *)command, sizeof(command));
}

static void ble_uart_responese_application_sensor_data_upload(void)
{

	
	uint8_t buffer[4]= {0};
	uint32_t resp[4];
	#ifdef TASK_APOLLO_USE_QUEUE
		memcpy(resp,(uint8_t *)&queue.p_data[1], sizeof(resp)); 
	#else
		memcpy(resp,(uint8_t *)&m_task_buffer.p_data[1], sizeof(resp)); 
	#endif
		buffer[0] = resp[0];
		buffer[1] = resp[1];
		buffer[2] = resp[2];
		buffer[3] = resp[3];
		NRF_LOG_INFO("ble_uart_responese_application_sensor_data_upload,%x,%x,%x",queue.p_data[0],queue.p_data[1],queue.p_data[2]);
		NRF_LOG_INFO("%x,%x-%x,%x",buffer[0],buffer[1],resp[0], resp[1]);
		drv_apollo2_uart_close();
		app_srv_apollo_sensor_send((uint8_t *)buffer, sizeof(buffer));
	
}


static void ble_uart_application_sensor_data_upload_transfer(void)
{

	
	uint8_t data[256];
	uint16_t len;
	uint32_t command;
	#ifdef TASK_APOLLO_USE_QUEUE
	memcpy((uint8_t *)data,(uint8_t *)&queue.p_data[1], 252);
	#else
	memcpy((uint8_t *)data,(uint8_t *)&m_task_buffer.p_data[1], 252);
	#endif
	drv_apollo2_uart_close();
	
	len = data[2] + 3;
	app_srv_apollo_sensor_send((uint8_t *)data, len);
	
}


