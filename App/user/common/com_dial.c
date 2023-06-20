#include "com_config.h"
#include "com_dial.h"

#include <time.h>

#include "drv_lcd.h"
#include "drv_extFlash.h"

#include "com_data.h"

#include "font_config.h"

#if DEBUG_ENABLED == 1 && COM_DATA_LOG_ENABLED == 1
	#define COM_DATA_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define COM_DATA_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define COM_DATA_WRITESTRING(...)
	#define COM_DATA_PRINTF(...)
#endif

extern SetValueStr SetValue;

DialDataStr DialData = {0};
void com_dial_init(void)
{	
	//判断节日表盘是否有效
	if(com_dial_festival_is_valid())
	{
		//自动设置为节日表盘，将最高位置1表示使用节日表盘
		SetValue.Theme |= 0x80;
	}
	else
	{
		SetValue.Theme &= 0x7F;
	}
	
	//清除数据
	memset(&DialData, 0, sizeof(DialDataStr));
	//获取表盘数据
	com_dial_get(SetValue.Theme, &DialData);
}

void com_dial_read(uint8_t num, DialDataStr *dial)
{
	//从外部flash中读取表盘数据
	dev_extFlash_enable();
	dev_extFlash_read(DIAL_DATA_START_ADDRESS + (num * DIAL_DATA_SIZE), (uint8_t*)(dial), sizeof(DialDataStr));
	dev_extFlash_disable();
}

void com_dial_delete(void)
{
	//删除外部flash中的表盘
	dev_extFlash_enable();
	dev_extFlash_erase(DIAL_DATA_START_ADDRESS, (1408*1024));
	dev_extFlash_disable();
}
/*void com_dial_test1(DialDataStr *dial)
	{
          
   		dial->Number = 110;
		memcpy(dial->Name, "炫酷运动", 8);
		
		//背景
		dial->Background.type	= DIAL_BACKGROUND_TYPE_DEFAULT;
		dial->Background.color	= LCD_BLACK;
		dial->Background.img.bckgrndcolor	= LCD_NONE;
		dial->Background.img.forecolor		= LCD_NONE;
		dial->Background.img.x_axis 		= 0;
		dial->Background.img.y_axis 		= 0;
		dial->Background.img.width			= img_dial_background_coolsport.width;
		dial->Background.img.height 		= img_dial_background_coolsport.height;
		dial->Background.img.stride 		= img_dial_background_coolsport.width;
		dial->Background.img.imgtype		= img_dial_background_coolsport.imgtype;
		
		//时间
		dial->Time.type 						= DIAL_TIME_TYPE_NUMBER_NO_SECOND;
		dial->Time.align                        =  DIAL_ACTIVEX_ALIGN_CENTER_1;
		dial->Time.hourNum.numImg.bckgrndcolor	= LCD_NONE;
		dial->Time.hourNum.numImg.forecolor 	= LCD_CYAN;
		dial->Time.hourNum.numImg.x_axis		= 120 - 4 - Font_Number_dial_72.height;
		dial->Time.hourNum.numImg.y_axis		= 120;
		dial->Time.hourNum.numImg.width 		= Font_Number_dial_72.width;
		dial->Time.hourNum.numImg.height		= Font_Number_dial_72.height;
		dial->Time.hourNum.numImg.stride		= Font_Number_dial_72.width;
		dial->Time.hourNum.numImg.imgtype		= Font_Number_dial_72.imgtype;
		dial->Time.hourNum.kerning				= 4;
		
		dial->Time.minNum.numImg.bckgrndcolor	= LCD_NONE;
		dial->Time.minNum.numImg.forecolor		= LCD_WHITE;
		dial->Time.minNum.numImg.x_axis 		= 120 + 4;
		dial->Time.minNum.numImg.y_axis 		= 120;
		dial->Time.minNum.numImg.width			= Font_Number_dial_72.width;
		dial->Time.minNum.numImg.height 		= Font_Number_dial_72.height;
		dial->Time.minNum.numImg.stride 		= Font_Number_dial_72.width;
		dial->Time.minNum.numImg.imgtype		= Font_Number_dial_72.imgtype;
		dial->Time.minNum.kerning				= 4;
		dial->Time.gap							= 10;
		
		//日期
		dial->Date.type 					= DIAL_DATE_TYPE_DEFAULT;
		dial->Date.align = DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Date.dateStr.bckgrndcolor 	= LCD_NONE;
		dial->Date.dateStr.forecolor		= LCD_WHITE;
		dial->Date.dateStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Date.dateStr.size 			= LCD_FONT_13_SIZE;
		dial->Date.dateStr.x_axis			= 120 - 4 - Font_Number_dial_72.height - 12 - Font_Number_13.height;
		dial->Date.dateStr.y_axis			= LCD_CENTER_JUSTIFIED;
		dial->Date.dateStr.kerning			= 1;
		dial->Date.dateStr.width			= Font_Number_13.width;
		dial->Date.dateStr.height			= Font_Number_13.height;
		//星期
		dial->Date.weekStr.bckgrndcolor 	= LCD_NONE;
		dial->Date.weekStr.forecolor		= LCD_WHITE;
		dial->Date.weekStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Date.weekStr.size 			= LCD_FONT_13_SIZE;
		dial->Date.weekStr.x_axis			= dial->Date.dateStr.x_axis + Font_Number_13.height - img_week_13x13[0].height;
		dial->Date.weekStr.y_axis			= LCD_CENTER_JUSTIFIED;
		dial->Date.weekStr.kerning			= 3;
		dial->Date.weekStr.width			= img_week_13x13[0].width;
		dial->Date.weekStr.height			= img_week_13x13[0].height;
		dial->Date.gap						= 9;
		
	   //工具栏
		dial->Widget.type 					= DIAL_WIDGET_TYPE_DEFAULT;
		dial->Widget.align                  = DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Widget.gap                    = 4;
		//蓝牙
		dial->Widget.ble.connectImg.bckgrndcolor	= LCD_NONE;
		dial->Widget.ble.connectImg.forecolor 	= LCD_CYAN;
		dial->Widget.ble.connectImg.x_axis 		= 200 + img_dial_weather_lists[0].height/2 - img_dial_ble.height/2 ;
		dial->Widget.ble.connectImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Widget.ble.connectImg.width 		= img_dial_ble.width;
		dial->Widget.ble.connectImg.height	 	= img_dial_ble.height;
		dial->Widget.ble.connectImg.stride 		= img_dial_ble.stride;
		dial->Widget.ble.connectImg.imgtype 		= img_dial_ble.imgtype;
		
		dial->Widget.ble.disconnectImg.bckgrndcolor	= LCD_NONE;
		dial->Widget.ble.disconnectImg.forecolor 	= LCD_WHITE;
		dial->Widget.ble.disconnectImg.x_axis 		= 200 + img_dial_weather_lists[0].height/2 - img_dial_ble.height/2;
		dial->Widget.ble.disconnectImg.y_axis 		= 120+2;
		dial->Widget.ble.disconnectImg.width 		= img_dial_ble.width;
		dial->Widget.ble.disconnectImg.height	 	= img_dial_ble.height;
		dial->Widget.ble.disconnectImg.stride 		= img_dial_ble.stride;
		dial->Widget.ble.disconnectImg.imgtype 		= img_dial_ble.imgtype;


		//闹钟
		dial->Widget.alarmImg.bckgrndcolor 	= LCD_NONE;
		dial->Widget.alarmImg.forecolor 		= LCD_WHITE;
		dial->Widget.alarmImg.x_axis 		= 200 + img_dial_weather_lists[0].height/2 - img_dial_alarm.height/2;
		dial->Widget.alarmImg.y_axis 		= 120+2+img_dial_ble.width+4;
		dial->Widget.alarmImg.width 		= img_dial_alarm.width;
		dial->Widget.alarmImg.height 		= img_dial_alarm.height;
		dial->Widget.alarmImg.stride 		= img_dial_alarm.stride;
		dial->Widget.alarmImg.imgtype 		= img_dial_alarm.imgtype;
		//勿扰状态
	
		dial->Widget.DNDImg.x_axis 		= 0XFF;
	


		 //状态栏

		 
		 //天气
		      //天气图标
			 dial->Widget.dial_weather.d_weatherImg.bckgrndcolor = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherImg.forecolor	 = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherImg.x_axis		 = 200;//204
			 dial->Widget.dial_weather.d_weatherImg.y_axis		 = LCD_CENTER_JUSTIFIED;
			 dial->Widget.dial_weather.d_weatherImg.width		 = img_dial_weather_lists[0].width;
			 dial->Widget.dial_weather.d_weatherImg.height		 = img_dial_weather_lists[0].height;
			 dial->Widget.dial_weather.d_weatherImg.stride		 = img_dial_weather_lists[0].stride;
			 dial->Widget.dial_weather.d_weatherImg.imgtype 	 = img_dial_weather_lists[0].imgtype;
		 
			 //温度数值
			 dial->Widget.dial_weather.d_weatherStr.bckgrndcolor  = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherStr.forecolor	 = LCD_WHITE;
			 dial->Widget.dial_weather.d_weatherStr.type 		 = DIAL_STRING_TYPE_CUSTOM;
			 dial->Widget.dial_weather.d_weatherStr.size 		 = LCD_FONT_13_SIZE;
			 dial->Widget.dial_weather.d_weatherStr.x_axis		 = 200 + img_dial_weather_lists[0].height/2 - 13/2 ;
			 dial->Widget.dial_weather.d_weatherStr.y_axis		 = LCD_CENTER_JUSTIFIED;
			 dial->Widget.dial_weather.d_weatherStr.kerning		 = 1;
			 dial->Widget.dial_weather.d_weatherStr.width		 = 6;
			 dial->Widget.dial_weather.d_weatherStr.height		 = Font_Number_13.height;


		
		//状态栏
		//步数
		dial->Status.Type1.type = DIAL_STATUS_TYPE_8;
		dial->Status.Type1.step.stepImg.bckgrndcolor	= LCD_NONE;
		dial->Status.Type1.step.stepImg.forecolor		= LCD_CYAN;
		dial->Status.Type1.step.stepImg.x_axis		= 120 - 4 - img_dial_step.height;
		dial->Status.Type1.step.stepImg.y_axis		= 40;
		dial->Status.Type1.step.stepImg.width			= img_dial_step.width;
		dial->Status.Type1.step.stepImg.height			= img_dial_step.height;
		dial->Status.Type1.step.stepImg.stride		= img_dial_step.stride;
		dial->Status.Type1.step.stepImg.imgtype 		= img_dial_step.imgtype;
		
		dial->Status.Type1.step.stepStr.bckgrndcolor	= LCD_NONE;
		dial->Status.Type1.step.stepStr.forecolor		= LCD_WHITE;
		dial->Status.Type1.step.stepStr.type			= DIAL_STRING_TYPE_CUSTOM;
		dial->Status.Type1.step.stepStr.size			= LCD_FONT_13_SIZE;
		dial->Status.Type1.step.stepStr.x_axis			= 120;
		dial->Status.Type1.step.stepStr.y_axis		= 40;
		dial->Status.Type1.step.stepStr.kerning 		= 1;
		dial->Status.Type1.step.stepStr.width			= Font_Number_13.width;
		dial->Status.Type1.step.stepStr.height			= Font_Number_13.height;
		dial->Status.Type1.step.gap 					= 0;
		
		//dial->Status.Type1.stepDeg.type 			= DIAL_STEP_DEGREE_TYPE_ARC_1;
		dial->Status.Type1.stepDeg.bckgrndcolor 	    = LCD_LIGHTGRAY;
		dial->Status.Type1.stepDeg.forecolor			= LCD_WHITE;

   
        

		
		//电量
		dial->Battery.type = DIAL_BATTERY_TYPE_4;
		
		
		dial->Battery.batImg.batImg.forecolor	= LCD_LIGHTGRAY;
	    dial->Battery.batImg.batImg.x_axis		= 200 + img_dial_weather_lists[0].height/2 - img_dial_bat_big.height/2;
		dial->Battery.batImg.batImg.y_axis		= LCD_CENTER_JUSTIFIED;
		dial->Battery.batImg.batImg.width		= img_dial_bat_big.width;
		dial->Battery.batImg.batImg.height		= img_dial_bat_big.height;
		dial->Battery.batImg.batImg.stride		= img_dial_bat_big.stride;
		dial->Battery.batImg.batImg.imgtype 	= img_dial_bat_big.imgtype;
		dial->Battery.batImg.x_axis 			= dial->Battery.batImg.batImg.x_axis;
		dial->Battery.batImg.y_axis 			= dial->Battery.batImg.batImg.y_axis;
		dial->Battery.batImg.height 			= dial->Battery.batImg.batImg.height;
		dial->Battery.batImg.width				= img_dial_bat_big.width - 2;
		dial->Battery.batImg.normalColor			= LCD_GREEN;
		dial->Battery.batImg.lowColor			= LCD_RED;
		dial->Battery.gap = 4;
		
		dial->Setting.reserve1[0] = DIAL_STYLE_ELSE_1;
		//心率
		dial->Heart.type = 0xFF;
		
		

	uint8_t index = 0; //存储位置 
	dev_extFlash_enable();  
	dev_extFlash_erase(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), DIAL_DATA_SIZE);
	dev_extFlash_write(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), (uint8_t *)&DialData, sizeof(DialDataStr));	 
	dev_extFlash_write(DIAL_BACKGROUND_IMG_ADDRESS + (index * DIAL_DATA_SIZE) + 0x1000, (uint8_t *)img_dial_background_coolsport.pxdata, img_dial_background_coolsport.height*img_dial_background_coolsport.width);   
	dev_extFlash_write(DIAL_HOUR_IMG_ADDRESS + (index * DIAL_DATA_SIZE) + 0x1000, (uint8_t *)Font_Number_dial_72.pxdata, Font_Number_dial_72.height*10*11);  
	dev_extFlash_write(DIAL_MIN_IMG_ADDRESS + (index * DIAL_DATA_SIZE) + 0x1000, (uint8_t *)Font_Number_dial_72.pxdata, Font_Number_dial_72.height*10*11);	  
	uint32_t flag = 0xFEFFFFFF;	
	dev_extFlash_write(DIAL_DATA_START_ADDRESS + ((index + 1) * DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4); 
	dev_extFlash_disable();

	}*/

/*void com_dial_test2(DialDataStr *dial)
{

   
	dial->Number = 111;
	memcpy(dial->Name, "数字游戏", 8);


	//背景
			dial->Background.type	= DIAL_BACKGROUND_TYPE_DEFAULT;
			dial->Background.color	= LCD_BLACK;
			dial->Background.img.bckgrndcolor	= LCD_NONE;
			dial->Background.img.forecolor		= LCD_NONE;
			dial->Background.img.x_axis 		= 0;
			dial->Background.img.y_axis 		= 0;
		


     


          	//时间
		dial->Time.type 						= DIAL_TIME_TYPE_NUMBER_NO_SECOND;
		dial->Time.align                        =  DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Time.hourNum.numImg.bckgrndcolor	= LCD_NONE;
		dial->Time.hourNum.numImg.forecolor 	= LCD_GREEN;
		dial->Time.hourNum.numImg.x_axis		= 120 - Font_Number_dial_73.height/2;
		dial->Time.hourNum.numImg.y_axis		= LCD_CENTER_JUSTIFIED;
		dial->Time.hourNum.numImg.width 		= Font_Number_dial_73.width;
		dial->Time.hourNum.numImg.height		= Font_Number_dial_73.height;
		dial->Time.hourNum.numImg.stride		= Font_Number_dial_73.width;
		dial->Time.hourNum.numImg.imgtype		= Font_Number_dial_73.imgtype;
		dial->Time.hourNum.kerning				= 2;
		
		dial->Time.colon = 1  ;
		
		dial->Time.minNum.numImg.bckgrndcolor	= LCD_NONE;
		dial->Time.minNum.numImg.forecolor		= LCD_GREEN;
		dial->Time.minNum.numImg.x_axis 		= 120 - Font_Number_dial_73.height/2;
		dial->Time.minNum.numImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Time.minNum.numImg.width			= Font_Number_dial_73.width;
		dial->Time.minNum.numImg.height 		= Font_Number_dial_73.height;
		dial->Time.minNum.numImg.stride 		= Font_Number_dial_73.width;
		dial->Time.minNum.numImg.imgtype		= Font_Number_dial_73.imgtype;
		dial->Time.minNum.kerning				= 2;
		dial->Time.gap							= 24;



      //日期
		dial->Date.type 					= DIAL_DATE_TYPE_CENTER_11;
	    dial->Date.align = DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Date.dateStr.bckgrndcolor 	= LCD_NONE;
		dial->Date.dateStr.forecolor		= LCD_GREEN;
		dial->Date.dateStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Date.dateStr.size 			= LCD_FONT_19_SIZE;
			dial->Date.dateStr.kerning			= 1;
		dial->Date.dateStr.width			= Font_Number_19.width;
		dial->Date.dateStr.height			= Font_Number_19.height;
		dial->Date.dateStr.x_axis			= 120 - Font_Number_dial_73.height/2 - 24 - dial->Date.dateStr.height;
		dial->Date.dateStr.y_axis			= LCD_CENTER_JUSTIFIED;
	
		//星期
		dial->Date.weekStr.bckgrndcolor 	= LCD_NONE;
		dial->Date.weekStr.forecolor		= LCD_GREEN;
		dial->Date.weekStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Date.weekStr.size 			= LCD_FONT_19_SIZE;
		dial->Date.weekStr.x_axis			= dial->Date.dateStr.x_axis + dial->Date.dateStr.height - img_week_20x19[0].height;
		dial->Date.weekStr.y_axis			= LCD_CENTER_JUSTIFIED;
		dial->Date.weekStr.kerning			= 5;
		dial->Date.weekStr.width			= img_week_20x19[0].width;
		dial->Date.weekStr.height			= img_week_20x19[0].height;
		dial->Date.gap						= 10;


    
	//工具栏
	 dial->Widget.type					 = DIAL_WIDGET_TYPE_DEFAULT;
	 dial->Widget.align 				 = DIAL_ACTIVEX_ALIGN_CENTER_0;
	 dial->Widget.gap					 = 6;

	  dial->Widget.dial_weather.d_weatherImg.x_axis= 0xFF;
	   //勿扰状态
	 dial->Widget.DNDImg.bckgrndcolor = LCD_NONE;
	 dial->Widget.DNDImg.forecolor	  = LCD_LIGHTGRAY;
	 dial->Widget.DNDImg.x_axis 	 = 120 - Font_Number_dial_73.height/2 - 24 - dial->Date.dateStr.height - 12 - img_dial_dndisturb.height;
	 dial->Widget.DNDImg.y_axis 	 = LCD_CENTER_JUSTIFIED;
	 dial->Widget.DNDImg.width		 = img_dial_dndisturb.width;
	 dial->Widget.DNDImg.height 	 = img_dial_dndisturb.height;
	 dial->Widget.DNDImg.stride 	 = img_dial_dndisturb.stride;
	 dial->Widget.DNDImg.imgtype	 = img_dial_dndisturb.imgtype;
	
	 //蓝牙
	 dial->Widget.ble.connectImg.bckgrndcolor	 = LCD_NONE;
	 dial->Widget.ble.connectImg.forecolor	 = LCD_GREEN;
	 dial->Widget.ble.connectImg.x_axis 	 = 120 - Font_Number_dial_73.height/2 - 24 - dial->Date.dateStr.height - 12 - img_dial_dndisturb.height/2 - img_dial_ble.height/2 ;
	 dial->Widget.ble.connectImg.y_axis 	 = LCD_CENTER_JUSTIFIED;
	 dial->Widget.ble.connectImg.width		 = img_dial_ble.width;
	 dial->Widget.ble.connectImg.height 	 = img_dial_ble.height;
	 dial->Widget.ble.connectImg.stride 	 = img_dial_ble.stride;
	 dial->Widget.ble.connectImg.imgtype		 = img_dial_ble.imgtype;
	 
	 dial->Widget.ble.disconnectImg.bckgrndcolor = LCD_NONE;
	 dial->Widget.ble.disconnectImg.forecolor	 = LCD_WHITE;
	 dial->Widget.ble.disconnectImg.x_axis		 = 120 - Font_Number_dial_73.height/2 - 24 - dial->Date.dateStr.height - 12 - img_dial_dndisturb.height/2 - img_dial_ble.height/2;
	 dial->Widget.ble.disconnectImg.y_axis		 = 120+2;
	 dial->Widget.ble.disconnectImg.width		 = img_dial_ble.width;
	 dial->Widget.ble.disconnectImg.height		 = img_dial_ble.height;
	 dial->Widget.ble.disconnectImg.stride		 = img_dial_ble.stride;
	 dial->Widget.ble.disconnectImg.imgtype 	 = img_dial_ble.imgtype;
	
	
	 //闹钟
	 dial->Widget.alarmImg.bckgrndcolor  = LCD_NONE;
	 dial->Widget.alarmImg.forecolor	 = LCD_GREEN;
	 dial->Widget.alarmImg.x_axis		 = 120 - Font_Number_dial_73.height/2 - 24 - dial->Date.dateStr.height - 12 - img_dial_dndisturb.height/2 - img_dial_alarm.height/2;
	 dial->Widget.alarmImg.y_axis		 = 120+2+img_dial_ble.width+4;
	 dial->Widget.alarmImg.width		 = img_dial_alarm.width;
	 dial->Widget.alarmImg.height		 = img_dial_alarm.height;
	 dial->Widget.alarmImg.stride		 = img_dial_alarm.stride;
	 dial->Widget.alarmImg.imgtype		 = img_dial_alarm.imgtype;

	
	 	//步数
		dial->Status.Type1.type = DIAL_STATUS_TYPE_9;
	
		
		dial->Status.Type1.step.stepStr.bckgrndcolor	= LCD_NONE;
		dial->Status.Type1.step.stepStr.forecolor		= LCD_GREEN;
		dial->Status.Type1.step.stepStr.type			= DIAL_STRING_TYPE_CUSTOM;
		dial->Status.Type1.step.stepStr.size			= LCD_FONT_13_SIZE;
		dial->Status.Type1.step.stepStr.x_axis			= 120;
		dial->Status.Type1.step.stepStr.y_axis		= 40;
		dial->Status.Type1.step.stepStr.kerning 		= 1;
		dial->Status.Type1.step.stepStr.width			= Font_Number_13.width;
		dial->Status.Type1.step.stepStr.height			= Font_Number_13.height;
		dial->Status.Type1.step.gap 					= 0;
		
		dial->Status.Type1.stepDeg.type = DIAL_PROGRESSBAR_TYPE_5;
		dial->Status.Type1.stepDeg.star_x 				= 180 - 8/2;
		dial->Status.Type1.stepDeg.star_y 				= 120 + 29;
		dial->Status.Type1.stepDeg.height 				= 8;
		dial->Status.Type1.stepDeg.width 				= 64;
		dial->Status.Type1.stepDeg.order				= 1;
		dial->Status.Type1.stepDeg.forecolor  			= LCD_GREEN;
		dial->Status.Type1.stepDeg.bckgrndcolor 		= LCD_LIGHTGRAY;
	


        
		//电量进度条
		    dial->Battery.type = DIAL_BATTERY_TYPE_5;
			dial->Battery.batDeg.star_x = 180 - 8/2;
			dial->Battery.batDeg.star_y = 120 - 8 - 58;
			dial->Battery.batDeg.height = 8;
			dial->Battery.batDeg.width = 64;
			dial->Battery.batDeg.forecolor = LCD_GREEN;
			dial->Battery.batDeg.bckgrndcolor = LCD_LIGHTGRAY;
			dial->Battery.batDeg.order = 1;
            	//值大小
		dial->Battery.batStr.bckgrndcolor		= LCD_NONE;
		dial->Battery.batStr.forecolor			= LCD_GREEN;
		dial->Battery.batStr.type 				= DIAL_STRING_TYPE_CUSTOM;
		dial->Battery.batStr.size 				= LCD_FONT_13_SIZE;
       
		dial->Battery.batStr.x_axis 			 = 180 + 10/2 + 8;
		dial->Battery.batStr.y_axis 			 = 178;
		dial->Battery.batStr.kerning 			= 1;
		dial->Battery.batStr.width				= Font_Number_13.width;
		dial->Battery.batStr.height				= Font_Number_13.height;
       
	   dial->Setting.reserve1[0] = DIAL_ELSE;

	uint8_t index = 0; //存储位置 
	dev_extFlash_enable();  
	dev_extFlash_erase(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), DIAL_DATA_SIZE);
	dev_extFlash_write(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), (uint8_t *)&DialData, sizeof(DialDataStr));	 

	dev_extFlash_write(DIAL_HOUR_IMG_ADDRESS + (index * DIAL_DATA_SIZE) + 0x1000, (uint8_t *)Font_Number_dial_73.pxdata, Font_Number_dial_73.height*11*11);  
	dev_extFlash_write(DIAL_MIN_IMG_ADDRESS + (index * DIAL_DATA_SIZE) + 0x1000, (uint8_t *)Font_Number_dial_73.pxdata, Font_Number_dial_73.height*11*11);	  
	uint32_t flag = 0xFEFFFFFF;	
	dev_extFlash_write(DIAL_DATA_START_ADDRESS + ((index + 1) * DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4); 
	dev_extFlash_disable();
}*/


/*void com_dial_test3(DialDataStr *dial)
{

dial->Number = 112;


//背景
			dial->Background.type	= DIAL_BACKGROUND_TYPE_IMG;
			dial->Background.color	= LCD_BLACK;
			dial->Background.img.bckgrndcolor	= LCD_NONE;
			dial->Background.img.forecolor		= LCD_NONE;
			dial->Background.img.x_axis 		= 0;
			dial->Background.img.y_axis 		= 0;
			dial->Background.img.width			= img_dial_background_businesspointer.width;
			dial->Background.img.height 		= img_dial_background_businesspointer.height;
			dial->Background.img.stride 		= img_dial_background_businesspointer.width;
			dial->Background.img.imgtype		= img_dial_background_businesspointer.imgtype;


	   
	  // dial->Time.type						   = DIAL_TIME_TYPE_NUMBER_NO_SECOND;

     
      //日期
		dial->Date.type 					= DIAL_DATE_TYPE_CENTER_10;
	    dial->Date.align = DIAL_ACTIVEX_ALIGN_RIGHT_0;
		dial->Date.dateStr.bckgrndcolor 	= LCD_NONE;
		dial->Date.dateStr.forecolor		= LCD_CYAN;
		dial->Date.dateStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Date.dateStr.size 			= LCD_FONT_13_SIZE;
		dial->Date.dateStr.x_axis			= 120 -  Font_Number_13.height/2;
		dial->Date.dateStr.y_axis			= 208;
		dial->Date.dateStr.kerning			= 1;
		dial->Date.dateStr.width			= Font_Number_13.width;
		dial->Date.dateStr.height			= Font_Number_13.height;


	   dial->Widget.DNDImg.x_axis = 0xFF;
	   dial->Widget.alarmImg.x_axis = 0xFF;
	   dial->Widget.ble.connectImg.x_axis = 0xFF;
	   dial->Widget.dial_weather.d_weatherImg.x_axis = 0xFF;

  //电池/电量
		dial->Battery.type = DIAL_BATTERY_TYPE_6;
		
		
		dial->Battery.batImg.batImg.forecolor	= LCD_LIGHTGRAY;
		
		dial->Battery.batStr.bckgrndcolor		= LCD_NONE;
	    dial->Battery.batImg.batImg.x_axis		= 200 - img_dial_bat_big.height;
		dial->Battery.batImg.batImg.y_axis		= LCD_CENTER_JUSTIFIED;
		dial->Battery.batImg.batImg.width		= img_dial_bat_big.width;
		dial->Battery.batImg.batImg.height		= img_dial_bat_big.height;
		dial->Battery.batImg.batImg.stride		= img_dial_bat_big.stride;
		dial->Battery.batImg.batImg.imgtype 	= img_dial_bat_big.imgtype;
		dial->Battery.batImg.width				= img_dial_bat_big.width - 2;
		dial->Battery.batImg.normalColor		 	= LCD_LIGHTGRAY;
		dial->Battery.batImg.lowColor 			=  LCD_RED;
		dial->Battery.gap = 4;
           	//值大小
		dial->Battery.batStr.bckgrndcolor		= LCD_NONE;
		dial->Battery.batStr.forecolor			= LCD_LIGHTGRAY;
		dial->Battery.batStr.type 				= DIAL_STRING_TYPE_CUSTOM;
		dial->Battery.batStr.size 				= LCD_FONT_9_SIZE;
       
		dial->Battery.batStr.x_axis 			 = 200 - img_dial_bat_big.height/2 - Font_Number_9.height/2;
		dial->Battery.batStr.y_axis 			 = 120 - img_dial_bat_big.width/2;
		dial->Battery.batStr.kerning 			= 1;
		dial->Battery.batStr.width				= Font_Number_9.width;
		dial->Battery.batStr.height				= Font_Number_9.height;


   //步数
	
	
		dial->Status.Type1.type = DIAL_STATUS_TYPE_DEFAULT_1;
		dial->Status.Type1.align1 = DIAL_ACTIVEX_ALIGN_CENTER_1;
		dial->Status.Type1.step.stepImg.bckgrndcolor	= LCD_NONE;
		dial->Status.Type1.step.stepImg.forecolor 		= LCD_RED;
		dial->Status.Type1.step.stepImg.x_axis 			= 188 - 16 - img_dial_step_big.height ;
		dial->Status.Type1.step.stepImg.y_axis 			= 120 - 8 - img_dial_step_big.width;
		dial->Status.Type1.step.stepImg.width 			= img_dial_step_big.width;
		dial->Status.Type1.step.stepImg.height	 		= img_dial_step_big.height;
		dial->Status.Type1.step.stepImg.stride 			= img_dial_step_big.stride;
		dial->Status.Type1.step.stepImg.imgtype 		= img_dial_step_big.imgtype;
		
		dial->Status.Type1.step.stepStr.bckgrndcolor 	= LCD_NONE;
		dial->Status.Type1.step.stepStr.forecolor 		= LCD_LIME;
		dial->Status.Type1.step.stepStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Status.Type1.step.stepStr.size 			= LCD_FONT_19_SIZE;
		dial->Status.Type1.step.stepStr.x_axis	 		= 188 - 16 - Font_Number_19.height;
		dial->Status.Type1.step.stepStr.y_axis 			= 120;
		dial->Status.Type1.step.stepStr.kerning 		= 1;
		dial->Status.Type1.step.stepStr.width			= Font_Number_19.width;
		dial->Status.Type1.step.stepStr.height			= Font_Number_19.height;
		dial->Status.Type1.step.gap 					= 0;
	
		

	 
		 //时间
	 dial->Time.type = DIAL_TIME_TYPE_WATCHHAND_6;


 
     uint8_t index = 0; //存储位置 
	 dev_extFlash_enable();  
	 dev_extFlash_erase(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), DIAL_DATA_SIZE);
	 dev_extFlash_write(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), (uint8_t *)&DialData, sizeof(DialDataStr));   
	 dev_extFlash_write(DIAL_BACKGROUND_IMG_ADDRESS + (index * DIAL_DATA_SIZE) + 0x1000, (uint8_t *)img_dial_background_businesspointer.pxdata, img_dial_background_businesspointer.height*img_dial_background_businesspointer.width);   
	
	 uint32_t flag = 0xFEFFFFFF; 
	 dev_extFlash_write(DIAL_DATA_START_ADDRESS + ((index + 1) * DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4); 
	 dev_extFlash_disable();


}*/
void com_dial_cod_test1(DialDataStr *dial)
{
	dial->Number = 110;
	memcpy(dial->Name, "商务指针", 8);

	//背景
	//dial->Background.type	= DIAL_BACKGROUND_TYPE_IMG;
	
	dial->Background.type	= DIAL_BACKGROUND_TYPE_DEFAULT;
	dial->Background.color	= LCD_BLACK;
	/*dial->Background.img.bckgrndcolor	= LCD_NONE;
	dial->Background.img.forecolor		= LCD_NONE;
	dial->Background.img.x_axis 		= 0;
	dial->Background.img.y_axis 		= 0;
	dial->Background.img.width			= img_dial_background_cod_business.width;
	dial->Background.img.height 		= img_dial_background_cod_business.height;
	dial->Background.img.stride 		= img_dial_background_cod_business.width;
	dial->Background.img.imgtype		= img_dial_background_cod_business.imgtype;*/


	//日期
				dial->Date.type 					= DIAL_DATE_TYPE_CENTER_10;
				dial->Date.align					= DIAL_ACTIVEX_ALIGN_6;
				dial->Date.gap=4; 
				dial->Date.dateStr.bckgrndcolor 	= LCD_NONE;
				dial->Date.dateStr.forecolor		= LCD_BLACK;
				dial->Date.dateStr.type 			= DIAL_STRING_TYPE_CUSTOM;
				
				dial->Date.dateStr.size 			= LCD_FONT_12_SIZE;
				dial->Date.dateStr.x_axis		= 120 - Font_Number_12.height/2;
				dial->Date.dateStr.y_axis		= 192;
				dial->Date.dateStr.kerning		= 1;
				dial->Date.dateStr.width		= Font_Number_12.width;
				dial->Date.dateStr.height		= Font_Number_12.height;
				



  //电池非独立放置


  	//工具栏
	 dial->Widget.type					 = DIAL_WIDGET_TYPE_DEFAULT;
	 dial->Widget.align 				 = DIAL_ACTIVEX_ALIGN_CENTER_0;
	 dial->Widget.gap					 = 4;


	 //蓝牙
	 dial->Widget.ble.connectImg.bckgrndcolor	 = LCD_NONE;
	 dial->Widget.ble.connectImg.forecolor	 = LCD_DARKTURQUOISE;
	 dial->Widget.ble.connectImg.x_axis 	 = 196 -img_dial_bat_28x13.height/2 - img_dial_ble.height/2 ;
	 dial->Widget.ble.connectImg.y_axis 	 = LCD_CENTER_JUSTIFIED;
	 dial->Widget.ble.connectImg.width		 = img_dial_ble.width;
	 dial->Widget.ble.connectImg.height 	 = img_dial_ble.height;
	 dial->Widget.ble.connectImg.stride 	 = img_dial_ble.stride;
	 dial->Widget.ble.connectImg.imgtype		 = img_dial_ble.imgtype;
	 
	 dial->Widget.ble.disconnectImg.bckgrndcolor = LCD_NONE;
	 dial->Widget.ble.disconnectImg.forecolor	 = LCD_WHITE;
	 dial->Widget.ble.disconnectImg.x_axis		 = 196 -img_dial_bat_28x13.height/2 - img_dial_ble.height/2;
	 dial->Widget.ble.disconnectImg.y_axis		 = 120+2;
	 dial->Widget.ble.disconnectImg.width		 = img_dial_ble.width;
	 dial->Widget.ble.disconnectImg.height		 = img_dial_ble.height;
	 dial->Widget.ble.disconnectImg.stride		 = img_dial_ble.stride;
	 dial->Widget.ble.disconnectImg.imgtype 	 = img_dial_ble.imgtype;
		   //勿扰状态
	 dial->Widget.DNDImg.bckgrndcolor = LCD_NONE;
	 dial->Widget.DNDImg.forecolor	  = LCD_LIGHTGRAY;
	 dial->Widget.DNDImg.x_axis 	 = 196 -img_dial_bat_28x13.height/2 - img_dial_dndisturb.height/2;
	 dial->Widget.DNDImg.y_axis 	 = LCD_CENTER_JUSTIFIED;
	 dial->Widget.DNDImg.width		 = img_dial_dndisturb.width;
	 dial->Widget.DNDImg.height 	 = img_dial_dndisturb.height;
	 dial->Widget.DNDImg.stride 	 = img_dial_dndisturb.stride;
	 dial->Widget.DNDImg.imgtype	 = img_dial_dndisturb.imgtype;
	
	
	 //闹钟
	 dial->Widget.alarmImg.bckgrndcolor  = LCD_NONE;
	 dial->Widget.alarmImg.forecolor	 = LCD_WHITE;
	 dial->Widget.alarmImg.x_axis		 = 196 - img_dial_bat_28x13.height/2 - img_dial_alarm.height/2;
	 dial->Widget.alarmImg.y_axis		 = 120+2+img_dial_ble.width+4;
	 dial->Widget.alarmImg.width		 = img_dial_alarm.width;
	 dial->Widget.alarmImg.height		 = img_dial_alarm.height;
	 dial->Widget.alarmImg.stride		 = img_dial_alarm.stride;
	 dial->Widget.alarmImg.imgtype		 = img_dial_alarm.imgtype;

		dial->Widget.dial_weather.d_weatherImg.x_axis = 0xFF;//无天气
		//电量
		
		dial->Battery.type = DIAL_BATTERY_TYPE_4;
		dial->Battery.batImg.batImg.bckgrndcolor	= LCD_NONE;
		dial->Battery.batImg.batImg.forecolor	=  LCD_WHITE;
		dial->Battery.batImg.batImg.x_axis 		= 196 -  img_dial_bat_28x13.height;
		dial->Battery.batImg.batImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Battery.batImg.batImg.width 		= img_dial_bat_28x13.width;
		dial->Battery.batImg.batImg.height	 	= img_dial_bat_28x13.height;
		dial->Battery.batImg.batImg.stride 		= img_dial_bat_28x13.stride;
		dial->Battery.batImg.batImg.imgtype 	= img_dial_bat_28x13.imgtype;
        dial->Battery.batImg.width 				= img_dial_bat_28x13.width - 2;
		dial->Battery.batImg.normalColor		 	= LCD_DARKTURQUOISE;
		dial->Battery.batImg.lowColor 			=  LCD_RED;
		dial->Battery.gap = 4;


	//步数
		dial->Status.Type1.type = DIAL_STATUS_TYPE_DEFAULT_1;
		dial->Status.Type1.align1 = DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Status.Type1.step.stepImg.bckgrndcolor	= LCD_NONE;
		dial->Status.Type1.step.stepImg.forecolor 		= LCD_GREEN;
		dial->Status.Type1.step.stepImg.x_axis 			= 172 - img_dial_step_16x16.height ;
		dial->Status.Type1.step.stepImg.y_axis 			= LCD_CENTER_JUSTIFIED;
		dial->Status.Type1.step.stepImg.width 			= img_dial_step_16x16.width;
		dial->Status.Type1.step.stepImg.height	 		= img_dial_step_16x16.height;
		dial->Status.Type1.step.stepImg.stride 			= img_dial_step_16x16.stride;
		dial->Status.Type1.step.stepImg.imgtype 		= img_dial_step_16x16.imgtype;
		//规格
		dial->Status.Type1.step.stepStr.bckgrndcolor 	= LCD_NONE;
		dial->Status.Type1.step.stepStr.forecolor 		= LCD_WHITE;
		dial->Status.Type1.step.stepStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Status.Type1.step.stepStr.size 			= LCD_FONT_16_1_SIZE;
		dial->Status.Type1.step.stepStr.x_axis	 		= 172 - Font_Number_16_1.height;
		dial->Status.Type1.step.stepStr.y_axis 			= LCD_CENTER_JUSTIFIED;
		dial->Status.Type1.step.stepStr.kerning 		= 1;
		dial->Status.Type1.step.stepStr.width			= Font_Number_16_1.width;
		dial->Status.Type1.step.stepStr.height			= Font_Number_16_1.height;
		dial->Status.Type1.step.gap 					= 2;
//天气
	//dial->Weather.weather.stepImg.x_axis = 0xFF;

	//时间
	dial->Time.type = DIAL_TIME_TYPE_WATCHHAND_6;
  
		
	dial->Setting.reserve1[0] = DIAL_STYLE_ELSE_1;


	 uint8_t index = 0; //存储位置 
	 dev_extFlash_enable();  
	 dev_extFlash_erase(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), DIAL_DATA_SIZE);
	 dev_extFlash_write(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), (uint8_t *)&DialData, sizeof(DialDataStr));   
//	 dev_extFlash_write(DIAL_BACKGROUND_IMG_ADDRESS + (index * DIAL_DATA_SIZE) + 0x1000, (uint8_t *)img_dial_background_cod_business.pxdata, img_dial_background_cod_business.height*img_dial_background_cod_business.width);   
	
	 uint32_t flag = 0xFEFFFFFF; 
	 dev_extFlash_write(DIAL_DATA_START_ADDRESS + ((index + 1) * DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4); 
	 dev_extFlash_disable();


}
/*void com_dial_cod_test2(DialDataStr *dial)
	{
			  
			dial->Number = 111;
			memcpy(dial->Name, "经典数字", 8);
			
			//背景
			dial->Background.type	= DIAL_BACKGROUND_TYPE_DEFAULT;
			dial->Background.color	= LCD_BLACK;
		
			
			//时间
			dial->Time.type 						= DIAL_TIME_TYPE_NUMBER_NO_SECOND;
			dial->Time.align						=  DIAL_ACTIVEX_ALIGN_CENTER_1;
			dial->Time.hourNum.numImg.bckgrndcolor	= LCD_NONE;
			dial->Time.hourNum.numImg.forecolor 	= LCD_CYAN;
			dial->Time.hourNum.numImg.x_axis		= 120 - 4 - Font_Number_dial_72.height;
			dial->Time.hourNum.numImg.y_axis		= 120;
			dial->Time.hourNum.numImg.width 		= Font_Number_dial_72.width;
			dial->Time.hourNum.numImg.height		= Font_Number_dial_72.height;
			dial->Time.hourNum.numImg.stride		= Font_Number_dial_72.width;
			dial->Time.hourNum.numImg.imgtype		= Font_Number_dial_72.imgtype;
			dial->Time.hourNum.kerning				= 4;
			
			dial->Time.minNum.numImg.bckgrndcolor	= LCD_NONE;
			dial->Time.minNum.numImg.forecolor		= LCD_WHITE;
			dial->Time.minNum.numImg.x_axis 		= 120 + 4;
			dial->Time.minNum.numImg.y_axis 		= 120;
			dial->Time.minNum.numImg.width			= Font_Number_dial_72.width;
			dial->Time.minNum.numImg.height 		= Font_Number_dial_72.height;
			dial->Time.minNum.numImg.stride 		= Font_Number_dial_72.width;
			dial->Time.minNum.numImg.imgtype		= Font_Number_dial_72.imgtype;
			dial->Time.minNum.kerning				= 4;
			dial->Time.gap							= 10;
			
			//日期
			dial->Date.type 					= DIAL_DATE_TYPE_CENTER_11;
			dial->Date.align = DIAL_ACTIVEX_ALIGN_CENTER_0;
			dial->Date.dateStr.bckgrndcolor 	= LCD_NONE;
			dial->Date.dateStr.forecolor		= LCD_WHITE;
			dial->Date.dateStr.type 			= DIAL_STRING_TYPE_CUSTOM;
			dial->Date.dateStr.size 			= LCD_FONT_13_SIZE;
			dial->Date.dateStr.x_axis			= 120 - 4 - Font_Number_dial_72.height - 12 - Font_Number_13.height;
			dial->Date.dateStr.y_axis			= LCD_CENTER_JUSTIFIED;
			dial->Date.dateStr.kerning			= 1;
			dial->Date.dateStr.width			= Font_Number_13.width;
			dial->Date.dateStr.height			= Font_Number_13.height;
			//星期
			dial->Date.weekStr.bckgrndcolor 	= LCD_NONE;
			dial->Date.weekStr.forecolor		= LCD_WHITE;
			dial->Date.weekStr.type 			= DIAL_STRING_TYPE_CUSTOM;
			dial->Date.weekStr.size 			= LCD_FONT_13_SIZE;
			dial->Date.weekStr.x_axis			= dial->Date.dateStr.x_axis + Font_Number_13.height - img_week_13x13[0].height;
			dial->Date.weekStr.y_axis			= LCD_CENTER_JUSTIFIED;
			dial->Date.weekStr.kerning			= 3;
			dial->Date.weekStr.width			= img_week_13x13[0].width;
			dial->Date.weekStr.height			= img_week_13x13[0].height;
			dial->Date.gap						= 9;
			
		   //工具栏
			dial->Widget.type					= DIAL_WIDGET_TYPE_DEFAULT;
			dial->Widget.align					= DIAL_ACTIVEX_ALIGN_CENTER_0;
			dial->Widget.gap					= 4;
			//蓝牙
			dial->Widget.ble.connectImg.bckgrndcolor	= LCD_NONE;
			dial->Widget.ble.connectImg.forecolor	= LCD_CYAN;
			dial->Widget.ble.connectImg.x_axis		= 200 + img_dial_weather_lists[0].height/2 - img_dial_ble.height/2 ;
			dial->Widget.ble.connectImg.y_axis		= LCD_CENTER_JUSTIFIED;
			dial->Widget.ble.connectImg.width		= img_dial_ble.width;
			dial->Widget.ble.connectImg.height		= img_dial_ble.height;
			dial->Widget.ble.connectImg.stride		= img_dial_ble.stride;
			dial->Widget.ble.connectImg.imgtype 		= img_dial_ble.imgtype;
			
			dial->Widget.ble.disconnectImg.bckgrndcolor = LCD_NONE;
			dial->Widget.ble.disconnectImg.forecolor	= LCD_WHITE;
			dial->Widget.ble.disconnectImg.x_axis		= 200 + img_dial_weather_lists[0].height/2 - img_dial_ble.height/2;
			dial->Widget.ble.disconnectImg.y_axis		= 120+2;
			dial->Widget.ble.disconnectImg.width		= img_dial_ble.width;
			dial->Widget.ble.disconnectImg.height		= img_dial_ble.height;
			dial->Widget.ble.disconnectImg.stride		= img_dial_ble.stride;
			dial->Widget.ble.disconnectImg.imgtype		= img_dial_ble.imgtype;
	
	
			//闹钟
			dial->Widget.alarmImg.bckgrndcolor	= LCD_NONE;
			dial->Widget.alarmImg.forecolor 		= LCD_WHITE;
			dial->Widget.alarmImg.x_axis		= 200 + img_dial_weather_lists[0].height/2 - img_dial_alarm.height/2;
			dial->Widget.alarmImg.y_axis		= 120+2+img_dial_ble.width+4;
			dial->Widget.alarmImg.width 		= img_dial_alarm.width;
			dial->Widget.alarmImg.height		= img_dial_alarm.height;
			dial->Widget.alarmImg.stride		= img_dial_alarm.stride;
			dial->Widget.alarmImg.imgtype		= img_dial_alarm.imgtype;
			//勿扰状态
		
			dial->Widget.DNDImg.x_axis		= 0XFF;
		
	
	
			 //状态栏
	
			 
			 //天气
				  //天气图标
				  dial->Weather.type = DIAL_WEATHER_TYPE_3;
				 dial->Widget.dial_weather.d_weatherImg.bckgrndcolor = LCD_NONE;
				 dial->Widget.dial_weather.d_weatherImg.forecolor	 = LCD_NONE;
				 dial->Widget.dial_weather.d_weatherImg.x_axis		 = 200;//204
				 dial->Widget.dial_weather.d_weatherImg.y_axis		 = LCD_CENTER_JUSTIFIED;
				 dial->Widget.dial_weather.d_weatherImg.width		 = img_dial_weather_lists[0].width;
				 dial->Widget.dial_weather.d_weatherImg.height		 = img_dial_weather_lists[0].height;
				 dial->Widget.dial_weather.d_weatherImg.stride		 = img_dial_weather_lists[0].stride;
				 dial->Widget.dial_weather.d_weatherImg.imgtype 	 = img_dial_weather_lists[0].imgtype;
			 
				 //温度数值
				 dial->Widget.dial_weather.d_weatherStr.bckgrndcolor  = LCD_NONE;
				 dial->Widget.dial_weather.d_weatherStr.forecolor	 = LCD_WHITE;
				 dial->Widget.dial_weather.d_weatherStr.type		 = DIAL_STRING_TYPE_CUSTOM;
				 dial->Widget.dial_weather.d_weatherStr.size		 = LCD_FONT_13_SIZE;
				 dial->Widget.dial_weather.d_weatherStr.x_axis		 = 200 + img_dial_weather_lists[0].height/2 - 13/2 ;
				 dial->Widget.dial_weather.d_weatherStr.y_axis		 = LCD_CENTER_JUSTIFIED;
				 dial->Widget.dial_weather.d_weatherStr.kerning 	 = 1;
				 dial->Widget.dial_weather.d_weatherStr.width		 = 6;
				 dial->Widget.dial_weather.d_weatherStr.height		 = Font_Number_13.height;
	
	
			
			//状态栏
			//步数
			dial->Status.Type1.type = DIAL_STATUS_TYPE_8;
			dial->Status.Type1.step.stepImg.bckgrndcolor	= LCD_NONE;
			dial->Status.Type1.step.stepImg.forecolor		= LCD_CYAN;
			dial->Status.Type1.step.stepImg.x_axis		= 120 - 4 - img_dial_step_16x16.height;
			dial->Status.Type1.step.stepImg.y_axis		= 40;
			dial->Status.Type1.step.stepImg.width			= img_dial_step_16x16.width;
			dial->Status.Type1.step.stepImg.height			= img_dial_step_16x16.height;
			dial->Status.Type1.step.stepImg.stride		    = img_dial_step_16x16.stride;
			dial->Status.Type1.step.stepImg.imgtype 		= img_dial_step_16x16.imgtype;
			
			dial->Status.Type1.step.stepStr.bckgrndcolor	= LCD_NONE;
			dial->Status.Type1.step.stepStr.forecolor		= LCD_WHITE;
			dial->Status.Type1.step.stepStr.type			= DIAL_STRING_TYPE_CUSTOM;
			dial->Status.Type1.step.stepStr.size			= LCD_FONT_13_SIZE;
			dial->Status.Type1.step.stepStr.x_axis			= 120;
			dial->Status.Type1.step.stepStr.y_axis		= 40;
			dial->Status.Type1.step.stepStr.kerning 		= 1;
			dial->Status.Type1.step.stepStr.width			= Font_Number_13.width;
			dial->Status.Type1.step.stepStr.height			= Font_Number_13.height;
			dial->Status.Type1.step.gap 					= 0;
			
			dial->Status.Type1.stepDeg.bckgrndcolor 		= LCD_LIGHTGRAY;
			dial->Status.Type1.stepDeg.forecolor			= LCD_WHITE;
	
	   
			
	
			
			//电量
			dial->Battery.type = DIAL_BATTERY_TYPE_4;
			
			
			dial->Battery.batImg.batImg.forecolor	= LCD_LIGHTGRAY;
			dial->Battery.batImg.batImg.x_axis		= 200 + img_dial_weather_lists[0].height/2 - img_dial_bat_28x13.height/2;
			dial->Battery.batImg.batImg.y_axis		= LCD_CENTER_JUSTIFIED;
			dial->Battery.batImg.batImg.width		= img_dial_bat_28x13.width;
			dial->Battery.batImg.batImg.height		= img_dial_bat_28x13.height;
			dial->Battery.batImg.batImg.stride		= img_dial_bat_28x13.stride;
			dial->Battery.batImg.batImg.imgtype 	= img_dial_bat_28x13.imgtype;
			dial->Battery.batImg.x_axis 			= dial->Battery.batImg.batImg.x_axis;
			dial->Battery.batImg.y_axis 			= dial->Battery.batImg.batImg.y_axis;
			dial->Battery.batImg.height 			= dial->Battery.batImg.batImg.height;
			dial->Battery.batImg.width				= img_dial_bat_28x13.width - 2;
			dial->Battery.batImg.normalColor			= LCD_CYAN;
			dial->Battery.batImg.lowColor			= LCD_RED;
			dial->Battery.gap = 4;
			
			dial->Setting.reserve1[0] = DIAL_STYLE_ELSE_1;
			//心率
			dial->Heart.type = 0xFF;
			
			uint8_t index = 0; //存储位置 
				dev_extFlash_enable();	
				dev_extFlash_erase(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), DIAL_DATA_SIZE);
				dev_extFlash_write(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), (uint8_t *)&DialData, sizeof(DialDataStr));	 ;   
				
				uint32_t flag = 0xFEFFFFFF; 
				dev_extFlash_write(DIAL_DATA_START_ADDRESS + ((index + 1) * DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4); 
				dev_extFlash_disable();

	
	
	
		}*/


/*void com_dial_cod_test3(DialDataStr *dial)
	{
				  
				dial->Number = 112;
				memcpy(dial->Name, "运动休闲", 8);
				
				//背景
				dial->Background.type	= DIAL_BACKGROUND_TYPE_DEFAULT;
				dial->Background.color	= LCD_BLACK;
			
				
				//时间
				dial->Time.type 						= DIAL_TIME_TYPE_NUMBER_NO_SECOND_3;
				dial->Time.align						=  DIAL_ACTIVEX_ALIGN_CENTER_0;
				dial->Time.hourNum.numImg.bckgrndcolor	= LCD_NONE;
				dial->Time.hourNum.numImg.forecolor 	= LCD_GRAY;
				dial->Time.hourNum.numImg.x_axis		= 120 - Font_Number_41.height/2;
				dial->Time.hourNum.numImg.y_axis		= 120;
				dial->Time.hourNum.numImg.width 		= Font_Number_41.width;
				dial->Time.hourNum.numImg.height		= Font_Number_41.height;
				dial->Time.hourNum.numImg.stride		= Font_Number_41.width;
				dial->Time.hourNum.numImg.imgtype		= Font_Number_41.imgtype;
				dial->Time.hourNum.kerning				= 1;
				
				dial->Time.minNum.numImg.bckgrndcolor	= LCD_NONE;
				dial->Time.minNum.numImg.forecolor		= LCD_WHITE;
				dial->Time.minNum.numImg.x_axis 		= 120 - Font_Number_41.height/2;
				dial->Time.minNum.numImg.y_axis 		= 120;
				dial->Time.minNum.numImg.width			= Font_Number_41.width;
				dial->Time.minNum.numImg.height 		= Font_Number_41.height;
				dial->Time.minNum.numImg.stride 		= Font_Number_41.width;
				dial->Time.minNum.numImg.imgtype		= Font_Number_41.imgtype;
				dial->Time.minNum.kerning				= 1;
				dial->Time.gap							= 12;
				
				//日期
				dial->Date.type 					= DIAL_DATE_TYPE_DEFAULT;
				dial->Date.align = DIAL_ACTIVEX_ALIGN_6;
				dial->Date.dateStr.bckgrndcolor 	= LCD_NONE;
				dial->Date.dateStr.forecolor		= LCD_WHITE;
				dial->Date.dateStr.type 			= DIAL_STRING_TYPE_CUSTOM;
				dial->Date.dateStr.size 			= LCD_FONT_16_1_SIZE;
				dial->Date.dateStr.x_axis			= 120  - Font_Number_41.height/2 - 18 - 8 -img_week_13x13[0].height - 4 - Font_Number_16_1.height;
				dial->Date.dateStr.y_axis			= 36;
				dial->Date.dateStr.kerning			= 1;
				dial->Date.dateStr.width			= Font_Number_16_1.width;
				dial->Date.dateStr.height			= Font_Number_16_1.height;
				//星期
				dial->Date.weekStr.bckgrndcolor 	= LCD_NONE;
				dial->Date.weekStr.forecolor		= LCD_WHITE;
				dial->Date.weekStr.type 			= DIAL_STRING_TYPE_CUSTOM;
				dial->Date.weekStr.size 			= LCD_FONT_13_SIZE;
				dial->Date.weekStr.x_axis			= dial->Date.dateStr.x_axis + dial->Date.dateStr.height + 4;
				dial->Date.weekStr.y_axis			= 36;
				dial->Date.weekStr.kerning			= 3;
				dial->Date.weekStr.width			= img_week_13x13[0].width;
				dial->Date.weekStr.height			= img_week_13x13[0].height;
				dial->Date.gap						= 8;
				
				//工具栏
			dial->Widget.type 					= DIAL_WIDGET_TYPE_DEFAULT;
			dial->Widget.align                  = DIAL_ACTIVEX_ALIGN_7;
			dial->Widget.gap                    = 4;
			  //勿扰状态
			 dial->Widget.DNDImg.bckgrndcolor = LCD_NONE;
			 dial->Widget.DNDImg.forecolor	  = LCD_GRAY;
			 dial->Widget.DNDImg.x_axis 	 = 120  - Font_Number_41.height/2 - 26 - img_dial_dndisturb.height;//217
			 dial->Widget.DNDImg.y_axis 	 = 192;
			 dial->Widget.DNDImg.width		 = img_dial_dndisturb.width;
			 dial->Widget.DNDImg.height 	 = img_dial_dndisturb.height;
			 dial->Widget.DNDImg.stride 	 = img_dial_dndisturb.stride;
			 dial->Widget.DNDImg.imgtype	 = img_dial_dndisturb.imgtype;
			//蓝牙
			dial->Widget.ble.connectImg.bckgrndcolor	= LCD_NONE;
			dial->Widget.ble.connectImg.forecolor 	= LCD_CYAN;
			
			dial->Widget.ble.connectImg.x_axis 		= 120  - Font_Number_41.height/2 - 26  - img_dial_dndisturb.height/2 - img_dial_ble.height/2;
			dial->Widget.ble.connectImg.y_axis 		= 192;
			dial->Widget.ble.connectImg.width 		= img_dial_ble.width;
			dial->Widget.ble.connectImg.height	 	= img_dial_ble.height;
			dial->Widget.ble.connectImg.stride 		= img_dial_ble.stride;
			dial->Widget.ble.connectImg.imgtype 		= img_dial_ble.imgtype;

			//img_dial_ble
			dial->Widget.ble.disconnectImg.bckgrndcolor	= LCD_NONE;
			dial->Widget.ble.disconnectImg.forecolor 	= LCD_WHITE;
			
			dial->Widget.ble.disconnectImg.x_axis 		= 120  - Font_Number_41.height/2 - 26  - img_dial_dndisturb.height/2 - img_dial_ble.height/2;
			dial->Widget.ble.disconnectImg.y_axis 		= 192;
			dial->Widget.ble.disconnectImg.width 		= img_dial_ble.width;
			dial->Widget.ble.disconnectImg.height	 	= img_dial_ble.height;
			dial->Widget.ble.disconnectImg.stride 		= img_dial_ble.stride;
			dial->Widget.ble.disconnectImg.imgtype 		= img_dial_ble.imgtype;
	       

			//闹钟
			dial->Widget.alarmImg.bckgrndcolor 	= LCD_NONE;
			dial->Widget.alarmImg.forecolor 		= LCD_WHITE;
			dial->Widget.alarmImg.x_axis 		= 120  - Font_Number_41.height/2 - 26  - img_dial_dndisturb.height/2 - img_dial_alarm.height/2;
			dial->Widget.alarmImg.y_axis 		= 192;
			dial->Widget.alarmImg.width 		= img_dial_alarm.width;
			dial->Widget.alarmImg.height 		= img_dial_alarm.height;
			dial->Widget.alarmImg.stride 		= img_dial_alarm.stride;
			dial->Widget.alarmImg.imgtype 		= img_dial_alarm.imgtype;
				
		
		
				 //状态栏
		
				 
				 //天气
					    //天气图标
					 dial->Weather.type = DIAL_WEATHER_TYPE_4;
					 dial->Widget.dial_weather.d_weatherImg.bckgrndcolor = LCD_NONE;
					 dial->Widget.dial_weather.d_weatherImg.forecolor	 = LCD_NONE;
					 dial->Widget.dial_weather.d_weatherImg.x_axis		 = 120  - Font_Number_41.height/2 - 22 -  8 - Font_Number_16_1.height - 4 - img_Weather_lists[0].height ;
					 dial->Widget.dial_weather.d_weatherImg.y_axis		 = LCD_CENTER_JUSTIFIED;
					 dial->Widget.dial_weather.d_weatherImg.width		 = img_Weather_lists[0].width;
					 dial->Widget.dial_weather.d_weatherImg.height		 = img_Weather_lists[0].height;
					 dial->Widget.dial_weather.d_weatherImg.stride		 = img_Weather_lists[0].stride;
					 dial->Widget.dial_weather.d_weatherImg.imgtype 	 = img_Weather_lists[0].imgtype;
				 
					 //温度数值
					 dial->Widget.dial_weather.d_weatherStr.bckgrndcolor  = LCD_NONE;
					 dial->Widget.dial_weather.d_weatherStr.forecolor	 = LCD_WHITE;
					 dial->Widget.dial_weather.d_weatherStr.type		 = DIAL_STRING_TYPE_CUSTOM;
					 dial->Widget.dial_weather.d_weatherStr.size		 = LCD_FONT_16_1_SIZE;
					 dial->Widget.dial_weather.d_weatherStr.x_axis		 = dial->Widget.dial_weather.d_weatherImg.x_axis + img_Weather_lists[0].height + 4 ;
					 dial->Widget.dial_weather.d_weatherStr.y_axis		 = LCD_CENTER_JUSTIFIED;
					 dial->Widget.dial_weather.d_weatherStr.kerning 	 = 1;
					 dial->Widget.dial_weather.d_weatherStr.width		 = Font_Number_16_1.width;
					 dial->Widget.dial_weather.d_weatherStr.height		 = Font_Number_16_1.height;
		
		
				
			
		
		 //步数及卡路里
	 	dial->Status.Type1.type = DIAL_STATUS_TYPE_6;
		dial->Status.Type1.align1 = DIAL_ACTIVEX_ALIGN_CENTER_0;
	
		//步数:进度条
		dial->Status.Type1.stepDeg.type = DIAL_PROGRESSBAR_TYPE_5;
		dial->Status.Type1.stepDeg.star_x 				= 120 + Font_Number_41.height/2 + 16;
		dial->Status.Type1.stepDeg.star_y 				= 120-65;
		dial->Status.Type1.stepDeg.height 				= 8;
		dial->Status.Type1.stepDeg.width 				= 130;
		dial->Status.Type1.stepDeg.forecolor  			= LCD_CORNFLOWERBLUE;
		dial->Status.Type1.stepDeg.bckgrndcolor 		= LCD_NONE;
			
	   //图标
		dial->Status.Type1.step.stepImg.bckgrndcolor	= LCD_NONE;
		dial->Status.Type1.step.stepImg.forecolor 		= LCD_CORNFLOWERBLUE;
		dial->Status.Type1.step.stepImg.x_axis 			= dial->Status.Type1.stepDeg.star_x + dial->Status.Type1.stepDeg.height/2 - img_dial_step_16x16.height/2 ;
		dial->Status.Type1.step.stepImg.y_axis 			= dial->Status.Type1.stepDeg.star_y + dial->Status.Type1.stepDeg.width + 8;
		dial->Status.Type1.step.stepImg.width 			= img_dial_step_16x16.width;
		dial->Status.Type1.step.stepImg.height	 		= img_dial_step_16x16.height;
		dial->Status.Type1.step.stepImg.stride 			= img_dial_step_16x16.stride;
		dial->Status.Type1.step.stepImg.imgtype 		= img_dial_step_16x16.imgtype;	

        	//规格
		dial->Status.Type1.step.stepStr.bckgrndcolor 	= LCD_NONE;
		dial->Status.Type1.step.stepStr.forecolor 		= LCD_GRAY;
		dial->Status.Type1.step.stepStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Status.Type1.step.stepStr.size 			= LCD_FONT_16_1_SIZE;
		dial->Status.Type1.step.stepStr.x_axis	 		= dial->Status.Type1.stepDeg.star_x + dial->Status.Type1.stepDeg.height + 4;
		dial->Status.Type1.step.stepStr.y_axis 			= LCD_CENTER_JUSTIFIED;
		dial->Status.Type1.step.stepStr.kerning 		= 1;
		dial->Status.Type1.step.stepStr.width			= Font_Number_16_1.width;
		dial->Status.Type1.step.stepStr.height			= Font_Number_16_1.height;
		dial->Status.Type1.step.gap 					= 4;
		
          //卡里路进度条及图标
        dial->Status.Type2.stepDeg.type = DIAL_PROGRESSBAR_TYPE_5;
		dial->Status.Type2.stepDeg.star_x 				= dial->Status.Type1.step.stepStr.x_axis + dial->Status.Type1.step.stepStr.height + 4 ;
		dial->Status.Type2.stepDeg.star_y 				= 120-65;
		dial->Status.Type2.stepDeg.height 				= 8;
		dial->Status.Type2.stepDeg.width 				= 130;
		dial->Status.Type2.stepDeg.forecolor  			= LCD_CORNFLOWERBLUE;
		dial->Status.Type2.stepDeg.bckgrndcolor 		= LCD_NONE;
		
		dial->Status.Type2.calorie.stepImg.forecolor 		= LCD_RED;
        dial->Status.Type2.calorie.stepImg.bckgrndcolor	    = LCD_NONE;
		dial->Status.Type2.calorie.stepImg.x_axis 			=  dial->Status.Type2.stepDeg.star_x + dial->Status.Type2.stepDeg.height/2 - img_dial_cal_15x20.height/2 ;
		dial->Status.Type2.calorie.stepImg.y_axis 			= dial->Status.Type2.stepDeg.star_y + dial->Status.Type2.stepDeg.width + 8;
		dial->Status.Type2.calorie.stepImg.width 			= img_dial_cal_15x20.width;
		dial->Status.Type2.calorie.stepImg.height	 		= img_dial_cal_15x20.height;
		dial->Status.Type2.calorie.stepImg.stride 			= img_dial_cal_15x20.stride;
		dial->Status.Type2.calorie.stepImg.imgtype 		    = img_dial_cal_15x20.imgtype;
		
		
		//电量
		dial->Battery.type = DIAL_BATTERY_TYPE_3;
			
				//电量进度条
		dial->Battery.batDeg.type = DIAL_PROGRESSBAR_TYPE_0;
        dial->Battery.batDeg.star_x = LCD_CENTER_LINE;
		dial->Battery.batDeg.star_y = LCD_CENTER_LINE;
		dial->Battery.batDeg.star_ang = 210;
		dial->Battery.batDeg.range = 60;
		dial->Battery.batDeg.height = 118;
		dial->Battery.batDeg.width = 4;
		dial->Battery.batDeg.forecolor = LCD_WHITE;
		dial->Battery.batDeg.bckgrndcolor = LCD_GRAY;
		dial->Battery.batDeg.order = 0;
			
				
				dial->Setting.reserve1[0] = DIAL_STYLE_ELSE_1;
				//心率
				dial->Heart.type = 0xFF;
				
			uint8_t index = 0; //存储位置 
				dev_extFlash_enable();	
				dev_extFlash_erase(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), DIAL_DATA_SIZE);
				dev_extFlash_write(DIAL_DATA_START_ADDRESS + (index * DIAL_DATA_SIZE), (uint8_t *)&DialData, sizeof(DialDataStr));	 ;   
				
				uint32_t flag = 0xFEFFFFFF; 
				dev_extFlash_write(DIAL_DATA_START_ADDRESS + ((index + 1) * DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4); 
				dev_extFlash_disable();
		
		
		
	}*/

void com_dial_get(uint8_t number, DialDataStr *dial)
{
	if((number & 0x80) && (number != 0xFF))
	{
		//节日表盘
		com_dial_read(DIAL_FESTIVAL_NUM - 1, dial);
		return;
	}
	else if((number >= DIAL_DEFAULT_MAX_NUM) && (number < DIAL_MAX_NUM))
	{
		//判断当前主题是否有效
		if(com_dial_is_valid(number) == 1)
		{
			//读取外部flash中的表盘数据
			com_dial_read(number - DIAL_DEFAULT_MAX_NUM, dial);
			return;
		}
	}

	/*if(com_dial_custom_is_valid())
	{
		//自定义默认表盘
		com_dial_read(DIAL_FESTIVAL_NUM,dial);
	}*/
	else
	{

	 #if DIAL_TEST_FLAG
     
	// com_dial_test1(dial);//炫酷运动

	 
	 //com_dial_test2(dial);//数字游戏

	 
	// com_dial_test3(dial);//商务指针

	 
	 com_dial_cod_test1(dial);//咕咚商务指针

	// com_dial_cod_test2(dial);//咕咚经典数字
	 
	// com_dial_cod_test3(dial);//咕咚运动休闲
	 #else
		
		//初始化为默认表盘
	#ifdef COD 
		dial->Number = DIAL_DEFAULT_NUMBER;
		memcpy(dial->Name, "户外探索", 8);
		
		//背景
		dial->Background.type 	= DIAL_BACKGROUND_TYPE_DEFAULT;
		dial->Background.color 	= LCD_BLACK;
		
		//时间
		dial->Time.type 						= DIAL_TIME_TYPE_DEFAULT;
		dial->Time.align                        =  DIAL_ACTIVEX_ALIGN_CENTER_0;;
		dial->Time.hourNum.numImg.bckgrndcolor 	= LCD_NONE;
		dial->Time.hourNum.numImg.forecolor     = LCD_WHITE;
		dial->Time.hourNum.numImg.x_axis 	  	= LCD_LINE_CNT_MAX/2 - (18 +  Font_Number_47.height);
		dial->Time.hourNum.numImg.y_axis 	  	= LCD_CENTER_JUSTIFIED;
		dial->Time.hourNum.numImg.width			= Font_Number_47.width;
		dial->Time.hourNum.numImg.height 	  	= Font_Number_47.height;
		dial->Time.hourNum.numImg.stride 		= Font_Number_47.width;
		dial->Time.hourNum.numImg.imgtype 		= Font_Number_47.imgtype;
		dial->Time.hourNum.kerning 				= 4;

		dial->Time.minNum.numImg.bckgrndcolor 	= LCD_NONE;
		dial->Time.minNum.numImg.forecolor 		= LCD_WHITE;
		dial->Time.minNum.numImg.x_axis 		= LCD_LINE_CNT_MAX/2 - (18 +  Font_Number_47.height);
		dial->Time.minNum.numImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Time.minNum.numImg.width 			= Font_Number_47.width;
		dial->Time.minNum.numImg.height 		= Font_Number_47.height;
		dial->Time.minNum.numImg.stride 		= Font_Number_47.width;
		dial->Time.minNum.numImg.imgtype 		= Font_Number_47.imgtype;
		dial->Time.minNum.kerning 				= 4;
		dial->Time.gap 							= 8;
		
		//日期
		dial->Date.type 					= DIAL_DATE_TYPE_CENTER_12;
		dial->Date.align					= DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Date.dateStr.bckgrndcolor 	= LCD_NONE;
		dial->Date.dateStr.forecolor		= LCD_WHITE;
		dial->Date.dateStr.type 			= DIAL_STRING_TYPE_CUSTOM;
				
		dial->Date.dateStr.size 			= LCD_FONT_16_1_SIZE;
		dial->Date.dateStr.x_axis		= 29;
		dial->Date.dateStr.y_axis		= LCD_CENTER_JUSTIFIED;
		dial->Date.dateStr.kerning		= 1;
		dial->Date.dateStr.width		= Font_Number_16_1.width;
		dial->Date.dateStr.height		= Font_Number_16_1.height;
				
		dial->Date.weekStr.bckgrndcolor = LCD_NONE;
		dial->Date.weekStr.forecolor	= LCD_WHITE;
		dial->Date.weekStr.type 		= DIAL_STRING_TYPE_SONG;
		dial->Date.weekStr.size 		= LCD_FONT_16_SIZE;
		dial->Date.weekStr.x_axis		= 28;
		dial->Date.weekStr.y_axis		= LCD_CENTER_JUSTIFIED;
		dial->Date.weekStr.kerning		= 1;
		dial->Date.weekStr.width		= 16;//img_week_13x13[0].width;
		dial->Date.weekStr.height		=16; //img_week_13x13[0].height;
		dial->Date.gap=5; 




		 
		 //天气
		      //天气图标
		     dial->Weather.type = DIAL_WEATHER_TYPE_0;
		     dial->Weather.weather.stepImg.x_axis =0xFF;
			 dial->Widget.dial_weather.d_weatherImg.bckgrndcolor = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherImg.forecolor	 = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherImg.x_axis		 = 0xFF;//dial->Time.hourNum.numImg.x_axis + dial->Time.hourNum.numImg.height - Font_Number_16_1.height - 4 - img_dial_weather_lists[0].height ;
			 dial->Widget.dial_weather.d_weatherImg.y_axis		 = 36;
			 dial->Widget.dial_weather.d_weatherImg.width		 = img_dial_weather_lists[0].width;//img_dial_weather_lists[0]
			 dial->Widget.dial_weather.d_weatherImg.height		 = img_dial_weather_lists[0].height;
			 dial->Widget.dial_weather.d_weatherImg.stride		 = img_dial_weather_lists[0].stride;
			 dial->Widget.dial_weather.d_weatherImg.imgtype 	 = img_dial_weather_lists[0].imgtype;
		 
			 //温度数值
			 dial->Widget.dial_weather.d_weatherStr.bckgrndcolor  = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherStr.forecolor	 = LCD_WHITE;
			 dial->Widget.dial_weather.d_weatherStr.type 		 = DIAL_STRING_TYPE_CUSTOM;
			  dial->Widget.dial_weather.d_weatherStr.size 		 = LCD_FONT_16_1_SIZE;
			 
			 dial->Widget.dial_weather.d_weatherStr.x_axis		 = 0xFF;//dial->Time.hourNum.numImg.x_axis + dial->Time.hourNum.numImg.height - Font_Number_16_1.height;
			 dial->Widget.dial_weather.d_weatherStr.y_axis		 = 36;
			 dial->Widget.dial_weather.d_weatherStr.kerning		 = 1;
			 dial->Widget.dial_weather.d_weatherStr.width		 = 6;
			 
			 dial->Widget.dial_weather.d_weatherStr.height		 = 16;
		 
		
		
		//工具栏
		dial->Widget.type 					= DIAL_WIDGET_TYPE_DEFAULT;
		dial->Widget.align                  = DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Widget.gap                    = 4;
		  //勿扰状态
		 dial->Widget.DNDImg.bckgrndcolor = LCD_NONE;
		 dial->Widget.DNDImg.forecolor	  = LCD_LIGHTGRAY;
		 dial->Widget.DNDImg.x_axis 	 = 232 - img_dial_dndisturb.height;//217
		 dial->Widget.DNDImg.y_axis 	 = LCD_CENTER_JUSTIFIED;
		 dial->Widget.DNDImg.width		 = img_dial_dndisturb.width;
		 dial->Widget.DNDImg.height 	 = img_dial_dndisturb.height;
		 dial->Widget.DNDImg.stride 	 = img_dial_dndisturb.stride;
		 dial->Widget.DNDImg.imgtype	 = img_dial_dndisturb.imgtype;
		//蓝牙
		dial->Widget.ble.connectImg.bckgrndcolor	= LCD_NONE;
		dial->Widget.ble.connectImg.forecolor 	= LCD_CYAN;
		
		dial->Widget.ble.connectImg.x_axis 		= 232 - img_dial_dndisturb.height/2 - img_dial_ble.height/2;
		dial->Widget.ble.connectImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Widget.ble.connectImg.width 		= img_dial_ble.width;
		dial->Widget.ble.connectImg.height	 	= img_dial_ble.height;
		dial->Widget.ble.connectImg.stride 		= img_dial_ble.stride;
		dial->Widget.ble.connectImg.imgtype 		= img_dial_ble.imgtype;

		//img_dial_ble
		dial->Widget.ble.disconnectImg.bckgrndcolor	= LCD_NONE;
		dial->Widget.ble.disconnectImg.forecolor 	= LCD_WHITE;
		
		dial->Widget.ble.disconnectImg.x_axis 		= 232 - img_dial_dndisturb.height/2 - img_dial_ble.height/2;
		dial->Widget.ble.disconnectImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Widget.ble.disconnectImg.width 		= img_dial_ble.width;
		dial->Widget.ble.disconnectImg.height	 	= img_dial_ble.height;
		dial->Widget.ble.disconnectImg.stride 		= img_dial_ble.stride;
		dial->Widget.ble.disconnectImg.imgtype 		= img_dial_ble.imgtype;
       

		//闹钟
		dial->Widget.alarmImg.bckgrndcolor 	= LCD_NONE;
		dial->Widget.alarmImg.forecolor 		= LCD_WHITE;
		dial->Widget.alarmImg.x_axis 		= 232 - img_dial_dndisturb.height/2 - img_dial_alarm.height/2;
		dial->Widget.alarmImg.y_axis 		= 120+4+img_dial_ble.width+4;
		dial->Widget.alarmImg.width 		= img_dial_alarm.width;
		dial->Widget.alarmImg.height 		= img_dial_alarm.height;
		dial->Widget.alarmImg.stride 		= img_dial_alarm.stride;
		dial->Widget.alarmImg.imgtype 		= img_dial_alarm.imgtype;
		


	
	



		
		/*图标:步数*/
		dial->Status.Type1.type = DIAL_STATUS_TYPE_DEFAULT_2;
		dial->Status.Type1.align1 = DIAL_ACTIVEX_ALIGN_CENTER_1;
		dial->Status.Type1.step.stepImg.bckgrndcolor	= LCD_NONE;
		dial->Status.Type1.step.stepImg.forecolor 		= LCD_CYAN;
		dial->Status.Type1.step.stepImg.x_axis 			= 120  -10  ;
		dial->Status.Type1.step.stepImg.y_axis 			= 40;
		dial->Status.Type1.step.stepImg.width 			= img_dial_step_16x16.width;
		dial->Status.Type1.step.stepImg.height	 		= img_dial_step_16x16.height;
		dial->Status.Type1.step.stepImg.stride 			= img_dial_step_16x16.stride;
		dial->Status.Type1.step.stepImg.imgtype 		= img_dial_step_16x16.imgtype;
		/*规格*/
		dial->Status.Type1.step.stepStr.bckgrndcolor 	= LCD_NONE;
		dial->Status.Type1.step.stepStr.forecolor 		= LCD_WHITE;
		dial->Status.Type1.step.stepStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Status.Type1.step.stepStr.size 			= LCD_FONT_17_SIZE;
		dial->Status.Type1.step.stepStr.x_axis	 		= 120  - 10 + img_dial_step_16x16.height + 7;
		dial->Status.Type1.step.stepStr.y_axis 			= 40;
		dial->Status.Type1.step.stepStr.kerning 		= 1;
		dial->Status.Type1.step.stepStr.width			= Font_Number_17.width;
		dial->Status.Type1.step.stepStr.height			= Font_Number_17.height;
		dial->Status.Type1.step.gap 					= 2;
	
		
	//电量
		
		dial->Battery.type = DIAL_BATTERY_TYPE_DEFAULT;
		dial->Battery.align = DIAL_ACTIVEX_ALIGN_CENTER_0;
		//值大小
		
		dial->Battery.batStr.type 				= DIAL_STRING_TYPE_CUSTOM;
		dial->Battery.batStr.x_axis 			 = 8;
		dial->Battery.batStr.y_axis 			 = LCD_CENTER_JUSTIFIED;
		dial->Battery.batStr.bckgrndcolor		= LCD_NONE;
		dial->Battery.batStr.forecolor			= LCD_WHITE;
		dial->Battery.batStr.size 				= LCD_FONT_12_SIZE;
      	dial->Battery.batStr.kerning 			= 1;
		dial->Battery.batStr.width				= Font_Number_12.width;
		dial->Battery.batStr.height				= Font_Number_12.height;
		
		//电池图标
		dial->Battery.batImg.batImg.bckgrndcolor	= LCD_NONE;
		dial->Battery.batImg.batImg.forecolor	=  LCD_WHITE;
		dial->Battery.batImg.batImg.x_axis 		= 8;
		dial->Battery.batImg.batImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Battery.batImg.batImg.width 		= img_dial_bat_28x13.width;
		dial->Battery.batImg.batImg.height	 	= img_dial_bat_28x13.height;
		dial->Battery.batImg.batImg.stride 		= img_dial_bat_28x13.stride;
		dial->Battery.batImg.batImg.imgtype 		= img_dial_bat_28x13.imgtype;
		dial->Battery.batImg.x_axis 				= 2;
		dial->Battery.batImg.y_axis 				= 2;
		dial->Battery.batImg.height 				= 8;
		dial->Battery.batImg.width 				= img_dial_bat_28x13.width - 3;
		dial->Battery.batImg.normalColor		 	= LCD_GREEN;
		dial->Battery.batImg.lowColor 			=  LCD_RED;
		dial->Battery.gap = 4;



      
	  //心率(图标)
			  dial->Heart.type = 0xFF;//DIAL_HEART_TYPE_DEFAULT;
			  dial->Heart.align = DIAL_ACTIVEX_ALIGN_6;
		  
			  dial->Heart.heartImg.width   = img_dial_heartrate_16x15.width;
			  dial->Heart.heartImg.height  = img_dial_heartrate_16x15.height;
			  dial->Heart.heartImg.stride  = img_dial_heartrate_16x15.stride;
			  dial->Heart.heartImg.imgtype = img_dial_heartrate_16x15.imgtype;
			  dial->Heart.heartImg.x_axis  = 120 - 20/2 - img_dial_heartrate_16x15.height/2;
			//	dial->Heart.heartImg.y_axis  = 180;
			  dial->Heart.heartImg.y_axis  = 10;
				  
	  
			  dial->Heart.heartImg.bckgrndcolor   = LCD_NONE;
			  dial->Heart.heartImg.forecolor  =  LCD_RED;
			//心率(数据)
			   dial->Heart.heartStr.x_axis = 120 - 20/2 - Font_Number_16_1.height/2;
			   dial->Heart.heartStr.y_axis = dial->Heart.heartImg.y_axis + img_dial_heartrate_16x15.width + 4;
			   dial->Heart.heartStr.bckgrndcolor = LCD_NONE;
			   dial->Heart.heartStr.forecolor = LCD_WHITE;
			   dial->Heart.heartStr.width = Font_Number_16_1.width;
			   dial->Heart.heartStr.height= Font_Number_16_1.height;
			   dial->Heart.heartStr.size= LCD_FONT_16_1_SIZE;
			   dial->Heart.heartStr.kerning = 1;


//卡路里



   //环境:气压&高度
   dial->Ambient.type = DIAL_AMBIENT_TYPE_2;

	  //气压图标

      dial->Graph.Type1.type = DIAL_GRAPH_TYPE_12H_PRESSURE;
      dial->Graph.Type1.star_x = 158;
	  dial->Graph.Type1.height = 240 - 158;
	  dial->Graph.Type1.star_y = 38;
	  dial->Graph.Type1.width = 164;
	  dial->Graph.Type1.forecolor = LCD_RED;
	  dial->Graph.Type1.bckgrndcolor = LCD_RED;
		
		dial->Setting.reserve1[0] = DIAL_STYLE_DEFAULT;
   #else
		dial->Number = DIAL_DEFAULT_NUMBER;
		memcpy(dial->Name, "户外探索", 8);
		
		//背景
		dial->Background.type 	= DIAL_BACKGROUND_TYPE_DEFAULT;
		dial->Background.color 	= LCD_BLACK;
		
		//时间
		dial->Time.type 						= DIAL_TIME_TYPE_DEFAULT;
		dial->Time.align                        =  DIAL_ACTIVEX_ALIGN_CENTER_0;;
		dial->Time.hourNum.numImg.bckgrndcolor 	= LCD_NONE;
		dial->Time.hourNum.numImg.forecolor     = LCD_WHITE;
		dial->Time.hourNum.numImg.x_axis 	  	= LCD_LINE_CNT_MAX/2 - (2 + 16 + 2 + 12 +  Font_Number_47.height);
		dial->Time.hourNum.numImg.y_axis 	  	= LCD_CENTER_JUSTIFIED;
		dial->Time.hourNum.numImg.width			= Font_Number_47.width;
		dial->Time.hourNum.numImg.height 	  	= Font_Number_47.height;
		dial->Time.hourNum.numImg.stride 		= Font_Number_47.width;
		dial->Time.hourNum.numImg.imgtype 		= Font_Number_47.imgtype;
		dial->Time.hourNum.kerning 				= 1;

		dial->Time.minNum.numImg.bckgrndcolor 	= LCD_NONE;
		dial->Time.minNum.numImg.forecolor 		= LCD_WHITE;
		dial->Time.minNum.numImg.x_axis 		= LCD_LINE_CNT_MAX/2 - (2 + 16 + 2 + 12 +  Font_Number_47.height);
		dial->Time.minNum.numImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Time.minNum.numImg.width 			= Font_Number_47.width;
		dial->Time.minNum.numImg.height 			= Font_Number_47.height;
		dial->Time.minNum.numImg.stride 			= Font_Number_47.width;
		dial->Time.minNum.numImg.imgtype 		= Font_Number_47.imgtype;
		dial->Time.minNum.kerning 				= 1;
		dial->Time.gap 							= 8;
		
		//日期
				dial->Date.type 					= DIAL_DATE_TYPE_DEFAULT;
				dial->Date.align					= DIAL_ACTIVEX_ALIGN_6;
				dial->Date.dateStr.bckgrndcolor 	= LCD_NONE;
				dial->Date.dateStr.forecolor		= LCD_WHITE;
				dial->Date.dateStr.type 			= DIAL_STRING_TYPE_CUSTOM;
				
				dial->Date.dateStr.size 			= LCD_FONT_16_1_SIZE;
				dial->Date.dateStr.x_axis		= dial->Time.hourNum.numImg.x_axis + dial->Time.hourNum.numImg.height -  img_week_13x13[0].height - 4 - Font_Number_16_1.height;
				dial->Date.dateStr.y_axis		= 182;
				dial->Date.dateStr.kerning		= 1;
				dial->Date.dateStr.width		= Font_Number_16_1.width;
				dial->Date.dateStr.height		= Font_Number_16_1.height;
				
				dial->Date.weekStr.bckgrndcolor = LCD_NONE;
				dial->Date.weekStr.forecolor	= LCD_WHITE;
				dial->Date.weekStr.type 		= DIAL_STRING_TYPE_CUSTOM;
				dial->Date.weekStr.size 		= LCD_FONT_13_SIZE;
				dial->Date.weekStr.x_axis		= dial->Time.hourNum.numImg.x_axis + dial->Time.hourNum.numImg.height -  img_week_13x13[0].height;
				dial->Date.weekStr.y_axis		= 182;
				dial->Date.weekStr.kerning		= 1;
				dial->Date.weekStr.width		= img_week_13x13[0].width;
				dial->Date.weekStr.height		= img_week_13x13[0].height;
				dial->Date.gap=8; 




		 
		 //天气
		      //天气图标
		     dial->Weather.type = DIAL_WEATHER_TYPE_0;
		 	 dial->Weather.weather.stepImg.x_axis =0xFF;
			 dial->Widget.dial_weather.d_weatherImg.bckgrndcolor = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherImg.forecolor	 = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherImg.x_axis		 = dial->Time.hourNum.numImg.x_axis + dial->Time.hourNum.numImg.height - Font_Number_16_1.height - 4 - img_dial_weather_lists[0].height ;
			 dial->Widget.dial_weather.d_weatherImg.y_axis		 = 36;
			 dial->Widget.dial_weather.d_weatherImg.width		 = img_dial_weather_lists[0].width;//img_dial_weather_lists[0]
			 dial->Widget.dial_weather.d_weatherImg.height		 = img_dial_weather_lists[0].height;
			 dial->Widget.dial_weather.d_weatherImg.stride		 = img_dial_weather_lists[0].stride;
			 dial->Widget.dial_weather.d_weatherImg.imgtype 	 = img_dial_weather_lists[0].imgtype;
		 
			 //温度数值
			 dial->Widget.dial_weather.d_weatherStr.bckgrndcolor  = LCD_NONE;
			 dial->Widget.dial_weather.d_weatherStr.forecolor	 = LCD_WHITE;
			 dial->Widget.dial_weather.d_weatherStr.type 		 = DIAL_STRING_TYPE_CUSTOM;
			  dial->Widget.dial_weather.d_weatherStr.size 		 = LCD_FONT_16_1_SIZE;
			 
			 dial->Widget.dial_weather.d_weatherStr.x_axis		 = dial->Time.hourNum.numImg.x_axis + dial->Time.hourNum.numImg.height - Font_Number_16_1.height;
			 dial->Widget.dial_weather.d_weatherStr.y_axis		 = 36;
			 dial->Widget.dial_weather.d_weatherStr.kerning		 = 1;
			 dial->Widget.dial_weather.d_weatherStr.width		 = 6;
			 
			 dial->Widget.dial_weather.d_weatherStr.height		 = 16;
		 
		
		
		//工具栏
		dial->Widget.type 					= DIAL_WIDGET_TYPE_DEFAULT;
		dial->Widget.align                  = DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Widget.gap                    = 4;
		  //勿扰状态
		 dial->Widget.DNDImg.bckgrndcolor = LCD_NONE;
		 dial->Widget.DNDImg.forecolor	  = LCD_LIGHTGRAY;
		 dial->Widget.DNDImg.x_axis 	 = 232 - img_dial_dndisturb.height;//217
		 dial->Widget.DNDImg.y_axis 	 = LCD_CENTER_JUSTIFIED;
		 dial->Widget.DNDImg.width		 = img_dial_dndisturb.width;
		 dial->Widget.DNDImg.height 	 = img_dial_dndisturb.height;
		 dial->Widget.DNDImg.stride 	 = img_dial_dndisturb.stride;
		 dial->Widget.DNDImg.imgtype	 = img_dial_dndisturb.imgtype;
		//蓝牙
		dial->Widget.ble.connectImg.bckgrndcolor	= LCD_NONE;
		dial->Widget.ble.connectImg.forecolor 	= LCD_CYAN;
		
		dial->Widget.ble.connectImg.x_axis 		= 232 - img_dial_dndisturb.height/2 - img_dial_ble.height/2;
		dial->Widget.ble.connectImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Widget.ble.connectImg.width 		= img_dial_ble.width;
		dial->Widget.ble.connectImg.height	 	= img_dial_ble.height;
		dial->Widget.ble.connectImg.stride 		= img_dial_ble.stride;
		dial->Widget.ble.connectImg.imgtype 		= img_dial_ble.imgtype;

		//img_dial_ble
		dial->Widget.ble.disconnectImg.bckgrndcolor	= LCD_NONE;
		dial->Widget.ble.disconnectImg.forecolor 	= LCD_WHITE;
		
		dial->Widget.ble.disconnectImg.x_axis 		= 232 - img_dial_dndisturb.height/2 - img_dial_ble.height/2;
		dial->Widget.ble.disconnectImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Widget.ble.disconnectImg.width 		= img_dial_ble.width;
		dial->Widget.ble.disconnectImg.height	 	= img_dial_ble.height;
		dial->Widget.ble.disconnectImg.stride 		= img_dial_ble.stride;
		dial->Widget.ble.disconnectImg.imgtype 		= img_dial_ble.imgtype;
       

		//闹钟
		dial->Widget.alarmImg.bckgrndcolor 	= LCD_NONE;
		dial->Widget.alarmImg.forecolor 		= LCD_WHITE;
		dial->Widget.alarmImg.x_axis 		= 232 - img_dial_dndisturb.height/2 - img_dial_alarm.height/2;
		dial->Widget.alarmImg.y_axis 		= 120+4+img_dial_ble.width+4;
		dial->Widget.alarmImg.width 		= img_dial_alarm.width;
		dial->Widget.alarmImg.height 		= img_dial_alarm.height;
		dial->Widget.alarmImg.stride 		= img_dial_alarm.stride;
		dial->Widget.alarmImg.imgtype 		= img_dial_alarm.imgtype;
		


	
	



		
		/*图标:步数*/
		dial->Status.Type1.type = DIAL_STATUS_TYPE_DEFAULT;
		dial->Status.Type1.align1 = DIAL_ACTIVEX_ALIGN_CENTER_0;
		dial->Status.Type1.step.stepImg.bckgrndcolor	= LCD_NONE;
		dial->Status.Type1.step.stepImg.forecolor 		= LCD_CYAN;
		dial->Status.Type1.step.stepImg.x_axis 			= 120  - 2 - img_dial_step_16x16.height ;
		dial->Status.Type1.step.stepImg.y_axis 			= LCD_CENTER_JUSTIFIED;
		dial->Status.Type1.step.stepImg.width 			= img_dial_step_16x16.width;
		dial->Status.Type1.step.stepImg.height	 		= img_dial_step_16x16.height;
		dial->Status.Type1.step.stepImg.stride 			= img_dial_step_16x16.stride;
		dial->Status.Type1.step.stepImg.imgtype 		= img_dial_step_16x16.imgtype;
		/*规格*/
		dial->Status.Type1.step.stepStr.bckgrndcolor 	= LCD_NONE;
		dial->Status.Type1.step.stepStr.forecolor 		= LCD_WHITE;
		dial->Status.Type1.step.stepStr.type 			= DIAL_STRING_TYPE_CUSTOM;
		dial->Status.Type1.step.stepStr.size 			= LCD_FONT_16_1_SIZE;
		dial->Status.Type1.step.stepStr.x_axis	 		= 120  - 2 - Font_Number_16_1.height;
		dial->Status.Type1.step.stepStr.y_axis 			= LCD_CENTER_JUSTIFIED;
		dial->Status.Type1.step.stepStr.kerning 		= 1;
		dial->Status.Type1.step.stepStr.width			= Font_Number_16_1.width;
		dial->Status.Type1.step.stepStr.height			= Font_Number_16_1.height;
		dial->Status.Type1.step.gap 					= 2;
	
		
	//电量
		
		dial->Battery.type = DIAL_BATTERY_TYPE_DEFAULT;
		dial->Battery.align = DIAL_ACTIVEX_ALIGN_CENTER_0;
		//值大小
		
		dial->Battery.batStr.type 				= DIAL_STRING_TYPE_CUSTOM;
		dial->Battery.batStr.x_axis 			 = 10;
		dial->Battery.batStr.y_axis 			 = LCD_CENTER_JUSTIFIED;
		dial->Battery.batStr.bckgrndcolor		= LCD_NONE;
		dial->Battery.batStr.forecolor			= LCD_WHITE;
		dial->Battery.batStr.size 				= LCD_FONT_12_SIZE;
      	dial->Battery.batStr.kerning 			= 1;
		dial->Battery.batStr.width				= Font_Number_12.width;
		dial->Battery.batStr.height				= Font_Number_12.height;
		
		//电池图标
		dial->Battery.batImg.batImg.bckgrndcolor	= LCD_NONE;
		dial->Battery.batImg.batImg.forecolor	=  LCD_WHITE;
		dial->Battery.batImg.batImg.x_axis 		= 10;
		dial->Battery.batImg.batImg.y_axis 		= LCD_CENTER_JUSTIFIED;
		dial->Battery.batImg.batImg.width 		= img_dial_bat_28x13.width;
		dial->Battery.batImg.batImg.height	 	= img_dial_bat_28x13.height;
		dial->Battery.batImg.batImg.stride 		= img_dial_bat_28x13.stride;
		dial->Battery.batImg.batImg.imgtype 		= img_dial_bat_28x13.imgtype;
		dial->Battery.batImg.x_axis 				= 2;
		dial->Battery.batImg.y_axis 				= 2;
		dial->Battery.batImg.height 				= 8;
		dial->Battery.batImg.width 				= img_dial_bat_28x13.width - 3;
		dial->Battery.batImg.normalColor		 	= LCD_GREEN;
		dial->Battery.batImg.lowColor 			=  LCD_RED;
		dial->Battery.gap = 4;



      
	  //心率(图标)
			  dial->Heart.type = DIAL_HEART_TYPE_DEFAULT;
			  dial->Heart.align = DIAL_ACTIVEX_ALIGN_6;
		  
			  dial->Heart.heartImg.width   = img_dial_heartrate_16x15.width;
			  dial->Heart.heartImg.height  = img_dial_heartrate_16x15.height;
			  dial->Heart.heartImg.stride  = img_dial_heartrate_16x15.stride;
			  dial->Heart.heartImg.imgtype = img_dial_heartrate_16x15.imgtype;
			  dial->Heart.heartImg.x_axis  = 120 - 20/2 - img_dial_heartrate_16x15.height/2;
			//	dial->Heart.heartImg.y_axis  = 180;
			  dial->Heart.heartImg.y_axis  = 10;
				  
	  
			  dial->Heart.heartImg.bckgrndcolor   = LCD_NONE;
			  dial->Heart.heartImg.forecolor  =  LCD_RED;
			//心率(数据)
			   dial->Heart.heartStr.x_axis = 120 - 20/2 - Font_Number_16_1.height/2;
			   dial->Heart.heartStr.y_axis = dial->Heart.heartImg.y_axis + img_dial_heartrate_16x15.width + 4;
			   dial->Heart.heartStr.bckgrndcolor = LCD_NONE;
			   dial->Heart.heartStr.forecolor = LCD_WHITE;
			   dial->Heart.heartStr.width = Font_Number_16_1.width;
			   dial->Heart.heartStr.height= Font_Number_16_1.height;
			   dial->Heart.heartStr.size= LCD_FONT_16_1_SIZE;
			   dial->Heart.heartStr.kerning = 1;


//卡路里



   //环境:气压&高度
   dial->Ambient.type = DIAL_AMBIENT_TYPE_DEFAULT;

	  //气压图标

      dial->Graph.Type1.type = DIAL_GRAPH_TYPE_12H_PRESSURE;
      dial->Graph.Type1.star_x = 158;
	  dial->Graph.Type1.height = 240 - 158;
	  dial->Graph.Type1.star_y = 38;
	  dial->Graph.Type1.width = 164;
	  dial->Graph.Type1.forecolor = LCD_RED;
	  dial->Graph.Type1.bckgrndcolor = LCD_RED;
		
		dial->Setting.reserve1[0] = DIAL_STYLE_DEFAULT;	   
   #endif
   #endif
	}
}

/**
  * @brief  判断表盘是否无效
  * @param  num: 0为默认表盘 1-7对应外部flash中的表盘
  * @retval 
  */
uint8_t com_dial_is_valid(uint8_t number)
{
	uint8_t i;
	uint32_t ID = 0;
	uint32_t flag = 0;
	
	if(number >= DIAL_DEFAULT_MAX_NUM)
	{
		i = number - DIAL_DEFAULT_MAX_NUM;
	}
	else
	{
		//默认表盘，存储在内部flash，返回有效
		return 1;
	}
	
	dev_extFlash_enable();
	//读取表盘ID
	dev_extFlash_read(DIAL_DATA_START_ADDRESS + (i * DIAL_DATA_SIZE) + sizeof(DialInfo), (uint8_t *)(&ID), 4);
	//读取表盘完整标志
	dev_extFlash_read(DIAL_DATA_START_ADDRESS + ((i + 1) * DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4);
	dev_extFlash_disable();
	
	if((ID != 0xFFFFFFFF) && (flag == 0xFEFFFFFF))
	{
		//有效表盘
		return 1;
	}
	else
	{
		//无效表盘
		return 0;
	}
}

/**
  * @brief  每天0点调用，判断节日表盘是否有效，初始化节日表盘数据
  * @param
  * @retval
  */
void com_dial_festival_init(void)
{
	if(com_dial_festival_is_valid())
	{
		//自动设置为节日表盘，将最高位置1表示使用节日表盘
		SetValue.Theme |= 0x80;
		//清除数据
		memset(&DialData, 0, sizeof(DialDataStr));
		//获取节日表盘数据
		com_dial_get(SetValue.Theme, &DialData);
	}
	else
	{
		if((SetValue.Theme & 0x80) && (SetValue.Theme != 0xFF))
		{
			SetValue.Theme &= 0x7F;
			
			//将节日表盘置为无效
			com_dial_set_invalid(DIAL_FESTIVAL_NUM - 1);

			//清除数据
			memset(&DialData, 0, sizeof(DialDataStr));
			//获取表盘数据
			com_dial_get(SetValue.Theme, &DialData);
		}
	}
}

/**
  * @brief  判断是否有有效的节日表盘
  * @param
  * @retval 1为有效
  */
uint8_t com_dial_festival_is_valid(void)
{
	uint8_t status = 0;
	DialFestTimeStr star_time,end_time;
	time_t star_sec,end_sec,second;
	struct tm time = {0};
	
	if(com_dial_is_valid(DIAL_FESTIVAL_NUM))
	{
		//有节日表盘，判断有效时间
		memset(&star_time, 0 , sizeof(DialFestTimeStr));
		memset(&end_time, 0 , sizeof(DialFestTimeStr));
		dev_extFlash_enable();
		dev_extFlash_read(DIAL_DATA_START_ADDRESS + ((DIAL_FESTIVAL_NUM - 1) * DIAL_DATA_SIZE) + 16, (uint8_t *)(&star_time), sizeof(DialFestTimeStr));
		dev_extFlash_read(DIAL_DATA_START_ADDRESS + ((DIAL_FESTIVAL_NUM - 1) * DIAL_DATA_SIZE) + 16 + 4, (uint8_t *)(&end_time), sizeof(DialFestTimeStr));
		dev_extFlash_disable();
		
		time.tm_year = 2000 - 1900 + star_time.year;
		time.tm_mon = star_time.month - 1;
		time.tm_mday = star_time.day;
		time.tm_hour = 0;
		time.tm_min = 0;
		time.tm_sec = 0;
		star_sec = mktime(&time);
		
		time.tm_year = 2000 - 1900 + end_time.year;
		time.tm_mon = end_time.month - 1;
		time.tm_mday = end_time.day;
		time.tm_hour = 23;
		time.tm_min = 59;
		time.tm_sec = 59;
		end_sec = mktime(&time);
		
		am_hal_rtc_time_get(&RTC_time);
		time.tm_year = 2000 - 1900 +  RTC_time.ui32Year;
		time.tm_mon = RTC_time.ui32Month - 1;
		time.tm_mday = RTC_time.ui32DayOfMonth;
		time.tm_hour = RTC_time.ui32Hour;
		time.tm_min = RTC_time.ui32Minute;
		time.tm_sec = RTC_time.ui32Second;
		second = mktime(&time);
		
		if((second >= star_sec) && (second <= end_sec))
		{
			//节日表盘有效
			status = 1;
		}
	}
	
	return status;
}

/**
  * @brief  将表盘置为无效
  * @param  num:外部flash存储空间(0-6)
  * @retval 
  */
void com_dial_set_invalid(uint8_t num)
{
	uint32_t flag = 0xF0FFFFFF;
	
	//把表盘设置为无效表盘
	dev_extFlash_enable();
	dev_extFlash_write(DIAL_DATA_START_ADDRESS + ((num + 1)* DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4);
	dev_extFlash_disable();
}

/**
  * @brief  判断是否有有效的自定义默认表盘
  * @param
  * @retval 1为有效
  */
uint8_t com_dial_custom_is_valid(void)
{
	uint32_t ID = 0;
	
	dev_extFlash_enable();
	//读取表盘ID
	dev_extFlash_read(DIAL_DATA_START_ADDRESS + (DIAL_FESTIVAL_NUM * DIAL_DATA_SIZE) + sizeof(DialInfo), (uint8_t *)(&ID), 4);
	dev_extFlash_disable();
	
	if(ID != 0xFFFFFFFF)
	{
		//有效表盘
		return 1;
	}
	else
	{
		//无效表盘
		return 0;
	}
}

/**
  * @brief  将自定义表盘置写入flash中
  * @param  num:外部flash存储空间(0-6)
  * @retval 
  */
void com_dial_custom_write(uint8_t num, DialDataStr *dial)
{
	dev_extFlash_enable();
	dev_extFlash_erase(DIAL_DATA_START_ADDRESS + (num* DIAL_DATA_SIZE), 4096);
	dev_extFlash_write(DIAL_DATA_START_ADDRESS + (num* DIAL_DATA_SIZE), (uint8_t *)(dial), sizeof(DialDataStr));
	dev_extFlash_disable();
}

/**
  * @brief  将自定义表盘置写入flash中
  * @param  num:外部flash存储空间(0-6)
  * @retval 
  */
void com_dial_custom_delete(uint8_t num)
{
	dev_extFlash_enable();
	dev_extFlash_erase(DIAL_DATA_START_ADDRESS + (num* DIAL_DATA_SIZE), 4096);
	dev_extFlash_disable();
}

