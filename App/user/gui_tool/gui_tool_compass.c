#include "gui_tool_config.h"
#include "gui_tool_compass.h"

#include "drv_lcd.h"
#include "drv_key.h"
#include "drv_lis3mdl.h"
#include "time_notify.h"
#include "algo_magn_cal.h"

#include "task_tool.h"


#include "gui_tool.h"

#include "com_data.h"

#include "font_config.h"

#if DEBUG_ENABLED == 1 && GUI_TOOL_COMPASS_LOG_ENABLED == 1
	#define GUI_TOOL_COMPASS_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_COMPASS_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_COMPASS_WRITESTRING(...)
	#define GUI_TOOL_COMPASS_PRINTF(...)		        
#endif

static uint16_t m_Bubble_x;	//ָ��������������
static uint16_t m_Bubble_y;	//ָ��������������

extern SetValueStr SetValue;

void gui_tool_compass_bubble_set(uint16_t x, uint16_t y)
{
	//����ָ������ѹ��λ��
	m_Bubble_x = x;
	m_Bubble_y = y;
}

void gui_tool_compass_init(void)
{
	//��ʼ��ָ�����������
	gui_tool_compass_bubble_set(LCD_CENTER_LINE, LCD_CENTER_ROW);
}

static void gui_tool_compass_bubble(uint16_t x,uint16_t y)
{
	//uint8_t Width = 80;	//�����˶����
	
	uint8_t Width = 33;	//�����˶����
	uint8_t StartX = (LCD_LINE_CNT_MAX - Width)/2;	
	uint8_t StartY = (LCD_PIXEL_ONELINE_MAX - Width)/2;
	
	if(x < StartX)
	{
		x = StartX;
	}
	else if(x > (StartX+Width))
	{
		x = StartX+Width;
	}
	
	if(y < StartY)
	{
		y = StartY;
	}
	else if(y > (StartY+Width))
	{
		y = StartY+Width;
	}


	
	x -= (img_tool_compass_point.height/2);
	y -= (img_tool_compass_point.width/2);
	LCD_SetPicture(x, y, LCD_WHITE, LCD_NONE ,&img_tool_compass_point);
}

void gui_tool_compass_paint(float angle)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t i;
	uint16_t mil;
	uint8_t mil_h,mil_l;
	uint16_t a,b,tmp;
	uint16_t u16Angle = (uint16_t)angle;
	
	//���ñ���Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//ָ�����Բ��
   LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,55,4,0,360,LCD_WHITE);


	
	LCD_SetRectangle(0, 15, 118, 4, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);//�Ϸ�����
	//�����϶�
	u16Angle %= 360;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	
	for(i = 0; i < 4; i++)
	{
		tmp = 360 - u16Angle;
		
		get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, tmp, 55, &a, &b);
		tmp = (tmp + 350) % 360;
		
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 55, 4, tmp, tmp + 20, LCD_BLACK);
		word.x_axis = b - 8;
		word.y_axis = a - 8;
		if(i==0)
			{
           
		   word.forecolor = LCD_RED;

		   }
		else
			{
            
			word.forecolor = LCD_WHITE;

		   }
		LCD_SetString((char *)CompassStrs[SetValue.Language][i],&word);
		u16Angle = (u16Angle + 90) % 360;
	}

	
	word.forecolor = LCD_WHITE;
	//����
	gui_tool_compass_bubble(m_Bubble_x, m_Bubble_y);

	/*СԲ��4������*/
	//LCD_SetRectangle(76, 17, 120, 2, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
   // LCD_SetRectangle(147, 17, 120, 2, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
   // LCD_SetRectangle(120, 2, 76, 17, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
   // LCD_SetRectangle(120, 2, 147, 17, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	
	
	LCD_SetPicture(120-33/2, 120-33/2, LCD_RED, LCD_NONE, &img_tool_compass_circle);
	

	//�Ƕ�
	memset(str,0,sizeof(str));
	sprintf(str,"%d",u16Angle);
	word.x_axis = 20;
	
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_29_SIZE;
	LCD_SetNumber(str,&word);

	/*��*/
	word.y_axis = 120 + (strlen(str) * (Font_Number_29.width + 1))/2;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_WHITE ,LCD_NONE, &img_tool_compass_flag_degree);
	

	//��λ
	word.x_axis = 200;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_19_SIZE;
    mil = angle * 6000/360;
    mil_l = mil % 100;
    mil_h = mil / 100;
	memset(str,0,sizeof(str));
	sprintf(str,"%d-%02d",mil_h,mil_l);
    LCD_SetNumber(str,&word);	
}

void gui_tool_compass_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
			//ָ����У׼����,��ʼ��У׼ֵ
			magn_para_init();
		
			ScreenState = DISPLAY_SCREEN_COMPASS_CALIBRATION;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			//���ع��߲˵�����
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

void gui_tool_compass_cail_update(uint8_t type)
{
	SetWord_t word = {0};
	
	uint8_t color = LCD_BLACK;		
	
    LCD_SetRectangle(120 - 100/2 ,100,0,240,color,0,0,LCD_FILL_ENABLE);
	
	//�ƶ���СԲ��
	if(type == 5)
	{
		word.x_axis =110;
		word.y_axis =110;
		
       
	   LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&img_tool_compass_cal_point);

	
      
		
        LCD_SetPicture( 120 - 69/2,LCD_CENTER_JUSTIFIED,LCD_GRAY,LCD_NONE,&img_tool_compass_cal_track);

		
	}
	else
	{
		//��8��
		word.x_axis = 120 - 69/2;
		word.y_axis = 25;
	  
	  LCD_SetPicture(word.x_axis,LCD_CENTER_JUSTIFIED,LCD_GRAY,LCD_NONE,&img_tool_compass_cal_track);
		
		if(type == 0)
		{
			word.x_axis = 82;
			word.y_axis = 163;
		}
		else if(type == 1)
		{
			
			word.x_axis = 110;
			word.y_axis = 110;
		}
		else if(type == 2)
		{
			word.x_axis = 145;
			word.y_axis = 71;
		}
		else if(type == 3)
		{
			word.x_axis = 114;
			word.y_axis = 36;
		}
		else if(type == 4)
		{
			word.x_axis = 82;
			word.y_axis = 72;
		}
		else if(type == 6)
		{
			word.x_axis = 145;
			word.y_axis = 161;
		}
		else    //7
		{
			word.x_axis = 114;
			word.y_axis = 190;
		}
		
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&img_tool_compass_cal_point);
		
	}
}

void gui_tool_compass_cali_paint(uint8_t type)
{
	SetWord_t word = {0};
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//ָ����У׼
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_BLACK;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)CompassCalibrationStrs[SetValue.Language][2],&word);


	//����ֱ�
	word.x_axis = 180;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_BLACK;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)CompassCalibrationStrs[SetValue.Language][0],&word);
	//�밴�켣У׼
	word.x_axis += 22;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetString((char *)CompassCalibrationStrs[SetValue.Language][1],&word);
	
	gui_tool_compass_cail_update(type);
}

void gui_tool_compass_cali_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_BACK:
			//����ָ�������
			//GUI_TOOL_COMPASS_PRINTF("Forced return\r\n");
			magn_para_reset();//У׼������ǿ���˳�������У׼��ز����ͽṹ��
			ScreenState = DISPLAY_SCREEN_COMPASS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
