#include "timer_ble_searching.h"

#include "timer_config.h"
#include "task_display.h"
#include "timer_communication.h"
#include "task_ble.h"


#if DEBUG_ENABLED == 1 && TIMER_BLE_SEARCHING_LOG_ENABLED == 1
	#define TIMER_BLE_SEARCHING_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TIMER_BLE_SEARCHING_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TIMER_BLE_SEARCHING_WRITESTRING(...)
	#define TIMER_BLE_SEARCHING_PRINTF(...)       
#endif

#define BLE_SEARCHING_TIME			15 * 1000			//√Î

static TimerHandle_t BLE_Searching_Timer = NULL;		//SOS timer

static void vTimer_BLE_Searching_Callback(TimerHandle_t xTimer)
{
	DISPLAY_MSG  msg = {0,0};
		
	switch(ScreenState)
	{
		case DISPLAY_SCREEN_BEIDOU_MACHINE_SEARCHING:{
			ScreenState = DISPLAY_SCREEN_BEIDOU_MACHINE_RESULT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, 100);				
		}break;
		case DISPLAY_SCREEN_BEIDOU_MACHINE_CONNECTING:{
			timer_waiting_uninit();
			ScreenState = DISPLAY_SCREEN_BEIDOU_MACHINE_CONN_RESUALT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, 100);							
		}break;
		default:{
			//
		}break;
	}
	//SEGGER_RTT_printf(0,"ble_searching_timer callback...\n");	
}


void ble_searching_timer_start(void)
{		
	if (BLE_Searching_Timer == NULL)
	{
		BLE_Searching_Timer = xTimerCreate("BLE_Searching_Timer", BLE_SEARCHING_TIME / portTICK_PERIOD_MS,
								pdTRUE,
								NULL,
								vTimer_BLE_Searching_Callback);
		
		if(xTimerStart(BLE_Searching_Timer, 100) != pdPASS)
		{
			TIMER_BLE_SEARCHING_PRINTF("[TIMER_SOS]:SOSTimer success\n");
		}
	}
	else
	{
		if(xTimerStart(BLE_Searching_Timer, 100) != pdPASS)
		{
			TIMER_BLE_SEARCHING_PRINTF("[TIMER_SOS]:SOSTimer success\n");
		}
	}
}



void ble_searching_timer_delete(void)
{
	if(BLE_Searching_Timer != NULL)
	{
		xTimerDelete(BLE_Searching_Timer, 100);
		BLE_Searching_Timer = NULL;
	}
}



