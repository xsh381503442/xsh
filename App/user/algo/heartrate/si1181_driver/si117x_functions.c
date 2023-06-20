/*************************************************************************//**
 * @file 
 * @brief Si117x reusable functions 
 * @version 16.4.28
 *****************************************************************************
 * @section License
 * <b>(C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ******************************************************************************
 * 
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 *****************************************************************************/


#include "si117x_functions.h"
#include "si117x_config.h"
#include "am_mcu_apollo.h"
#include "am_util_delay.h"
#include "drv_config.h"


#ifdef  OHR_SPI
//#include "ohr_spi_driver.h"
#else
//#include "drv_i2c_hw.h"
#endif


#define MOUDLE_INFO_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					  0
#define MOUDLE_NAME                     "[SI117X_FUNCTIONS]:"

#if DEBUG_ENABLED == 1 && MOUDLE_INFO_ENABLED == 1
#define INFO_PRINTF(...)                  SEGGER_RTT_printf(0,##__VA_ARGS__)       
#define INFO_HEXDUMP(p_data, len)        SEGGER_RTT_Write(0, (const char*)p_data, len);           
#else
#define INFO_PRINTF(...) 
#define INFO_HEXDUMP(p_data, len)             
#endif

#if DEBUG_ENABLED == 1 && MOUDLE_DEBUG_ENABLED == 1
#define DEBUG_PRINTF(...)                 SEGGER_RTT_printf(0,##__VA_ARGS__)      
#define DEBUG_HEXDUMP(p_data, len)        SEGGER_RTT_Write(0, (const char*)p_data, len);      
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif


extern SemaphoreHandle_t Appolo_I2C_Semaphore;

#define delay_10ms()  am_util_delay_ms(10)
void drv_i2c_hw1_init(void)
{
	const am_hal_iom_config_t IOMConfig =
	{
		.ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
		.ui32ClockFrequency = AM_HAL_IOM_400KHZ,
		.bSPHA = 0,
		.bSPOL = 0,
		.ui8WriteThreshold = 4,
		.ui8ReadThreshold = 60,
	};
	am_hal_iom_pwrctrl_enable(I2C_1_MODULE);
	am_hal_iom_config(I2C_1_MODULE, &IOMConfig);
	am_hal_iom_enable(I2C_1_MODULE);	
	am_hal_iom_power_off_save(I2C_1_MODULE);
	
	am_hal_gpio_pin_config(GPIO_PIN_NUM_I2C_1_SCL, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(GPIO_PIN_NUM_I2C_1_SDA, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	
	
}

void drv_i2c_hw1_enable(void)
{	
	xSemaphoreTake( Appolo_I2C_Semaphore, portMAX_DELAY );
	am_hal_gpio_pin_config(GPIO_PIN_NUM_I2C_1_SCL, GPIO_PIN_NUM_I2C_1_SCL_CONFIG);
	am_hal_gpio_pin_config(GPIO_PIN_NUM_I2C_1_SDA, GPIO_PIN_NUM_I2C_1_SDA_CONFIG);	
	am_hal_iom_power_on_restore(I2C_1_MODULE);	
}

void drv_i2c_hw1_disable(void)
{
	am_hal_gpio_pin_config(GPIO_PIN_NUM_I2C_1_SCL, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(GPIO_PIN_NUM_I2C_1_SDA, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_iom_power_off_save(I2C_1_MODULE);
	xSemaphoreGive( Appolo_I2C_Semaphore );	
}

void drv_i2c_hw1_write(uint32_t address, uint8_t reg, uint8_t *p_data, uint32_t len)
{	
	am_hal_iom_i2c_write(I2C_1_MODULE,address, (uint32_t *)p_data, len, AM_HAL_IOM_OFFSET(reg));

}
void drv_i2c_hw1_read(uint32_t address, uint8_t reg, uint8_t *p_data, uint32_t len)
{
	//am_hal_iom_i2c_write(I2C_1_MODULE,address, (uint32_t *)&reg, 1, AM_HAL_IOM_RAW|AM_HAL_IOM_NO_STOP);
	am_hal_iom_i2c_read(I2C_1_MODULE, address, (uint32_t *)p_data, len, AM_HAL_IOM_OFFSET(reg));
}

#if 0
const am_hal_iom_config_t g_HR_IOMConfig =
{
    .ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
    .ui32ClockFrequency = AM_HAL_IOM_400KHZ,
    .bSPHA = 0,
    .bSPOL = 0,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
};

void drv_i2c_hw1_enable(void)
{	 
	xSemaphoreTake( Appolo_I2C_Semaphore, portMAX_DELAY );
	
	// Enable IOM
	bsp_iom_enable(BSP_HEARTRATE_IOM,g_HR_IOMConfig);

}

void drv_i2c_hw1_disable(void)
{
	//Disable IOM
	bsp_iom_disable(BSP_HEARTRATE_IOM);
	
	xSemaphoreGive( Appolo_I2C_Semaphore );		
}

void drv_i2c_hw1_write(uint32_t address, uint8_t reg, uint8_t *p_data, uint32_t len)
{	
	am_hal_iom_i2c_write(BSP_HEARTRATE_IOM,address, (uint32_t *)p_data, len, AM_HAL_IOM_OFFSET(reg));

}
void drv_i2c_hw1_read(uint32_t address, uint8_t reg, uint8_t *p_data, uint32_t len)
{
	am_hal_iom_i2c_write(BSP_HEARTRATE_IOM,address, (uint32_t *)&reg, 1, AM_HAL_IOM_RAW|AM_HAL_IOM_NO_STOP);
	am_hal_iom_i2c_read(BSP_HEARTRATE_IOM, address, (uint32_t *)p_data, len, AM_HAL_IOM_RAW);
}

#endif


#define delay_10ms()  am_util_delay_ms(10)


/**************************************************************************//**
 * @brief Write to Si117x register
 *****************************************************************************/
int16_t Si117xWriteToRegister(uint8_t address, uint8_t data)
{
	drv_i2c_hw1_write(SI117X_I2C_ADDRESS, address, (uint8_t *)&data, sizeof(data));
	return 0;

}

/**************************************************************************//**
 * @brief Read from Si117x register.
 *****************************************************************************/
int16_t Si117xReadFromRegister(uint8_t address)
{
		uint8_t data;
		drv_i2c_hw1_read(SI117X_I2C_ADDRESS, address, (uint8_t *)&data, sizeof(data));	
	  return data;

}

/**************************************************************************//**
 * @brief block write to si117x
 *****************************************************************************/
int16_t Si117xBlockWrite(uint8_t address, uint8_t length, uint8_t *values)
{
	drv_i2c_hw1_write(SI117X_I2C_ADDRESS, address, values, length);
	return 0;
}

/**************************************************************************//**
 * @brief Block read from Si117x.
 *****************************************************************************/
int16_t Si117xBlockRead(uint8_t address, uint8_t length, uint8_t *values)
{
	drv_i2c_hw1_read(SI117X_I2C_ADDRESS, address, values, length);	
		return 0;
}

/**************************************************************************//**
 * @brief Initialize low level handle and clear irq queue.
 *****************************************************************************/
s16 Si117xInit(void)
{
  u8 data1,data;//[0x3F];
	s16 error;
	//for(int i=0;i<0x3f;++i)
	//I2CDevice_Read (SI117X_I2C_ADDRESS, 0x3A, &data[i], 1);
	//Si117xWriteToRegister(0x3A, 0x00); // Set HW_KEY to 0
	data = Si117xReadFromRegister (REG_HW_KEY); // Read back HW_KEY to check
	
	Si117xWriteToRegister(REG_HW_KEY, 0x00); // Set HW_KEY to 0
	
	data1 = Si117xReadFromRegister (REG_HW_KEY); // Read back HW_KEY to check
	
  //data = Si117xReadFromRegister(REG_PART_ID);

  if ((data != 0x71) && (data != 0x72) && (data != 0x73) && (data != 0x74) && (data != 0x75) &&
		                                     (data != 0x81) && (data != 0x82) && (data != 0x83) && (data != 0x84) && (data != 0x85))
	  error = -1;

	
  return error;
}


/*****************************************************************************/
/**************    Compile Switches   ****************************************/
/*****************************************************************************/

/***************************************************************************//**
 * @brief 
 *   Resets the Si117x, clears any interrupts and initializes the HW_KEY 
 *   register.
 * @param[in]  
 *   The programmer's toolkit handle
 * @retval  0       
 *   Success
 * @retval  <0      
 *   Error
 ******************************************************************************/
s16 Si117xReset(void)
{
  s16 retval = 0;

  //
  // Do not access the Si117x earlier than 25 ms from power-up. 
  // Uncomment the following lines if Si117xReset() is the first
  // instruction encountered, and if your system MCU boots up too 
  // quickly. 
  //
  delay_10ms();
  delay_10ms();
  delay_10ms();

  // Perform the Reset Command
  retval += Si117xWriteToRegister(REG_COMMAND, CMD_RESET_SW);

  // Delay for 10 ms. This delay is needed to allow the Si117x
  // to perform internal reset sequence. 
  delay_10ms();
  delay_10ms();
  delay_10ms();

  retval += Si117xWriteToRegister(REG_HW_KEY, 0x00);
  return retval;
}
/***************************************************************************//**
 * @brief 
 *   Helper function to send a command to the Si117x
 ******************************************************************************/
static uint8_t maxcount1 = 0;
/***************************************************************************//**
 * @brief 
 *   Helper function to send a command to the Si117x
 ******************************************************************************/
int16_t _sendCmd(uint8_t command) {
  int16_t response_before = 0, response_after = 0;
  int16_t retval;
  uint8_t count = 0;
  const uint8_t max_count = 8;
  maxcount1 = 0;
  count = 0;
  while (count < max_count) {
    retval = Si117xReadFromRegister(REG_RESPONSE0);
    if (retval < 0) {
      return -1;  // i2c error occurred
    } else if ((retval & RSP0_COUNTER_ERR) == RSP0_COUNTER_ERR) {
      // CMD_ERR flag was set. Typically caused by a previous invalid command code or read/write to invalid parameter address
      // return the command error code * -1
      return -2;
    } else if ((retval & RSP0_COUNTER_MASK) == response_before && count > 0) {
      // cmd_ctr is stable, okay to execute command
      break;
    }
    response_before = retval & RSP0_COUNTER_MASK;
    count++;
  }
  if(maxcount1<count)maxcount1 = count;
  // Return an error if command counter is not stable.
  // This typically indicates a communication error.
  if (count >= max_count) {
    return -3;
  }

  // Send the command.
  retval = Si117xWriteToRegister(REG_COMMAND, command);
  if (retval != 0) {
    return -4;
  }

  if (command == 0x1C)
  {
  	delay_10ms();
  }

  // Wait for the change in the command counter, confirming command execution.
  if (command != CMD_RESET_CMD_CTR && command != CMD_RESET_SW && command != CMD_SET_I2C_ADDR) {
    count = 0;
    while (count < max_count) {
      response_after = Si117xReadFromRegister(REG_RESPONSE0);
      if (response_after < 0) {
        return -5;  // i2c error occurred
      } else if ((response_after & RSP0_COUNTER_ERR) == RSP0_COUNTER_ERR) {
        // CMD_ERR flag was set. Typically caused by invalid command code or read/write to invalid parameter address
        // return the command error code * -1
        return -6;
      } else if ((response_after & RSP0_COUNTER_MASK) == ((response_before + 1) & RSP0_COUNTER_MASK)) {
        break;
      }
      count++;
    }

    // Return an error if command counter did not increment after max_count retries.
    // This typically indicates a communication error.
    if (count >= max_count) {
      return -7;
    }
		if(maxcount1<count)maxcount1 = count;
  }

  // Command was sent and executed correctly.
  return 0;
}

/***************************************************************************//**
 * @brief 
 *   Sends a NOP command to the Si117x
 * @param[in] 
 *   The programmer's toolkit handle
 * @retval  0       Success
 * @retval  <0      Error
 ******************************************************************************/
s16 Si117xNop(void)
{
  return _sendCmd(0x00);
}

/***************************************************************************//**
 * @brief
 *   Sends the UPDATE_FIFO_COUNT command and returns the current FIFO count
 * @param[in] si117x_handle
 *   The programmer's toolkit handle
 * @param[in] rev_id
 *   The contents REV_ID register of the Si117x. 2 == -B2, 3 == -B3
 *   The resulting FIFO count
 * @retval  >= 0    the FIFO count
 * @retval  < 0      Error
 ******************************************************************************/
int16_t Si117xUpdateFifoCount(uint8_t rev_id)
{
  int16_t retval;
	uint16_t high, low, high_check;
	if (rev_id == 2) {
	  retval = _sendCmd(CMD_UPDATE_FIFO_COUNT);

	  if (retval < 0) {
	    return retval;
	  }

	  high = Si117xReadFromRegister( REG_FIFO_COUNT_H);
	  low =  Si117xReadFromRegister( REG_FIFO_COUNT_L);
	}
	else {
    // -B3, no command counter update
    // re-read FIFO counter if there is a rollover of the counter during reading of the multibyte counter
    do
    {
      high = Si117xReadFromRegister( REG_FIFO_COUNT_H);
      low = Si117xReadFromRegister( REG_FIFO_COUNT_L);
      high_check = Si117xReadFromRegister( REG_FIFO_COUNT_H);

      // return error if i2c error occurs
      if (high < 0 || low < 0 || high_check < 0)
        return -1;
    } while( high != high_check);
	}

	retval = (high << 8) + low;
  return retval;
}
/***************************************************************************//**
 * @brief 
 *   Sets up a new I2C address to the Si117x
 * @param[in] si117x_handle
 *   The programmer's toolkit handle
 * @param[in] value   
 *   The New I2C address
 * @retval  0       
 *   Success
 * @retval  < 0
 *   Error
 ******************************************************************************/
int16_t Si117xSetI2CAddress( uint8_t value)
{
  int16_t retval;

  retval=Si117xWriteToRegister(REG_HOSTIN0, value);
  retval += _sendCmd(CMD_SET_I2C_ADDR);
  return retval;
}

/***************************************************************************//**
 * @brief 
 *   Sends a START command to the Si117x
 * @param[in] 
 *   The programmer's toolkit handle
 * @retval  0       
 *   Success
 * @retval  <0      
 *   Error
 ******************************************************************************/
int16_t Si117xStart(void)
{
  return _sendCmd(CMD_START);
}

s16 Si117xSendCmd(u8 command)
{
  return _sendCmd(command);
}
/***************************************************************************//**
 * @brief 
 *   Sends a STOP command to the Si117x
 * @param[in] 
 *   The programmer's toolkit handle
 * @param[in] value   
 *   The byte value to mark FIFO
 * @retval  0       
 *   Success
 * @retval  <0      
 *   Error
 ******************************************************************************/
int16_t Si117xStop(void)
{
  return _sendCmd(CMD_STOP);
}

/***************************************************************************//**
 * @brief 
 *   Sends a FLUSH_FIFO command to the Si117x
 * @param[in] si117x_handle
 *   The programmer's toolkit handle
 * @retval  0       
 *   Success
 * @retval  <0      
 *   Error
 ******************************************************************************/
int16_t Si117xFlushFIFO(void)
{
  Si117xWriteToRegister(REG_FIFO_FLUSH, 1);
  delay_10ms();
  Si117xWriteToRegister(REG_FIFO_FLUSH, 0);

  return _sendCmd(CMD_FLUSH_FIFO);
}

/***************************************************************************//**
 * @brief 
 *   Inserts a shielded synch message in FIFO
 * @param[in] si117x_handle
 *   The programmer's toolkit handle
 * @param[in] value   
 *   The byte value to be inserted in FIFO
 * @retval  0       
 *   Success
 * @retval  <0      
 *   Error
 ******************************************************************************/
int16_t Si117xInsertSynchMsg( uint8_t value)
{
  int16_t retval;

	retval = Si117xWriteToRegister(REG_HOSTIN0, value);
	retval += _sendCmd(CMD_INSERT_SYNCH_MSG);
	return retval;
}


/***************************************************************************//**
 * @brief 
 *   Reloads a new I2C address to the PPG slave based on MISO pin status
 * @param[in] si117x_handle
 *   The programmer's toolkit handle
 * @param[in] value
 *   The New I2C address
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si117xReloadI2CAddress( uint8_t value)
{
  int16_t retval;

	retval = Si117xWriteToRegister(REG_HOSTIN0, value);
	retval += _sendCmd(CMD_RELOAD_I2C_ADDR);	// CMD_REALOAD_I2C_ADDR
	return retval;
}

/***************************************************************************//**
 * @brief
 *   Sets the three wire SPI mode
 * @param[in] si117x_handle
 *   The programmer's toolkit handle
 * @retval  0
 *   Success
 * @retval  <0
 *   Error
 ******************************************************************************/
int16_t Si117xSetThreeWireSPI(void)
{
  return _sendCmd(CMD_SET_THREE_WIRE_SPI);
}

/***************************************************************************//**
 * @brief
 *   Reads a Parameter from the Si117x
 * @param[in] si117x_handle 
 *   The programmer's toolkit handle
 * @param[in] address   
 *   The address of the parameter. 
 * @retval <0       
 *   Error
 * @retval 0-255    
 *   Parameter contents
 ******************************************************************************/
int16_t Si117xParamRead( uint8_t address)
{
  // returns Parameter[address]
  int16_t retval;
  uint8_t cmd = CMD_PARAM_QUERY_BYTE_MASK + (address & 0x3F);

  retval= _sendCmd(cmd);
  if( retval != 0 ) {
    return retval;
  }

  retval = Si117xReadFromRegister(REG_RESPONSE1);
  return retval;
}

/***************************************************************************//**
 * @brief 
 *   Writes a byte to an Si117x Parameter
 * @param[in] si117x_handle 
 *   The programmer's toolkit handle
 * @param[in] address 
 *   The parameter address 
 * @param[in] value   
 *   The byte value to be written to the Si117x parameter
 * @retval 0    
 *   Success
 * @retval <0   
 *   Error
 * @note This function ensures that command completion is checked.
 * If setting parameter is not done properly, no measurements will
 * occur. This is the most common error. It is highly recommended
 * that host code make use of this function.
 ******************************************************************************/
int16_t Si117xParamSet( uint8_t address, uint8_t value)
{
  int16_t retval;
  uint8_t buffer[2];

  buffer[0]= value;						// parameter value
  buffer[1]= CMD_PARAM_SET_BYTE_MASK + (address & 0x3F);	// param_write command

  retval = Si117xWriteToRegister(REG_HOSTIN0, buffer[0]);
	if(retval!=0)
	{
		DEBUG_PRINTF("Si117xParamSet step1 address = 0x%X, error = %d",address,retval);
	}	

  retval += _sendCmd(buffer[1]);
	if(retval!=0)
	{
		DEBUG_PRINTF("Si117xParamSet step2 address = 0x%X, error = %d",address,retval);
	}
	if(maxcount1>=5)DEBUG_PRINTF("_sendCmd max count %d",maxcount1);
  return retval;
}


/***************************************************************************//**
 * @brief
 *   Reads and clears an interrupt from the 117x
 * @param[in] si117x_handle
 *   The programmer's toolkit handle
 * @param[in] rev_id
 *   The device revision. Determines the interrupt method
 * . 0 = -B3 (read irq_status to clear), 1 = -B2 (send CMD_CLEAR_IRQ_STATUS)
 * @retval <0
 *   Error
 * @retval 0-255
 *   irq_status - which interrupt sources caused the interrupt
 ******************************************************************************/
int16_t Si117xGetIrqStatus( uint8_t rev_id)
{
	// returns Parameter[address]
	int16_t retval;
	uint8_t irq_status;

	if( rev_id == 2 )
	{
		// -B2, command based approach
		irq_status = Si117xReadFromRegister(REG_IRQ_STATUS_B2);
		if( irq_status )
		{
			retval=Si117xWriteToRegister(REG_HOSTIN0, irq_status);
			retval += _sendCmd(CMD_CLEAR_IRQ_STATUS);
		}
	}
	else
	{
		// -B3, hardware i2c reg approach, push-pull mode
		irq_status = Si117xReadFromRegister(REG_IRQ_STATUS);
	}

	return irq_status;
}
