#include "app_srv_cod.h"
#include "ble_srv_nrf52.h"
#include "drv_ble_flash.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "cmd_nrf52_srv.h"
#include "dev_apollo2.h"
#include "cmd_gpregret.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "peer_manager.h"
#include "drv_apollo2.h"
#include "cmd_apollo2.h"
#include "task_ble_conn.h"
#include "drv_ble_flash.h"
#include "nrf_ble_gatt.h"
#include "ble_db_discovery.h"
#include "app_srv_ancs.h"
#include "watch_config.h"
#include "ble_bas.h"
#include "am_bootloader.h"
#include "semphr.h"

#include "cod_ble_api.h"
#include "dev_extFlash.h"
#include "task_ble_conn.h"
#include "nrf_pwr_mgmt.h"
#include "drv_wdt.h"

#include "app_srv_apollo.h"
#include "task_extFlash.h"
#include "com_ringbuffer.h"


#define COD_BLEPRINT_LOG_ENABLE				
#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 						2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME app_srv_cod
#define NRF_LOG_LEVEL       					LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

#define FUNC_COMPILE  1

//#define DELAY_MS(X)                 nrf_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)

#define EXT_FLASH_OAT_ADDRESS    		0x700000  //ota外部flahs存储起始地址
#define EXT_FLASH_OTA_SIZE       		0x100000  //ota外部flahs存储大小
#define EXT_FLASH_END_ADDRESS    		0x800000  //ota外部flahs存储结束地址
#define APPLICATION_ADDRESS_OFFSET      1024      //APPLICATION偏移地址
#define EXT_FLASH_ASTRO_ADDRESS			0x278000  //星历下载地址
#define EXT_FLASH_DIAL_ADDRESS 			0x336000  //咕咚自定义表盘下载地址
#define DATA_DEBUG 0

BLE_BAS_DEF(m_bas);

cod_mtu_size cod_usr_ble_tx_mtu = MTU_155;
static cod_ble_basic_process_cb cod_usr_ble_basic_process_cb = {0};
static cod_ble_data_trans_process_cb cod_usr_ble_data_sync_process_cb = {0};
static cod_ble_ota_process_cb cod_usr_ble_ota_process_cb = {0};
static cod_ble_cmd_gen_cb cod_usr_ble_cmd_gen_cb = {0};
static cod_ble_log cod_usr_ble_log = {0};
static cod_ble_memory_delegate cod_usr_ble_memory_delegate = {0};

cod_ble_dev_param usr_dev_param = {0};

extern uint8_t cod_uart_normal_open;
static uint32_t cod_ble_ota_file_size = 0;
static uint32_t cod_ble_ota_currentAddr = EXT_FLASH_OAT_ADDRESS;
static cod_ble_file_type cod_ble_ota_file_type = FILE_TYPE_OTA;

cod_ble_msg_push_setting cod_msg_push_setting ;
extern _drv_ble_flash_ancs_switch g_ancs_switch;

extern uint16_t g_conn_handle_peripheral;
extern nrf_ble_gatt_t m_gatt;
extern SemaphoreHandle_t SportSemaphore;
extern uint8_t frame_id_flag;
static bool check_ble_is_updata(void);
static bool check_apollo_is_updata(void);


extern void cmd_nrf52_srv_bond_request(void);

cod_sport_data_converter sport_converter;
cod_swim_data_converter swim_converter;
cod_sport_data_convert_callback sport_callback;
uint8_t sport_buffer[50];
#if 0
uint8_t send_buffer[MTU_155];
uint8_t remain_buffer[MTU_155];
uint8_t send_buffer_len,remain_buffer_len
#endif	
uint8_t get_data_flag; // 0-获取帧数; 1-获取数据;
uint32_t total_size, last_total_size,sport_summary_size,swim_summary_size;
uint16_t cur_frame_id,total_frame_nums,cur_sport_id; 
uint32_t cur_total_size[210];
uint8_t get_data_buf[3];

extern void ble_conn_params_set_conn_params(ble_adv_evt_t new_adv_params);




void read_cod_ble_dev_param(cod_ble_dev_param *usr_param)
{
		dev_extFlash_enable();
		dev_extFlash_read( COD_BLE_DEV_START_ADDRESS,(uint8_t*)(usr_param),sizeof(cod_ble_dev_param));
		dev_extFlash_disable();
		NRF_LOG_DEBUG("%s: %d\r\n",__func__,usr_param->crc);
		if (usr_param->crc==0xFFFFFFFF)
		{
			memset(usr_param,0,sizeof(cod_ble_dev_param));
		}
}

void write_cod_ble_dev_param(cod_ble_dev_param *usr_param)
{
	uint32_t read_crc;
	usr_param->crc = am_bootloader_fast_crc32((uint8_t*)(&usr_param->crc)+sizeof(usr_param->crc),  
													sizeof(cod_ble_dev_param)-sizeof(usr_param->crc));	
	dev_extFlash_enable();
	dev_extFlash_read(COD_BLE_DEV_START_ADDRESS,(uint8_t*)(&read_crc),sizeof(read_crc));
	if (read_crc != usr_param->crc)
	{
		dev_extFlash_erase(COD_BLE_DEV_START_ADDRESS,COD_BLE_DEV_SIZE);
		dev_extFlash_write(COD_BLE_DEV_START_ADDRESS,(uint8_t*)(usr_param),sizeof(cod_ble_dev_param));
	}
	dev_extFlash_disable();
	NRF_LOG_DEBUG("%s: %x,%x\r\n",__func__,usr_param->crc,read_crc);

	
	
}


void delete_sport_data(void)
{

	//将Act_Stop_Time清零，作为删除标准。
	rtc_time_t stop_time = {0};
	rtc_time_t read_stop_time = {0};
	uint32_t stop_time_offset = 8;
	dev_extFlash_enable();
	for (uint8_t day = 0;day< 7;day++)
	{	
		for (uint8_t num = 0; num < DAY_ACTIVITY_MAX_NUM; num++)
		{
			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*day+ACTIVITY_DATA_START_ADDRESS + 
																ACTIVITY_DAY_DATA_SIZE*num + stop_time_offset ,(uint8_t*)(&read_stop_time),sizeof(rtc_time_t));
			if (read_stop_time.Day >= 1 && read_stop_time.Day <= 31)
			{
				NRF_LOG_DEBUG("%s: %x\r\n",__func__,read_stop_time.Day);
				dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*day+ACTIVITY_DATA_START_ADDRESS + 
																ACTIVITY_DAY_DATA_SIZE*num + stop_time_offset ,(uint8_t*)(&stop_time),sizeof(rtc_time_t));
			}
		}
	}
	dev_extFlash_disable();

	cod_cmd_delete_data_resp(DATA_TYPE_SPORT,true);
}


#define SRV_APOLLO_RESET_TYPE_JUMP_BOOTLOADER 1
#define TIMER_INTERVAL            			  5000 
#define OSTIMER_WAIT_FOR_QUEUE                2 
static TimerHandle_t m_timer; 
static uint8_t m_reset_type = 0;

//全局变量
uint8_t g_battery_level = 0;

static void timeout_handler(TimerHandle_t xTimer)
{
	uint32_t err_code;
    UNUSED_PARAMETER(xTimer);
    NRF_LOG_DEBUG("timeout_handler");
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
	
	if(m_reset_type == SRV_APOLLO_RESET_TYPE_JUMP_BOOTLOADER)
	{
		NRF_LOG_DEBUG("jump bootloader start sd_power_gpregret_clr");
		err_code = sd_power_gpregret_clr(0, 0xffffffff);
		APP_ERROR_CHECK(err_code);
		err_code = sd_power_gpregret_set(0, CMD_GPREGRET_BOOTLOADER_BLE_DFU);
		APP_ERROR_CHECK(err_code);		
		drv_wdt_feed();	
		nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);
		NRF_LOG_DEBUG("jump bootloader end sd_power_gpregret_set");
	}	
}

static void timer_create(void)
{
	m_timer = xTimerCreate("DFUR",
							 TIMER_INTERVAL,
							 pdFALSE,
							 NULL,
							 timeout_handler);
	if (NULL == m_timer)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}
static void timer_start(uint32_t timeout_msec)
{
	if( xTimerChangePeriod( m_timer, timeout_msec , OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		NRF_LOG_DEBUG("The command was successfully sent.");
	}
	else
	{
		NRF_LOG_DEBUG("The command could not be sent, even after waiting for 100 ticks");
		NRF_LOG_DEBUG("to pass.  Take appropriate action here.");
	}
}
void drv_ble_send_param_data(uint8_t *p_data,uint32_t length)
{
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = p_data;
	param.length =length;	
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
	while(drv_ble_send_data(&param) != NRF_SUCCESS && count--)
	{
		DELAY_MS(500);
	}
}


/**
 * 指令处理异常
 * @param error
 */
static void on_process_error(cod_cmd_process_error error)
{
	NRF_LOG_ERROR("[%s]:%d\r\n",__func__,error);
}

/**
 * 连接测试请求
 * @param phone_tx_max 手机的最大发送MTU
 * @param phone_rx_max 手机的最大接收MTU
 */
static void on_connection_test(uint16_t phone_tx_max, uint16_t phone_rx_max)
{
	NRF_LOG_INFO("[%s]tx %d.rx %d\r\n",__func__,phone_tx_max,phone_rx_max);
	
	uint16_t ble_mtu;
	
	ble_mtu =MIN(phone_tx_max,phone_rx_max);

	if (ble_mtu < cod_usr_ble_tx_mtu)
	{
		cod_usr_ble_tx_mtu = MTU_20;
		cod_ble_set_tx_mtu(cod_usr_ble_tx_mtu);
	}

	cod_cmd_connect_test_resp(cod_usr_ble_tx_mtu, cod_usr_ble_tx_mtu);
}

/**
 * 获取设备版本请求
 */
static void on_get_device_version()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);

	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_INFO_APOLLO2 | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
	
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));
	
}

/**
 * 获取设备电量请求
 */
static void on_get_device_battery()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_INFO_BAT | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));

	
}

static void task_ble_conn_evt_cod_cmd_device_mac_resp(void)
{

	uint32_t			   err_code;
	ble_gap_addr_t device_addr;
	cod_ble_mac_address mac= {0};
	err_code = sd_ble_gap_addr_get(&device_addr);

	

	for ( uint8_t i = 6; i >0;)
	{	
		i--;
		mac.content[5-i]= device_addr.addr[i];
	}
	NRF_LOG_INFO("[%s] %d\r\n",__func__,err_code);
	NRF_LOG_INFO("mac %x,%x,%x,%x,%x,%x\r\n",mac.content[0],mac.content[1],mac.content[2],mac.content[3],mac.content[4],mac.content[5]);
	cod_cmd_device_mac_resp(usr_dev_param.product_type, &mac);
}

/**
 * 获取mac地址请求
 */
static void on_get_device_mac()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	//task_ble_conn_handle(TASK_BLE_CONN_EVT_COD_CMD_DEVICE_MAC_RESP,NULL,0);
	task_ble_conn_evt_cod_cmd_device_mac_resp();

}

/**
 * 获取星历状态
 */
static void on_check_agps()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION};		
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));
}
#if (FUNC_COMPILE == 0)
/**
 * 手机端请求同步IMEI和IMSI
 */
static void on_sync_imei_imsi()
{
	
}

#endif
static void on_set_ios_ancs(bool status)
{
	NRF_LOG_INFO("[%s] %d\r\n",__func__,status);

	if (status){
		cmd_nrf52_srv_bond_request();
		#if 0
		uint32_t error;
		// Discover peer's services.
		extern ble_db_discovery_t m_db_disc[];
		error  = ble_db_discovery_start(&m_db_disc[DISCONVER_ARRAY_ANCS], g_conn_handle_peripheral);
		if (error != NRF_ERROR_BUSY )
		{
			APP_ERROR_CHECK(error);
		}
		#endif 
	}

	cod_cmd_ios_ancs_resp();

}

static void on_set_device_mac(uint16_t product_type, cod_ble_mac_address *mac_address)
{

	
	uint32_t err_code;
	ble_gap_addr_t device_addr;
	
	//usr_dev_param.product_type= product_type;
	
	err_code = sd_ble_gap_addr_get(&device_addr);
	NRF_LOG_INFO("[%s] type %d\r\n",__func__,product_type);
	
	for ( uint8_t i = 6; i >0;)
	{	
		NRF_LOG_INFO("%x",mac_address->content[i]);
		i--;
		device_addr.addr[5-i]= mac_address->content[i];
	}
	 
	err_code = sd_ble_gap_addr_set(&device_addr);
	NRF_LOG_INFO("[%s] err %d\r\n",__func__,err_code);
	cod_cmd_set_device_mac_resp();
	
}

/**
 * 手机绑定请求
 * @param platform app 平台
 * @param name 用户名
 * @param user_id 用户id
 */
static void on_bind_device(cod_app_platform platform,
					   char *name, uint8_t name_len,
					   char *user_id, uint8_t user_id_len)
{
	

	NRF_LOG_HEXDUMP_INFO(name,name_len);
	NRF_LOG_HEXDUMP_INFO(user_id,user_id_len);
	
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_ON_BIND_DEVICE | BLE_UART_CCOMMAND_MASK_APPLICATION};
	usr_dev_param.platform = (uint8_t)platform;
	usr_dev_param.name_len = name_len;
	if (name_len > 20)
	{
		usr_dev_param.name_len = 20;
		
	}
	memcpy((uint8_t *)usr_dev_param.name,(uint8_t *)name,usr_dev_param.name_len);
	usr_dev_param.name[19] = 0;
	usr_dev_param.user_id_len = user_id_len;
	if (user_id_len >20)
	{
		usr_dev_param.user_id_len = 20;
	
	}
	memcpy((uint8_t *)usr_dev_param.user_id,(uint8_t *)user_id,usr_dev_param.user_id_len);
	usr_dev_param.user_id[19] = 0;

	NRF_LOG_INFO("[%s] %d,%s,%d,%s,%d\r\n",__func__,usr_dev_param.platform,usr_dev_param.name,usr_dev_param.name_len,usr_dev_param.user_id,usr_dev_param.user_id_len);

	write_cod_ble_dev_param(&usr_dev_param);
	
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));
	
}

/**
 * 设备解绑
 */
static void on_unbind_device()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
}

/**
 * 查询设备绑定关系
 */
static void on_check_bind()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	 //NRF_LOG_HEXDUMP_INFO(queue.p_data, queue.length);
	 bool is_bind;
	 //pm_conn_sec_status_t conn_sec_status;
	// pm_conn_sec_status_get(g_conn_handle_peripheral, &conn_sec_status);
	
	 read_cod_ble_dev_param(&usr_dev_param);
	 if (usr_dev_param.bind_status == STATUS_BIND_SUCCESS)
	 {
		 is_bind = true;
	 }
	 else
	 {
		 is_bind = false;
	 }
	 NRF_LOG_INFO("TASK_BLE_CONN_EVT_COD_CMD_CHECK_BIND_RESP  %d, %d,%d,%s,%d,%s\n",is_bind, (cod_app_platform)usr_dev_param.platform,usr_dev_param.name_len,usr_dev_param.name,usr_dev_param.user_id_len,usr_dev_param.user_id );
	 cod_cmd_check_bind_resp(is_bind, (cod_app_platform)usr_dev_param.platform,usr_dev_param.name_len,usr_dev_param.name,usr_dev_param.user_id_len,usr_dev_param.user_id);

}



/**
 * 发送绑定状态
 * @param status
 */
static void on_send_bind_status(cod_bind_status status)
{
	uint32_t command[2] = {BLE_UART_CCOMMAND_APPLICATION_SEND_BIND_STATUS | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
	command[1] = (cod_bind_status)status;
	usr_dev_param.bind_status = status; 
	write_cod_ble_dev_param(&usr_dev_param);
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));

}


#if (FUNC_COMPILE == 0)
/**
 * 手机端对硬件的主动同步IMEI和IMSI的回复
 */
static void on_sync_imei_imsi_resp()
{
	
}
#endif

/**
 * 时间设置
 * @param time
 */
static void on_set_time(cod_ble_time *time)
{
	
	
	NRF_LOG_INFO("on_set_time %d,%x,%x,%x,%x,%x\r\n",(int16_t)time->year,time->month,time->day,time->hour,time->minute,time->second);
	_ble_current_time ble_time;

	ble_time.command = BLE_UART_CCOMMAND_APPLICATION_CURRENT_TIME_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	ble_time.year  = time->year-2000;
	ble_time.month = time->month;
	ble_time.date  = time->day;
	ble_time.hour  = time->hour;
	ble_time.minute= time->minute;
	ble_time.second =time->second;
	if(time->week < 6){
		ble_time.day_of_week = time->week +1;//星期定义格式转换
	}
	else{
		ble_time.day_of_week = 0;//星期天格式转换
	}
	ble_time.time_zone = time ->time_zone;

	ble_time.crc = am_bootloader_fast_crc32((uint8_t*)(&ble_time.crc)+sizeof(ble_time.crc),  
														sizeof(_ble_current_time)-sizeof(ble_time.crc)-sizeof(ble_time.command));	
	//NRF_LOG_HEXDUMP_DEBUG(&ble_time, sizeof(_ble_current_time));
	drv_ble_send_param_data((uint8_t *)&ble_time,sizeof(_ble_current_time));
}

/**
 * 时间读取
 */
static void on_get_time()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);

	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_CURRENT_TIME_READ | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
	//NRF_LOG_HEXDUMP_DEBUG(&command, sizeof(command))
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));

}
#if (FUNC_COMPILE == 0)

/**
 * 闹钟设置
 * @param alarm_list
 */
static void on_set_alarm(cod_ble_alarm_list* alarm_list)
{
	
}
#endif

#if (FUNC_COMPILE == 0)
/**
 * 闹钟读取
 */
static void on_get_alarm()
{
	
}
#endif
/**
 * 用户信息设置
 * @param userInfo
 */
static void on_set_user_info(cod_ble_user_info *userInfo)
{
	
	_ble_user_info cod_user_info;
	cod_user_info.command = BLE_UART_CCOMMAND_APPLICATION_PERSONAL_INFO_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	cod_user_info.gender = (uint8_t)userInfo->gender;
	cod_user_info.age  = userInfo ->age;
	cod_user_info.height = userInfo->height;
	cod_user_info.weight= userInfo->weight;
	NRF_LOG_INFO("[%s] %d,%d,%d,%d\r\n",__func__,cod_user_info.gender,cod_user_info.age,cod_user_info.height,cod_user_info.weight);

	drv_ble_send_param_data((uint8_t *)&cod_user_info,sizeof(cod_user_info));
}

/**
 * 读取用户信息
 */
static void on_get_user_info()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_PERSONAL_INFO_READ | BLE_UART_CCOMMAND_MASK_APPLICATION}; 
    drv_ble_send_param_data((uint8_t *)command,sizeof(command));	
}
#if (FUNC_COMPILE == 0)

/**
 * 久坐提醒设置
 * @param sit_remind_list
 */
static void on_set_sit_remind(cod_ble_sit_remind_list *sit_remind_list)
{
	
}
#endif
#if (FUNC_COMPILE == 0)

/**
 * 读取久坐提醒
 */
static void on_get_sit_remind()
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 睡眠设置
 * @param sleep_setting
 */
static void on_set_sleep_setting(cod_ble_sleep_setting *sleep_setting)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 读取睡眠设置
 */
static void on_get_sleep_setting()
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 喝水提醒设置
 * @param drink_remind_list
 */
static void on_set_drink_remind(cod_ble_drink_remind_list *drink_remind_list)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 读取喝水提醒
 */
static void on_get_drink_remind()
{
	
}
#endif
/**
 * 消息推送设置
 * @param msg_push_setting
 */
static void on_set_msg_push_setting(cod_ble_msg_push_setting *msg_push_setting)
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	memcpy(&cod_msg_push_setting,msg_push_setting,sizeof(cod_ble_msg_push_setting));
	if (cod_msg_push_setting.phone_call == true)
	{
		NRF_LOG_INFO("phone_call\r\n");
		g_ancs_switch.call = ANCS_MESSAGE_SWITCH_ON;
	}
	else{
		g_ancs_switch.call = ANCS_MESSAGE_SWITCH_OFF;
	}
	
	if (cod_msg_push_setting.phone_sms == true){
		NRF_LOG_INFO("phone_sms\r\n");
		g_ancs_switch.sms = ANCS_MESSAGE_SWITCH_ON;
	}
	else{
		g_ancs_switch.sms = ANCS_MESSAGE_SWITCH_OFF;
	}
	
	if (cod_msg_push_setting.qq == true){
		NRF_LOG_INFO("qq\r\n");
		g_ancs_switch.qq = ANCS_MESSAGE_SWITCH_ON;
	}
	else{
		g_ancs_switch.qq = ANCS_MESSAGE_SWITCH_OFF;
	}
	
	if (cod_msg_push_setting.wechat == true){
		NRF_LOG_INFO("wechat\r\n");
		g_ancs_switch.wechat= ANCS_MESSAGE_SWITCH_ON;
	}
	else{
		g_ancs_switch.wechat= ANCS_MESSAGE_SWITCH_OFF;
	}
	
	if (cod_msg_push_setting.company_wechat == true){
		NRF_LOG_INFO("company_wechat\r\n");
		g_ancs_switch.company_wechat= ANCS_MESSAGE_SWITCH_ON;
	}
	else{
		g_ancs_switch.company_wechat= ANCS_MESSAGE_SWITCH_OFF;
	}
	
    if (cod_msg_push_setting.dingding == true){
		NRF_LOG_INFO("dingding\r\n");
		g_ancs_switch.dingding= ANCS_MESSAGE_SWITCH_ON;
	}
   	else{
		g_ancs_switch.dingding= ANCS_MESSAGE_SWITCH_OFF;
	}

	drv_ble_flash_ancs_switch_write(&g_ancs_switch);
	
	cod_cmd_set_msg_push_setting_resp();
	#if 0 
	if ((g_ancs_switch.dingding != ANCS_MESSAGE_SWITCH_OFF) ||(g_ancs_switch.company_wechat != ANCS_MESSAGE_SWITCH_OFF) ||(g_ancs_switch.wechat != ANCS_MESSAGE_SWITCH_OFF)
		|| (g_ancs_switch.qq != ANCS_MESSAGE_SWITCH_OFF) ||(g_ancs_switch.sms != ANCS_MESSAGE_SWITCH_OFF)|| (g_ancs_switch.call != ANCS_MESSAGE_SWITCH_OFF))
	{
		read_cod_ble_dev_param(&usr_dev_param);
		NRF_LOG_INFO("read_cod_ble_dev_param platform %d\r\n",usr_dev_param.platform);
		if (usr_dev_param.platform == IOS)
		{
			uint32_t error;
			// Discover peer's services.
			extern ble_db_discovery_t m_db_disc[];
			error  = ble_db_discovery_start(&m_db_disc[DISCONVER_ARRAY_ANCS], g_conn_handle_peripheral);
			if (error != NRF_ERROR_BUSY )
			{
				APP_ERROR_CHECK(error);
			}
		}
		
	}
	#endif

}

/**
 * 读取消息推送设置
 */
static void on_get_msg_push_setting()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	uint8_t msg_buf[20];
	cod_ble_buffer * p_buf;
	memset(&cod_msg_push_setting,0,sizeof(cod_ble_msg_push_setting));
	if (g_ancs_switch.call == ANCS_MESSAGE_SWITCH_ON)
	{
		NRF_LOG_INFO("phone_call\r\n");
		cod_msg_push_setting.phone_call = true;
	}

	if (g_ancs_switch.sms == ANCS_MESSAGE_SWITCH_ON){
		NRF_LOG_INFO("phone_sms\r\n");
		cod_msg_push_setting.phone_sms = true;
	}
	
	if (g_ancs_switch.qq == ANCS_MESSAGE_SWITCH_ON){
		NRF_LOG_INFO("qq\r\n");
		cod_msg_push_setting.qq = true;
	}
	
	if (g_ancs_switch.wechat == ANCS_MESSAGE_SWITCH_ON){
		NRF_LOG_INFO("wechat\r\n");
		cod_msg_push_setting.wechat = true;
	}
	
	if (g_ancs_switch.company_wechat == ANCS_MESSAGE_SWITCH_ON){
		NRF_LOG_INFO("company_wechat\r\n");
		cod_msg_push_setting.company_wechat = true;
	}
	
    if (g_ancs_switch.dingding == ANCS_MESSAGE_SWITCH_ON){
		NRF_LOG_INFO("dingding\r\n");
		cod_msg_push_setting.dingding = true;
	}
    p_buf = cod_msg_push_setting2bytes(&cod_msg_push_setting);
	if (p_buf->len > 20)
	{
		p_buf->len = 20;
	}
	memset(msg_buf,0,sizeof(msg_buf));
	memcpy(msg_buf,p_buf->buffer,p_buf->len);
	cod_buffer_release(p_buf);
	NRF_LOG_HEXDUMP_INFO(msg_buf, 20);
	cod_cmd_get_msg_push_setting_resp(msg_buf,sizeof(msg_buf));
	
}
#if (FUNC_COMPILE == 0)
/**
 * 表盘设置
 * @param dial_plate_index
 */
static void on_set_dial_plate(uint8_t dial_plate_index)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 表盘读取
 */
static void on_get_dial_plate()
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 运动自动暂停设置
 * @param auto_pause
 */
static void on_set_auto_pause(bool auto_pause)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 读取运动自动暂停设置
 */
static void on_get_auto_pause()
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 运动公里牌/时间提醒设置
 * @param sport_notify_setting
 */
static void on_set_sport_notify_setting(cod_ble_sport_notify_setting *sport_notify_setting)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 读取运动公里牌/时间提醒设置
 */
static void on_get_sport_notify_setting()
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 运动功能键设置
 * @param sport_btn_func
 */
static void on_set_sport_btn_func(cod_ble_sport_btn_func sport_btn_func)
{
	
}
#endif
#if (FUNC_COMPILE == 0)

/**
 * 读取运动功能键设置
 */
static void on_get_sport_btn_func()
{
	
}
#endif
/**
 * 同步咕咚计步目标
 * @param step_target
 */
static void on_set_step_target(uint32_t step_target)
{
	NRF_LOG_INFO("[%s] %d\r\n",__func__,step_target);
	uint32_t command[2];
	command[0] = BLE_UART_CCOMMAND_APPLICATION_SPORTS_PARAM_WRITE | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	command[1] = step_target;
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));	
}
#if (FUNC_COMPILE == 0)
/**
 * 抬腕亮屏设置
 * @param step_target
 */
static void on_set_raise_to_wake(cod_ble_raise_to_wake_setting* setting)
{
	
}
#endif
#if (FUNC_COMPILE == 0)

/**
 * 全天心率检测开关
 * @param enable
 */
static void on_set_all_day_heart_check(bool enable)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 自动记录运动开关设置
 * @param enable
 * @param distance_threshold 阈值:单位(m)
 */
static void on_set_auto_record_sport(bool enable, uint16_t distance_threshold)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 公制和英制的设置
 * @param enable
 * @param distance_threshold
 */
static void on_set_distance_unit(cod_ble_distance_unit unit)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 独立运动心率检测开关设置.这个开关对应的是app上的省电模式
 * @param enable
 */
static void on_set_heart_check_in_standalone_sport(bool enable)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 设置语言
 * @param language
 */
static void on_set_language(cod_ble_language language)
{
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 获取当前语言
 */
static void on_get_cur_language()
{	
	
}
#endif
#if (FUNC_COMPILE == 0)
/**
 * 获取支持的语言
 */
static void on_get_support_language()
{
	
}
#endif
/**
 * 地理信息设置
 * @param info
 */
static void on_set_location_info(cod_ble_location_info* info)
{
	
	uint32_t command[10] = {0};
	
	command[0] = BLE_UART_CCOMMAND_APPLICATION_LOCATION_INFO | BLE_UART_CCOMMAND_MASK_APPLICATION;
	if(info->city_name_len>10)
	{
		info->city_name_len = 10;
		info->city_name[9] = 0;
	}
	
	memcpy((uint8_t *)(command+1),(uint8_t *)info->city_name,info->city_name_len);
	NRF_LOG_INFO("[%s] %s,%d\r\n",__func__,info->city_name,info->city_name_len);
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));	

}
#if (FUNC_COMPILE == 0)
/**
 * 紧急联系人设置
 * @param emergency_contacts
 */
static void on_set_emergency_contacts(cod_ble_emergency_contacts* emergency_contacts)
{
	
}
#endif

/**
 * 设置安静心率
 * @param heart_rate
 */
static void on_set_rest_heart_rate(uint8_t heart_rate)
{
	NRF_LOG_INFO("[%s] %d\r\n",__func__,heart_rate);
	uint32_t command[2];
	command[0] = BLE_UART_CCOMMAND_APPLICATION_REST_HEARTRATE | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	command[1] = heart_rate;
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));		
}


/**
 * 设置客户端信息
 * @param info
 */
static void on_set_app_info(cod_ble_app_info* info)
{
	usr_dev_param.platform = info->platform;
	if (info->nick_name_len > 20)
	{
		usr_dev_param.name_len = 20;
		info->nick_name[19]=0;
	}
	memcpy((uint8_t *)usr_dev_param.name,(uint8_t *)info->nick_name,usr_dev_param.name_len);

	NRF_LOG_INFO("[%s] name:%s\r\n",__func__,usr_dev_param.name);
	
	cod_cmd_set_app_info_resp();
}

#if (FUNC_COMPILE == 0)
/**
 * 开关硬件日志记录
 * @param log_setting
 */
static void on_set_log_setting(cod_ble_log_setting* log_setting)
{
	
}
#endif
/**
 * 运动控制的回复
 * @param success 控制是否成功。一般不需要管。
 */
static void on_sport_ctrl_resp(bool success)
{
	NRF_LOG_INFO("[%s] %d\r\n",__func__,success);
}
#if (FUNC_COMPILE == 0)

/**
 * 设置极限心率
 * @param enable 开关极限心率
 * @param heart_rate
 */
static void on_set_limit_heart_rate(bool enable, uint8_t heart_rate)
{

}
#endif
/**
 * 同步手机来电状态.
 * 来电指令，?机来电后，发指令让?表震动。
 * 在?机上拒接或忽略或接听，?表停止震动也不再显示来电图标。
 */
static void on_sync_phone_call_status(cod_ble_phone_call_status status)
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	uint32_t command[2];
	command[0] = BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_STATUS | BLE_UART_CCOMMAND_MASK_APPLICATION; 	
	command[1] = (uint32_t)(status);
	
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));	

}

/**
 * 设备控制主端来电的回复
 */
static void on_phone_call_ctrl_resp(cod_ble_phone_call_ctrl ctrl)
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
}


/**
 * 设置设备端来电震动延迟
 * @param delay_second 秒
 */
static void on_set_phone_call_virb_delay(float delay_second)
{
	
	uint32_t command[2];
	command[0] = BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_DELAY | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	if (delay_second > 30){
		delay_second = 30;
	}
	command[1] = (uint32_t)(delay_second +1);
	NRF_LOG_INFO("[%s] %d\r\n",__func__,command[1]);
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));	

}
#if (FUNC_COMPILE == 0)

/**
 * 主动触发SOS的回复
 */
static void on_sos_resp(bool success)
{
	
}
#endif
/**
 * 手机端提醒类型设置
 * @param msg_type
 * @param title UTF-16小端编码,包括'\0'，不会为空，但如果只有'\0'，表示没有标题
 * @param title_len msg的长度，包括'\0'
 * @param msg UTF-16小端编码,包括'\0'
 * @param msg_len msg的长度，包括'\0'
 */
static void on_notify_msg(cod_ble_msg_type msg_type, char* title, uint16_t title_len,
					  char* msg, uint16_t msg_len)
{
	NRF_LOG_INFO("[%s] %d\r\n",__func__,sizeof(_app_srv_ancs_msg));
	_app_srv_ancs_msg  app_srv_ancs_msg;
	char str[4];
	
	memset(&app_srv_ancs_msg,0,sizeof(_app_srv_ancs_msg));
	app_srv_ancs_msg.command = BLE_UART_CCOMMAND_APPLICATION_APP_MESSAGE_ANDROID | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
	if (msg_type == MSG_TYPE_PHONE_CALL)
	{	
		if (g_ancs_switch.call == ANCS_MESSAGE_SWITCH_ON)
		{
			app_srv_ancs_msg.type = APP_MSG_TYPE_CALL;
		}
	}
	else if (msg_type == MSG_TYPE_PHONE_SMS){
		if (g_ancs_switch.sms == ANCS_MESSAGE_SWITCH_ON){
			app_srv_ancs_msg.type = APP_MSG_TYPE_SMS;
		}
	}
	else if (msg_type == MSG_TYPE_QQ){
		if (g_ancs_switch.qq == ANCS_MESSAGE_SWITCH_ON){
			app_srv_ancs_msg.type = APP_MSG_TYPE_QQ;
		 }
	}
	else if (msg_type == MSG_TYPE_WECHAT){
		if (g_ancs_switch.wechat == ANCS_MESSAGE_SWITCH_ON){
			app_srv_ancs_msg.type = APP_MSG_TYPE_WECHAT;
		}
	}
	else if (msg_type == MSG_TYPE_COMPANY_WECHAT){
		if (g_ancs_switch.company_wechat == ANCS_MESSAGE_SWITCH_ON){
			app_srv_ancs_msg.type = APP_MSG_TYPE_COMPANY_WECHAT;
		}
	}
	else if (msg_type == MSG_TYPE_DINGDING){
		 if (g_ancs_switch.dingding == ANCS_MESSAGE_SWITCH_ON){
			app_srv_ancs_msg.type = APP_MSG_TYPE_DINGDING;
		 }
	}

	if (app_srv_ancs_msg.type == 0)
	{
		cod_cmd_notify_msg_resp();
		return;
	}
	
	if(title_len > ATTR_TITLE_SIZE)
	{
		title_len = ATTR_TITLE_SIZE;
		title[ATTR_TITLE_SIZE-1] = 0;
		
	}
    memcpy((uint8_t *)&app_srv_ancs_msg.title,(uint8_t *)title,title_len);

	if(msg_len > ATTR_MESSAGE_SIZE)
	{
		msg_len = ATTR_MESSAGE_SIZE;
		msg[ATTR_MESSAGE_SIZE-1] = 0;
	}
    memcpy((uint8_t *)&app_srv_ancs_msg.message,(uint8_t *)msg,msg_len);

	memset(str,0,sizeof(str));
	
	sprintf(str,"%-4d",msg_len);
	for(uint8_t i = 0;i< 4;i++)
	{
		if (str[i]== 0x20)
		{
			str[i] =0;
		}
	}
	NRF_LOG_INFO("tpye %d\r\n",app_srv_ancs_msg.type);
	memcpy((uint8_t *)&app_srv_ancs_msg.message_size,(uint8_t *)str,4);
	
	app_srv_ancs_msg.crc = am_bootloader_fast_crc32((uint8_t*)(&app_srv_ancs_msg.crc)+sizeof(app_srv_ancs_msg.crc),  
														sizeof(_app_srv_ancs_msg)-sizeof(app_srv_ancs_msg.crc)-sizeof(app_srv_ancs_msg.command));		
	//NRF_LOG_HEXDUMP_DEBUG(&app_srv_ancs_msg.message,32);
	//task_ble_conn_handle(TASK_BLE_CONN_EVT_COD_MESSAGE_SEND, (uint8_t *)&app_srv_ancs_msg, sizeof(_app_srv_ancs_msg));
	drv_ble_send_param_data((uint8_t *)&app_srv_ancs_msg,sizeof(_app_srv_ancs_msg));		
	
}
#if (FUNC_COMPILE == 0)

/**
 * 勿扰模式设置
 * @param no_disturb
 */
static void on_set_no_disturb(cod_ble_no_disturb* no_disturb){
}
#endif
/**
 * 运动投屏
 * @param cast
 */
static void on_sport_cast(cod_ble_sport_cast* cast){

	NRF_LOG_INFO("[%s] %d,%d\r\n",__func__,cast->sport_status,cast->pace);
	//NRF_LOG_HEXDUMP_INFO(cast,sizeof(cod_ble_sport_cast));
	_ble_data_info sport_cast_info;
	if ((cast->sport_type == SPORT_TYPE_OUTDOOR_RUN) ||(cast->sport_type == SPORT_TYPE_OUTDOOR_WALK)||
		(cast->sport_type == SPORT_TYPE_RIDE) ||(cast->sport_type == SPORT_TYPE_CLIMB))
	{
		sport_cast_info.command = BLE_UART_CCOMMAND_APPLICATION_ON_SPORT_CAST | BLE_UART_CCOMMAND_MASK_APPLICATION;
		memcpy((uint8_t *)sport_cast_info.data ,(uint8_t *)cast ,sizeof(cod_ble_sport_cast));
		sport_cast_info.len = sizeof(sport_cast_info.command ) + sizeof(cod_ble_sport_cast);
    	drv_ble_send_param_data((uint8_t *)&sport_cast_info, sport_cast_info.len);
	}
	else
	{
		
		cod_ble_cast_result result;
		result = COD_BLE_CAST_RESULT_NOT_SUPPORT;
		cod_cmd_cast_resp(result);
	}
		
}

/**
 * 训练投屏
 * @param castt
 */
static void on_training_cast(cod_ble_training_cast* cast)
{
	NRF_LOG_INFO("[%s]training_type %d,action_type %d, action_target:%d,action_actual:%d,time:%d\r\n",__func__,cast->training_type,cast->action_type,cast->action_target,cast->action_actual,cast->time);
	  _ble_data_info training_cast_info;
	  cod_user_training_cast user_training_cast;
	  memset(&user_training_cast,0,sizeof(cod_user_training_cast));
	
    user_training_cast.training_status = cast->training_status;
    user_training_cast.training_type = cast->training_type;
    user_training_cast.action_type = cast->action_type;      
    user_training_cast.course_name_len = cast->course_name_len;          
    user_training_cast.action_name_len = cast->action_name_len;   
    user_training_cast.action_target = cast->action_target;     
    user_training_cast.action_actual = cast->action_actual;    
    user_training_cast.calorie = cast->calorie;            
    user_training_cast.time  = cast->time;            
    user_training_cast.heart_rate = cast->avg_heart_rate;

	if (user_training_cast.course_name_len > 20)
	{
		user_training_cast.course_name_len =20;
		cast->course_name[19] = 0;
	}
	if (user_training_cast.action_name_len > 20)
	{
		user_training_cast.action_name_len =20;
		cast->action_name[19] = 0;
	}
	
	memcpy((uint8_t *)user_training_cast.course_name ,(uint8_t *)cast->course_name,user_training_cast.course_name_len);
	memcpy((uint8_t *)user_training_cast.action_name ,(uint8_t *)cast->action_name,user_training_cast.action_name_len);
	
	training_cast_info.command = BLE_UART_CCOMMAND_APPLICATION_ON_TRAINING_CAST | BLE_UART_CCOMMAND_MASK_APPLICATION;
	
	memcpy((uint8_t *)training_cast_info.data ,(uint8_t *)&user_training_cast ,sizeof(cod_user_training_cast));
	training_cast_info.len = sizeof(training_cast_info.command ) + sizeof(cod_user_training_cast);

	drv_ble_send_param_data((uint8_t *)&training_cast_info, training_cast_info.len);

}

/**
 * 同步24小时天气
 * @param weather_list 天气数组。第一为当前小时的天气，后面每个天气的时间递增1小时。
 * @param num 天气个数
 */
static void on_sync_weather(cod_ble_weather* weather_list, uint8_t num)
{
	NRF_LOG_INFO("[%s] %d\r\n",__func__,num);

	_ble_data_info sync_weather_info ;
	
	memset(&sync_weather_info,0,sizeof(_ble_data_info));

	sync_weather_info.command = BLE_UART_CCOMMAND_APPLICATION_WEATHER_INFO_REQUEST | BLE_UART_CCOMMAND_MASK_APPLICATION;

	if (num > 24)
	{
		num =24;
	}
	memcpy((uint8_t *)sync_weather_info.data ,(uint8_t *)weather_list ,sizeof(cod_ble_weather) *num);
	sync_weather_info.len = sizeof(sync_weather_info.command ) + sizeof(cod_ble_weather) *24;
	
	drv_ble_send_param_data((uint8_t *)&sync_weather_info, sync_weather_info.len);

}




/**
 * 获取数据帧数
 * @param type 数据类型
 */
static void on_get_data_frame_num(cod_ble_data_type type)
{
 NRF_LOG_INFO("[%s] %d\r\n",__func__,type);
#if (DATA_DEBUG == 0)

	if (type == DATA_TYPE_STEP)
	{
		uint32_t command[2];
		command[0] = BLE_UART_CCOMMAND_APPLICATION_DATA_FRAME_NUM | BLE_UART_CCOMMAND_MASK_APPLICATION; 	
		command[1] = (uint32_t)(type);
	
		drv_ble_send_param_data((uint8_t *)command,sizeof(command));
	}
	else if (type == DATA_TYPE_SPORT)
	{
		get_data_buf[0] =(uint8_t)type;
		task_extFlash_handle(TASK_EXTFLASH_EVT_COD_CMD_ON_GET_DATA_FRAME_NUM,0,get_data_buf,sizeof(get_data_buf));
	}
	else
	{
		cod_cmd_get_data_frame_num_resp(type,0,false);
	}
#else
	NRF_LOG_INFO("cod_cmd_get_data_frame_num_resp\r\n");
	cod_cmd_get_data_frame_num_resp(type,0,true);
#endif
}

/**
 * 删除数据
 * @param type 数据类型
 */
static void on_delete_data(cod_ble_data_type type)
{
	NRF_LOG_INFO("[%s] %d\r\n",__func__,type);
#if (DATA_DEBUG == 0)
	if (type == DATA_TYPE_STEP)
	{
		uint32_t command[2];
		command[0] = BLE_UART_CCOMMAND_APPLICATION_DELETE_DATA | BLE_UART_CCOMMAND_MASK_APPLICATION; 	
		command[1] = (uint32_t)(type);
	
		drv_ble_send_param_data((uint8_t *)command,sizeof(command));
	}
	else if (type == DATA_TYPE_SPORT )
	{
		delete_sport_data();

	}
	else
	{
		cod_cmd_delete_data_resp(type,false);
	}
#else
	NRF_LOG_INFO("cod_cmd_delete_data_resp\r\n");
	cod_cmd_delete_data_resp(type,true);
#endif
}

/**
 * 获取数据
 * @param type 数据类型
 * @param start_frame 需要传输的起始帧号
 */
static void on_get_data(cod_ble_data_type type, uint16_t start_frame)
{
	NRF_LOG_INFO("[%s] %d,%d\r\n",__func__,type,start_frame);
#if (DATA_DEBUG == 0)
	//uint8_t msg_buf[3];

	if ((type == DATA_TYPE_SPORT)&&(start_frame !=0)) //唤醒等待发送10帧后运动数据，继续发送
	{
		frame_id_flag = 0;
		xSemaphoreGive(SportSemaphore);
	}
	else
	{
		if((type == DATA_TYPE_SPORT)&&(frame_id_flag == 1)) // //10帧运动数据有误，重发
		{
			frame_id_flag = 2;
			xSemaphoreGive(SportSemaphore);
			DELAY_MS(10);
			
		}
		get_data_buf[0] =(uint8_t)type;
		get_data_buf[1] =(uint8_t)start_frame;
		get_data_buf[2] =(uint8_t)start_frame >>8;
		//NRF_LOG_INFO("msg %d,%d,%d\r\n",msg_buf[0],msg_buf[1],msg_buf[2]);
		task_extFlash_handle(TASK_EXTFLASH_EVT_COD_CMD_ON_GET_DATA,0,get_data_buf,sizeof(get_data_buf));
	}
#else
	uint8_t data[1] = {0x0a};
	NRF_LOG_INFO("cod_cmd_get_data_resp\r\n");
	cod_cmd_get_data_resp(type,1,false,data,1);
#endif
}

/**
 * 实时数据传输指令
 * @param type 数据类型
 * @param start 开始或者结束
 */
static void on_real_time_data_trans(bool start, cod_ble_rt_data_type type)
{
	NRF_LOG_INFO("[%s] %d,%d\r\n",__func__,start,type);
#if (DATA_DEBUG == 0)

	uint32_t command[2];
	command[0] = BLE_UART_CCOMMAND_APPLICATION_RT_DATA_TRANS | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	command[1] = (start << 16) |(uint16_t) type;
	drv_ble_send_param_data((uint8_t *)command,sizeof(command));
	if (type == RT_DATA_TYPE_HEART_BP_BO)
	{	
		if (start)
		{
			cod_uart_normal_open = start;
		}
		else
		{
			cod_uart_normal_open =2;
		}
		DELAY_MS(10);
		 if (start)
		{
			ble_conn_params_set_conn_params(BLE_ADV_EVT_FAST);
		}
		else
		{
			ble_conn_params_set_conn_params(BLE_ADV_EVT_SLOW);
		}
	}

#else
	if (type ==RT_DATA_TYPE_TODAY_SUMMARY)
	{
		NRF_LOG_INFO("cod_cmd_rt_today_summary_data_resp\r\n");
		cod_cmd_rt_today_summary_data_resp(100,102,1000);
	}
	else if (type ==RT_DATA_TYPE_HEART_BP_BO)
	{
		NRF_LOG_INFO("cod_cmd_rt_heart_bp_bo_resp\r\n");
		cod_cmd_rt_heart_bp_bo_resp(75,0,0,0);
	}
#endif
}

/**
 * 查询硬件具备的Sensor能力
 */
static void on_get_sensor_capability()
{
	NRF_LOG_INFO("[%s]\r\n",__func__);
	cod_ble_sensor_cap sensor_cap[3];
	sensor_cap[0].type = SENSOR_TYPE_ACC;
	sensor_cap[0].max_freq = 100;

	sensor_cap[1].type = SENSOR_TYPE_GYRO;
	sensor_cap[1].max_freq = 100;
	
	sensor_cap[2].type = SENSOR_TYPE_HEART;
	sensor_cap[2].max_freq = 1;
	NRF_LOG_INFO("cod_cmd_get_sensor_capability_resp\r\n");
	cod_cmd_get_sensor_capability_resp(sensor_cap,3);

}

/**
 * 传感器数据传输指令。
 * @param start 开始或者结束
 * @param sensor_req_array 传感器请求数组
 * @param req_num 数组长度
 */
static void on_sensor_data_trans(bool start, cod_ble_sensor_req* sensor_req_array, uint8_t req_num)
{
	NRF_LOG_INFO("[%s] %d,%d\r\n",__func__,start,req_num);
#if (DATA_DEBUG == 0)
	
	_ble_data_info cod_sensor_req ;
	uint32_t temp = 0;
	temp = (start << 16) | req_num ;
	cod_sensor_req.command = BLE_UART_CCOMMAND_APPLICATION_SENSOR_DATA_TRANS | BLE_UART_CCOMMAND_MASK_APPLICATION;
	memcpy((uint8_t *)cod_sensor_req.data ,(uint8_t *)&temp ,4);
	memcpy((uint8_t *)(cod_sensor_req.data +4) ,(uint8_t *)sensor_req_array ,sizeof(cod_ble_sensor_req)*req_num);
	cod_sensor_req.len = sizeof(cod_sensor_req.command ) + sizeof(cod_ble_sensor_req) *req_num +4;
	
	drv_ble_send_param_data((uint8_t *)&cod_sensor_req, cod_sensor_req.len);
	if (start)
	{
		cod_uart_normal_open = start;		
	}
	else
	{
		cod_uart_normal_open =2;
	}
	DELAY_MS(10);
	if (start)
	{
		ble_conn_params_set_conn_params(BLE_ADV_EVT_FAST);
	}
	else
	{
		ble_conn_params_set_conn_params(BLE_ADV_EVT_SLOW);
	}
#else
	uint8_t data[2] ={0x5a,0xa5};
	NRF_LOG_INFO("on_sensor_data_trans\r\n");
	cod_cmd_sensor_data_trans_resp(data,2);
#endif
	
}


static void on_real_time_log_trans(bool start, cod_ble_rt_log_type type)
{
	NRF_LOG_INFO("[%s] %d,%d\r\n",__func__,start);

	uint32_t command[2];
	command[0] = BLE_UART_CCOMMAND_APPLICATION_REAL_TIME_LOG_TRANS | BLE_UART_CCOMMAND_MASK_APPLICATION; 
	command[1] = start;
	
	if (type == RT_LOG_TYPE_HRAET_RAW)
	{
		drv_ble_send_param_data((uint8_t *)command,sizeof(command));
	}
	else
    {
		return;
    }

	
	if (start)
	{
		cod_uart_normal_open = start;		
	}
	else
	{
		cod_uart_normal_open =2;
	}
	DELAY_MS(10);
	if (start)
	{
		ble_conn_params_set_conn_params(BLE_ADV_EVT_FAST);
	}
	else
	{
		ble_conn_params_set_conn_params(BLE_ADV_EVT_SLOW);
	}

	
}


//OTA指令处理回调
/**
  * OTA传输开始请求
  * @param phone_mtu 手机本次的传输MTU
  * @param file_size 文件大小
  * @param file_type 文件类型
  */
static void on_ota_start(uint16_t phone_mtu, uint32_t file_size, cod_ble_file_type file_type)
{
    NRF_LOG_DEBUG("on_ota_start.phone_mtu=%d,file_size=0x%x,file_type=%d",phone_mtu,file_size,file_type);
	//cod_cmd_ota_start_failed_resp //升级启动失败
	if((g_battery_level < 20) && (FILE_TYPE_OTA == file_type))
	{
		NRF_LOG_DEBUG("COD_BLE_OTA_START_FAILED_TYPE_LOW_BAT. g_battery_level=%d",g_battery_level);
		cod_cmd_ota_start_failed_resp(COD_BLE_OTA_START_FAILED_TYPE_LOW_BAT);
		return;
	}
	//运动中

	cod_ble_ota_file_type = file_type;
	ble_conn_params_set_conn_params(BLE_ADV_EVT_FAST);
    if(FILE_TYPE_OTA == cod_ble_ota_file_type)
    {
    	cod_ble_ota_file_size = file_size;
		cod_ble_ota_currentAddr = EXT_FLASH_OAT_ADDRESS;
		//cod_ble_ota_phone_mtu = phone_mtu;
		//擦除外部FLASH
        task_extFlash_handle(TASK_EXTFLASH_EVT_OTA_ERASE_RAW, cod_ble_ota_currentAddr, NULL, cod_ble_ota_file_size);
		
    	//发送指令给apollo	
        uint32_t command[180] = {BLE_UART_CCOMMAND_APP_BLE_DFU_START | BLE_UART_CCOMMAND_MASK_APP};
        memset((uint8_t *)(command+1),0,sizeof(command)-4);		
        drv_ble_send_param_data((uint8_t *)(&command), sizeof(command));
			
    }
	else if(FILE_TYPE_FONT == cod_ble_ota_file_type)
	{

	}
	else if(FILE_TYPE_GPS == cod_ble_ota_file_type)
	{
		cod_ble_ota_file_size = file_size;
		cod_ble_ota_currentAddr = EXT_FLASH_ASTRO_ADDRESS;
		//擦除外部FLASH
        task_extFlash_handle(TASK_EXTFLASH_EVT_OTA_ERASE_RAW, cod_ble_ota_currentAddr, NULL, cod_ble_ota_file_size);
        //OTA start resp
        cod_cmd_ota_start_resp(cod_usr_ble_tx_mtu,10,200,FILE_TYPE_GPS);
	}
	else if(FILE_TYPE_BP == cod_ble_ota_file_type)
	{
		cod_ble_ota_file_size = file_size;
		cod_ble_ota_currentAddr = EXT_FLASH_DIAL_ADDRESS;
		//擦除外部FLASH
        task_extFlash_handle(TASK_EXTFLASH_EVT_OTA_ERASE_RAW, cod_ble_ota_currentAddr, NULL, cod_ble_ota_file_size);
        //OTA start resp
        //cod_cmd_ota_start_resp(cod_usr_ble_tx_mtu,10,200,FILE_TYPE_BP);
        uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_UPDATA_BP_START | BLE_UART_CCOMMAND_MASK_APPLICATION};
		drv_ble_send_param_data((uint8_t *)(&command), sizeof(command));
	}
}
/**
   * 组CRC校验请求
   * @param is_success sdk判断组的crc校验是否成功
   * @param data_type 数据类型
   */
static void on_ota_group_crc(bool is_success, uint8_t data_type)
{
	NRF_LOG_DEBUG("on_ota_group_crc. is_success=%d,data_type=%d",is_success,data_type);
	cod_cmd_ota_group_crc_resp(is_success,data_type);
}

/**
  * 文件CRC校验请求
  * @param is_success sdk判断文件的crc校验是否成功。SDK判断只是一个预判，
  * 实际要以硬件重新读取文件数据计算得到的CRC校验为准。
  * @param file_crc
  * @param data_type
  */
static void on_ota_file_crc(bool is_success, uint32_t file_crc, uint8_t data_type)
{
	NRF_LOG_DEBUG("on_ota_file_crc. is_success=%d,file_crc=0x%x,data_type=%d,",is_success,file_crc,data_type);
	//read flash and check
	uint32_t cod_crc32 = 0;
	ble_conn_params_set_conn_params(BLE_ADV_EVT_SLOW);
	if(FILE_TYPE_OTA == cod_ble_ota_file_type)
	{
		is_success = cod_check_crc_from_external_flash(file_crc,EXT_FLASH_OAT_ADDRESS,cod_ble_ota_file_size,&cod_crc32);	
	    NRF_LOG_DEBUG("cod_crc32=0x%x,file_crc=0x%x",cod_crc32,file_crc);
		
		if(is_success)
		{
			
			NRF_LOG_DEBUG("on_ota_file_crc success!");
			if(check_ble_is_updata())
			{
				//开始升级蓝牙
				NRF_LOG_DEBUG("start updata ble!");
				m_reset_type = SRV_APOLLO_RESET_TYPE_JUMP_BOOTLOADER;	
				NRF_LOG_DEBUG("SRV_APOLLO_RESET_TYPE_JUMP_BOOTLOADER");
				timer_start(500);
			}
			else if(check_apollo_is_updata())
			{
				//发送指令给apollo进行升级
				NRF_LOG_DEBUG("start updata apollo!");
				uint32_t command[128] = {BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE | BLE_UART_CCOMMAND_MASK_APP};
				memset((uint8_t *)(command+1),0,sizeof(command)-4);		
				drv_ble_send_param_data((uint8_t *)(&command), sizeof(command));			
			}
			else
			{
				is_success = false; // 数据内容错误
			}
		}
		else
		{
			NRF_LOG_DEBUG("on_ota_file_crc error!");		
		}
	}
	else if (FILE_TYPE_GPS == cod_ble_ota_file_type)
	{
		if (is_success)
		{
			uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_UPDATA_AGPS_TIME | BLE_UART_CCOMMAND_MASK_APPLICATION};
			drv_ble_send_param_data((uint8_t *)(&command), sizeof(command));
		}
	}
	else if (FILE_TYPE_BP == cod_ble_ota_file_type)
	{
		if (is_success)
		{
			uint32_t command[1] = {BLE_UART_CCOMMAND_APPLICATION_UPDATA_BP_FINISH | BLE_UART_CCOMMAND_MASK_APPLICATION};
			drv_ble_send_param_data((uint8_t *)(&command), sizeof(command));
		}
	}
	
	cod_cmd_ota_file_crc_resp(is_success,data_type);
}

/**
  * 每帧的文件数据返回
  * @param file_data
  * @param data_len
  */
static void on_ota_data(uint8_t *file_data, uint16_t data_len)
{
    NRF_LOG_DEBUG("on_ota_data. data_len=%d",data_len);
	task_extFlash_handle(TASK_EXTFLASH_EVT_OTA_WRITE_RAW, cod_ble_ota_currentAddr, file_data, data_len);
	cod_ble_ota_currentAddr = cod_ble_ota_currentAddr + data_len;
}




/**
   * 指令发生器的回调。SDK使用者可以根据cod_cmd_type决定当前指令是否是直接发送，还是放到任务队列。
   * 一般而言，CMD_TYPE_SENSOR类型需要直接发送。
   * @param cmd_type 指令类型
   * @param cmds 组装好的指令数组
   * @param cmd_num 组装好的指令数组长度
   */

static void on_cod_cmd_gen(cod_cmd_type cmd_type, cod_ble_buffer* cmds, uint8_t cmd_num)
{
	NRF_LOG_INFO("[%s] type:%d,cmd_num:%d,len:%d\r\n",__func__,cmd_type,cmd_num,cmds->len);

	//NRF_LOG_HEXDUMP_INFO(cmds->buffer, cmds->len);
	
	for(uint8_t i=0;i<cmd_num;i++ )
    {
    	#if 0
    	if (cmds[i].len < 50)
    	{
    		NRF_LOG_HEXDUMP_INFO(cmds[i].buffer,cmds[i].len);
    	}
		else if (cmds[i].len >=50 && cmds[i].len < 100 )
		{
			NRF_LOG_HEXDUMP_INFO(cmds[i].buffer, 50);
			DELAY_MS(10);
			NRF_LOG_HEXDUMP_INFO(cmds[i].buffer +50, cmds[i].len -50);
		}
		else
		{
			NRF_LOG_HEXDUMP_INFO(cmds[i].buffer, 50);
			DELAY_MS(10);
			NRF_LOG_HEXDUMP_INFO(cmds[i].buffer +50,50);
			DELAY_MS(10);
			NRF_LOG_HEXDUMP_INFO(cmds[i].buffer +100,cmds[i].len -100);
		}
		#endif
        app_srv_bas_level_send(cmds[i].buffer,cmds[i].len);
    }
	
}

static void print_log_cb(int level, char *tag, char *content)
{
	#ifdef COD_BLEPRINT_LOG_ENABLE
	switch(level)
	{
		case LOG_VERBOSE:
		case LOG_DEBUG:
			NRF_LOG_DEBUG("[%s]:%s\r\n",tag,content);
			break;
		case LOG_INFO:
			NRF_LOG_INFO("[%s]:%s\r\n",tag,content);
			break;
		case LOG_WARNING:
			NRF_LOG_WARNING("[%s]:%s\r\n",tag,content);
			break;
		case LOG_ERROR:
			NRF_LOG_ERROR("[%s]:%s\r\n",tag,content);
			break;
		default:
			break;
		
	}
	#endif
}

static void* malloc_cb(uint32_t size)
{
	//NRF_LOG_INFO("[%s] size=%d\r\n",__func__,size);
	return pvPortMalloc(size);
}

static void free_cb(void* p)
{
	vPortFree(p);
	//NRF_LOG_INFO("[%s] count=%d\r\n",__func__, cod_get_malloc_count());
}

static void cod_on_sport_detail_data_converted(uint8_t *data, uint16_t data_len)
{

	if (get_data_flag == 0)
	{
		total_size += data_len;
		//NRF_LOG_DEBUG("%s total_size %d\n",__func__,total_size);
	}
	else if (get_data_flag == 1)
	{
		//NRF_LOG_DEBUG("%s send data_len %d \r\n",__func__,data_len);
		cod_sport_data_copy_send(data,data_len);
	}
}

static void cod_on_sport_summary_converted(uint8_t *data, uint16_t data_len)
{
	if (get_data_flag == 0)
	{
		total_size += data_len;
		//NRF_LOG_DEBUG("%s total_size %d\n",__func__,total_size);
	}
}


//codoon sdk 初始化
void cod_sdk_init(void)
{
	
	//基础指令处理回调
	cod_usr_ble_basic_process_cb.on_process_error = on_process_error;
	cod_usr_ble_basic_process_cb.on_connection_test = on_connection_test;
	cod_usr_ble_basic_process_cb.on_get_device_version = on_get_device_version;
	cod_usr_ble_basic_process_cb.on_get_device_battery = on_get_device_battery;
	cod_usr_ble_basic_process_cb.on_get_device_mac =on_get_device_mac;
	cod_usr_ble_basic_process_cb.on_check_agps=on_check_agps;
	cod_usr_ble_basic_process_cb.on_sync_imei_imsi=NULL;
	cod_usr_ble_basic_process_cb.on_set_ios_ancs=on_set_ios_ancs;	
	cod_usr_ble_basic_process_cb.on_set_device_mac= on_set_device_mac;
	cod_usr_ble_basic_process_cb.on_bind_device= on_bind_device;
	cod_usr_ble_basic_process_cb.on_unbind_device= on_unbind_device;		
	cod_usr_ble_basic_process_cb.on_check_bind=on_check_bind;	
	cod_usr_ble_basic_process_cb.on_sync_imei_imsi_resp=NULL;
	cod_usr_ble_basic_process_cb.on_send_bind_status=on_send_bind_status;	
	cod_usr_ble_basic_process_cb.on_set_time= on_set_time;	
	cod_usr_ble_basic_process_cb.on_get_time=on_get_time;
	cod_usr_ble_basic_process_cb.on_set_alarm=NULL;
	cod_usr_ble_basic_process_cb.on_get_alarm =NULL;
	cod_usr_ble_basic_process_cb.on_set_user_info= on_set_user_info;	
	cod_usr_ble_basic_process_cb.on_get_user_info=on_get_user_info;
	cod_usr_ble_basic_process_cb.on_set_sit_remind= NULL;
	cod_usr_ble_basic_process_cb.on_get_sit_remind=NULL;
	cod_usr_ble_basic_process_cb.on_set_sleep_setting=NULL;
	cod_usr_ble_basic_process_cb.on_get_sleep_setting=NULL;
	cod_usr_ble_basic_process_cb.on_set_drink_remind=NULL;
	cod_usr_ble_basic_process_cb.on_get_drink_remind=NULL;
	cod_usr_ble_basic_process_cb.on_set_msg_push_setting=on_set_msg_push_setting;
	cod_usr_ble_basic_process_cb.on_get_msg_push_setting=on_get_msg_push_setting;
	cod_usr_ble_basic_process_cb.on_set_dial_plate= NULL;
	cod_usr_ble_basic_process_cb.on_get_dial_plate= NULL;	
	cod_usr_ble_basic_process_cb.on_set_auto_pause= NULL;	
	cod_usr_ble_basic_process_cb.on_get_auto_pause=NULL;
	cod_usr_ble_basic_process_cb.on_set_sport_notify_setting= NULL;
	cod_usr_ble_basic_process_cb.on_get_sport_notify_setting= NULL;
	cod_usr_ble_basic_process_cb.on_set_sport_btn_func= NULL;
	cod_usr_ble_basic_process_cb.on_get_sport_btn_func= NULL;	
	cod_usr_ble_basic_process_cb.on_set_step_target= on_set_step_target;
	cod_usr_ble_basic_process_cb.on_set_raise_to_wake = NULL;	
	cod_usr_ble_basic_process_cb.on_set_all_day_heart_check = NULL;
	cod_usr_ble_basic_process_cb.on_set_auto_record_sport = NULL;
	cod_usr_ble_basic_process_cb.on_set_distance_unit = NULL;
	cod_usr_ble_basic_process_cb.on_set_heart_check_in_standalone_sport= NULL;
	cod_usr_ble_basic_process_cb.on_set_language= NULL;
	cod_usr_ble_basic_process_cb.on_get_cur_language= NULL;
	cod_usr_ble_basic_process_cb.on_get_support_language= NULL;
	cod_usr_ble_basic_process_cb.on_set_location_info= on_set_location_info;
	cod_usr_ble_basic_process_cb.on_set_emergency_contacts= NULL;
	cod_usr_ble_basic_process_cb.on_set_rest_heart_rate= on_set_rest_heart_rate;
	cod_usr_ble_basic_process_cb.on_set_app_info= on_set_app_info;
	cod_usr_ble_basic_process_cb.on_set_log_setting= NULL;
	cod_usr_ble_basic_process_cb.on_sport_ctrl_resp= on_sport_ctrl_resp;
	cod_usr_ble_basic_process_cb.on_set_limit_heart_rate= NULL;
	cod_usr_ble_basic_process_cb.on_sync_phone_call_status= on_sync_phone_call_status;
	cod_usr_ble_basic_process_cb.on_phone_call_ctrl_resp= on_phone_call_ctrl_resp;
	cod_usr_ble_basic_process_cb.on_set_phone_call_virb_delay= on_set_phone_call_virb_delay;
	cod_usr_ble_basic_process_cb.on_sos_resp= NULL;
	cod_usr_ble_basic_process_cb.on_notify_msg= on_notify_msg;	
	cod_usr_ble_basic_process_cb.on_set_no_disturb= NULL;	
	cod_usr_ble_basic_process_cb.on_sport_cast= on_sport_cast;	
	cod_usr_ble_basic_process_cb.on_training_cast= on_training_cast;
	cod_usr_ble_basic_process_cb.on_sync_weather= on_sync_weather;
	
	//数据传输指令处理回调
	cod_usr_ble_data_sync_process_cb.on_get_data_frame_num = on_get_data_frame_num;
	cod_usr_ble_data_sync_process_cb.on_delete_data = on_delete_data;
	cod_usr_ble_data_sync_process_cb.on_get_data = on_get_data;
	cod_usr_ble_data_sync_process_cb.on_real_time_data_trans = on_real_time_data_trans;
	cod_usr_ble_data_sync_process_cb.on_get_sensor_capability = on_get_sensor_capability;
	cod_usr_ble_data_sync_process_cb.on_sensor_data_trans = on_sensor_data_trans;
	cod_usr_ble_data_sync_process_cb.on_real_time_log_trans= on_real_time_log_trans;
		
	// OTA指令处理回调
	cod_usr_ble_ota_process_cb.on_ota_start= on_ota_start;
	cod_usr_ble_ota_process_cb.on_ota_file_crc= on_ota_file_crc;
	cod_usr_ble_ota_process_cb.on_ota_group_crc= on_ota_group_crc;
	cod_usr_ble_ota_process_cb.on_ota_data = on_ota_data;
	
	//指令组装回调
	cod_usr_ble_cmd_gen_cb.on_cod_cmd_gen = on_cod_cmd_gen;

	cod_usr_ble_log.print_log = print_log_cb;

	cod_usr_ble_memory_delegate.malloc = malloc_cb;
	cod_usr_ble_memory_delegate.free = free_cb;

	
 

	
	cod_ble_init(cod_usr_ble_tx_mtu,
                  &cod_usr_ble_basic_process_cb,
                  &cod_usr_ble_data_sync_process_cb,
                  &cod_usr_ble_ota_process_cb,
                  &cod_usr_ble_cmd_gen_cb,
                  &cod_usr_ble_log,
                  &cod_usr_ble_memory_delegate);

	
	sport_callback.on_sport_detail_data_converted = cod_on_sport_detail_data_converted;
	sport_callback.on_sport_summary_converted = cod_on_sport_summary_converted;
	
	//cod_sport_data_conv_init_by_outer_buffer(&sport_converter,sport_buffer,sizeof(sport_buffer),&sport_callback);

	//cod_swim_data_conv_init_by_outer_buffer(&swim_converter,sport_buffer,sizeof(sport_buffer),&sport_callback);

	sport_summary_size = cod_get_sport_summary_data_size();

	swim_summary_size = cod_get_swim_summary_data_size();

}




void app_srv_bas_level_send(uint8_t *p_data, uint16_t length)
{
	//NRF_LOG_DEBUG("app_srv_bas_level_send");
	uint16_t mtu = cod_usr_ble_tx_mtu;	
	uint32_t bytes_sent = 0;
	uint16_t packet_length = 0;	
	uint32_t err_code;

	while(bytes_sent < length)
	{
		packet_length = (length - bytes_sent) > mtu ? mtu : length - bytes_sent;
		err_code = ble_bas_battery_level_string_send(&m_bas, p_data + bytes_sent, &packet_length);	
		
		/**
		 * @retval ::NRF_ERROR_BUSY For @ref BLE_GATT_HVX_INDICATION Procedure already in progress. Wait for a @ref BLE_GATTS_EVT_HVC event and retry.		
		 * @retval ::NRF_ERROR_RESOURCES Too many notifications queued.
		 *                               Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry.		
		*/
		if(err_code == NRF_ERROR_BUSY) 
		{
			//Wait for a @ref BLE_GATTS_EVT_HVC event and retry
			//NRF_LOG_WARNING("app_srv_bas_level_send NRF_ERROR_BUSY");
			DELAY_MS(1);
		}
		else if(err_code == NRF_ERROR_RESOURCES) 
		{
			//Wait for a @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event and retry
			//NRF_LOG_WARNING("app_srv_bas_level_send NRF_ERROR_RESOURCES");
			DELAY_MS(1);
		}
		else if(err_code == NRF_SUCCESS) 
		{
			bytes_sent += packet_length;
		}
		else //if ((err_code == NRF_ERROR_INVALID_STATE)||(err_code == NRF_ERROR_INVALID_PARAM))
		{
			/**<Connection handle invalid or Char notification is not enabled>*/
			break;
		}		
		
	}		
	NRF_LOG_DEBUG("app_srv_bas_level_send finish %d",err_code);
}



static void bas_evt_handler (ble_bas_t * p_bas,ble_bas_evt_t *p_evt)
{
	switch(p_evt->evt_type)
	{
		case BLE_BAS_EVT_NOTIFICATION_ENABLED:
			task_ble_conn_handle(TASK_BLE_CONN_EVT_CONNECT, NULL, 0);
			break;
		case BLE_BAS_EVT_NOTIFICATION_DISABLED:
			task_ble_conn_handle(TASK_BLE_CONN_EVT_DISCONNECT, NULL, 0);
			break;
		case BLE_BAS_EVT_LEVEL_DATA:
			//NRF_LOG_INFO("BLE_BAS_EVT_LEVEL_DATA --> %d",p_evt->params.data.length);
			//NRF_LOG_HEXDUMP_INFO(p_evt->params.data.p_data, p_evt->params.data.length);
			//task_ble_conn_handle(TASK_BLE_CONN_EVT_COD_CMD_PROCESS,(uint8_t *)p_evt->params.data.p_data,p_evt->params.data.length);
			cod_cmd_process((uint8_t *)p_evt->params.data.p_data,p_evt->params.data.length);
			//cod_cmd_tx_mtu_test_resp(155);
			break;
		default:
			break;
	}
}


uint32_t app_srv_cod_init(void)
{
    ble_bas_init_t bas_init;
	
    uint32_t err_code;
	memset(&bas_init, 0, sizeof(bas_init));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.write_perm);
	
    bas_init.evt_handler = bas_evt_handler;
	bas_init.support_notification = true;
	bas_init.p_report_ref = NULL;
	bas_init.initial_batt_level = 100;
    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

	timer_create();

    return NRF_SUCCESS;
}

//大小端模式转换
uint32_t reversebytes_uint32t(uint32_t value){
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 | 
        (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24; 
} 

//检查蓝牙数据正确性
//start head(4byte) + extro info len(1byte) + extra info data(nbyte) + apollo len(4byte) + apollo(nbyte) + apollo crc(4byte)
//ble ID(2byte) + ble len(4byte) + ble data(nbyte) + ble crc(4byte)
static bool check_ble_is_updata(void)
{
	NRF_LOG_DEBUG("[%s]",__func__);

	cod_ota_file_info_t cod_ota_file;
	memset((uint8_t *)&cod_ota_file,0,sizeof(cod_ota_file_info_t));	
	
	uint32_t ble_bootsetting_address = 0;
	uint32_t ble_application_address = 0;
	uint32_t apollo_length_address = 0; //apollo代码长度存储位置
	_ble_boot_setting buffer = {0};

	uint32_t bank_addr = 0;
	uint32_t bank_size = 0;
	uint32_t bank_crc = 0;	

	dev_extFlash_enable();
	//获取额外数据长度
	dev_extFlash_read(EXT_FLASH_OAT_ADDRESS + sizeof(cod_ota_file.command), (uint8_t *)&cod_ota_file.extra_info_len, sizeof(cod_ota_file.extra_info_len));
	NRF_LOG_DEBUG("cod_ota_file.extra_info_len=0x%x",cod_ota_file.extra_info_len);
	
	//获取apollo数据长度 FLASH里面的数据是大端模式
	apollo_length_address = EXT_FLASH_OAT_ADDRESS + sizeof(cod_ota_file.command) + sizeof(cod_ota_file.extra_info_len) + cod_ota_file.extra_info_len; 	
	NRF_LOG_DEBUG("apollo_length_address=0x%x",apollo_length_address);
	dev_extFlash_read(apollo_length_address, (uint8_t *)&cod_ota_file.apollo.data_len, sizeof(cod_ota_file.apollo.data_len));

	cod_ota_file.apollo.data_len = reversebytes_uint32t(cod_ota_file.apollo.data_len);//转成小端模式
	NRF_LOG_DEBUG("cod_ota_file.apollo.data_len0x%x",cod_ota_file.apollo.data_len);
	
	//计算ble的bootsetting地址
	ble_bootsetting_address = apollo_length_address + sizeof(cod_ota_file.apollo.data_len) + cod_ota_file.apollo.data_len + sizeof(cod_ota_file.apollo.crc) + sizeof(cod_ota_file.ble_id) + sizeof(cod_ota_file.ble.data_len);
	ble_application_address = ble_bootsetting_address + APPLICATION_ADDRESS_OFFSET;

	NRF_LOG_DEBUG("ble_bootsetting_address=0x%x,ble_application_address=0x%x",ble_bootsetting_address,ble_application_address);
	
	if(ble_application_address > EXT_FLASH_END_ADDRESS) //非法地址
    {
    	NRF_LOG_INFO("ble_application_address is invalid");
 		return false;
	}
	dev_extFlash_read(ble_bootsetting_address, (uint8_t *)&buffer, sizeof(_ble_boot_setting));
	dev_extFlash_disable();	

	NRF_LOG_INFO("boot_setting_crc --> 0x%08x",buffer.boot_setting_crc);
	
	uint32_t boot_setting_crc_dest = am_bootloader_fast_crc32((uint8_t*)(&buffer.boot_setting_crc)+sizeof(buffer.boot_setting_crc),sizeof(_ble_boot_setting) - sizeof(buffer.boot_setting_crc));
	NRF_LOG_INFO("boot_setting_crc_dest --> 0x%08x",boot_setting_crc_dest);	

	if(buffer.boot_setting_crc == boot_setting_crc_dest)
	{
		NRF_LOG_INFO("bootsetting flash crc check is correct");
		
	}
	else
	{
		NRF_LOG_INFO("bootsetting flash crc check is wrong");
		return false;
	}

	if (buffer.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW) //&& (buffer.bank_application.version >  ))
	{
			NRF_LOG_INFO("application image is new");
			bank_addr = ble_application_address;//bootsetting里面的地址由于工具问题不能更改，只能软件处理
			bank_size = buffer.bank_application.image_size;
			bank_crc = buffer.bank_application.image_crc;
			if(am_bootloader_check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
			{
				NRF_LOG_INFO("application image crc is correct");
				NRF_LOG_INFO("g_ble_boot_setting.bank_application.image_crc --> 0x%08X",buffer.bank_application.image_crc);
				return true;
			}
			else
			{
				NRF_LOG_INFO("application image crc is wrong");
			}		
	}

	return false;	
}

//检查apollo数据正确性
//start head(4byte) + extro info len(1byte) + extra info data(nbyte) + apollo len(4byte) + apollo(nbyte) + apollo crc(4byte)
//ble ID(2byte) + ble len(4byte) + ble data(nbyte) + ble crc(4byte)

static bool check_apollo_is_updata(void)
{
	NRF_LOG_DEBUG("[%s]",__func__);
	cod_ota_file_info_t cod_ota_file;
	memset((uint8_t *)&cod_ota_file,0,sizeof(cod_ota_file_info_t));	

	uint32_t apollo_bootsetting_address = 0;
	uint32_t apollo_application_address = 0;
	_apollo_boot_setting buffer = {0};

	uint32_t bank_addr = 0;
	uint32_t bank_size = 0;
	uint32_t bank_crc = 0;	

	dev_extFlash_enable();
	dev_extFlash_read(EXT_FLASH_OAT_ADDRESS + sizeof(cod_ota_file.command), (uint8_t *)&cod_ota_file.extra_info_len, sizeof(cod_ota_file.extra_info_len));
	apollo_bootsetting_address = EXT_FLASH_OAT_ADDRESS + sizeof(cod_ota_file.command) + sizeof(cod_ota_file.extra_info_len) + cod_ota_file.extra_info_len + sizeof(cod_ota_file.apollo.data_len);
	apollo_application_address = apollo_bootsetting_address + APPLICATION_ADDRESS_OFFSET;

	if(apollo_bootsetting_address > EXT_FLASH_END_ADDRESS) //非法地址
    {
    	NRF_LOG_INFO("apollo_application_address is invalid");
 		return false;
	}
	dev_extFlash_read(apollo_bootsetting_address, (uint8_t *)&buffer, sizeof(_apollo_boot_setting));
	dev_extFlash_disable();	

	NRF_LOG_INFO("boot_setting_crc --> 0x%08x",buffer.boot_setting_crc);
	
	uint32_t boot_setting_crc_dest = am_bootloader_fast_crc32((uint8_t*)(&buffer.boot_setting_crc)+sizeof(buffer.boot_setting_crc),sizeof(_apollo_boot_setting) - sizeof(buffer.boot_setting_crc));
	NRF_LOG_INFO("boot_setting_crc_dest --> 0x%08x",boot_setting_crc_dest);	

	if(buffer.boot_setting_crc == boot_setting_crc_dest)
	{
		NRF_LOG_INFO("bootsetting flash crc check is correct");
		
	}
	else
	{
		NRF_LOG_INFO("bootsetting flash crc check is wrong");
		return false;
	}

	if (buffer.bank_application.command == BOOT_SETTING_BANK_COMMAND_NEW) //&& (buffer.bank_application.version >  ))
	{
		NRF_LOG_INFO("application image is new");
		bank_addr = apollo_application_address;//bootsetting里面的地址由于工具问题不能更改，只能软件处理
		bank_size = buffer.bank_application.image_size;
		bank_crc = buffer.bank_application.image_crc;
		if(am_bootloader_check_crc_from_external_flash(bank_crc,bank_addr,bank_size))
		{
			NRF_LOG_INFO("application image crc is correct");
			NRF_LOG_INFO("g_ble_boot_setting.bank_application.image_crc --> 0x%08X",buffer.bank_application.image_crc);
			return true;
		}
		else
		{
			NRF_LOG_INFO("application image crc is wrong");
		}		
	}

	return false;	
}


