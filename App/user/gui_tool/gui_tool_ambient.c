#include <stdlib.h>
#include "gui_tool_config.h"
#include "gui_tool_ambient.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_tool.h"

#include "gui.h"
#include "gui_tool.h"

#include "font_config.h"

#include "com_data.h"


#if DEBUG_ENABLED == 1 && GUI_TOOL_AMBIENT_LOG_ENABLED == 1
	#define GUI_TOOL_AMBIENT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_AMBIENT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_AMBIENT_WRITESTRING(...)
	#define GUI_TOOL_AMBIENT_PRINTF(...)		        
#endif

#define	GUI_TOOL_AMBIENT_NEW_UI

#define GUI_TOOL_AMBIENT_MENU_PRESSURE	0	//气压
#define	GUI_TOOL_AMBIENT_MENU_ALTITUDE	1	//高度
#define GUI_TOOL_AMBIENT_MENU_MAX		GUI_TOOL_AMBIENT_MENU_ALTITUDE

static uint8_t m_gui_ambient_menu_index = GUI_TOOL_AMBIENT_MENU_PRESSURE;
static uint8_t m_gui_ambient_char;		//正负号
static int16_t m_gui_ambient_altitude;	//高度绝对值
static uint8_t m_gui_ambient_set;		//设置位数

void gui_tool_ambient_value(float pres,float alt,float temp)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t starty1,starty2;
	
	starty1 = 42; //图片起始坐标
	starty2 = 176;//单位起始坐标
	
	//清屏
	LCD_SetRectangle(68, 140, 40, 160, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	
	//气压
	
	word.x_axis = 70;
	word.y_axis = starty1;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_CORNFLOWERBLUE, LCD_NONE, &img_tool_ambient_pa);
    word.x_axis = 70 + img_tool_ambient_pa.height/2 - 29/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_29_SIZE;
	word.kerning = 1;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	memset(str,0,sizeof(str));
	sprintf(str,"%.1f",pres);
	LCD_SetNumber(str,&word);
	
	//hPa
	word.x_axis =  70 + img_tool_ambient_pa.height/2 - 18/2;
	word.y_axis = starty2;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_hPa_36X18);
	





	
	//高度
	word.x_axis = 70 + img_tool_ambient_pa.height + 24;
	word.y_axis = starty1;
	
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_FORESTGREEN, LCD_NONE, &img_tool_ambient_height);

	word.x_axis += (img_tool_ambient_height.height/2 - 29/2);
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_29_SIZE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	sprintf(str,"%d",(int)(alt + SetValue.Altitude_Offset));
	LCD_SetNumber(str,&word);
	
	//m
	word.x_axis = 70 + img_tool_ambient_pa.height + 24 +  img_tool_ambient_height.height/2 - 18/2;
	word.y_axis = starty2;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_m_18X18);;
	




	
	//温度
	word.x_axis =70 + (img_tool_ambient_pa.height + 24)*2;
	word.y_axis = starty1;

		if(temp < 17)
	{
		//舒适值以下显示蓝色
		word.forecolor = LCD_DARKTURQUOISE;
	}
	else if(temp <= 24)
	{
		//舒适值以内显示绿色
		word.forecolor = LCD_LIME;
	}
	else
	{
		//舒适值以上显示红色
		word.forecolor = LCD_RED;
	}

	word.bckgrndcolor = LCD_NONE;
	LCD_SetPicture(word.x_axis, word.y_axis, word.forecolor, LCD_NONE, &img_tool_ambient_temp);

	word.x_axis += (img_tool_ambient_temp.height/2 - 29/2);
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_29_SIZE;
	/*if(temp < 17)
	{
		//舒适值以下显示蓝色
		word.forecolor = LCD_DARKTURQUOISE;
	}
	else if(temp <= 24)
	{
		//舒适值以内显示绿色
		word.forecolor = LCD_LIME;
	}
	else
	{
		//舒适值以上显示红色
		word.forecolor = LCD_RED;
	}*/
	memset(str,0,sizeof(str));
	sprintf(str,"%0.1f",temp);
	LCD_SetNumber(str,&word);
	
	//°
	word.x_axis = 70 + (img_tool_ambient_pa.height + 24)*2 + img_tool_ambient_height.height/2 - 18/2;
	word.y_axis = starty2;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_C_22X18);
	
	//C
//	word.x_axis -= 5;
//	word.y_axis += 8;
	
//	LCD_SetLine(word.x_axis + 25,2,40,148,LCD_WHITE);
	
	//指示右
//	word.x_axis = 116;
//	word.y_axis = 220;
//	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Pointing_Right_12X8);
}

void gui_tool_ambient_paint(float pres,float alt,float temp)
{
	//将背景设置为黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	

	//环境大图标
	LCD_SetPicture(15,LCD_CENTER_JUSTIFIED,LCD_NONE,LCD_NONE,&img_tools_big_lists[4]);
	

	gui_tool_ambient_value(pres, alt, temp);
	
	//时间
	//gui_bottom_time();
	
	//ok键提示
	//gui_button_ok(LCD_WHITE);
}

void gui_tool_ambient_menu_paint(void)
{
	SetWord_t word = {0};
	
	//将背景设置为白色
	LCD_SetBackgroundColor(LCD_BLACK);


    /*环境监测*/
    word.x_axis = 120 - 24/2 - 36 - 24 - 16;
    word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)ToolStrs[SetValue.Language][7],&word);

    LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
    LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

	word.x_axis = 120 - 24/2;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_CYAN;
    word.bckgrndcolor = LCD_NONE;
	if (m_gui_ambient_menu_index == GUI_TOOL_AMBIENT_MENU_PRESSURE)
	 {
		 LCD_SetString((char *)AmbientStrs[SetValue.Language][0],&word);
	 }
	 else
		 {
		   
		   LCD_SetString((char *)AmbientStrs[SetValue.Language][1],&word);
	
	
		}

   
    word.x_axis = 120 + 24/2 + 36 + 24;
    word.forecolor = LCD_GRAY;
	word.size = LCD_FONT_16_SIZE;
    if (m_gui_ambient_menu_index == GUI_TOOL_AMBIENT_MENU_PRESSURE)
    {
		LCD_SetString((char *)AmbientStrs[SetValue.Language][1],&word);
    }
	else
		{
          
		  LCD_SetString((char *)AmbientStrs[SetValue.Language][0],&word);


	   }

	
	gui_page_down(LCD_PERU);
	
	
}

void gui_tool_ambient_pressure_chart(void)
{
	GraphStr  graph = {0};
	uint8_t min_x,max_x;
	uint8_t min_y,max_y;
	uint8_t min_i,max_i;
	uint8_t x_start,y_start,x_len,y_len;
	SetWord_t word = {0};
	char str_min[10],str_max[10];
	uint8_t n;
	
	x_start = 30;	//X坐标轴起点位置
	x_len = 180;	//X坐标轴长度
	y_start = 74;	//Y坐标轴起点位置
	y_len = 112;    //Y坐标轴长度
	
	//X轴
	LCD_SetRectangle(y_start + y_len, 1, x_start, x_len, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//Y轴
	LCD_SetRectangle(y_start, y_len, x_start, 1, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//Y轴刻度
	LCD_SetRectangle(y_start + (y_len/4), 1, 26, 4, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	LCD_SetRectangle(y_start + (y_len/4*2), 1, 26, 4, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	LCD_SetRectangle(y_start + (y_len/4*3), 1, 26, 4, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	if(AmbientData.PressureSum < AMBIENT_DATA_MAX_NUM)
	{
		n = 0;
	}
	else
	{
		n = AmbientData.PressureNum;
	}
	
	max_i = n;
	min_i = n;
	graph.max = AmbientData.PressureData[max_i];
	graph.min = AmbientData.PressureData[min_i];
	
	for(uint16_t i = 0; i < AmbientData.PressureSum; i++)
	{
		if(AmbientData.PressureData[n] >= graph.max)
		{
			graph.max = AmbientData.PressureData[n];
			max_i = n;
		}
		
		if(AmbientData.PressureData[n] < graph.min)
		{
			graph.min = AmbientData.PressureData[n];
			min_i = n;
		}
		
		n++;
		if(n >= AMBIENT_DATA_MAX_NUM)
		{
			n = 0;
		}
	}
	
	y_start = 96;	//曲线实际起始行
	y_len = 70;		//曲线实际高（不包括最低的值20）
	
	if((graph.max - graph.min) < (y_len/2))
	{
		graph.min = graph.max - (y_len/2);
	}
	
	//计算Y轴刻度步长
	graph.number = 3;
//	Get_Axes_Tick(&graph);
	
	uint8_t len;
	uint8_t start_point;
	uint8_t last_y,last_len,mid_y,now_y;
	uint8_t color = LCD_CORNFLOWERBLUE;
	uint8_t min_len = 20;
	
	if(AmbientData.PressureSum < AMBIENT_DATA_MAX_NUM)
	{
		n = 0;
		start_point = ((AMBIENT_DATA_MAX_NUM - AmbientData.PressureSum)*2) + x_start;
	}
	else
	{
		n = AmbientData.PressureNum;
		start_point = x_start;
	}
	
	for(uint16_t i = 0; i < AmbientData.PressureSum; i++)
	{
		len = (AmbientData.PressureData[n] - graph.min) * y_len / (graph.max - graph.min);
		now_y = y_start + y_len - len;
		
		if(i > 0)
		{
			LCD_SetRectangle(now_y, 1, start_point, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
			LCD_SetRectangle(now_y + 1, len - 1 + min_len, start_point, 1, color, 0, 0, LCD_FILL_ENABLE);
			
			if(now_y > last_y)
			{
				mid_y = (now_y - last_y)/2 + last_y;
				LCD_SetRectangle(mid_y, now_y - mid_y, start_point - 1, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				LCD_SetRectangle(now_y, len + min_len, start_point - 1, 1, color, 0, 0, LCD_FILL_ENABLE);
				
				LCD_SetRectangle(last_y, mid_y - last_y, start_point - 2, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
			}
			else if(now_y < last_y)
			{
				mid_y = (last_y - now_y)/2 + now_y;
				LCD_SetRectangle(mid_y, last_y - mid_y, start_point - 1, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				LCD_SetRectangle(last_y, last_len + min_len, start_point - 1, 1, color, 0, 0, LCD_FILL_ENABLE);
				
				LCD_SetRectangle(now_y, mid_y - now_y, start_point, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
			}
			else
			{
				LCD_SetRectangle(now_y, 1, start_point - 1, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				LCD_SetRectangle(now_y + 1, len - 1 + min_len, start_point - 1, 1, color, 0, 0, LCD_FILL_ENABLE);
			}
		}
		else
		{
			if(AmbientData.PressureSum < AMBIENT_DATA_MAX_NUM)
			{
				LCD_SetRectangle(now_y, 1, start_point, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				LCD_SetRectangle(now_y + 1, len - 1 + min_len, start_point, 1, color, 0, 0, LCD_FILL_ENABLE);
			}
		}
		
		if(n == min_i)
		{
			min_x = start_point;
			min_y = now_y;
		}
		
		if(n == max_i)
		{
			max_x = start_point;
			max_y = now_y;
		}
		
		start_point += 2;
		last_y = now_y;
		last_len = len;
		
		n++;
		if(n >= AMBIENT_DATA_MAX_NUM)
		{
			n = 0;
		}
	}
	
	if(AmbientData.PressureSum >= 2)
	{
		//标注最小值箭头
		LCD_SetPicture(min_y - Img_Pointing_Down_12X8.height - 1, 
						min_x - ((Img_Pointing_Down_12X8.width-2)/2), 
						LCD_WHITE, LCD_NONE, &Img_Pointing_Down_12X8);
		
		//标注最大值箭头
		LCD_SetPicture(max_y - Img_Pointing_Down_12X8.height - 1, 
						max_x - ((Img_Pointing_Down_12X8.width-2)/2), 
						LCD_RED, LCD_NONE, &Img_Pointing_Down_12X8);
		
		word.size = LCD_FONT_19_SIZE;
		word.forecolor = LCD_NONE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		
		//标注最小值数值
		memset(str_min,0,sizeof(str_min));
		sprintf(str_min,"%.1f",AmbientData.PressureData[min_i]/10.f);
		
		if(((strlen(str_min)*(Font_Number_16.width + word.kerning)/2)) + x_start + 2 > min_x)
		{
			//数值会在Y轴上，则向右移
			word.y_axis = x_start + 2;
		}

		else
		{
			//居中显示
			word.y_axis = min_x - (strlen(str_min)*(Font_Number_16.width + word.kerning)/2);
		}
		word.x_axis = min_y - Img_Pointing_Down_12X8.height - 1 - Font_Number_16.height - 8;
		
		word.forecolor = LCD_WHITE;
		LCD_SetNumber(str_min,&word);
		
		min_y = word.y_axis;
		min_x = word.x_axis;
		
		//标注最大值数值
		memset(str_max,0,sizeof(str_max));
		sprintf(str_max,"%.1f",AmbientData.PressureData[max_i]/10.f);
		
		if(((strlen(str_max)*(Font_Number_16.width + word.kerning)/2)) + x_start + 2 > max_x)
		{
			//数值会在Y轴上，则向右移
			word.y_axis = x_start + 2;
		}

		else
		{
			//居中显示
			word.y_axis = max_x - (strlen(str_max)*(Font_Number_16.width + word.kerning)/2);
		}
		word.x_axis = max_y - Img_Pointing_Down_12X8.height - 1 - Font_Number_16.height - 8;
		if(((word.x_axis >= min_x) && (word.x_axis <= (min_x + Font_Number_16.height)))
				|| ((word.x_axis + Font_Number_16.height  >= min_x) && (word.x_axis + Font_Number_16.height <= (min_x + Font_Number_16.height))))
		{
			if(((word.y_axis >= min_y) && (word.y_axis <= (min_y + ((Font_Number_16.width + word.kerning)*strlen(str_min)))))
				|| ((word.y_axis + ((Font_Number_16.width + word.kerning)*strlen(str_max)) >= min_y) && (word.y_axis + ((Font_Number_16.width + word.kerning)*strlen(str_max)) <= (min_y + ((Font_Number_16.width + word.kerning)*strlen(str_min))))))
			{
				//如果最大值与最小值有重叠，则将最大值向上移
				word.x_axis = min_x - (Font_Number_16.height + 2);
			}
		}
		
		if((word.y_axis + ((Font_Number_16.width + word.kerning)*strlen(str_max))) > (x_start + x_len + 6))
		{
			//防止超过右边边界
			word.y_axis = x_start + x_len + 6 - ((Font_Number_16.width + word.kerning)*strlen(str_max));
		}
		
		word.forecolor = LCD_RED;
		LCD_SetNumber(str_max,&word);
	}
}

void gui_tool_ambient_pressure_paint(float pres)
{
	SetWord_t word = {0};
//	char str[10];
	
	//将背景设置为黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
//	memset(str,0,sizeof(str));
//	sprintf(str,"%.1f",pres);
	
	//当前气压
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)AmbientStrs[SetValue.Language][0],&word);
	
	word.x_axis = 20 + 24 - 16;
	word.y_axis = 120 + 24 + 4;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString("hPa",&word);
	
	//图表
	gui_tool_ambient_pressure_chart();
	
	//过去12小时
	word.x_axis = 202;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)AmbientStrs[SetValue.Language][2],&word);
	
	//上翻提示
	gui_page_up(LCD_PERU);
	
	//下翻提示
	gui_page_down(LCD_PERU);
}

static void gui_tool_ambient_altitude_chart(void)
{
	GraphStr  graph = {0};
	uint8_t min_x,max_x;
	uint8_t min_y,max_y;
	uint8_t min_i,max_i;
	uint8_t x_start,y_start,x_len,y_len;
	SetWord_t word = {0};
	char str_min[10],str_max[10];
	uint8_t n;
	
	x_start = 30;	//X坐标轴起点位置
	x_len = 180;	//X坐标轴长度
	y_start = 74;	//Y坐标轴起点位置
	y_len = 112;    //Y坐标轴长度
	
	//X轴
	LCD_SetRectangle(y_start + y_len, 1, x_start, x_len, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//Y轴
	LCD_SetRectangle(y_start, y_len, x_start, 1, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	//Y轴刻度
	LCD_SetRectangle(y_start + (y_len/4), 1, 26, 4, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	LCD_SetRectangle(y_start + (y_len/4*2), 1, 26, 4, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	LCD_SetRectangle(y_start + (y_len/4*3), 1, 26, 4, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	if(AmbientData.AltitudeSum < AMBIENT_DATA_MAX_NUM)
	{
		n = 0;
	}
	else
	{
		n = AmbientData.AltitudeNum;
	}
	
	max_i = n;
	min_i = n;
	graph.max = AmbientData.AltitudeData[max_i];
	graph.min = AmbientData.AltitudeData[min_i];
	
	for(uint8_t i = 0; i < AmbientData.AltitudeSum; i++)
	{
		if(AmbientData.AltitudeData[n] >= graph.max)
		{
			graph.max = AmbientData.AltitudeData[n];
			max_i = n;
		}
		
		if(AmbientData.AltitudeData[n] < graph.min)
		{
			graph.min = AmbientData.AltitudeData[n];
			min_i = n;
		}
		
		n++;
		if(n >= AMBIENT_DATA_MAX_NUM)
		{
			n = 0;
		}
	}
	
	y_len = 70;
	y_start = 96;
	
	if((graph.max - graph.min) < (y_len/4))
	{
		graph.min = graph.max - (y_len/4);
	}
	
	//计算Y轴刻度步长
	graph.number = 3;
//	Get_Axes_Tick(&graph);
	
	uint8_t len;
	uint8_t start_point;
	uint8_t last_y,last_len,mid_y,now_y;
	uint8_t color = LCD_FORESTGREEN;
	uint8_t min_len = 20;
	
	if(AmbientData.AltitudeSum < AMBIENT_DATA_MAX_NUM)
	{
		n = 0;
		start_point = ((AMBIENT_DATA_MAX_NUM - AmbientData.AltitudeSum)*2) + x_start;
	}
	else
	{
		n = AmbientData.AltitudeNum;
		start_point = x_start;
	}
	
	for(uint16_t i = 0; i < AmbientData.AltitudeSum; i++)
	{
		len = (AmbientData.AltitudeData[n] - graph.min) * y_len / (graph.max - graph.min);
		now_y = y_start + y_len - len;
		
		if(i > 0)
		{
			LCD_SetRectangle(now_y, 1, start_point, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
			LCD_SetRectangle(now_y + 1, len - 1 + min_len, start_point, 1, color, 0, 0, LCD_FILL_ENABLE);
			
			if(now_y > last_y)
			{
				mid_y = (now_y - last_y)/2 + last_y;
				LCD_SetRectangle(mid_y, now_y - mid_y, start_point - 1, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				LCD_SetRectangle(now_y, len + min_len, start_point - 1, 1, color, 0, 0, LCD_FILL_ENABLE);
				
				LCD_SetRectangle(last_y, mid_y - last_y, start_point - 2, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
			}
			else if(now_y < last_y)
			{
				mid_y = (last_y - now_y)/2 + now_y;
				LCD_SetRectangle(mid_y, last_y - mid_y, start_point - 1, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				LCD_SetRectangle(last_y, last_len + min_len, start_point - 1, 1, color, 0, 0, LCD_FILL_ENABLE);
				
				LCD_SetRectangle(now_y, mid_y - now_y, start_point, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
			}
			else
			{
				LCD_SetRectangle(now_y, 1, start_point - 1, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				LCD_SetRectangle(now_y + 1, len - 1 + min_len, start_point - 1, 1, color, 0, 0, LCD_FILL_ENABLE);
			}
		}
		else
		{
			if(AmbientData.AltitudeSum < AMBIENT_DATA_MAX_NUM)
			{
				LCD_SetRectangle(now_y, 1, start_point, 1, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
				LCD_SetRectangle(now_y + 1, len - 1 + min_len, start_point, 1, color, 0, 0, LCD_FILL_ENABLE);
			}
		}
			
		if(n == min_i)
		{
			min_x = start_point;
			min_y = now_y;
		}
		
		if(n == max_i)
		{
			max_x = start_point;
			max_y = now_y;
		}
		
		start_point += 2;
		last_y = now_y;
		last_len = len;
		
		n++;
		if(n >= AMBIENT_DATA_MAX_NUM)
		{
			n = 0;
		}
	}
	
	if(AmbientData.AltitudeSum >= 2)
	{
		//标注最小值箭头
		LCD_SetPicture(min_y - Img_Pointing_Down_12X8.height - 1, 
						min_x - ((Img_Pointing_Down_12X8.width-2)/2), 
						LCD_WHITE, LCD_NONE, &Img_Pointing_Down_12X8);
		
		//标注最大值箭头
		LCD_SetPicture(max_y - Img_Pointing_Down_12X8.height - 1, 
						max_x - ((Img_Pointing_Down_12X8.width-2)/2), 
						LCD_RED, LCD_NONE, &Img_Pointing_Down_12X8);
		
		word.size = LCD_FONT_19_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		
		//标注最小值数值
		memset(str_min,0,sizeof(str_min));
		sprintf(str_min,"%d",AmbientData.AltitudeData[min_i]);
		
		if(((strlen(str_min)*(Font_Number_16.width + word.kerning)/2)) + x_start + 2 > min_x)
		{
			//数值会在Y轴上，则向右移
			word.y_axis = x_start + 2;
		}
//		else if(((strlen(str_min)*(Font_Number_16.width + 1)/2)) + min_x > (x_start + x_len))
//		{
//			//数值会在Y轴上，则向左移
//			word.y_axis = x_start + x_len - (strlen(str_min)*(Font_Number_16.width + 1));
//		}
		else
		{
			//居中显示
			word.y_axis = min_x - (strlen(str_min)*(Font_Number_16.width + word.kerning)/2);
		}
		word.x_axis = min_y - Img_Pointing_Down_12X8.height - 1 - Font_Number_16.height - 8;
		LCD_SetNumber(str_min,&word);
		
		min_y = word.y_axis;
		min_x = word.x_axis;
		
		//标注最大值数值
		memset(str_max,0,sizeof(str_max));
		sprintf(str_max,"%d",AmbientData.AltitudeData[max_i]);
		
		if(((strlen(str_max)*(Font_Number_16.width + word.kerning)/2)) + x_start + 2 > max_x)
		{
			//数值会在Y轴上，则向右移
			word.y_axis = x_start + 2;
		}
//		else if(((strlen(str_max)*(Font_Number_16.width + word.kerning)/2)) + max_x > (x_start + x_len))
//		{
//			//数值会在Y轴上，则向左移
//			word.y_axis = x_start + x_len - (strlen(str_max)*(Font_Number_16.width + word.kerning));
//		}
		else
		{
			//居中显示
			word.y_axis = max_x - (strlen(str_max)*(Font_Number_16.width + word.kerning)/2);
		}
		word.x_axis = max_y - Img_Pointing_Down_12X8.height - 1 - Font_Number_16.height - 8;
		if(((word.x_axis >= min_x) && (word.x_axis <= (min_x + Font_Number_16.height)))
				|| ((word.x_axis + Font_Number_16.height  >= min_x) && (word.x_axis + Font_Number_16.height <= (min_x + Font_Number_16.height))))
		{
			if(((word.y_axis >= min_y) && (word.y_axis <= (min_y + ((Font_Number_16.width + word.kerning)*strlen(str_min)))))
				|| ((word.y_axis + ((Font_Number_16.width + word.kerning)*strlen(str_max)) >= min_y) && (word.y_axis + ((Font_Number_16.width + word.kerning)*strlen(str_max)) <= (min_y + ((Font_Number_16.width + word.kerning)*strlen(str_min))))))
			{
				//如果最大值与最小值有重叠，则将最大值向上移
				word.x_axis = min_x - (Font_Number_16.height + 2);
			}
		}
		
		if((word.y_axis + ((Font_Number_16.width + word.kerning)*strlen(str_max))) > (x_start + x_len + 6))
		{
			//防止超过右边边界
			word.y_axis = x_start + x_len + 6 - ((Font_Number_16.width + word.kerning)*strlen(str_max));
		}
		
		word.forecolor = LCD_RED;
		LCD_SetNumber(str_max,&word);
	}
}

void gui_tool_ambient_altitude_paint(float alt)
{
	SetWord_t word = {0};
//	char str[10];
	
	//将背景设置为黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	
	
	//当前高度
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)AmbientStrs[SetValue.Language][1],&word);
	
	word.x_axis = 20 + 24 - 16;
	word.y_axis = 120 + 24 + 4;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString("m",&word);
	
	//图表
	gui_tool_ambient_altitude_chart();
	
	//过去5小时
	word.x_axis = 202;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)AmbientStrs[SetValue.Language][3],&word);
	
	//上翻提示
	gui_page_up(LCD_PERU);

	//ok键提示
	//gui_button_ok(LCD_BLACK);
	
	//下翻提示
	gui_page_down(LCD_PERU);
}

void gui_tool_ambient_altitude_calibration_paint(void)
	{
			SetWord_t word = {0};
			char str[10];
			
			//将背景设置为黑色
			LCD_SetBackgroundColor(LCD_BLACK);
			
			
			//高度校准
			word.x_axis = 30;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_24_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 0;
			LCD_SetString((char *)AmbientStrs[SetValue.Language][4],&word);
			
			memset(str,0,sizeof(str));
			sprintf(str,"%04d",m_gui_ambient_altitude);
		
			/*+ -号*/
			word.x_axis = (LCD_PIXEL_ONELINE_MAX - 56)/2;
			word.kerning = 2;
			word.y_axis = (LCD_PIXEL_ONELINE_MAX - Font_Number_56.width - ((Font_Number_56.width+word.kerning)*strlen(str)-word.kerning) - Img_M_30X28.width - 8 - 8)/2;
			word.size = LCD_FONT_56_SIZE;
			word.forecolor = LCD_CYAN;
			word.bckgrndcolor = LCD_NONE;
			
			if(m_gui_ambient_char)
			{
				LCD_SetNumber("+",&word);
			}
			else
			{
				LCD_SetNumber("-",&word);
			}
		
		   /*校准值*/
			word.x_axis =  120 - Font_Number_56.height/2;
			word.y_axis += (Font_Number_56.width + 8);
			word.forecolor = LCD_WHITE;
			LCD_SetNumber(str,&word);
		
			/*m*/
			word.x_axis = 120 + Font_Number_56.height/2 - Img_M_30X28.height;
			word.y_axis += (Font_Number_56.width*4 + 3* word.kerning + 8);
			LCD_SetPicture(word.x_axis, word.y_axis, LCD_WHITE, LCD_NONE, &Img_M_30X28);
			
			//选择设置位
			word.x_axis += (Img_M_30X28.height + 8);
			if(m_gui_ambient_set==0)
				{
				  
				  word.y_axis = (LCD_PIXEL_ONELINE_MAX - Font_Number_56.width - ((Font_Number_56.width+word.kerning)*strlen(str)-word.kerning) - Img_M_30X28.width - 8 - 8)/2;
	
			   }
			else
				{
			word.y_axis = ((LCD_PIXEL_ONELINE_MAX - Font_Number_56.width - ((Font_Number_56.width+word.kerning)*strlen(str)-word.kerning) - Img_M_30X28.width - 8 - 8)/2
							+  Font_Number_56.width + 8 + (m_gui_ambient_set-1)*(Font_Number_56.width + 2));
				}
			LCD_SetRectangle(word.x_axis, 4, word.y_axis, Font_Number_56.width, LCD_CYAN, 0, 0, LCD_FILL_ENABLE);
		}


void gui_tool_ambient_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			ScreenState = DISPLAY_SCREEN_AMBIENT_MENU;
			m_gui_ambient_menu_index = GUI_TOOL_AMBIENT_MENU_PRESSURE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//返回工具菜单界面
			tool_task_close();
			ScreenState = DISPLAY_SCREEN_TOOL;
			msg.cmd = MSG_DISPLAY_SCREEN;
			msg.value = Get_Curr_Tool_Index();
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_tool_ambient_menu_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			if(m_gui_ambient_menu_index != GUI_TOOL_AMBIENT_MENU_ALTITUDE)
			{
				//气压界面
				ScreenState = DISPLAY_SCREEN_AMBIENT_PRESSURE;
			}
			else
			{
				//海拔界面
				ScreenState = DISPLAY_SCREEN_AMBIENT_ALTITUDE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if(m_gui_ambient_menu_index > 0)
			{
				m_gui_ambient_menu_index--;
				
			}
			else
				{
                 m_gui_ambient_menu_index = GUI_TOOL_AMBIENT_MENU_MAX;

			    }
			msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_DOWN:
			if(m_gui_ambient_menu_index < GUI_TOOL_AMBIENT_MENU_MAX)
			{
				m_gui_ambient_menu_index++;
				
			}
			else
				{
                m_gui_ambient_menu_index = 0;

			   }
			msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			break;
		case KEY_BACK:
			//返回环境监测界面
			ScreenState = DISPLAY_SCREEN_AMBIENT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_tool_ambient_pressure_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
#if defined GUI_TOOL_AMBIENT_NEW_UI
		case KEY_UP:
		case KEY_DOWN:
			//查看过去5小时海拔
			ScreenState = DISPLAY_SCREEN_AMBIENT_ALTITUDE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//返回环境监测界面
			ScreenState = DISPLAY_SCREEN_AMBIENT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
#else
		case KEY_BACK:
			//返回环境监测菜单界面
			ScreenState = DISPLAY_SCREEN_AMBIENT_MENU;
			m_gui_ambient_menu_index = GUI_TOOL_AMBIENT_MENU_PRESSURE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
#endif
		default:
			break;
	}
}

void gui_tool_ambient_altitude_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	float pres,alt,temp;
	
	switch(Key_Value)
	{
		case KEY_OK:
			//高度校准界面
			tool_ambient_value_get(&pres, &alt, &temp);
			m_gui_ambient_altitude = (int16_t)(SetValue.Altitude_Offset + alt);
			if(m_gui_ambient_altitude >= 0)
			{
				//正数
				m_gui_ambient_char = 1;
			}
			else
			{
				//负数
				m_gui_ambient_char = 0;
				m_gui_ambient_altitude = abs(m_gui_ambient_altitude);
			}
			
			if(m_gui_ambient_altitude > 9999)
			{
				//防止超过4位
				m_gui_ambient_altitude = 9999;
			}
			
			//从正负号开始设置
			m_gui_ambient_set = 0;
		
			ScreenState = DISPLAY_SCREEN_AMBIENT_ALTITUDE_CALIBRATION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
#if defined GUI_TOOL_AMBIENT_NEW_UI
		case KEY_UP:
		case KEY_DOWN:
			//查看过去12小时气压
			ScreenState = DISPLAY_SCREEN_AMBIENT_PRESSURE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//返回环境监测界面
			ScreenState = DISPLAY_SCREEN_AMBIENT;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
#else
		case KEY_BACK:
			//返回环境监测菜单界面
			ScreenState = DISPLAY_SCREEN_AMBIENT_MENU;
			m_gui_ambient_menu_index = GUI_TOOL_AMBIENT_MENU_ALTITUDE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
#endif
		default:
			break;
	}
}

void gui_tool_ambient_altitude_calibration_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	uint8_t g,s,b,q;
	float pres,alt,temp;
	
	switch(Key_Value)
	{
		case KEY_OK:
			if(m_gui_ambient_set < 4)
			{
				m_gui_ambient_set++;
			}
			else
			{
				//保存设置
				tool_ambient_value_get(&pres, &alt, &temp);
				if(m_gui_ambient_char)
				{
					SetValue.Altitude_Offset = (m_gui_ambient_altitude - alt);
				}
				else
				{
					SetValue.Altitude_Offset = (-m_gui_ambient_altitude - alt);
				}
				
				ScreenState = DISPLAY_SCREEN_AMBIENT_ALTITUDE;
			}
			
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if(m_gui_ambient_set == 0)
			{
				m_gui_ambient_char = !m_gui_ambient_char;
			}
			else 	
			{
				q = abs(m_gui_ambient_altitude)/1000;
				b = abs(m_gui_ambient_altitude)%1000/100;
				s = abs(m_gui_ambient_altitude)%100/10;
				g = abs(m_gui_ambient_altitude)%10;
				if(m_gui_ambient_set == 1)
				{
					if(q < 9)
					{
						q++;
					}
					else
					{
						q = 0;
					}
				}
				else if(m_gui_ambient_set == 2)
				{
					if(b < 9)
					{
						b++;
					}
					else
					{
						b = 0;
					}
				}
				else if(m_gui_ambient_set == 3)
				{
					if(s < 9)
					{
						s++;
					}
					else
					{
						s = 0;
					}
				}
				else
				{
					if(g < 9)
					{
						g++;
					}
					else
					{
						g = 0;
					}
				}
				m_gui_ambient_altitude = (q*1000)+(b*100)+(s*10)+g;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_DOWN:
			if(m_gui_ambient_set == 0)
			{
				m_gui_ambient_char = !m_gui_ambient_char;
			}
			else
			{
				q = abs(m_gui_ambient_altitude)/1000;
				b = abs(m_gui_ambient_altitude)%1000/100;
				s = abs(m_gui_ambient_altitude)%100/10;
				g = abs(m_gui_ambient_altitude)%10;
				if(m_gui_ambient_set == 1)
				{
					if(q > 0)
					{
						q--;
					}
					else
					{
						q = 9;
					}
				}
				else if(m_gui_ambient_set == 2)
				{
					if(b > 0)
					{
						b--;
					}
					else
					{
						b = 9;
					}
				}
				else if(m_gui_ambient_set == 3)
				{
					if(s > 0)
					{
						s--;
					}
					else
					{
						s = 9;
					}
				}
				else
				{
					if(g > 0)
					{
						g--;
					}
					else
					{
						g = 9;
					}
				}
				m_gui_ambient_altitude = (q*1000)+(b*100)+(s*10)+g;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			if(m_gui_ambient_set > 0)
			{
				m_gui_ambient_set--;
			}
			else
			{
				//返回环境监测高度界面
				ScreenState = DISPLAY_SCREEN_AMBIENT_ALTITUDE;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

//坐标轴范围及刻度的自适应算法
void Get_Axes_Tick(GraphStr *graph)
{
	float cormin,cormax;
	float corstep,tmpstep,temp;
	int cornumber,tmpnumber,extranumber;
	
	cormax = graph->max;
	cormin = graph->min;
	cornumber = graph->number;
	
	if(cormax < cormin)
	{
		return ;
	}
	else if(cormax == cormin)
	{
		cormin = 0;
	}

	corstep=(cormax-cormin)/cornumber;
	if(pow(10,(int)(log(corstep)/log(10)))==corstep)
	{
		temp = pow(10,(int)(log(corstep)/log(10)));
	}
	else
	{
		temp = pow(10,((int)(log(corstep)/log(10))+1));
	}
	//将步长修正到(0,1)间
	tmpstep = (corstep/temp);
	//选取规范步长
	if((tmpstep>=0.f) && (tmpstep<=0.1f))
	{
		tmpstep = 0.1f;
	}
	else if((tmpstep>=0.100001f) && (tmpstep<=0.2f))
	{
		tmpstep = 0.2f;
	}
	else if((tmpstep>=0.200001f) && (tmpstep<=0.25f))
	{
		tmpstep = 0.25f;
	}
	else if((tmpstep>=0.250001f) && (tmpstep<=0.5f))
	{
		tmpstep = 0.5f;
	}
	else
	{
		tmpstep = 1.f;
	}
	tmpstep = tmpstep * temp;
	if((int)(cormin/tmpstep) != (cormin/tmpstep))
	{
		if(cormin<0)
		{
			cormin = (-1) * ceil(fabs(cormin/tmpstep))*tmpstep;
		}
		else
		{
			cormin = (int)(fabs(cormin/tmpstep))*tmpstep;
		}

	}
	if((int)(cormax/tmpstep) != (cormax/tmpstep))
	{
		cormax = (int)(cormax/tmpstep+1)*tmpstep;
	}
	tmpnumber = (cormax-cormin)/tmpstep;
	if(tmpnumber<cornumber)
	{
		extranumber = cornumber - tmpnumber;
		tmpnumber = cornumber;
		if(extranumber%2 == 0)
		{
			cormax = cormax + tmpstep*(int)(extranumber/2);
		}
		else
		{
			cormax = cormax + tmpstep*(int)(extranumber/2+1);
		}
		cormin = cormin - tmpstep*(int)(extranumber/2);
	}
	cornumber = tmpnumber;
	
	graph->max = cormax;
	graph->min = cormin;
	graph->number = cornumber;
	graph->step = tmpstep;
}

