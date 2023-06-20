#ifdef BOSH_KEIL_API 
#include "drv_bosh_keil_api.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#include "am_util.h"


 
#include <string.h>





#include "sensorcontrol.h"




#include "bsx_activity_bit_identifier.h"
#include "bsx_android.h"
#include "bsx_constant.h"
#include "bsx_datatypes.h"
#include "bsx_library.h"
#include "bsx_module_identifier.h"
#include "bsx_physical_sensor_identifier.h"
#include "bsx_property_set_identifier.h"
#include "bsx_return_value_identifier.h"
#include "bsx_user_def.h"
#include "bsx_vector_index_identifier.h"
#include "bsx_virtual_sensor_identifier.h"



#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_BOSH_KEIL_API]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif


#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


#define AUX_I2C_ADDR  0x69  

#define AUX_I2C_SDA           43
#define AUX_I2C_SCL           42
#define AUX_I2C_INT1          4
#define AUX_I2C_INT2          2
#define AUX_I2C_MODULE        3




void drv_bosh_keil_api_init(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_bosh_keil_api_init\n");
	uint32_t error;
	error = init_sensorlist();
	ERR_CHECK(error);
	
	uint8_t data[1];
	
	error = bmi160_read_reg(BMI160_USER_CHIP_ID__REG,data, sizeof(data));	
	ERR_CHECK(error);
	DEBUG_PRINTF(MOUDLE_NAME"Chip ID --> %02X\n",data[0]);
	
}




void drv_bosh_keil_api_uninit(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_bosh_keil_api_uninit\n");

	
}


//BSX_CREATE_FIFO_GLOBAL(in, BSX_BUFFER_TYPE_S32, 3, 3)
//BSX_CREATE_FIFO_GLOBAL(out, BSX_BUFFER_TYPE_S32, 3, 3)
//static bsx_u8_t m_config[] = {28,0,0,4,127,16,0,0,43,0,111,3,0,0,24,0,0,0,13,162,16,68,128,5,128,0,162,28,0,69,64,68,0,154,13,25,8,63,153,20,0,154,13,25,8,63,153,21,0,0,13,0,8,0,0,24,0,1,13,0,8,0,0,25,0,205,21,204,8,62,204,1,0,226,21,224,8,57,177,2,0,1,21,0,8,0,0,3,0,0,21,224,8,64,0,8,0,1,1,239,8,255,0,1,0,0,1,0,8,0,0,2,0,1,1,0,8,0,0,5,0,0,1,0,8,0,0,6,0,1,3,239,8,255,0,1,0,0,3,0,8,0,0,2,0,1,3,0,8,0,0,5,0,0,3,0,8,0,0,6,0,1,2,239,8,255,0,1,0,0,2,0,8,0,0,2,0,1,2,0,8,0,0,5,0,0,2,0,8,0,0,6,0,1,6,0,8,0,0,2,0,1,6,239,8,255,0,1,0,1,9,0,8,0,0,2,0,1,9,239,8,255,0,1,0,1,8,0,8,0,0,2,0,1,8,239,8,255,0,1,0,1,7,0,8,0,0,2,0,1,7,239,8,255,0,1,0,2,40,0,8,0,0,1,0,2,41,0,8,0,0,1,0,1,42,0,8,0,0,1,0,0,254,0,24,8,1,1,1,0,0,0,0,32,1,1,8,0,0,0,0,32,32,8,0,255,254,255,24,127,255,2,255,255,255,255,255,127,255,127,255,255,255,255,255,127,255,127,0,0,254,0,192,1,1,3,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,32,0,0,0,0,32,32,32,32,0,0,0,0,32,32,32,32,0,0,0,0,32,0,32,32,0,0,0,0,32,0,0,32,0,0,0,0,32,32,32,32,0,0,0,0,32,32,32,32,0,0,0,0,0,32,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,0,32,32,32,32,0,0,0,0,32,32,32,32,0,0,0,0,0,0,32,0,18,254,32,192,60,172,4,172,205,32,204,18,61,204,60,204,88,204,139,205,58,160,61,160,18,139,32,88,60,172,58,172,88,32,139,18,58,160,60,160,18,139,32,88,60,172,58,204,205,204,204,205,61,204,61,204,88,204,139,205,58,160,61,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,232,10,28,28,10,65,232,65,172,18,32,32,18,60,172,60,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,14,0,8,0,0,6,0,1,23,0,8,0,0,8,0,1,24,0,8,0,0,8,0,1,40,0,8,0,0,1,0,1,41,0,8,0,0,1,0,0,3,128,40,63,0,3,0,0,0,0,0,0,0,0,0,0,0,128,0,63,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,63,0,0,0,1,3,0,8,0,0,7,0,217,0,0,1,0,155,0};
//static bsx_u8_t work_buffer[512];
void drv_bosh_keil_api_test(void)
{
	int error;
	S32 x,y,z;
	error = dev_acc_read_xyzdata( &x, &y, &z);	
	DEBUG_PRINTF("x --> " LOG_FLOAT_MARKER "\r\n", LOG_FLOAT(x));
	DEBUG_PRINTF("y --> " LOG_FLOAT_MARKER "\r\n", LOG_FLOAT(y));
	DEBUG_PRINTF("z --> " LOG_FLOAT_MARKER "\r\n", LOG_FLOAT(z));
	
	bsx_version_t bsx_version_p;
	error = bsx_get_version(&bsx_version_p);
	ERR_CHECK(error);

	DEBUG_PRINTF(MOUDLE_NAME"bugfix_major --> 0x%02X\n",bsx_version_p.bugfix_major);
	DEBUG_PRINTF(MOUDLE_NAME"bugfix_minor --> 0x%02X\n",bsx_version_p.bugfix_minor);
	DEBUG_PRINTF(MOUDLE_NAME"version_major --> 0x%02X\n",bsx_version_p.version_major);
	DEBUG_PRINTF(MOUDLE_NAME"version_minor --> 0x%02X\n",bsx_version_p.version_minor);

	
//	static bsx_instance_t m_bsx_p;
//	static bsx_u8_t m_threads = BSX_THREAD_MAIN;
//	static bsx_u8_t config_set_id = 1;
//	static bsx_u32_t m_n_data_out;
//	error = bsx_init(&m_bsx_p);	
//	ERR_CHECK(error);
	
//	bsx_u32_t n_serialized_configuration;
//	error = bsx_get_configuration(&m_bsx_p, config_set_id,
//																 m_config, sizeof(m_config),
//																 work_buffer, sizeof(work_buffer),
//																 &n_serialized_configuration);	
//	ERR_CHECK(error);
//		
//	error = bsx_reset(&m_bsx_p);
//	ERR_CHECK(error);
																	
																 
//	error = bsx_set_configuration(&m_bsx_p,m_config, sizeof(m_config),work_buffer, sizeof(work_buffer));	
//	ERR_CHECK(error);
	
	
//	error = bsx_do_steps(&m_bsx_p, m_threads,&bsx_in_g, 1, &bsx_out_g, &m_n_data_out);
//	ERR_CHECK(error);
	
	
	
}










#endif //BOSH_KEIL_API





