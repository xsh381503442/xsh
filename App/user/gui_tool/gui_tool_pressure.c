#include "gui_tool_config.h"
#include "gui_tool_pressure.h"

#include "drv_lcd.h"

#include "font_config.h"

#include "com_data.h"

#if DEBUG_ENABLED == 1 && GUI_TOOL_PRESSURE_LOG_ENABLED == 1
	#define GUI_TOOL_PRESSURE_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_PRESSURE_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_PRESSURE_WRITESTRING(...)
	#define GUI_TOOL_PRESSURE_PRINTF(...)		        
#endif

/*void gui_tool_pressure_value(float pres,float alt,float temp)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//清屏
	LCD_SetRectangle(80, 110, 40, 128, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//气压
	word.x_axis = 80;
	word.y_axis = 50;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Pressure_14X19);

	word.x_axis -= 6;
	word.y_axis += 40;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",(int32_t)pres,(uint32_t)(fabs(pres)*10)%10);
	#else
	sprintf(str,"%.1f",pres);
	#endif
	LCD_SetNumber(str,&word);
	
	//hPa
	word.x_axis += 7;
	word.y_axis = 166;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_hPa_36X18);
	
//	LCD_SetLine(word.x_axis + 26,2,40,148,LCD_WHITE);
	
	//高度
	word.x_axis += 36 + 2;
	word.y_axis = 50;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Altitude_14X19);

	word.x_axis -= 6;
	word.y_axis += 40;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	sprintf(str,"%d",(int)(alt));
	LCD_SetNumber(str,&word);
	
	//m
	word.x_axis += 7;
	word.y_axis = 166;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_m_18X18);;
	
//	LCD_SetLine(word.x_axis + 26,2,40,148,LCD_WHITE);
	
	//温度
	word.x_axis += 36 + 2;
	word.y_axis = 50;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Temperature_14X19);

	word.x_axis -= 6;
	word.y_axis += 40;
	word.size = LCD_FONT_24_SIZE;
	if(temp < 17)
	{
		//舒适值以下显示蓝色
		word.forecolor = LCD_DARKTURQUOISE;
	}
	else if(temp < 25)
	{
		//舒适值以内显示绿色
		word.forecolor = LCD_LIME;
	}
	else
	{
		//舒适值以上显示绿色
		word.forecolor = LCD_RED;
	}
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",(int32_t)temp,(uint32_t)(fabs(temp)*10)%10);
	#else
	sprintf(str,"%0.1f",temp);
	#endif
	LCD_SetNumber(str,&word);
	
	//°
	word.x_axis += 7;
	word.y_axis = 166;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_C_22X18);
	
	//C
//	word.x_axis -= 5;
//	word.y_axis += 8;
	
//	LCD_SetLine(word.x_axis + 25,2,40,148,LCD_WHITE);
	
	//指示右
	word.x_axis = 116;
	word.y_axis = 220;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Pointing_Right_12X8);
}*/

/*void gui_tool_pressure_paint(float pres,float alt,float temp)
{
	//将背景设置为黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//日历天气图标
	LCD_SetPicture(15,109,LCD_NONE,LCD_NONE,&Img_tools[Img_Pressure_21X28]);

	gui_tool_pressure_value(pres, alt, temp);
}*/

/*void gui_tool_pressure_btn_evt(uint32_t Key_Value)
{
	


}*/

