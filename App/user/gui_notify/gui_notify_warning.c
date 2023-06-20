#include "gui_notify_warning.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "task_ble.h"
#include "lib_boot_setting.h"
#include "lib_boot.h"
#include "img_tool.h"
#include "lib_app_data.h"
#include "com_sport.h"
#include "drv_extflash.h"
#include "time_notify.h"
#include "cmd_ble.h"
#include "drv_ble.h"
#include "lib_error.h"
#include "task.h"
#include "drv_light.h"
#include "img_home.h"

#define DELAY_MS(X)                 vTaskDelay(X / portTICK_PERIOD_MS)
#define OSTIMER_WAIT_FOR_QUEUE           100
static TimerHandle_t m_timer; 
#define TIMER_INIT_INTERVAL_MS        1000
static bool m_timer_start = 0;
static uint8_t progress = 0;
#define FIND_WATCH_PROGRESS_INTERVAL       1000
static bool m_is_paint_busy = false;


static void gui_notify_warning_index_watch_info_ble_off(void)
{
#if defined WATCH_COM_SETTING
	SetWord_t word = {0};
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString((char *)NotifyWarningStrs[SetValue.Language][0],&word);//"提示"
	
	word.x_axis = LCD_PIXEL_ONELINE_MAX/2 - 8 - 24;
	LCD_SetString((char *)NotifyWarningStrs[SetValue.Language][1],&word);//"请先开启蓝牙"
	word.x_axis = LCD_PIXEL_ONELINE_MAX/2 + 8;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)NotifyWarningStrs[SetValue.Language][2],&word);//"系统>蓝牙"
#else
	SetWord_t word = {0};
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_RED;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("请打开蓝牙",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("TURN BLE ON",&word);								
	}
#endif
}
static void gui_notify_warning_index_accessory_ble_off(void)
{
#if  defined WATCH_COM_SETTING
	SetWord_t word = {0};
	word.x_axis = 30;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;
	LCD_SetString((char *)NotifyWarningStrs[SetValue.Language][0],&word);//"提示"
	
	word.x_axis = LCD_PIXEL_ONELINE_MAX/2 - 8 - 24;
	LCD_SetString((char *)NotifyWarningStrs[SetValue.Language][1],&word);//"请先开启蓝牙"
	word.x_axis = LCD_PIXEL_ONELINE_MAX/2 + 8;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)NotifyWarningStrs[SetValue.Language][2],&word);//"系统>蓝牙"
#else
	SetWord_t word = {0};
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_RED;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	if(SetValue.Language == L_CHINESE)
	{
		LCD_SetString("请打开蓝牙",&word);						
	}
	else if(SetValue.Language == L_ENGLISH)
	{
		LCD_SetString("TURN BLE ON",&word);								
	}
#endif
}


void gui_notify_warning_paint(uint8_t index)
{	
#if  defined WATCH_COM_SETTING
	LCD_SetBackgroundColor(LCD_WHITE);
#else
	LCD_SetBackgroundColor(LCD_BLACK);
#endif

	switch(index)
	{
		case GUI_NOTIFY_WARNING_INDEX_WATCH_INFO_BLE_OFF:{
			gui_notify_warning_index_watch_info_ble_off();
			
		}break;
		case GUI_NOTIFY_WARNING_INDEX_ACCESSORY_BLE_OFF:{
			gui_notify_warning_index_accessory_ble_off();
			
		}break;			
	}	
}



void gui_notify_warning_btn_evt(uint32_t evt)
{
	
	switch(evt)
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

static void callback(TimerHandle_t xTimer)
{
	if(!m_is_paint_busy)
	{
		if(progress >= 11)
		{
			progress = 0;
		}else{
			progress++;
		}	
	}	

	DISPLAY_MSG  msg = {0,0};
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
}

void gui_notify_findwatch_timer_init(void)
{	
	m_timer_start = 0;
	m_timer = xTimerCreate("FDWATCH",
							 ( TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS ),
							 pdTRUE,
							 NULL,
							 callback);
	if (NULL == m_timer)
	{
		ERR_HANDLER(ERR_NO_MEM);
	}
}



void gui_notify_findwatch_timer_uninit(void)
{	
	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
	}

}

void gui_notify_findwatch_timer_start(uint32_t timeout_msec)
{
	if(m_timer_start)
	{
		return;
	}
	m_timer_start = 1;	
	
	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		
	}
	else
	{
		ERR_HANDLER(ERR_NO_MEM);
	}		
}

void gui_notify_findwatch_timer_stop(void)
{
	if(!m_timer_start)
	{
		return;
	}
	m_timer_start = 0;
	progress = 0;
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
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 20, 
							start_degree, end_degree, LCD_CYAN);
		}
		else
		{
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 20, 
							start_degree, end_degree, LCD_GRAY);
		}
	}
}

//寻找手表界面
void gui_notify_find_watch_piant(void)
{
	m_is_paint_busy = true;
	SetWord_t word = {0};
	LCD_SetBackgroundColor(LCD_BLACK);
		
	word.x_axis = 108;	
	word.y_axis = 60;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString((char *)FindwatchStrs[SetValue.Language][0],&word);//	"寻找腕表中..."
	
	word.x_axis += 26;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	LCD_SetString((char *)FindwatchStrs[SetValue.Language][1],&word);//"按OK键确认"

	progress_display(progress);
	gui_notify_findwatch_timer_start(FIND_WATCH_PROGRESS_INTERVAL);

	m_is_paint_busy = false;
	
}
//寻找手表按键处理
void gui_notify_find_watch_btn_evt(uint32_t evt)
{	
	switch(evt)
	{		
		case (KEY_BACK):{
		
		}break;			
		case (KEY_UP):{

		}break;	
			
		case (KEY_OK):{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
			gui_notify_findwatch_response();
			drv_light_disable();
			gui_notify_findwatch_timer_stop();
		}break;		
		case (KEY_DOWN):{
			
		}break;					
	
	}
}


void gui_notify_findwatch_response(void)
{	
	uint32_t command[2] = {BLE_UART_RESPONSE_APPLICATION_FIND_WATCH | BLE_UART_CCOMMAND_MASK_APPLICATION, FIND_WATCH_PARAM_FOUND};
	_uart_data_param param;
	param.user_callback = NULL;
	param.p_data = (uint8_t *)command;
	param.length = sizeof(command);
	uint8_t count = TASK_UART_DELAY_COUNT_OUT;
#ifndef WATCH_HAS_NO_BLE 
	while(drv_ble_send_data(&param) != ERR_SUCCESS && count--)
	{
		DELAY_MS(TASK_UART_DELAY);
	}		
#endif
}


