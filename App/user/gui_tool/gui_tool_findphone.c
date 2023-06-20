#include "gui_tool_findphone.h"
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

#define GUI_TOOL_FINDPHONDE_LOG_ENABLED   1
#if DEBUG_ENABLED == 1 && GUI_TOOL_FINDPHONDE_LOG_ENABLED == 1
	#define GUI_TOOL_FINDPHONDE_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_FINDPHONDE_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_FINDPHONDE_WRITESTRING(...)
	#define GUI_TOOL_FINDPHONDE_PRINTF(...)		        
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
#define FIND_PHONE_PROGRESS_INTERVAL       1000
#define FIND_PHONE_PROGRESS_TIMEOUT_2_MIN  (120)
static uint32_t m_progress_timeout = 0;
static bool m_is_paint_busy = false;
static void callback(TimerHandle_t xTimer)
{
	GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:callback\n");
	
	m_progress_timeout++;
	
	if(m_progress_timeout >= FIND_PHONE_PROGRESS_TIMEOUT_2_MIN)
	{
		m_progress_timeout = 0;
		gui_tool_findphone_timer_stop();
		
		//挂起工具任务
		tool_task_close();
		
		//返回原界面
		timer_notify_rollback(DISPLAY_SCREEN_FINDPHONE, DISPLAY_SCREEN_TOOL);
		
		return;
	}


	if(!m_is_paint_busy)
	{
		if(progress >= 11)
		{
			progress = 0;
		}else{
			progress++;
		}
		
		if(ScreenState == DISPLAY_SCREEN_FINDPHONE)
		{
			DISPLAY_MSG  msg = {0,0};
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
		}
	
	}		
	
	
}




void gui_tool_findphone_timer_init(void)
{
	GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:gui_tool_findphone_timer_init\n");
	m_timer_start = 0;
	m_timer = xTimerCreate("FDPHONE",
												 ( TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS ),
												 pdTRUE,
												 NULL,
												 callback);
	if (NULL == m_timer)
	{
		ERR_HANDLER(ERR_NO_MEM);
	}
}



void gui_tool_findphone_timer_uninit(void)
{
	GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:gui_tool_findphone_timer_uninit\n");
	if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
		ERR_HANDLER(ERR_NO_MEM);
	}

}

void gui_tool_findphone_timer_start(uint32_t timeout_msec)
{
	if(m_timer_start)
	{
		return;
	}
	m_timer_start = 1;
	
	uint32_t command[2] = {BLE_UART_CCOMMAND_APPLICATION_FIND_PHONE | BLE_UART_CCOMMAND_MASK_APPLICATION, FIND_PHONE_PARAM_START};
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
	GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:gui_tool_findphone_timer_start\n");
	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:xTimerChangePeriod success\n");
	}
	else
	{
		ERR_HANDLER(ERR_NO_MEM);
	}		
}


void gui_tool_findphone_timer_stop(void)
{
	if(!m_timer_start)
	{
		return;
	}
	m_timer_start = 0;
	progress = 0;
	m_progress_timeout = 0;
	GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:gui_tool_findphone_timer_stop\n");	
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
			GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:progress : start_degree --> %d, end_degree --> %d\n",start_degree,end_degree);	
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 20, 
							start_degree, end_degree, LCD_YELLOW);
		}
		else
		{
			GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:start_degree --> %d, end_degree --> %d\n",start_degree,end_degree);	
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 20, 
							start_degree, end_degree, LCD_GRAY);
		}
	}
}


void gui_tool_findphone_paint(void)
{
	m_is_paint_busy = true;
	
	SetWord_t word = {0};
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	

	LCD_SetBackgroundColor(LCD_BLACK);

	word.x_axis = 40;
	#if defined (WATCH_TOOL_COUNTDOWN)
	LCD_SetString((char *)ToolStrs[SetValue.Language][6],&word);
	#else
	LCD_SetString((char *)ToolStrs[SetValue.Language][5],&word);
	#endif
	

	//LCD_SetPicture(78,74,LCD_NONE,LCD_NONE,&Img_findphone_92x84);	

	if(SetValue.SwBle > BLE_DISCONNECT)
	{
		GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:ble is on\n");	
		progress_display(progress);
		gui_tool_findphone_timer_start(FIND_PHONE_PROGRESS_INTERVAL);
		
	}	
	else{
		GUI_TOOL_FINDPHONDE_PRINTF("[GUI_TOOL_FINDPHONDE]:ble is off\n");	
		gui_tool_findphone_timer_stop();
		word.x_axis = 180;
		LCD_SetString((char *)FindphoneStrs[SetValue.Language][1],&word);	
	}
	
	m_is_paint_busy = false;	
}


void gui_tool_findphone_btn_evt(uint32_t Key_Value)
{
	switch(Key_Value)
	{
		case (KEY_BACK):{
			gui_tool_findphone_timer_stop();
			if(SetValue.SwBle > BLE_DISCONNECT)
			{
				uint32_t command[2] = {BLE_UART_CCOMMAND_APPLICATION_FIND_PHONE | BLE_UART_CCOMMAND_MASK_APPLICATION, FIND_PHONE_PARAM_END};
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
			
			//挂起工具任务
			tool_task_close();
	
			DISPLAY_MSG  msg = {0,0};
		
			ScreenState = DISPLAY_SCREEN_TOOL;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;
		default:
			break;
	}
}

