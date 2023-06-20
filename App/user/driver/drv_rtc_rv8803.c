/**
*Module:RTC
*Chip:rv8803
*Version: V 0.0.1
*Time: 2020/10/23
**/
#include "drv_rtc_rv8803.h"
#include "algo_time_calibration.h"
#include "bsp.h"
#include "bsp_iom.h"
#include "bsp_rtc.h"
#include "com_data.h"
#include "task_gps.h"
#include "isr_uart.h"
#include "com_dial.h"
#include "task_sport.h"

#include "drv_extFlash.h"
extern SemaphoreHandle_t Appolo_I2C_Semaphore;

#define DRV_RTC_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && DRV_RTC_LOG_ENABLED == 1
	#define DRV_RTC_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_RTC_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define DRV_RTC_LOG_WRITESTRING(...)
	#define DRV_RTC_LOG_PRINTF(...)		        
#endif

#ifdef WATCH_RTC_RV8803

#define RV8803_SEC			0x00
#define RV8803_MIN			0x01
#define RV8803_HOUR			0x02
#define RV8803_WEEK			0x03
#define RV8803_DAY			0x04
#define RV8803_MONTH		0x05
#define RV8803_YEAR			0x06

#define RV8803_EXTENSION    0x0D
#define RV8803_FLAG    		0x0E
#define RV8803_CONTROL    	0x0F

#define RV8803_SUBSEC		0x10

#define USEL_MASK  0x20   // EXTENSION REG ;0 Second update, 1 Minute update
#define UF_MASK    0x20   // FLAG REG;Periodic Time Update Flag
#define UIE_MASK   0x20   // CONTROL REG;Periodic Time Update Interrupt Enable


/*BCD码转十进制*/
uint8_t bcd_to_hex(uint8_t data)
{
	uint8_t temp;
	temp = ((data>>4)*10 + (data&0x0f));
	return temp;
}
/*十进制转BCD码*/
uint8_t hex_to_bcd(uint32_t data)
{
	uint8_t temp;
	temp = (((data/10)<<4) + (data%10));
	return temp;
}


//rv8803星期格式转换为平台星期格式
uint32_t drv_rtc_rv8803_weekday_to_ui32Weekday(uint8_t weekday)
{
	uint32_t ui32Weekday;

	for (uint8_t i = 0;i < 7;i++)
	{
		if ((weekday >> i) & 0x01)
		{
			ui32Weekday = i;
			break;	
		}
	}
	
	//0-6:1-六 转 日- 六
	if(ui32Weekday < 6){
			ui32Weekday = ui32Weekday +1;//星期定义格式转换
		}
		else{
			ui32Weekday = 0;//星期天格式转换
		}
	return ui32Weekday;

}

//平台星期格式转换为rv8803星期格式
uint8_t drv_rtc_rv8803_ui32Weekday_to_weekday(uint32_t ui32Weekday)
{
	uint8_t weekday;
	//0-6:日- 六转 1-六
	if(ui32Weekday > 0){
		ui32Weekday = ui32Weekday - 1;//星期定义格式转换
	}
	else{
		ui32Weekday = 6;//星期天格式转换
	}

	weekday = 1 << ui32Weekday;
	
	return weekday;

}

const am_hal_iom_config_t g_rv8803_sIOMConfig =
{
	.ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
	.ui32ClockFrequency = AM_HAL_IOM_400KHZ,
	.bSPHA = 0,
	.bSPOL = 0,
	.ui8WriteThreshold = 4,
	.ui8ReadThreshold = 60,
};


void drv_rtc_i2c_enable(void)
{
	xSemaphoreTake( Appolo_I2C_Semaphore, portMAX_DELAY );
	bsp_iom_enable(BSP_RTC_RV8803_IOM,g_rv8803_sIOMConfig);
}

void drv_rtc_i2c_disable(void)
{

	bsp_iom_disable(BSP_RTC_RV8803_IOM);
	//DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_disable\n");
	xSemaphoreGive( Appolo_I2C_Semaphore );

}


/*
	\brief      时间获取,没有毫秒
	
*/
uint32_t int_count = 0;
uint32_t drv_rtc_rv8803_time_no_ms_get(am_hal_rtc_time_t *pTime)
{
	
	am_hal_iom_buffer(7) sResult = {0};
	am_hal_iom_buffer(1) subResult ={0};
	
	drv_rtc_i2c_enable();
	
	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, sResult.words, 7, AM_HAL_IOM_OFFSET(RV8803_SEC));

	//am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, subResult.words, 1, AM_HAL_IOM_OFFSET(RV8803_SUBSEC));
	
	drv_rtc_i2c_disable();

	pTime->ui32Year = bcd_to_hex(sResult.bytes[RV8803_YEAR]);
	pTime->ui32Month = bcd_to_hex(sResult.bytes[RV8803_MONTH]);
	pTime->ui32DayOfMonth = bcd_to_hex(sResult.bytes[RV8803_DAY]);
	pTime->ui32Hour = bcd_to_hex(sResult.bytes[RV8803_HOUR]);
	pTime->ui32Minute = bcd_to_hex(sResult.bytes[RV8803_MIN]);
	pTime->ui32Second = bcd_to_hex(sResult.bytes[RV8803_SEC]);
	
	pTime->ui32Hundredths =0;// bcd_to_hex(subResult.bytes[0]);
	pTime->ui32Weekday = drv_rtc_rv8803_weekday_to_ui32Weekday(sResult.bytes[RV8803_WEEK]);
	
	DRV_RTC_LOG_PRINTF("time get %d,%d,%d,%d,%d,%d\r\n",pTime->ui32Year,pTime->ui32Month,pTime->ui32DayOfMonth,pTime->ui32Hour,pTime->ui32Minute,pTime->ui32Second);
	DRV_RTC_LOG_PRINTF("weekday %d,%d\r\n",pTime->ui32Weekday,sResult.bytes[RV8803_WEEK]);
	return 0;
}

/*
	\brief      时间获取,有毫秒， hundredths of seconds,单位为10ms/unit
	
*/
uint32_t drv_rtc_rv8803_time_has_ms_get(am_hal_rtc_time_t *pTime)
{
	
	am_hal_iom_buffer(7) sResult = {0};
	am_hal_iom_buffer(1) subResult ={0};
	
	drv_rtc_i2c_enable();
	
	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, sResult.words, 7, AM_HAL_IOM_OFFSET(RV8803_SEC));

	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, subResult.words, 1, AM_HAL_IOM_OFFSET(RV8803_SUBSEC));
	
	drv_rtc_i2c_disable();

	pTime->ui32Year = bcd_to_hex(sResult.bytes[RV8803_YEAR]);
	pTime->ui32Month = bcd_to_hex(sResult.bytes[RV8803_MONTH]);
	pTime->ui32DayOfMonth = bcd_to_hex(sResult.bytes[RV8803_DAY]);
	pTime->ui32Hour = bcd_to_hex(sResult.bytes[RV8803_HOUR]);
	pTime->ui32Minute = bcd_to_hex(sResult.bytes[RV8803_MIN]);
	pTime->ui32Second = bcd_to_hex(sResult.bytes[RV8803_SEC]);
	
	pTime->ui32Hundredths =bcd_to_hex(subResult.bytes[0]);
	pTime->ui32Weekday = drv_rtc_rv8803_weekday_to_ui32Weekday(sResult.bytes[RV8803_WEEK]);
	
	DRV_RTC_LOG_PRINTF("time get %d,%d,%d,%d,%d,%d\r\n",pTime->ui32Year,pTime->ui32Month,pTime->ui32DayOfMonth,pTime->ui32Hour,pTime->ui32Minute,pTime->ui32Second);
	DRV_RTC_LOG_PRINTF("weekday %d,%d,sub %d\r\n",pTime->ui32Weekday,sResult.bytes[RV8803_WEEK],pTime->ui32Hundredths);
	return 0;
}


/*!
	\brief      时间设置
*/
void drv_rtc_rv8803_time_set(am_hal_rtc_time_t *pTime)
{
	
	am_hal_iom_buffer(7) sCammand = {0};
	
	
	sCammand.bytes[RV8803_SEC]   = hex_to_bcd(pTime->ui32Second);
	sCammand.bytes[RV8803_MIN]   = hex_to_bcd(pTime->ui32Minute);
	sCammand.bytes[RV8803_HOUR]  = hex_to_bcd(pTime->ui32Hour);
	sCammand.bytes[RV8803_WEEK]  = drv_rtc_rv8803_ui32Weekday_to_weekday(pTime->ui32Weekday);
	sCammand.bytes[RV8803_DAY]   = hex_to_bcd(pTime->ui32DayOfMonth);
	sCammand.bytes[RV8803_MONTH] = hex_to_bcd(pTime->ui32Month);
	sCammand.bytes[RV8803_YEAR]  = hex_to_bcd(pTime->ui32Year);
	DRV_RTC_LOG_PRINTF("set %d,%d,%d,%d,%d,%d\r\n",pTime->ui32Second,pTime->ui32Minute,pTime->ui32Hour,pTime->ui32DayOfMonth,pTime->ui32Month,pTime->ui32Year);
	DRV_RTC_LOG_PRINTF("weekday %d,%d\r\n",pTime->ui32Weekday,sCammand.bytes[RV8803_WEEK]);
	drv_rtc_i2c_enable();

	am_hal_iom_i2c_write(BSP_RTC_RV8803_IOM,RTC_RV8803_ADD,sCammand.words,7,AM_HAL_IOM_OFFSET(RV8803_SEC));
		
	drv_rtc_i2c_disable();
	
}


/*!
	\brief      时间设置,中断中使用 
*/
void drv_rtc_rv8803_interrupt_time_set(am_hal_rtc_time_t *pTime)
{
	am_hal_iom_buffer(7) sCammand = {0};
	
	
	sCammand.bytes[RV8803_SEC]   = hex_to_bcd(pTime->ui32Second);
	sCammand.bytes[RV8803_MIN]   = hex_to_bcd(pTime->ui32Minute);
	sCammand.bytes[RV8803_HOUR]  = hex_to_bcd(pTime->ui32Hour);
	sCammand.bytes[RV8803_WEEK]  = drv_rtc_rv8803_ui32Weekday_to_weekday(pTime->ui32Weekday);
	sCammand.bytes[RV8803_DAY]   = hex_to_bcd(pTime->ui32DayOfMonth);
	sCammand.bytes[RV8803_MONTH] = hex_to_bcd(pTime->ui32Month);
	sCammand.bytes[RV8803_YEAR]  = hex_to_bcd(pTime->ui32Year);
	DRV_RTC_LOG_PRINTF("set %d,%d,%d,%d,%d,%d\r\n",pTime->ui32Second,pTime->ui32Minute,pTime->ui32Hour,pTime->ui32DayOfMonth,pTime->ui32Month,pTime->ui32Year);
	DRV_RTC_LOG_PRINTF("weekday %d,%d\r\n",pTime->ui32Weekday,sCammand.bytes[RV8803_WEEK]);

	bsp_iom_enable(BSP_RTC_RV8803_IOM,g_rv8803_sIOMConfig);

	am_hal_iom_i2c_write(BSP_RTC_RV8803_IOM,RTC_RV8803_ADD,sCammand.words,7,AM_HAL_IOM_OFFSET(RV8803_SEC));
		
	bsp_iom_disable(BSP_RTC_RV8803_IOM);	
	
}


uint32_t drv_rtc_flag_reg_read(void)
{
	am_hal_iom_buffer(1) subResult ={0};
	am_hal_iom_status_e ret;
	drv_rtc_i2c_enable();

	ret = am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, subResult.words, 1, AM_HAL_IOM_OFFSET(RV8803_FLAG));
	
	drv_rtc_i2c_disable();
	DRV_RTC_LOG_PRINTF("drv_rtc_flag_reg_read %d\r\n",ret);
	if (ret ==AM_HAL_IOM_SUCCESS )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


static void rtc_int_handle(void)
{
   DISPLAY_MSG  msg = {0};	
	 static BaseType_t xHigherPriorityTaskWoken = false;	

	
      if(Get_TaskSportSta() == true)
       {
             if(TaskSportHandle != NULL)
             {
                  //活动开始的情况下,每秒钟发送信号量到sporttask
                  xSemaphoreGiveFromISR(ActSemaphore, &xHigherPriorityTaskWoken);
             }
            
              //在活动过程中，每分钟需要检测一次数据
              msg.cmd = MSG_UPDATE_TIME;  
              xQueueSendFromISR(DisplayQueue, &msg, &xHigherPriorityTaskWoken);
              
       }
	   else if(ScreenState != DISPLAY_SCREEN_LOGO)
		{
			xHigherPriorityTaskWoken = pdFALSE;		
			//更新时间
			msg.cmd = MSG_UPDATE_TIME;	
			xQueueSendFromISR(DisplayQueue, &msg, &xHigherPriorityTaskWoken);
				
		}	
        if( xHigherPriorityTaskWoken )
		{
			/* Actual macro used here is port specific. */
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
        


}

//中断初始化函数
void drv_rtc_irq_init(void)
{
	am_hal_gpio_int_register(BSP_GPIO_RTC_INT, rtc_int_handle);
	am_hal_gpio_pin_config(BSP_GPIO_RTC_INT,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_polarity_bit_set(BSP_GPIO_RTC_INT,AM_HAL_GPIO_FALLING);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(BSP_GPIO_RTC_INT));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(BSP_GPIO_RTC_INT));
	
}

/*
  @brief  Periodic Time Update Interrupt function.
  
   Valid values for ui32RepeatInterval:
   
   AM_HAL_RTC_ALM_RPT_MIN,One-Minute update time,
   AM_HAL_RTC_ALM_RPT_SEC,One-Second update time,

*/

void drv_rtv_periodic_time_int_set(uint32_t ui32RepeatInterval)
{
	uint8_t USEL_value;
	am_hal_iom_buffer(1) exensionResult ={0};
	am_hal_iom_buffer(1) flagResult ={0};
	am_hal_iom_buffer(1) controlResult ={0};
	am_hal_iom_buffer(1) sCammand = {0};

	if (ui32RepeatInterval == AM_HAL_RTC_ALM_RPT_MIN)
	{
		USEL_value = 0x20;
	}
	else
	{
		USEL_value = 0x00;
	}
	drv_rtc_i2c_enable();

	//1. Initialize bits UIE and UF to 0.
	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, controlResult.words, 1, AM_HAL_IOM_OFFSET(RV8803_CONTROL));
	sCammand.bytes[0] = controlResult.bytes[0] &( ~UIE_MASK);
	am_hal_iom_i2c_write(BSP_RTC_RV8803_IOM,RTC_RV8803_ADD,sCammand.words,1,AM_HAL_IOM_OFFSET(RV8803_CONTROL));
	
	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, flagResult.words, 1, AM_HAL_IOM_OFFSET(RV8803_FLAG));
	sCammand.bytes[0] = flagResult.bytes[0] &( ~UF_MASK);
	am_hal_iom_i2c_write(BSP_RTC_RV8803_IOM,RTC_RV8803_ADD,sCammand.words,1,AM_HAL_IOM_OFFSET(RV8803_FLAG));
	
	//2. Choose the timer source clock and write the corresponding value in the USEL bit.
	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, exensionResult.words, 1, AM_HAL_IOM_OFFSET(RV8803_EXTENSION));

	sCammand.bytes[0] = controlResult.bytes[0] &( ~USEL_MASK);
	sCammand.bytes[0] |= USEL_value;
	am_hal_iom_i2c_write(BSP_RTC_RV8803_IOM,RTC_RV8803_ADD,sCammand.words,1,AM_HAL_IOM_OFFSET(RV8803_EXTENSION));

	//3.Set the UIE bit to 1 to enable the Periodic Time Update Interrupt function
	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, controlResult.words, 1, AM_HAL_IOM_OFFSET(RV8803_CONTROL));

	sCammand.bytes[0] = controlResult.bytes[0] | UIE_MASK;
	am_hal_iom_i2c_write(BSP_RTC_RV8803_IOM,RTC_RV8803_ADD,sCammand.words,1,AM_HAL_IOM_OFFSET(RV8803_CONTROL));
	
	drv_rtc_i2c_disable();

	
}

#ifdef WATCH_RV8803_CALIBRATION
void rv8803_cailbration_apollo_rtc(void)
{
	am_hal_rtc_time_t pTime;	
	am_hal_iom_buffer(7) sResult = {0};
	am_hal_iom_buffer(1) subResult ={0};
	
	drv_rtc_i2c_enable();
	
	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, sResult.words, 7, AM_HAL_IOM_OFFSET(RV8803_SEC));

	am_hal_iom_i2c_read(BSP_RTC_RV8803_IOM, RTC_RV8803_ADD, subResult.words, 1, AM_HAL_IOM_OFFSET(RV8803_SUBSEC));
	
	pTime.ui32Year = bcd_to_hex(sResult.bytes[RV8803_YEAR]);
	pTime.ui32Month = bcd_to_hex(sResult.bytes[RV8803_MONTH]);
	pTime.ui32DayOfMonth = bcd_to_hex(sResult.bytes[RV8803_DAY]);
	pTime.ui32Hour = bcd_to_hex(sResult.bytes[RV8803_HOUR]);
	pTime.ui32Minute = bcd_to_hex(sResult.bytes[RV8803_MIN]);
	pTime.ui32Second = bcd_to_hex(sResult.bytes[RV8803_SEC]);
	
	pTime.ui32Hundredths =bcd_to_hex(subResult.bytes[0]);
	pTime.ui32Weekday = drv_rtc_rv8803_weekday_to_ui32Weekday(sResult.bytes[RV8803_WEEK]);
	pTime.ui32Century =0;
	
	am_hal_rtc_time_set(&pTime);
	
	drv_rtc_i2c_disable();

	DRV_RTC_LOG_PRINTF("rv8803_cailbration_apollo_rtc\r\n");

}
#endif
#endif



