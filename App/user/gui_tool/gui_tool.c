#include "gui_tool_config.h"
#include "gui_tool.h"

#include "bsp_timer.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_tool.h"
#include "task_hrt.h"
#include "task_ble.h"

#include "com_data.h"

#include "gui.h"
#include "gui_sport.h"
#include "gui_heartrate.h"
#include "gui_tool_countdown.h"

#include "font_config.h"



#if DEBUG_ENABLED == 1 && GUI_TOOL_LOG_ENABLED == 1
	#define GUI_TOOL_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_TOOL_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_TOOL_WRITESTRING(...)
	#define GUI_TOOL_PRINTF(...)		        
#endif

	#define Max_Tool_Nums 	8
    const ScreenState_t m_select_tool_buf[Max_Tool_Nums] = {
			DISPLAY_SCREEN_RECOVERY,DISPLAY_SCREEN_VO2MAX,DISPLAY_SCREEN_LACTATE_THRESHOLD,
			DISPLAY_SCREEN_COUNTDOWN,DISPLAY_SCREEN_STOPWATCH, DISPLAY_SCREEN_GPS, 
			 DISPLAY_SCREEN_COMPASS,DISPLAY_SCREEN_AMBIENT
			};
		
		
  




static uint8_t Tool_Nums;			//当前界面总数

static uint8_t Curr_Tool_Num;		//当前界面序号
static ScreenState_t Record_Curr_Tool_Num;  //存储上一次界面序号
//ScreenState_t Curr_Tool_Index;	//首选监测界面，后续根据使用统计获取
static ScreenState_t Select_Tool_Index[Max_Tool_Nums];	//当前使用的界面
static ScreenState_t Remove_Tool_Index[Max_Tool_Nums];	//移除的界面

extern SetValueStr SetValue;
extern float last_co_value ;




//获取当前界面
ScreenState_t Get_Curr_Tool_Index(void)
{
	return Select_Tool_Index[Curr_Tool_Num];
}

//根据序号获取界面
ScreenState_t Get_Tool_Index(uint8_t Num)
{
	return Select_Tool_Index[Num];
}

//根据界面获取序号
uint8_t Get_Curr_Tool_Num(ScreenState_t ScreenSta)
{
	uint8_t i;
   
	for (i=0;i<Tool_Nums;i++)
	{
		if (Select_Tool_Index[i]==ScreenSta)
		{
			break;
		}
	}
	return i;
}

//获取自定义界面的个数
uint8_t Get_Tool_Nums(void)
{
    return Tool_Nums;
}

//设置当前选择界面
ScreenState_t Set_Curr_Tool_Index( ScreenState_t ScreenSta)
{
	int8_t i;
   
	for (i=0;i<Tool_Nums;i++)
	{
		if (Select_Tool_Index[i]==ScreenSta)
		{
			Curr_Tool_Num = i;
			break;
		}
	}
	return Select_Tool_Index[Curr_Tool_Num];
}


uint8_t is_custom_tool(ScreenState_t index)
{
	bool ret = 0;
	switch(index)
	{

	    case DISPLAY_SCREEN_RECOVERY:				
			if (((SetValue.U_CustomTool.val>>0) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_VO2MAX:				
			if (((SetValue.U_CustomTool.val>>1) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_LACTATE_THRESHOLD:		
			if (((SetValue.U_CustomTool.val>>2) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_COUNTDOWN:	
			if (((SetValue.U_CustomTool.val>>3) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_STOPWATCH:			
			if (((SetValue.U_CustomTool.val>>4) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_GPS:		
			if (((SetValue.U_CustomTool.val>>5) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_COMPASS:			
			if (((SetValue.U_CustomTool.val>>6) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		case DISPLAY_SCREEN_AMBIENT:		
			if (((SetValue.U_CustomTool.val>>7) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		 default:
			break;
	}

	return ret;
	
}

//自定义工具界面选择
void Set_Select_Tool_Index(void)
{
    uint8_t i,j=0,k=0;
	uint32_t Custom_Tool = SetValue.U_CustomTool.val;	//自定义工具选择，后续放参数设置
   // ScreenState_t Indextmp = DISPLAY_SCREEN_GPS;
    
    memset(Select_Tool_Index,0,sizeof(Select_Tool_Index));
	memset(Remove_Tool_Index,0,sizeof(Remove_Tool_Index));

	
	if((Custom_Tool & 0xC1) == 0)
	{
		//防止无任何选项，三项不可移除功能
		Custom_Tool = 0xC1;
	}
	
	for(i = 0; i < Max_Tool_Nums; i++)
	{
		if(is_custom_tool(m_select_tool_buf[i]) == 1) 
		{
			//已添加的界面
			Select_Tool_Index[j] = (ScreenState_t)m_select_tool_buf[i];
			j++;
		}
		else
		{
			//已移除的界面
			Remove_Tool_Index[k] = (ScreenState_t)m_select_tool_buf[i];
			k++;
		}
	}
	
    Tool_Nums = j;

	
}
 
uint8_t ToolIndexRecAdjust(void)
{
	int i;
	
	for(i = 0; i < Tool_Nums; i++)
	{
		if(Select_Tool_Index[i] == tool_menu_index_record())
			return 1;
	}
	
	return 0;
}

ScreenState_t AdjustToolIndex(void)
{
	return Select_Tool_Index[0];
}

//创建工具选择界面
void CreateToolDisplay(void)
{

	DISPLAY_MSG  msg = {0};
	ScreenState = DISPLAY_SCREEN_TOOL;	
	//SetupTimer1(2);
	msg.cmd = MSG_DISPLAY_SCREEN;
	msg.value = Get_Curr_Tool_Index();
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
}


const Index_axis tool_center_axis[] = {
//{195,163},
//{163,195},
//{120,207},
//{76, 195},
//{44, 163},
//{33, 120},
//{44, 76},
//{76, 44},
//{120, 33},
//{163, 44},
{148,35},
{193,67},	//-144 Degree
{210,120},//180 Degree	
{193,173},//144 Degree	
{148,206},//108 Degree	
{92,206},	//72 Degree	
{47,173},	//36 Degree
{30,120}, //0 Degree
{47,67},	//-36 Degree
{92,35},	//-72 Degree
{148,35},	//-108 Degree
{193,67},	//-144 Degree
{210,120},//180 Degree
{193,173},//144 Degree	
{148,208},


};


/*外圈向内圈变动，内圈坐标*/
/*const Index_axis tool_img_axis[] = 
{
 	{190-18,23+18},
	{120,0+25},
    {50+18,23+18},
	{6+18,83},
	{6+18,157},
	{50+18,217-18},
	{120,240-25},
	{190-18,217-18}

};*/
/*
sin(36) = 0.5878;
cos(36) = 0.8090;
sin(72) = 0.9511;
cos(72) = 0.3090;

*/
/*外圈向内圈变动，内圈坐标*/
const Index_axis tool_img_axis[] = 
{
 	{120 + (120 - 25)*0.5878,25 + (120 - 25) - (120 - 25)*0.8090},
	{120,25},
    {120-(120 - 25)*0.5878,25 + (120 - 25) - (120 - 25)*0.8090},
	{120-(120 - 25)*0.9511,25 + (120 - 25) - (120 - 25)*0.3090},
	{120-(120 - 25)*0.9511,120 + (120 - 25)*0.3090},
	{120-(120 - 25)*0.5878,120 + (120 - 25)*0.8090},
	{120,120 + (120 - 25)},
	{120 + (120 - 25)*0.5878,120 + (120 - 25)*0.8090}
};



const uint8_t img_tools_fore_color[] = 
{
  LCD_RED,LCD_RED,LCD_RED,LCD_RED,LCD_NONE,LCD_RED,LCD_RED,LCD_NONE

};

//显示工具选择界面
void gui_tool_select_paint(ScreenState_t state)
{
	SetWord_t word = {0};
	uint8_t index,i,screen_index;
   ScreenState_t imgnum_t;
   uint8_t imgnum=0;
    uint8_t tool_nums;
    uint8_t curr_tool_num;
    
	//if((state < DISPLAY_SCREEN_GPS) || (state > DISPLAY_SCREEN_FINDPHONE))
		
	if((state < DISPLAY_SCREEN_HOME) || (state > DISPLAY_SCREEN_FINDPHONE))
	{
		//防止索引错误
		state =	Get_Curr_Tool_Index();
	}
	index = Get_Curr_Tool_Num(state);
    tool_nums = Get_Tool_Nums();

	for(i = 0; i < Max_Tool_Nums; i++)
			{
				if(Select_Tool_Index[index] == m_select_tool_buf[i])
				{
		           screen_index = i;
					break;
				}
			}
	
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	

	
	for(curr_tool_num = 0; curr_tool_num <tool_nums; curr_tool_num ++)
	{ 

	/*选择的界面在原始索引中的位置*/
		imgnum_t = Get_Tool_Index(curr_tool_num);
			for(i = 0; i < Max_Tool_Nums; i++)
			{
				if(imgnum_t == m_select_tool_buf[i])
				{
		           imgnum = i; //在最初的界面索引中的序列号
					break;
				}
			}
  
		
		word.x_axis = tool_img_axis[curr_tool_num].x_axis - img_tools_lists[imgnum].height/2;
		word.y_axis = tool_img_axis[curr_tool_num].y_axis - img_tools_lists[imgnum].width/2;
		LCD_SetPicture(word.x_axis,word.y_axis,img_tools_fore_color[imgnum],LCD_NONE,&img_tools_lists[imgnum]);
		
    }

	   
		word.x_axis = tool_img_axis[index].x_axis - img_tools_lists[screen_index].height/2;
		word.y_axis = tool_img_axis[index].y_axis - img_tools_lists[screen_index].width/2;
		LCD_SetCircle(tool_img_axis[index].x_axis,tool_img_axis[index].y_axis,img_tools_lists[screen_index].height/2,LCD_CYAN,0,1);
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_WHITE,LCD_NONE,&img_tools_lists[screen_index]);
		
	//显示提示
	
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_BLACK;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 0;
	 LCD_SetString((char *)ToolStrs[SetValue.Language][screen_index],&word);



}

void gui_tool_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			//进入相应的监测界面
			//StopTimer1();
			ScreenState = Get_Curr_Tool_Index();

			if (ScreenState == DISPLAY_SCREEN_COUNTDOWN)
			{

			  GUI_TOOL_PRINTF("go to DISPLAY_SCREEN_COUNTDOWN\r\n");
				set_tool_countdown_index(GUI_TOOL_COUNTDOWN_MENU_START);
			}

				msg.cmd = MSG_START_TOOL;
				
				msg.value = Get_Curr_Tool_Index();
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		
			break;
		case KEY_UP:

			if( Curr_Tool_Num == Tool_Nums - 1 )
			{
				Curr_Tool_Num = 0;
				sports_menu_index_record();
			}
			else
			{
				Curr_Tool_Num++;
				sports_menu_index_record();
			}
			
			//ResetTimer1(2);
			msg.cmd = MSG_DISPLAY_SCREEN;
			msg.value = Get_Curr_Tool_Index();
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			break;
		case KEY_DOWN:	
			if( Curr_Tool_Num == 0 )
			{
				Curr_Tool_Num = Tool_Nums - 1;
				sports_menu_index_record();
			}
			else
			{
				Curr_Tool_Num--;
				sports_menu_index_record();
			}
			
			//ResetTimer1(2);
			msg.cmd = MSG_DISPLAY_SCREEN;
			msg.value = Get_Curr_Tool_Index() ;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			
			break;
		case KEY_BACK:
			//返回待机界面
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
     

ScreenState_t tool_menu_index_record(void)
{
	Record_Curr_Tool_Num = Get_Curr_Tool_Index();
	return Record_Curr_Tool_Num ;
}

