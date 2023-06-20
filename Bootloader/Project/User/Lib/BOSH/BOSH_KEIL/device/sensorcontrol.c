#ifdef BOSH_KEIL_API 
/*!
 * @section LICENSE
 *
 * (C) Copyright 2011 - 2015 Bosch Sensortec GmbH All Rights Reserved
 *------------------------------------------------------------------------------
 * Disclaimer
 *
 * Common: Bosch Sensortec products are developed for the consumer goods
 * industry. They may only be used within the parameters of the respective valid
 * product data sheet.  Bosch Sensortec products are provided with the express
 * understanding that there is no warranty of fitness for a particular purpose.
 * They are not fit for use in life-sustaining, safety or security sensitive
 * systems or any system or device that may lead to bodily harm or property
 * damage if the system or device malfunctions. In addition, Bosch Sensortec
 * products are not fit for use in products which interact with motor vehicle
 * systems.  The resale and/or use of products are at the purchaser's own risk
 * and his own responsibility. The examination of fitness for the intended use
 * is the sole responsibility of the Purchaser.
 *
 * The purchaser shall indemnify Bosch Sensortec from all third party claims,
 * including any claims for incidental, or consequential damages, arising from
 * any product use not covered by the parameters of the respective valid product
 * data sheet or not approved by Bosch Sensortec and reimburse Bosch Sensortec
 * for all costs in connection with such claims.
 *
 * The purchaser must monitor the market for the purchased products,
 * particularly with regard to product safety and inform Bosch Sensortec without
 * delay of all security relevant incidents.
 *
 * Engineering Samples are marked with an asterisk (*) or (e). Samples may vary
 * from the valid technical specifications of the product series. They are
 * therefore not intended or fit for resale to third parties or for use in end
 * products. Their sole purpose is internal client testing. The testing of an
 * engineering sample may in no way replace the testing of a product series.
 * Bosch Sensortec assumes no liability for the use of engineering samples. By
 * accepting the engineering samples, the Purchaser agrees to indemnify Bosch
 * Sensortec from all claims arising from the use of engineering samples.
 *
 * Special: This software module (hereinafter called "Software") and any
 * information on application-sheets (hereinafter called "Information") is
 * provided free of charge for the sole purpose to support your application
 * work. The Software and Information is subject to the following terms and
 * conditions:
 *
 * The Software is specifically designed for the exclusive use for Bosch
 * Sensortec products by personnel who have special experience and training. Do
 * not use this Software if you do not have the proper experience or training.
 *
 * This Software package is provided `` as is `` and without any expressed or
 * implied warranties, including without limitation, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 * Bosch Sensortec and their representatives and agents deny any liability for
 * the functional impairment of this Software in terms of fitness, performance
 * and safety. Bosch Sensortec and their representatives and agents shall not be
 * liable for any direct or indirect damages or injury, except as otherwise
 * stipulated in mandatory applicable law.
 *
 * The Information provided is believed to be accurate and reliable. Bosch
 * Sensortec assumes no responsibility for the consequences of use of such
 * Information nor for any infringement of patents or other rights of third
 * parties which may result from its use.
 *
 * @file            sensorcontrol.c
 * @date
 * @commit
 *
 * @brief
 * This file contains sensor device adapter layer APIs
 *
 */

/*********************************************************************/
/* own header files */
#include "stdio.h"
//#include "SAM4S.h" //bosh development board chip
//#include "board.h" //bosh development board chip
//#include "twi.h" //bosh development board chip
#include "sensorcontrol.h"
#include "define.h"

#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"
#define MOUDLE_LOG_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					1
#define MOUDLE_NAME                     "[SENSORCONTROL]:"

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
/*********************************************************************/
/* static variables */
#if defined(INCLUDE_BMI160API)
static struct bmi160_t bmi160_m;
#endif

#if defined(INCLUDE_BMM050API)
static struct bmm050_t bmm050_m;
#endif


#if defined(INCLUDE_BMA2X2API)
static struct bma2x2_t bma2x2_m;
#endif

#if defined(INCLUDE_BMG160API)
static struct bmg160_t bmg160_m;
#endif

#include "am_mcu_apollo.h"
#include "am_util.h"

#define AUX_I2C_ADDR  0x69  

#define AUX_I2C_SDA           43
#define AUX_I2C_SCL           42
#define AUX_I2C_INT1          4
#define AUX_I2C_INT2          2
#define AUX_I2C_MODULE        3


#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

/*********************************************************************/
/* static variables */
U8 always_on_set_flag_u8r_m = 0;


/*********************************************************************/
/* functions */

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
void dev_i2c_delay(U32 ms)
{
	delaym_local(ms);
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
static void bus_disable(void)
{
	am_hal_iom_disable(AUX_I2C_MODULE);
	am_hal_gpio_pin_config(AUX_I2C_SCL,AM_HAL_PIN_DISABLE);
	am_hal_gpio_pin_config(AUX_I2C_SDA,AM_HAL_PIN_DISABLE);	
	am_hal_iom_pwrctrl_disable(AUX_I2C_MODULE);
}


/*!
 * @brief: configure the i2c read 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
s8 BST_SENSOR_I2C_READ(u8 device_addr, u8 register_addr, u8 *register_data, u8 length)
{
//	 twi_packet_t packet_rx;
//	 
//	 packet_rx.chip        = device_addr;
//	 packet_rx.addr[0]     = register_addr;
//	 packet_rx.addr_length = 0x01;
//	 packet_rx.buffer      = register_data;
//	 packet_rx.length      = length;
//	 
//	 return twi_master_read(TWI0, &packet_rx);
	
	
	bus_enable();
	am_hal_iom_i2c_read(AUX_I2C_MODULE,device_addr,(uint32_t *)register_data,length,AM_HAL_IOM_OFFSET(register_addr));
	bus_disable();	
	
	
	return 0;
 }

s8 BST_SENSOR_I2C_BURST_READ(u8 device_addr, u8 register_addr, u8 *register_data, u32 length)
{
	bus_enable();
	am_hal_iom_i2c_read(AUX_I2C_MODULE,device_addr,(uint32_t *)register_data,length,AM_HAL_IOM_OFFSET(register_addr));
	bus_disable();		
	return 0;
 }
/*!
 * @brief: configure the i2c write 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
s8 BST_SENSOR_I2C_WRITE(u8 device_addr, u8 register_addr, u8 *register_data, u8 length)
{
//	twi_packet_t packet_tx;
//	
//	packet_tx.chip        = device_addr;
//	packet_tx.addr[0]     = register_addr;
//	packet_tx.addr_length = 0x01;
//	packet_tx.buffer      = register_data;
//	packet_tx.length      = length;
//	
//	return twi_master_write(TWI0, &packet_tx);

	bus_enable();
	am_hal_iom_i2c_write(AUX_I2C_MODULE,device_addr,(uint32_t *)register_data,length,AM_HAL_IOM_OFFSET(register_addr));	
	bus_disable();
	
	return 0;
	
}
void delaym_local(u32 mili_seconds)
{
	DELAY_MS(mili_seconds);
}

S32 sel_configure(U8 sel_per)
{

	return 0;	
}
int32_t set_shuttle_vddio(uint8_t ucState)
{

	return 0;	
}
int32_t set_shuttle_vdd(uint8_t ucState)
{

	return 0;	
}

U8 dev_i2c_init(void)
{

	return 0;
}
S16 get_vs_configured_phy_odr(phys_id_enum_t phy_sensor_id)
{

	return 0;
}



#if defined(INCLUDE_BMI160API)
/*!
 * @brief:  mapping to sensor odr  register setting value from library 
 *              phy sensor odr output (from API :bsx_update_subscription) 
 * 
 * @param[in] : library configured phy sensor odr output
 * @param[out] : no
 * @param[in,out] : no
 *
 * @return :the phy sensor register setting value
 */
u8 bmi160_odr_acc_map(s16 update_phy_odr_output)
{
    u8 phy_sensor_reg_value = 0;
	switch (update_phy_odr_output)
	{
	    case 12:
		{
			phy_sensor_reg_value = BMI160_ACCEL_OUTPUT_DATA_RATE_12_5HZ;
		}
		break;
		
		case 50:
		{
			phy_sensor_reg_value = BMI160_ACCEL_OUTPUT_DATA_RATE_25HZ;
		}
		break;
		
		case 100:
		{
			phy_sensor_reg_value = BMI160_ACCEL_OUTPUT_DATA_RATE_100HZ;
		}
		break;
		case 200:
		{
			phy_sensor_reg_value = BMI160_ACCEL_OUTPUT_DATA_RATE_200HZ;
		}
		break;
		default :
		break;	 
	}
    return phy_sensor_reg_value;
}



/*!
 * @brief:  mapping to sensor odr  register setting value from library 
 *              phy sensor odr output (from API :bsx_update_subscription) 
 * 
 * @param[in] : library configured phy sensor odr output
 * @param[out] : no
 * @param[in,out] : no
 *
 * @return :the phy sensor register setting value
 */
u8 bmi160_odr_gyro_map(s16 update_phy_odr_output)
{
    u8 phy_sensor_reg_value = 0;
	switch (update_phy_odr_output)
	{
		case 25:
		{
			phy_sensor_reg_value = BMI160_GYRO_OUTPUT_DATA_RATE_25HZ;
		}
		break;
		
		case 50:
		{
			phy_sensor_reg_value = BMI160_GYRO_OUTPUT_DATA_RATE_50HZ;
		}
		break;
		
		case 100:
		{
			phy_sensor_reg_value = BMI160_GYRO_OUTPUT_DATA_RATE_100HZ;
		}
		break;

		case 200:
		{
			phy_sensor_reg_value = BMI160_GYRO_OUTPUT_DATA_RATE_200HZ;
		}
		break;

		case 400:
		{
			phy_sensor_reg_value = BMI160_GYRO_OUTPUT_DATA_RATE_400HZ;
		}
		break;
		default :
		break;	
	}
    return phy_sensor_reg_value;
}



/*!
 * @brief: bmi160 sensor error handler
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_err_handle_BMI160(BMI160_RETURN_FUNCTION_TYPE ErrorInput)
{
	U8 ret = OK;
	switch(ErrorInput)
	{
		case OK:
		{
			ret = ErrorInput;
		}
		break;
		
		case E_BMI160_NULL_PTR:
		{
			ret = ERROR_PARAMETER_NULL_PTR;
		}
		break;

		case E_BMI160_OUT_OF_RANGE:
		{
			ret = ERROR_PARAMETER_OUT_OF_RANGE;
		}
		break;

		case E_BMI160_COMM_RES:
		//case ERROR:
		default :
		{
			ret = ERROR_OTHERS;
	    }
	}
	return ret;
}


#endif

#if defined(INCLUDE_BMA2X2API)
/*!
 * @brief:  mapping to sensor odr  register setting value from library 
 *              phy sensor odr output (from API :bsx_update_subscription) 
 * 
 * @param[in] : library configured phy sensor odr output
 * @param[out] : no
 * @param[in,out] : no
 *
 * @return :the phy sensor register setting value
 */
u8 bma2x2_odr_acc_map(s16 update_phy_odr_output)
{
    u8 phy_sensor_reg_value = 0;
	switch (update_phy_odr_output)
	{
	    case 6:
		{
			phy_sensor_reg_value = BMA2x2_BW_7_81HZ;
		}
		break;
		
		case 12:
		{
			phy_sensor_reg_value = BMA2x2_BW_15_63HZ;
		}
		break;
		
		case 50:
		{
			phy_sensor_reg_value = BMA2x2_BW_62_50HZ;
		}
		break;
		
		case 100:
		{
			phy_sensor_reg_value = BMA2x2_BW_125HZ;
		}
		break;
		case 200:
		{
			phy_sensor_reg_value = BMA2x2_BW_250HZ;
		}
		break;
		case 400:
		{
			phy_sensor_reg_value = BMA2x2_BW_500HZ;
		}
		break;
		case 800:
		{
			phy_sensor_reg_value = BMA2x2_BW_1000HZ;
		}
		break;

		default :
		break;
	}
    return phy_sensor_reg_value; 

}
#endif



#if defined(INCLUDE_BMA2X2API)

/*!
 * @brief: acc error handler
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_err_handle_A(int ErrorInput)
{
	U8 ret = OK;
	switch(ErrorInput)
	{
		case OK:
		{
			ret = ErrorInput;
		}
		break;
		case E_BMA2x2_NULL_PTR:
		{
			ret = ERROR_PARAMETER_NULL_PTR;
		}
		break;

		case E_OUT_OF_RANGE:
		{
			ret = ERROR_PARAMETER_OUT_OF_RANGE;
		}
		break;

		case ERROR:
		//case E_EEPROM_BUSY:
		default :
			ret = ERROR_OTHERS;
	}
	return ret;
}

#endif



/*!
 * @brief:  acc sensor read rawdata
 *             
 * @param[in] : 
 * @param[out] : S32 *x, S32 *y, S32 *z
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_acc_read_xyzdata( S32 *x, S32 *y, S32 *z)
{
	U8 ret = OK;
	int acc_ret;
	
#if defined(INCLUDE_BMA2X2API)
	struct bma2x2_accel_data acc;
	acc_ret = bma2x2_read_accel_xyz(&acc);
#endif

#if defined(INCLUDE_BMI160API)
	struct bmi160_accel_t acc;
	
	acc_ret = bmi160_read_accel_xyz(&acc);
#endif

//	printf("chip ID = %d\r\n", bma2x2_m.chip_id);
	if(OK == acc_ret)
	{
	/* Axis Remap, customer should remap their sensor axis follow the android design */
#if defined(WSF_SFD20)
		*x = acc.y*(1);	 //assignig accelerometer x values to x
		*y = acc.x*(1); 	//assignig accelerometer x values to y
		*z = acc.z*(-1); 	//assignig accelerometer x values to z
#elif defined(WSF_SFD21)
		*x = acc.x*(-1); 	//assignig accelerometer x values to y
		*y = acc.y*(1);	 //assignig accelerometer x values to x
		*z = acc.z*(-1); 	//assignig accelerometer x values to z
#endif
	}
	else
	{		
#if defined(INCLUDE_BMA2X2API)
		ret = dev_err_handle_A(acc_ret);
#endif

#if defined(INCLUDE_BMI160API)
		ret = dev_err_handle_BMI160(acc_ret);
#endif
	}
	return ret;

}



 /*!
 * @brief: acc sensor get grange
 *             
 * @param[in] : 
 * @param[out] : U8 *range
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_acc_get_grange( U8 *range)
{
	U8 Range;
	U8 ret = OK;
	S32 acc_ret;
#if defined(INCLUDE_BMA2X2API)
	acc_ret = bma2x2_get_range(&Range);
	if(OK == acc_ret)
	{
		switch (Range)
		{
			case BMA2x2_RANGE_2G:
			{
				*range = ACCGRANGE_2G;
			}
			break;
			case BMA2x2_RANGE_4G:
			{
				*range = ACCGRANGE_4G;
			}
			break;
			case BMA2x2_RANGE_8G:
			{
				*range = ACCGRANGE_8G;
			}
			break;
			case BMA2x2_RANGE_16G:
			{
				*range = ACCGRANGE_16G;
			}
			break;
			default:
				break;
		}
	}
	else
	{
		ret = dev_err_handle_A(acc_ret);
	}
#elif defined(INCLUDE_BMI160API)
	acc_ret = bmi160_get_accel_range(&Range);
	ret = dev_err_handle_BMI160(acc_ret);
#endif


	return ret;
}



 /*!
 * @brief: acc sensor set grange
 *             
 * @param[in] : U8 range
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return : 0 when success
 */ 
U8 dev_acc_set_grange(U8 range)
{
	U8 ret = OK;
	S32 acc_ret;
	U8 acc_range;
#if defined(INCLUDE_BMA2X2API)

	switch (range)
	{
		case ACCGRANGE_2G:
		{
			acc_range = BMA2x2_RANGE_2G;	// set range 2g for grange
		}
		break;
		case ACCGRANGE_4G:
		{
			acc_range = BMA2x2_RANGE_4G;;	// set range 4g for grange
		}
		break;
		case ACCGRANGE_8G:
		{
			acc_range = BMA2x2_RANGE_8G;;	// set range 8g for grange
		}
		break;
		case ACCGRANGE_16G:
		{
			acc_range = BMA2x2_RANGE_16G;; // set range 16g for grange
		}
		break;
		default:
			ret = ERROR_PARAMETER_OUT_OF_RANGE;
			goto EXIT;
	}
	acc_ret = bma2x2_set_range(acc_range);
	if(OK != acc_ret)
	{
		ret = dev_err_handle_A(acc_ret);
	}

#elif defined(INCLUDE_BMI160API)
	acc_ret = bmi160_set_accel_range(acc_range);
	if(OK != acc_ret)
	{
		ret = dev_err_handle_BMI160(acc_ret);
	}
#endif

//EXIT:
	return ret;
}


 /*!
 * @brief: acc sensor get operation mode
 *             
 * @param[in] : 
 * @param[out] : U8 *opmode, value of operation mode
 * @param[in,out] : 
 *
 * @return :communication routines
 */ 
U8 dev_acc_get_opmode( U8 *opmode)
{
	U8 mode;
	U8 ret = OK;
	S32 acc_ret;
#if defined(INCLUDE_BMA2X2API)
	acc_ret = bma2x2_get_power_mode(&mode );
	if(OK == acc_ret)
	{
		switch (mode)
		{
			case BMA2x2_MODE_SUSPEND:
			{
				*opmode = ACCOPMODE_SLEEP;
			}
			break;
			case BMA2x2_MODE_LOWPOWER1:
			{
				*opmode = ACCOPMODE_LOWPOWER;
			}
				break;
			case BMA2x2_MODE_NORMAL:
			{
				*opmode = ACCOPMODE_REGULAR;
			}
			break;
			default:
				break;
		}
	}
	else
	{
		ret = dev_err_handle_A(acc_ret);
	}

#elif defined(INCLUDE_BMI160API)
	acc_ret = bmi160_get_accel_power_mode_stat(&mode);
	if(OK == acc_ret)
	{
		switch (mode)
		{
			case 0x10:
			{
				*opmode = ACCOPMODE_SLEEP;
			}
			break;
			case 0x11:
			{
				*opmode = ACCOPMODE_REGULAR;
			}
			break;
			case 0x12:
			{
				*opmode = ACCOPMODE_LOWPOWER;
			}
			break;
			default:
				break;
		}
	}
	else
	{
		ret = dev_err_handle_BMI160(acc_ret);
	}

#endif

	return ret;

}


 /*!
 * @brief: acc sensor set operation mode
 *             
 * @param[in] : U8 opmode,location of the opmode
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :communication routines
 */ 
U8 dev_acc_set_opmode( U8 opmode)
{
	U8 ret = OK;
	S32 acc_ret;
#if defined(INCLUDE_BMA2X2API)

	switch (opmode)
	{
		case ACCOPMODE_SLEEP:
		{
			acc_ret = bma2x2_set_power_mode(BMA2x2_MODE_SUSPEND); // it refers to accelerometer sleep mode
		}
		break;
		case ACCOPMODE_LOWPOWER:
		{
			acc_ret = bma2x2_set_power_mode(BMA2x2_MODE_LOWPOWER1);// it refers to accelerometer low power mode
		}
		break;
		case ACCOPMODE_REGULAR:
		{
			acc_ret = bma2x2_set_power_mode(BMA2x2_MODE_NORMAL);   // it refers to accelerometer normal mode
		}
		break;

		default:
		{
			acc_ret = E_OUT_OF_RANGE;
		}
		break;
	}

	if(OK != acc_ret)
	{
		ret = dev_err_handle_A(acc_ret);
	}

#elif defined(INCLUDE_BMI160API)
		switch (opmode)
		{

			case ACCOPMODE_SLEEP:
			{
				acc_ret = bmi160_set_command_register(0x10); // it refers to accelerometer suspend mode
			}
			break;
			case ACCOPMODE_LOWPOWER:
			{
				acc_ret = bmi160_set_command_register(0x12); // it refers to accelerometer low power mode
			}
			break;
			case ACCOPMODE_REGULAR:
			{
				acc_ret = bmi160_set_command_register(0x11); // it refers to accelerometer normal mode
			}
			break;

			default:
			{
				acc_ret = E_BMI160_OUT_OF_RANGE;
			}
			break;
		}
	delaym_local(3);


	if(OK != acc_ret)
	{
		ret = dev_err_handle_BMI160(acc_ret);
	}
#endif
	return ret;
}


 /*!
 * @brief: acc sensor get bandwidth
 *             
 * @param[in] : 
 * @param[out] : value of bandwidth
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_acc_get_bandwidth( U8 *bw)
{
	U8 BW;
	U8 ret = OK;
	S32 acc_ret;

#if defined(INCLUDE_BMA2X2API)
	acc_ret = bma2x2_get_bw(&BW);	  //reads in which bandwidth range accelerometer works
	if(OK == acc_ret)
	{
		switch (BW)
		{
			case BMA2x2_BW_7_81HZ:
			{
				*bw = ACC_BANDWIDTH_7_81HZ;
			}	
			break;
			case BMA2x2_BW_15_63HZ:
			{
				*bw = ACC_BANDWIDTH_15_63HZ;
			}
			break;
			case BMA2x2_BW_31_25HZ:
			{
				*bw = ACC_BANDWIDTH_31_25HZ;
			}
			break;
			case BMA2x2_BW_62_50HZ:
			{
				*bw = ACC_BANDWIDTH_62_50HZ;
			}
			break;
			case BMA2x2_BW_125HZ:
			{
				*bw = ACC_BANDWIDTH_125HZ;
			}
			break;
			default:
				break;
		}
	}

	else
	{
		ret = dev_err_handle_A(acc_ret);
	}

#elif defined(INCLUDE_BMI160API)
	acc_ret = bmi160_get_accel_bw(&BW);
	*bw = BW;
	ret = dev_err_handle_BMI160(acc_ret);
#endif

	return ret;
}


 /*!
 * @brief: acc sensor set bandwidth
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return : communication routines
 */ 
U8 dev_acc_set_bandwidth(U8 bw)
{
	U8 ret = OK;
	S32 acc_ret = 0;
#if defined(INCLUDE_BMA2X2API)

	switch (bw)
	{
		case ACC_BANDWIDTH_7_81HZ:
		{
			//accelerometer bandwidth approxmately set to 7.81HZ
			acc_ret = bma2x2_set_bw(BMA2x2_BW_7_81HZ);
		}
		break;
		case ACC_BANDWIDTH_15_63HZ:
		{
			//accelerometer bandwidth approxmately set to 15.63HZ
			acc_ret = bma2x2_set_bw(BMA2x2_BW_15_63HZ);
		}
		break;
		case ACC_BANDWIDTH_31_25HZ:
		{
			//accelerometer bandwidth approxmately set to 31.24HZ
			acc_ret = bma2x2_set_bw(BMA2x2_BW_31_25HZ);
		}
		break;
		case ACC_BANDWIDTH_62_50HZ:
		{
			//accelerometer bandwidth approxmately set to 62.5HZ
			acc_ret = bma2x2_set_bw(BMA2x2_BW_62_50HZ );
		}
		break;
		case ACC_BANDWIDTH_125HZ:
		{
			//accelerometer bandwidth approxmately set to 125HZ
			acc_ret = bma2x2_set_bw(BMA2x2_BW_125HZ);
		}
		break;
		default:
			break;
	}

	if(OK != acc_ret)
	{
		ret = dev_err_handle_A(acc_ret);
	}
#elif defined(INCLUDE_BMI160API)

	acc_ret = bmi160_set_accel_bw(bw);

	if(OK != acc_ret)
	{
		ret = dev_err_handle_BMI160(acc_ret);
	}

#endif



	return ret;
}


 /*!
 * @brief: get acc sensor register value
 *             
 * @param[in] : U8 address,address of the register. U8 count, Number of bytes of read from register.
 * @param[out] : 
 * @param[in,out] :  U8 *data, Data read from the register
 *
 * @return :result of communication routines
 */ 
U8 dev_acc_get_register(U8 *data,U8 address,U8 count)
{
	U8 ret = OK;
	S32 acc_ret;
#if defined(INCLUDE_BMA2X2API)
	acc_ret = bma2x2_read_reg(address,data,count);
	ret = dev_err_handle_A(acc_ret);

#elif defined(INCLUDE_BMI160API)
	acc_ret = bmi160_read_reg(address,data,count);
	ret = dev_err_handle_BMI160(acc_ret);

#endif

	return ret;
}

 /*!
 * @brief: set acc sensor register value
 *             
 * @param[in] : U8 address,address of the register. U8 count,Number of bytes of written to sensor
 * @param[out] : 
 * @param[in,out] : U8 *data, Data to be written to the register
 *
 * @return :result of communication routines
 */ 
U8 dev_acc_set_register(U8 *data,U8 address,U8 count)
{
	U8 ret = OK;
	S32 acc_ret;
#if defined(INCLUDE_BMA2X2API)
	acc_ret = bma2x2_write_reg(address,data,count);
	ret = dev_err_handle_A(acc_ret);

#elif defined(INCLUDE_BMI160API)
	acc_ret = bmi160_write_reg(address,data,count);
	ret = dev_err_handle_BMI160(acc_ret);
#endif

	return ret;
}

#if defined(INCLUDE_USECASE_NDOF)||defined(INCLUDE_USECASE_M4G)||defined(INCLUDE_USECASE_COMPASS)
#if defined(INCLUDE_BMM050API)

 /*!
 * @brief: mag sensor error handler
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_err_handle_M(BMM050_RETURN_FUNCTION_TYPE ErrorInput)
{
	U8 ret = OK;
	switch(ErrorInput)
	{
		case OK:
		{
			ret = ErrorInput;
		}
		break;
		case E_BMM050_NULL_PTR:
		{
			ret = ERROR_PARAMETER_NULL_PTR;
		}
		break;

		case E_BMM050_OUT_OF_RANGE:
		{
			ret = ERROR_PARAMETER_OUT_OF_RANGE;
		}
		break;

		case ERROR:
		default :
		{
			ret = ERROR_OTHERS;
		}
	}
	return ret;
}




/*!
 * @brief: mag sensor set operation mode
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_mag_set_opmode( U8 opmode)
{
	U8 ret = OK;
	int mag_ret;
	unsigned char func_state;
	unsigned char rep_xy, rep_z;

	switch (opmode)
	{
		case MAGOPMODE_FORCE:
		{
			/**< it refers magnetometer is active, low repetition number */
			func_state = BMM050_FORCED_MODE;
			rep_xy = BMM050_LOWPOWER_REPXY;
			rep_z = BMM050_LOWPOWER_REPZ;
			always_on_set_flag_u8r_m = 0;
		}
		break;
		case MAGOPMODE_REGULAR:
		{
			/**< it refers magnetometer is active, default repetition number */
			func_state = BMM050_FORCED_MODE;
			rep_xy = BMM050_REGULAR_REPXY;
			rep_z = BMM050_REGULAR_REPZ;
			always_on_set_flag_u8r_m = 0;
		}
		break;
		case MAGOPMODE_HIGHACCURACY:
		{
			/**< it refers magnetometer is active, very low noise output values  */
			func_state = BMM050_FORCED_MODE;
			rep_xy = BMM050_HIGHACCURACY_REPXY;
			rep_z = BMM050_HIGHACCURACY_REPZ;
			always_on_set_flag_u8r_m = 0;
		}
		break;
		case MAGOPMODE_SLEEP:
		{
			/**< It refers bmc050 OFF state */
			func_state = BMM050_SUSPEND_MODE;
			always_on_set_flag_u8r_m = 0;
		}
		break;
		case MAGOPMODE_ALWAYSON:
		{
			/**< it refers magnetometer is active, very low power consumption, calibration part is active	*/
			func_state = BMM050_FORCED_MODE;
			rep_xy = BMM050_LOWPOWER_REPXY;
			rep_z = BMM050_LOWPOWER_REPZ;
			always_on_set_flag_u8r_m = 1;
		}
		break;

		default:
		{
			ret = ERROR_PARAMETER_OUT_OF_RANGE;
			goto EXIT_1;
		}
	}
	mag_ret = bmm050_set_functional_state(func_state);
	if(OK!=mag_ret)
		goto EXIT;
	delaym_local(1);
	if(MAGOPMODE_SLEEP != opmode)
	{
		mag_ret = bmm050_set_rep_XY(rep_xy);
		if(OK!=mag_ret)
			goto EXIT;
		mag_ret = bmm050_set_rep_Z(rep_z);
	}
EXIT:
	ret = dev_err_handle_M(mag_ret);

EXIT_1:

	return ret;
}

 /*!
 * @brief: mag sensor set operation mode
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_mag_get_opmode( U8 *opmode)
{
	U8 mode;
	U8 repetitionsxy = 0;                                       /* Holds number of XY-Repetitions */
	U8 repetitionsz = 0;
	U8 ret = OK;
	BMM050_RETURN_FUNCTION_TYPE mag_ret;

	mag_ret = bmm050_get_rep_XY(&repetitionsxy);
	if(OK!=mag_ret)
		goto EXIT;

	mag_ret = bmm050_get_rep_Z(&repetitionsz);
	if(OK!=mag_ret)
		goto EXIT;

	mag_ret = bmm050_get_power_mode(&mode);
	if(OK!=mag_ret)
		goto EXIT;

	if (mode == BMM050_OFF)
	{
		*opmode = MAGOPMODE_SLEEP;	/**< It refers bmc050 OFF state */
	}
	else
	{
		if (((repetitionsxy == BMM050_LOWPOWER_REPXY)&&(repetitionsz == BMM050_LOWPOWER_REPZ) && (mode == BMM050_ON)))
		{
			if (always_on_set_flag_u8r_m == 1)
			{
				*opmode = MAGOPMODE_ALWAYSON;	/**< it refers magnetometer is active, very low power consumption, calibration part is active	*/
			}
			else
			{
				*opmode = MAGOPMODE_FORCE;	/**< it refers magnetometer is active, low repetition number */
			}
		}
		else
		{
			if (((repetitionsxy == BMM050_REGULAR_REPXY)&&(repetitionsz == BMM050_REGULAR_REPZ)  && (mode == BMM050_ON)))
			{
				*opmode = MAGOPMODE_REGULAR;   /**< it refers magnetometer is active, default repetition number */
			}
			else
			{
				if (((repetitionsxy == BMM050_HIGHACCURACY_REPXY)&&(repetitionsz == BMM050_HIGHACCURACY_REPZ) && (mode == BMM050_ON)))
				{
					*opmode = MAGOPMODE_HIGHACCURACY;	 /**< it refers magnetometer is active, very low noise output values  */
				}
			}
		}
	}
EXIT:
	ret = dev_err_handle_M(mag_ret);
	return ret;
}


 /*!
 * @brief: mag sensor read raw data
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_mag_read_xyzdata( S32 *x, S32 *y, S32 *z)
{
	U8 ret = OK;
	BMM050_RETURN_FUNCTION_TYPE mag_ret;
	
#if defined(INCLUDE_BMI160API_BMM050_2TH_INTERFACE)/* bmm050 2th interface */
	struct bmi160_mag_xyz_s32_t mdata;
#elif defined(INCLUDE_BMM050API)/* bmm050 I2C connect directively */
	struct bmm050_mag_s32_data_t mdata;
#endif

	//struct bmm050_mag_s32_data_t mdata;
	/*note : bmm050 rawdata output data are all 16bits compensated data with 
	API:bmm050_read_mag_data_XYZ_s32, 16 LSB represent 1uT and stored as 32bits format*/
	//mag_ret = bmm050_read_mag_data_XYZ_s32(&mdata);
	
	
#if defined(INCLUDE_BMI160API_BMM050_2TH_INTERFACE)/* bmm050 2th interface */
	mag_ret = bmi160_bmm150_mag_compensate_xyz(&mdata);	
#elif defined(INCLUDE_BMM050API)/* bmm050 I2C connect directively(mag forece mode) */
    mag_ret = bmm050_read_mag_data_XYZ_s32(&mdata);
#endif   	
	
	if(OK!=mag_ret)
		goto EXIT;

	//mag_ret = bmm050_set_functional_state(BMM050_FORCED_MODE);
	if(OK!=mag_ret)
		goto EXIT;

	//if((mdata.datax != BMM050_OVERFLOW_OUTPUT_S32)&&
	//	(mdata.datay != BMM050_OVERFLOW_OUTPUT_S32)&&
	//	(mdata.dataz != BMM050_OVERFLOW_OUTPUT_S32))
	if(OK == mag_ret)
	{
	//axis remapping
#if defined(WSF_SFD20)
	
#if defined(INCLUDE_BMI160API_BMM050_2TH_INTERFACE)/* bmm050 2th interface */
	   *x = (mdata.x)*(1);
	   *y = (mdata.y)*(1);
	   *z = (mdata.z)*(-1);
#elif defined(INCLUDE_BMM050API)/* bmm050 I2C connect directively */
		*x = (mdata.datay)*(1);
		*y = (mdata.datax)*(1);
		*z = (mdata.dataz)*(-1);
#endif
		
#elif defined(WSF_SFD21)

#if defined(INCLUDE_BMI160API_BMM050_2TH_INTERFACE)/* bmm050 2th interface */
        *x = (mdata.x)*(-1);
      	*y = (mdata.y)*(1);
      	*z = (mdata.z)*(-1);
#elif defined(INCLUDE_BMM050API)/* bmm050 I2C connect directively */
      	*x = (mdata.datax)*(-1);
		*y = (mdata.datay)*(1);
		*z = (mdata.dataz)*(-1);
#endif

#endif
	}
	else
	{
		ret = ERROR_RESULT_OUT_OF_RANGE;
		goto EXIT1;
	}
EXIT:
	ret = dev_err_handle_M(mag_ret);
EXIT1:
	return ret;
}


/*!
 * @brief: mag sensor self test
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_mag_self_test(void)
{
	int err = 0;
	U8 selftest_result;
	S16 adv_res;
	struct bmm050_mag_s32_data_t bmm050data;

	//Self test
	{
		//Power On, Device check
		err = bmm050_init(&bmm050_m);
		if(err)
			goto EXIT;
		delaym_local(1);
		if(bmm050_m.company_id == 0x32)
		{
			printf("Mag Self Test -- Chip ID: OK! \r\n");
		}
		else
		{
			printf("Mag Self Test -- Chip ID: Not OK! \r\n");
        }
		//Status check
		err = bmm050_set_functional_state(BMM050_SLEEP_MODE);
		if(err)
			goto EXIT;
		err = bmm050_set_functional_state(BMM050_NORMAL_MODE);
		if(err)
			goto EXIT;

		delaym_local(3);

		err = bmm050_set_selftest(1);
		if(err)
			goto EXIT;
		delaym_local(3);

		err = bmm050_get_selftest_XYZ(&selftest_result);
		if(err)
			goto EXIT;
		if(7 == selftest_result)
		{
			printf("Mag Self Test -- Status Check: OK! \r\n");
		}
		else
		{
			printf("Mag Self Test -- Status Check: Not OK! \r\n");
		}
		err = bmm050_soft_rst();
		if(err)
			goto EXIT;
		delaym_local(1);

		//ADC Mag Field Level
		err = bmm050_read_mag_data_XYZ_s32(&bmm050data);
		if(!err)
		{
			if(((bmm050data.datax>(-20800))&&(bmm050data.datax<20800))&&
				((bmm050data.datay>(-20800))&&(bmm050data.datay<20800))&&
				((bmm050data.dataz>(-40000))&&(bmm050data.dataz<40000)))
			{
				printf("Mag Self Test -- ADC:Mag Field Level: OK! \r\n");
				err = bmm050_soft_rst();
				delaym_local(1);
			}
			else
			{
				printf("Mag Self Test -- ADC:Mag Field Level: Not OK! \r\n");
				goto EXIT;
			}
		}
		else
			goto EXIT;
	}
	//Advanced self test
	err = bmm050_perform_advanced_selftest (&adv_res);
	if(!err) 
	{
		if((adv_res > 2880)&&(adv_res < 4320))
		{
			printf("Mag Self Test -- Mag Advanced Self Test Result: OK! \r\n");
		}
		else
		{
			printf("Mag Self Test -- Mag Advanced Self Test Result: Not OK! \r\n");
		}
	}

EXIT:
	err = bmm050_soft_rst();
	delaym_local(1);
	printf("%s [Error Code:%d]  \r\n", __func__, err);
	return err;
}

#elif defined(BMI160_MAG_INTERFACE_SUPPORT)


/*!
 * @brief: mag sensor set operation mode
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_mag_set_opmode( U8 opmode)
{
	U8 ret = OK;
	unsigned char func_state;
	switch (opmode)
	{
		case MAGOPMODE_FORCE:
		{
			/**< it refers magnetometer is active, low repetition number */
			func_state = BMI160_MAG_FORCE_MODE;
	    }
		break;

		case MAGOPMODE_SLEEP:
		{
			/**< It refers bmc050 OFF state */
			func_state = BMI160_MAG_SUSPEND_MODE;
		}	
		break;

		default:
		{
			ret = ERROR_PARAMETER_OUT_OF_RANGE;
			goto EXIT;
		}
	}
	ret = bmi160_set_bmm150_mag_and_secondary_if_power_mode(func_state);
	if(OK!=ret)
		goto EXIT;

EXIT:
	return ret;
}



/*!
 * @brief: read bmm055 mag data from secondary interface
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_mag_read_xyzdata_secondary_interface( S32 *x, S32 *y, S32 *z)
{
	U8 ret = OK;
	struct bmi160_mag_xyz_s32_t data;
	ret = bmi160_bmm150_mag_compensate_xyz(&data);

	if (0 != ret) 
	{
		printf("read akm failed.\r\n");
		return ret;
	}

	*x = data.x;
	*y = data.y;
	*z = data.z;

	dev_mag_set_opmode(MAGOPMODE_FORCE);

	return ret;
}

#endif
#endif

#if defined(INCLUDE_USECASE_NDOF)||defined(INCLUDE_USECASE_IMU)

#if defined(INCLUDE_BMG160API)

/*!
 * @brief: error handle
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_err_handle_G(BMG160_RETURN_FUNCTION_TYPE ErrorInput)
{
	U8 ret = OK;
	switch(ErrorInput)
	{
		case OK:
		{
			ret = ErrorInput;
		}
		break;
		case E_BMG160_NULL_PTR:
		{
			ret = ERROR_PARAMETER_NULL_PTR;
		}
		break;

		case E_BMG160_OUT_OF_RANGE:
		{
			ret = ERROR_PARAMETER_OUT_OF_RANGE;
		}
		break;

		case ERROR:
		default :
		{
			ret = ERROR_OTHERS;
		}
		break;
	}
	return ret;
}
#endif


/*!
 * @brief: gyro read sensor rawdata
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_gyro_read_xyzdata( S32 *x, S32 *y, S32 *z)
{
	U8 ret= OK;

#if defined(INCLUDE_BMG160API)
	BMG160_RETURN_FUNCTION_TYPE gyro_ret;
    struct	bmg160_data_t gyro_raw;

	gyro_ret = bmg160_get_data_XYZ(&gyro_raw);
	if(OK != gyro_ret)
	{
	    goto EXIT;
	}
		
	//axis remapping
	*x = gyro_raw.datay;
	*y = gyro_raw.datax;
	*z = gyro_raw.dataz*(-1);
#endif

#if defined(INCLUDE_BMI160API)
	BMI160_RETURN_FUNCTION_TYPE gyro_ret;
	struct	bmi160_gyro_t gyro_raw;
	gyro_ret = bmi160_read_gyro_xyz(&gyro_raw);
	if(OK != gyro_ret)
	{
	    goto EXIT;
	}

	//axis remapping
	*x = gyro_raw.y;
	*y = gyro_raw.x;
	*z = gyro_raw.z*(-1);

#endif

EXIT:
#if defined(INCLUDE_BMG160API)
	ret = dev_err_handle_G(gyro_ret);
#endif

#if defined(INCLUDE_BMI160API)
	ret = dev_err_handle_BMI160(gyro_ret);
#endif
	return ret;
}



/*!
 * @brief: gyro set operation mode
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_gyro_set_opmode(U8 opmode)
{
#if defined(INCLUDE_BMG160API)
	return dev_err_handle_G(bmg160_set_power_mode(opmode));
#elif defined(INCLUDE_BMI160API)
	if(opmode>=0x14&&opmode<=0x17)
	{
		return dev_err_handle_BMI160(bmi160_set_command_register(opmode));
	}
	else
	{
		return ERROR_PARAMETER_OUT_OF_RANGE;
	}
#endif
}


/*!
 * @brief: gyro sensor set range
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_gyro_set_range(U8 range)
{
#if defined(INCLUDE_BMG160API)
	return dev_err_handle_G(bmg160_set_range_reg(range));
#elif defined(INCLUDE_BMI160API)
	if(range<=0x7)
		return dev_err_handle_BMI160(bmi160_set_gyro_range(range));
	else
		return ERROR_PARAMETER_OUT_OF_RANGE;
#endif
}


/*!
 * @brief: gyro sensor set bandwidth
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_gyro_set_bandwidth(U8 bandwidth)
{
#if defined(INCLUDE_BMG160API)
	return dev_err_handle_G(bmg160_set_bw(bandwidth));
#elif defined(INCLUDE_BMI160API)
	if(bandwidth<=0x3)
	{
		return dev_err_handle_BMI160(bmi160_set_gyro_bw(bandwidth));
	}
	else
	{
		return ERROR_PARAMETER_OUT_OF_RANGE;
	}
#endif

}
#endif





/*!
 * @brief: initialize the sensors. customer should change according to the board
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 init_sensorlist(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"init_sensorlist\n");
	U8 ret = OK;
//	int acc_ret;
//	U8 data, mag_dev_addr, mag_urst_len;
//	u8 banwid = 0;
//	u8 range_get = 0;
//	u8 ret_bma2x2 = 0;
//	u8 ret_bmm050 = 0;
//	u8 ret_bmi160 = 0;	
/*power on the shuttle board */	
//	set_shuttle_vdd(OFF);
//	set_shuttle_vddio(OFF);
//	delaym_local(500);
//	set_shuttle_vdd(ON);
//	set_shuttle_vddio(ON);
//	delaym_local(500);
/*According to the schematic of shuttle board, the I2C and SPI Bus are using the same GPIO.
	So, one bus should be isolated while the other is being used. */
//	sel_configure(1); 
	
#if defined (INCLUDE_BMG160API)
	BMG160_RETURN_FUNCTION_TYPE gyro_ret;
#endif

#if defined(INCLUDE_BMI160API_BMM050_2TH_INTERFACE)
	bmi160_m.dev_addr = BMI160_I2C_ADDR1;
	bmi160_m.bus_write = BST_SENSOR_I2C_WRITE;
	bmi160_m.bus_read = BST_SENSOR_I2C_READ;
	bmi160_m.burst_read = BST_SENSOR_I2C_BURST_READ;
	bmi160_m.delay_msec = delaym_local;
	/*This function must be called in advance to initialize the I2C READ AND WRITE function*/
	ret = bmi160_init(&bmi160_m);
	ERR_CHECK(ret);
	delaym_local(10);
	ret = bmi160_set_accel_under_sampling_parameter(BMI160_US_DISABLE); //0 normal mode,1 lowpower mode cond+acc_nom
	ERR_CHECK(ret);
	delaym_local(10);
	ret = bmi160_set_accel_range(BMI160_ACCEL_RANGE_2G);
	ERR_CHECK(ret);
	delaym_local(10);
	ret = bmi160_set_accel_output_data_rate(BMI160_GYRO_OUTPUT_DATA_RATE_100HZ,BMI160_ACCEL_NORMAL_AVG4);//set acc phy ODR
	ERR_CHECK(ret);
	delaym_local(10);
	ret = bmi160_set_accel_bw(BMI160_ACCEL_NORMAL_AVG4);
	ERR_CHECK(ret);
	delaym_local(10);
	ret = bmi160_set_command_register(0x11); // Sets the PMU mode for the Accelerometer to normal
	ERR_CHECK(ret);
	delaym_local(50);
	ret = bmi160_set_gyro_range((uint8_t)0);
	ERR_CHECK(ret);
	delaym_local(10);
	ret = bmi160_set_gyro_output_data_rate(BMI160_GYRO_OUTPUT_DATA_RATE_100HZ);//set gyro phy ODR
	ERR_CHECK(ret);
	delaym_local(10);
	ret = bmi160_set_gyro_bw(BMI160_GYRO_NORMAL_MODE);
	ERR_CHECK(ret);
	delaym_local(10);
	ret = bmi160_set_command_register(0x15); // Sets the PMU mode for the Gyroscope to normal
	ERR_CHECK(ret);
//	delaym_local(90);
//	ret += bmi160_bmm150_mag_interface_init(&bmi160_m.chip_id);
//	bmi160_set_mag_burst(3);
//	delaym_local(10);
	if(OK!=ret)
	{
	    goto EXIT;
	}  
#endif

#if defined (INCLUDE_BMI160API_DIRECT)
	bmi160_m.bus_write = BST_SENSOR_I2C_WRITE;
	bmi160_m.bus_read = BST_SENSOR_I2C_READ;
	bmi160_m.delay_msec = delaym_local;
	bmi160_m.dev_addr = BMI160_I2C_ADDR2;
	acc_ret = bmi160_init(&bmi160_m);
	if(OK!=acc_ret)
	{
	    goto EXIT;
	}	
	debug_sensor("bmi160 Chip ID: %x\r\n", bmi160_m.chip_id);
#endif	

#if defined(INCLUDE_BMA2X2API)
	//bma2x2.dev_addr = 0x10; //(in bmc156 sensor)
	bma2x2_m.dev_addr = 0x18; //(in bmi055 sensor)
	bma2x2_m.bus_write = BST_SENSOR_I2C_WRITE;
	bma2x2_m.bus_read = BST_SENSOR_I2C_READ;
	bma2x2_m.burst_read = BST_SENSOR_I2C_READ;
	bma2x2_m.delay_msec = delaym_local;
	/*This function must be called in advance to initialize the I2C READ AND WRITE function*/
	acc_ret = bma2x2_init(&bma2x2_m);
	delaym_local(10);
	bma2x2_set_power_mode(BMA2x2_MODE_NORMAL);
	bma2x2_set_bw(bma2x2_odr_acc_map(get_vs_configured_phy_odr(ENUM_ACCELEROMETER)));
	/* This API used to read back the written value of bandwidth*/
	bma2x2_get_bw(&banwid);
	bma2x2_set_range(BMA2x2_RANGE_2G);
	bma2x2_get_range(&range_get);
	if(OK!=acc_ret)
	{
	    goto EXIT;
	}  
#endif

#if defined(INCLUDE_BMM050API_DRICTIVE_INTERFACE)
	bmm050_m.dev_addr = 0x12;
	bmm050_m.bus_write = BST_SENSOR_I2C_WRITE;
	bmm050_m.bus_read = BST_SENSOR_I2C_READ;
	bmm050_m.delay_msec = delaym_local;
	/*This function must be called in advance to initialize the I2C READ AND WRITE function*/
	ret_bmm050 = bmm050_init(&bmm050_m);
	delaym_local(10);
	bmm050_set_functional_state(BMM050_NORMAL_MODE);
	/*use bmm050 forcemode read mag rawdata,do not need odr setting*/
	if(OK!=ret_bmm050)
	{
	    goto EXIT;
	} 
#endif

#if defined(INCLUDE_BMG160API)
    bmg160_m.dev_addr = 0x68;
	bmg160_m.bus_write = BST_SENSOR_I2C_WRITE;
	bmg160_m.bus_read = BST_SENSOR_I2C_READ;
	bmg160_m.delay_msec = delaym_local;
	gyro_ret = bmg160_init(&bmg160_m);
	if(OK != gyro_ret)
	{
	    goto EXIT;
	}  
#endif

#if defined(BMI160_MAG_INTERFACE_SUPPORT)
	bmi160_get_i2c_device_addr(&mag_dev_addr);
	bmi160_set_i2c_device_addr(mag_dev_addr);
	bmi160_bmm150_mag_interface_init();
	bmi160_set_mag_burst(3);
	bmi160_get_mag_burst(&mag_urst_len);
#endif

EXIT:
	return ret;
}

#include <string.h>	
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>	


double b_abs(float x)
{
	return abs(x);
}


#if 0
U8 dev_i2c_init(void)
{
	Pin pins[] = {PINS_TWI0};
	int i;

	/* Configure TWI pins. */
	for (i = 0; i < 500; i ++);
	PIO_Configure(pins, PIO_LISTSIZE(pins));
	twi = TWID_Initialize(ID_TWI0,TWI0,400000, BOARD_MCK);
	for (i = 0; i < 5000; i ++);

	return 0;
}
#endif
#endif //BOSH_KEIL_API

/** @}*/





