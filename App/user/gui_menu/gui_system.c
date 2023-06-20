#include "SEGGER_RTT.h"
#include "gui_system.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "font_displaystrs.h"
#include "task_display.h"
#include "com_data.h"
#include "img_sport.h"
#include "img_menu.h"
#include "time_progress.h"
#include "task_ble.h"
#include "gui_accessory.h"
#include "drv_lsm6dsl.h"
#include "font_config.h"
#include "time_notify.h"
#include "gui_sport.h"





#define GUI_SYSTEM_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_SYSTEM_LOG_ENABLED == 1
	#define GUI_SYSTEM_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SYSTEM_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SYSTEM_WRITESTRING(...)
	#define GUI_SYSTEM_PRINTF(...)		        
#endif

extern SetValueStr SetValue;
#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK

//勿扰状态索引
static uint8_t m_gui_silent_index = 0;
static uint8_t m_gui_silent_time_index = 0;

_time_set silent_time_set;

//抬手亮屏索引
static uint8_t m_gui_autolight_index = 0;
static uint8_t m_gui_autolight_time_index = 0;

_time_set autolight_time_set;


#define GUI_SYSTEM_TIME_START_HOUR  	0		//开始小时
#define GUI_SYSTEM_TIME_START_MIN 		1		//开始分钟
#define GUI_SYSTEM_TIME_END_HOUR		2		//结束小时
#define GUI_SYSTEM_TIME_END_MIN 		3		//结束分钟



#define GUI_SYSTEM_BLUETOOTH                  			 0
#define GUI_SYSTEM_BASE_HEARTRATE                  	     1
#define GUI_SYSTEM_AUTO_SCREENLOCK                       2
#define GUI_SYSTEM_LANGUGAGE                  			 3
#define GUI_SYSTEM_BACKLIGHT                  			 4
#define GUI_SYSTEM_SOUND                 				 5
#define GUI_SYSTEM_SILENT                 				 6
#define GUI_SYSTEM_AUTO_BACKLIGHT                        7


static uint8_t m_gui_system_index = 0;

_base_heartrate m_base_heartrate;



#define GUI_SYSTEM_BASE_HEARTRATE_SWITCH                0
#define GUI_SYSTEM_BASE_HEARTRATE_HOUR 	                1
#define GUI_SYSTEM_BASE_HEARTRATE_MINUTE                2
static uint8_t m_gui_system_base_heartrate_index = 0;


#define GUI_SYSTEM_LANGUAGE_CHINESE                   0
#define GUI_SYSTEM_LANGUAGE_ENGLISH 	                1
static uint8_t m_gui_system_language_index = 0;


#define GUI_SYSTEM_BACKLIGHT_10_SECOND     0
#define GUI_SYSTEM_BACKLIGHT_30_SECOND     1
#define GUI_SYSTEM_BACKLIGHT_60_SECOND     2
static uint8_t m_gui_system_backlight_index = 0;

#define GUI_SYSTEM_SOUND_SOUND_N_VIBERATE          0
#define GUI_SYSTEM_SOUND_SOUND                     1
#define GUI_SYSTEM_SOUND_VIBERATE                  2
static uint8_t m_gui_system_sound_index = 0;





extern _ble_accessory g_ble_accessory;

void system_silent_mode_set(void)
{
//	SetValue.SwVibration = 0;
//	SetValue.SwBuzzer = 0;
}

void system_silent_mode_clear(void)
{
//	SetValue.SwVibration = 1;
//	SetValue.SwBuzzer = 1;
}

uint8_t is_silent_time(void)
{

	uint8_t ret =0;

	if ((SetValue.NoDisturbStartHour*60 + SetValue.NoDisturbStartMinute) <=(SetValue.NoDisturbEndHour *60 +SetValue.NoDisturbEndMinute) )
	{
		if (((RTC_time.ui32Hour*60 +RTC_time.ui32Minute ) >= (SetValue.NoDisturbStartHour*60 + SetValue.NoDisturbStartMinute)) 
			&& (((RTC_time.ui32Hour*60 +RTC_time.ui32Minute )<=(SetValue.NoDisturbEndHour*60 + SetValue.NoDisturbEndMinute))))
		{
			ret = 1;
		}	

	}
	else
	{
		if (((RTC_time.ui32Hour*60 +RTC_time.ui32Minute ) >= (SetValue.NoDisturbStartHour*60 + SetValue.NoDisturbStartMinute)) 
			|| (((RTC_time.ui32Hour*60 +RTC_time.ui32Minute )<=(SetValue.NoDisturbEndHour*60 + SetValue.NoDisturbEndMinute))))
		{
			ret = 1;
		}
	}

 return ret;

}

uint8_t system_silent_mode_get(void)
{
	if ((SetValue.SwNoDisturb ==1) && (is_silent_time() ==1) )
		
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t is_raiselight_time(void)
{

	uint8_t ret =0;

	if ((SetValue.AutoLightStartHour*60 + SetValue.AutoLightStartMinute) <=(SetValue.AutoLightEndHour *60 +SetValue.AutoLightEndMinute) )
	{
		if (((RTC_time.ui32Hour*60 +RTC_time.ui32Minute ) >= (SetValue.AutoLightStartHour*60 + SetValue.AutoLightStartMinute)) 
			&& (((RTC_time.ui32Hour*60 +RTC_time.ui32Minute )<=(SetValue.AutoLightEndHour*60 + SetValue.AutoLightEndMinute))))
		{
			ret = 1;
		}	

	}
	else
	{
		if (((RTC_time.ui32Hour*60 +RTC_time.ui32Minute ) >= (SetValue.AutoLightStartHour*60 + SetValue.AutoLightStartMinute)) 
			|| (((RTC_time.ui32Hour*60 +RTC_time.ui32Minute )<=(SetValue.AutoLightEndHour*60 + SetValue.AutoLightEndMinute))))
		{
			ret = 1;
		}
	}

 return ret;

}

uint8_t system_raiselight_mode_get(void)
{
	if ((SetValue.SwRaiseForLightScreen ==1) && (is_raiselight_time() ==1) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void menu( char* menu1, char* menu2, char* menu3, char* option, const LCD_ImgStruct *pic)
{
	SetWord_t word = {0};
	word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
    LCD_SetString(menu1,&word);   //更改使用字库字体  
    /*if(m_gui_system_index==GUI_SYSTEM_BLUETOOTH)
    	{
        LCD_SetPicture(120 - 24/2 - 36 - 24 - 16/2 -16/2,120 - LCD_FONT_16_SIZE - 4 - 16 ,LCD_RED,LCD_NONE,&img_set_menu_l[2]);
    	}*/
   
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

	
	
		


	


	
	if(option != NULL)
	{

	    word.x_axis = 120 - (24+8+16)/2;
        word.y_axis = LCD_CENTER_JUSTIFIED;
        word.size = LCD_FONT_24_SIZE;
        word.forecolor = LCD_WHITE;
        word.bckgrndcolor = LCD_NONE;
	    LCD_SetString(menu2,&word);	//更改使用字库字体
		word.x_axis += 24+8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		if((m_gui_system_index==GUI_SYSTEM_SILENT)||(m_gui_system_index==GUI_SYSTEM_AUTO_BACKLIGHT))
			{
			word.forecolor = LCD_WHITE;
           }
		else
			{
		word.forecolor = LCD_LIGHTGRAY;
			}
		
        word.bckgrndcolor = LCD_NONE;
		word.size = LCD_FONT_16_SIZE;		
		LCD_SetString(option,&word);	


			
	}
	else
		{

            word.x_axis = 120 - 24/2;
            word.y_axis = LCD_CENTER_JUSTIFIED;
            word.size = LCD_FONT_24_SIZE;
            word.forecolor = LCD_WHITE;
             word.bckgrndcolor = LCD_NONE;
	       LCD_SetString(menu2,&word);	//更改使用字库字体


	    }
	if(pic != NULL)
	{
		LCD_SetPicture(103, 200, LCD_NONE, LCD_NONE, pic);				
	}	
	

	
	word.x_axis = 120 + 24/2 + 36 + 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_LIGHTGRAY;
    word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
    LCD_SetString(menu3,&word);	//更改使用字库字体	

}

static void menu_system( char* menu1, char* menu2, char* menu3)
{
	SetWord_t word = {0};
	word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	/*if(m_gui_system_backlight_index==GUI_SYSTEM_BACKLIGHT_10_SECOND)
		{
	    word.forecolor = LCD_CYAN;
		}*/
	//else
		//{
      	 word.forecolor = LCD_LIGHTGRAY;
	  //  }
	word.bckgrndcolor = LCD_NONE;
	LCD_DisplayGTString(menu1,&word);	//更改使用字库字体
	
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);


	word.x_axis = 120 - 24/2;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
	LCD_DisplayGTString(menu2,&word);	//更改使用字库字体
	
		
	
	word.x_axis = 120 + 24/2 + 36 + 24;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.forecolor = LCD_LIGHTGRAY;
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString(menu3,&word);	//更改使用字库字体			

}

void gui_system_paint(void)
{
	char str[20]= {0};
	SetWord_t word = {0};
	LCD_SetBackgroundColor(LCD_BLACK);

	switch(m_gui_system_index)
	{

		case GUI_SYSTEM_BLUETOOTH:{

				if(SetValue.SwBle > BLE_DISCONNECT)
				{
				
					/*menu((char *)SysHintStrs[SetValue.Language][2],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],(char *)SysHintStrs[SetValue.Language][0],&Img_Slide_Switch_On_18X44);*/
				menu((char *)SystemStrs[SetValue.Language][GUI_SYSTEM_INDEX_MAX-1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],(char *)SysHintStrs[SetValue.Language][0],&img_v_switch_on);	
				}
				else if(SetValue.SwBle == BLE_DISCONNECT)
				{
					menu((char *)SystemStrs[SetValue.Language][GUI_SYSTEM_INDEX_MAX-1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],(char *)SysHintStrs[SetValue.Language][1],&img_v_switch_on);	
				}
				else{
					menu((char *)SysHintStrs[SetValue.Language][2],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],(char *)SysHintStrs[SetValue.Language][4],&img_v_switch_off);				
				}

				
						
		}break;

        
		case GUI_SYSTEM_BASE_HEARTRATE:{
			if(SetValue.Language == L_CHINESE)
			{
				menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
							(char *)SystemStrs[SetValue.Language][m_gui_system_index],
							(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],NULL,NULL);					
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
							(char *)SystemStrs[SetValue.Language][m_gui_system_index],
							(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],NULL,NULL);					
			}							
		}break;				
		case GUI_SYSTEM_AUTO_SCREENLOCK:{
			if(SetValue.Language == L_CHINESE)
			{
				if(SetValue.SwAutoLockScreen == 1)
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"开启",&img_v_switch_on);				
				}
				else{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"关闭",&img_v_switch_off);				
				}
			
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				if(SetValue.SwAutoLockScreen == 1)
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"ON",&img_v_switch_on);				
				}
				else{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"OFF",&img_v_switch_off);				
				}				
			}							
		}break;	
		case GUI_SYSTEM_LANGUGAGE:{
			if(SetValue.Language == L_CHINESE)
			{
					
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"中文",NULL);		
				
			}
			else if(SetValue.Language == L_ENGLISH)
			{

					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"ENGLISH",NULL);
				
			}
							
		}break;
		case GUI_SYSTEM_BACKLIGHT:{
			if(SetValue.Language == L_CHINESE)
			{
				char p_str[10];
				memset(p_str,0,sizeof(p_str));
				sprintf(p_str,"%d秒",(SetValue.backlight_timeout_mssec/1000));

				menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],p_str,NULL);						
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				char p_str[10];
				memset(p_str,0,sizeof(p_str));

				sprintf(p_str,"%dSEC",(SetValue.backlight_timeout_mssec/1000));

				menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],p_str,NULL);				
			}							
		}break;				
		case GUI_SYSTEM_SOUND:{
			if(SetValue.Language == L_CHINESE)
			{
				if((SetValue.SwBuzzer == 1) && (SetValue.SwVibration == 0))
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"声音",NULL);					
				}
				else if((SetValue.SwBuzzer == 0) && (SetValue.SwVibration == 1))
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"震动",NULL);					
				}
				else if((SetValue.SwBuzzer == 1) && (SetValue.SwVibration == 1))
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"声音加震动",NULL);				
				}
				else if((SetValue.SwBuzzer == 0) && (SetValue.SwVibration == 0))
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"无声音震动",NULL);
				}
			}
			else if(SetValue.Language == L_ENGLISH)
			{
				if((SetValue.SwBuzzer == 1) && (SetValue.SwVibration == 0))
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"SOUND",NULL);					
				}
				else if((SetValue.SwBuzzer == 0) && (SetValue.SwVibration == 1))
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"VIBERATE",NULL);					
				}
				else if((SetValue.SwBuzzer == 1) && (SetValue.SwVibration == 1))
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"SOUND AND VIBERATE",NULL);				
				}
			  else if((SetValue.SwBuzzer == 0) && (SetValue.SwVibration == 0))
				{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],"NO SOUND AND VIBERATE",NULL);
				}
			}							
		}break;			
		case GUI_SYSTEM_SILENT:{
			
				if(SetValue.SwNoDisturb == 1)
				{
					memset(str,0,sizeof(str));
					sprintf(str,"%02d:%02d-%02d:%02d", SetValue.NoDisturbStartHour, SetValue.NoDisturbStartMinute, SetValue.NoDisturbEndHour,SetValue.NoDisturbEndMinute);
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],str,NULL);
					
					
					/*word.forecolor = LCD_WHITE;
					word.bckgrndcolor = LCD_NONE;
					word.x_axis = 120 + 24/2 +4;
					word.y_axis = LCD_CENTER_JUSTIFIED;
					word.size = LCD_FONT_16_SIZE;
					word.kerning= 1;
					LCD_SetNumber(str,&word);	*/	
				}
				else{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index + 1],(char *)SysHintStrs[SetValue.Language][4],NULL);				
				}			
								
		}break;				
		case GUI_SYSTEM_AUTO_BACKLIGHT:{

		
				if(SetValue.SwRaiseForLightScreen == 1)
				{
				    	memset(str,0,sizeof(str));
					sprintf(str,"%02d:%02d-%02d:%02d", SetValue.AutoLightStartHour, SetValue.AutoLightStartMinute, SetValue.AutoLightEndHour,SetValue.AutoLightEndMinute);
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][0],str,NULL);

				
					/*word.forecolor = LCD_WHITE;
					word.bckgrndcolor = LCD_NONE;
					word.x_axis = 120 + 24/2 +4;
					word.y_axis = LCD_CENTER_JUSTIFIED;
					word.size = LCD_FONT_16_SIZE;
					word.kerning= 1;
					LCD_SetNumber(str,&word);*/	
				}
				else{
					menu((char *)SystemStrs[SetValue.Language][m_gui_system_index - 1],
								(char *)SystemStrs[SetValue.Language][m_gui_system_index],
								(char *)SystemStrs[SetValue.Language][0],(char *)SysHintStrs[SetValue.Language][4],NULL);				
				}								
		
			
		
		}break;	
	}			
  LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
  LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
}






void gui_system_btn_evt(uint32_t evt)
{	
	switch(evt)
	{		
		case (KEY_BACK):{
			m_gui_system_index = 0;
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{
			if(m_gui_system_index <= 0)
			{
				m_gui_system_index = GUI_SYSTEM_INDEX_MAX - 1;
			}else{
				m_gui_system_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
			
		case (KEY_OK):{
			switch(m_gui_system_index)
			{
				case GUI_SYSTEM_BLUETOOTH:{
					if(SetValue.SwBle >= BLE_DISCONNECT)
					{
						SetValue.SwBle = false;
						memset(&g_ble_accessory,0,sizeof(g_ble_accessory));
						dev_ble_system_off();
					}else{
						SetValue.SwBle = BLE_DISCONNECT;
						dev_ble_system_on();
					}
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);		
			
				}break;

				case GUI_SYSTEM_BASE_HEARTRATE:{
					m_gui_system_base_heartrate_index = 0;
					m_base_heartrate.enable = SetValue.SwBasedMonitorHdr;
					m_base_heartrate.hour = SetValue.DefBasedMonitorHdrHour;
					m_base_heartrate.minute = SetValue.DefBasedMonitorHdrMinute;
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
				}break;				
				case GUI_SYSTEM_AUTO_SCREENLOCK:{
					if(SetValue.SwAutoLockScreen == 1)
					{
						SetValue.SwAutoLockScreen = 0;
					}else{
						SetValue.SwAutoLockScreen = 1;
					}					
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}break;	
				case GUI_SYSTEM_LANGUGAGE:{
//					m_gui_system_language_index = 0;
//					DISPLAY_MSG  msg = {0,0};
//					ScreenState = DISPLAY_SCREEN_SYSTEM_LANGUAGE;
//					msg.cmd = MSG_DISPLAY_SCREEN;
//					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
				}break;	
				case GUI_SYSTEM_BACKLIGHT:{
					m_gui_system_backlight_index = 0;
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM_BACKLIGHT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
				}break;				
				case GUI_SYSTEM_SOUND:{
					if((SetValue.SwVibration == 0) && (SetValue.SwBuzzer == 1))
					{
						//声音
						m_gui_system_sound_index = GUI_SYSTEM_SOUND_SOUND;
					}
					else if((SetValue.SwVibration == 1) && (SetValue.SwBuzzer == 0))
					{
						//震动
						m_gui_system_sound_index = GUI_SYSTEM_SOUND_VIBERATE;
					}
					else
					{
						//声音加震动
						m_gui_system_sound_index = GUI_SYSTEM_SOUND_SOUND_N_VIBERATE;
					}
					
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM_SOUND;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}break;	
//				勿扰模式
				case GUI_SYSTEM_SILENT:{
					m_gui_silent_index = 0;
					DISPLAY_MSG  msg = {0,0};

					silent_time_set.start_hour = SetValue.NoDisturbStartHour;
					silent_time_set.start_minute = SetValue.NoDisturbStartMinute;
					silent_time_set.end_hour = SetValue.NoDisturbEndHour;
					silent_time_set.end_minute = SetValue.NoDisturbEndMinute;
					ScreenState = DISPLAY_SCREEN_SYSTEM_SILENT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
				}break;				
				case GUI_SYSTEM_AUTO_BACKLIGHT:{
					m_gui_autolight_index = 0;
					DISPLAY_MSG  msg = {0,0};

					
					 autolight_time_set.start_hour = SetValue.AutoLightStartHour;
					 autolight_time_set.start_minute =SetValue.AutoLightStartMinute;
					 autolight_time_set.end_hour = SetValue.AutoLightEndHour;
					 autolight_time_set.end_minute = SetValue.AutoLightEndMinute;
					ScreenState = DISPLAY_SCREEN_SYSTEM_AUTOLIGHT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}break;	
				
				default :
				break;
			}	
				
		}break;		
		case (KEY_DOWN):{
			if(m_gui_system_index >= ( GUI_SYSTEM_INDEX_MAX - 1))
			{
				m_gui_system_index = 0;
			}else{
				m_gui_system_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;					
	}
}
static void menu_time( char* menu1, char* menu2, char* menu3, uint8_t bck_color,const LCD_ImgStruct *pic)
{
	SetWord_t word = {0};
	
	char str[20]={0};
	word.x_axis = 26;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
   LCD_SetString(menu1,&word);	
   

	if(ScreenState == DISPLAY_SCREEN_SYSTEM_SILENT)
	{

			 	if(SetValue.SwNoDisturb==1)
				{
					
					
						word.x_axis = 120-12-16;
						word.y_axis = 120+4;
						word.size = LCD_FONT_16_SIZE;
						word.forecolor = LCD_LIGHTGRAY;
						word.bckgrndcolor = LCD_NONE;
						word.kerning = 0;			
						LCD_SetString("开启",&word);	
					
				

					
					LCD_SetPicture(120-12-16/2 -11/2 ,120 -4 - 27,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
				}

			else{
				
					
						word.x_axis = 120-12-16;
						word.y_axis = 120+4;
						word.size = LCD_FONT_16_SIZE;
						word.forecolor = LCD_LIGHTGRAY;
						word.bckgrndcolor = LCD_NONE;
						word.kerning = 0;			
						LCD_SetString("关闭",&word);
					
								
					
					LCD_SetPicture(120-12-16/2 -11/2 ,120 -4 - 27,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
				}



		    if(SetValue.SwNoDisturb==1)
					{
					 word.forecolor = LCD_WHITE;
					}
				else
					{
					 
					 word.forecolor = LCD_LIGHTGRAY;
			
					}

	 }
	else if(ScreenState == DISPLAY_SCREEN_SYSTEM_AUTOLIGHT)
		{

		 	if(SetValue.SwRaiseForLightScreen == 1)
			{
				
				
					word.x_axis = 120-12-16;
					word.y_axis = 120+4;
					word.size = LCD_FONT_16_SIZE;
					word.forecolor = LCD_LIGHTGRAY;
					word.bckgrndcolor = LCD_NONE;
					word.kerning = 0;			
					LCD_SetString("开启",&word);	
				
			

				
				LCD_SetPicture(120-12-16/2 -11/2 ,120 -4 - 27,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
			}

           else
			{
			
				
					word.x_axis = 120-12-16;
					word.y_axis = 120+4;
					word.size = LCD_FONT_16_SIZE;
					word.forecolor = LCD_LIGHTGRAY;
					word.bckgrndcolor = LCD_NONE;
					word.kerning = 0;			
					LCD_SetString("关闭",&word);
				
							
				
				LCD_SetPicture(120-12-16/2 -11/2 ,120 -4 - 27,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
			}



	    if(SetValue.SwRaiseForLightScreen==1)
				{
				 word.forecolor = LCD_WHITE;
				}
			else
				{
				 
				 word.forecolor = LCD_LIGHTGRAY;
		
				}

	    }
	else;
 

	LCD_SetRectangle(120 + 24/2 +Font_Number_29.height/2-2,4,120-8,16, word.forecolor,0,0,LCD_FILL_ENABLE);
	word.x_axis = 120 + 24/2;
	word.y_axis = 120 - 8 -10-4.5*Font_Number_29.width;	
	word.size = LCD_FONT_29_SIZE;
    word.bckgrndcolor = LCD_NONE;
 
	memset(str,0,sizeof(str));
	if(ScreenState ==DISPLAY_SCREEN_SYSTEM_SILENT)
		{
         
		 sprintf(str,"%02d:%02d", silent_time_set.start_hour, silent_time_set.start_minute);

	    }
	else if(ScreenState ==DISPLAY_SCREEN_SYSTEM_AUTOLIGHT)
		{
		 sprintf(str,"%02d:%02d", autolight_time_set.start_hour, autolight_time_set.start_minute);

	  }
	else;

	LCD_SetNumber(str,&word);

	
	
	word.y_axis = 120 + 8 + 10;	
	memset(str,0,sizeof(str));

	
	if(ScreenState ==DISPLAY_SCREEN_SYSTEM_SILENT)
    {	
		sprintf(str,"%02d:%02d", silent_time_set.end_hour, silent_time_set.end_minute);
		}

	else if(ScreenState ==DISPLAY_SCREEN_SYSTEM_AUTOLIGHT)
		{
        sprintf(str,"%02d:%02d", autolight_time_set.end_hour, autolight_time_set.end_minute);
	}
	else;
	LCD_SetNumber(str,&word);

	
	
	
	
	
	

}

void gui_system_silent_paint(void)
{
	char str[20]={0};
	LCD_SetBackgroundColor(LCD_BLACK);
	
 
//   memset(str,0,sizeof(str));
  // sprintf(str,"%02d:%02d-%02d:%02d", SetValue.NoDisturbStartHour, SetValue.NoDisturbStartMinute, SetValue.NoDisturbEndHour,SetValue.NoDisturbEndMinute);
  	menu_time((char *)SystemStrs[SetValue.Language][6],(char *)SysHintStrs[SetValue.Language][3],str,LCD_BLACK,&img_v_switch_on);				
			
			
}

void gui_system_silent_time_paint(void)
{
	char str[20]={0};
	SetWord_t word = {0};
	LCD_SetBackgroundColor(LCD_BLACK);

	word.x_axis = 28;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	
	LCD_SetString((char *)SysHintStrs[SetValue.Language][5],&word);	

	word.x_axis = 52+24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_49_SIZE;
	word.forecolor = LCD_WHITE;	
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;

	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", silent_time_set.start_hour, silent_time_set.start_minute);

	LCD_SetNumber(str,&word);

	word.x_axis = 76+Font_Number_49.height+24;	
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", silent_time_set.end_hour, silent_time_set.end_minute);

	LCD_SetNumber(str,&word);
 //  LCD_SetRectangle(120-8,16,120-2,4,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	switch(m_gui_silent_time_index)
		{
			case GUI_SYSTEM_TIME_START_HOUR:{ 
				LCD_SetRectangle(76+Font_Number_49.height+4,6,120 - (2+0.25)*Font_Number_49.width,2*Font_Number_49.width,LCD_CYAN,0,0,LCD_FILL_ENABLE); 
			}break;
		
			case GUI_SYSTEM_TIME_START_MIN:{		
				LCD_SetRectangle(76+Font_Number_49.height+4,6,120+0.25*Font_Number_49.width,2*Font_Number_49.width,LCD_CYAN,0,0,LCD_FILL_ENABLE); 
			}break; 
			case GUI_SYSTEM_TIME_END_HOUR:{	
				LCD_SetRectangle(76+Font_Number_49.height+24+Font_Number_49.height+4,6,120 - (2+0.25)*Font_Number_49.width,2*Font_Number_49.width,LCD_CYAN,0,0,LCD_FILL_ENABLE); 
			}break;
			case GUI_SYSTEM_TIME_END_MIN:{	
				LCD_SetRectangle(76+Font_Number_49.height+24+Font_Number_49.height+4,6,120 +0.25*Font_Number_49.width,2*Font_Number_49.width,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}break;
			default:
				break;
		}	
	
	
   
}

void gui_system_autolight_paint(void)
{
	char str[20]={0};
	LCD_SetBackgroundColor(LCD_BLACK);
	//memset(str,0,sizeof(str));
	//sprintf(str,"%02d:%02d-%02d:%02d", SetValue.AutoLightStartHour, SetValue.AutoLightStartMinute, SetValue.AutoLightEndHour,SetValue.AutoLightEndMinute);

		menu_time((char *)SystemStrs[SetValue.Language][7],(char *)SysHintStrs[SetValue.Language][3],str,LCD_BLACK,&img_v_switch_on);	
}

void gui_system_autolight_time_paint(void)
{
	char str[20]={0};
	SetWord_t word = {0};
	LCD_SetBackgroundColor(LCD_BLACK);

	word.x_axis = 28;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	
	LCD_SetString((char *)SysHintStrs[SetValue.Language][5],&word);	

	word.x_axis = 52+24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_49_SIZE;
	word.forecolor = LCD_WHITE;	
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;

	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", autolight_time_set.start_hour, autolight_time_set.start_minute);

	LCD_SetNumber(str,&word);

	word.x_axis = 76+Font_Number_49.height+24;	
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", autolight_time_set.end_hour, autolight_time_set.end_minute);

	LCD_SetNumber(str,&word);
 //  LCD_SetRectangle(120-8,16,120-2,4,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	switch(m_gui_autolight_time_index)
		{
			case GUI_SYSTEM_TIME_START_HOUR:{ 
				LCD_SetRectangle(76+Font_Number_49.height+4,6,120 - (2+0.25)*Font_Number_49.width,2*Font_Number_49.width,LCD_CYAN,0,0,LCD_FILL_ENABLE); 
			}break;
		
			case GUI_SYSTEM_TIME_START_MIN:{		
				LCD_SetRectangle(76+Font_Number_49.height+4,6,120+0.25*Font_Number_49.width,2*Font_Number_49.width,LCD_CYAN,0,0,LCD_FILL_ENABLE); 
			}break; 
			case GUI_SYSTEM_TIME_END_HOUR:{	
				LCD_SetRectangle(76+Font_Number_49.height+24+Font_Number_49.height+4,6,120 - (2+0.25)*Font_Number_49.width,2*Font_Number_49.width,LCD_CYAN,0,0,LCD_FILL_ENABLE); 
			}break;
			case GUI_SYSTEM_TIME_END_MIN:{	
				LCD_SetRectangle(76+Font_Number_49.height+24+Font_Number_49.height+4,6,120 +0.25*Font_Number_49.width,2*Font_Number_49.width,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}break;
			default:
				break;
		}	
	
	
   
}	
	
	
   

void gui_system_silent_btn_evt(uint32_t evt)
{
        switch(evt)
        {       
           
           
			case (KEY_UP):
            case (KEY_DOWN):
				{
		            if (SetValue.SwNoDisturb == 1)
		            {
					  
						
						SetValue.SwNoDisturb = 0;
					    DISPLAY_MSG  msg = {0,0};
					    ScreenState = DISPLAY_SCREEN_SYSTEM_SILENT;
					    msg.cmd = MSG_DISPLAY_SCREEN;
					    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		           }
					else
		                {
		                   
							SetValue.SwNoDisturb = 1;
		                    DISPLAY_MSG  msg = {0,0};
		                    ScreenState = DISPLAY_SCREEN_SYSTEM_SILENT;
		                    msg.cmd = MSG_DISPLAY_SCREEN;
		                    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		               }
		       }
			break;
         case (KEY_OK):{
			    
                 
                    if(SetValue.SwNoDisturb==0)
                    {
                       DISPLAY_MSG  msg = {0,0};
			        ScreenState = DISPLAY_SCREEN_SYSTEM;
			        msg.cmd = MSG_DISPLAY_SCREEN;
			        xQueueSend(DisplayQueue, &msg, portMAX_DELAY); 					
                    }
					else
					 {
					   DISPLAY_MSG  msg = {0,0};
			        ScreenState = DISPLAY_SCREEN_SYSTEM_SILENT_TIME;
			        msg.cmd = MSG_DISPLAY_SCREEN;
			        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
                          

					  }
                    
		          }break;  

		     case (KEY_BACK):{
                DISPLAY_MSG  msg = {0,0};
                ScreenState = DISPLAY_SCREEN_SYSTEM;
                msg.cmd = MSG_DISPLAY_SCREEN;
                xQueueSend(DisplayQueue, &msg, portMAX_DELAY);          
            }break;         

      }

}

void gui_system_silent_time_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_silent_time_index)
			{
				case GUI_SYSTEM_TIME_START_HOUR:{
					m_gui_silent_time_index = GUI_SYSTEM_TIME_START_MIN;						
				}break;
				case GUI_SYSTEM_TIME_START_MIN:{
					m_gui_silent_time_index = GUI_SYSTEM_TIME_END_HOUR;
				}break;
				case GUI_SYSTEM_TIME_END_HOUR:{
					m_gui_silent_time_index = GUI_SYSTEM_TIME_END_MIN;						
				}break;	
				case GUI_SYSTEM_TIME_END_MIN:{
					m_gui_silent_time_index = GUI_SYSTEM_TIME_START_HOUR;

					SetValue.NoDisturbStartHour = silent_time_set.start_hour;
					SetValue.NoDisturbStartMinute= silent_time_set.start_minute;
					SetValue.NoDisturbEndHour = silent_time_set.end_hour;
					SetValue.NoDisturbEndMinute= silent_time_set.end_minute;

					if (is_silent_time() == 1)
					{
						system_silent_mode_set();
					}
					else
					{
						system_silent_mode_clear();
					}
					
					
					ScreenState = DISPLAY_SCREEN_SYSTEM_SILENT;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_silent_time_index)
			{
				case GUI_SYSTEM_TIME_START_HOUR:{
					if(silent_time_set.start_hour <= 0)
					{
						silent_time_set.start_hour = 23;
					}else{
						silent_time_set.start_hour--;
					}						
				}break;		
				case GUI_SYSTEM_TIME_START_MIN:{
					if(silent_time_set.start_minute <= 0)
					{
						silent_time_set.start_minute = 59;
					}else{
						silent_time_set.start_minute--;
					}						
				}break;
				case GUI_SYSTEM_TIME_END_HOUR:{
					if(silent_time_set.end_hour <= 0)
					{
						silent_time_set.end_hour = 23;
					}else{
						silent_time_set.end_hour--;
					}						
				}break;
				case GUI_SYSTEM_TIME_END_MIN:{
					if(silent_time_set.end_minute <= 0)
					{
						silent_time_set.end_minute = 59;
					}else{
						silent_time_set.end_minute--;
					}						
				}break;	
				
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_silent_time_index)
			{
				case GUI_SYSTEM_TIME_START_HOUR:{
					if(silent_time_set.start_hour >= 23)
					{
						silent_time_set.start_hour = 0;
					}else{
						silent_time_set.start_hour++;
					}						
				}break;		
				case GUI_SYSTEM_TIME_START_MIN:{
					if(silent_time_set.start_minute >= 59)
					{
						silent_time_set.start_minute = 0;
					}else{
						silent_time_set.start_minute++;
					}						
				}break;
				case GUI_SYSTEM_TIME_END_HOUR:{
					if(silent_time_set.end_hour >= 23)
					{
						silent_time_set.end_hour = 0;
					}else{
						silent_time_set.end_hour++;
					}						
				}break;
				case GUI_SYSTEM_TIME_END_MIN:{
					if(silent_time_set.end_minute >= 59)
					{
						silent_time_set.end_minute = 0;
					}else{
						silent_time_set.end_minute++;
					}						
				}break;	
				
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_silent_time_index)
			{
			case GUI_SYSTEM_TIME_START_HOUR:{
					ScreenState = DISPLAY_SCREEN_SYSTEM_SILENT;						
				}break;
				case GUI_SYSTEM_TIME_START_MIN:{
					m_gui_silent_time_index = GUI_SYSTEM_TIME_START_HOUR;
				}break;
				case GUI_SYSTEM_TIME_END_HOUR:{
					m_gui_silent_time_index = GUI_SYSTEM_TIME_START_MIN;						
				}break;	
				case GUI_SYSTEM_TIME_END_MIN:{
					m_gui_silent_time_index = GUI_SYSTEM_TIME_END_HOUR;					
				
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}

void gui_system_autolight_btn_evt(uint32_t evt)
{
        switch(evt)
        {       
           
            case (KEY_UP):
			case (KEY_DOWN):
			  {
                if (SetValue.SwRaiseForLightScreen == 1)
                {
                    SetValue.SwRaiseForLightScreen =0;
                    DISPLAY_MSG  msg = {0,0};
                    ScreenState = DISPLAY_SCREEN_SYSTEM_AUTOLIGHT;
                    msg.cmd = MSG_DISPLAY_SCREEN;
                    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
               }
				  else 
                {
                    SetValue.SwRaiseForLightScreen =1;
                    DISPLAY_MSG  msg = {0,0};
                    ScreenState = DISPLAY_SCREEN_SYSTEM_AUTOLIGHT;
                    msg.cmd = MSG_DISPLAY_SCREEN;
                    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
               }
            }break;
          
         case (KEY_OK):{
			    if(SetValue.SwRaiseForLightScreen  == 0)
                {  
                    
			        DISPLAY_MSG  msg = {0,0};
			        ScreenState = DISPLAY_SCREEN_SYSTEM;
			        msg.cmd = MSG_DISPLAY_SCREEN;
			        xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
                }
                else 
                {
                
                    DISPLAY_MSG  msg = {0,0};
                    ScreenState = DISPLAY_SCREEN_SYSTEM_AUTOLIGHT_TIME;
                    msg.cmd = MSG_DISPLAY_SCREEN;
                    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

                }
		}break;          
             case (KEY_BACK):{
                DISPLAY_MSG  msg = {0,0};
                ScreenState = DISPLAY_SCREEN_SYSTEM;
                msg.cmd = MSG_DISPLAY_SCREEN;
                xQueueSend(DisplayQueue, &msg, portMAX_DELAY);          
            }break;         
      }

}
void gui_system_autolight_time_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_autolight_time_index)
			{
				case GUI_SYSTEM_TIME_START_HOUR:{
					m_gui_autolight_time_index = GUI_SYSTEM_TIME_START_MIN;						
				}break;
				case GUI_SYSTEM_TIME_START_MIN:{
					m_gui_autolight_time_index = GUI_SYSTEM_TIME_END_HOUR;
				}break;
				case GUI_SYSTEM_TIME_END_HOUR:{
					m_gui_autolight_time_index = GUI_SYSTEM_TIME_END_MIN;						
				}break;	
				case GUI_SYSTEM_TIME_END_MIN:{
					m_gui_autolight_time_index = GUI_SYSTEM_TIME_START_HOUR;

					SetValue.AutoLightStartHour = autolight_time_set.start_hour;
					SetValue.AutoLightStartMinute= autolight_time_set.start_minute;
					SetValue.AutoLightEndHour = autolight_time_set.end_hour;
					SetValue.AutoLightEndMinute= autolight_time_set.end_minute;
					ScreenState = DISPLAY_SCREEN_SYSTEM_AUTOLIGHT;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_autolight_time_index)
			{
				case GUI_SYSTEM_TIME_START_HOUR:{
					if(autolight_time_set.start_hour <= 0)
					{
						autolight_time_set.start_hour = 23;
					}else{
						autolight_time_set.start_hour--;
					}						
				}break;		
				case GUI_SYSTEM_TIME_START_MIN:{
					if(autolight_time_set.start_minute <= 0)
					{
						autolight_time_set.start_minute = 59;
					}else{
						autolight_time_set.start_minute--;
					}						
				}break;
				case GUI_SYSTEM_TIME_END_HOUR:{
					if(autolight_time_set.end_hour <= 0)
					{
						autolight_time_set.end_hour = 23;
					}else{
						autolight_time_set.end_hour--;
					}						
				}break;
				case GUI_SYSTEM_TIME_END_MIN:{
					if(autolight_time_set.end_minute <= 0)
					{
						autolight_time_set.end_minute = 59;
					}else{
						autolight_time_set.end_minute--;
					}						
				}break;	
				
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_autolight_time_index)
			{
				case GUI_SYSTEM_TIME_START_HOUR:{
					if(autolight_time_set.start_hour >= 23)
					{
						autolight_time_set.start_hour = 0;
					}else{
						autolight_time_set.start_hour++;
					}						
				}break;		
				case GUI_SYSTEM_TIME_START_MIN:{
					if(autolight_time_set.start_minute >= 59)
					{
						autolight_time_set.start_minute = 0;
					}else{
						autolight_time_set.start_minute++;
					}						
				}break;
				case GUI_SYSTEM_TIME_END_HOUR:{
					if(autolight_time_set.end_hour >= 23)
					{
						autolight_time_set.end_hour = 0;
					}else{
						autolight_time_set.end_hour++;
					}						
				}break;
				case GUI_SYSTEM_TIME_END_MIN:{
					if(autolight_time_set.end_minute >= 59)
					{
						autolight_time_set.end_minute = 0;
					}else{
						autolight_time_set.end_minute++;
					}						
				}break;	
				
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_autolight_time_index)
			{
			case GUI_SYSTEM_TIME_START_HOUR:{
					ScreenState = DISPLAY_SCREEN_SYSTEM_AUTOLIGHT;						
				}break;
				case GUI_SYSTEM_TIME_START_MIN:{
					m_gui_autolight_time_index = GUI_SYSTEM_TIME_START_HOUR;
				}break;
				case GUI_SYSTEM_TIME_END_HOUR:{
					m_gui_autolight_time_index = GUI_SYSTEM_TIME_START_MIN;						
				}break;	
				case GUI_SYSTEM_TIME_END_MIN:{
					m_gui_autolight_time_index = GUI_SYSTEM_TIME_END_HOUR;					
				
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}


static void system_base_heartrate(_base_heartrate base_heartrate)
{
	SetWord_t word = {0};
	word.x_axis = 26;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString("基础心率",&word);	



	if(base_heartrate.enable==1)
	{
		if(SetValue.Language == L_CHINESE)
		{
			word.x_axis = 120-12-16;
			word.y_axis = 120+4;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_LIGHTGRAY;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 0;			
			LCD_SetString("开启",&word);	
		}
		else if(SetValue.Language == L_ENGLISH)
		{
			word.x_axis = 120-12-16;
			word.y_axis = 120+4;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_LIGHTGRAY;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 0;			
			LCD_SetString("ON",&word);		
		}	

		
		LCD_SetPicture(120-12-16/2 -11/2 ,120 -4 - 27,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
	}

else{
	
		if(SetValue.Language == L_CHINESE)
		{
			word.x_axis = 120-12-16;
			word.y_axis = 120+4;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_LIGHTGRAY;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 0;			
			LCD_SetString("关闭",&word);
		}
		else if(SetValue.Language == L_ENGLISH)
		{
			word.x_axis = 120-12-16;
			word.y_axis = 120+4;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_LIGHTGRAY;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 0;			
			LCD_SetString("OFF",&word);		
		}			
		
		LCD_SetPicture(120-12-16/2 -11/2 ,120 -4 - 27,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
	}
   
            


	
    char p_str[23];
	memset(p_str,0,sizeof(p_str));	
	sprintf(p_str,"%02d:%02d",base_heartrate.hour,base_heartrate.minute);
	word.x_axis = 120 + 12;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	if(m_base_heartrate.enable)
		{
	     word.forecolor = LCD_WHITE;
		}
	else
		{
         
	     word.forecolor = LCD_LIGHTGRAY;

	    }
	word.bckgrndcolor = LCD_NONE;	
	LCD_SetNumber(p_str,&word);	
	switch(m_gui_system_base_heartrate_index)
	{
		case GUI_SYSTEM_BASE_HEARTRATE_SWITCH:{	
			//LCD_SetRectangle(128,4,96,56,LCD_CORNFLOWERBLUE,0,0,LCD_FILL_ENABLE);	//改为图标表示
		}break;
		case GUI_SYSTEM_BASE_HEARTRATE_HOUR:{
			LCD_SetRectangle(120 + 12 +Font_Number_56.height + 2,4,120 - 2*Font_Number_56.width - 0.25*Font_Number_56.width,2*Font_Number_56.width,LCD_CYAN,0,0,LCD_FILL_ENABLE);	
		}break;		
		case GUI_SYSTEM_BASE_HEARTRATE_MINUTE:{
			LCD_SetRectangle(120 + 12 +Font_Number_56.height + 2,4,120  + 0.25*Font_Number_56.width,2*Font_Number_56.width,LCD_CYAN,0,0,LCD_FILL_ENABLE);	
		}break;		
	}
	
	

}

void gui_system_base_heartrate_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

	system_base_heartrate(m_base_heartrate);

}


void gui_system_base_heartrate_btn_evt(uint32_t evt)
{	
	switch(evt)
	{		
		case (KEY_BACK):{
			switch(m_gui_system_base_heartrate_index)
			{
				case GUI_SYSTEM_BASE_HEARTRATE_SWITCH:{
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}break;
				case GUI_SYSTEM_BASE_HEARTRATE_HOUR:{
					m_gui_system_base_heartrate_index = GUI_SYSTEM_BASE_HEARTRATE_SWITCH;
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}break;		
				case GUI_SYSTEM_BASE_HEARTRATE_MINUTE:{
					m_gui_system_base_heartrate_index = GUI_SYSTEM_BASE_HEARTRATE_HOUR;
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}break;		
				
			}			
		}break;	
			case (KEY_OK):{
			switch(m_gui_system_base_heartrate_index)
			{
				case GUI_SYSTEM_BASE_HEARTRATE_SWITCH:{
					
					if(m_base_heartrate.enable)
					{
						m_gui_system_base_heartrate_index = GUI_SYSTEM_BASE_HEARTRATE_HOUR;
						DISPLAY_MSG  msg = {0,0};
						ScreenState = DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
					}else{
						SetValue.SwBasedMonitorHdr = false;
						m_gui_system_base_heartrate_index = GUI_SYSTEM_BASE_HEARTRATE_SWITCH;
						DISPLAY_MSG  msg = {0,0};
						ScreenState = DISPLAY_SCREEN_SYSTEM;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);						
					}
					
					
				}break;
				case GUI_SYSTEM_BASE_HEARTRATE_HOUR:{
					m_gui_system_base_heartrate_index = GUI_SYSTEM_BASE_HEARTRATE_MINUTE;
					if(m_base_heartrate.hour == 12)
					{
						m_base_heartrate.minute = 0;
					}
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);					
				}break;		
				case GUI_SYSTEM_BASE_HEARTRATE_MINUTE:{
					SetValue.SwBasedMonitorHdr = m_base_heartrate.enable;
					SetValue.DefBasedMonitorHdrHour = m_base_heartrate.hour;
					SetValue.DefBasedMonitorHdrMinute = m_base_heartrate.minute;						
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_SYSTEM;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
				}break;		
			}
		}break;		
		case (KEY_UP):{
			switch(m_gui_system_base_heartrate_index)
			{
				case GUI_SYSTEM_BASE_HEARTRATE_SWITCH:{
					if(m_base_heartrate.enable ==1)
						{

                       m_base_heartrate.enable=0;

					}
					else
						{
                        
						m_base_heartrate.enable=1;

					   }
				}break;
				case GUI_SYSTEM_BASE_HEARTRATE_HOUR:{
					if(m_base_heartrate.hour >= 12)
					{
						m_base_heartrate.hour = 5;
					}else{
						m_base_heartrate.hour++;
					}						
				}break;		
				case GUI_SYSTEM_BASE_HEARTRATE_MINUTE:{
					if(m_base_heartrate.hour == 12)
					{
						m_base_heartrate.minute = 0;
					}
					else
					{
						if(m_base_heartrate.minute >= 59)
						{
							m_base_heartrate.minute = 0;
						}else{
							m_base_heartrate.minute++;
						}	
					}
				}break;		
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
			
		}break;	
			
	
		case (KEY_DOWN):{
			switch(m_gui_system_base_heartrate_index)
			{
				case GUI_SYSTEM_BASE_HEARTRATE_SWITCH:{
					if(m_base_heartrate.enable ==1)
						{

                       m_base_heartrate.enable=0;

					}
					else
						{
                        
						m_base_heartrate.enable=1;

					   }
				}break;
				case GUI_SYSTEM_BASE_HEARTRATE_HOUR:{
					if(m_base_heartrate.hour <= 5)
					{
						m_base_heartrate.hour = 12;
					}else{
						m_base_heartrate.hour--;
					}					
				}break;		
				case GUI_SYSTEM_BASE_HEARTRATE_MINUTE:{
					if(m_base_heartrate.hour == 12)
					{
						m_base_heartrate.minute = 0;
					}
					else
					{
						if(m_base_heartrate.minute <= 0)
						{
							m_base_heartrate.minute = 59;
						}else{
							m_base_heartrate.minute--;
						}	
					}
				}break;		
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;					
	}
}



void gui_system_language_paint(void)
{
	LCD_SetBackgroundColor(BACKGROUND_COLOR);

	switch(m_gui_system_language_index)
	{
		case GUI_SYSTEM_LANGUAGE_CHINESE:{		
			if(SetValue.Language == L_CHINESE)
			{
			menu_system("语言设置",
										(char *)LanguageStrs[SetValue.Language][m_gui_system_language_index],
										(char *)LanguageStrs[SetValue.Language][m_gui_system_language_index + 1]);						
			}
			else if(SetValue.Language == L_ENGLISH)
			{
			menu_system("LANGUAGE",
										(char *)LanguageStrs[SetValue.Language][m_gui_system_language_index],
										(char *)LanguageStrs[SetValue.Language][m_gui_system_language_index + 1]);							
			}				
		}break;
		case GUI_SYSTEM_LANGUAGE_ENGLISH:{
			if(SetValue.Language == L_CHINESE)
			{
			menu_system("语言设置",
										(char *)LanguageStrs[SetValue.Language][m_gui_system_language_index],
										(char *)LanguageStrs[SetValue.Language][0]);						
			}
			else if(SetValue.Language == L_ENGLISH)
			{
			menu_system("LANGUAGE",
										(char *)LanguageStrs[SetValue.Language][m_gui_system_language_index],
										(char *)LanguageStrs[SetValue.Language][0]);							
			}				
		
		}break;	
	}
	
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}


void gui_system_language_btn_evt(uint32_t evt)
{	
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;			
		case (KEY_UP):{
			if(m_gui_system_language_index <= 0)
			{
				m_gui_system_language_index = GUI_LANGUAGE_INDEX_MAX - 1;
			}else{
				m_gui_system_language_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM_LANGUAGE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
			
		case (KEY_OK):{
			switch(m_gui_system_language_index)
			{
				case GUI_SYSTEM_LANGUAGE_CHINESE:{
					SetValue.Language = L_CHINESE;
				}break;
				//case GUI_SYSTEM_LANGUAGE_ENGLISH:{		//英文界面未完成，暂时隐去 V1.0.2B103
				//	SetValue.Language = L_ENGLISH;
				//}break;	
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;		
		case (KEY_DOWN):{
			if(m_gui_system_language_index >= ( GUI_LANGUAGE_INDEX_MAX - 1))
			{
				m_gui_system_language_index = 0;
			}else{
				m_gui_system_language_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM_LANGUAGE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;					
	}
}






void gui_system_backlight_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

	switch(m_gui_system_backlight_index)
	{
		case GUI_SYSTEM_BACKLIGHT_10_SECOND:{		
			if(SetValue.Language == L_CHINESE)
			{
			/*menu_system("背光时长",
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index + 1]);*/
				menu_system((char *)BacklightTimeStrs[SetValue.Language][GUI_BACKLIGHT_TIME_INDEX_MAX-1],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index + 1]);
			}
			else if(SetValue.Language == L_ENGLISH)
			{
			menu_system((char *)BacklightTimeStrs[SetValue.Language][2],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index + 1]);							
			}				
		}break;
		case GUI_SYSTEM_BACKLIGHT_30_SECOND:{
			if(SetValue.Language == L_CHINESE)
			{
			menu_system((char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index-1],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index + 1]);				
			}
			else if(SetValue.Language == L_ENGLISH)
			{
			menu_system((char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index-1],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index + 1]);					
			}				
		
		}break;	
		case GUI_SYSTEM_BACKLIGHT_60_SECOND:{
			if(SetValue.Language == L_CHINESE)
			{
			menu_system((char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index-1],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index],
										(char *)BacklightTimeStrs[SetValue.Language][0]);						
			}
			else if(SetValue.Language == L_ENGLISH)
			{
			menu_system((char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index-1],
										(char *)BacklightTimeStrs[SetValue.Language][m_gui_system_backlight_index],
										(char *)BacklightTimeStrs[SetValue.Language][0]);							
			}				
		
		}break;			
	}
	if(m_gui_system_backlight_index!=GUI_SYSTEM_BACKLIGHT_10_SECOND)
		{
        LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
		}
  LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
	
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}


void gui_system_backlight_btn_evt(uint32_t evt)
{	
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;			
		case (KEY_UP):{
			if(m_gui_system_backlight_index <= 0)
			{
				m_gui_system_backlight_index = GUI_BACKLIGHT_TIME_INDEX_MAX - 1;
			}else{
				m_gui_system_backlight_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM_BACKLIGHT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;	
			
		case (KEY_OK):{
			switch(m_gui_system_backlight_index)
			{
				case GUI_SYSTEM_BACKLIGHT_10_SECOND:{		
					SetValue.backlight_timeout_mssec = 7000;
				}break;
				case GUI_SYSTEM_BACKLIGHT_30_SECOND:{
					SetValue.backlight_timeout_mssec = 15000;
				}break;	
				case GUI_SYSTEM_BACKLIGHT_60_SECOND:{
					SetValue.backlight_timeout_mssec = 30000;
				}break;			

				default:
					break;
			}				
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;		
		case (KEY_DOWN):{
			if(m_gui_system_backlight_index >= ( GUI_BACKLIGHT_TIME_INDEX_MAX - 1))
			{
				m_gui_system_backlight_index = 0;
			}else{
				m_gui_system_backlight_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM_BACKLIGHT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;					
	}
}




void gui_system_sound_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);

	switch(m_gui_system_sound_index)
	{
		case GUI_SYSTEM_SOUND_SOUND_N_VIBERATE:{		
			if(SetValue.Language == L_CHINESE)
			{
			/*menu_system("声音选择",
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index + 1]);*/	
			menu_system((char *)SystemSoundNViberateStrs[SetValue.Language][GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX-1],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index + 1]);	
			}
			else if(SetValue.Language == L_ENGLISH)
			{
			menu_system((char *)SystemSoundNViberateStrs[SetValue.Language][GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX-1],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index + 1]);							
			}				
		}break;
		case GUI_SYSTEM_SOUND_SOUND:{
			if(SetValue.Language == L_CHINESE)
			{
			menu_system((char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index-1],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index + 1]);				
			}
			else if(SetValue.Language == L_ENGLISH)
			{
			menu_system((char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index-1],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index + 1]);					
			}				
		
		}break;	
		case GUI_SYSTEM_SOUND_VIBERATE:{
			if(SetValue.Language == L_CHINESE)
			{
			menu_system((char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index-1],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index],
										(char *)SystemSoundNViberateStrs[SetValue.Language][0]);						
			}
			else if(SetValue.Language == L_ENGLISH)
			{
			menu_system((char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index-1],
										(char *)SystemSoundNViberateStrs[SetValue.Language][m_gui_system_sound_index],
										(char *)SystemSoundNViberateStrs[SetValue.Language][0]);							
			}				
		
		}break;			
	}
	if(m_gui_system_sound_index!=GUI_SYSTEM_SOUND_SOUND_N_VIBERATE)
		{
	   LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
		}
  LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
//	LCD_DisplaySomeLine(0,LCD_LINE_CNT_MAX);
}


void gui_system_sound_btn_evt(uint32_t evt)
{	
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}break;			
		case (KEY_UP):{
			if(m_gui_system_sound_index <= 0)
			{
				m_gui_system_sound_index = GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX - 1;
			}else{
				m_gui_system_sound_index--;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM_SOUND;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;	
			
		case (KEY_OK):{
			switch(m_gui_system_sound_index)
			{
				case GUI_SYSTEM_SOUND_SOUND_N_VIBERATE:{	
					SetValue.SwVibration = 1;
					SetValue.SwBuzzer = 1;
				}break;
				case GUI_SYSTEM_SOUND_SOUND:{
					SetValue.SwVibration = 0;
					SetValue.SwBuzzer = 1;			
				}break;	
				case GUI_SYSTEM_SOUND_VIBERATE:{
					SetValue.SwVibration = 1;
					SetValue.SwBuzzer = 0;				
				}break;			
			}	
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);				
		}break;		
		case (KEY_DOWN):{
			if(m_gui_system_sound_index >= ( GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX - 1))
			{
				m_gui_system_sound_index = 0;
			}else{
				m_gui_system_sound_index++;
			}
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_SYSTEM_SOUND;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;					
	}
}



