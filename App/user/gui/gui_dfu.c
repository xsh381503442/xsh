#include "gui_dfu.h"
#include "bsp_rtc.h"
#include "drv_lcd.h"
#include "drv_key.h"
#include "task_tool.h"
#include "gui.h"
#include "gui_tool.h"
#include "font_config.h"
#include "com_data.h"
#include "cmd_ble.h"
#include "lib_error.h"
#include "drv_ble.h"
#include "task_ble.h"
#include "time_notify.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"




#define GUI_DFU_LOG_ENABLED   0
#if DEBUG_ENABLED == 1 && GUI_DFU_LOG_ENABLED == 1
	#define GUI_DFU_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_DFU_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_DFU_WRITESTRING(...)
	#define GUI_DFU_PRINTF(...)		        
#endif


//#define DELAY_MS(X)                 am_util_delay_ms(X)
#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / portTICK_PERIOD_MS );}while(0)



extern SetValueStr SetValue;

#define OSTIMER_WAIT_FOR_QUEUE           100
static TimerHandle_t m_timer; 
#define TIMER_INIT_INTERVAL_MS        1000
static bool m_timer_start = 0;
static uint8_t progress = 0;
#define DFU_INTERVAL       1000

static bool m_is_paint_busy = false;

static void callback(TimerHandle_t xTimer)
{
	GUI_DFU_PRINTF("[GUI_DFU]:callback\n");
	
	
	if(!m_is_paint_busy && m_timer_start)
	{
		if(progress >= 11)
		{
			progress = 0;
		}else{
			progress++;
		}
		
		#ifdef COD 
		if(ScreenState == DISPLAY_SCREEN_BLE_DFU || ScreenState ==  DISPLAY_SCREEN_UPDATE_BP)
		#else
		if(ScreenState == DISPLAY_SCREEN_BLE_DFU)
		#endif
		{
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
		
	}
	
	

}



void gui_dfu_timer_init(void)
{
	GUI_DFU_PRINTF("[GUI_DFU]:gui_dfu_timer_init\n");
	m_timer_start = 0;
	m_timer = xTimerCreate("DFU",
												 ( TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdTRUE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
		ERR_HANDLER(ERR_NO_MEM);
	}
}


void gui_dfu_timer_uninit(void)
{
	GUI_DFU_PRINTF("[GUI_DFU]:gui_dfu_timer_uninit\n");
	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
	}

}


void gui_dfu_timer_start(uint32_t timeout_msec)
{
	if(m_timer_start)
	{
		return;
	}
	m_timer_start = 1;
	
	GUI_DFU_PRINTF("[GUI_DFU]:gui_dfu_timer_start\n");
	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		GUI_DFU_PRINTF("[GUI_DFU]:xTimerChangePeriod success\n");
	}
	else
	{
		ERR_HANDLER(ERR_NO_MEM);
	}		
}


void gui_dfu_timer_stop(void)
{
	if(!m_timer_start)
	{
		return;
	}
	m_timer_start = 0;
	progress = 0;

	GUI_DFU_PRINTF("[GUI_DFU]:gui_dfu_timer_stop\n");	
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
	}
}

static void progress_display(uint8_t progress)
{

	uint16_t start_degree = 0;
	uint16_t end_degree = 0;
	
	for(uint8_t i=0;i<12;i++)
	{
		start_degree = i*30+1;
		end_degree = start_degree+28;
		if(progress == i)
		{
			GUI_DFU_PRINTF("[GUI_DFU]:progress : start_degree --> %d, end_degree --> %d\n",start_degree,end_degree);	
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 20, 
							start_degree, end_degree, LCD_YELLOW);
		}
		else
		{
			GUI_DFU_PRINTF("[GUI_DFU]:start_degree --> %d, end_degree --> %d\n",start_degree,end_degree);	
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 20, 
							start_degree, end_degree, LCD_GRAY);
		}
	}
}
extern am_hal_rtc_time_t RTC_time;

void gui_dfu_paint(void)
{
	static bool is_paint = false;
	

	m_is_paint_busy = true;
	
	SetWord_t word = {0};
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	

	LCD_SetBackgroundColor(LCD_BLACK);

	if(!is_paint)
	{
		word.x_axis = 120-2-24;
		LCD_SetString("正在升级",&word);	
		word.x_axis = 120+2;
		LCD_SetString("请勿关机",&word);	
	
	}
	

	

	am_hal_rtc_time_get(&RTC_time);
	char p_str[20];
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
	word.x_axis = 180;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	LCD_SetNumber(p_str,&word);		
	
	
	progress_display(progress);
	gui_dfu_timer_start(DFU_INTERVAL);
		
	
	m_is_paint_busy = false;
	
}
#ifdef COD 
void gui_update_cod_dial_paint(void)
{
	static bool is_paint = false;
	

	m_is_paint_busy = true;
	
	SetWord_t word = {0};
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	

	LCD_SetBackgroundColor(LCD_BLACK);

	if(!is_paint)
	{
		word.x_axis = 104;
		LCD_SetString("正在更换表盘",&word);	
		word.x_axis = 120;
		word.forecolor = LCD_RED;
		LCD_SetString("请勿关机",&word);	
	
	}
	

	

	am_hal_rtc_time_get(&RTC_time);
	char p_str[20];
	memset(p_str,0,sizeof(p_str));
	sprintf(p_str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
	word.x_axis = 180;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	LCD_SetNumber(p_str,&word);		
	
	
	progress_display(progress);
	gui_dfu_timer_start(DFU_INTERVAL);
		
	
	m_is_paint_busy = false;
	
}
#endif

void gui_dfu_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{		
		case (KEY_BACK):{	
		}break;			
		case (KEY_UP):{		
		}break;				
		case (KEY_OK):{
		}break;		
		case (KEY_DOWN):{		
		}break;					
	
	}
}




