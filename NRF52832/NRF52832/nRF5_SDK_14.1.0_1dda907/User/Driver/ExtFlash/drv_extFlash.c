#include "drv_extFlash.h"
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_spi_mngr.h"
#include "nrf_delay.h"

//#include "boards.h"


#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME drv_extFlash
#define NRF_LOG_LEVEL       LOG_LEVEL_OFF
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();




#define EXTFLASH_REG_ID                        (0x9F)   
#define EXTFLASH_REG_ID_VAL                    (0x17)   
#define EXTFLASH_REG_PAGE_WRITE      				   (0x02)  /* Write to Memory instruction */
#define EXTFLASH_REG_WRSR       							 (0x01)  /* Write Status Register instruction */
#define EXTFLASH_REG_WREN       							 (0x06)  /* Write enable instruction */
#define EXTFLASH_REG_READ       							 (0x03)  /* Read from Memory instruction */
#define EXTFLASH_REG_RDSR       							 (0x05)  /* Read Status Register instruction  */
#define EXTFLASH_REG_SECTOR_ERASE         		 (0x20)  /* Sector Erase instruction */
#define EXTFLASH_REG_CHIP_ERASE         			 (0xC7)  /* Bulk Erase instruction */        
#define EXTFLASH_REG_BLOCK_32K_ERASE					 (0x52)
#define EXTFLASH_REG_BLOCK_64K_ERASE					 (0xD8)
#define EXTFLASH_REG_POWER_DOWN       				 (0xB9)
#define EXTFLASH_REG_RELEASE_POWER_DOWN        (0xAB)

#define EXTFLASH_WIP_FLAG   									 (0x01)  /* Write In Progress (WIP) flag */
#define EXTFLASH_DUMMY_BYTE 									 (0xA5)
#define EXTFLASH_SET        									 (1)

#define AM_DEVICES_SPIFLASH_WEL         0x02        // Write enable latch
#define AM_DEVICES_SPIFLASH_WIP         0x01        // Write in progress
#define AM_DEVICES_SPIFLASH_WREN        0x06        // Write enable          




#ifdef LAYER_APPLICATION 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#define TASK_EXTFLASH_DELAY            10
#define DELAY_US(X)                 nrf_delay_us(X)
#define DELAY_MS(X)                 nrf_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X );}while(0)
#endif

void drv_extFlash_enable(void)
{
	#ifdef LAYER_APPLICATION 
	nrf_gpio_cfg_input(EXTFLASH_READY_PIN, NRF_GPIO_PIN_NOPULL);
	while(!nrf_gpio_pin_read(EXTFLASH_READY_PIN))
	{
		DELAY_MS(TASK_EXTFLASH_DELAY);
	}	
	nrf_gpio_cfg_output(EXTFLASH_READY_PIN);
	nrf_gpio_pin_clear(EXTFLASH_READY_PIN);	
	#endif
	
	nrf_gpio_cfg_output(EXTFLASH_SPI_SCK_PIN);
	nrf_gpio_cfg_output(EXTFLASH_SPI_MOSI_PIN);
	nrf_gpio_cfg_input(EXTFLASH_SPI_MISO_PIN, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_output(EXTFLASH_SPI_SS_PIN);
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);

	NRF_SPI0->PSELSCK = EXTFLASH_SPI_SCK_PIN;
	NRF_SPI0->PSELMOSI = EXTFLASH_SPI_MOSI_PIN;
	NRF_SPI0->PSELMISO = EXTFLASH_SPI_MISO_PIN;

	NRF_SPI0->FREQUENCY = NRF_DRV_SPI_FREQ_4M;		
	NRF_SPI0->CONFIG = ((SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos) | (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos));
	NRF_SPI0->EVENTS_READY = 0U;
	NRF_SPI0->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);	
}
void drv_extFlash_disable(void)
{
	nrf_gpio_cfg_default(EXTFLASH_SPI_SS_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_SCK_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_MOSI_PIN);
	nrf_gpio_cfg_default(EXTFLASH_SPI_MISO_PIN);	
	NRF_SPI0->PSELSCK  = (uint32_t)0xFFFFFFFF;
	NRF_SPI0->PSELMOSI = (uint32_t)0xFFFFFFFF;
	NRF_SPI0->PSELMISO = (uint32_t)0xFFFFFFFF;
	NRF_SPI0->EVENTS_READY = 0U;
	NRF_SPI0->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);
	
	#ifdef LAYER_APPLICATION 
	nrf_gpio_cfg_input(EXTFLASH_READY_PIN, NRF_GPIO_PIN_PULLUP);
	#endif
}


static uint8_t transfer(uint8_t byte)
{
	uint8_t rx_data;
	NRF_SPI0->TXD = (uint32_t)(byte);
	while (NRF_SPI0->EVENTS_READY == 0U);
	NRF_SPI0->EVENTS_READY = 0U;
	rx_data = (uint8_t)NRF_SPI0->RXD;
	return rx_data;
}

static void wait_for_write_end(void)
{
	uint8_t status = 0;  
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);	
	transfer(EXTFLASH_REG_RDSR);
	do
	{
			status = transfer(EXTFLASH_DUMMY_BYTE);
	}
	while ((status & EXTFLASH_WIP_FLAG) == EXTFLASH_SET);
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
}
static void write_enable(void)
{ 
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_WREN); 
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
}

void drv_extFlash_sector_erase(uint32_t addr)
{
	NRF_LOG_INFO("drv_extFlash_sector_erase");
	write_enable(); 
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_SECTOR_ERASE);
	transfer((addr & 0xFF0000) >> 16);
	transfer((addr & 0xFF00) >> 8);
	transfer(addr & 0xFF); 
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
	wait_for_write_end();
}

void drv_extFlash_32K_erase(uint32_t addr)
{
	write_enable();
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_BLOCK_32K_ERASE);
	transfer((addr & 0xFF0000) >> 16);
	transfer((addr & 0xFF00) >> 8);
	transfer(addr & 0xFF);	
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
	wait_for_write_end();
}

void drv_extFlash_64K_erase(uint32_t addr)
{
	write_enable(); 
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_BLOCK_64K_ERASE);
	transfer((addr & 0xFF0000) >> 16);
	transfer((addr & 0xFF00) >> 8);
	transfer(addr & 0xFF);	 
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
	wait_for_write_end();
}

void drv_extFlash_chip_erase(void)
{
	write_enable(); 
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_CHIP_ERASE); 
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
	wait_for_write_end();	
}
void drv_extFlash_page_write( uint32_t addr, uint8_t * p_data, uint16_t len)
{
	write_enable(); 
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_PAGE_WRITE);
	transfer((addr & 0xFF0000) >> 16);
	transfer((addr & 0xFF00) >> 8);
	transfer(addr & 0xFF);
	while (len--)
	{
			transfer(*p_data);
			p_data++;
	}	  
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
	wait_for_write_end();
	
}
void drv_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len)
{  
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_READ);
	transfer((addr & 0xFF0000) >> 16);
	transfer((addr& 0xFF00) >> 8);
	transfer(addr & 0xFF);
	while (len--) 
	{
			*p_data = transfer(EXTFLASH_DUMMY_BYTE);
			p_data++;
	}	  
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
}

uint32_t drv_extFlash_readID(void)
{
	
	NRF_LOG_INFO("drv_extFlash_readID"); 
	uint8_t id = 0;
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_ID);
	id = transfer(EXTFLASH_DUMMY_BYTE);
	NRF_LOG_INFO("ID-->0x%02X",id);
	id = transfer(EXTFLASH_DUMMY_BYTE);
	NRF_LOG_INFO("ID-->0x%02X",id);
	id = transfer(EXTFLASH_DUMMY_BYTE);
	NRF_LOG_INFO("ID-->0x%02X",id);
	id = transfer(EXTFLASH_DUMMY_BYTE);
	NRF_LOG_INFO("ID-->0x%02X",id);
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
	return id;
}



void drv_extFlash_pwr_down(void)
{
	NRF_LOG_INFO("drv_extFlash_pwr_down");
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
	transfer(EXTFLASH_REG_POWER_DOWN);  
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);
}

void drv_extFlash_pwr_up(void)
{
	NRF_LOG_INFO("drv_extFlash_pwr_up");
	nrf_gpio_pin_clear(EXTFLASH_SPI_SS_PIN);
//	transfer(0x66); 
//	transfer(0x99); 
	
	transfer(EXTFLASH_REG_RELEASE_POWER_DOWN); 
	nrf_gpio_pin_set(EXTFLASH_SPI_SS_PIN);

}









