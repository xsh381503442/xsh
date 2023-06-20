#ifdef BOSH_OFFICIAL_SITE_API
#include "drv_bosh_official_api.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#include "am_util.h"


 
#include <string.h>








#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_BOSH_OFFICIAL_API]:"

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




_bmi160_data bmi160_data;



#define AUX_I2C_ADDR  0x68  

#define AUX_I2C_SDA           43
#define AUX_I2C_SCL           42
#define AUX_I2C_INT1          4
#define AUX_I2C_INT2          2
#define AUX_I2C_MODULE        3


static uint8_t m_buffer[1024] = {0};


struct bmi160_fifo_frame m_fifo = 
{
	/*! Data buffer of user defined length is to be mapped here */
	.data = m_buffer,
	/*! While calling the API  "bmi160_get_fifo_data" , length stores
	 *  number of bytes in FIFO to be read (specified by user as input)
	 *  and after execution of the API ,number of FIFO data bytes
	 *  available is provided as an output to user
	 */
	.length = 0,
	/*! FIFO time enable */
	.fifo_time_enable = 0,
	/*! Enabling of the FIFO header to stream in header mode */
	.fifo_header_enable = 0,
	/*! Streaming of the Accelerometer, Gyroscope
	sensor data or both in FIFO */
	.fifo_data_enable = 0,
	/*! Will be equal to length when no more frames are there to parse */
	.accel_byte_start_idx = 0,
	/*! Will be equal to length when no more frames are there to parse */
	.gyro_byte_start_idx = 0,
	/*! Will be equal to length when no more frames are there to parse */
	.aux_byte_start_idx = 0,
	/*! Value of FIFO sensor time time */
	.sensor_time = 0,
	/*! Value of Skipped frame counts */
	.skipped_frame_count = 0,

};
static void int1_handler(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"int1_handler\n");

}
static void int2_handler(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"int2_handler\n");

}
static void bus_enable(void)
{
// <400000=> 400KHZ
// <375000=> 375KHZ
// <250000=> 250KHZ
// <125000=> 125KHZ
// <100000=> 100KHZ
// <50000=> 50KHZ
// <10000=> 10KHZ 	
	const am_hal_iom_config_t IOM3_Config =
	{
		.ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
		.ui32ClockFrequency = 100000,
		.bSPHA = 0,
		.bSPOL = 0,
		.ui8WriteThreshold = 4,
		.ui8ReadThreshold = 60,
	};
	
	am_hal_iom_pwrctrl_enable(AUX_I2C_MODULE);
	am_hal_iom_config(2,&IOM3_Config);
	am_hal_gpio_pin_config(AUX_I2C_SCL, (AM_HAL_PIN_42_M3SCL | AM_HAL_GPIO_PULL1_5K)) 
	am_hal_gpio_pin_config(AUX_I2C_SDA, (AM_HAL_PIN_43_M3SDA | AM_HAL_GPIO_PULL1_5K));
	am_hal_iom_enable(AUX_I2C_MODULE);		
}
static void int_enable(void)
{
	am_hal_gpio_int_register(AUX_I2C_INT1, int1_handler);
	com_apollo2_gpio_int_init(AUX_I2C_INT1, 0);	
	am_hal_gpio_int_register(AUX_I2C_INT2, int2_handler);
	com_apollo2_gpio_int_init(AUX_I2C_INT2, 0);	
}
static void bus_disable(void)
{
	am_hal_iom_disable(AUX_I2C_MODULE);
	am_hal_gpio_pin_config(AUX_I2C_SCL,AM_HAL_PIN_DISABLE);
	am_hal_gpio_pin_config(AUX_I2C_SDA,AM_HAL_PIN_DISABLE);	
	am_hal_iom_pwrctrl_disable(AUX_I2C_MODULE);
}
static void int_disable(void)
{
	com_apollo2_gpio_int_uninit(AUX_I2C_INT1);	
	com_apollo2_gpio_int_uninit(AUX_I2C_INT2);	
}
static int8_t bmi160_read_fptr(uint8_t dev_addr, uint8_t reg_addr,uint8_t *data, uint16_t len)
{	
//	am_hal_iom_buffer(256) psCommand;
//	bus_enable();
//	am_hal_iom_i2c_read(AUX_I2C_MODULE,dev_addr,psCommand.words,len,AM_HAL_IOM_OFFSET(reg_addr));
//	bus_disable();	
//	memcpy(data,psCommand.bytes,len);
	
	bus_enable();
	am_hal_iom_i2c_read(AUX_I2C_MODULE,dev_addr,(uint32_t *)data,len,AM_HAL_IOM_OFFSET(reg_addr));
	bus_disable();	
	return 0;
}
static int8_t bmi160_write_fptr(uint8_t dev_addr, uint8_t reg_addr,uint8_t *data, uint16_t len)
{
//	am_hal_iom_buffer(256) psCommand;
//	memcpy(psCommand.bytes,data,len);
//	bus_enable();
//	am_hal_iom_i2c_write(AUX_I2C_MODULE,dev_addr,psCommand.words,len,AM_HAL_IOM_OFFSET(reg_addr));	
//	bus_disable();		
	
	bus_enable();
	am_hal_iom_i2c_write(AUX_I2C_MODULE,dev_addr,(uint32_t *)data,len,AM_HAL_IOM_OFFSET(reg_addr));	
	bus_disable();	
	return 0;
}
static void bmi160_delay_fptr(uint32_t period)
{
	DELAY_MS(period);
}


static struct bmi160_dev m_bmi160_dev = 
{
	/*! Chip Id */
	.chip_id = BMI160_CHIP_ID,
	/*! Device Id */
	.id = AUX_I2C_ADDR,
	/*! 0 - I2C , 1 - SPI Interface */
	.interface = 0,
	/*! Hold active interrupts status for any and sig motion
	 *  0 - Any-motion enable, 1 - Sig-motion enable,
	 *  -1 neither any-motion nor sig-motion selected */
	.any_sig_sel = BMI160_BOTH_ANY_SIG_MOTION_DISABLED,
	/*! Structure to configure Accel sensor */
	.accel_cfg = 
	{
		/*! power mode */
		.power = BMI160_ACCEL_LOWPOWER_MODE,
		/*! output data rate */
		.odr = BMI160_ACCEL_ODR_0_78HZ,
		/*! range */
		.range = BMI160_ACCEL_RANGE_2G,
		/*! bandwidth */
		.bw = BMI160_ACCEL_BW_OSR4_AVG1,	
	},
	/*! Structure to hold previous/old accel config parameters.
	 * This is used at driver level to prevent overwriting of same
	 * data, hence user does not change it in the code */
	.prev_accel_cfg = 
	{
		/*! power mode */
		.power = BMI160_ACCEL_LOWPOWER_MODE,
		/*! output data rate */
		.odr = BMI160_ACCEL_ODR_0_78HZ,
		/*! range */
		.range = BMI160_ACCEL_RANGE_2G,
		/*! bandwidth */
		.bw = BMI160_ACCEL_BW_OSR4_AVG1,	
	},
	/*! Structure to configure Gyro sensor */
	.gyro_cfg = 
	{
		/*! power mode */
		.power = BMI160_GYRO_SUSPEND_MODE,
		/*! output data rate */
		.odr = BMI160_GYRO_ODR_RESERVED,
		/*! range */
		.range = BMI160_GYRO_RANGE_125_DPS,
		/*! bandwidth */
		.bw = BMI160_GYRO_BW_OSR4_MODE,	
	},	
	/*! Structure to hold previous/old gyro config parameters.
	 * This is used at driver level to prevent overwriting of same
	 * data, hence user does not change it in the code */
	.prev_gyro_cfg = 
	{
		/*! power mode */
		.power = BMI160_GYRO_SUSPEND_MODE,
		/*! output data rate */
		.odr = BMI160_GYRO_ODR_RESERVED,
		/*! range */
		.range = BMI160_GYRO_RANGE_125_DPS,
		/*! bandwidth */
		.bw = BMI160_GYRO_BW_OSR4_MODE,	
	},
	/*! Structure to configure the auxiliary sensor */
	.aux_cfg = 
	{
		/*! Aux sensor, 1 - enable 0 - disable */
		.aux_sensor_enable = 0,
		/*! Aux manual/auto mode status */
		.manual_enable = 0,
		/*! Aux read burst length */
		.aux_rd_burst_len = 0,
		/*! output data rate */
		.aux_odr = 0,
		/*! i2c addr of auxiliary sensor */
		.aux_i2c_addr = 0,
	},
	/*! Structure to hold previous/old aux config parameters.
	 * This is used at driver level to prevent overwriting of same
	 * data, hence user does not change it in the code */
	.prev_aux_cfg = 
	{
		/*! Aux sensor, 1 - enable 0 - disable */
		.aux_sensor_enable = 0,
		/*! Aux manual/auto mode status */
		.manual_enable = 0,
		/*! Aux read burst length */
		.aux_rd_burst_len = 0,
		/*! output data rate */
		.aux_odr = 0,
		/*! i2c addr of auxiliary sensor */
		.aux_i2c_addr = 0,
	},	
	 /*! FIFO related configurations */
	.fifo = &m_fifo,
	/*! Read function pointer */
	.read = bmi160_read_fptr,
	/*! Write function pointer */
	.write = bmi160_write_fptr,
	/*!  Delay function pointer */
	.delay_ms = bmi160_delay_fptr,

};




void drv_bosh_official_api_read_acc(void)
{
	uint32_t error;

	
	
	error = bmi160_get_sensor_data(BMI160_ACCEL_ONLY, &bmi160_data.accel,&bmi160_data.gyro, &m_bmi160_dev);
	ERR_CHECK(error);
	
	DEBUG_PRINTF("accel X --> " LOG_FLOAT_MARKER, LOG_FLOAT(bmi160_data.accel.x));	
	DEBUG_PRINTF(" Y --> " LOG_FLOAT_MARKER, LOG_FLOAT(bmi160_data.accel.y));	
	DEBUG_PRINTF(" Z --> " LOG_FLOAT_MARKER"\n", LOG_FLOAT(bmi160_data.accel.z));	
	DEBUG_PRINTF("sensortime --> %08X\n",bmi160_data.accel.sensortime);
	
	DEBUG_PRINTF("gyro X --> " LOG_FLOAT_MARKER, LOG_FLOAT(bmi160_data.gyro.x));	
	DEBUG_PRINTF(" Y --> " LOG_FLOAT_MARKER, LOG_FLOAT(bmi160_data.gyro.y));	
	DEBUG_PRINTF(" Z --> " LOG_FLOAT_MARKER"\n", LOG_FLOAT(bmi160_data.gyro.z));	
	DEBUG_PRINTF("sensortime --> %08X\n",bmi160_data.gyro.sensortime);


	
}
//static struct bmi160_int_settg m_int_config =
//{
//	/*! Interrupt channel */
//	.int_channel = BMI160_INT_CHANNEL_NONE,
//	/*! Select Interrupt */
//	.int_type = BMI160_ACC_ANY_MOTION_INT,
//	/*! Structure configuring Interrupt pins */
//	.int_pin_settg = 
//	{
//		/*! To enable either INT1 or INT2 pin as output.
//		 * 0- output disabled ,1- output enabled */
//	 .output_en = 0,
//		/*! 0 - push-pull 1- open drain,only valid if output_en is set 1 */
//	 .output_mode = 0, 
//		/*! 0 - active low , 1 - active high level.
//		 * if output_en is 1,this applies to interrupts,else PMU_trigger */
//	 .output_type = 0, 
//		/*! 0 - level trigger , 1 - edge trigger  */
//	 .edge_ctrl = 0, 
//		/*! To enable either INT1 or INT2 pin as input.
//		 * 0 - input disabled ,1 - input enabled */
//	 .input_en = 0, 
//		/*! latch duration*/
//	 .latch_dur = 0, 
//	
//	},
//	/*! Union configures required interrupt */
//	.int_type_cfg = {0},
//	/*! FIFO FULL INT 1-enable, 0-disable */
//	.fifo_full_int_en = 0,
//	/*! FIFO WTM INT 1-enable, 0-disable */
//	.fifo_WTM_int_en = 0,


//};

void drv_bosh_official_api_init(void)
{
	uint32_t error;

	int_enable();
	

	error = bmi160_init(&m_bmi160_dev);
	ERR_CHECK(error);

	DEBUG_PRINTF(MOUDLE_NAME"Chip ID --> %02X\n",m_bmi160_dev.chip_id);

	DELAY_MS(5);
	
//	error = bmi160_soft_reset(&m_bmi160_dev);
//	ERR_CHECK(error);

//	error = bmi160_aux_init(&m_bmi160_dev);
//	ERR_CHECK(error);
	
//	m_bmi160_dev.any_sig_sel = BMI160_BOTH_ANY_SIG_MOTION_DISABLED;
	m_bmi160_dev.accel_cfg.power = BMI160_ACCEL_LOWPOWER_MODE;
//	m_bmi160_dev.gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;
//	m_bmi160_dev.aux_cfg.aux_sensor_enable = 0;	
	
	
//	error = bmi160_set_int_config(&m_int_config, &m_bmi160_dev);
//	ERR_CHECK(error);
//	
//	error = bmi160_set_sens_conf(&m_bmi160_dev);
//	ERR_CHECK(error);

	
	error = bmi160_set_power_mode(&m_bmi160_dev);
	ERR_CHECK(error);
//	
//	DELAY_MS(5);
	

//	error = bmi160_update_nvm(&m_bmi160_dev);
//	ERR_CHECK(error);
	
//	uint8_t data[1];	
//	error = bmi160_get_regs(BMI160_ACCEL_RANGE_ADDR, data, sizeof(data), &m_bmi160_dev);
//	ERR_CHECK(error);
//	DEBUG_PRINTF(MOUDLE_NAME"BMI160_ACCEL_RANGE_ADDR value --> %02X\n",data[0]);

	
	
}




void drv_bosh_official_api_uninit(void)
{
	uint32_t error;
	m_bmi160_dev.accel_cfg.power = BMI160_ACCEL_SUSPEND_MODE;
//	m_bmi160_dev.gyro_cfg.power = BMI160_GYRO_SUSPEND_MODE;
//	m_bmi160_dev.aux_cfg.aux_sensor_enable = 0;
	
	error = bmi160_set_power_mode(&m_bmi160_dev);
	ERR_CHECK(error);
	DELAY_MS(5);
	error = bmi160_soft_reset(&m_bmi160_dev);
	ERR_CHECK(error);	
	
	int_disable();
}



#endif
