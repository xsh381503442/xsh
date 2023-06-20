#include "drv_hrt.h"
#include "bsp.h"
#include "am_mcu_apollo.h"
//#include "drv_i2c_hw.h"
#include "si117x_functions.h"
#include "si117x_config.h"
#include "am_util_delay.h"
#ifdef OHRLIB_PRESENT	
#include "Ohrdriver.h"
#endif
//#include "rtt_log.h"
//#include "drv_nrf52_uart.h"
#include "task_display.h"


#define MOUDLE_INFO_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					  0
#define MOUDLE_NAME                     "[DRV_HRT]:"

#if DEBUG_ENABLED == 1 && MOUDLE_INFO_ENABLED == 1
#define INFO_PRINTF(...)                  SEGGER_RTT_printf(0,##__VA_ARGS__)    
#define INFO_HEXDUMP(p_data, len)         SEGGER_RTT_Write(0, (const char*)p_data, len);       
#else
#define INFO_PRINTF(...) 
#define INFO_HEXDUMP(p_data, len)             
#endif

#if DEBUG_ENABLED == 1 && MOUDLE_DEBUG_ENABLED == 1
#define DEBUG_PRINTF(...)                SEGGER_RTT_printf(0,##__VA_ARGS__)     
#define DEBUG_HEXDUMP(p_data, len)       SEGGER_RTT_Write(0, (const char*)p_data, len);    
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif

#ifdef COD
#define OHRLOG
extern uint8_t cod_sport_cast_flag;
#endif

extern uint8_t rtc_hw_flag;

static drv_hrt_callback m_callback;
static uint8_t irq_flag = 0;
extern uint32_t real_time_log_flag;


uint8_t drv_getHdrValue(void)
{
#ifdef HRT_DEBUG_TEST

	uint8_t data;
	if (OHRL_data.heartrate == 0)
	{
		data = OHRL_data.onwrist;
	}
	else
	{
		data = OHRL_data.heartrate;
	}
	return data;
#else	
	return OHRL_data.heartrate;
#endif 
}

void drv_clearHdrValue(void)
{
    OHRL_data.heartrate = 0;

}

void drv_getHeartrateVersion(char strVersion[],uint8_t length)
{

}

bool drv_receivePacket(void)
{
	return 0;
}

#ifdef WATCH_COD_BSP

static void drv_hrt_irq_init(void);
#ifdef OHRLOG
 uint8_t ohrlogbuff[32*20+1];
#endif




void drv_hrt_init(drv_hrt_callback callback)
{
	m_callback = callback;
	
	am_hal_gpio_pin_config(GPIO_PIN_NUM_HRT_5V_ENABLE, AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_clear(GPIO_PIN_NUM_HRT_5V_ENABLE);
	am_hal_gpio_pin_config(GPIO_PIN_NUM_HRT_3V3_ENABLE, AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_clear(GPIO_PIN_NUM_HRT_3V3_ENABLE);	
	
	
	drv_hrt_all_auto_test();
	
	drv_hrt_irq_init();	
	
}

#define tab_encrypt 0xF6000
void drv_hrt_enable(void)
{
	drv_hrt_3v3_enable();
	am_util_delay_ms(50);
	drv_i2c_hw1_enable();
	
	uint8_t current = 0;  //电流0 是正常心率测量，电流51是心率模块数据测试
	uint8_t error = 0;

	if ((ScreenState == DISPLAY_SCREEN_POST_HDR)|| (ScreenState == DISPLAY_SCREEN_POST_HDR_SNR))
	{
		current = 51;
	}
	//urrent = 20;
	//Feed_liscense(tab_encrypt);
#ifdef OHRLOG
	error = OHRL_init((uint32_t *)tab_encrypt , current, ohrlogbuff);
#else
	error = OHRL_init((uint32_t *)tab_encrypt, current, NULL);
#endif	

	if (rtc_hw_flag == 1)
	{
		Initial_Pedometer(3);
	}
	else
	{
		Initial_Pedometer(0);
	}
	
	if(OHRL_ENCRYPT_CHECK_PASS == error)
	{
		INFO_PRINTF(MOUDLE_NAME"OHRL_init success=========================\r\n");
	}
	else
	{
		INFO_PRINTF(MOUDLE_NAME"OHRL_init fail\r\n");
	}

	int16_t ret = Si117x_Start(OHRL_GetLEDcurrent(),IsHeartSportMode());
	if(ret ==0) 
	{ 
		//启动成功
		//打开LED电源， 启动成功后再打开LED电源！
		INFO_PRINTF(MOUDLE_NAME"ohrl success!!!\r\n");
		drv_hrt_5v_enable();
		drv_hrt_irq_enable();		
		INFO_PRINTF(MOUDLE_NAME"drv_hrt_irq_enable finish...\r\n");
	} 
	else
	{ 
		//启动失败 
		INFO_PRINTF(MOUDLE_NAME"ohrl fail --> %d\r\n",ret);
	}		
	drv_i2c_hw1_disable();

}

void drv_hrt_disable(void)
{
	drv_hrt_irq_disable();
	drv_i2c_hw1_enable();
	Si117x_Stop();
	drv_i2c_hw1_disable();
	drv_hrt_5v_disable();
	drv_hrt_3v3_disable();
}
void drv_hrt_5v_enable(void)
{
	am_hal_gpio_out_bit_set(GPIO_PIN_NUM_HRT_5V_ENABLE);

}

void drv_hrt_5v_disable(void)
{
	am_hal_gpio_out_bit_clear(GPIO_PIN_NUM_HRT_5V_ENABLE);

}
void drv_hrt_3v3_enable(void)
{
	am_hal_gpio_out_bit_set(GPIO_PIN_NUM_HRT_3V3_ENABLE);

}

void drv_hrt_3v3_disable(void)
{
	am_hal_gpio_out_bit_clear(GPIO_PIN_NUM_HRT_3V3_ENABLE);

}

void drv_hrt_suspend(void)
{
		DEBUG_PRINTF(MOUDLE_NAME"PPG SUSPEND\n");
		drv_i2c_hw1_enable();
		Si117x_Stop();
		drv_i2c_hw1_disable();
		drv_hrt_5v_disable();
}

void drv_hrt_restart(void)
{
	DEBUG_PRINTF(MOUDLE_NAME"PPG RESTART\n");
	drv_i2c_hw1_enable();
	int16_t ret = Si117x_Start(OHRL_GetLEDcurrent(),IsHeartSportMode());
	if(ret ==0) 
	{ 
		//启动成功
		//打开LED电源， 启动成功后再打开LED电源！
		INFO_PRINTF(MOUDLE_NAME"ohrl success!!!\r\n");
		drv_hrt_5v_enable();
		drv_hrt_irq_enable();		
		INFO_PRINTF(MOUDLE_NAME"drv_hrt_irq_enable finish...\r\n");
	} 
	else
	{ 
		//启动失败 
		INFO_PRINTF(MOUDLE_NAME"ohrl fail --> %d\r\n",ret);
	}		
	drv_i2c_hw1_disable();	
}


#ifdef OHRLOG
static void sendData(uint8_t *data, uint8_t len)
{
#ifndef COD	
	uint8_t chksum = len,i;
	uint8_t sdata;
	uint8_t temp[50];

	temp[0] = 0xFC;
	temp[1] =  len;
	
	for(i=0;i<len;i++)
	{
		sdata = *data++;
		temp[2+i] = sdata;
		chksum += sdata;
	}
	temp[2+len] = (~chksum+1);	
	drv_nrf52_send((uint8_t *)temp, len+3);
#endif
}

static uint8_t index = 0;
uint8_t hrt_buf[150];
uint8_t index_nums = 0;
static uint8_t Log_sendout(uint8_t * logbuf)
{
#ifdef COD 
#if 0
	uint8_t index_nums = 0;
	memset(&hrt_buf,0,sizeof(hrt_buf));
	while(index!=logbuf[0] && index_nums <12)
	{
	    memcpy(&hrt_buf[index_nums *20],&logbuf[index*20+1],20);
		index_nums++;
		 if(++index==32)
		 {
			index=0;
		 }
	}
	INFO_PRINTF("Log_sendout %d\r\n",index_nums);	
	ble_uart_response_application_real_time_log_trains(&hrt_buf[0], index_nums *20);
	
	return(0);
#else
	if(index!=logbuf[0])
	{
	     memcpy(&hrt_buf[index_nums *20],&logbuf[index*20+1],20);
		 index_nums++;
		 if(++index==32)
		 {
			index=0;
		 }
		 if ((index_nums >=6) || (index==logbuf[0]))
		 {
		 	//vTaskDelay(10);
			ble_uart_response_application_real_time_log_trains(&hrt_buf[0], index_nums *20);
			index_nums = 0;
			
		 }
		 return 1;
		 
	}
	return(0);

#endif
#else

	if(index!=logbuf[0])
	{
		
	    sendData(&logbuf[index*20+1], 20); //send raw data to nrf52
		 if(++index==32)
		 {
			index=0;
		 }
		 return 1;
		 
	}
	return(0);
#endif

}	
#endif


void drv_hrt_int_handler(void)
{
#ifdef OHRLOG
	//if(index!=ohrlogbuff[0])
	//{
	//  INFO_PRINTF("log is not empty!!!\r\n");	
	//}
#endif
	//drv_i2c_hw1_enable();
	HeartRateMonitor_interrupt_handle();	
	//drv_i2c_hw1_disable();	
  #ifdef OHRLOG	
  	if (real_time_log_flag)
  	{
  		INFO_PRINTF("log is not empty11\r\n");	
		#ifdef HRT_DEBUG_TEST
  		if (cod_sport_cast_flag ||Get_TaskSportSta())
		#else
		if (cod_sport_cast_flag)
		#endif
  		{
  			INFO_PRINTF("log is not empty222\r\n");	
			index_nums = 0;
 			while(Log_sendout(ohrlogbuff)==1);
  		}
  	}
  #endif	
}



static void hrt_int_handle(void)
{
	//DEBUG_PRINTF(MOUDLE_NAME"hrt_int_handle\r\n");
	if(m_callback && irq_flag)
	{
	#ifdef HRT_DEBUG_TEST
		extern uint32_t hrt_int_count_callback;
		hrt_int_count_callback++;
	#endif
		m_callback(HRT_EVT_PPG_INTERRUPT, true, NULL, 0);
	}
}
static void drv_hrt_irq_init(void)
{
	am_hal_gpio_int_register(GPIO_PIN_NUM_HRT_INT, hrt_int_handle);
	am_hal_gpio_pin_config(GPIO_PIN_NUM_HRT_INT,AM_HAL_GPIO_INPUT|AM_HAL_GPIO_PULLUP);
	am_hal_gpio_int_polarity_bit_set(GPIO_PIN_NUM_HRT_INT,AM_HAL_GPIO_FALLING);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(GPIO_PIN_NUM_HRT_INT));

}


void drv_hrt_irq_disable(void)
{
	irq_flag = 0;
	am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(GPIO_PIN_NUM_HRT_INT));

}
void drv_hrt_irq_enable(void)
{
	
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(GPIO_PIN_NUM_HRT_INT));
	irq_flag = 1;
}


uint32_t drv_hrt_all_auto_test(void)
{
	uint32_t error = 0;
	drv_hrt_3v3_enable();
	am_util_delay_ms(50);	
	drv_i2c_hw1_enable();
	int16_t ret = Si117x_checkstart();
	if(ret != 0)
	{
		error = 1;
	}	
	drv_i2c_hw1_disable();		
	drv_hrt_3v3_disable();
	
	return error;
}

#endif

