#include "drv_config.h"
#include "drv_gt24l24.h"

#include "bsp.h"
#include "bsp_iom.h"
#include "drv_lcd.h"
#include "am_util_delay.h"
#include "HFMA2Ylib.h"

#include "drv_extFlash.h"
#include "task_ble.h"
#include "com_data.h"

#if DEBUG_ENABLED == 1 && DRV_GT24L24_LOG_ENABLED == 1
	#define DRV_GT24L24_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_GT24L24_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define DRV_GT24L24_WRITESTRING(...)
	#define DRV_GT24L24_PRINTF(...)		        
#endif


//#define DELAY_US(X)                 am_util_delay_us(X)
#define DELAY_MS(X)                 am_util_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)

extern SetValueStr SetValue;
extern SemaphoreHandle_t Appolo_SPI_Semaphore;

//*****************************************************************************
//
// Configuration structure for the IO Master.
//
//*****************************************************************************
const am_hal_iom_config_t g_sGT24L24_IOMConfig =
{
    .ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
    .ui32ClockFrequency = AM_HAL_IOM_8MHZ,
    .bSPHA = 0,
    .bSPOL = 0,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
};

void drv_gt24l24_enable(void)
{
	DRV_GT24L24_PRINTF("[drv_gt24L24]: drv_gt24l24_enable\n");
	xSemaphoreTake( Appolo_SPI_Semaphore, portMAX_DELAY );
#ifndef WATCH_HAS_NO_BLE 
	if(SetValue.SwBle != false )
	{
		am_hal_gpio_pin_config(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT);
		while(!am_hal_gpio_input_bit_read(EXTFLASH_READY_PIN))
		{
			DELAY_MS(EXTFLASH_DELAY);
		}
		am_hal_gpio_pin_config(EXTFLASH_READY_PIN,AM_HAL_GPIO_OUTPUT);	
		am_hal_gpio_out_bit_clear(EXTFLASH_READY_PIN);			
	}	
#endif
	
	
	// Enable IOM
	bsp_iom_enable(BSP_GT24L24_IOM,g_sGT24L24_IOMConfig);
		
	// Enable GT24L24 chip select
	am_hal_gpio_pin_config(BSP_GPIO_GT24L24_CE,BSP_GPIO_CFG_GT24L24_CE);
}

void drv_gt24l24_disable(void)
{
	DRV_GT24L24_PRINTF("[drv_gt24L24]: drv_gt24l24_disable\n");
	
	// Disable GT24L24 chip select
	//am_hal_gpio_pin_config(BSP_GPIO_GT24L24_CE,AM_HAL_GPIO_3STATE);
	am_hal_gpio_pin_config(BSP_GPIO_GT24L24_CE,AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULLUP);
	// Disable IOM
	bsp_iom_disable(BSP_GT24L24_IOM);
#ifndef WATCH_HAS_NO_BLE 
	if(SetValue.SwBle != false )
	{
		am_hal_gpio_pin_config(EXTFLASH_READY_PIN,AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULLUP);
	}	
#endif
	xSemaphoreGive( Appolo_SPI_Semaphore );	
}



void drv_gt24l24_sleep(void)
{
	am_hal_iom_buffer(1) sData;

	//Enter deep power down mode
	sData.bytes[0] = DRV_GT24L24_SLEEP;
	
	am_hal_iom_spi_write(BSP_GT24L24_IOM, BSP_GT24L24_CS,
                             sData.words, 1, AM_HAL_IOM_RAW);
}

void drv_gt24l24_wakeup(void)
{
	am_hal_iom_buffer(1) sData;

	//Wake up
	sData.bytes[0] = DRV_GT24L24_WAKEUP;
	
	am_hal_iom_spi_write(BSP_GT24L24_IOM, BSP_GT24L24_CS,
                             sData.words, 1, AM_HAL_IOM_RAW);

}


void dev_gt24l24_sleep(void)
{
	am_hal_iom_buffer(1) sData;

	//Enter deep power down mode
	sData.bytes[0] = DRV_GT24L24_SLEEP;
	
	drv_gt24l24_enable();
	am_hal_iom_spi_write(BSP_GT24L24_IOM, BSP_GT24L24_CS,
                             sData.words, 1, AM_HAL_IOM_RAW);
	drv_gt24l24_disable();
	
	am_util_delay_us(25);
}

void dev_gt24l24_wakeup(void)
{
	am_hal_iom_buffer(1) sData;

	//Wake up
	sData.bytes[0] = DRV_GT24L24_WAKEUP;
	
	drv_gt24l24_enable();
	am_hal_iom_spi_write(BSP_GT24L24_IOM, BSP_GT24L24_CS,
                             sData.words, 1, AM_HAL_IOM_RAW);
	drv_gt24l24_disable();
	
	am_util_delay_us(25);
}

bool drv_gt24l24_self_test(void)
{
	const char i_buf[5] = {0x3E,0x51,0x49,0x45,0x3E};
	uint8_t o_buf[5] = {0};
	
	//读取5X7的'0'的数据
	drv_gt24l24_enable();
	
	am_hal_iom_buffer(1) sData;
	sData.bytes[0] = DRV_GT24L24_WAKEUP;
	am_hal_iom_spi_write(BSP_GT24L24_IOM, BSP_GT24L24_CS,
                             sData.words, 1, AM_HAL_IOM_RAW);	
	
	ASCII_GetData(0x30,ASCII_5X7,o_buf);

	sData.bytes[0] = DRV_GT24L24_SLEEP;
	am_hal_iom_spi_write(BSP_GT24L24_IOM, BSP_GT24L24_CS,
                             sData.words, 1, AM_HAL_IOM_RAW);	
	drv_gt24l24_disable();
	
	if(memcmp(o_buf,i_buf,sizeof(o_buf)) != 0)
	{	
		return 1;
	}
	return 0;
}

//条件putdata的数据写入X，Y的LCD显示缓冲RAM中
void drv_lcdram_map( uint8_t *getdate,uint8_t *putdata,uint8_t high,uint8_t width,uint8_t style)
{
	uint8_t i,j,hbyte,wbyte;
	uint8_t i_8,j_8;
	
	wbyte = (width+7)/8;
	hbyte = (high+7)/8;
	
	//--------------------------------------------------------------------------
	// Y--> W; Y-->X; Y-->Z;
	//--------------------------------------------------------------------------
	if( style == HZ_MODE4 )	//竖置横排 转 横置横排 ( Y-->W )
	{
		for( i = 0; i < high; i++ )
			for( j = 0; j < width; j++ )
			{
				i_8 = i/8;
				if((*(putdata+i_8*width+j)&(0x01<<(i%8))) > 0)
					getdate[wbyte*i+j/8] |= (0x80>>(j%8));
				else
					getdate[wbyte*i+j/8] &= (~(0x80>>(j%8)));
			}
	}
	
	//--------------------------------------------------------------------------
	// W--> Y;  W-->Z;W-->X;
	//--------------------------------------------------------------------------
	if( style == HZ_MODE00 )	//竖置横排 (W--> Y)
	{
		for( i = 0; i < high; i++ )
			for( j = 0; j < width; j++ )
			{
				i_8 = i/8;
				if((*(putdata+wbyte*i+j/8)&(0x80>>(j%8))) > 0)
					getdate[i_8*width+j] |= (0x01<<(i%8));
				else
					getdate[i_8*width+j] &= (~(0x01<<(i%8)));
			}
	}
	else if(style == HZ_MODE01)	//竖置竖排 (W-->Z)
	{
		for( i = 0; i < high; i++ )
			for( j = 0; j < width; j++ )
			{
				if((*(putdata+wbyte*i+j/8)&(0x80>>(j%8))) > 0)
					getdate[j*hbyte+i/8] |= (0x01<<(i%8));
				else
					getdate[j*hbyte+i/8]  &= (~(0x01<<(i%8)));
			}
	}
	else if(style == HZ_MODE11)//横置竖排 (W-->X)
	{
		for( i = 0; i < high; i++ )
			for( j = 0; j < width; j++ )
			{
				j_8 = j/8;
				if((*(putdata+wbyte*i+j/8)&(0x80>>(j%8))) > 0)
					getdate[j_8*high+i] |= (0x80>>(j%8));
				else
					getdate[j_8*high+i] &= (~(0x80>>(j%8)));
			}
	}
	else if(style == HZ_MODE10)//横置横排 做镜像(W-->W')
	{
		for( i = 0; i < high; i++ )
			for( j = 0; j < width; j++ )
			{
				if((*(putdata+wbyte*i+j/8)&(0x80>>(j%8)))>0 )
					*(getdate+wbyte*i+(width-j)/8) |=(0x80>>((width-j)%8));
				else
					*(getdate+wbyte*i+(width-j)/8) &=~(0x80>>((width-j)%8));
			}
	}
}

//此函数为gt24l24中库中函数，读取相应地址的数据
void r_dat_bat(unsigned long TAB_addr,unsigned int Num,unsigned char *p_arr)
{
	uint16_t i = 0;
	uint32_t pui32WriteBuffer[1];
    uint8_t *pui8WritePtr;

    uint32_t pui32ReadBuffer[32];
    uint8_t *pui8ReadPtr;

    pui8WritePtr = (uint8_t *)(&pui32WriteBuffer);
    pui8ReadPtr = (uint8_t *)(&pui32ReadBuffer);
	
	pui8WritePtr[0] = DRV_GT24L24_Read;
	pui8WritePtr[1] = (TAB_addr&0xFF0000)>>16;
	pui8WritePtr[2] = (TAB_addr&0xFF00)>>8;
	pui8WritePtr[3] = (TAB_addr&0xFF);
	
	am_hal_iom_spi_write(BSP_GT24L24_IOM, BSP_GT24L24_CS,
                             pui32WriteBuffer, 4, AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);
	
	am_hal_iom_spi_read(BSP_GT24L24_IOM, BSP_GT24L24_CS,
							pui32ReadBuffer, Num, AM_HAL_IOM_RAW);
	
	for(i=0;i<Num;i++)
	{
		p_arr[i] = pui8ReadPtr[i];
	}
}

void drv_gt24l24_gb_getdata(unsigned char charset, unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf)
{
	switch(size)
	{
		case 16:
			//GB18030 宋体
			drv_gt24l24_enable();
			drv_gt24l24_wakeup();
			hzbmp16(charset,code,codeex,size,buf);
			drv_gt24l24_sleep();
			drv_gt24l24_disable();
			break;
		case 24:
			//GB2312 黑体
			drv_gt24l24_enable();
			drv_gt24l24_wakeup();
			hzbmp24(charset,code,codeex,size,buf);
			drv_gt24l24_sleep();
			drv_gt24l24_disable();
			break;
		default:
			break;
	}
}

void drv_gt24l24_ascii_getdata(unsigned char ASCIICode,unsigned long ascii_kind,unsigned char *DZ_Data)
{
	drv_gt24l24_enable();
	drv_gt24l24_wakeup();
	ASCII_GetData(ASCIICode,ascii_kind,DZ_Data);
	drv_gt24l24_sleep();
	drv_gt24l24_disable();
}
