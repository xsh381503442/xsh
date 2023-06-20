#include "drv_extFlash.h"

#include "am_mcu_apollo.h"

#include "am_devices_spiflash.h"
#include "am_util_delay.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "com_data.h"
#include "drv_lcd.h"
//#define DELAY_US(X)                 am_util_delay_us(X)
#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)


extern SetValueStr SetValue;
extern SemaphoreHandle_t Appolo_SPI_Semaphore;

static uint8_t spiflash_status(void)
{
	am_hal_iom_buffer(1) psResponse; 
	am_hal_iom_spi_read(DRV_EXTFLASH_MOULDE,
											DRV_EXTFLASH_CHIP_SELECT,
											psResponse.words, 1,
											AM_HAL_IOM_OFFSET(W25Q128_REG_RDSR));
	return psResponse.bytes[0];
}


static void drv_extFlash_sector_erase(uint32_t addr)
{	
    am_hal_iom_buffer(4) psCommand;

    //
    // Send the write-enable command to prepare the external flash for program
    // operations.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;
    am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                         DRV_EXTFLASH_CHIP_SELECT,
                         psCommand.words, 1, AM_HAL_IOM_RAW);

    //
    // Wait for the write enable latch status bit.
    //
    while ( !(spiflash_status() & AM_DEVICES_SPIFLASH_WEL) );

    //
    // Prepare the sector erase command, followed by the three-byte external
    // flash address.
    //
//    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_SE;
		psCommand.bytes[0] = W25Q128_REG_SECTOR_ERASE;
    psCommand.bytes[1] = (addr & 0x00FF0000) >> 16;
    psCommand.bytes[2] = (addr & 0x0000FF00) >> 8;
    psCommand.bytes[3] = addr & 0x000000FF;

    //
    // Send the command to erase the desired sector.
    //
    am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                         DRV_EXTFLASH_CHIP_SELECT,
                         psCommand.words, 4, AM_HAL_IOM_RAW);

    //
    // Wait for status to indicate that the write is no longer in progress.
    //
    while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );	
	
	
	
}

static void drv_extFlash_32K_erase(uint32_t addr)
{
    am_hal_iom_buffer(4) psCommand;

    //
    // Send the write-enable command to prepare the external flash for program
    // operations.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;
    am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                         DRV_EXTFLASH_CHIP_SELECT,
                         psCommand.words, 1, AM_HAL_IOM_RAW);

    //
    // Wait for the write enable latch status bit.
    //
    while ( !(spiflash_status() & AM_DEVICES_SPIFLASH_WEL) );

    //
    // Prepare the sector erase command, followed by the three-byte external
    // flash address.
    //
		psCommand.bytes[0] = W25Q128_REG_BLOCK_32K_ERASE;
    psCommand.bytes[1] = (addr & 0x00FF0000) >> 16;
    psCommand.bytes[2] = (addr & 0x0000FF00) >> 8;
    psCommand.bytes[3] = addr & 0x000000FF;

    //
    // Send the command to erase the desired sector.
    //
    am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                         DRV_EXTFLASH_CHIP_SELECT,
                         psCommand.words, 4, AM_HAL_IOM_RAW);

    //
    // Wait for status to indicate that the write is no longer in progress.
    //
    while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );		
	
}
static void drv_extFlash_64K_erase(uint32_t addr)
{
    am_hal_iom_buffer(4) psCommand;

    //
    // Send the write-enable command to prepare the external flash for program
    // operations.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;
    am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                         DRV_EXTFLASH_CHIP_SELECT,
                         psCommand.words, 1, AM_HAL_IOM_RAW);

    //
    // Wait for the write enable latch status bit.
    //
    while ( !(spiflash_status() & AM_DEVICES_SPIFLASH_WEL) );

    //
    // Prepare the sector erase command, followed by the three-byte external
    // flash address.
    //
    psCommand.bytes[0] = W25Q128_REG_BLOCK_64K_ERASE;
    psCommand.bytes[1] = (addr & 0x00FF0000) >> 16;
    psCommand.bytes[2] = (addr & 0x0000FF00) >> 8;
    psCommand.bytes[3] = addr & 0x000000FF;

    //
    // Send the command to erase the desired sector.
    //
    am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                         DRV_EXTFLASH_CHIP_SELECT,
                         psCommand.words, 4, AM_HAL_IOM_RAW);

    //
    // Wait for status to indicate that the write is no longer in progress.
    //
    while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );		
	
	
}

static void drv_extFlash_chip_erase(void)
{
    am_hal_iom_buffer(1) psCommand;

    //
    // Send the write-enable command to prepare the external flash for program
    // operations.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;
    am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                         DRV_EXTFLASH_CHIP_SELECT,
                         psCommand.words, 1, AM_HAL_IOM_RAW);

    //
    // Wait for the write enable latch status bit.
    //
    while ( !(spiflash_status() & AM_DEVICES_SPIFLASH_WEL) );

    //
    // Send the bulk erase command.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_BE;
    am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                         DRV_EXTFLASH_CHIP_SELECT,
                         psCommand.words, 1, AM_HAL_IOM_RAW);

    //
    // Wait for status to indicate that the write is no longer in progress.
    //
    while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );
}
static void drv_extFlash_page_write( uint32_t addr, uint8_t * p_data, uint16_t len)
{
	
    uint32_t i;
    uint32_t ui32DestAddress;
    uint32_t ui32BytesRemaining;
    uint32_t ui32TransferSize;
    uint8_t *pui8Source;

    am_hal_iom_buffer(1) psEnableCommand;
    am_hal_iom_buffer(64) psWriteCommand;

    //
    // Prepare the command for write-enable.
    //
    psEnableCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;

    //
    // Set the total number of bytes, and the starting transfer destination.
    //
    ui32BytesRemaining = len;
    pui8Source = p_data;
    ui32DestAddress = addr;

    while ( ui32BytesRemaining )
    {
        //
        // Set up a write command to hit the beginning of the next "chunk" of
        // flash.
        //
        psWriteCommand.bytes[0] = AM_DEVICES_SPIFLASH_PP;
        psWriteCommand.bytes[1] = (ui32DestAddress & 0x00FF0000) >> 16;
        psWriteCommand.bytes[2] = (ui32DestAddress & 0x0000FF00) >> 8;
        psWriteCommand.bytes[3] = ui32DestAddress & 0x000000FF;

        //
        // Set the transfer size to either 32, or the number of remaining
        // bytes, whichever is smaller.
        //
        ui32TransferSize = ui32BytesRemaining > 32 ? 32 : ui32BytesRemaining;

        //
        // Fill the rest of the command buffer with the data that we actually
        // want to write to flash.
        //
        for ( i = 0; i < ui32TransferSize; i++ )
        {
            psWriteCommand.bytes[4 + i] = pui8Source[i];
        }

        //
        // Send the write-enable command to prepare the external flash for
        // program operations, and wait for the write-enable latch to be set in
        // the status register.
        //
        am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                             DRV_EXTFLASH_CHIP_SELECT,
                             psEnableCommand.words,
                             1, AM_HAL_IOM_RAW);

        while ( !(spiflash_status() & AM_DEVICES_SPIFLASH_WEL) );

        //
        // Send the write command.
        //
        am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                             DRV_EXTFLASH_CHIP_SELECT,
                             psWriteCommand.words,
                             (ui32TransferSize + 4), AM_HAL_IOM_RAW);

        //
        // Wait for status to indicate that the write is no longer in progress.
        //
        while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );

        //
        // Update the number of bytes remaining, as well as the source and
        // destination pointers
        //
        ui32BytesRemaining -= ui32TransferSize;
        pui8Source += ui32TransferSize;
        ui32DestAddress += ui32TransferSize;
    }
	
	
}
static void drv_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len)
{
    uint32_t i, ui32BytesRemaining, ui32TransferSize, ui32CurrentReadAddress;
    uint8_t *pui8Dest;

    uint32_t pui32WriteBuffer[1];
    uint8_t *pui8WritePtr;

    uint32_t pui32ReadBuffer[16];
    uint8_t *pui8ReadPtr;

    pui8WritePtr = (uint8_t *)(&pui32WriteBuffer);
    pui8ReadPtr = (uint8_t *)(&pui32ReadBuffer);

    //
    // Set the total number of bytes,and the starting transfer destination.
    //
    ui32BytesRemaining = len;
    pui8Dest = p_data;
    ui32CurrentReadAddress = addr;

    while ( ui32BytesRemaining )
    {
        //
        // Set the transfer size to either 64, or the number of remaining
        // bytes, whichever is smaller.
        //
        ui32TransferSize = ui32BytesRemaining > 64 ? 64 : ui32BytesRemaining;

        pui8WritePtr[0] = AM_DEVICES_SPIFLASH_READ;
        pui8WritePtr[1] = (ui32CurrentReadAddress & 0x00FF0000) >> 16;
        pui8WritePtr[2] = (ui32CurrentReadAddress & 0x0000FF00) >> 8;
        pui8WritePtr[3] = ui32CurrentReadAddress & 0x000000FF;

        //
        // Send the read command.
        //
        am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
                             DRV_EXTFLASH_CHIP_SELECT,
                             pui32WriteBuffer, 4,
                             AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

        am_hal_iom_spi_read(DRV_EXTFLASH_MOULDE,
                            DRV_EXTFLASH_CHIP_SELECT, pui32ReadBuffer,
                            ui32TransferSize, AM_HAL_IOM_RAW);

        //
        // Copy the received bytes over to the RxBuffer
        //
        for ( i = 0; i < ui32TransferSize; i++ )
        {
            pui8Dest[i] = pui8ReadPtr[i];
        }

        //
        // Update the number of bytes remaining and the destination.
        //
        ui32BytesRemaining -= ui32TransferSize;
        pui8Dest += ui32TransferSize;
        ui32CurrentReadAddress += ui32TransferSize;
    }
}




static uint32_t drv_extFlash_ID_read(void)
{
	am_hal_iom_buffer(3) psResponse;
	am_hal_iom_spi_read(DRV_EXTFLASH_MOULDE,
											DRV_EXTFLASH_CHIP_SELECT,
											psResponse.words, 3,
											AM_HAL_IOM_OFFSET(0x9F));
	return psResponse.words[0] & 0x00FFFFFF;
	
	
}
static void drv_extFlash_pwr_down(void)
{
	am_hal_iom_buffer(1) psEnableCommand;

	psEnableCommand.bytes[0] = W25Q128_REG_POWER_DOWN;	
	am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
											 DRV_EXTFLASH_CHIP_SELECT,
											 psEnableCommand.words,
											 1, AM_HAL_IOM_RAW);
	

}

static void drv_extFlash_pwr_up(void)
{
	am_hal_iom_buffer(1) psEnableCommand;

	psEnableCommand.bytes[0] = W25Q128_REG_RELEASE_POWER_DOWN;	
	am_hal_iom_spi_write(DRV_EXTFLASH_MOULDE,
											 DRV_EXTFLASH_CHIP_SELECT,
											 psEnableCommand.words,
											 1, AM_HAL_IOM_RAW);
	

}


static void drv_extFlash_enable(void)
{
	if (SPI_No_Semaphore == 0)
	{
    	xSemaphoreTake( Appolo_SPI_Semaphore, portMAX_DELAY ); 
	}
#ifndef WATCH_HAS_NO_BLE 
	if(SetValue.SwBle != false )
	{
		if (SPI_No_Semaphore == 0)
		{
			am_hal_gpio_pin_config(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT);
			while(!am_hal_gpio_input_bit_read(EXTFLASH_READY_PIN))
			{
				DELAY_MS(EXTFLASH_DELAY);
			}
		}
		am_hal_gpio_pin_config(EXTFLASH_READY_PIN,AM_HAL_GPIO_OUTPUT);	
		am_hal_gpio_out_bit_clear(EXTFLASH_READY_PIN);			
	}
#endif
	
	am_hal_iom_pwrctrl_enable(DRV_EXTFLASH_MOULDE);
	const am_hal_iom_config_t g_spi0_sIOMConfig =
	{
			.ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
			.ui32ClockFrequency = DRV_EXTFLASH_SPEED,
			.bSPHA = 0,
			.bSPOL = 0,
			.ui8WriteThreshold = 4,
			.ui8ReadThreshold = 60,
	};	
	am_hal_iom_config(DRV_EXTFLASH_MOULDE, &g_spi0_sIOMConfig);
	am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_CLK, AM_HAL_PIN_5_M0SCK);
	am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_MISO, AM_HAL_PIN_6_M0MISO);
	am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_MOSI, AM_HAL_PIN_7_M0MOSI);
	am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_CS, AM_HAL_PIN_45_M0nCE3);
	am_hal_iom_enable(DRV_EXTFLASH_MOULDE);	
}

static void drv_extFlash_disable(void)
{
	am_hal_iom_disable(DRV_EXTFLASH_MOULDE);	
	am_hal_iom_pwrctrl_disable(DRV_EXTFLASH_MOULDE);
	am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_CLK, AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_MISO, AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULLUP);
    am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_MOSI, AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_CS, AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULLUP);	
	//am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_CLK, AM_HAL_GPIO_3STATE);
	//am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_MISO, AM_HAL_GPIO_3STATE);
	//am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_MOSI, AM_HAL_GPIO_3STATE);
	//am_hal_gpio_pin_config(DRV_EXTFLASH_PIN_CS, AM_HAL_GPIO_3STATE);
	
	if (SPI_No_Semaphore == 0)
	{
#ifndef WATCH_HAS_NO_BLE 
	if(SetValue.SwBle != false )
	{
		am_hal_gpio_pin_config(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULLUP);
	}
#endif
	xSemaphoreGive( Appolo_SPI_Semaphore );	
	}
}


#define FLASH_PAGE_SIZE W25Q128_PAGE_SIZE
#define DEV_EXTFLASH_ERASE_TYPE_SECTOR                 1
#define DEV_EXTFLASH_ERASE_TYPE_32K                 	 2
#define DEV_EXTFLASH_ERASE_TYPE_64K                    3
#define DEV_EXTFLASH_ERASE_TYPE_CHIP                   4

static void buf_write( uint32_t addr, uint8_t* p_data, uint16_t len)
{
    uint8_t num_of_page = 0, num_of_single = 0, address = 0, count = 0, temp = 0;

    address = addr % FLASH_PAGE_SIZE;
    count = FLASH_PAGE_SIZE - address;
    num_of_page =  len / FLASH_PAGE_SIZE;
    num_of_single = len % FLASH_PAGE_SIZE;
    if (address == 0)
    {   
        if (num_of_page == 0) 
        {
            drv_extFlash_page_write(addr,p_data, len);
        }
        else 
        {
            while (num_of_page--)
            {
                drv_extFlash_page_write(addr,p_data, FLASH_PAGE_SIZE);
                addr +=  FLASH_PAGE_SIZE;
                p_data += FLASH_PAGE_SIZE;
            }
            drv_extFlash_page_write(addr,p_data, num_of_single);
        }
    }
    else 
    {
        if (num_of_page == 0)
        {
            if (num_of_single > count) 
            {
                temp = num_of_single - count;
                drv_extFlash_page_write(addr,p_data, count);
                addr +=  count;
                p_data += count;
                drv_extFlash_page_write(addr,p_data, temp);
            }
            else
            {
                drv_extFlash_page_write(addr,p_data, len);
            }
        }
        else 
        {
            len -= count;
            num_of_page =  len / FLASH_PAGE_SIZE;
            num_of_single = len % FLASH_PAGE_SIZE;

            drv_extFlash_page_write(addr,p_data, count);
            addr +=  count;
            p_data += count;

            while (num_of_page--)
            {
                drv_extFlash_page_write(addr,p_data, FLASH_PAGE_SIZE);
                addr +=  FLASH_PAGE_SIZE;
                p_data += FLASH_PAGE_SIZE;
            }

            if (num_of_single != 0)
            {
                drv_extFlash_page_write(addr,p_data, num_of_single);
            }
        }
    }
}



void dev_extFlash_write(uint32_t addr, uint8_t* p_data, uint16_t len)
{
	buf_write(  addr,  p_data,  len);
}



void dev_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len)
{
	drv_extFlash_read( addr,  p_data,  len);
}

void dev_extFlash_erase(uint32_t addr, uint32_t size)
{
	uint8_t erase_type = 0;
	uint32_t remain_size = size;
	uint32_t address = addr;
	
	if((remain_size%W25Q128_SECTOR_SIZE) != 0)
	{
		remain_size += W25Q128_SECTOR_SIZE - remain_size%W25Q128_SECTOR_SIZE;
	}
	
	while(true)
	{
		if (remain_size >= (64*1024) && ((address%W25Q128_64K_SIZE) == 0))
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_64K;
		}
		else if(remain_size >= (32*1024) && ((address%W25Q128_32K_SIZE) == 0))
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_32K;
		}
		else if(remain_size >= W25Q128_SECTOR_SIZE)
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_SECTOR;
		}		
	
		switch(erase_type)
		{
			case DEV_EXTFLASH_ERASE_TYPE_64K:{
				drv_extFlash_64K_erase(address);
				address += (64*1024);
				remain_size -= (64*1024);
			}break;
			case DEV_EXTFLASH_ERASE_TYPE_32K:{
				drv_extFlash_32K_erase(address);
				address += (32*1024);	
				remain_size -= (32*1024);
			}break;
			case DEV_EXTFLASH_ERASE_TYPE_SECTOR:{
				drv_extFlash_sector_erase(address);
				address += (4*1024);
				remain_size -= (4*1024);
			}break;			
		}
		if(remain_size == 0)
		{
			break;
		}
	}
}

void dev_extFlash_chip_erase(void)
{
	drv_extFlash_enable();
	drv_extFlash_pwr_up();
	drv_extFlash_chip_erase();
	drv_extFlash_pwr_down();
	drv_extFlash_disable();
}

bool dev_extFlash_IDverify(uint8_t times)
{
	uint32_t id , i;
	drv_extFlash_enable();
    drv_extFlash_pwr_up();
	for(i=0;i<times;i++)
	{
		id = drv_extFlash_ID_read();
		
		if(id == W25Q128_REG_ID_VAL)
		{
			break;
		}
	}
    drv_extFlash_pwr_down();
	drv_extFlash_disable();
	if(i == times)
	{
//		ERR_HANDLER(ERR_NOT_FOUND);
		return false;
	}
	return true;
}


void dev_extFlash_enable(void)
{
	drv_extFlash_enable();
	drv_extFlash_pwr_up();
}

void dev_extFlash_disable(void)
{
	drv_extFlash_pwr_down();
	drv_extFlash_disable();
}


void dev_extFlash_enter_deep_sleep(void)
{
	drv_extFlash_enable();
	drv_extFlash_pwr_down();
	drv_extFlash_disable();

}

void dev_extFlash_exit_deep_sleep(void)
{

	drv_extFlash_enable();
	drv_extFlash_pwr_up();
	drv_extFlash_disable();


}






