#include "drv_extFlash.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "com_apollo2.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"

#include "am_devices_spiflash.h"
 #include "am_util_delay.h"
#include <string.h>

//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
//#include "portmacro.h"
//#include "portable.h"
//#include "event_groups.h"
//#include "semphr.h"

#include "board.h"


#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DRV_EXTFALSH]:"

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

 
#define EXTFLASH_DELAY            10

//#define DELAY_US(X)                 am_util_delay_us(X)
#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

#ifdef LAYER_APPLICATION 
#include "task_ble.h"
extern _bluetooth_status g_bluetooth_status;
#endif


static uint8_t spiflash_status(void)
{
	am_hal_iom_buffer(1) psResponse; 
	am_hal_iom_spi_read(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
											BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
											psResponse.words, 1,
											AM_HAL_IOM_OFFSET(W25Q128_REG_RDSR));
	return psResponse.bytes[0];
}


void drv_extFlash_sector_erase(uint32_t addr)
{	
	DEBUG_PRINTF(MOUDLE_NAME"drv_extFlash_sector_erase\n");
	
    am_hal_iom_buffer(4) psCommand;

    //
    // Send the write-enable command to prepare the external flash for program
    // operations.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;
    am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                         BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
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
    am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                         BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
                         psCommand.words, 4, AM_HAL_IOM_RAW);

    //
    // Wait for status to indicate that the write is no longer in progress.
    //
    while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );	
	
	
	
}
void drv_extFlash_32K_erase(uint32_t addr)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_extFlash_32K_erase\n");
	
    am_hal_iom_buffer(4) psCommand;

    //
    // Send the write-enable command to prepare the external flash for program
    // operations.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;
    am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                         BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
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
    am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                         BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
                         psCommand.words, 4, AM_HAL_IOM_RAW);

    //
    // Wait for status to indicate that the write is no longer in progress.
    //
    while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );		
	
}
void drv_extFlash_64K_erase(uint32_t addr)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_extFlash_64K_erase\n");
	
    am_hal_iom_buffer(4) psCommand;

    //
    // Send the write-enable command to prepare the external flash for program
    // operations.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;
    am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                         BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
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
    am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                         BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
                         psCommand.words, 4, AM_HAL_IOM_RAW);

    //
    // Wait for status to indicate that the write is no longer in progress.
    //
    while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );		
	
	
}
void drv_extFlash_chip_erase(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_extFlash_chip_erase\n");
    am_hal_iom_buffer(1) psCommand;

    //
    // Send the write-enable command to prepare the external flash for program
    // operations.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_WREN;
    am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                         BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
                         psCommand.words, 1, AM_HAL_IOM_RAW);

    //
    // Wait for the write enable latch status bit.
    //
    while ( !(spiflash_status() & AM_DEVICES_SPIFLASH_WEL) );

    //
    // Send the bulk erase command.
    //
    psCommand.bytes[0] = AM_DEVICES_SPIFLASH_BE;
    am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                         BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
                         psCommand.words, 1, AM_HAL_IOM_RAW);

    //
    // Wait for status to indicate that the write is no longer in progress.
    //
    while ( spiflash_status() & AM_DEVICES_SPIFLASH_WIP );
}
void drv_extFlash_page_write( uint32_t addr, uint8_t * p_data, uint16_t len)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_extFlash_page_write\n");
	
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
        am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                             BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
                             psEnableCommand.words,
                             1, AM_HAL_IOM_RAW);

        while ( !(spiflash_status() & AM_DEVICES_SPIFLASH_WEL) );

        //
        // Send the write command.
        //
        am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                             BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
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
void drv_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_extFlash_read\n");
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
        am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                             BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
                             pui32WriteBuffer, 4,
                             AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

        am_hal_iom_spi_read(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
                            BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX, pui32ReadBuffer,
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




uint32_t drv_extFlash_ID_read(void)
{
	LOG_PRINTF(MOUDLE_NAME"drv_extFlash_ID_read\n");
	am_hal_iom_buffer(3) psResponse;
	am_hal_iom_spi_read(AM_HAL_IOM_SPIMODE,
											BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
											psResponse.words, 3,
											AM_HAL_IOM_OFFSET(W25Q128_REG_ID));

	LOG_HEXDUMP(psResponse.bytes, 3);
	

	
	return psResponse.words[0] & 0x00FFFFFF;
	
	
}
void drv_extFlash_pwr_down(void)
{
	LOG_PRINTF(MOUDLE_NAME"drv_extFlash_pwr_down\n");
	am_hal_iom_buffer(1) psEnableCommand;

	psEnableCommand.bytes[0] = W25Q128_REG_POWER_DOWN;	
	am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
											 BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
											 psEnableCommand.words,
											 1, AM_HAL_IOM_RAW);
	

}

void drv_extFlash_pwr_up(void)
{
	LOG_PRINTF(MOUDLE_NAME"drv_extFlash_pwr_up\n");
	am_hal_iom_buffer(1) psEnableCommand;

	psEnableCommand.bytes[0] = W25Q128_REG_RELEASE_POWER_DOWN;	
	am_hal_iom_spi_write(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX,
											 BSP_APOLLO2_EXTFLASH_CHIP_SELECT_INDEX,
											 psEnableCommand.words,
											 1, AM_HAL_IOM_RAW);
	

}


void drv_extFlash_enable(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_extFlash_enable\n");

	#ifdef LAYER_APPLICATION 
	if(g_bluetooth_status.is_on)
	{
		PIN_CONFIG(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT);
		while(!PIN_READ_INPUT(EXTFLASH_READY_PIN))
		{
			DELAY_MS(EXTFLASH_DELAY);
		}
		PIN_CONFIG(EXTFLASH_READY_PIN,AM_HAL_GPIO_OUTPUT);	
		PIN_CLEAR(EXTFLASH_READY_PIN);			
	}
	#endif
	
	am_hal_iom_pwrctrl_enable(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX);
	const am_hal_iom_config_t g_spi0_sIOMConfig =
	{
			.ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
			.ui32ClockFrequency = DRV_EXTFLASH_SPEED,
			.bSPHA = 0,
			.bSPOL = 0,
			.ui8WriteThreshold = 4,
			.ui8ReadThreshold = 60,
	};	
	am_hal_iom_config(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX, &g_spi0_sIOMConfig);

	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_CLK, BSP_APOLLO2_EXTFLASH_CLK_CONFIG);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_MISO, BSP_APOLLO2_EXTFLASH_MISO_CONFIG);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_MOSI, BSP_APOLLO2_EXTFLASH_MOSI_CONFIG);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_CS, BSP_APOLLO2_EXTFLASH_CS_CONFIG);
	am_hal_iom_enable(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX);	
}

void drv_extFlash_disable(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"drv_extFlash_disable\n");
	am_hal_iom_disable(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX);	
	am_hal_iom_pwrctrl_disable(BSP_APOLLO2_EXTFLASH_MOUDLE_INDEX);
	
	
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_CLK, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_MISO, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_MOSI, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	PIN_CONFIG(BSP_APOLLO2_EXTFLASH_CS, AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);	

	
	
	#ifdef LAYER_APPLICATION 
	if(g_bluetooth_status.is_on)
	{
		PIN_CONFIG(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULLUP);
	}
	#endif
}










