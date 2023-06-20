#include "gui_config.h"
#include "gui.h"

#include "drv_lcd.h"
#include "drv_battery.h"

#include "gui_step.h"
#include "gui_tool_ambient.h"

#include "algo_hdr.h"

#include "font_config.h"

#include "com_data.h"

#if DEBUG_ENABLED == 1 && GUI_LOG_ENABLED == 1
	#define GUI_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_WRITESTRING(...)
	#define GUI_PRINTF(...)		        
#endif

extern am_hal_rtc_time_t RTC_time;
extern _drv_bat_event g_bat_evt;

void gui_bottom_time(void)
{
	SetWord_t word = {0};
	char str[10];
	
	am_hal_rtc_time_get(&RTC_time);
	
	//清屏
	//LCD_SetRectangle(220, 12, 108, 24, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	
	LCD_SetPicture(200, LCD_CENTER_JUSTIFIED, LCD_WHITE, LCD_NONE, &img_calendar_year);
	
	memset(str,0,10);
	sprintf(str,"%d",RTC_time.ui32Year+2000);

	word.x_axis = 200;
	word.kerning = 1;
	word.y_axis = LCD_PIXEL_ONELINE_MAX/2 -(strlen(str)*(Font_Number_13.width+ word.kerning) - word.kerning + img_calendar_year.width/2+3);
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetNumber(str,&word);



	
	memset(str,0,10);
	sprintf(str,"%02d",RTC_time.ui32Month);
	word.kerning = 1;
	word.y_axis = LCD_PIXEL_ONELINE_MAX/2 + img_calendar_year.width/2 + 3;
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetNumber(str,&word);
    word.y_axis += strlen(str)*(Font_Number_13.width + word.kerning) - word.kerning + 3;

	
	LCD_SetPicture(200,  word.y_axis, LCD_WHITE, LCD_NONE, &img_calendar_month);
	
}

void gui_bottom_time_battery(uint8_t x)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t percent = g_bat_evt.level_percent;
	uint8_t color;
	
	am_hal_rtc_time_get(&RTC_time);
	
	//清屏
	LCD_SetRectangle(x, 12, 92, 56, LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	
	//底部时间
	memset(str,0,10);
	sprintf(str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);

	word.x_axis = x;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - strlen(str)*(Font_Number_8.width + 1) - 5 - Img_Battery_26X12.width) / 2;;
	word.size = LCD_FONT_8_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	//底部电量
	word.y_axis += (strlen(str)*(Font_Number_8.width + 1) + 5);
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Battery_26X12);
	
	if(percent > 100)
	{
		percent = 100;
	}
	
	if(percent <= 10)
	{
		color = LCD_RED;
	}
	else
	{
		color = LCD_WHITE;
	}
	
	//填充剩余电量
	LCD_SetRectangle(word.x_axis + 2, 8, word.y_axis + 2, percent * (Img_Battery_26X12.width - 5) / 100, color, 0, 0, LCD_FILL_ENABLE);
}

void gui_battery(uint8_t x, uint8_t forecolor)
{
	char str[10] = {0};
	uint8_t color;
	uint8_t percent = g_bat_evt.level_percent;
	SetWord_t word = {0};
	
	//百分比 图标 (居中显示)
	memset(str,0,sizeof(str));
	sprintf(str,"%d%%", percent);
	
	//百分比
	word.size 	= LCD_FONT_16_SIZE;
	word.x_axis = x;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (8*strlen(str)) - 4 - Img_Battery_26X12.width)/2;
	word.forecolor 	  = forecolor;
	word.bckgrndcolor = LCD_NONE;
	word.kerning 	  = 0;
	LCD_SetString(str, &word);

	//电量图标
	word.x_axis += 2;
	word.y_axis += (8*strlen(str) + 4);
	LCD_SetPicture(word.x_axis, word.y_axis, forecolor, LCD_BLACK, &Img_Battery_26X12);
	
	//填充图标
	if(percent <= 10)
	{
		color = LCD_RED;
	}
	else
	{
		color = LCD_LIGHTGRAY;
	}
	
	if(percent > 100)
	{
		percent = 100;
	}
	
	LCD_SetRectangle(word.x_axis + 2, 8, word.y_axis + 2, 
						percent * (Img_Battery_26X12.width - 5) / 100, 
						color, 0, 0, LCD_FILL_ENABLE);
}

void gui_page_up(uint8_t color)
{
	//上翻提示
	LCD_SetPicture(6, LCD_CENTER_JUSTIFIED, color, LCD_NONE, &Img_Pointing_Up_12X8);
}

void gui_page_down(uint8_t color)
{
	//下翻提示
	LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, color, LCD_NONE, &Img_Pointing_Down_12X8);
}

void gui_button_up(uint8_t color)
{
//	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, 60, 106, &t_a, &t_b);
	
	//up键提示
	LCD_SetPicture(62, 207, color, LCD_NONE, &Img_Pointing_RightUp_10X10);
}
void gui_button_middle(uint8_t color)
{
//	get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, 60, 106, &t_a, &t_b);
	
	//up键提示
	LCD_SetPicture(114, 6, color, LCD_NONE, &img_point_middle);
}

void gui_button_down(uint8_t color)
{
	//down键提示
	LCD_SetPicture(167, 207, color, LCD_NONE, &Img_Pointing_RightDown_10X10);
}

void gui_button_ok(uint8_t color)
{
	//ok键提示
	LCD_SetPicture(116, 6, color, LCD_NONE, &Img_Pointing_Left_12X8);
}

void gui_button_back(uint8_t color)
{
	//back键提示
	LCD_SetPicture(168, 24, color, LCD_NONE, &Img_Pointing_LeftDown_10X10);
}
/*底部星期排列显示 
*入参:星期中第几天，入参范围0~6
*如输入0，排列则从星期日起
*/
void gui_daysofweek_paint(uint8_t day)
{
	SetWord_t word = {0};
	uint8_t mDayNum = 0,mNewDayNum = 0;
	uint8_t len;
	if(ScreenState == DISPLAY_SCREEN_WEEK_AVERAGE_HDR)
		{
 
           word.x_axis = 180;
		   len = 12;
	   }
	else
		{
	   word.x_axis = 190;
	   len = 8;
		}
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	//word.kerning = 1;
	
	if(day == Sat)
	{
		day = Sun;
	}
	else if( day >= 7)
	{//如果入参错误，置day=1
		mDayNum = 1;
	}
	else
	{
		day += 1;
	}
	for(uint8_t i = 0;i < 7;i++)
	{
		mDayNum = day + i;
		
		if(mDayNum >= 7)
		{
			mNewDayNum = mDayNum - 7;
		}
		else
		{
			mNewDayNum = mDayNum;
		}
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * len) / 2 + i * (13 + len);
		
		LCD_SetPicture(word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_week_13x13[mNewDayNum]);
		
	}
}

//环形进度圈

/*
centery :x;

centerx :y


*/
void gui_progress_circle(uint8_t centery, uint8_t centerx, uint8_t radius, uint8_t thickness, 
							uint16_t startangle, uint16_t rangeangle, float percent, uint8_t forecolor, uint8_t bckgrndcolor)
{
	uint16_t ang = (uint16_t)(percent*rangeangle/100) + startangle;
	
	if((percent > 0) && (ang == startangle))
	{
		//只要有进度，但未达到1度，就显示1度
		ang = startangle + 1;
	}
	
	if((bckgrndcolor != LCD_NONE) && (percent < 100))
	{
		//如果背景色不为空，且进度不到100
		LCD_SetArc(centery, centerx, radius, thickness, ang, startangle, bckgrndcolor);
		
		//LCD_SetArc(centery, centerx, radius, thickness, ang, startangle+rangeangle, bckgrndcolor);
	}
	
	if(percent > 0)
	{
		//如果进度不为0
		LCD_SetArc(centery, centerx, radius, thickness, startangle, ang, forecolor);
		//LCD_SetArc(centery, centerx, radius+5, thickness+6, startangle, ang, forecolor);
	}
}

/*非360环形进度圈*/
void gui_progress_loop(uint8_t centery, uint8_t centerx, uint8_t radius, uint8_t thickness, 
							uint16_t startangle, uint16_t rangeangle, float percent, uint8_t forecolor, uint8_t bckgrndcolor,uint8_t flag)
{
	uint16_t ang = (uint16_t)(percent*rangeangle/100) + startangle;
	
	uint16_t up_x,up_y,down_x,down_y,centre_x,centre_y;
	if((percent > 0) && (ang == startangle))
	{
		//只要有进度，但未达到1度，就显示1度
		ang = startangle + 1;
	}
	
	if((bckgrndcolor != LCD_NONE) && (percent < 100))
	{
		//如果背景色不为空，且进度不到100-即需要画出原始进度条
		
		
		LCD_SetArc(centery, centerx, radius, thickness, ang, startangle+rangeangle, bckgrndcolor);
	}
	
	if(percent > 0)
	{
		//如果进度不为0
		LCD_SetArc(centery, centerx, radius+2, thickness+4, startangle, ang, forecolor);
	}
	
  if((flag==1)&&(percent > 0))//圆环终点处加标志圆
   {
   
    get_circle_point(centery, centerx, ang, radius+2, &up_x,&up_y);  //圆左上顶点坐标
    get_circle_point(centery, centerx, ang, radius-thickness-2 , &down_x,&down_y);  //圆右下点坐标
    centre_x = (up_x + down_x)/2;
    centre_y = (up_y + down_y)/2;
    
	LCD_SetCircle(centre_y, centre_x, (thickness+4)/2, forecolor, (thickness+4)/2, LCD_FILL_ENABLE);
	
	LCD_SetCircle(centre_y, centre_x, (thickness+4)/2, LCD_WHITE,  1, LCD_FILL_DISABLE);
	

   }
   else;

	
}

void gui_progess_toothed(uint16_t rangeangle, uint8_t toothed_num)
{
	uint8_t i,radius,thickness,weight;
	 
	int16_t startangle,endangle; 
   uint8_t percent;
 
 percent = g_bat_evt.level_percent;
for (i= 0; i< toothed_num;i++ ) 
	{ 
      startangle = i*rangeangle*1.0/toothed_num;
	  
	if(startangle%45==0)//大齿轮
		{
          radius = 104;
		  thickness = 16;
		  weight = 3;

	    }
	else//小齿轮
		{

          radius = 104;
		  thickness = 12;
		  
		  weight = 2;
	   }
	startangle = i*360/toothed_num -weight/2; 
     endangle = i*360/toothed_num  + (weight+1)/2; 
	 if(((135+weight/2)<startangle)&&(startangle<(225-weight/2)))
	 {
	  continue;
	 }
if (startangle < 0)	
	{	
	startangle = 360 + startangle;	
	}	
if (endangle < 0)	
	{	
	endangle = 360 + endangle;
	}	
		if(percent<=50)
		{
			  if((startangle>=(225-weight/2))&&(startangle<=((percent*1.0/100)*270+225)))
				{
					 
				LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,radius,thickness,startangle,endangle,LCD_CYAN); 
				}
				else
				{
					LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,radius,thickness,startangle,endangle,LCD_CORNFLOWERBLUE); 
			
			
				}


		}
	else
		{
         if((startangle<=135)&&(startangle>=(((uint16_t)((percent*1.0/100)*270+225))%360)))
         	{
             
			 LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,radius,thickness,startangle,endangle,LCD_CORNFLOWERBLUE); 
		    }
		 else
		 	{
             
			 LCD_SetArc(LCD_CENTER_LINE,LCD_CENTER_ROW,radius,thickness,startangle,endangle,LCD_CYAN); 
		   }

	   }

  }


}

static void gui_progress_circle_0(float percent, ProgressBarStr *bar)
{
	uint8_t centerx,centery;
	uint8_t radius,thickness;
	uint8_t forecolor,bckgrndcolor;
	uint16_t startangle,rangeangle;
	uint16_t n,ang;
	
	centerx 		= bar->star_x;
	centery 		= bar->star_y;
	radius 			= bar->height;
	thickness 		= bar->width;
	forecolor 		= bar->forecolor;
	bckgrndcolor 	= bar->bckgrndcolor;
	startangle 		= bar->star_ang;
	rangeangle 		= bar->range;
	
	ang = (uint16_t)(percent*rangeangle/100);
	
	if((bar->type == DIAL_PROGRESSBAR_TYPE_1)
		|| (bar->type == DIAL_PROGRESSBAR_TYPE_6))
	{
		if((percent > 0) && (ang == 0))
		{
			//只要有进度，但未达到1度，就显示1度
			ang = 1;
		}
	}

	if((bckgrndcolor != LCD_NONE) && (percent < 100))
	{
		if(bar->order)
		{
			//逆时针填充
			LCD_SetArc(centerx, centery, radius, thickness, startangle, 
						startangle + rangeangle - ang, bckgrndcolor);
		}
		else
		{
			//顺时针填充
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, radius, thickness, 
						startangle + ang, startangle + rangeangle, bckgrndcolor);
		}
	}
	
	if(ang > 0)
	{
		if(bar->order)
		{
			//逆时针
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, radius, thickness, 
						startangle + rangeangle - ang, startangle + rangeangle, forecolor);
		}
		else
		{
			//顺时针
			
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, radius, thickness, 
						startangle, startangle + ang, forecolor);
		}
	}

	if(bar->type == DIAL_PROGRESSBAR_TYPE_2)
	{
		//电量 每节长度为8，间隙为2
		ang = startangle + 8;
		n = (rangeangle + 2)/(8 + 2);
		for(uint8_t i = 0; i < n; i++)
		{
			LCD_SetDialScale(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, ang, ang + 2, LCD_BLACK);
			ang += 10;
		}
	}
}

//方形进度条
static void gui_progress_circle_1(uint8_t starx,	uint8_t stary, uint8_t height, uint8_t width, 
								float percent, uint8_t forecolor, uint8_t bckgrndcolor)
{
	if(width < height)
	{
		return;
	}
	
	//左端半圆
	if(percent > 0)
	{
		LCD_SetArc(starx + (height/2), stary + (height/2), height/2, height/2, 180, 360, forecolor);
	}
	else
	{
		LCD_SetArc(starx + (height/2), stary + (height/2), height/2, 1, 180, 360, forecolor);
	}
	
	//右端半圆
	if(percent >= 100)
	{
		LCD_SetArc(starx + (height/2), stary + width - (height/2), height/2, height/2, 0, 180, forecolor);
	}
	else
	{
		LCD_SetArc(starx + (height/2), stary + width - (height/2), height/2, 1, 0, 180, forecolor);
	}

	LCD_SetRectangle(starx, 1, stary + (height/2), width - height, forecolor, 0, 0, LCD_FILL_ENABLE);
	LCD_SetRectangle(starx + height, 1, stary + (height/2), width - height, forecolor, 0, 0, LCD_FILL_ENABLE);
	
	//进度
	LCD_SetRectangle(starx, height + 1, stary + (height/2), (width - height + 1)*percent/100, forecolor, 0, 0, LCD_FILL_ENABLE);
}

static void gui_progress_circle_2(float percent, ProgressBarStr *bar)
{
	uint8_t centerx,centery;
	uint8_t radius,thickness;
	uint8_t forecolor,bckgrndcolor;
	uint16_t startangle,rangeangle;
	uint16_t ang;
	uint8_t i,n,sum;
	
	centerx 		= bar->star_x;
	centery 		= bar->star_y;
	radius 			= bar->height;
	thickness 		= bar->width;
	forecolor 		= bar->forecolor;
	bckgrndcolor 	= bar->bckgrndcolor;
	startangle 		= bar->star_ang;
	rangeangle 		= bar->range;
	
	//总节数
	sum = (rangeangle + 2)/(6 + 2);
	//完成节数
	n = percent*sum/100;
	
	if((bckgrndcolor != LCD_NONE) && (percent < 100))
	{
		ang = startangle + (6 + 2)*n;
		for(i = 0; i < sum - n; i++)
		{
			LCD_SetDialScale(centerx, centery, radius, thickness, ang, ang + 6, forecolor);
			ang += 8;
		}
	}
	
	if(percent > 0)
	{
		ang = startangle;
		for(i = 0; i < n; i++)
		{
			LCD_SetDialScale(centerx, centery, radius, thickness, ang, ang + 6, forecolor);
			ang += 8;
		}
	}
}

void gui_progess_bar(float percent, ProgressBarStr *bar)
{
	if((bar->type == DIAL_PROGRESSBAR_TYPE_0)
		|| (bar->type == DIAL_PROGRESSBAR_TYPE_1)
		|| (bar->type == DIAL_PROGRESSBAR_TYPE_2)
		|| (bar->type == DIAL_PROGRESSBAR_TYPE_6))
	{
		//环形进度圈
		gui_progress_circle_0(percent, bar);
	}
	else if(bar->type == DIAL_PROGRESSBAR_TYPE_3)
	{
		//环形断点进度圈(步数)
		gui_progress_circle_2(percent, bar);
	}
	else if((bar->type == DIAL_PROGRESSBAR_TYPE_4)
		|| (bar->type == DIAL_PROGRESSBAR_TYPE_5))
	{
		//方形进度条
		
		gui_progress_circle_1(bar->star_x, bar->star_y, bar->height, bar->width, 
								percent, bar->forecolor, bar->bckgrndcolor);
	}
}

//里程直方图
void gui_distance_histogram(uint8_t starx, uint8_t height, uint8_t stary, uint8_t width)
{
	SetWord_t word = {0};
	GraphStr graph = {0};
	uint8_t mDayNum,mNewDayNum;
	uint8_t mileCommon = 0,mileSports = 0;
	uint32_t temp;
	char* weekBuf[] = {"SU","MO","TU","WE","TH","FR","SA"};
	
	//一周中的最大里程
	graph.max = get_weekmain_max_distance() / 100;
	if(graph.max < (SetValue.AppSet.Goal_Distance*1000))
	{
		graph.max = SetValue.AppSet.Goal_Distance*1000;
	}
//	graph.min = 0;
//	graph.number = 3;
//	Get_Axes_Tick(&graph);
	
	mDayNum = RTC_time.ui32Weekday + 1;
	if(mDayNum >= 7)
	{
		mDayNum = 0;
	}
	
	word.y_axis = stary;
	for(uint8_t i = 0;i < 7; i++)
	{
		mNewDayNum = (mDayNum + i)%7;
		
		temp = get_week_common_distance(mNewDayNum);
		mileCommon = temp/100*height/graph.max;
		if((mileCommon == 0) && (temp > 0))
		{
			mileCommon = 1;
		}
		
		temp = get_week_sport_distance(mNewDayNum);
		mileSports = temp/100*height/graph.max;
		if((mileSports == 0) && (temp > 0))
		{
			mileSports = 1;
		}
		
		//日常里程
		word.x_axis = starx + height - mileCommon;
		LCD_SetRectangle(word.x_axis, mileCommon, word.y_axis, 14, LCD_PERU, 1, 1, LCD_FILL_ENABLE);
		
		//运动里程
		word.x_axis = starx + height - mileCommon - mileSports;
		LCD_SetRectangle(word.x_axis, mileSports, word.y_axis, 14, LCD_LIGHTGREEN, 1, 1, LCD_FILL_ENABLE);
		
		//星期
		word.x_axis = starx + height + 2;
		word.y_axis -= 2;
		word.bckgrndcolor = LCD_NONE;
		word.forecolor = LCD_LIGHTGRAY;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString(weekBuf[mNewDayNum],&word);
		word.y_axis += 2;
		
		word.y_axis += 8 + 14;
	}
}

//最大摄氧量
void gui_VO2max_histogram(uint8_t starx)
{
	uint8_t i,range;
	char str[10];
	SetWord_t word = {0};
	uint16_t vo2max = DayMainData.VO2MaxMeasuring;
	
	if(vo2max > 99)
	{
		//防止最大摄氧量值异常
		vo2max = 99;
	}
	
	range = get_VO2max_range(vo2max, SetValue.AppSet.Sex,SetValue.AppSet.Age);
	
	for(i = 0; i < 6 ; i++)
	{
#if  defined WATCH_COM_SHORTCUNT_UP
		if((range == i) && (SetValue.IsMeasuredVO2max == 2)
			&& (vo2max != 0))
#else
		if((range == i)
			&& (vo2max != 0))
#endif
		{
			word.x_axis = starx;
		}
		else
		{
			word.x_axis = starx + 12;
		}
		
		word.y_axis = 2 + (i*(35+5));
		LCD_SetRectangle(word.x_axis, LCD_LINE_CNT_MAX - word.x_axis, word.y_axis, 35, get_VO2max_range_color(i), 1, 1, LCD_FILL_ENABLE);
#if  defined WATCH_COM_SHORTCUNT_UP
		if((range == i) && (SetValue.IsMeasuredVO2max == 2)
			&& (vo2max != 0))
#else
		if((range == i)
			&& (vo2max != 0))
#endif
		{
			memset(str,0,sizeof(str));
			sprintf(str,"%d",vo2max);
			
			//最大摄氧量数值
			word.x_axis = starx + 8;
			word.size = LCD_FONT_24_SIZE;
			word.forecolor = LCD_WHITE;
			word.bckgrndcolor = LCD_NONE;
			word.kerning = 1;
			word.y_axis += ((36 - (strlen(str)*(Font_Number_24.width + word.kerning)) + word.kerning)/2);
			LCD_SetNumber(str,&word);
			
			//最大摄氧量程度
			word.x_axis += (Font_Number_24.height + 8);
			word.y_axis = 2 + (i*(35+5)) + 5;
			LCD_SetPicture(word.x_axis, word.y_axis,LCD_WHITE, LCD_NONE, &Img_Vo2Max_Level[range]);
		}
	}
}
//X轴刻度上限158,下限209,差51 除3 =17;Y轴 40~200;48*N

//过去12H气压变化图
/*
starx :正顺序



*/
void gui_12h_pressure(uint8_t starx, uint8_t height, uint8_t stary, uint8_t width, uint8_t forecolor, uint8_t bckgrndcolor)
{
	uint16_t i;
	uint16_t max_v,min_v;
	int n;
	uint8_t y_len;
	uint8_t min_len = 4;
	uint8_t len;
	uint8_t start_point;
	uint8_t now_y;
	//曲线实际高（不包括最低的值）
	y_len = height - 30 - 4;//52 -4 = 48

	//X轴刻度上限158,下限209,差51 除3 =17;Y轴 40~200;48*N
	//横线
    LCD_SetRectangle(158-1,1,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(158+17-1,1,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(158+17*2-1,1,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
  
    LCD_SetRectangle(158+17*3-1,1,0,240,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);//此横线与圆的交点坐标为(208,39);(208,201);线段长201-39=162

	//竖线
	
	LCD_SetRectangle(158,51,48-1,1,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(158,51,48*2-1,1,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
    
	LCD_SetRectangle(158,51,48*3-1,1,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	
	LCD_SetRectangle(158,51,48*4-1,1,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
		if(AmbientData.PressureSum<1)
		{
		return;

		}
	
	max_v = AmbientData.PressureData[0];
	min_v = AmbientData.PressureData[0];
	
	for(i = 1; i < AmbientData.PressureSum; i++)
	{
		if(AmbientData.PressureData[i] >= max_v)
		{
			max_v = AmbientData.PressureData[i];
		}
		
		if(AmbientData.PressureData[i] < min_v)
		{
			min_v = AmbientData.PressureData[i];
		}
		
	
	}
	
	if((max_v - min_v) < (y_len/2))//最大最小值相差很小;减小最小值
	{
		min_v = max_v - (y_len/2);
	}
	
//圆的交点坐标为(208,38);(208,202);线段长202-38=164
	
	if(AmbientData.PressureSum <=82)
	{
	   if(AmbientData.PressureNum==0)
	   	{
		n = 0;
	   	}
	   else
	   	{
        
		n = AmbientData.PressureNum -1;

	    }
		start_point =  stary + width - 2 - ((82 - AmbientData.PressureSum)*2);
		
	}
	else
	{
		
		  if(AmbientData.PressureNum==0)
	   	{
		n = AMBIENT_DATA_MAX_NUM-1;
	   	}
	   else
	   	{
        
		n = AmbientData.PressureNum -1;

	    }
		start_point = stary + width -2;
	}
	
	for(i = 0; i < AmbientData.PressureSum; i++)
	{
		len = (AmbientData.PressureData[n] - min_v) * y_len / (max_v - min_v);// 2*L
		now_y = starx + y_len - len;//,对应X坐标值
		if(now_y<starx)
			{
              now_y=starx;
			

		    }
			
	
		LCD_SetRectangle(now_y, len + min_len, start_point, 2, forecolor, 0, 0, LCD_FILL_ENABLE);
		start_point -= 2;
		
		n--;
		if(n < 0)
		{
			n = AmbientData.PressureSum - 1;
		}

		if(start_point<=stary - 2)
		{
		 break;
		}
	}

	
	

}

/*
翻页进度条
page 	:第几页 (0到total-1)
total 	:共几页 (>1)
*/
void gui_page_progress_bar(uint8_t page, uint8_t total)
{
	uint16_t star_ang = 40;
	uint16_t end_ang = 140;
	uint16_t len,temp,step;
	
	if(total < 10)
	{
		step = (end_ang - star_ang) / total;
		temp = (end_ang - star_ang) % total;
		len = step;
	}
	else
	{
		len = (end_ang - star_ang) / 10;
		step = (end_ang - star_ang - len) / (total - 1);
		temp = (end_ang - star_ang - len) % (total - 1);
	}

	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 14, star_ang - 1, end_ang + 1, LCD_BLACK);
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 12, star_ang, end_ang, LCD_WHITE);
	
	if(page < temp)
	{
		LCD_SetDialScale(LCD_CENTER_LINE, LCD_CENTER_ROW, 118, 6, 
						star_ang + 1 + (page*step) + page, 
						star_ang + 1 + (page*step) + page + len - 2, LCD_BLACK);
	}
	else
	{
		LCD_SetDialScale(LCD_CENTER_LINE, LCD_CENTER_ROW, 118, 6, 
						star_ang + 1 + (page*step) + temp, 
						star_ang + 1 + (page*step) + temp + len - 2, LCD_BLACK);
	}
}
/*
运动布局预览图
x_axis	:行
y_axis	:列
bckgrndcolor	:背景颜色
num		:第几部分(0到total-1)
total	:共几部分(1到4)
*/
void gui_sport_layout_preview(uint16_t x_axis, uint16_t y_axis, uint8_t bckgrndcolor, uint8_t num, uint8_t total)
{
	uint8_t selectedColor,color;
	uint8_t width = 48;
	
	//选中区域颜色
	selectedColor = LCD_LIGHTGRAY;
	
	if(bckgrndcolor == LCD_BLACK)
	{
		color = LCD_WHITE;
	}
	else
	{
		//背景色
		bckgrndcolor = LCD_WHITE;
		
		color = LCD_BLACK;
	}
	
	LCD_SetRectangle(x_axis, width, y_axis, width, bckgrndcolor, 0, 0, LCD_FILL_ENABLE);
	
	if(total == 1)
	{
		LCD_SetRectangle(x_axis, width, y_axis, width, selectedColor, 0, 0, LCD_FILL_ENABLE);
	}
	else if(total == 2)
	{
		if(total == 0)
		{
			LCD_SetRectangle(x_axis, width/2, y_axis, width, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		else
		{
			LCD_SetRectangle(x_axis + (width/2), width/2, y_axis, width, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		
		LCD_SetRectangle(x_axis + (width/2), 1, y_axis, width, color, 0, 0, LCD_FILL_ENABLE);
	}
	else if(total == 3)
	{
		if(num == 0)
		{
			LCD_SetRectangle(x_axis, width/3, y_axis, width, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		else if(num == 1)
		{
			LCD_SetRectangle(x_axis + (width/3), width - (width/3*2), y_axis, width, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		else
		{
			LCD_SetRectangle(x_axis + width - (width/3), width/3, y_axis, width, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		
		LCD_SetRectangle(x_axis + (width/3), 1, y_axis, width, color, 0, 0, LCD_FILL_ENABLE);
		LCD_SetRectangle(x_axis + width - (width/3), 1, y_axis, width, color, 0, 0, LCD_FILL_ENABLE);
	}
	else
	{
		if(num == 0)
		{
			LCD_SetRectangle(x_axis, width/3, y_axis, width, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		else if(num == 1)
		{
			LCD_SetRectangle(x_axis + (width/3), width - (width/3*2), y_axis, width/2, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		else if(num == 2)
		{
			LCD_SetRectangle(x_axis + (width/3), width - (width/3*2), y_axis + (width/2), width/2, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		else
		{
			LCD_SetRectangle(x_axis + width - (width/3), width/3, y_axis, width, selectedColor, 0, 0, LCD_FILL_ENABLE);
		}
		
		LCD_SetRectangle(x_axis + (width/3), 1, y_axis, width, color, 0, 0, LCD_FILL_ENABLE);
		LCD_SetRectangle(x_axis + width - (width/3), 1, y_axis, width, color, 0, 0, LCD_FILL_ENABLE);
		LCD_SetRectangle(x_axis + (width/3), width - (width/3*2), y_axis + (width/2), 1, color, 0, 0, LCD_FILL_ENABLE);
	}

	uint16_t r;
	r = sqrt(width*width*5/4);
	LCD_SetCircle(x_axis + (width/2), y_axis + (width/2), r, bckgrndcolor, r - (width/2), LCD_FILL_DISABLE);
	LCD_SetCircle(x_axis + (width/2), y_axis + (width/2), width/2 + 1, color, 2, LCD_FILL_DISABLE);
}

