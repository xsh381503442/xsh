#include "drv_font.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#include <string.h>

#include "am_util.h"
#include "am_devices_spiflash.h"





#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_FONT]:"

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

#define DELAY_US(X)                 am_util_delay_us(X)
#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)




//For HFMA2Ylib
void r_dat_bat(unsigned long TAB_addr,unsigned int Num,unsigned char *p_arr)
{
	am_hal_iom_buffer(4) write;
	write.bytes[0] = DRV_FONT_REG_Read;
	write.bytes[1] = (TAB_addr&0xFF0000)>>16;
	write.bytes[2] = (TAB_addr&0xFF00)>>8;
	write.bytes[3] = (TAB_addr&0xFF);
	
	am_hal_iom_spi_write(DRV_FONT_MOULDE, 
	                     DRV_FONT_CHIP_SELECT,
                       write.words, 
	                     4, AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);
	
	am_hal_iom_spi_read(DRV_FONT_MOULDE, 
	                    DRV_FONT_CHIP_SELECT,
							        (uint32_t *)p_arr, 
											Num, AM_HAL_IOM_RAW);
	
	
	
}




void drv_font_pwr_down(void)
{
	LOG_PRINTF(MOUDLE_NAME"drv_font_pwr_down\n");
	am_hal_iom_buffer(1) psEnableCommand;

	psEnableCommand.bytes[0] = DRV_FONT_REG_POWER_DOWN;	
	am_hal_iom_spi_write(DRV_FONT_MOULDE,
											 DRV_FONT_CHIP_SELECT,
											 psEnableCommand.words,
											 1, AM_HAL_IOM_RAW);
	

}


void drv_font_pwr_up(void)
{
	LOG_PRINTF(MOUDLE_NAME"drv_font_pwr_up\n");
	am_hal_iom_buffer(1) psEnableCommand;

	psEnableCommand.bytes[0] = DRV_FONT_REG_RELEASE_POWER_DOWN;	
	am_hal_iom_spi_write(DRV_FONT_MOULDE,
											 DRV_FONT_CHIP_SELECT,
											 psEnableCommand.words,
											 1, AM_HAL_IOM_RAW);
	

}



void drv_font_enable(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_font_enable\n");

	am_hal_iom_pwrctrl_enable(DRV_FONT_MOULDE);
	const am_hal_iom_config_t g_spi0_sIOMConfig =
	{
			.ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
			.ui32ClockFrequency = DRV_FONT_SPEED,
			.bSPHA = 0,
			.bSPOL = 0,
			.ui8WriteThreshold = 4,
			.ui8ReadThreshold = 60,
	};	
	am_hal_iom_config(DRV_FONT_MOULDE, &g_spi0_sIOMConfig);

	PIN_CONFIG(DRV_FONT_PIN_CLK, AM_HAL_PIN_5_M0SCK);
	PIN_CONFIG(DRV_FONT_PIN_MISO, AM_HAL_PIN_6_M0MISO);
	PIN_CONFIG(DRV_FONT_PIN_MOSI, AM_HAL_PIN_7_M0MOSI);
	PIN_CONFIG(DRV_FONT_PIN_CS, AM_HAL_PIN_48_M0nCE6);
	am_hal_iom_enable(DRV_FONT_MOULDE);	
}

void drv_font_disable(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_font_disable\n");
	am_hal_iom_disable(DRV_FONT_MOULDE);	
	
	PIN_CONFIG(DRV_FONT_PIN_CLK, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(DRV_FONT_PIN_MISO, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(DRV_FONT_PIN_MOSI, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(DRV_FONT_PIN_CS,AM_HAL_GPIO_INPUT | AM_HAL_GPIO_PULLUP);	
	
	
	am_hal_iom_pwrctrl_disable(DRV_FONT_MOULDE);
}










