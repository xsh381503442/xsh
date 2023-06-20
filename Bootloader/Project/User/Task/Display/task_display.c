#include "task_display.h"

#include "drv_lcd.h"
#include "gui_boot.h"
#include "gui_post_watch_info.h"
#include "gui_exception.h"

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "SEGGER_RTT.h"
#include "gui_back_door.h"


#define TASK_DISPLAY_LOG_ENABLED    1
#if DEBUG_ENABLED == 1 && TASK_DISPLAY_LOG_ENABLED == 1
	#define TASK_DISPLAY_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_DISPLAY_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_DISPLAY_WRITESTRING(...)
	#define TASK_DISPLAY_PRINTF(...)		        
#endif

#define TaskDisplay_Priority       5
#define TaskDisplay_StackDepth      512*2


static TaskHandle_t	TaskDisplayHandle = NULL;
QueueHandle_t 	DisplayQueue;
ScreenState_t   ScreenState;
ScreenState_t   ScreenStateSave;





static void DisplayScreen(ScreenState_t index)
{

	switch(ScreenState)
	{
		case DISPLAY_SCREEN_BOOTLOADER:
			gui_boot_paint();
			break;
		case DISPLAY_SCREEN_POST_WATCH_INFO:
			gui_post_watch_info_paint();
		break;	
		case DISPLAY_SCREEN_EXCEPTION:
			gui_exception_paint();
		break;	
		case DISPLAY_SCREEN_BACK_DOOR:
			gui_back_door_paint();
		break;	
		
		default:
			break;
	}
}


static void TaskDisplay(void* pvParameter)
{
	static DISPLAY_MSG msg = {MSG_DEFAULT,0};

	while(1)
	{
		if (xQueueReceive(DisplayQueue, &msg, portMAX_DELAY) == pdTRUE)
    {
			switch( msg.cmd ) 
			{					
				case MSG_DISPLAY_SCREEN:
					//刷新整屏界面,用于更改界面
					DisplayScreen((ScreenState_t)msg.value);
					LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
					break;

					
					
				default:
					break;
			}
		}
	}
}

void task_display_init(void)
{
	if( TaskDisplayHandle == NULL )
	{
	  xTaskCreate(TaskDisplay, "Task Display",TaskDisplay_StackDepth, 0, TaskDisplay_Priority, &TaskDisplayHandle); 
		DisplayQueue = xQueueCreate( 5,sizeof( DISPLAY_MSG ));
	}
	
}


