#include "gui_post_watch_info.h"
#include "SEGGER_RTT.h"
#include "drv_key.h"
#include "drv_lcd.h"
#include <string.h>
#include "task_display.h"
#include "com_data.h"
#include "lib_app_data.h"

#include "QR_Encode.h"

#define GUI_POST_WATCH_INFO_LOG_ENABLED 1

#if DEBUG_ENABLED == 1 && GUI_POST_WATCH_INFO_LOG_ENABLED == 1
	#define GUI_POST_WATCH_INFO_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_POST_WATCH_INFO_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_POST_WATCH_INFO_WRITESTRING(...)
	#define GUI_POST_WATCH_INFO_PRINTF(...)		        
#endif

extern SetValueStr SetValue;

//static int toVal(char *pcAsciiStr)
//{
//	int iRetVal = 0;
//	iRetVal += pcAsciiStr[1] - '0';
//	iRetVal += pcAsciiStr[0] == ' ' ? 0 : (pcAsciiStr[0] - '0') * 10;
//	return iRetVal;
//}
//static char *pcMonth[] =
//{
//    "January",
//    "February",
//    "March",
//    "April",
//    "May",
//    "June",
//    "July",
//    "August",
//    "September",
//    "October",
//    "November",
//    "December",
//    "Invalid month"
//};

//static int mthToIndex(char *pcMon)
//{
//    int idx;
//    for (idx = 0; idx < 12; idx++)
//    {
//        if ( am_util_string_strnicmp(pcMonth[idx], pcMon, 3) == 0 )
//        {
//            return idx+1;
//        }
//    }
//    return 12;
//}


#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10  + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))  
  
#define MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5)  : __DATE__ [2] == 'b' ? 1  : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3)  : __DATE__ [2] == 'y' ? 4  : __DATE__ [2] == 'n' ? 5  : __DATE__ [2] == 'l' ? 6 : __DATE__ [2] == 'g' ? 7 : __DATE__ [2] == 'p' ? 8 : __DATE__ [2] == 't' ? 9 : __DATE__ [2] == 'v' ? 10 : 11)  
  
#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10+ (__DATE__ [5] - '0'))  
  
#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)  

#define HOUR ((__TIME__ [0] == ' ' ? 0 : __TIME__ [0] - '0') * 10+ (__TIME__ [1] - '0')) 
#define MINUTE ((__TIME__ [3] == ' ' ? 0 : __TIME__ [3] - '0') * 10+ (__TIME__ [4] - '0'))
#define SECOND ((__TIME__ [6] == ' ' ? 0 : __TIME__ [6] - '0') * 10+ (__TIME__ [7] - '0'))

void gui_post_watch_info_paint(void)
{
	LCD_SetBackgroundColor(LCD_WHITE);
	SetWord_t word = {0};
	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.kerning = 0;	
	char p_char[20];
	
	
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"编译时间");	
	LCD_SetString(p_char,&word);		
	
	word.x_axis += 20;
	memset(p_char,0,sizeof(p_char));
	sprintf(p_char,"%d-%02d-%02d %02d:%02d:%02d",YEAR, MONTH + 1, DAY,HOUR,MINUTE,SECOND);
	LCD_SetString(p_char,&word);		
	
	_lib_app_data lib_app_data;
	lib_app_data_read(&lib_app_data);		
	if(lib_app_data_crc_check_bootsetting(&lib_app_data))
	{
		uint16_t year = (uint16_t)(lib_app_data.ble.build_date>>16);
		uint8_t month = (uint8_t)(lib_app_data.ble.build_date>>8);
		uint8_t day = (uint8_t)(lib_app_data.ble.build_date>>0);
		uint8_t hour = (uint8_t)(lib_app_data.ble.build_time>>16);	
		uint8_t minute = (uint8_t)(lib_app_data.ble.build_time>>8);
		uint8_t second = (uint8_t)(lib_app_data.ble.build_time>>0);
		
		if(year != 0)
		{
			word.x_axis += 20;
			memset(p_char,0,sizeof(p_char));
			sprintf(p_char,"蓝牙编译时间");
			LCD_SetString(p_char,&word);			
			
			word.x_axis += 20;
			memset(p_char,0,sizeof(p_char));
			sprintf(p_char,"%d-%02d-%02d %02d:%02d:%02d",year, month + 1, day,hour,minute,second);
			LCD_SetString(p_char,&word);				
		}
	}
	
	word.x_axis = 110;
	word.y_axis = 70;
//	LCD_QR_CODE("深圳市龙腾飞通讯装备技术有限公司",&word,4);

	

}



void gui_post_watch_info_btn_evt(uint32_t evt)
{
	switch(evt)
	{		
		case (KEY_BACK):{
			DISPLAY_MSG  msg = {0,0};
			ScreenState = DISPLAY_SCREEN_POST_SETTINGS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			
		case (KEY_UP):{

		}break;	
			
		case (KEY_OK):{

			
		}break;		
		case (KEY_DOWN):{

		}break;					
	
	}

}





