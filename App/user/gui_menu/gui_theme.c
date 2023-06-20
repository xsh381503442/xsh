#include "gui_theme.h"
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

#define GUI_THEME_LOG_ENABLED 0

#if DEBUG_ENABLED == 1 && GUI_THEME_LOG_ENABLED == 1
	#define GUI_MENU_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_MENU_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_MENU_WRITESTRING(...)
	#define GUI_MENU_PRINTF(...)		        
#endif

#define BACKGROUND_COLOR      LCD_WHITE
#define FOREGROUND_COLOR      LCD_BLACK


	#define GUI_THEME_SCREEN_THEME_0		0	//默认主题

	#define	GUI_THEME_SCREEN_THEME_APPLY	0	//应用
	#define	GUI_THEME_SCREEN_THEME_CUSTOM	1	//自定义

	#define GUI_THEME_CUSTOM_COLOR_SUM		10
	static uint8_t m_theme_cuttom_color_buf[GUI_THEME_CUSTOM_COLOR_SUM] = {
							LCD_WHITE, 
							LCD_DARKTURQUOISE, LCD_TEAL, LCD_GREEN, 
							LCD_LIME, LCD_YELLOW, LCD_PERU, 
							LCD_PINK, LCD_HOTPINK, LCD_RED};

	 uint8_t m_theme_index = GUI_THEME_SCREEN_THEME_0;
	static uint8_t m_theme_setting_index;
	static uint8_t m_theme_custom_index;





static uint8_t m_theme_num;
ThemeDataStr m_theme_data[DIAL_MAX_NUM];
static DialDataStr m_dial_data;
 
void gui_theme_init(uint8_t type)
{
	uint8_t i;
	uint32_t flag,ID;
	
	m_theme_num = DIAL_DEFAULT_MAX_NUM;
	memset(m_theme_data,0,sizeof(ThemeDataStr)*(DIAL_MAX_NUM));
	
	//默认主题名称
	for(i = 0; i < DIAL_DEFAULT_MAX_NUM; i++)
	{
		memcpy(m_theme_data[i].name,ThemeNameStrs[SetValue.Language][i],strlen(ThemeNameStrs[SetValue.Language][i]));
		m_theme_data[i].number = i;
	}
	
	dev_extFlash_enable();
	for(i = 0; i < DIAL_DOWNLOAD_MAX_NUM; i++)
	{
		ID = 0;
		flag = 0;
		//读取表盘ID
		dev_extFlash_read(DIAL_DATA_START_ADDRESS + (i * DIAL_DATA_SIZE) + sizeof(DialInfo), (uint8_t *)(&ID), 4);
		//读取表盘完整标志
		dev_extFlash_read(DIAL_DATA_START_ADDRESS + ((i + 1) * DIAL_DATA_SIZE) - 4, (uint8_t *)(&flag), 4);
		if((ID != 0xFFFFFFFF) && (flag == 0xFEFFFFFF))//此处计算总的表盘数量及表盘名字
		{
			//表盘为有效且完整
			m_theme_data[m_theme_num].number = i + DIAL_DEFAULT_MAX_NUM;
			dev_extFlash_read(DIAL_DATA_START_ADDRESS + (i * DIAL_DATA_SIZE) + sizeof(DialInfo) + 2, (uint8_t *)(m_theme_data[m_theme_num].name), GUI_THEME_NAME_SIZE);
			m_theme_num += 1;
		}
	}
	dev_extFlash_disable();
	
	if(type == 1)
	{
		//获取当前表盘索引
		m_theme_index = 0;
		
		for(i = 0; i < m_theme_num; i++)
		{
			if(m_theme_data[i].number == (SetValue.Theme & 0x7F))
			{
				m_theme_index = i;
				break;
			}
		}
		
		if(m_theme_num <= DIAL_DEFAULT_MAX_NUM)
		{
			//添加
			memcpy(m_theme_data[DIAL_DEFAULT_MAX_NUM].name,ThemeStrs[SetValue.Language][4],4);
		}
	}
	
}

static void theme( char* menu1, char* menu2, char* menu3)
{
	//LCD_SetRectangle(0,80,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
	
	LCD_SetBackgroundColor(LCD_BLACK);
	
	/*SetWord_t word = {0};
	word.x_axis = 40;		//16号字体对齐线
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = BACKGROUND_COLOR;
	word.bckgrndcolor = FOREGROUND_COLOR;
	word.kerning = 0;*/
	SetWord_t word = {0};
    word.x_axis = 120 - 24/2 - 36 - 24 - 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 0;
	/*if(m_theme_index==0)
		{
         
    
        	LCD_SetPicture( word.x_axis + 24/2 - img_set_menu_l[0].height/2,120 - 2*LCD_FONT_24_SIZE/2 - 8 - img_set_menu_l[0].width,LCD_RED,LCD_NONE,&img_set_menu_l[0]);//主题图标
           
		   LCD_DisplayGTString(menu1,&word);   //更改使用字库字体  
	     }*/
	//else
		//{
          
		  word.size = LCD_FONT_16_SIZE;
		  
		  word.forecolor = LCD_LIGHTGRAY;
		  LCD_DisplayGTString(menu1,&word);   //更改使用字库字体  


	 // }
       

	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	

	/*word.x_axis = 108; //24号字体对齐线
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = FOREGROUND_COLOR;
	word.bckgrndcolor = BACKGROUND_COLOR;*/	
	word.x_axis = 108;	 
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_24_SIZE;
	LCD_DisplayGTString(menu2,&word);	//更改使用字库字体	
	
	
	
	/*word.x_axis = 186;	
	word.size = LCD_FONT_16_SIZE;*/
	word.x_axis = 120 + 24/2 + 36 + 24;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.size = LCD_FONT_16_SIZE;
	LCD_DisplayGTString(menu3,&word);	//更改使用字库字体
	
	if(m_theme_num > 1)
	{
		//下翻提示
		
		gui_page_up(LCD_PERU);
		gui_page_down(LCD_PERU);
	}
}

void gui_theme_paint(void)
{
	LCD_SetBackgroundColor(LCD_BLACK);
	
	if(m_theme_num > DIAL_DEFAULT_MAX_NUM)
	{
	 if(m_theme_index==0)
	 	{
		/*theme((char *)MenuStrs[SetValue.Language][0],
						m_theme_data[m_theme_index].name,
						m_theme_data[(m_theme_index+1)%m_theme_num].name);*/
		theme((char *)m_theme_data[m_theme_num-1].name,
						m_theme_data[m_theme_index].name,
						m_theme_data[(m_theme_index+1)%m_theme_num].name);
	 	}
	 else
	 	{
         theme((char *)m_theme_data[m_theme_index-1].name,
						m_theme_data[m_theme_index].name,
						m_theme_data[(m_theme_index+1)%m_theme_num].name);

	    }
	}
	else
	{
		//只有默认表盘
		theme((char *)MenuStrs[SetValue.Language][0],
						m_theme_data[m_theme_index].name,
						m_theme_data[(m_theme_index+1)%(m_theme_num+1)].name);

	}
	

}

void gui_theme_btn_evt(uint32_t evt)
{
	DISPLAY_MSG  msg = {0,0};
	
	switch(evt)
	{		
		case (KEY_BACK):{
			m_theme_index = 0;
			ScreenState = DISPLAY_SCREEN_MENU;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
		}break;			

			
		case (KEY_OK):{
			if(com_dial_is_valid(m_theme_data[m_theme_index].number) != 1)
			{
				//返回主题菜单界面
			 
				ScreenState = DISPLAY_SCREEN_THEME_INVALID;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//当前表盘无效，则重新读取表盘数据
				gui_theme_init(1);
				
				ScreenStateSave = DISPLAY_SCREEN_THEME;
				timer_notify_display_start(2000,1,false);
			}
			else
			{
				if((m_theme_num <= DIAL_DEFAULT_MAX_NUM) && (m_theme_index == DIAL_DEFAULT_MAX_NUM))
				{
					//提示连接APP下载界面
					ScreenState = DISPLAY_SCREEN_THEME_PREVIEW;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else
				{
					//获取预览的表盘数据
					memset(&m_dial_data, 0, sizeof(DialDataStr));
					com_dial_get(m_theme_data[m_theme_index].number, &m_dial_data);
                     GUI_MENU_PRINTF("dial_value:%d\r\n",m_dial_data.Setting.reserve1[0]);
					ScreenState = DISPLAY_SCREEN_THEME_PREVIEW;
					m_theme_setting_index = GUI_THEME_SCREEN_THEME_APPLY;
					m_theme_custom_index = m_dial_data.Setting.color;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					
					ScreenStateSave = DISPLAY_SCREEN_THEME_APP_MENU;						
					timer_notify_display_start(2000,1,false);
				}

			}
		
		}break;	

		case (KEY_UP):{
			if(m_theme_num <= DIAL_DEFAULT_MAX_NUM)
			{
				if(m_theme_index <= 0)
				{
					m_theme_index = m_theme_num;
				}
				else
				{
					m_theme_index--;
				}
			}
			else
			{
				if(m_theme_index <= 0)
				{
					m_theme_index = m_theme_num - 1;
				}
				else
				{
					m_theme_index--;
				}
			}
			
			ScreenState = DISPLAY_SCREEN_THEME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;	
		case (KEY_DOWN):{
			if(m_theme_num <= DIAL_DEFAULT_MAX_NUM)
			{
				if(m_theme_index >= m_theme_num)
				{
					m_theme_index = 0;
				}
				else
				{
					m_theme_index++;
				}
			}
			else
			{
				if(m_theme_index >= (m_theme_num - 1))
				{
					m_theme_index = 0;
				}
				else
				{
					m_theme_index++;
				}
			}
				
			ScreenState = DISPLAY_SCREEN_THEME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;					
	}
}


void gui_theme_preview_paint(void)
{
	if((m_theme_num <= DIAL_DEFAULT_MAX_NUM) && (m_theme_index == DIAL_DEFAULT_MAX_NUM))
	{
		LCD_SetBackgroundColor(BACKGROUND_COLOR);
		
		LCD_SetRectangle(0,80,0,LCD_LINE_CNT_MAX,FOREGROUND_COLOR,0,0,LCD_FILL_ENABLE);
	
		SetWord_t word = {0};
		word.x_axis = 40;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = BACKGROUND_COLOR;
		word.bckgrndcolor = FOREGROUND_COLOR;
		word.kerning = 0;
		LCD_SetString((char *)MenuStrs[SetValue.Language][0],&word);
		
		//更多表盘主题
		word.x_axis = 120;
		word.forecolor = FOREGROUND_COLOR;
		word.bckgrndcolor = BACKGROUND_COLOR;
		LCD_SetString((char *)ThemeStrs[SetValue.Language][5],&word);
		
		//请连接APP下载
		word.x_axis += Font_Number_16.height + 8;
		LCD_SetString((char *)ThemeStrs[SetValue.Language][6],&word);
	}
	else
	{
		//预览表盘
		
		GUI_MENU_PRINTF("preview_value:%d\r\n",m_dial_data.Setting.reserve1[0]);
		gui_dial_preview(m_theme_data[m_theme_index].number, &m_dial_data);
	}
}

void gui_theme_preview_btn_evt(uint32_t evt)
{
	DISPLAY_MSG  msg = {0,0};
	
	switch(evt)
	{		
		case (KEY_BACK):
#if  defined WATCH_COM_SETTING
			if((m_theme_num <= DIAL_DEFAULT_MAX_NUM) && (m_theme_index == DIAL_DEFAULT_MAX_NUM))
			{
			
			}
			else
#endif
			{
				timer_notify_display_stop(false);
			}
		
			ScreenState = DISPLAY_SCREEN_THEME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			break;
		case (KEY_OK):
#if  defined WATCH_COM_SETTING
			if((m_theme_num <= DIAL_DEFAULT_MAX_NUM) && (m_theme_index == DIAL_DEFAULT_MAX_NUM))
			{
			
			}
			else
#endif
			{
				timer_notify_display_stop(false);
				
				ScreenState = DISPLAY_SCREEN_THEME_APP_MENU;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

static void gui_theme_app_menu_option(char* menu1, char* menu2)
{
	SetWord_t word = {0};

	//word.x_axis = 120;
	//LCD_SetRectangle(word.x_axis, 120, 0, LCD_LINE_CNT_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	//LCD_SetRectangle(word.x_axis, 1, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//menu1
	LCD_SetCircle(120,120,64,LCD_BLACK,0,1);
	LCD_SetArc(120,120,64,3,0,360,LCD_WHITE);
	//gui_progress_circle(120,120,64,3,0,360,100,LCD_WHITE,LCD_BLACK);
	word.x_axis = 120 - 24/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString(menu1,&word);
	
	//分割线
	//word.x_axis += (24 + 16);
	//LCD_SetRectangle(word.x_axis, 1, 0, LCD_LINE_CNT_MAX, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//menu2
	if(menu2 != NULL)
	{
		word.x_axis += 12;
		LCD_SetString(menu2,&word);
	}
}

static void gui_theme_app_custom_color(uint8_t index)
{
	uint8_t color;
	
	if((index == 0) || (index > GUI_THEME_CUSTOM_COLOR_SUM))
	{
		//默认颜色
		index = 0;
	}
	color = m_theme_cuttom_color_buf[index];
	
	//时
	if(m_dial_data.Setting.U_DialSetting.bits.TimeHour)
	{
		m_dial_data.Time.hourNum.numImg.forecolor = color;
	}
	
	//分
	if(m_dial_data.Setting.U_DialSetting.bits.TimeMin)
	{
		m_dial_data.Time.minNum.numImg.forecolor = color;
	}
	
	//秒
	if(m_dial_data.Setting.U_DialSetting.bits.TimeSec)
	{
		m_dial_data.Time.secNum.numImg.forecolor = color;
	}
	
	//年月日
	if(m_dial_data.Setting.U_DialSetting.bits.Date)
	{
		m_dial_data.Date.dateStr.forecolor = color;
	}
	
	//星期
	if(m_dial_data.Setting.U_DialSetting.bits.Week)
	{
		m_dial_data.Date.weekStr.forecolor = color;
	}
	
	//步数完成度
	//热量完成度
	//里程完成度
	if(m_dial_data.Setting.U_DialSetting.bits.StepDegree)
	{
		switch(m_dial_data.Status.Type1.type)
		{
			case DIAL_STATUS_TYPE_DEFAULT:
			case DIAL_STATUS_TYPE_1:
			case DIAL_STATUS_TYPE_2:
			case DIAL_STATUS_TYPE_3:
			case DIAL_STATUS_TYPE_7:
				if(index  > 0)
				{
					m_dial_data.Status.Type1.stepDeg.forecolor = color;
				}
				else
				{
					//默认为灰色
					m_dial_data.Status.Type1.stepDeg.forecolor = LCD_LIGHTGRAY;
				}
				break;
			case DIAL_STATUS_TYPE_4:
			case DIAL_STATUS_TYPE_5:
			case DIAL_STATUS_TYPE_6:
				if(index  > 0)
				{
					m_dial_data.Status.Type2.stepDeg.forecolor = color;
				}
				else
				{
					//默认为灰色
					m_dial_data.Status.Type2.stepDeg.forecolor = LCD_LIGHTGRAY;
				}
				break;
			default:
				break;
		}
	}
	
	//步数
	//里程
	//热量
	
	//目标步数
	//目标里程
	//目标热量
	
	//电量
	if(m_dial_data.Setting.U_DialSetting.bits.Battery)
	{
		m_dial_data.Battery.batImg.normalColor = color;
		m_dial_data.Battery.batImg.batImg.forecolor = color;
	}
	
	//电量完成度
	if(m_dial_data.Setting.U_DialSetting.bits.BatteryDegree)
	{
		m_dial_data.Battery.batDeg.forecolor = color;
	}

	
	//背景颜色
	if(m_dial_data.Setting.U_DialSetting.bits.BckgrndColor)
	{
		m_dial_data.Background.color = color;
	}
	
	//背景图片颜色
	if(m_dial_data.Setting.U_DialSetting.bits.BckgrndImgColor)
	{
		m_dial_data.Background.img.forecolor = color;
	}
}

void gui_theme_app_menu_paint(void)
{
	if(com_dial_is_valid(m_theme_data[m_theme_index].number) != 1)	
	{
		//返回主题菜单界面
		ScreenState = DISPLAY_SCREEN_THEME_INVALID;
		gui_theme_invalid_paint();
		
		//当前表盘无效，则重新读取表盘数据
		gui_theme_init(1);
		
		ScreenStateSave = DISPLAY_SCREEN_THEME;
		timer_notify_display_start(2000,1,false);
	}
	else
	{
		if((m_dial_data.Setting.type > 0) && (m_dial_data.Setting.type != 0xFF))
		{
			//有设置项
			gui_theme_app_custom_color(m_theme_custom_index);
		}
		
		//预览主题
		
		GUI_MENU_PRINTF("preview_1_value:%d\r\n",m_dial_data.Setting.reserve1[0]);
		gui_dial_preview(m_theme_data[m_theme_index].number, &m_dial_data);
		
		if((m_dial_data.Setting.type > 0) && (m_dial_data.Setting.type != 0xFF))
		{
			//有设置项
			if(ScreenState == DISPLAY_SCREEN_THEME_APP_CUSTOM)
			{
				//自定义
				uint8_t star_ang = 40;
				uint8_t end_ang = 140;
				uint8_t temp = (end_ang - star_ang) / GUI_THEME_CUSTOM_COLOR_SUM;

				LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, star_ang - 1, end_ang + 1, LCD_BLACK);
				LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 12, star_ang, end_ang, LCD_WHITE);
				
				LCD_SetDialScale(LCD_CENTER_LINE, LCD_CENTER_ROW, 118, 6, 
									star_ang + 1 + (m_theme_custom_index*temp), 
									star_ang + 1 + (m_theme_custom_index*temp) + temp - 2, LCD_BLACK);
			}
			else
			{
				//设置选项菜单
				if(m_theme_setting_index == GUI_THEME_SCREEN_THEME_APPLY)
				{
					//是否应用？
					gui_theme_app_menu_option((char *)ThemeStrs[SetValue.Language][1], (char *)ThemeStrs[SetValue.Language][3]);
				}
				else
				{
					//自定义
					gui_theme_app_menu_option((char *)ThemeStrs[SetValue.Language][3], NULL);
				}
				
				//下翻提示
				gui_page_down(LCD_BLACK);
			}
		}
		else
		{
			//是否应用？
			gui_theme_app_menu_option((char *)ThemeStrs[SetValue.Language][1], NULL);
		}
	}
}

void gui_theme_app_menu_btn_evt(uint32_t evt)
{
	DISPLAY_MSG  msg = {0,0};
	
	switch(evt)
	{		
		case (KEY_BACK):
			//返回主题菜单
			ScreenState = DISPLAY_SCREEN_THEME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);			
			break;
		case (KEY_OK):
			if(com_dial_is_valid(m_theme_data[m_theme_index].number) != 1)	
			{
				//返回主题菜单界面
				ScreenState = DISPLAY_SCREEN_THEME_INVALID;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				
				//当前表盘无效，则重新读取表盘数据
				gui_theme_init(1);
				
				ScreenStateSave = DISPLAY_SCREEN_THEME;
				timer_notify_display_start(2000,1,false);
			}
			else
			{
				if((m_theme_setting_index == GUI_THEME_SCREEN_THEME_APPLY)
					|| (ScreenState == DISPLAY_SCREEN_THEME_APP_CUSTOM))
				{
					if((SetValue.Theme & 0x80) && (SetValue.Theme != 0xFF))
					{
						//当前表盘为节日表盘，将表盘设置为无效
						com_dial_set_invalid(DIAL_FESTIVAL_NUM - 1);
					}
					
					if((SetValue.Theme != m_theme_data[m_theme_index].number)
						|| (m_theme_custom_index != m_dial_data.Setting.color))
					{
						if(m_theme_custom_index != m_dial_data.Setting.color)
						{
							//保存自定义表盘数据
							m_dial_data.Setting.color = m_theme_custom_index;
							if(m_theme_data[m_theme_index].number == 0)
							{
								if(m_theme_custom_index == 0)
								{
									//删除默认表盘自定义数据
									com_dial_custom_delete(DIAL_FESTIVAL_NUM);
								}
								else
								{
									//默认表盘自定义数据
									com_dial_custom_write(DIAL_FESTIVAL_NUM,&m_dial_data);
								}
							}
							else
							{
								com_dial_custom_write(m_theme_data[m_theme_index].number - 1,&m_dial_data);
							}
						}
						
						SetValue.Theme = m_theme_data[m_theme_index].number;
						
						//清除数据
						memset(&DialData, 0, sizeof(DialDataStr));
						//获取表盘数据
//						memcpy(&DialData, &m_dial_data, sizeof(DialDataStr));
						com_dial_get(SetValue.Theme, &DialData);
					}
					
					//应用当前选中表盘
					ScreenState = DISPLAY_SCREEN_THEME_APP_SUCCESS;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					
					ScreenStateSave = DISPLAY_SCREEN_HOME;
					timer_notify_display_start(1200,1,false);
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_THEME_APP_CUSTOM;
					m_theme_custom_index = m_dial_data.Setting.color;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
			}
			break;
		case KEY_UP:
			if((m_dial_data.Setting.type > 0) && (m_dial_data.Setting.type != 0xFF))
			{
				//有设置项
				if(ScreenState == DISPLAY_SCREEN_THEME_APP_CUSTOM)
				{
					if((m_theme_custom_index == 0) || (m_theme_custom_index > GUI_THEME_CUSTOM_COLOR_SUM))
					{
						m_theme_custom_index = GUI_THEME_CUSTOM_COLOR_SUM - 1;
					}
					else
					{
						m_theme_custom_index--;
					}
				}
				else
				{
					if(m_theme_setting_index == GUI_THEME_SCREEN_THEME_APPLY)
					{
						m_theme_setting_index = GUI_THEME_SCREEN_THEME_CUSTOM;
					}
					else
					{
						m_theme_setting_index = GUI_THEME_SCREEN_THEME_APPLY;
					}
				}
					
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		case KEY_DOWN:
			if((m_dial_data.Setting.type > 0) && (m_dial_data.Setting.type != 0xFF))
			{
				//有设置项
				if(ScreenState == DISPLAY_SCREEN_THEME_APP_CUSTOM)
				{
					if(m_theme_custom_index >= (GUI_THEME_CUSTOM_COLOR_SUM - 1))
					{
						m_theme_custom_index = 0;
					}
					else
					{
						m_theme_custom_index++;
					}
				}
				else
				{
					if(m_theme_setting_index == GUI_THEME_SCREEN_THEME_APPLY)
					{
						m_theme_setting_index = GUI_THEME_SCREEN_THEME_CUSTOM;
					}
					else
					{
						m_theme_setting_index = GUI_THEME_SCREEN_THEME_APPLY;
					}
				}
					
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}
			break;
		default:
			break;
	}
}

void gui_theme_app_success_paint(void)
{
	//应用表盘
	gui_dial_preview(m_theme_data[m_theme_index].number, &m_dial_data);
	
	GUI_MENU_PRINTF("preview_2_value:%d\r\n",m_dial_data.Setting.reserve1[0]);
	//表盘应用成功
	LCD_SetPicture(88, 88, LCD_NONE, LCD_NONE, &Img_Save_Success_64X64);
}

void gui_theme_app_success_btn_evt(uint32_t evt)
{	
	DISPLAY_MSG  msg = {0,0};
	
	switch(evt)
	{		
		case (KEY_BACK):
		case (KEY_OK):
			//返回主界面
			timer_notify_display_stop(false);
			
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

void gui_theme_invalid_paint(void)
{	
	//黑色背景
	LCD_SetBackgroundColor(LCD_BLACK);
	
	SetWord_t word = {0};

	//主题无效，已更新
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	LCD_SetString((char *)ThemeStrs[SetValue.Language][2],&word);
}

void gui_theme_invalid_btn_evt(uint32_t evt)
{
	DISPLAY_MSG  msg = {0,0};
	
	switch(evt)
	{		
		case (KEY_BACK):
		case (KEY_OK):
			//返回主界面
			timer_notify_display_stop(false);
			ScreenState = DISPLAY_SCREEN_THEME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}



