#include "gui_xt_ble_auth.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include "drv_extFlash.h"
#include <string.h>
#include "font_config.h"
#include "task_display.h"
#include "gui.h"
#include "com_data.h"
#include "img_tool.h"
#include "time_notify.h"
#include "gui_dial.h"
#include "gui_menu_config.h"
#include "lib_error.h"

#ifdef WATCH_HAS_XINGTANG_FUNC

#include "task_ble.h"

#define GUI_XT_BLE_AUTH_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_XT_BLE_AUTH_LOG_ENABLED == 1
	#define GUI_XT_BLE_AUTH_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_XT_BLE_AUTH_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_XT_BLE_AUTH_WRITESTRING(...)
	#define GUI_XT_BLE_AUTH_PRINTF(...)		        
#endif

#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK
#define BLE_CONN_COLOR        LCD_RED  
#define BLE_DISCONN_COLOR     LCD_YELLOW  

#define DISPLAY_SCREEN_XT_BLE_DEVICE1	   0
#define DISPLAY_SCREEN_XT_BLE_DEVICE2	   1
#define DISPLAY_SCREEN_XT_BLE_DEVICE3	   2
#define DISPLAY_SCREEN_XT_BLE_DEVICE4	   3
#define DISPLAY_SCREEN_XT_BLE_DEVICE5	   4
#define DISPLAY_SCREEN_XT_BLE_DEVICE6	   5
#define DISPLAY_SCREEN_XT_BLE_DEVICE7	   6
#define DISPLAY_SCREEN_XT_BLE_DEVICE8	   7
#define DISPLAY_SCREEN_XT_BLE_DEVICE9	   8
#define DISPLAY_SCREEN_XT_BLE_DEVICE10	   9
#define DISPLAY_SCREEN_XT_BLE_DEVICE11	   10
#define DISPLAY_SCREEN_XT_BLE_DEVICE12	   11
#define DISPLAY_SCREEN_XT_BLE_DEVICE13	   12
#define DISPLAY_SCREEN_XT_BLE_DEVICE14	   13
#define DISPLAY_SCREEN_XT_BLE_DEVICE15	   14
#define DISPLAY_SCREEN_XT_BLE_DEVICE16	   15
static uint8_t m_xt_ble_auth_index = DISPLAY_SCREEN_XT_BLE_DEVICE1;

#define DISPLAY_SCREEN_XT_BLE_DEVICE_OPTION1   1
#define DISPLAY_SCREEN_XT_BLE_DEVICE_OPTION2   2
#define DISPLAY_SCREEN_XT_BLE_DEVICE_OPTION3   3
#define DISPLAY_SCREEN_XT_BLE_DEVICE_OPTION4   4
static uint8_t m_xt_ble_option_index = DISPLAY_SCREEN_XT_BLE_DEVICE_OPTION1;


static char xt_ble_device_id[10];

extern _device_info g_device_info[NRF_SDH_BLE_CENTRAL_LINK_COUNT];


#define TIMER_INIT_INTERVAL_MS 			(1000)
#define OSTIMER_WAIT_FOR_QUEUE			(100)
#define	XT_SCREEN_DISPLAY_INTERVAL		(3)

static TimerHandle_t m_timer;
static uint32_t test_count = 0;
static void callback(TimerHandle_t xTimer)
{
	DISPLAY_MSG  msg = {0,0};
	ScreenState = DISPLAY_SCREEN_XT_BLE_AUTH;
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
    GUI_XT_BLE_AUTH_PRINTF("RTC_TIMER:%d,%d,%d",RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second);
    GUI_XT_BLE_AUTH_PRINTF("gui_xt_ble_auth_callback!%d\r\n",test_count++);
}

static void timer_xt_ble_auth_init(void)
{
    if(m_timer != NULL)
    {
        return;
    }
    GUI_XT_BLE_AUTH_PRINTF("timer_xt_ble_auth_init\n");
	m_timer = xTimerCreate("XTTIMER",
						 ( XT_SCREEN_DISPLAY_INTERVAL * TIMER_INIT_INTERVAL_MS / portTICK_PERIOD_MS ),
						 pdTRUE,
						 NULL,
						 callback);
	if (NULL == m_timer)
	{
		ERR_HANDLER(ERR_NO_MEM);
        GUI_XT_BLE_AUTH_PRINTF("[GUI_XT_BLE_AUTH]:xTimerCreate fail\n");
	}
	else
	{
		if(xTimerStart(m_timer, 100) != pdPASS)
		{
			ERR_HANDLER(ERR_NO_MEM);
		}
	}
}

static void timer_xt_ble_auth_uninit(void)
{
	if(m_timer != NULL)
	{
		if (pdPASS != xTimerDelete(m_timer, OSTIMER_WAIT_FOR_QUEUE))
		{
            ERR_HANDLER(ERR_NO_MEM);
			GUI_XT_BLE_AUTH_PRINTF("[GUI_XT_BLE_AUTH]:xTimerDelete fail\n");
		}
		else {
			m_timer = NULL;
		}
	}
}

static void timer_xt_ble_auth_start(uint32_t timeout_msec)
{
	if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		GUI_XT_BLE_AUTH_PRINTF("[GUI_XT_BLE_AUTH]:xTimerChangePeriod success\n");
	}
	else
	{
        ERR_HANDLER(ERR_NO_MEM);
		GUI_XT_BLE_AUTH_PRINTF("[GUI_XT_BLE_AUTH]:xTimerChangePeriod fail\n");
	}	
}

static void timer_xt_ble_auth_stop(void)
{
	if(m_timer != NULL)
	{
		if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
		{
            ERR_HANDLER(ERR_NO_MEM);
			GUI_XT_BLE_AUTH_PRINTF("[GUI_XT_BLE_AUTH]:xTimerStop fail\n");
		}
	}
}


//连接状态显示
static void xt_conn_state_circle_display(uint8_t x_axis, uint8_t y_axis, uint8_t device_id)
{
    uint16_t radius = 8;
    uint8_t conn_color = LCD_RED;       //连接时红色
    uint8_t disconn_color = LCD_YELLOW; //断开实黄色
    if(g_device_info[device_id].conn_state == 1){ //已连接圆点显示
        LCD_SetCircle(x_axis, y_axis, radius, conn_color, 2, LCD_FILL_ENABLE);
    }
    else{
        LCD_SetCircle(x_axis, y_axis, radius, disconn_color, 2, LCD_FILL_ENABLE);
    }
}

//蓝牙认证显示五栏
static void xt_ble_auth(uint8_t device_id)
{	
	SetWord_t word = {0};	
    
	//设备1
	word.x_axis = 10; 
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;
	word.kerning = 0;
	sprintf((char *)xt_ble_device_id, "设备%02x",g_device_info[(device_id+14)%16].addr[0]);
	LCD_DisplayGTString(xt_ble_device_id,&word);
    xt_conn_state_circle_display(word.x_axis+8,80,(device_id+14)%16);
	word.x_axis = 40;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,BACKGROUND_COLOR,0,LCD_FILL_ENABLE);

	
	//设备2
	word.x_axis = 50;
	word.size = LCD_FONT_16_SIZE;
	sprintf((char *)xt_ble_device_id, "设备%02x",g_device_info[(device_id+15)%16].addr[0]);
	LCD_DisplayGTString(xt_ble_device_id,&word);
    xt_conn_state_circle_display(word.x_axis+8,80,(device_id+15)%16);
	word.x_axis = 80;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,BACKGROUND_COLOR,0,LCD_FILL_ENABLE);	

	
	//设备3
	word.x_axis = 108;
	word.size = LCD_FONT_24_SIZE;
	sprintf((char *)xt_ble_device_id, "设备%02x",g_device_info[device_id].addr[0]);
	LCD_DisplayGTString(xt_ble_device_id,&word);
    xt_conn_state_circle_display(word.x_axis+12,70,device_id);
	word.x_axis = 160;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,BACKGROUND_COLOR,0,LCD_FILL_ENABLE);	

	
	//设备4
	word.x_axis = 170;
	word.size = LCD_FONT_16_SIZE;
	sprintf((char *)xt_ble_device_id, "设备%02x",g_device_info[(device_id+1)%16].addr[0]);
	LCD_DisplayGTString(xt_ble_device_id,&word);
    xt_conn_state_circle_display(word.x_axis+8,80,(device_id+1)%16);
	word.x_axis = 200;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,BACKGROUND_COLOR,0,LCD_FILL_ENABLE);	
	
	//设备5
	word.x_axis = 210;
	word.size = LCD_FONT_16_SIZE;
	sprintf((char *)xt_ble_device_id, "设备%02x",g_device_info[(device_id+2)%16].addr[0]);
	LCD_DisplayGTString(xt_ble_device_id,&word);
    xt_conn_state_circle_display(word.x_axis+8,80,(device_id+2)%16);
	
}

void gui_xt_ble_auth_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);
	xt_ble_auth(m_xt_ble_auth_index);
	timer_xt_ble_auth_init();	//进入界面上时才开始启动定时器，但是需要避免重读创建定时器
}

void gui_xt_ble_auth_btn_evt(uint32_t evt)
{
	DISPLAY_MSG  msg = {0,0};
	
	switch(evt)
	{		
		case (KEY_BACK):{
			timer_xt_ble_auth_uninit();
			m_xt_ble_auth_index = DISPLAY_SCREEN_XT_BLE_DEVICE1;
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
			timer_xt_ble_auth_uninit();
			if(m_xt_ble_auth_index>DISPLAY_SCREEN_XT_BLE_DEVICE1){
				m_xt_ble_auth_index--;
			}
			else{
				m_xt_ble_auth_index = DISPLAY_SCREEN_XT_BLE_DEVICE16;
			}
			ScreenState = DISPLAY_SCREEN_XT_BLE_AUTH;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
			
		case (KEY_OK):{
//			timer_xt_ble_auth_uninit();
//			ScreenState = DISPLAY_SCREEN_XT_DEVICE_OPTION;
//			msg.cmd = MSG_DISPLAY_SCREEN;
//			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;		
		case (KEY_DOWN):{
			timer_xt_ble_auth_uninit();
			if(m_xt_ble_auth_index < DISPLAY_SCREEN_XT_BLE_DEVICE16){
				m_xt_ble_auth_index++;
			}
			else{
				m_xt_ble_auth_index = DISPLAY_SCREEN_XT_BLE_DEVICE1;
			}
			ScreenState = DISPLAY_SCREEN_XT_BLE_AUTH;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;
		default:
		{

		}break;					
	}
}

void gui_xt_ble_device_option_paint(void)
{

}

void gui_xt_ble_device_option_btn_evt(uint32_t evt)
{
	DISPLAY_MSG  msg = {0,0};
	
	switch(evt)
	{		
		case (KEY_BACK):{
			m_xt_ble_option_index = DISPLAY_SCREEN_XT_BLE_DEVICE_OPTION1;
			ScreenState = DISPLAY_SCREEN_XT_BLE_AUTH;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;	

		case (KEY_UP):{
			if(m_xt_ble_option_index > DISPLAY_SCREEN_XT_BLE_DEVICE_OPTION1)
			{	
				m_xt_ble_option_index--;
			}
			ScreenState = DISPLAY_SCREEN_XT_DEVICE_OPTION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;	
			
		case (KEY_OK):{
			ScreenState = DISPLAY_SCREEN_XT_PINKEY;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	

		case (KEY_DOWN):{
			if(m_xt_ble_option_index < DISPLAY_SCREEN_XT_BLE_DEVICE_OPTION4)
			{
				m_xt_ble_option_index++;
			}			
			ScreenState = DISPLAY_SCREEN_XT_DEVICE_OPTION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;

		default:
		{
			
		}break;					
	}
}

void gui_xt_ble_pinkey_paint(void)
{

}

void gui_xt_ble_pinkey_btn_evt(uint32_t evt)
{

}

#endif
