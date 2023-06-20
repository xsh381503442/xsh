#include "gui_sport_config.h"
#include "gui_sport.h"

#include "time_progress.h"
#include "timer_app.h"
#include "bsp_timer.h"

#include "drv_lcd.h"
#include "drv_key.h"

#include "task_sport.h"
#include "task_gps.h"
#include "task_step.h"
#include "time_notify.h"
#include "task_hrt.h"

#include "gui.h"
#include "gui_sport_cycling.h"
#include "gui_sport_walk.h"
#include "gui_sport_marathon.h"
#include "gui_sport_crosscountry_hike.h"
#include "gui_sport_indoorrun.h"
#include "gui_run.h"
#include "gui_swimming.h"
#include "gui_climbing.h"
#include "gui_crosscountry.h"
#include "gui_heartrate.h"

#include "com_data.h"

#include "font_config.h"
#include "com_sport.h"
#include "drv_lis3mdl.h"
#include "timer_traceback.h"
#include "drv_battery.h"

#include "gui_tool_gps.h"
#include "gui_home.h"
#include "gui_trainplan.h"
#include "algo_hdr.h"
#include "com_sport.h"

#include "algo_sport.h"

#include "Ohrdriver.h"

#include "algo_trackoptimizing.h"

#if DEBUG_ENABLED == 1 && GUI_SPORT_LOG_ENABLED == 1
	#define GUI_SPORT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SPORT_WRITESTRING(...)
	#define GUI_SPORT_PRINTF(...)		        
#endif

#define Max_Sport_Nums 6

extern SetValueStr SetValue;
extern am_hal_rtc_time_t RTC_time;
extern uint16_t m_gui_sport_swim_laps;
#ifdef COD 
extern cod_ble_sport_cast cod_user_sport_cast;
#endif
extern float sport_alt;
extern int16_t sport_altitude;
uint8_t gui_sport_index = 0;
uint8_t gui_sport_type = 0;//
TraceBackStr TRACE_BACK_DATA;

//心率恢复率相关参数
uint8_t sport_end_flag = 0;  //是否按运动结束选项标志
uint8_t sport_end_hrvalue = 0;   //运动结束时心率值
uint8_t sport_min_hrvalue = 0;   //运动一分钟后心率值
uint32_t sport_end_time = 0;    //运动结束计时

uint8_t hdr_color_value[6] = {LCD_GRAY,LCD_SPRINGGREEN,LCD_MEDIUMBLUE,LCD_FORESTGREEN,LCD_BROWN,LCD_MAROON};

//volatile ScreenState_t Sport_Index;	//监测界面子索引

//uint32_t Custom_Sport = 0x3F;//自定义工具选择，后续放参数设置

static bool cancel_index;//放弃选项索引(否,是)

static uint8_t sport_feel_index; //运动感受选项索引

static uint8_t Sport_Nums;//当前界面总数

static uint8_t Curr_Sport_Num = 0xFF;//当前界面序号

static uint8_t m_sport_ready_index = 0;


static ScreenState_t Record_Sport_Num ;//存储上一次界面序号

volatile ScreenState_t Pause_Save_Index;	//暂停界面保存，用于返回不同了类型暂停界面时的索引

volatile ScreenState_t Save_Detail_Index;	//详情界面保存，用于返回不同了类型详情界面时的索引

extern uint8_t drv_getHdrValue(void);

ScreenState_t Select_Sport_Index[Max_Sport_Nums];//当前使用的界面数组
   
static uint8_t gps_flicker;		//gps定位图标闪烁 0:不闪烁；1:循环闪烁
uint8_t g_sport_status;			//运动状态
extern GPSMode m_GPS_data; 	   //GPS数据
extern uint8_t gps_located;

uint32_t progress_pre_timer=0;		

#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
static bool m_is_sport_event_cloud_valid = false;//越野跑、徒步越野是否有云迹导航轨迹

void set_is_sport_event_cloud_valid(bool status)
{
	m_is_sport_event_cloud_valid = status;
}
bool get_is_sport_event_cloud_valid(void)
{
	return m_is_sport_event_cloud_valid;
}
#endif

//#define	SELECT_SPORTS_SUM						7		//有7项运动
/*const ScreenState_t m_select_sport_buf[SELECT_SPORTS_SUM] = {
			DISPLAY_SCREEN_CROSSCOUNTRY_HIKE, DISPLAY_SCREEN_CROSSCOUNTRY,
			DISPLAY_SCREEN_RUN,DISPLAY_SCREEN_CYCLING,
			DISPLAY_SCREEN_CLIMBING,DISPLAY_SCREEN_SWIMMING,
			DISPLAY_SCREEN_TRIATHLON};*/
#define	SELECT_SPORTS_SUM						6		//有6项运动
const ScreenState_t m_select_sport_buf[SELECT_SPORTS_SUM] = {
			DISPLAY_SCREEN_RUN,DISPLAY_SCREEN_CYCLING,DISPLAY_SCREEN_CLIMBING,
			DISPLAY_SCREEN_SWIMMING,DISPLAY_SCREEN_CROSSCOUNTRY_HIKE, DISPLAY_SCREEN_CROSSCOUNTRY,
			};


extern _drv_bat_event g_bat_evt;
#ifdef COD 
extern uint32_t location_timer_count;

#endif

//#if defined WATCH_SIM_SPORT

uint8_t m_sport_data_display_buf[SPORT_DATA_DISPLAY_SUM]={SPORT_DATA_DISPALY_TOTAL_DISTANCE,SPORT_DATA_DISPALY_REALTIME_PACE,SPORT_DATA_DISPALY_TOTAL_STEP,SPORT_DATA_DISPALY_TOTAL_TIME};	//运动数据显示项
static uint8_t m_sport_data_display_index = 0;	//  栏索引，当前选中栏

uint8_t m_sport_data_display_set_buf[SPORT_DATA_DISPLAY_SET_MAX]={0}; 	//运动数据显示设置项
uint8_t m_sport_data_display_set_sum = 0;								//运动数据显示设置当前总个数

uint8_t m_sport_data_display_set_index = 0;								//运动数据显示设置索引

uint8_t m_sport_remind_set_buf[SPORT_REMIND_SET_MAX]={0}; 	//运动提醒设置项
uint8_t m_sport_remind_set_sum = 0;									//运动提醒设置当前总个数
uint8_t m_sport_remind_set_index = 0;			//运动提醒设置索引

 uint8_t sport_cast_detail_index = 0; //暂停菜单界面索引

extern gps_data_distance gps_data_distance_sm;



/*心率提醒设置*/
#define GUI_SPORT_REMIND_SET_HEART_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_HEART_TENS 	                1	//十位
#define GUI_SPORT_REMIND_SET_HEART_ONES                		2	//个位
static uint8_t m_gui_sport_remind_set_heart_index = 0;
_remind_heartrate m_remind_heartrate;

/*配速提醒设置*/
#define GUI_SPORT_REMIND_SET_PACE_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_PACE_MIN 	                	1	//分钟
#define GUI_SPORT_REMIND_SET_PACE_SEC                		2	//秒
static uint8_t m_gui_sport_remind_set_pace_index = 0;
_remind_pace m_remind_pace;
		
/*距离提醒设置*/
#define GUI_SPORT_REMIND_SET_DISTANCE_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_DISTANCE_KM 	                	1	//千米
#define GUI_SPORT_REMIND_SET_DISTANCE_HM                		2	//百米
static uint8_t m_gui_sport_remind_set_distance_index = 0;
_remind_distance m_remind_distance;

/*自动计圈设置*/
#define GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_AUTOCIRCLE_KM 	                	1	//千米
#define GUI_SPORT_REMIND_SET_AUTOCIRCLE_HM                		2	//百米
static uint8_t m_gui_sport_remind_set_autocircle_index = 0;
_remind_distance m_remind_autocircle;



/*燃脂目标设置*/
#define GUI_SPORT_REMIND_SET_GOALKCAL_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_GOALKCAL_HUNDREDS	                1	//百位
#define GUI_SPORT_REMIND_SET_GOALKCAL_TENS                		2	//十位
#define GUI_SPORT_REMIND_SET_GOALKCAL_ONES						3	//个位
static uint8_t m_gui_sport_remind_set_goalkcal_index = 0;
_remind_goalkcal m_remind_goalkcal;


/*时间提醒设置*/
#define GUI_SPORT_REMIND_SET_TIME_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_TIME_TENS                		1	//十位
#define GUI_SPORT_REMIND_SET_TIME_ONES						2	//个位
static uint8_t m_gui_sport_remind_set_time_index = 0;
_remind_heartrate m_remind_time;


/*速度提醒设置*/
#define GUI_SPORT_REMIND_SET_SPEED_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_SPEED_TENS 	                	1	//十位
#define GUI_SPORT_REMIND_SET_SPEED_ONES                		2	//小数点一位
static uint8_t m_gui_sport_remind_set_speed_index = 0;
_remind_heartrate m_remind_speed;


/*泳池长度设置*/
#define GUI_SPORT_REMIND_SET_SWIMPOOL_TENS                		0	//十位
#define GUI_SPORT_REMIND_SET_SWIMPOOL_ONES						1	//个位
static uint8_t m_gui_sport_remind_set_swimpool_index = 0;
_remind_heartrate m_remind_swimpool;

/*计圈提醒设置*/
#define GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_GOALCIRCLE_TENS                	1	//十位
#define GUI_SPORT_REMIND_SET_GOALCIRCLE_ONES					2	//个位
static uint8_t m_gui_sport_remind_set_goalcircle_index = 0;
_remind_heartrate m_remind_goalcircle;

/*室内跑长度设置*/
#define GUI_SPORT_REMIND_SET_INDOORLENG_TENS                		0	//十位
#define GUI_SPORT_REMIND_SET_INDOORLENG_ONES						1	//个位
static uint8_t m_gui_sport_remind_set_indoorleng_index = 0;
_remind_heartrate m_remind_indoorleng;


/*高度提醒设置*/
#define GUI_SPORT_REMIND_SET_ALTITUDE_SWITCH                	0	//开关
#define GUI_SPORT_REMIND_SET_ALTITUDE_THOUS                		1	//千位
#define GUI_SPORT_REMIND_SET_ALTITUDE_HUND						2	//百位
static uint8_t m_gui_sport_remind_set_altitude_index = 0;
_remind_altitude m_remind_altitude;



//#endif

//暂时这样写
void gui_sport_feel_init(void)
{
	sport_feel_index = FeelAbitTired;
}


//获取当前界面
ScreenState_t Get_Curr_Sport_Index(void)
{
    if (Curr_Sport_Num == 0xFF)
    
{
#if defined WATCH_SIM_SPORT
		Set_Curr_Sport_Index(DISPLAY_SCREEN_RUN);
#else
        Set_Curr_Sport_Index(DISPLAY_SCREEN_CROSSCOUNTRY);
#endif

    }
 
      return Select_Sport_Index[Curr_Sport_Num];
}

//根据序号获取界面
ScreenState_t Get_Sport_Index(uint8_t Num)
{
    return Select_Sport_Index[Num];
}

//根据界面获取序号
uint8_t Get_Curr_Sport_Num(ScreenState_t ScreenSta)
{
    uint8_t i;
   
       for (i=0;i<Sport_Nums;i++)
       {
        if (Select_Sport_Index[i]==ScreenSta)
        {
       
            break;
         }
       
       }
      return i;

}

//获取自定义界面的个数
uint8_t Get_Sport_Nums(void)
{
    return Sport_Nums;
}

//设置当前选择界面
ScreenState_t Set_Curr_Sport_Index( ScreenState_t ScreenSta)
{
    uint8_t i;
   
       for (i=0;i<Sport_Nums;i++)
       {
        if (Select_Sport_Index[i]==ScreenSta)
        {
            Curr_Sport_Num = i;
            break;
         }
       
       }
       if (Curr_Sport_Num == 0xFF)
        {
           Curr_Sport_Num = 0;

       }
      return Select_Sport_Index[Curr_Sport_Num];
}

uint8_t is_custom_sport(ScreenState_t index)
{
	bool ret = 0;
	switch(index)
	{
		case DISPLAY_SCREEN_RUN:				//跑步
			if (((SetValue.U_CustomSport.val) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;

			case DISPLAY_SCREEN_CYCLING:			//骑行
			if (((SetValue.U_CustomSport.val>>1) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;

			case DISPLAY_SCREEN_CLIMBING:			//登山
			if (((SetValue.U_CustomSport.val>>2) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
			case DISPLAY_SCREEN_SWIMMING:				//室内游泳
			if (((SetValue.U_CustomSport.val>>3) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:	//徒步越野
			if (((SetValue.U_CustomSport.val>>4) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
			case DISPLAY_SCREEN_CROSSCOUNTRY:	//越野跑
			if (((SetValue.U_CustomSport.val>>5) & 0x01) == 0x01) 
			{
				ret =1;
			}
			break;
		
		 default:
			break;
	}

	return ret;
	
}

//自定义运动界面选择
void Set_Select_Sport_Index(void)
{
    uint8_t i;
    uint8_t j = 0;
	
	memset(Select_Sport_Index,0,sizeof(Select_Sport_Index));

	for (i=0; i<SELECT_SPORTS_SUM; i++)
	{
		if (is_custom_sport(m_select_sport_buf[i]) == 1) 
     	{     
      	 	Select_Sport_Index[j]= m_select_sport_buf[i];
       		j++;
     	 }
  
    }
    Sport_Nums = j;	

}

uint8_t SportIndexRecAdjust(void)
{
	int i;
	
	for(i = 0; i < Sport_Nums; i++)
	{
		if(sports_menu_index_record() == Select_Sport_Index[i])
		{
			return 1;
		}
	}
	
	return 0;
}

//创建运动选择界面
void CreateSportDisplay(void)
{
    DISPLAY_MSG  msg = {0};
    ScreenState = DISPLAY_SCREEN_SPORT;	
    msg.cmd = MSG_DISPLAY_SCREEN;
   // msg.value = Get_Curr_Sport_Index();
    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
}

//稍后继续长按up键返回登山等待界面
void SportContinueLaterDisplay(void)
{
	DISPLAY_MSG  msg = {0};
	switch(ContinueLaterDataSave.SportType)
	{
		case DISPLAY_SCREEN_RUN: 
#if defined WATCH_SIM_SPORT
			ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
			
			//结束稍后继续
			sport_continue_later_end();
#else
			ScreenState = DISPLAY_SCREEN_RUN_PAUSE;
#if defined WATCH_COM_SPORT
			sport_continue_later_end();
#endif

#endif
			break;
		case DISPLAY_SCREEN_MARATHON:
#if defined WATCH_SIM_SPORT		
			sport_continue_later_end();
			ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;
#elif defined WATCH_COM_SPORT
			sport_continue_later_end();
			ScreenState = DISPLAY_SCREEN_MARATHON_PAUSE;

#endif	
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY:

			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
			//结束稍后继续
#if defined WATCH_COM_SPORT
			sport_continue_later_end();
#endif
			break;
		case DISPLAY_SCREEN_INDOORRUN:
#if defined WATCH_SIM_SPORT		
			sport_continue_later_end();
			ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
#elif defined WATCH_COM_SPORT
			sport_continue_later_end();
			ScreenState = DISPLAY_SCREEN_INDOORRUN_PAUSE;
#endif				
			break;
		case DISPLAY_SCREEN_WALK:
#if defined WATCH_SIM_SPORT		
			sport_continue_later_end();
			ScreenState = DISPLAY_SCREEN_WALK_PAUSE;
#elif defined WATCH_COM_SPORT
			sport_continue_later_end();
			ScreenState = DISPLAY_SCREEN_WALK_PAUSE;
#endif			
			break;
		case DISPLAY_SCREEN_CLIMBING:
#if defined WATCH_COM_SPORT
			ScreenState = DISPLAY_SCREEN_CLIMBING_PAUSE;				
#else

			if(TRACE_BACK_DATA.is_traceback_flag )
			{
				ScreenState = DISPLAY_SCREEN_CLIMBING_TRACKBACK_PAUSE;
			}
			else
			{
				ScreenState = DISPLAY_SCREEN_CLIMBING_PAUSE;
			}
#endif		
			//进入稍后继续，就需要开启传感器，暂时添加在这个
			sport_continue_later_end();
			break;
		case DISPLAY_SCREEN_CYCLING:
			ScreenState = DISPLAY_SCREEN_CYCLING_PAUSE;
			
			//结束稍后继续
			sport_continue_later_end();
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE;
		
			//结束稍后继续
			sport_continue_later_end();
			break;
		case DISPLAY_SCREEN_SWIMMING:
			
			break;
		case DISPLAY_SCREEN_TRIATHLON:
			break;
		default:
			break;
	}
	msg.cmd = MSG_DISPLAY_SCREEN;
	msg.value = Get_Curr_Sport_Index();
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
}

//运动图标中心位置
const Sport_index_axis sport_center_axis[] = {
//decagonal axis 十等分坐标系
{47,67},	//-36 Degree
{92,35},	//-72 Degree
{148,35},	//-108 Degree	
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
{148,206},//108 Degree
{92,206},	//72 Degree
{47,173},	//36 Degree


//dodecagonal axis 十二等分坐标系
//{120, 30},  //-90 Degree
//{165, 42},	//-120 Degree
//{198, 75},	//-150 Degree
//{210,120},	//-180 Degree
//{198,165},	//150 Degree
//{165,198},	//120 Degree
//{120,210},	//90 Degree
//{72, 204},	//60 Degree	//Modify from 75,198
//{40, 170},	//30 Degree	//Modify from 42,165
//{30, 120},	//0 Degree
//{40, 70},		//-30 Degree //Modify from 30,120
//{72, 36},		//-60 Degree //Modify from 75,42
//{120, 30},	//-90 Degree
//{165, 42},	//-120 Degree
//{198, 75},	//-150 Degree
//{210,120},	//-180 Degree
//{198,165},	//150 Degree
//{165,198},	//120 Degree
//{120,210},	//90 Degree

};
/*
0.7071


*/
const Sport_index_axis sport_img_axis[] = 
{
{120 + 0.7071*80,120- 0.7071*80},
{120,40},
{120 - 0.7071*80,120 - 0.7071*80},

{120 - 0.7071*80,120 + 0.7071*80},
{120,120+80},
{120 + 0.7071*80,120 + 0.7071*80}
};
const sport_angle_range angle_range[] = 
{
  	{180+45/2,225+45/2},
  	{225+45/2,270+45/2},
  	{270+45/2,315+45/2},
  	{45/2,45+45/2},
  	{45+45/2,90+45/2},
  	{90+45/2,135+45/2}


};

//显示运动选择界面
void gui_sport_select_paint(ScreenState_t state)
{
	SetWord_t word = {0};
	uint8_t index,i,screen_index;
	ScreenState_t imgnum_s;
    uint8_t imgnum;
    uint8_t sport_nums;
    uint8_t curr_sport_num;
	
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
    LCD_SetArc(120,120,120,6,180+45/2,315+45/2,LCD_CYAN);
   	LCD_SetArc(120,120,120,6,45/2,135+45/2,LCD_CYAN);

	
   	LCD_SetArc(120,120,40,2,270+30,90-30,LCD_CYAN);
    
   	LCD_SetArc(120,120,40,2,90+30,270-30,LCD_CYAN);
	if((state < DISPLAY_SCREEN_RUN) || (state > DISPLAY_SCREEN_TRIATHLON))
	{
		//防止索引错误
		state =	Get_Curr_Sport_Index();
	}
		index = Get_Curr_Sport_Num(state);
    sport_nums = Get_Sport_Nums();
   	for(i = 0; i < SELECT_SPORTS_SUM; i++)
		{
		 if(Select_Sport_Index[index] == m_select_sport_buf[i])
			{
		        screen_index = i;
				break;
			}
		}
	

   for (curr_sport_num = 0; curr_sport_num <sport_nums; curr_sport_num ++)
   {
	   imgnum_s = Get_Sport_Index(curr_sport_num);

       for(i = 0; i < SELECT_SPORTS_SUM; i++)
	{
		if(imgnum_s == m_select_sport_buf[i])
		{
           imgnum = i;
			break;
		}
	}
	   
      word.x_axis = sport_img_axis[curr_sport_num].x_axis - img_custom_sports[imgnum].height/2;
	  word.y_axis = sport_img_axis[curr_sport_num].y_axis - img_custom_sports[imgnum].width/2;
      LCD_SetPicture(word.x_axis,word.y_axis,LCD_CYAN,LCD_NONE,&img_custom_sports[imgnum]);
    
    }
   word.x_axis = sport_img_axis[index].x_axis - img_custom_sports[screen_index].height/2;
   word.y_axis = sport_img_axis[index].y_axis - img_custom_sports[screen_index].width/2;
   LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&img_custom_sports[screen_index]);
  
  LCD_SetArc(120,120,120,6,angle_range[index].start_angle,angle_range[index].end_angle,LCD_RED);
	//显示提示
	word.x_axis = 108;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_BLACK;
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	word.kerning = 0;
  LCD_SetString((char *)SportStrs[SetValue.Language][screen_index],&word);
	
}

//运动显示项获取保存的参数值
void gui_sport_data_init(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:	
			    m_run_data_display_buf[0] = SetValue.RunData1;
				m_run_data_display_buf[1] = SetValue.RunData2;
			    m_run_data_display_buf[2] = SetValue.RunData3;
     		    m_run_data_display_buf[3] = SetValue.RunData4;
				
				break;
		/*	case DISPLAY_SCREEN_WALK:
				m_walk_data_display_buf[0] =SetValue.WlakData1;
				m_walk_data_display_buf[1] =SetValue.WlakData2;
				m_walk_data_display_buf[2] =SetValue.WlakData3;	
				break;
			case DISPLAY_SCREEN_MARATHON:
				m_marathon_data_display_buf[0] = SetValue.MarathonData1;
				m_marathon_data_display_buf[1] = SetValue.MarathonData2;
				m_marathon_data_display_buf[2] = SetValue.MarathonData3;
				break;*/
			case DISPLAY_SCREEN_CYCLING:
				m_cycling_data_display_buf[0] = SetValue.CyclingData1;
				m_cycling_data_display_buf[1] = SetValue.CyclingData2;
				m_cycling_data_display_buf[2] = SetValue.CyclingData3;
				m_cycling_data_display_buf[3] = SetValue.CyclingData4;
				break;
			case DISPLAY_SCREEN_SWIMMING:
				 m_swim_data_display_buf[0] = SetValue.SwimRunData1;
				 m_swim_data_display_buf[1] = SetValue.SwimRunData2;
				 m_swim_data_display_buf[2] = SetValue.SwimRunData3;
                 m_swim_data_display_buf[3] = SetValue.SwimRunData4;
				 

				 
				break;


		 case DISPLAY_SCREEN_CLIMBING:
				 m_climbing_data_display_buf[0] = SetValue.ClimbingData1;
				 m_climbing_data_display_buf[1] = SetValue.ClimbingData2;
				 m_climbing_data_display_buf[2] = SetValue.ClimbingData3;
                 m_climbing_data_display_buf[3] = SetValue.ClimbingData4;
				  
				break;


			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
				 m_hike_data_display_buf[0] = SetValue.HikeData1;
				 m_hike_data_display_buf[1] = SetValue.HikeData2;
				 m_hike_data_display_buf[2] = SetValue.HikeData3;
                 m_hike_data_display_buf[3] = SetValue.HikeData4;
				 

				 
				break;


			case DISPLAY_SCREEN_CROSSCOUNTRY:
				 m_crosscountry_data_display_buf[0] = SetValue.CrosscountryData1;
				 m_crosscountry_data_display_buf[1] = SetValue.CrosscountryData2;
				 m_crosscountry_data_display_buf[2] = SetValue.CrosscountryData3;
                 m_crosscountry_data_display_buf[3] = SetValue.CrosscountryData4;
				 

				 
				break;
			/*case DISPLAY_SCREEN_INDOORRUN:
				m_indoorrun_data_display_buf[0] = SetValue.IndoorRunData1;
				m_indoorrun_data_display_buf[1] = SetValue.IndoorRunData2;
				m_indoorrun_data_display_buf[2] = SetValue.IndoorRunData3;
				
				break;*/
			default:
				break;
		}
}
#if defined WATCH_SPORT_EVENT_SCHEDULE
uint32_t g_tool_or_sport_goto_screen = DISPLAY_SCREEN_HOME;//主界面代表进入前初始状态,>0显示从哪个界面进入的云迹导航运动
/*
初始化运动 赛事等参数
type:活动类型
m_status 1代表从运动进入
*/
void init_sport_event_data_str(uint8_t type,uint8_t m_status)
{
	GUI_SPORT_PRINTF("[gui_sport]:init_sport_event_data_str type=0x%x,m_status=%d\r\n",type,m_status);

	if(g_tool_or_sport_goto_screen == DISPLAY_SCREEN_GPS_DETAIL)
	{//从工具GPS进入的导航运动 无赛事
		memset(&g_sport_event_status,0,sizeof(SportEventStatusStr));
		if(type == ACT_CROSSCOUNTRY)
		{
			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
			gui_sport_crosscountry_init();
		}
		else if(type == ACT_HIKING)
		{
			ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
			gui_sport_crosscountry_hike_init();
		}
	}
	else
	{
		if(type == ACT_CROSSCOUNTRY)
		{
			memset(&g_sport_event_status,0,sizeof(SportEventStatusStr));
			g_sport_event_status = get_sport_event_schedule_number(ACT_CROSSCOUNTRY);//获取当前是否有越野跑赛事
			GUI_SPORT_PRINTF("[gui_sport]:1>>g_sport_event_status-ACT_CROSSCOUNTRY->sport_type=%d,status=%d,nums=%d,total_num=%d,event_id=%d\r\n"
				,g_sport_event_status.sport_type,g_sport_event_status.status,g_sport_event_status.nums
				,g_sport_event_status.total_nums,g_sport_event_status.event_id);
			GUI_SPORT_PRINTF("[gui_sport]:2>>g_sport_event_status.sport_event.U_EventSport.bits=%d-%02d-%02d %d:%02d:%02d-endsec:%02d,status=%d,num=%d,\n"
			,g_sport_event_status.event_str.U_EventSport.bits.Year,g_sport_event_status.event_str.U_EventSport.bits.Month
				,g_sport_event_status.event_str.U_EventSport.bits.Day
				,g_sport_event_status.event_str.U_EventSport.bits.Hour,g_sport_event_status.event_str.U_EventSport.bits.Minute
				,g_sport_event_status.event_str.U_EventSport.bits.Second_Start
				,g_sport_event_status.event_str.U_EventSport.bits.Second_End,g_sport_event_status.event_str.U_EventSport.bits.Event_Status
				,g_sport_event_status.event_str.U_EventSport.bits.Shedule_Nums);
			if(is_sport_event_schedule_valid(g_sport_event_status) == true)
			{//赛事未结束 显示是否进行已知赛事
				ScreenState = DISPLAY_SCREEN_SPORT_EVENT_RESTART_SELECT;
				g_sport_event_status.sport_type = ACT_CROSSCOUNTRY;
				m_gui_sport_event_restart_select_key_index = 0;
			}
			else
			{//正常运动显示
				memset(&g_sport_event_status,0,sizeof(SportEventStatusStr));
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
				gui_sport_crosscountry_init();
			}
		}
		else if(type == ACT_HIKING)
		{
			memset(&g_sport_event_status,0,sizeof(SportEventStatusStr));
			g_sport_event_status = get_sport_event_schedule_number(ACT_HIKING);//获取当前是否有徒步越野赛事
			GUI_SPORT_PRINTF("[gui_sport]:g_sport_event_status-ACT_HIKING->sport_type=%d,status=%d,nums=%d,total_num=%d,event_id=%d\r\n"
				,g_sport_event_status.sport_type,g_sport_event_status.status,g_sport_event_status.nums
				,g_sport_event_status.total_nums,g_sport_event_status.event_id);
			GUI_SPORT_PRINTF("[gui_sport]:2>>g_sport_event_status.sport_event.U_EventSport.bits=%d-%02d-%02d %d:%02d:%02d-endsec:%02d,status=%d,num=%d,\n"
				,g_sport_event_status.event_str.U_EventSport.bits.Year,g_sport_event_status.event_str.U_EventSport.bits.Month
				,g_sport_event_status.event_str.U_EventSport.bits.Day
				,g_sport_event_status.event_str.U_EventSport.bits.Hour,g_sport_event_status.event_str.U_EventSport.bits.Minute
				,g_sport_event_status.event_str.U_EventSport.bits.Second_Start
				,g_sport_event_status.event_str.U_EventSport.bits.Second_End,g_sport_event_status.event_str.U_EventSport.bits.Event_Status
				,g_sport_event_status.event_str.U_EventSport.bits.Shedule_Nums);
			if(is_sport_event_schedule_valid(g_sport_event_status) == true)
			{//赛事未结束 显示是否进行已知赛事
				ScreenState = DISPLAY_SCREEN_SPORT_EVENT_RESTART_SELECT;
				g_sport_event_status.sport_type = ACT_HIKING;
				m_gui_sport_event_restart_select_key_index = 0;
			}
			else
			{//正常运动显示
				memset(&g_sport_event_status,0,sizeof(SportEventStatusStr));
				ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				gui_sport_crosscountry_hike_init();
			}
		}

		GUI_SPORT_PRINTF("[gui_sport]:-->DISPLAY_SCREEN_HOME=%d,%d,%d--***m_status=%d,type=0x%x,g_tool_or_sport_goto_screen=%d"
					,DISPLAY_SCREEN_HOME,DISPLAY_SCREEN_CROSSCOUNTRY,DISPLAY_SCREEN_CROSSCOUNTRY_HIKE
					,m_status,type,g_tool_or_sport_goto_screen);

		if(m_status == 1)
		{
			bool status = is_has_sport_new_cloud_navigation(type);
			//该赛事运动是否有云迹导航轨迹
			set_is_sport_event_cloud_valid(status);
		}
		else
		{
			set_is_sport_event_cloud_valid(false);
		}
	}
	
}
#endif
void gui_sport_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
    ScreenState_t   CurrSprtindex;
	
		switch( Key_Value )
		{
			case KEY_OK:         
      			//进入相应的运动界面
            CurrSprtindex = Get_Curr_Sport_Index();
						if(CurrSprtindex != DISPLAY_SCREEN_RUN)
						{
							ReadySport(CurrSprtindex);
						}
            switch(CurrSprtindex)
            {
                case DISPLAY_SCREEN_RUN:
				#ifndef COD 
				if(get_trainplan_valid_flag() && (Get_IsCompleteTodayTrainPlan() != 1)
					 && Get_SomeDayofTrainPlan() > 0 && Get_TrainPlanBeforeDays() == 0
						 && Is_TodayTimeTrainPlan() == true)
					{//训练计划开启且今天训练计划未完成且需要选择训练计划或跑步时 暂不ready
						ScreenState = DISPLAY_SCREEN_TRAINPLAN_RUN_SELECT;
					}
				else
				#endif
				  {
					Set_IsTrainPlanOnGoing(false);
					ReadySport(CurrSprtindex);
  					
  					ScreenState = DISPLAY_SCREEN_SPORT_READY;
  					ScreenSportSave = DISPLAY_SCREEN_RUN;
  				
  
				 }
                    break;
                case DISPLAY_SCREEN_MARATHON:
#if defined WATCH_SIM_SPORT
					ScreenState = DISPLAY_SCREEN_SPORT_READY;
					ScreenSportSave = DISPLAY_SCREEN_MARATHON;
#else
					//马拉松
					ScreenState = DISPLAY_SCREEN_MARATHON;
					gui_sport_marathon_init();
#endif
                    break;
//Jason_V1.10:Add-in CrossCountryRunning Function:								
                case DISPLAY_SCREEN_CROSSCOUNTRY:

							 ScreenState = DISPLAY_SCREEN_SPORT_READY;
					         ScreenSportSave = DISPLAY_SCREEN_CROSSCOUNTRY;
									
      					//ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
      					//gui_sport_crosscountry_init();
									
                    break;
//Jason_V1.00:Addin Swimming Function:	
                case DISPLAY_SCREEN_SWIMMING:
					
					ScreenState = DISPLAY_SCREEN_SPORT_READY;
					ScreenSportSave = DISPLAY_SCREEN_SWIMMING;
					task_hrt_close();//关闭心率
					
                    break;
//Jason's code end in here
                case DISPLAY_SCREEN_INDOORRUN:
#if defined WATCH_SIM_SPORT
					ScreenState = DISPLAY_SCREEN_SPORT_READY;
					ScreenSportSave = DISPLAY_SCREEN_INDOORRUN;
#elif defined WATCH_COM_SPORT
					ScreenState = DISPLAY_SCREEN_INDOORRUN;
					gui_sport_indoorrun_init();
#else
					ScreenState = DISPLAY_SCREEN_INDOORRUN;
#endif
                    break;
                case DISPLAY_SCREEN_WALK:
#if defined WATCH_SIM_SPORT 
					ScreenState = DISPLAY_SCREEN_SPORT_READY;
					ScreenSportSave = DISPLAY_SCREEN_WALK;
#else
					//健走
					ScreenState = DISPLAY_SCREEN_WALK;
					gui_sport_walk_init();
#endif
                    break;
                case DISPLAY_SCREEN_CLIMBING://登山
                     ScreenState = DISPLAY_SCREEN_SPORT_READY;
					ScreenSportSave = DISPLAY_SCREEN_CLIMBING;

					
                    break;
                case DISPLAY_SCREEN_CYCLING://骑行界面
					
					
					
					ScreenState = DISPLAY_SCREEN_SPORT_READY;
					ScreenSportSave = DISPLAY_SCREEN_CYCLING;
					
				

                    break;

				case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
										//徒步越野
                              ScreenState = DISPLAY_SCREEN_SPORT_READY;
					         ScreenSportSave = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
									
										//gui_sport_crosscountry_hike_init();
                    break;
               /* case DISPLAY_SCREEN_TRIATHLON:
					//铁人三项界面
					ScreenState = DISPLAY_SCREEN_TRIATHLON;
                    break;*/
                default:
			        break;
            }

				//准备状态
				g_sport_status = SPORT_STATUS_READY;
				memset(&TRACE_BACK_DATA, 0, sizeof(TraceBackStr));
			gui_sport_data_init();
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
             progress_pre_timer = location_timer_count;
			break;
		case KEY_UP:
			if( Curr_Sport_Num == Sport_Nums -1 )
			{
				Curr_Sport_Num = 0;
				sports_menu_index_record();
			}
			else
			{
				Curr_Sport_Num++;
				sports_menu_index_record();
			}
			
			CreateSportDisplay();
			
			break;
		case KEY_DOWN:				
			if( Curr_Sport_Num == 0 )
			{
				Curr_Sport_Num = Sport_Nums -1;
				sports_menu_index_record();
			}
			else
			{
				Curr_Sport_Num--;
				sports_menu_index_record();
			}

			CreateSportDisplay();
			
			break;
		case KEY_BACK:
			//退出运动选择关闭gps,心率
			CloseGPSTask();	
			task_hrt_close();
			
			progress_pre_timer = 0;
			location_timer_count = 0;
			ScreenState = DISPLAY_SCREEN_HOME;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			timer_timeout_back_stop(false);

			break;
		default:
			break;
	}
}

//圆点界面索引。 index:当前索引号，  nums:索引个数   type:不同位置
void gui_sport_index_circle_paint(uint8_t index, uint8_t nums, uint8_t type)
{
	uint8_t radius = 4;
	uint8_t i;
	SetWord_t word = {0};
	
	switch(type)
	{
		case 0:
			word.x_axis = 10;
			break;
		case 1:
			word.x_axis = 16;
			break;
		default:
			return;
	}
	word.y_axis =(LCD_LINE_CNT_MAX - 2*radius *(2 * nums - 1))/2;
	
	for (i = 0; i < nums; i++)
	{
		if (index == i)
		{
			LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_selected_8X8);
		}
		else
		{
			LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_unselected_8X8);
		}
		word.y_axis += (radius * 4);
	}
}

void gui_sport_index_circle_vert_paint(uint8_t index, uint8_t nums)
{

   uint16_t x,y;
	uint8_t i;
	SetWord_t word = {0};
		if((index<nums-1)&&(nums>=3))
			{

            
			for (i = 0; i < nums; i++)
				{
						 
				 get_circle_point(120, 120, 235-i*10, 108, &x,&y); 
					word.x_axis= y;
					
					word.y_axis= x;
					
					if (index == i)
					{
						LCD_SetPicture(word.x_axis,word.y_axis,LCD_BLACK,LCD_NONE,&Img_selected_8X8);
					}
					else
					{
						LCD_SetPicture(word.x_axis,word.y_axis,LCD_BLACK,LCD_NONE,&Img_unselected_8X8);
					}
				}

		   }
		else
			{
              
			  for (i = 0; i < nums; i++)
				{
									   
				 get_circle_point(120, 120, 235-i*10, 100, &x,&y); 
				  word.x_axis= y;
				  word.y_axis= x;
				 if (index == i)
					 {
						 LCD_SetPicture(word.x_axis,word.y_axis,LCD_BLACK,LCD_NONE,&Img_selected_8X8);
					 }
				 else
					  {
						 LCD_SetPicture(word.x_axis,word.y_axis,LCD_BLACK,LCD_NONE,&Img_unselected_8X8);
					  }
				  }


		     }
	
	


}



//航迹打点提示语
void gui_sport_dot_draw_prompt(void)
{
	SetWord_t word = {0};
	word.x_axis = 40;
    word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	if (dottrack_out_time > 0)
	{
		LCD_SetString((char *)DotStrs[SetValue.Language][0],&word);
	}
	else if (dottrack_nogps_time > 0)
	{
		LCD_SetString((char *)DotStrs[SetValue.Language][1],&word);
	}


}
//打点标点
void gui_sport_dottrack_nums(uint32_t dot_nums)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint8_t len;
	word.x_axis = LCD_CENTER_LINE - Img_Dot_24X30.height/2;
	word.y_axis = LCD_CENTER_LINE - Img_Dot_24X30.width/2;
	
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_Dot_24X30);
	
	memset(str,0,10);
	sprintf(str,"%d",dot_nums);
	len = strlen(str);

	word.x_axis = LCD_CENTER_LINE - 10;
    word.y_axis = LCD_CENTER_LINE - 3*len;
    word.size = LCD_FONT_16_SIZE;
    word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;

	LCD_SetNumber(str,&word); 		
	
}

//打点震动声音提示
void gui_sport_dot_prompt(void)
{
	//震动声音不跟随系统，只受勿扰影响
	timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
	timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);

}
///保存航迹打点数据操作
void sport_dottrack(void)
{
	if (GetGpsStatus())
	{	
		dot_track_nums++;
		if (dot_track_nums < 20)
		{	
			dottrack_draw_time = 5;
			Store_DotTrack(store_gps_nums);
		}
		else
		{
			dottrack_out_time = 5;
		}		
	}
	else
	{
		dottrack_nogps_time = 5;
	}
	gui_sport_dot_prompt();

}


//导航箭头
void gui_sport_arrows(uint16_t angle,uint16_t y,uint16_t x, uint8_t color)
{
	uint16_t x1,y1,x2,y2,x3,y3;
	uint16_t tmp;
	uint8_t r1 = 22;	//中心点到顶点距离
	uint8_t r2 = 16;	//中心点到底边两点的距离
	
	//获取三个坐标点
	tmp = angle % 360;
	get_circle_point(y, x, tmp, r1, &x1, &y1);
	
	tmp = (angle + 130) % 360;
	get_circle_point(y, x, tmp, r2, &x2, &y2);
	
	tmp = (angle + 230) % 360;
	get_circle_point(y, x, tmp, r2, &x3, &y3);
	
	LCD_SetTriangle(y1, x1, y2, x2, y, x, color, 0, LCD_FILL_ENABLE);
	LCD_SetTriangle(y1, x1, y, x, y3, x3, color, 0, LCD_FILL_ENABLE);
}

//指北针
void gui_sport_compass(uint16_t angle)
{
		SetWord_t word = {0};
		uint16_t a,b;
		uint16_t tmp;
		uint8_t radius = 114;	//半径
		uint8_t thickness = 4;	//厚度
		
		//圆环
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, radius, thickness, 0, 360, LCD_MAROON);
		
		//NWSE
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_MAROON;
		word.bckgrndcolor = LCD_NONE;
		for(uint8_t i = 0; i < 4; i++)
		{
			tmp = 360 - angle;
			get_circle_point(LCD_CENTER_LINE, LCD_CENTER_ROW, tmp, radius - 2, &a, &b);
			tmp = (tmp + 354) % 360;
			LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, radius, thickness, tmp, tmp + 12, LCD_WHITE);
			word.x_axis = b - 3;
			word.y_axis = a - 4;
			word.forecolor = LCD_BLACK;
			//LCD_SetString((char *)CompassStrs[L_ENGLISH][i],&word);
			LCD_SetPicture(word.x_axis,word.y_axis,LCD_BLACK,LCD_NONE,&img_compass_nwse[i]);
			angle = (angle + 90) % 360;
		}
}


//时间+电量
void gui_sport_realtime_battery(uint8_t type)
{
	uint8_t line;
	
	switch(type)
	{
		case 0:
			line = 215;
			break;
		case 1:
			line = 210;
			break;
		case 2:
			line = 220;
			break;
		default:
			return;
	}
	
	gui_bottom_time_battery(line);
}

//实时时间，电量
void gui_realtime_battery_paint(uint8_t percent)
{
#if 1
	SetWord_t word = {0};
	char str[20] = {0};
	
	if(percent > 100)
	{
		percent = 100;
	}

    am_hal_rtc_time_get(&RTC_time);
    
	//百分比
	word.x_axis = 220;
	
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", RTC_time.ui32Hour,RTC_time.ui32Minute);
	
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - strlen(str)*8 - 4 - Img_Battery_26X12.width) / 2;
	word.size = LCD_FONT_8_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	//电量
//	word.x_axis += 4;
	word.y_axis += (strlen(str)*8 + 4);
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_NONE, LCD_NONE, &Img_Battery_26X12);
	
	//填充剩余电量
	LCD_SetRectangle(word.x_axis + 2, 8, word.y_axis + 2, percent * (Img_Battery_26X12.width - 5) / 100, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
 #endif
}

//翻页提示
void gui_sport_page_prompt(uint8_t type)
{
	switch(type)
	{
		case SPORT_PAGE_UP:
			//上翻提示
			gui_page_up(LCD_PERU);
			break;
		case SPORT_PAGE_DOWN:
			//下翻提示
			gui_page_down(LCD_BLACK);
			break;
		default:
			return;
	}
}

//设置GPS闪烁状态
void set_gps_flicker(uint8_t flicker)
{
    gps_flicker = flicker;
}

//获取GPS闪烁状态
uint8_t get_gps_flicker(void)
{
    return gps_flicker;
}

//gps状态
void gui_sport_gps_status(uint8_t direction)
{
	SetWord_t word = {0};
	static bool gps_status = true;

    
	switch(direction)
	{
		case SPORT_GPS_LEFT1:	//GPS图标靠左1
			word.x_axis = 109;
			word.y_axis = 10;
			break;
		case SPORT_GPS_LEFT2:	//GPS图标靠左2
			word.x_axis = 109;
			word.y_axis = 18;
			break;
		case SPORT_GPS_LEFT3:	//GPS图标靠左3
			word.x_axis = 90;
			word.y_axis = 10;
			break;
		case SPORT_GPS_UP:		//GPS图标靠上
			word.x_axis = 10;
			word.y_axis = 112;
			break;
		case 4:
			word.x_axis = 200;
			word.y_axis = 112;
			break;
		case SPORT_GPS_RIGHT_UP://右上
			word.x_axis = 57;
			word.y_axis = 205;
			break;
			case SPORT_GPS_RIGHT_UP_6://右
			word.x_axis = 112;
			word.y_axis = 205;
			break;
			case SPORT_GPS_RIGHT_UP_COD:
				word.x_axis = 120-img_gps_god.height;
			    word.y_axis = 20;
				break;
		default:
			return;
	}

	if (GetGpsStatus()== true)
	{
		gps_status = true;
		if(direction==SPORT_GPS_RIGHT_UP_COD)
			{
			
				
			LCD_SetPicture(word.x_axis,word.y_axis,LCD_GREEN,LCD_NONE,&img_gps_god);
			}
		else
			{
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_FORESTGREEN,LCD_NONE,&img_tool_gps_13x21);
			}
	}
	else
	{
		if(gps_flicker == 0)
		{
		    
			if(direction==SPORT_GPS_RIGHT_UP_COD)
				{
                 
				 LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&img_gps_god);

			   }
			else
				{
			LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&img_tool_gps_13x21);
				}
		}
		else
		{
			if(gps_status)
			{   
				if(direction==SPORT_GPS_RIGHT_UP_COD)
			    	{
                   
				   LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&img_gps_god);

				  }
				else
					{
				LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&img_tool_gps_13x21);
					}
			}
			gps_status = !gps_status;  
		}
		
	}
}

//振动提示
void gui_sport_motor_prompt(void)
{
#if defined WATCH_SIM_SPORT
	if (Get_PauseSta() == false)
	{
		//用于开始或者暂停运动
		timer_notify_motor_start(500,500,1,false,NOTIFY_MODE_SPORTS);
		timer_notify_buzzer_start(500,500,1,false,NOTIFY_MODE_SPORTS);
	}
	else
	{
		//用于开始或者暂停运动
		timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
		timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
	}
#else
		//用于开始或者暂停运动
		timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
		timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
#endif
}



void SportContinueLaterFlagSet(void)
{
	IS_SPORT_CONTINUE_LATER = 1;
	#if defined (WATCH_STEP_ALGO)
	#else
	move_int =1;
	#endif
	
}
void SportContinueLaterFlagClr(void)
{
	IS_SPORT_CONTINUE_LATER = 0;
	
}


ScreenState_t sports_menu_index_record(void)
{
	Record_Sport_Num = Get_Curr_Sport_Index();
	return Record_Sport_Num;
}


//放弃字幕
void gui_sport_cancel_caption_paint(void)
{
    SetWord_t word = {0};
    
    word.x_axis = 48;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
    word.forecolor = LCD_BLACK;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)RunPauseStrs[SetValue.Language][3],&word);
    word.y_axis = 120 + 24 +2;

    LCD_SetString("?",&word);
}

//放弃选项 
void gui_sport_cancel_options_paint(bool index)
{
    SetWord_t word = {0};
   
    word.x_axis = 133;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);

		word.size = LCD_FONT_16_SIZE;
    //word.x_axis = 183;   24bit字体对齐线
		word.x_axis = 187;
    LCD_SetString((char *)CancelStrs[SetValue.Language][!index],&word);
}


//运动感受选项
void gui_sport_feel_options_paint(uint8_t index)
{
    SetWord_t word = {0};
    
   // word.x_axis = 104;
	//	word.size = LCD_FONT_32_SIZE;
		word.x_axis = 108;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_BLACK;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportFeelStrs[SetValue.Language][index],&word);

    word.x_axis = 44;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;

    if (index == FeelEase)
    {
        LCD_SetString((char *)SportFeelStrs[SetValue.Language][8],&word);

    }
    else  
    {
        LCD_SetString((char *)SportFeelStrs[SetValue.Language][index -1],&word);
    }

    word.x_axis = 180;

    if (index == FeelExhausted)
    {
        LCD_SetString((char *)SportFeelStrs[SetValue.Language][0],&word);

    }
    else 
    {
        LCD_SetString((char *)SportFeelStrs[SetValue.Language][index +1],&word);
    }
}

//运动放弃界面
void gui_sport_cancel_paint(void)
{
   SetWord_t word = {0};

    //设置背景白色
	LCD_SetBackgroundColor(LCD_WHITE);

    gui_sport_cancel_caption_paint();
    
    LCD_SetRectangle(120,120,0,LCD_LINE_CNT_MAX,LCD_BLACK,0,0,LCD_FILL_ENABLE);

    gui_sport_cancel_options_paint(cancel_index);

    word.x_axis = 170;
	LCD_SetRectangle(word.x_axis,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_WHEAT, LCD_NONE, &Img_Pointing_Down_12X8);

    LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
 
}

//运动感受界面
void gui_sport_feel_paint(void)
{
   SetWord_t word = {0};

    //设置背景黑色
	LCD_SetBackgroundColor(LCD_WHITE);
    
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Up_12X8);
    
    gui_sport_feel_options_paint(sport_feel_index);
    word.x_axis = 80;
	LCD_SetRectangle(word.x_axis-1,2,0 ,240,LCD_BLACK,0,0,LCD_FILL_ENABLE);

    word.x_axis = 160;
	LCD_SetRectangle(word.x_axis-1,2,0 ,240,LCD_BLACK,0,0,LCD_FILL_ENABLE);



    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);

 
}

//心率恢复率界面
void gui_sport_heart_recovery_paint(void)
{
	SetWord_t word = {0};
  	char str[18];
	uint8_t value = Get_Sport_Heart();
	
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_WHITE);

  	word.x_axis = 18;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;

	LCD_SetString((char *)SportDetailStrs[SetValue.Language][42],&word);

	word.x_axis = 40;
	word.y_axis = 42;
	word.size = LCD_FONT_29_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	if(value == 0)
	{
		//心率为0
		LCD_SetRectangle(word.x_axis + Font_Number_24.height, 2, 
							word.y_axis, Font_Number_24.width - 1, 
							LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
		
		LCD_SetRectangle(word.x_axis + Font_Number_24.height, 2, 
							word.y_axis + Font_Number_24.width + 2, Font_Number_24.width - 1, 
							LCD_BLACK, 0, 0, LCD_FILL_ENABLE);
	}
	else
	{
		memset(str,0,18);
		sprintf(str,"%d",value);
		LCD_SetNumber(str,&word);
	}

	if (sport_end_time >=60)
	{
		word.x_axis = 50;
		word.y_axis = 140;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;

		LCD_SetString("HRR",&word);

		word.x_axis = 40;
		word.y_axis = 168;
		word.size = LCD_FONT_29_SIZE;
		word.kerning = 1;
		memset(str,0,10);
		#if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%01d",ActivityData.ActivityDetails.CommonDetail.HRRecovery/10,ActivityData.ActivityDetails.CommonDetail.HRRecovery%10);
		#else
	    sprintf(str,"%0.1f",ActivityData.ActivityDetails.CommonDetail.HRRecovery/10.f);
		#endif

		LCD_SetNumber(str,&word);

	}
	//画心率波形图
	gui_heartrate_wave_paint();
	
	

	//底部计时
	memset(str,0,10);
	sprintf(str,"%d:%02d",sport_end_time/60,sport_end_time%60);

	word.x_axis = 208;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 1;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetNumber(str,&word);
}


//运动保存成功界面
void gui_sport_saved_paint(void)
{
   SetWord_t word = {0};

    //设置背景黑色
	LCD_SetBackgroundColor(LCD_WHITE);

    LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 0, 360, LCD_SPRINGGREEN);
	
   // LCD_SetPicture(60, LCD_CENTER_JUSTIFIED, LCD_CYAN, LCD_NONE, &img_set_time_utc);
   
   LCD_SetPicture(60, LCD_CENTER_JUSTIFIED, LCD_SPRINGGREEN, LCD_NONE, &img_sport_save_ok);
   

    word.x_axis = 150;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_BLACK;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportsPromptStrs[SetValue.Language][2],&word);
 
}

//运动正在保存界面
void gui_sport_saving_paint(uint8_t endangle)
{
   SetWord_t word = {0};


    //设置背景黑色
	LCD_SetBackgroundColor(LCD_WHITE);

   
	word.x_axis = 108;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_BLACK;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportsPromptStrs[SetValue.Language][3],&word);

    if (endangle > 6)
    {
        endangle = 6;
    }
    
    LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 0, 60*endangle, LCD_BROWN);
 
}

//界面切换消息
void gui_swich_msg(void)
{
    DISPLAY_MSG  msg = {0};
    msg.cmd = MSG_DISPLAY_SCREEN;
    xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

}

//运动后下键切换。FirstIndex : 运动中第一个子界面， LastIndex:运动中最后个子界面
void gui_sport_index_down(ScreenState_t FirstIndex,ScreenState_t LastIndex)
{
   // DISPLAY_MSG  msg = {0};
    if( Sport_Index == FirstIndex )
    {
        Sport_Index = LastIndex;
    }
    else
    {
        Sport_Index--;
    }
    ScreenState = Sport_Index;
    
    gui_swich_msg();

}

//运动后上键切换。FirstIndex : 运动中第一个子界面， LastIndex:运动中最后个子界面
void gui_sport_index_up(ScreenState_t FirstIndex,ScreenState_t LastIndex)
{
    //DISPLAY_MSG  msg = {0};
    if( Sport_Index == LastIndex )
    {
        Sport_Index = FirstIndex;
    }
    else
    {
        Sport_Index++;
    }
    ScreenState = Sport_Index ;
    gui_swich_msg();

}

void gui_sport_cancel_init(void)
{
	//默认为否
	cancel_index = false;
}

//运动放弃界面处理
void gui_sport_cancel_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
            if (cancel_index== true)
            {
							  trace_back_timer_delete();    //放弃关闭定时器。
                ScreenState = DISPLAY_SCREEN_HOME;
                //结束运动不保存
								SportContinueLaterFlagClr(); //清除稍后继续状态
                Set_SaveSta(0);
                CloseSportTask();

				//振动提示
				gui_sport_motor_prompt();
            }
            else
            {
                ScreenState = Pause_Save_Index;
            }
	        gui_swich_msg();
			break;
        case KEY_UP:
		case KEY_DOWN:
             cancel_index=!cancel_index;
             gui_swich_msg();
			break; 
		case KEY_BACK:
             ScreenState = Pause_Save_Index;
             gui_swich_msg();
			break;
		default:
			break;
	}	
}

//运动感受界面处理
void gui_sport_feel_btn_evt(uint32_t Key_Value)
{
    // DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_OK:
			{
			if (SetValue.HRRecoverySet == 1)
			{
				if((ActivityData.Activity_Type ==ACT_RUN ) ||(ActivityData.Activity_Type ==ACT_WALK)
					||(ActivityData.Activity_Type ==ACT_MARATHON)||(ActivityData.Activity_Type ==ACT_INDOORRUN)
					||(ActivityData.Activity_Type ==ACT_CROSSCOUNTRY)||(ActivityData.Activity_Type ==ACT_HIKING)
					||(ActivityData.Activity_Type ==ACT_CLIMB)||(ActivityData.Activity_Type ==ACT_CYCLING)
					||(ActivityData.Activity_Type ==ACT_TRAIN_PLAN_RUN))
				{
					ScreenState = DISPLAY_SCREEN_SPORT_HRRECROVY;
					SetSubjectiveEvaluate(sport_feel_index);
					gui_swich_msg();
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_SPORT_SAVING;
				//结束运动，保存数据
					SetSubjectiveEvaluate(sport_feel_index);
					Set_SaveSta(1);
					CloseSportTask();
			
			
					gui_swich_msg();
					timer_progress_start(200);
				}
			}
			else
			{
				ScreenState = DISPLAY_SCREEN_SPORT_SAVING;
				//结束运动，保存数据
				SetSubjectiveEvaluate(sport_feel_index);
				Set_SaveSta(1);
				CloseSportTask();
			
			
				gui_swich_msg();
				timer_progress_start(200);
			}

				//训练计划结果保存
			Store_TranPlanUploadData();
			
			//振动提示
			gui_sport_motor_prompt();
			}
			break;
		case KEY_DOWN:
           if (sport_feel_index == FeelExhausted)
           {
             sport_feel_index = FeelEase ;

           }
         else
           {
                sport_feel_index++;
           }
             gui_swich_msg();
			break;
		case KEY_UP:
           if (sport_feel_index == FeelEase)
           {
             sport_feel_index = FeelExhausted ;

           }
         else
           {
                sport_feel_index--;
           }
             gui_swich_msg();

            break; 
		case KEY_BACK:
            // ScreenState = Pause_Save_Index;
            // gui_swich_msg();
			break;
		default:
			break;
	}	
}
void gui_sport_heart_recovery_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_BACK:
			{
				ScreenState = DISPLAY_SCREEN_SPORT_SAVING;
				Set_SaveSta(1);
				CloseSportTask();
				gui_swich_msg();
				timer_progress_start(200);
			}
			break;
		default:
			break;

	}

}

void gui_sport_remaining_distance_paint(uint32_t distance)
{
	SetWord_t word = {0};
	char str[10];
 //   uint8_t len;

    memset(str,0,10);
 	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",distance/100000,distance/1000%100);
	#else
    sprintf(str,"%0.2f",distance/100000.f);
	#endif
//    len = strlen(str);
    word.x_axis = 30;
//    word.y_axis =(LCD_LINE_CNT_MAX - (len * 15 + 4+ 2 * 8))/2 ;
		word.y_axis = LCD_CENTER_JUSTIFIED;
    word.bckgrndcolor = LCD_BLACK;
    word.forecolor = LCD_WHITE;
    word.size = LCD_FONT_32_SIZE;
    word.kerning = 1;
    LCD_SetNumber(str,&word);
    
    //km
    word.x_axis = 50;
//    word.y_axis += (len*15 + 4);
		word.y_axis = 160;
    word.bckgrndcolor = LCD_BLACK;
    word.forecolor = LCD_WHITE;
    word.size = LCD_FONT_16_SIZE;
    word.kerning = 0;
    LCD_SetString("km",&word);

}


void gui_sport_sail_round_time_paint(uint8_t x,uint8_t y,uint8_t forecolor)
{

    SetWord_t word = {0};
		char str[10];
		
		word.x_axis = x;
		word.bckgrndcolor = LCD_NONE;
		word.forecolor = forecolor;
		word.size = LCD_FONT_48_SIZE;
		word.kerning = 1;
//时
		word.y_axis=LCD_CENTER_JUSTIFIED;
		if(ActivityData.Act_Stop_Time.Hour > 0)
		{
			//时分秒
			sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
		}
		else
		{
			//分秒
			sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
		}		
		LCD_SetNumber(str,&word);
}


//运动计时
void gui_sport_traceback_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};

	word.size = LCD_FONT_24_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.x_axis = 176;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	memset(str,0,sizeof(str));

	if(ActivityData.Act_Stop_Time.Hour > 0)
		{
			//时分秒
			sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
		}
	else
		{
			//分秒
			sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
		}		
	LCD_SetNumber(str,&word);	
}



void gui_sport_traceback_heartrate_paint(uint32_t distance, uint8_t heart)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	//里程图标
	word.x_axis = 40;
	word.y_axis = 60;
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_Distance_16X16);
	
	//里程数据
	word.y_axis += (Img_Distance_16X16.width + 4);
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_NONE;
	word.kerning = 1;
	
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02dkm",distance/100000,distance/1000%100);
	#else
    sprintf(str,"%0.2fkm",distance/100000.f);
	#endif
	LCD_SetString(str,&word);
	
	//心率图标
	word.y_axis = 140;
	LCD_SetPicture(word.x_axis,word.y_axis,LCD_NONE,LCD_NONE,&Img_heartrate_18X16);
	
	//心率数据
	word.y_axis += (Img_heartrate_18X16.width + 4);
	memset(str,0,sizeof(str));
    sprintf(str,"%d",heart);
	LCD_SetString(str,&word);

}


void gui_sport_traceback_nearest_distance(uint32_t distance)
{
	SetWord_t word = {0};
	char str[10] = {0};

//word.y_axis = (LCD_PIXEL_ONELINE_MAX - 5*Font_Number_24.width - 9 -  2*Font_Number_16.width)/2;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.x_axis = 176;
	
	word.y_axis = 40;
	LCD_SetString("????: ", &word);
	
	word.y_axis = 128;
	memset(str,0,sizeof(str));
		#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02dkm",distance/100000,distance/1000%100);
	#else
	sprintf(str, "%.2fkm", distance / 100000.f);
	#endif
	
	LCD_SetString(str,&word);	
}

//轨迹
static void gui_sport_c_1_track(uint16_t angle)
{
	//需判断是否是循迹返航
	
//	GUI_RUN_PRINTF("gui_sport track: angle: %d", angle);
	//指向箭头
	//gui_sport_arrows(angle, LCD_AQUA);
	  DrawLoadTrack();
	  DrawTrack();
       //航迹标点与指针切换显示
	   if (dottrack_draw_time > 0)
	   {
		 gui_sport_dottrack_nums(dot_track_nums); //航迹打点标号
	   }
	   else if (TrackMarkIndex <= TRACK_MARK_NUM_MAX )
	  {
	   	  draw_track_mark();
	  }
	  else 
	  {
	      Drawsport_arrows();
	  }
}


void gui_sport_traceback_offtrace_paint(uint32_t distance)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
    gui_sport_c_1_track(0);

	gui_sport_index_circle_paint(0, 2, 1);
	
	gui_sport_traceback_heartrate_paint(ActivityData.ActivityDetails.ClimbingDetail.Distance, Get_Sport_Heart());
	
	gui_sport_traceback_nearest_distance(distance);
	
	//gui_climbing_realtime_battery_paint(205, g_bat_evt.level_percent);
	gui_sport_realtime_battery(1);
	
	
	gui_sport_compass(0);
	
	gui_sport_gps_status(SPORT_GPS_LEFT2);
	
}



/********************************************
//
//运动循迹返航界面、按键处理及暂停界面
//
*********************************************/
void gui_sport_track_trackback_paint(uint32_t distance)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
    gui_sport_c_1_track(0);

	gui_sport_index_circle_paint(0, 2, 1);
	
	gui_sport_traceback_heartrate_paint(distance, Get_Sport_Heart());
	
	gui_sport_traceback_time();
	
	gui_sport_realtime_battery(1);
	

	gui_sport_compass(0);
	
	gui_sport_gps_status(SPORT_GPS_LEFT2);
}


void gui_sport_traceback_remaining_distance(uint32_t distance)
{
	SetWord_t word = {0};
	char str[10] = {0};

//word.y_axis = (LCD_PIXEL_ONELINE_MAX - 5*Font_Number_24.width - 9 -  2*Font_Number_16.width)/2;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.x_axis = 176;
	
	word.y_axis = 40;
	LCD_SetString("????: ", &word);
	
	word.y_axis = 128;
	memset(str,0,sizeof(str));
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02dkm",distance/100000,distance/1000%100);
	#else
	sprintf(str, "%.2fkm", distance / 100000.f);
	#endif
	
	LCD_SetString(str,&word);	
}





void gui_sport_track_trackback_paint2(uint32_t distance)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
    gui_sport_c_1_track(0);
	gui_sport_index_circle_paint(0, 2, 1);
	
	gui_sport_traceback_heartrate_paint(distance, Get_Sport_Heart());
	
	gui_sport_traceback_remaining_distance(TRACE_BACK_DATA.remaining_distance);
	
	gui_sport_realtime_battery(1);
	
	//gui_sport_c_1_track(0);
	gui_sport_compass(0);
	
	gui_sport_gps_status(SPORT_GPS_LEFT2);
}



//详情心率区间界面
void gui_sport_detail_heartratezone_paint(uint8_t backgroundcolor)
{
   SetWord_t word = {0};
   char str[10];
   uint8_t width,i;
   uint32_t MaxHeartRateZone = 0;
   uint8_t charlen;


   for (i = 0; i < 5;i++)
   {
        if (MaxHeartRateZone < ActivityData.ActivityDetails.CommonDetail.HeartRateZone[i])
        {
           MaxHeartRateZone = ActivityData.ActivityDetails.CommonDetail.HeartRateZone[i];
        } 
   }
   
    //设置背景黑色
    LCD_SetBackgroundColor(backgroundcolor);
    if (backgroundcolor == LCD_BLACK)
    {
        //黑底白字
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //白底黑字
        word.forecolor = LCD_BLACK;
    }
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Up_12X8);

    word.x_axis = 25;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][13],&word);
    //极限
    charlen = strlen(HeartZoneStrs[SetValue.Language][0]);
    word.x_axis = 60;
    word.y_axis = 43;
    word.size = LCD_FONT_16_SIZE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)HeartZoneStrs[SetValue.Language][0],&word);
    word.y_axis += (charlen*8+3);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[4]/60),(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[4]%60));
#if !defined WATCH_SIM_NUMBER
			word.kerning =1;
#endif
			LCD_SetNumber(str,&word);
#if !defined WATCH_SIM_NUMBER
			word.kerning =0;
#endif

    word.y_axis +=50;
    width = 80 * ActivityData.ActivityDetails.CommonDetail.HeartRateZone[4]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_RED,0,0,LCD_FILL_ENABLE);

    //无氧耐力
	charlen = strlen(HeartZoneStrs[SetValue.Language][1]);
    word.x_axis = 90;
    word.y_axis = 11;
    LCD_SetString((char *)HeartZoneStrs[SetValue.Language][1],&word);
    word.y_axis += (charlen*8+3);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[3]/60),(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[3]%60));
#if !defined WATCH_SIM_NUMBER
			word.kerning =1;
#endif
			LCD_SetNumber(str,&word);
#if !defined WATCH_SIM_NUMBER
			word.kerning =0;
#endif
    word.y_axis +=50;
    width = 80* ActivityData.ActivityDetails.CommonDetail.HeartRateZone[3]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_PERU,0,0,LCD_FILL_ENABLE);
	
	//有氧耐力
	charlen = strlen(HeartZoneStrs[SetValue.Language][2]);
    word.x_axis = 120;
    word.y_axis = 11;
    LCD_SetString((char *)HeartZoneStrs[SetValue.Language][2],&word);
    word.y_axis += (charlen*8+3);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[2]/60),(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[2]%60));
#if !defined WATCH_SIM_NUMBER
			word.kerning =1;
#endif
			LCD_SetNumber(str,&word);
#if !defined WATCH_SIM_NUMBER
			word.kerning =0;
#endif
    word.y_axis +=50;
    width = 80 * ActivityData.ActivityDetails.CommonDetail.HeartRateZone[2]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_OLIVERDRAB,0,0,LCD_FILL_ENABLE);

	//燃脂
	charlen = strlen(HeartZoneStrs[SetValue.Language][3]);
    word.x_axis = 150;
    word.y_axis = 43;
    LCD_SetString((char *)HeartZoneStrs[SetValue.Language][3],&word);
    word.y_axis += (charlen*8+3);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[1]/60),(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[1]%60));
#if !defined WATCH_SIM_NUMBER
			word.kerning =1;
#endif
			LCD_SetNumber(str,&word);
#if !defined WATCH_SIM_NUMBER
			word.kerning =0;
#endif
    word.y_axis +=50;
    width = 80* ActivityData.ActivityDetails.CommonDetail.HeartRateZone[1]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);

	//热身
	charlen = strlen(HeartZoneStrs[SetValue.Language][4]);
    word.x_axis = 180;
    word.y_axis = 43;
    LCD_SetString((char *)HeartZoneStrs[SetValue.Language][4],&word);
    word.y_axis += (charlen*8+3);
    memset(str,0,10);
    sprintf(str,"%d:%02d",(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[0]/60),(ActivityData.ActivityDetails.CommonDetail.HeartRateZone[0]%60));
#if !defined WATCH_SIM_NUMBER
			word.kerning =1;
#endif
			LCD_SetNumber(str,&word);
#if !defined WATCH_SIM_NUMBER
			word.kerning =0;
#endif

    word.y_axis +=50;
    width = 80* ActivityData.ActivityDetails.CommonDetail.HeartRateZone[0]/MaxHeartRateZone;
    LCD_SetRectangle(word.x_axis,16,word.y_axis,width,LCD_DARKTURQUOISE,0,0,LCD_FILL_ENABLE);

 
}




#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT

//数据显示总时间
void gui_sport_data_display_total_time_paint(uint8_t state,uint32_t time)
{
	SetWord_t word = {0};
	char str[10];
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 80+15;
		
		//word.size = LCD_FONT_49_SIZE;
		
		word.size = LCD_FONT_56_SIZE;
		
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	if(time >= 3600)
	{
					//时分秒
		sprintf(str,"%d:%02d:%02d",time/3600, time/60%60, time%60);
	}
	else
	{
					//分秒
		sprintf(str,"%d:%02d",time/60%60, time%60);
	}		
	LCD_SetNumber(str,&word);
	

}
//数据显示总时间(乳酸阈)
void gui_lactate_time_paint(uint8_t state,uint32_t time)
	{
		SetWord_t word = {0};
		char str[10];
		uint8_t len;
		uint8_t charlen;
		memset(str,0,10);
		if(time >= 3600)
	    {
					//时分秒
		sprintf(str,"%d:%02d:%02d",time/3600, time/60%60, time%60);
	   }
	   else
	   {
					//分秒
		sprintf(str,"%d:%02d",time/60%60, time%60);
	   }	
			len = strlen(str);
			word.bckgrndcolor = LCD_NONE;
			word.forecolor = LCD_WHITE;
			word.kerning = 1;
			 if(state == SPORT_DATA_DISPALY_INDEX_NEW_LACTATE)
				{
				   word.x_axis = 120 - 12 - Font_Number_37.height;
				   word.y_axis = 120 + 20 ;
				   word.size = LCD_FONT_37_SIZE;
				   LCD_SetNumber(str,&word);
	
				}
				else;
			
			
		 if(state == SPORT_DATA_DISPALY_INDEX_NEW_LACTATE)//新UI乳酸阈显示
		{
			 
			 word.size = LCD_FONT_16_SIZE;
			 word.x_axis = 120 - 12 - Font_Number_37.height - 12 - 16;
			 word.y_axis = 120 + 20; 
			 word.kerning = 0;
			LCD_SetString("用时",&word);
	
		 }
	 else;
			
			
		}


//数据显示总里程
void gui_sport_data_display_total_distance_paint(uint8_t state,uint32_t distance)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		//word.x_axis = 80 - 15 - 29;
		
		//word.size = LCD_FONT_29_SIZE;
		
		word.x_axis = 80 - 15 - 37;
		word.size = LCD_FONT_37_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	sprintf(str,"%0.2f",distance/100000.f);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=40;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=37-16;
		word.y_axis =120+(len*Font_Number_37.width)/2 + 8;
	}
	
	LCD_SetString("km",&word);

}

//数据显示总里程:乳酸阈
void gui_lactate_distance_paint(uint8_t state,uint32_t distance)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	memset(str,0,10);
	sprintf(str,"%0.2f",distance/100000.f);
	len = strlen(str);
	len = 3;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	 if(state == SPORT_DATA_DISPALY_INDEX_NEW_LACTATE)//新UI乳酸阈显示里程值
		{
         
		 word.x_axis = 120 - 12 - Font_Number_37.height;
		 word.y_axis = 120 - 20 - (len-1)*(Font_Number_37.width+word.kerning) - Font_Number_37.width/2 - 10;
     	 word.size = LCD_FONT_37_SIZE;
        LCD_SetNumber(str,&word);
	    }
	
	else;
	
	

 if(state == SPORT_DATA_DISPALY_INDEX_NEW_LACTATE)//新UI乳酸阈显示
	{
         
     	 word.size = LCD_FONT_16_SIZE;
         word.x_axis = 120 - 12 - Font_Number_37.height - 12 - 16;
		 word.y_axis = 120 - 20 - (len-1)*(Font_Number_37.width+word.kerning) - Font_Number_37.width/2 - 10; 
		 word.kerning = 0;
		LCD_SetString("里程km",&word);

	 }
 else;

}

//数据显示实时配速
void gui_sport_data_display_realtime_pace_paint(uint8_t state,uint16_t pace)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 160+15;
		
		//word.size = LCD_FONT_29_SIZE;
		
		word.size = LCD_FONT_37_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d:%02d",(pace/60),(pace%60));
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		//word.x_axis +=18;
		
		word.x_axis +=37 - 16;
		word.y_axis =120+(len*Font_Number_37.width)/2 + 8;
	}
	
	LCD_SetString("/km",&word);
	word.kerning = 1;
	
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
}

//数据显示实时配速:乳酸阈
void gui_lactate_pace_paint(uint8_t state,uint16_t pace)
	{
		SetWord_t word = {0};
		char str[10];
		uint8_t len;
		uint8_t charlen;
		memset(str,0,10);
		sprintf(str,"%d:%02d",(pace/60),(pace%60));
		len = strlen(str);
		len = 3;
		word.bckgrndcolor = LCD_NONE;
		word.forecolor = LCD_WHITE;
		word.kerning = 1;
		 if(state == SPORT_DATA_DISPALY_INDEX_NEW_LACTATE)
		 	{
               word.x_axis = 120 + 12 + 16 + 12 ;
		       word.y_axis = 120 - 20 - (len-1)*(Font_Number_37.width+word.kerning) - Font_Number_37.width/2 - 10;
     	       word.size = LCD_FONT_37_SIZE;
               LCD_SetNumber(str,&word);

		    }
			else;
		
		
	 if(state == SPORT_DATA_DISPALY_INDEX_NEW_LACTATE)//新UI乳酸阈显示
	{
         
     	 word.size = LCD_FONT_16_SIZE;
         word.x_axis = 120 + 8;
		 word.y_axis = 120 - 20 - (len-1)*(Font_Number_37.width+word.kerning) - Font_Number_37.width/2 - 10; 
		 
		 word.kerning = 0;
		LCD_SetString("配速/km",&word);

	 }
 else;
		
		
	}

void gui_lactate_bpm_paint(uint8_t state,uint16_t bpm)
	{
			SetWord_t word = {0};
			char str[10];
			uint8_t len;
			uint8_t charlen;
			memset(str,0,10);
			sprintf(str,"%d",bpm);
			len = strlen(str);
			word.bckgrndcolor = LCD_NONE;
			word.forecolor = LCD_WHITE;
			word.kerning = 1;
			 if(state == SPORT_DATA_DISPALY_INDEX_NEW_LACTATE)
				{
				   word.x_axis = 120 + 12 + 16 + 12 ;
				   word.y_axis = 120 + 20;
				   word.size = LCD_FONT_37_SIZE;
				   LCD_SetNumber(str,&word);
	
				}
				else;
			
			
		 if(state == SPORT_DATA_DISPALY_INDEX_NEW_LACTATE)//新UI乳酸阈显示
		{
			 
			 word.size = LCD_FONT_16_SIZE;
			 word.x_axis = 120 + 12;
			 word.y_axis = 120 + 20; 
			 word.kerning = 0;
			LCD_SetString("心率bpm",&word);
	
		 }
	 else;
			
			
		}


//数据显示实时速度
void gui_sport_data_display_realtime_speed_paint(uint8_t state,uint16_t speed)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",speed/10,speed%10);
	#else
	sprintf(str,"%.1f",speed/10.f);		//?????????
	#endif
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("km/h",&word);
	word.kerning = 1;
	
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
	

}
//飞腕电池显示
void gui_sport_battery_paint(void)
{
	SetWord_t word = {0};
	uint8_t percent = g_bat_evt.level_percent;
	uint8_t color;

#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	word.x_axis = 210;
#else
	word.x_axis = 170;
#endif
	word.y_axis = LCD_CENTER_LINE -(Img_Battery_32X15.width/2);
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_WHITE, LCD_NONE, &Img_Battery_32X15);

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
	LCD_SetRectangle(word.x_axis + 2, 11, word.y_axis + 2, percent * (Img_Battery_32X15.width - 6) / 100, color, 0, 0, LCD_FILL_ENABLE);
		
}

#endif

/*#if defined WATCH_COM_SPORT
//铁腕只一栏心率界面
void gui_sport_heart_1_paint(uint8_t heart)
{

	SetWord_t word = {0};
	char str[10];
	
	
	//心率图
	word.x_axis = 50;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &Img_heartrate_32X28);


	//心率值
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = get_hdr_range_color(heart);
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 92;
	word.size = LCD_FONT_48_SIZE;

	if(heart == 0)
	{	
		LCD_SetRectangle(121,3,93,26,LCD_WHITE,0,0,LCD_FILL_ENABLE);

		LCD_SetRectangle(121,3,121,26,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	}
	else
	{
		memset(str,0,10);
		sprintf(str,"%d",heart);
		LCD_SetNumber(str,&word);
		
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = 170;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.kerning = 0;
		LCD_SetString("bpm",&word);
	}

		
}


//铁腕只一栏实时时间界面
void gui_sport_realtime_1_paint(uint8_t type)
{

	SetWord_t word = {0};
	char str[30];

	am_hal_rtc_time_get(&RTC_time);
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	int32_t m_remain_minute1 = 0,m_remain_minute2 = 0;
	
	m_remain_minute1 = RTC_time.ui32Hour * 60 + RTC_time.ui32Minute;
	m_remain_minute2 = g_track_mark_data.track.close_time_hour * 60 
										 + g_track_mark_data.track.close_time_minute;
	if(g_track_mark_data.track.close_time_index > 0 && m_remain_minute1 <= m_remain_minute2
			&& is_sport_type_cloud_navigation_valid(ActivityData.Activity_Type) == true
			&& g_track_mark_data.is_close_time_valid == 1
			&& get_share_track_back_discreen() == GOTO_SHARE_TRACK_BY_GPS
			&& TRACE_BACK_DATA.is_traceback_flag == 0)
	{//若有关门时间 且 实际时间小于关门时间 赛事运动中有关门时间 非循迹返航
		LCD_SetRectangle(120,1,0,LCD_LINE_CNT_MAX,LCD_AUQAMARIN,0,0,LCD_FILL_ENABLE);
		
		if(type == 1)
		{
			//gps状态
			gui_sport_gps_status(SPORT_GPS_LEFT3);
		}
		//北京时间
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = 36;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.kerning = 0;
		LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][19],&word);
		
		word.x_axis += 32;
		word.kerning = 1;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_32_SIZE;
		memset(str,0,30);
		sprintf(str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
		LCD_SetNumber(str,&word);
		
		//距离CP1 关门时间还剩
		memset(str,0,30);
		sprintf(str,"%s%d %s",(char *)GuiSportEventStrs[SetValue.Language][20]
												 ,g_track_mark_data.track.close_time_index
												 ,(char *)GuiSportEventStrs[SetValue.Language][21]);
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = 130;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_RED;
		word.kerning = 0;
		LCD_SetString(str,&word);
		//关门时间
		m_remain_minute2 = m_remain_minute2 - m_remain_minute1;
		word.x_axis += 24;
		word.kerning = 1;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_32_SIZE;
		word.forecolor = LCD_WHITE;
		memset(str,0,30);
		sprintf(str,"%02d:%02d",m_remain_minute2 / 60,m_remain_minute2 % 60);
		LCD_SetNumber(str,&word);
	}
	else
	{//无关门时间显示
		if(type == 1)
		{
			//gps状态
			gui_sport_gps_status(SPORT_GPS_LEFT1);
		}
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = 50;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_LIGHTGRAY;
		word.kerning = 0;
		LCD_SetString("TIME",&word);
		
		
		//实时时间
		word.bckgrndcolor = LCD_NONE;
		word.forecolor = LCD_WHITE;
		word.kerning = 1;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;

		memset(str,0,30);
		sprintf(str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
		LCD_SetNumber(str,&word);
	}
#else
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 50;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.kerning = 0;
	LCD_SetString("TIME",&word);
	
	
	//实时时间
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 92;
	word.size = LCD_FONT_48_SIZE;

	memset(str,0,30);
	sprintf(str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
	LCD_SetNumber(str,&word);
#endif
	//电量
	gui_sport_battery_paint();
}

void gui_sport_pause_hint_paint(void)
{
	SetWord_t word = {0};

	//底圈
	LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 36, LCD_BLACK, 0, LCD_FILL_ENABLE);
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 40, 5, 0, 360, LCD_WHITE);
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 40, 1, 0, 360, LCD_BLACK);
	
	
	if (Get_PauseSta()== false)
	{
		//开始
		word.x_axis = LCD_CENTER_ROW - Img_start_30X42.height/2;
		word.y_axis = LCD_CENTER_LINE+5 - Img_start_30X42.width/2;
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_GREEN,LCD_NONE,&Img_start_30X42);
		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_BLACK);
		
	}
	else 
	{
		//暂停
		word.x_axis = LCD_CENTER_ROW - Img_pause_30X42.height/2;
		word.y_axis = LCD_CENTER_LINE - Img_pause_30X42.width/2;
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&Img_pause_30X42);

		//暂时增加红色圆圈，表示暂停状态
		LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}

}



//运动暂停计时
void gui_sport_pause_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;

	//计算总时长
	second = PauseTime.Pause_Stop_Time.Hour*60*60 + PauseTime.Pause_Stop_Time.Minute*60 + PauseTime.Pause_Stop_Time.Second;
	
	if(second >= 6000)
	{
		//暂停时长不超过99:59
		second = 5999;
	}
	
	memset(str,0,sizeof(str));
	sprintf(str,"%d:%02d",second/60,second%60);
	
	//暂停时长
	word.x_axis = 20;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - 48 - 5 - strlen(str)*(Font_Number_16.width+1))/2;
	word.size = LCD_FONT_16_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 0;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][1],&word);
	
	word.y_axis += 48 + 6;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
}

//运动暂停活动计时
void gui_sport_pause_acttime(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 57;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_48_SIZE;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	if(ActivityData.Act_Stop_Time.Hour > 0)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);	
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	LCD_SetNumber(str, &word);
}

//放弃菜单选项
void gui_sport_cancel_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//菜单第一项
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
	
	//菜单第二项
	if(index == 1)
	{
		index = 0;
	}
	else
	{
		index = 1;
	}
	word.x_axis = 183;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
}

//详情轨迹界面
void gui_sport_detail_track_paint(uint8_t backgroundcolor)
{
   SetWord_t word = {0};
  
  
    //设置背景黑色
     LCD_SetBackgroundColor(backgroundcolor);

    if (backgroundcolor == LCD_BLACK)
    {
        //黑底白字
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //白底黑字
        word.forecolor = LCD_BLACK;
    }
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Up_12X8);

    word.x_axis = 25;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
   // word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][12],&word);
    //保存轨迹显示
    DrawSaveTrack();
 

    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Down_12X8);  
}

void gui_sport_pause_swich(void)
{
	if (Get_PauseSta()==false)
	{
		g_sport_status = SPORT_STATUS_PAUSE;
		Set_PauseSta(true);
	}
	else
	{
		g_sport_status = SPORT_STATUS_START;
		Set_PauseSta(false);
	}
	
	switch(Save_Detail_Index)
	{
		case DISPLAY_SCREEN_RUN:
			Save_Detail_Index = DISPLAY_SCREEN_RUN_PAUSE;
			break;
		case DISPLAY_SCREEN_RUN_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_RUN;
			break;
		case DISPLAY_SCREEN_WALK:
			Save_Detail_Index = DISPLAY_SCREEN_WALK_PAUSE;
			break;
		case DISPLAY_SCREEN_WALK_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_WALK;
			break;
		case DISPLAY_SCREEN_MARATHON:
			Save_Detail_Index = DISPLAY_SCREEN_MARATHON_PAUSE;
			break;
		case DISPLAY_SCREEN_MARATHON_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_MARATHON;
			break;
		case DISPLAY_SCREEN_INDOORRUN:
			Save_Detail_Index = DISPLAY_SCREEN_INDOORRUN_PAUSE;
			break;
		case DISPLAY_SCREEN_INDOORRUN_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_INDOORRUN;
			break;
		case DISPLAY_SCREEN_CYCLING:
			Save_Detail_Index = DISPLAY_SCREEN_CYCLING_PAUSE;
			break;
		case DISPLAY_SCREEN_CYCLING_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_CYCLING;
			break;
		case DISPLAY_SCREEN_SWIMMING:
			Save_Detail_Index = DISPLAY_SCREEN_SWIM_PAUSE;
			break;
		case DISPLAY_SCREEN_SWIM_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_SWIMMING;
			break;
	}
}

void gui_sport_pause_hint_btn_evt(uint32_t Key_Value)
{
		switch(Key_Value)
		{
			case (KEY_OK):
			{		
				gui_sport_pause_swich();
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}break; 			
		}

}


#endif*/

//#if defined WATCH_SIM_SPORT

void gui_sport_pause_hint_paint(void)
{
	SetWord_t word = {0};

	//底圈
	//LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, LCD_BLACK, 45, LCD_FILL_ENABLE);
	//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, 5, 0, 360, LCD_WHITE);
	//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 40, 1, 0, 360, LCD_BLACK);
	
	
	if (Get_PauseSta()== false)
	{
		//开始
		word.x_axis = LCD_CENTER_ROW - img_sport_start.height/2;
		word.y_axis = LCD_CENTER_LINE+5 - img_sport_start.width/2;
		LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, LCD_FORESTGREEN, 45, LCD_FILL_ENABLE);
	    LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, 5, 0, 360, LCD_FORESTGREEN);
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_WHITE,LCD_NONE,&img_sport_start);
		//暂时增加红色圆圈，表示暂停状态
		//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_BLACK);
		
	}
	else 
	{
		//暂停
		word.x_axis = LCD_CENTER_ROW - img_sport_pause.height/2;
		word.y_axis = LCD_CENTER_LINE - img_sport_pause.width/2;
		LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, LCD_MAROON, 45, LCD_FILL_ENABLE);
	    LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, 5, 0, 360, LCD_MAROON);
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_WHITE,LCD_NONE,&img_sport_pause);//40X45

		//暂时增加红色圆圈，表示暂停状态
		//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 5, 0, 360, LCD_RED);
	}

}

void gui_sport_location_ok_data_paint(uint8_t type,uint8_t index)
	{
		if(type!=5)//非游泳(0~4:跑步，骑行，登山，徒步越野，越野跑)
		{
		  switch(index)
			{
			case 0:
			  gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
			  break;
			  case 1:
				
				gui_sport_heart_paint(1);
				break;
				case 2:
					   if(type==0)
						{
						gui_sport_run_track_paint();
						}
					   else if(type==1)
						{
						gui_sport_cycling_track_paint();
						}
						else if(type==2)
						{
						gui_sport_climbing_track_paint();
						}
						 else if(type==3)
						{
						
						 gui_sport_hike_track_paint();
							}
						  else if(type==4)
						{
						gui_sport_crosscountry_track_paint();
						}
						else;
					
					break;
					default:
						break;
			
			}
	
		}
	
		else//游泳
		{
		   switch(index)
			{
			 case 0:
			  gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
			  break;
			  case 1:
				
				gui_sport_swim_heart_paint();
				break;
				default:
					break;
					
	
			}
	
	
		}
	
	
	}


void gui_sport_pause_and_data_paint(uint8_t type,uint8_t index)
{
	if(type!=5)//非游泳(0~4:跑步，骑行，登山，徒步越野，越野跑)
	{
      switch(index)
      	{
      	case 0:
	      gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
		  break;
		  case 1:
		  	
			gui_sport_heart_paint(1);
		  	break;
			case 2:
                   if(type==0)
                   	{
                    gui_sport_run_track_paint();
				    }
				   else if(type==1)
				   	{
				   	gui_sport_cycling_track_paint();
				   	}
				    else if(type==2)
				   	{
				   	gui_sport_climbing_track_paint();
				   	}
					 else if(type==3)
				   	{
				   	
					 gui_sport_hike_track_paint();
					 	}
					  else if(type==4)
				   	{
				   	gui_sport_crosscountry_track_paint();
				   	}
                    else;
				
				break;
				default:
					break;
       	
      	}

	}

	else//游泳
	{
       switch(index)
       	{
         case 0:
	      gui_sport_data_display_preview_paint(SPORT_DATA_SHOW);
		  break;
		  case 1:
		  	
			gui_sport_swim_heart_paint();
		  	break;
			default:
				break;
				

	    }


	}
gui_sport_pause_hint_paint();


}

void gui_vice_pause_hint_paint(void)
{
	#ifdef COD 
	SetWord_t word = {0};

	//底圈
	LCD_SetCircle(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, LCD_BLACK, 45, LCD_FILL_ENABLE);
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_ROW, 45, 5, 0, 360, LCD_WHITE);
	
	
	if (cod_user_sport_cast.sport_status==COD_SPORT_STATUS_PAUSE)
	{
		//暂停
		word.x_axis = LCD_CENTER_ROW - img_sport_pause.height/2;
		word.y_axis = LCD_CENTER_LINE - img_sport_pause.width/2;
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_RED,LCD_NONE,&img_sport_pause);//40X45

	}
	else
	{
		//开始
		word.x_axis = LCD_CENTER_ROW - img_sport_start.height/2;
		word.y_axis = LCD_CENTER_LINE+5 - img_sport_start.width/2;
		LCD_SetPicture(word.x_axis,word.y_axis,LCD_GREEN,LCD_NONE,&img_sport_start);
		
	}

#endif
}


//运动暂停计时
void gui_sport_pause_time(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	uint32_t second;

	//计算总时长
	second = PauseTime.Pause_Stop_Time.Hour*60*60 + PauseTime.Pause_Stop_Time.Minute*60 + PauseTime.Pause_Stop_Time.Second;
	
	if(second >= 6000)
	{
		//暂停时长不超过99:59
		second = 5999;
	}
	
	memset(str,0,sizeof(str));
	sprintf(str,"%d:%02d",second/60,second%60);
	
	//已暂停
	word.x_axis = 20;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - 48 - 5 - strlen(str)*(Font_Number_19.width+1))/2;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportsPromptStrs[SetValue.Language][1],&word);

	//暂停时长
	word.y_axis += 48 + 6;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
}

//运动暂停活动计时
void gui_sport_pause_acttime(void)
{
	SetWord_t word = {0};
	char str[10] = {0};
	
	word.x_axis = 39+16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	memset(str,0,sizeof(str));
	if(ActivityData.Act_Stop_Time.Hour > 0)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",ActivityData.Act_Stop_Time.Hour, ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);	
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",ActivityData.Act_Stop_Time.Minute, ActivityData.Act_Stop_Time.Second);
	}
	LCD_SetNumber(str, &word);
}

//放弃菜单选项
void gui_sport_cancel_options(uint8_t index)
{
	SetWord_t word = {0};
	
	//菜单第一项
	word.x_axis = 133;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
	
	//菜单第二项
	if(index == 1)
	{
		index = 0;
	}
	else
	{
		index = 1;
	}
	word.x_axis = 183;
    word.forecolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)CancelStrs[SetValue.Language][index],&word);
}

//详情轨迹界面
void gui_sport_detail_track_paint(uint8_t backgroundcolor)
{
   SetWord_t word = {0};
  
  
    //设置背景黑色
     LCD_SetBackgroundColor(backgroundcolor);

    if (backgroundcolor == LCD_BLACK)
    {
        //黑底白字
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //白底黑字
        word.forecolor = LCD_BLACK;
    }
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Up_12X8);

    word.x_axis = 25;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_16_SIZE;
   // word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
    LCD_SetString((char *)SportDetailStrs[SetValue.Language][12],&word);
    //保存轨迹显示
    DrawSaveTrack();
 

 //   LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, word.forecolor, LCD_NONE, &Img_Pointing_Down_12X8);  
 
}

//所有运动(除游泳)心率界面
void gui_sport_heart_paint(uint8_t flag)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_WHITE);
	//圆点索引
    gui_sport_index_circle_vert_paint(1, 3);
	
	//心率值
	gui_sport_heart_1_paint(Get_Sport_Heart(),flag);

}




//运动心率界面
void gui_sport_heart_1_paint(uint8_t heart,uint8_t type)
{
  
   am_hal_rtc_time_t RTC_time;
	SetWord_t word = {0};
	char str[30];
   uint8_t hdrRange,i;
	if(type == 1)
	{
         
		 //gps状态
	   	if((g_sport_status == SPORT_STATUS_PAUSE)&&(ScreenState!=DISPLAY_SCREEN_REMIND_PAUSE))
			{
				//表示暂停状态
		        LCD_SetPicture(110,200,LCD_MAROON,LCD_NONE,&img_sport_pause_cod);//右上角箭头,指示ok键
				
				//gui_sport_gps_status(SPORT_GPS_RIGHT_UP_6);
			}
		else
			{
		     gui_sport_gps_status(SPORT_GPS_RIGHT_UP_6);
			}
	}
/*区间名字*/
   	word.x_axis = 50 + Img_heartrate_32X28.height/2 - 16/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
    word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	word.size = LCD_FONT_16_SIZE;
	hdrRange = get_hdr_range(heart);


   if(hdrRange>0)
   	{


  LCD_SetString( (char *)heartzonegodstrs[SetValue.Language][hdrRange-1],&word);


   }
   else;
	for(i=1;i<6;i++)
		{
		if(i!=hdrRange)
			{
	         LCD_SetArc(120,120,122,9,270-45/2+(i-1)*45,270-45/2+i*45,hdr_color_value[i]);
			}
	    else
	    	{

             
	         LCD_SetArc(120,120,122,14,270-45/2+(i-1)*45,270-45/2+i*45,hdr_color_value[i]);

		   }
		}

	
	//心率图标
	word.x_axis = 50;
	if(hdrRange>0)
	 {
      
	  word.y_axis = 120 - 2*16 - 4 - 32;
	  }
	else
		{
         word.y_axis = LCD_CENTER_JUSTIFIED;
	   }
    word.forecolor = get_hdr_range_color(heart);
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
    LCD_SetPicture(word.x_axis, word.y_axis, word.forecolor, LCD_NONE, &Img_heartrate_32X28);

	/*心率值*/
	word.x_axis = 120 - Font_Number_56.height/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
    word.forecolor = LCD_BLACK;


   //无心率
	if(heart == 0)
	{	
		LCD_SetRectangle(119,3,93,25,LCD_BLACK,0,0,LCD_FILL_ENABLE);

		LCD_SetRectangle(119,3,122,25,LCD_BLACK,0,0,LCD_FILL_ENABLE);
	}
	else
	{
		memset(str,0,10);
		sprintf(str,"%d",heart);
		LCD_SetNumber(str,&word);
		
   }
	
	am_hal_rtc_time_get(&RTC_time);
	#ifdef HRT_DEBUG_TEST		
	extern uint8_t acc_datalen;
    extern uint8_t si11_datalen;
	extern uint32_t hrt_int_num;
	extern uint32_t hrt_int_count;
    extern uint32_t hrt_int_count_callback;


	memset(str,0,30);
	
    sprintf(str,"%d:%d:%d:%d",acc_datalen,si11_datalen,hrt_int_count,hrt_int_count_callback);
    word.x_axis = 175;
	word.y_axis = LCD_CENTER_JUSTIFIED;
    word.forecolor =LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	
	word.size = LCD_FONT_13_SIZE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	memset(str,0,30);
	
    sprintf(str,"%d:%d:%d",OHRL_data.amb,OHRL_data.ppg,hrt_int_num );
	#else
	memset(str,0,30);
    sprintf(str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
	#endif
    //word.x_axis = 210;
	
    word.x_axis = 208;
	word.y_axis = LCD_CENTER_JUSTIFIED;
    word.forecolor =LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	
	//word.size = LCD_FONT_13_SIZE;
	
	word.size = LCD_FONT_19_SIZE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	//电量
//	gui_sport_battery_paint();
	
}

//飞腕只一栏实时时间界面
void gui_sport_realtime_paint(uint8_t type)
{

	SetWord_t word = {0};
	char str[10];

	if(type == 1)
	{
		//gps状态
		gui_sport_gps_status(SPORT_GPS_LEFT1);
	}
	
	am_hal_rtc_time_get(&RTC_time);
	
	//实时时间
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.x_axis = 92;
	word.size = LCD_FONT_48_SIZE;

	memset(str,0,10);
	sprintf(str,"%02d:%02d",RTC_time.ui32Hour,RTC_time.ui32Minute);
	LCD_SetNumber(str,&word);

	//电量
	gui_sport_battery_paint();
	
}

 void sport_ready_menu( char* menu1, char* menu2, char* menu3)
{
  //  LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	//LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	//上翻提示
	gui_page_up(LCD_PERU);
		//下翻提示
	gui_page_down(LCD_PERU);
		SetWord_t word = {0};
		uint8_t progress_pre=0;
		
		int32_t progress_count;

		progress_count = (location_timer_count - progress_pre_timer);
		if(progress_count<0)
			{

       progress_count=0;

		}
		/* word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	     word.y_axis = LCD_CENTER_JUSTIFIED;
	     word.size = LCD_FONT_16_SIZE;
	     word.forecolor = LCD_LIGHTGRAY;
  	     word.bckgrndcolor = LCD_NONE;*/
		 word.size = LCD_FONT_16_SIZE;
		 word.bckgrndcolor = LCD_NONE;
		 word.forecolor = LCD_WHITE;
		 word.kerning = 1;
		 word.x_axis = 36;	
		 word.y_axis = LCD_CENTER_JUSTIFIED;
  		 LCD_SetString(menu1,&word);
	 
		 if((ScreenSportSave != DISPLAY_SCREEN_INDOORRUN)
			 && (ScreenSportSave != DISPLAY_SCREEN_SWIMMING))
		 {
			 //gps状态
			 gui_sport_gps_status(SPORT_GPS_RIGHT_UP_COD);
			  word.x_axis = 120 + 2;	
			 word.y_axis = 20 + img_gps_god.width/2 - 1.5*8;
			 word.forecolor = LCD_WHITE;
		     word.bckgrndcolor = LCD_NONE;
			 
			 word.size = LCD_FONT_16_SIZE;
			 word.kerning = 0;
			 LCD_SetString("GPS",&word);
		 }
		


		 
		 word.size = LCD_FONT_24_SIZE;
		 word.x_axis = 108;
		 word.y_axis = LCD_CENTER_JUSTIFIED;
		 word.kerning = 1;
		 LCD_SetString(menu2,&word);
	 
		 //开始指示
		 if(m_sport_ready_index==GUI_SPORT_READY_INDEX_START)
		 	{
			 word.x_axis = 103;
			 word.y_axis = 120 + 24 + 16 + 8 + 12;
			 
			 if (GetGpsStatus()== true)
			 	{
                
				LCD_SetPicture(word.x_axis,word.y_axis,LCD_GREEN,LCD_NONE,&img_sportready_start);
			   }
			 else
			 	{
			 	
				LCD_SetPicture(word.x_axis,word.y_axis,LCD_LIGHTGRAY,LCD_NONE,&img_sportready_start);

			 }
		 	}
		 
		 //白色围框
		 LCD_SetRectangle(79,2,0,180,LCD_WHITE,0,0,LCD_FILL_ENABLE);
		 LCD_SetRectangle(159,2,0,180,LCD_WHITE,0,0,LCD_FILL_ENABLE);
		 LCD_SetArc(120, 179, 41, 2, 0, 180, LCD_WHITE);


		if(gps_located == 1)
			{
             LCD_SetRectangle(78,4,0,180,LCD_GREEN,0,0,LCD_FILL_ENABLE);
		 LCD_SetRectangle(158,4,0,180,LCD_GREEN,0,0,LCD_FILL_ENABLE);
		 LCD_SetArc(120, 179, 42, 4, 0, 180, LCD_GREEN);

		   }
		else if(m_GPS_data.possl_gpsnum==0)

			{
			    progress_pre = (progress_count*2)>=24?24:(progress_count*2);
				  if(progress_pre==24)
				  	{
                      
					  LCD_SetRectangle(78,4,0,180,LCD_GREEN,0,0,LCD_FILL_ENABLE);
				   }
				  else
				  	{
                 LCD_SetRectangle(78,4,0,180*progress_pre*0.01*4.16,LCD_GREEN,0,0,LCD_FILL_ENABLE);
				  	}



		    }
		else if(m_GPS_data.possl_gpsnum==1)
			{
              progress_pre = (25+progress_count*2)>=49?49:(25+progress_count*2);
				 
				 LCD_SetRectangle(78,4,0,180,LCD_GREEN,0,0,LCD_FILL_ENABLE);
				 if(progress_pre==49)
				 	{
                    
					LCD_SetArc(120, 179, 42, 4, 0, 180, LCD_GREEN);
				  }
				 else
				 	{
		         LCD_SetArc(120, 179, 42, 4, 0, 90*(progress_pre-25)*0.01*4.16, LCD_GREEN);
				 	}

		   }
		else if(m_GPS_data.possl_gpsnum==2)
			{
                progress_pre = (50+progress_count*2)>=74?74:(50+progress_count*2);
				  
                 LCD_SetRectangle(78,4,0,180,LCD_GREEN,0,0,LCD_FILL_ENABLE);
				 
		         LCD_SetArc(120, 180, 42, 4, 0, 90, LCD_GREEN);
				 if(progress_pre==74)
				 	{

                      
					  LCD_SetArc(120, 179, 42, 4, 90, 90+90, LCD_GREEN);
				    }
				 else
				 	{
		         LCD_SetArc(120, 179, 42, 4, 90, 90+90*(progress_pre-50)*0.01*4.16, LCD_GREEN);
				 	}

		   }
		else if(m_GPS_data.possl_gpsnum==3)
			{
            	  progress_pre =  (75+progress_count*2)>=99?99:(75+progress_count*2);
				 
                 LCD_SetRectangle(78,4,0,180,LCD_GREEN,0,0,LCD_FILL_ENABLE);
				 
		         LCD_SetArc(120, 179, 42, 4, 0, 180, LCD_GREEN);
				 if(progress_pre==99)
				 	{
                      
					  LCD_SetRectangle(158,4,14,180-14,LCD_GREEN,0,0,LCD_FILL_ENABLE);

				    }
				 else
				 	{
		         LCD_SetRectangle(158,4,(1-(progress_pre-75)*0.01*4)*180,(progress_pre-75)*0.01*4*180,LCD_GREEN,0,0,LCD_FILL_ENABLE);
				 	}
		   }
			
		 	else if(m_GPS_data.possl_gpsnum>=4)
		 		{
				  progress_pre = 99;
                 LCD_SetRectangle(78,4,0,180,LCD_GREEN,0,0,LCD_FILL_ENABLE);
				 
		         LCD_SetArc(120, 179, 42, 4, 0, 180, LCD_GREEN);
		         LCD_SetRectangle(158,4,14,180-14,LCD_GREEN,0,0,LCD_FILL_ENABLE);

			   }

			 
	  /*  GUI_SPORT_PRINTF("gpsnum:%d,location_timer_count:%d,progress_pre_timer:%d,progress_count:%d,progress_pre:%d\r\n",


		m_GPS_data.possl_gpsnum, location_timer_count,progress_pre_timer,progress_count,progress_pre);*/
		/* word.x_axis = 120 + 24/2 + 36 + 24;
		 word.y_axis = LCD_CENTER_JUSTIFIED;
		 word.kerning = 1;
         word.forecolor = LCD_LIGHTGRAY;
	   word.size = LCD_FONT_16_SIZE;*/
	   word.size = LCD_FONT_16_SIZE;
	   word.x_axis = 180;
	   word.y_axis = LCD_CENTER_JUSTIFIED;
	   word.kerning = 1;
		 LCD_SetString(menu3,&word);






}
//运动准备界面绘制
void gui_sport_ready_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
 
 switch(m_sport_ready_index)

	{
	case GUI_SPORT_READY_INDEX_DISPLAY:

		sport_ready_menu((char *)SportReadyStrs[SetValue.Language][2],(char *)SportReadyStrs[SetValue.Language][0],(char *)SportReadyStrs[SetValue.Language][1]);
		break;


	case GUI_SPORT_READY_INDEX_START:
		sport_ready_menu((char *)SportReadyStrs[SetValue.Language][0],(char *)SportReadyStrs[SetValue.Language][1],(char *)SportReadyStrs[SetValue.Language][2]);
			break;

	case GUI_SPORT_READY_INDEX_SET:
		
		sport_ready_menu((char *)SportReadyStrs[SetValue.Language][1],(char *)SportReadyStrs[SetValue.Language][2],(char *)SportReadyStrs[SetValue.Language][0]);
		break;



	}	
}






//数据显示平均配速
void gui_sport_data_display_ave_pace_paint(uint8_t state,uint16_t pace)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d:%02d",(pace/60),(pace%60));
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("/km",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}
	


}

//数据显示最佳配速
void gui_sport_data_display_optimum_pace_paint(uint8_t state,uint16_t pace)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d:%02d",(pace/60),(pace%60));
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("/km",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][5]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][5],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][5]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][5],&word);
	}
	


}

//数据显示实时心率
void gui_sport_data_display_realtime_heart_paint(uint8_t state,uint8_t heart)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d",heart);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("bpm",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
	


}

//数据显示平均心率
void gui_sport_data_display_ave_heart_paint(uint8_t state,uint8_t heart)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d",heart);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("bpm",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}
	


}

//数据显示实时步频
void gui_sport_data_display_realtime_frequency_paint(uint8_t state,uint16_t	frequency)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d",frequency);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("spm",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}

}

//数据显示平均步频
void gui_sport_data_display_ave_frequency_paint(uint8_t state,uint16_t frequency)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d",frequency);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("spm",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}


}

//数据显示卡路里
void gui_sport_data_display_calorie_paint(uint8_t state,uint32_t calorie)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d",calorie/1000);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=40;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=20;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("kcal",&word);

}

//数据显示上一圈配速
void gui_sport_data_display_last_pace_paint(uint8_t state,uint16_t pace)
{
	SetWord_t word = {0};
	char str[10];
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	//上一圈时间文字
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 20;
		
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 83;

	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 205;

	}

	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][SPORT_DATA_DISPALY_LAST_PACE],&word);

	//上一圈时间数据
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 40;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 101;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 165;
		word.size = LCD_FONT_32_SIZE;

	}
	
	memset(str,0,10);
	sprintf(str,"%d:%02d",(pace/60),(pace%60));
	LCD_SetNumber(str,&word);

}
//数据显示上一圈时间
void gui_sport_data_display_last_time_paint(uint8_t state,uint32_t time)
{
	SetWord_t word = {0};
	char str[10];
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	//上一圈时间文字
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 20;
		
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 83;

	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 205;

	}

	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][SPORT_DATA_DISPALY_LAST_TIME],&word);

	//上一圈时间数据
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 40;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 101;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 165;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	if(time >= 3600)
	{
					//时分秒
		sprintf(str,"%d:%02d:%02d",time/3600, time/60%60, time%60);
	}
	else
	{
					//分秒
		sprintf(str,"%d:%02d",time/60%60, time%60);
	}		
	LCD_SetNumber(str,&word);

}


//数据显示平均速度
void gui_sport_data_display_ave_speed_paint(uint8_t state,uint16_t speed)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",speed/10,speed%10);
	#else
	sprintf(str,"%.1f",speed/10.f);		//?????????
	#endif
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("km/h",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}
	

}

//数据显示最佳速度
void gui_sport_data_display_optimum_speed_paint(uint8_t state,uint16_t speed)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",speed/10,speed%10);
	#else
	sprintf(str,"%.1f",speed/10.f);		//?????????
	#endif
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("km/h",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][5]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][5],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][5]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][5],&word);
	}
	

}

//数据显示实时踏频
void gui_sport_data_display_realtime_cadence_paint(uint8_t state,uint32_t cadence)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d",cadence);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("rpm",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][4]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][4],&word);
	}
	

}

//数据显示平均踏频
void gui_sport_data_display_ave_cadence_paint(uint8_t state,uint32_t cadence)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	uint8_t charlen;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d",cadence);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("rpm",&word);
	word.kerning = 1;
	if (state == SPORT_DATA_DISPALY_INDEX_MID)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120-((len*26)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}
	else if(state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_DOWN)
	{
		charlen = strlen(SportReadyStrs[SetValue.Language][3]);
		word.y_axis =120 -((len*16)/2 + charlen *8+5);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][3],&word);
	}

}

//数据显示平均泳速
void gui_sport_data_display_ave_swim_pace_paint(uint8_t state,uint16_t pace)
{
		SetWord_t word = {0};
		char str[10];
		uint8_t len;
		word.bckgrndcolor = LCD_NONE;
		word.forecolor = LCD_WHITE;
		word.kerning = 1;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
		{
			word.x_axis = 34;
			word.size = LCD_FONT_32_SIZE;
		}
		else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
		{
			word.x_axis = 92;
			word.size = LCD_FONT_48_SIZE;
		}
		else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
		{
			word.x_axis = 170;
			word.size = LCD_FONT_32_SIZE;
	
		}
		memset(str,0,10);
	
		sprintf(str,"%d:%02d",(pace/60),(pace%60));
		len = strlen(str);
		LCD_SetNumber(str,&word);
		
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
		{
			word.x_axis +=38;
			word.y_axis =120+(len*26)/2 + 5;
		}
		else
		{
			word.x_axis +=18;
			word.y_axis =120+(len*16)/2 + 5;
		}
		
		LCD_SetString("/100m",&word);

}

//数据显示趟数
void gui_sport_data_display_swim_laps_paint(uint8_t state,uint16_t laps)
{
	SetWord_t word = {0};
	char str[10];
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	//趟数文字
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 20;
		
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 83;

	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 205;

	}

	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][SPORT_DATA_DISPALY_SWIM_LAPS],&word);

	//趟数数据
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 40;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 101;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 165;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	
	sprintf(str,"%d",laps);
			
	LCD_SetNumber(str,&word);

}

//数据显示总划次
void gui_sport_data_display_total_strokes_paint(uint8_t state,uint32_t strokes)
{
	SetWord_t word = {0};
	char str[10];
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	//总划数文字
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 20;
		
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 83;

	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 205;

	}

	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][SPORT_DATA_DISPALY_TOTAL_STROKES],&word);

	//总划数数据
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 40;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 101;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 165;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	
	sprintf(str,"%d",strokes);
			
	LCD_SetNumber(str,&word);

}

//数据显示平均划次
void gui_sport_data_display_ave_strokes_paint(uint8_t state,uint32_t strokes)
{
	SetWord_t word = {0};
	char str[10];
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	//平均划数文字
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 20;
		
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 83;

	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 205;

	}

	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][SPORT_DATA_DISPALY_AVE_STROKES],&word);

	//平均划数数据
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 40;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 101;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 165;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	
	sprintf(str,"%d",strokes);
			
	LCD_SetNumber(str,&word);

}

//数据显示平均swlof
void gui_sport_data_display_ave_swlof_paint(uint8_t state,uint16_t swlof)
{
	SetWord_t word = {0};
	char str[10];
	
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	//平均swolf文字
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 20;
		
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 83;

	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 205;

	}

	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][SPORT_DATA_DISPALY_AVE_SWOLF],&word);

	//平均swolf数据
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 40;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 101;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN ||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 165;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);
	
	sprintf(str,"%d",swlof);
			
	LCD_SetNumber(str,&word);	

}

//数据显示平均划频
void gui_sport_data_display_ave_swim_frequency_paint(uint8_t state,uint16_t frequency)
{
	SetWord_t word = {0};
	char str[10];
	uint8_t len;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_WHITE;
	word.kerning = 1;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if (state == SPORT_DATA_DISPALY_INDEX_UP || state == SPORT_DATA_DISPALY_INDEX_UP_CIRCLE)
	{
		word.x_axis = 34;
		word.size = LCD_FONT_32_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis = 92;
		word.size = LCD_FONT_48_SIZE;
	}
	else if (state == SPORT_DATA_DISPALY_INDEX_DOWN||state == SPORT_DATA_DISPALY_INDEX_DOWN_NOHINT)
	{
		word.x_axis = 170;
		word.size = LCD_FONT_32_SIZE;

	}
	memset(str,0,10);

	sprintf(str,"%d",frequency);
	len = strlen(str);
	LCD_SetNumber(str,&word);
	
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 0;
	if (state == SPORT_DATA_DISPALY_INDEX_MID ||state == SPORT_DATA_DISPALY_INDEX_MID_GPS)
	{
		word.x_axis +=38;
		word.y_axis =120+(len*26)/2 + 5;
	}
	else
	{
		word.x_axis +=18;
		word.y_axis =120+(len*16)/2 + 5;
	}
	
	LCD_SetString("spm",&word);


}


//飞腕数据显示项
/*void gui_sport_data_display_detail_paint(uint8_t state ,uint8_t m_sport_data_display)
{
   	switch(m_sport_data_display)
   	{

	
		case SPORT_DATA_DISPALY_TOTAL_TIME:
			gui_sport_data_display_total_time_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_TOTAL_DISTANCE:
			
			gui_sport_data_display_total_distance_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_REALTIME_PACE:
			gui_sport_data_display_realtime_pace_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_PACE:
			gui_sport_data_display_ave_pace_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_OPTIMUM_PACE:
			gui_sport_data_display_optimum_pace_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_REALTIME_HEART:
			gui_sport_data_display_realtime_heart_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_HEART:
			gui_sport_data_display_ave_heart_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_REALTIME_FREQUENCY:
			gui_sport_data_display_realtime_frequency_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_FREQUENCY:
			gui_sport_data_display_ave_frequency_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_CALORIE:
			gui_sport_data_display_calorie_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_LAST_PACE:
			gui_sport_data_display_last_pace_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_LAST_TIME:
			gui_sport_data_display_last_time_paint(state,0);
			break;	
		case SPORT_DATA_DISPALY_REALTIME_SPEED:
			gui_sport_data_display_realtime_speed_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_SPEED:
			gui_sport_data_display_ave_speed_paint(state,0);
			break;
	    case SPORT_DATA_DISPALY_OPTIMUM_SPEED:
			gui_sport_data_display_optimum_speed_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_REALTIME_CADENCE:
			gui_sport_data_display_realtime_cadence_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_CADENCE:
			gui_sport_data_display_ave_cadence_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_SWIM_PACE:
			gui_sport_data_display_ave_swim_pace_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_SWIM_LAPS:
			gui_sport_data_display_swim_laps_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_TOTAL_STROKES:
			gui_sport_data_display_total_strokes_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_STROKES:
			gui_sport_data_display_ave_strokes_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_SWOLF:
			gui_sport_data_display_ave_swlof_paint(state,0);
			break;
		case SPORT_DATA_DISPALY_AVE_SWIM_FREQUENCY:
			gui_sport_data_display_ave_swim_frequency_paint(state,0);
			break;
		default:

		
			break;
	}
}*/
#ifdef COD 
void gui_app_sport_data_show_str(uint8_t type,char *str)
	{   
	   uint16_t len;
	   len = strlen(str);
	   memset(str,0,len);
	   uint32_t show_value;
	   switch(type)
			{
			  case SPORT_DATA_DISPALY_TOTAL_DISTANCE:
				   show_value =cod_user_sport_cast.distance;
				   
				   sprintf(str,"%d.%02d",(show_value/1000),show_value/10%100);
			 break;
				
			case SPORT_DATA_DISPALY_TOTAL_TIME:
					
				   show_value = cod_user_sport_cast.time;
					
				
				if(show_value >= 3600)
				   {
						//时分秒
					sprintf(str,"%d:%02d:%02d",show_value/3600, show_value/60%60, show_value%60);
					}
					else
					{
						//分秒
					 sprintf(str,"%d:%02d",show_value/60%60, show_value%60);
				   }	
			break;
	
		
			
			case SPORT_DATA_DISPALY_AVE_PACE:
					show_value = cod_user_sport_cast.pace;
			
				 sprintf(str,"%d:%02d",(show_value/60),(show_value%60));
	
			break;
			
			case SPORT_DATA_DISPALY_TOTAL_STEP:
					if(ScreenSportSave == DISPLAY_SCREEN_RUN)
					{
				   
				   show_value =  cod_user_sport_cast.step;
				   
					}
			
				  
				  sprintf(str,"%d",show_value);
			break;
			
	
			case SPORT_DATA_DISPALY_REALTIME_HEART:
				  show_value   = cod_user_sport_cast.heart_rate;
				  
				  sprintf(str,"%d",show_value);
			break;
	
			
			
				case SPORT_DATA_DISPALY_REALTIME_SPEED:
					
						show_value	 = cod_user_sport_cast.rt_speed *100;
				 // sprintf(str,"%.2f",show_value/100.f);
				  sprintf(str,"%d.%02d",(show_value/100),show_value%100);
			break;
	
				case SPORT_DATA_DISPALY_ALTITUDE://海拔高度
				  show_value   = cod_user_sport_cast.altitude;
				  
				  sprintf(str,"%d",show_value);
			break;
		
	
		   default:
			break;
				}
	
	
	
	}

#endif

/*根据不同的数据项取对应的值;如需增加单位加上LCD_SetString;同种变量区分不同运动项目的值*/
void gui_sport_data_show_str(uint8_t sport_item,char *str)
{
   
   uint16_t len;
   len = strlen(str);
   memset(str,0,len);
   uint32_t show_value;
   int32_t show_int_value;
   switch(sport_item)
		{
          case SPORT_DATA_DISPALY_TOTAL_DISTANCE:
		  	if(ScreenSportSave == DISPLAY_SCREEN_RUN)
		  		{
		       show_value = ActivityData.ActivityDetails.RunningDetail.Distance/1000*1000;
		  		}
			else if(ScreenSportSave == DISPLAY_SCREEN_SWIMMING)
				{ 
                 
				 show_value = ActivityData.ActivityDetails.SwimmingDetail.Distance/100*100;
			    }
			else if(ScreenSportSave == DISPLAY_SCREEN_CYCLING)
				{
                  
				  show_value = ActivityData.ActivityDetails.CyclingDetail.Distance/1000*1000;
			    }
              	else if((ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY_HIKE)||(ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY))
				{
                  
				  show_value = ActivityData.ActivityDetails.CrosscountryDetail.Distance/1000*1000;
			    }
			else if(ScreenSportSave == DISPLAY_SCREEN_CLIMBING)
				{
                  
				  show_value = ActivityData.ActivityDetails.ClimbingDetail.Distance/1000*1000;
			    }
		       sprintf(str,"%0.2f",show_value/100000.f);
		 break;
			
	    case SPORT_DATA_DISPALY_TOTAL_TIME:
				
		       show_value = ActivityData.ActTime;
			if(show_value >= 3600)
	           {
					//时分秒
		        sprintf(str,"%d:%02d:%02d",show_value/3600, show_value/60%60, show_value%60);
	            }
	            else
	            {
					//分秒
		         sprintf(str,"%d:%02d",show_value/60%60, show_value%60);
	           }	
		break;
		
		case SPORT_DATA_DISPALY_AVE_PACE:

				if(ScreenSportSave == DISPLAY_SCREEN_RUN)
		  		{
		       show_value = CalculateSpeed(ActivityData.ActTime,0,ActivityData.ActivityDetails.RunningDetail.Distance,ACT_RUN);
		  		}
			else if(ScreenSportSave == DISPLAY_SCREEN_SWIMMING)
				{ 
                 
				 show_value = CalculateSpeed(ActivityData.ActTime,0,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);
			    }
			/*else if(ScreenSportSave == DISPLAY_SCREEN_CYCLING)//骑行无平均配速
				{
                  
				  show_value = ActivityData.ActivityDetails.RunningDetail.Distance;
			    }*/
			    	else if((ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY_HIKE)||(ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY))
				{
                  
				  show_value = CalculateSpeed(ActivityData.ActTime,0,ActivityData.ActivityDetails.CrosscountryDetail.Distance,ACT_HIKING);
			    }
			sprintf(str,"%d:%02d",(show_value/60),(show_value%60));

		break;
		
		case SPORT_DATA_DISPALY_REALTIME_PACE:
		     	if(ScreenSportSave == DISPLAY_SCREEN_RUN)
		  		{
		        show_value = ActivityData.ActivityDetails.RunningDetail.Pace;
		  		}
				 else if((ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY_HIKE)||(ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY))
				{
				  show_value = ActivityData.ActivityDetails.CrosscountryDetail.Speed;
			    }
			/*else if(ScreenSportSave == DISPLAY_SCREEN_SWIMMING)
				{ 
                 
				 show_value = ActivityData.ActivityDetails.SwimmingDetail.Speed;//测试先用平均配速代替
			    }
			else if(ScreenSportSave == DISPLAY_SCREEN_CYCLING)
				{
                  
				  show_value = ActivityData.ActivityDetails.RunningDetail.Distance;
			    }*/
			 sprintf(str,"%d:%02d",(show_value/60),(show_value%60));

		break;
		
		case SPORT_DATA_DISPALY_TOTAL_STEP:
				if(ScreenSportSave == DISPLAY_SCREEN_RUN)
		  		{
		       
			  show_value =  ActivityData.ActivityDetails.RunningDetail.Steps;
		       
		  		}
				 else if((ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY_HIKE)||(ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY))
				{
				  show_value =	ActivityData.ActivityDetails.CrosscountryDetail.Steps;
				
			    }
		
			  
			  sprintf(str,"%d",show_value);
		break;
				 /*步频数据*/
         case SPORT_DATA_DISPALY_REALTIME_FREQUENCY:
		  show_value  =  get_steps_freq();

		   	
			sprintf(str,"%d",show_value);
		break;
		case SPORT_DATA_DISPALY_TOTAL_CLIMB:
			if(ScreenSportSave == DISPLAY_SCREEN_RUN)
		  		{
		       
			   show_value = ActivityData.ActivityDetails.RunningDetail.RiseHeight; //咕咚累积爬升只计算了累积的爬升量，不包含下降的
		       
		  		}
			
			else if(ScreenSportSave == DISPLAY_SCREEN_CYCLING)
				{
                  
				  show_value = ActivityData.ActivityDetails.CyclingDetail.RiseHeight;
			    }
			  	else if(ScreenSportSave == DISPLAY_SCREEN_CLIMBING)
				{
                  
				  show_value = ActivityData.ActivityDetails.ClimbingDetail.RiseHeight;
			    }
			  
			  else if((ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY_HIKE)||(ScreenSportSave == DISPLAY_SCREEN_CROSSCOUNTRY))
				{
				  
				  show_value = ActivityData.ActivityDetails.CrosscountryDetail.RiseHeight;
			    }
			 sprintf(str,"%d",show_value);
		break;
        case SPORT_DATA_DISPALY_REALTIME_HEART:
		      show_value   = Get_Sport_Heart();
			  
			  sprintf(str,"%d",show_value);
		break;

		
		case SPORT_DATA_DISPALY_AVE_SPEED:
			if(ScreenSportSave == DISPLAY_SCREEN_CYCLING)
				{
		      show_value   = CalculateSpeed(ActivityData.ActTime,0,ActivityData.ActivityDetails.CyclingDetail.Distance,ACT_CYCLING);
				}
			/*else if(ScreenSportSave == DISPLAY_SCREEN_CLIMBING)
				{
		      show_value   = CalculateSpeed(ActivityData.ActTime,0,ActivityData.ActivityDetails.ClimbingDetail.Distance,ACT_CYCLING);
				}*/
			  sprintf(str,"%.1f",show_value/10.f);
		break;
			case SPORT_DATA_DISPALY_REALTIME_SPEED:
				
				if(ScreenSportSave == DISPLAY_SCREEN_CYCLING)
					{
		            show_value   = ActivityData.ActivityDetails.CyclingDetail.Speed;

					
					}
				
				sprintf(str,"%d.%01d",show_value/10,show_value%10);//km/h

				/*测试上坡里程BUG*/
			//	sprintf(str,"%d.%02d",ActivityData.ActivityDetails.CyclingDetail.UphillDistance/100000,ActivityData.ActivityDetails.CyclingDetail.UphillDistance/1000%100);

			
		break;

			case SPORT_DATA_DISPALY_ALTITUDE://海拔高度
			#ifdef COD 
		      show_int_value   = (int32_t)sport_altitude;
			#else
			show_int_value   = (int32_t)sport_alt;
			  #endif
			  sprintf(str,"%d",show_int_value);
			
		break;
		/*游泳趟数*/
       case SPORT_DATA_DISPALY_SWIM_LAPS:
	   	#ifdef COD 
		      show_value  = ActivityData.ActivityDetails.SwimmingDetail.laps;
			  
			  sprintf(str,"%d",show_value);
		#endif
		break;
		/*上一趟swolf*/
     case  SPORT_DATA_DISPALY_PREVIOUS_SWOLF:
          show_value  =  ActivityData.ActivityDetails.SwimmingDetail.Swolf/10;//测试时先用平均值代替
		   	
			sprintf(str,"%d",show_value);
        break;
		case	SPORT_DATA_DISPALY_AVE_SWIM_FREQUENCY:
                   show_value  =  ActivityData.ActivityDetails.SwimmingDetail.Frequency;
		
		          sprintf(str,"%d",show_value);
		break;

		
		case	SPORT_DATA_DISPALY_AVE_SWIM_PACE://游泳的平均配速
               show_value  =  ActivityData.ActivityDetails.SwimmingDetail.Speed;
		      
			  sprintf(str,"%d:%02d",show_value/60,show_value%60);
		break;

		case  SPORT_DATA_DISPALY_VER_AVE_SPEED:
			
			show_value	=  ActivityData.ActivityDetails.ClimbingDetail.Speed;
			sprintf(str,"%.1f",show_value/10.f);
			break;
	   default:
	   	break;
			}



}

//数据显示(预览)界面
void gui_sport_data_display_preview_paint(uint8_t flag)
{
   uint8_t i,font_size_flag=0;
   //设置背景黑色
    if(flag == SPORT_DATA_SHOW)
    {
		LCD_SetBackgroundColor(LCD_WHITE);
	}
	else
	{
		LCD_SetBackgroundColor(LCD_BLACK);
	}
	am_hal_rtc_time_get(&RTC_time);

	SetWord_t word = {0};
	char str[20]={0};
	uint16_t number_len;

    word.x_axis = 3;//12 ->3
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_13_SIZE;
    if(flag == SPORT_DATA_SHOW)
    {
		word.forecolor = LCD_BLACK;
    }
	else
	{
		word.forecolor = LCD_WHITE;
	}
	word.bckgrndcolor = LCD_NONE;
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", RTC_time.ui32Hour, RTC_time.ui32Minute);
	LCD_SetNumber(str,&word);
	if(flag == SPORT_DATA_SHOW)
    {
	 	LCD_SetRectangle(80 - 1 ,2,0,240,LCD_RED,0,0,LCD_FILL_ENABLE);
	 	LCD_SetRectangle(160 - 1,2,0,240,LCD_RED,0,0,LCD_FILL_ENABLE);
		LCD_SetRectangle(80 ,80,LCD_LINE_CNT_MAX/2 - 1,2,LCD_RED,0,0,LCD_FILL_ENABLE);
	}
	else
	{
		LCD_SetRectangle(80 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_CYAN,0,0,LCD_FILL_ENABLE);
	 	LCD_SetRectangle(160 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_CYAN,0,0,LCD_FILL_ENABLE);
		LCD_SetRectangle(80 + 4 ,80-4*2,LCD_LINE_CNT_MAX/2 - 1,2,LCD_CYAN,0,0,LCD_FILL_ENABLE);
	}

	if(flag==SPORT_DATA_SHOW)
		{
		if((ScreenState!=DISPLAY_SCREEN_SWIMMING)&&(gui_sport_type!=5))
		{
            	
			if((g_sport_status == SPORT_STATUS_PAUSE)&&(ScreenState!=DISPLAY_SCREEN_REMIND_PAUSE))
			{
				//表示暂停状态
		      
			  LCD_SetPicture(55,202,LCD_MAROON,LCD_NONE,&img_sport_pause_cod);
				
			}
		  else
			{
				gui_sport_gps_status(SPORT_GPS_RIGHT_UP);
			} 
			gui_sport_index_circle_vert_paint(0, 3);
		 }
		else
			{
		     if((g_sport_status == SPORT_STATUS_PAUSE)&&(ScreenState!=DISPLAY_SCREEN_REMIND_PAUSE))
			 
			{
				//表示暂停状态
		      
			  LCD_SetPicture(55,202,LCD_MAROON,LCD_NONE,&img_sport_pause_cod);
				
			}
		  else;
			
	        gui_sport_index_circle_vert_paint(0, 2);
			}
		}

	else
		{
        
		LCD_SetPicture(58,205,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//右上角箭头,指示ok键

        }
     	for(i=1;i<3;i++)
		{

		
		gui_sport_data_show_str(m_sport_data_display_buf[i],str);
		
		number_len = strlen(str);
		if(number_len>=6)
			{
           font_size_flag = 1;
			 break;
			 }
		}
	
	
	for(i=0;i<4;i++)
		{
	    gui_sport_data_four_bar_display_paint(i+1,m_sport_data_display_buf[i],flag,font_size_flag);
		}
	

    



}




void gui_sport_data_bar_display_paint(void)

{   
   LCD_SetBackgroundColor(LCD_BLACK);
    SetWord_t word = {0};
	LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);

	 LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
 
  LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
	word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
/*第一栏*/
	if(m_sport_data_display_index==0)
		{
        
		LCD_SetString("布局",&word);

		}
	else
		{

       
	   LCD_SetString((char *)sportbarstrs[SetValue.Language][m_sport_data_display_index - 1],&word);
       

	   }
		
/*第二栏*/

 // word.x_axis = 120 - 24/2;
  word.x_axis = 120 - 8 -24;

  word.y_axis = LCD_CENTER_JUSTIFIED;
  word.size = LCD_FONT_24_SIZE;
  word.forecolor = LCD_CYAN;
  word.bckgrndcolor = LCD_NONE;
  LCD_SetString((char *)sportbarstrs[SetValue.Language][m_sport_data_display_index],&word);


  word.x_axis = 120 + 8;
  word.y_axis = LCD_CENTER_JUSTIFIED;
  word.size = LCD_FONT_16_SIZE;
  word.forecolor = LCD_LIGHTGRAY;
  word.bckgrndcolor = LCD_NONE;
  LCD_SetString((char *)SportDataDisStrs[SetValue.Language][m_sport_data_display_buf[m_sport_data_display_index]],&word);



 LCD_SetPicture(120 -img_sport_bar[m_sport_data_display_index].height/2,120 - 3*LCD_FONT_24_SIZE*0.5 - 12 -img_sport_bar[m_sport_data_display_index].width,LCD_NONE,LCD_NONE, &img_sport_bar[m_sport_data_display_index]);
/*第三栏*/
word.x_axis = 120 + 24/2 + 36 + 24;
word.y_axis = LCD_CENTER_JUSTIFIED;
word.size = LCD_FONT_16_SIZE;
word.forecolor = LCD_LIGHTGRAY;
word.bckgrndcolor = LCD_NONE;

if(m_sport_data_display_index==3)
	{
	
	LCD_SetString((char *)sportbarstrs[SetValue.Language][0],&word);

	}
else
	{

   
   LCD_SetString((char *)sportbarstrs[SetValue.Language][m_sport_data_display_index + 1],&word);


   }






}
#ifdef COD 
void gui_vice_screen_hint_paint(void)
{
    LCD_SetBackgroundColor(LCD_BLACK);
    SetWord_t word = {0};
	word.x_axis = 120 + 8;			
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	if(cod_user_sport_cast.sport_type<=4)
	{
	
	 LCD_SetString((char *)Guivicescreensport[SetValue.Language][cod_user_sport_cast.sport_type-1],&word);


	 LCD_SetPicture(120-img_vice_sports[0].height,LCD_CENTER_JUSTIFIED,LCD_CYAN,LCD_NONE,&img_vice_sports[cod_user_sport_cast.sport_type-1]);
	}
	else;
   


}
void gui_vice_run_save_detail_1_paint(uint8_t backgroundcolor)

{
   SetWord_t word = {0};
   char str[20];
   char *colon = ":";
   uint8_t charlen;
   
   uint32_t second;
   //设置背景黑色
  LCD_SetBackgroundColor(backgroundcolor);


    if (backgroundcolor == LCD_BLACK)
    {
        //黑底白字
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //白底黑字
        word.forecolor = LCD_BLACK;
    }
    word.x_axis = 16;
    word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
    word.bckgrndcolor = LCD_NONE;
	LCD_SetString("户外跑", &word);


	
   //结束时间
 /* memset(str,0,20);
    sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute);
    word.x_axis = 40+8;
    word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_13_SIZE;
    word.bckgrndcolor = LCD_NONE;
    word.kerning = 0;
    LCD_SetNumber(str,&word);*/


	

	//距离
    charlen = strlen(sportdetailstrsgod[SetValue.Language][0]);
    word.x_axis = 120 - 4 - 19 - 4 - 16;
	word.y_axis = 35;
    word.size = LCD_FONT_16_SIZE;
    word.kerning = 0;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][0],&word);
	//:
    word.y_axis += charlen*8 + 1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",(uint8_t)(cod_user_sport_cast.distance/1000),cod_user_sport_cast.distance%1000);
	#else
    sprintf(str,"%d.%02d",((uint32_t)cod_user_sport_cast.distance/1000),(uint32_t)cod_user_sport_cast.distance/10%100);
	#endif
	word.x_axis = 120 - 4 - 19;
    word.y_axis = 35;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
	word.x_axis = 120 - 4 - 16;
    word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("km",&word);


//时长     
    charlen = strlen(sportdetailstrsgod[SetValue.Language][1]);
    word.x_axis = 120 - 4 - 19 - 4 - 16;
    word.y_axis = 130;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][1],&word);
    word.y_axis += charlen*8 + 1;
    LCD_SetString(colon,&word);
    word.x_axis = 120 - 4 - 19;
    word.y_axis = 130;
    word.size = LCD_FONT_19_SIZE;
    memset(str,0,20);
	  //计算时长
    second = cod_user_sport_cast.time;
	if(second >= 3600)
	{
		//时分秒
		sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
	}
	else
	{
		//分秒
		sprintf(str,"%d:%02d",second/60%60,second%60);
	}
    word.kerning = 1;
    LCD_SetNumber(str,&word);



//平均配速 
    charlen = strlen(sportdetailstrsgod[SetValue.Language][2]);
    word.x_axis =  120 + 4 + 12;
    word.y_axis = 35;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][2],&word);
	
    word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
	
    memset(str,0,20);
    sprintf(str,"%d:%02d",(cod_user_sport_cast.pace/60),(cod_user_sport_cast.pace%60));
    word.x_axis =  120 + 4 + 12 + 16 + 4;
    word.y_axis = 35;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
    word.y_axis += strlen(str)*10 - 0.5*10 + 8;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("/km",&word);

   
   //平均心率
    charlen = strlen(sportdetailstrsgod[SetValue.Language][3]);
    word.x_axis = 120 + 4 + 12;
    word.y_axis = 130;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][3],&word);
	word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
	
    memset(str,0,20);
    sprintf(str,"%d",cod_user_sport_cast.avg_heart_rate);
	
    word.x_axis = 120 + 4 + 12 + 16 + 4;
    word.y_axis = 130;
    word.kerning = 1;	
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.y_axis += strlen(str)*10  + 8;
    word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("bpm",&word);


	word.x_axis = 163+36;//框坐标
	LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_PERU,0,0,LCD_FILL_ENABLE);
    LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);

    
 
}
void gui_vice_run_save_detail_2_paint(uint8_t backgroundcolor)
{
   SetWord_t word = {0};
   char str[20];
   char *colon = ":";
   uint8_t charlen;
  
    //设置背景黑色 
    LCD_SetBackgroundColor(backgroundcolor);

    if (backgroundcolor == LCD_BLACK)
    {
        //黑底白字
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //白底黑字
        word.forecolor = LCD_BLACK;
    }
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
    
    //卡路里
    charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
    word.x_axis = 120 - 4 - 19 - 4 - 16 - 16;
	word.y_axis = 35;
    word.size = LCD_FONT_16_SIZE;
    word.kerning = 0;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
    word.y_axis += charlen*8 + 1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d.%01d",(uint32_t)(cod_user_sport_cast.calorie),(uint32_t)(cod_user_sport_cast.calorie*10)%10);
    word.x_axis = 120 - 4 - 19 - 16;
    word.y_axis = 35;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.x_axis = 120 - 4 - 16 - 16;
    word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("kcal",&word);


	//总步数
    charlen = strlen(sportdetailstrsgod[SetValue.Language][5]);
    word.x_axis = 120 - 4 - 19 - 4 - 16 - 16;
    word.y_axis = 130;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][5],&word);
	word.y_axis += charlen*8 + 1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d",cod_user_sport_cast.step);
    word.x_axis = 120 - 4 - 19 - 16;
    word.y_axis = 130;
    word.size = LCD_FONT_19_SIZE;
    word.kerning = 1;
    LCD_SetNumber(str,&word);
  

    //平均步频
    charlen = strlen(sportdetailstrsgod[SetValue.Language][6]);
     word.x_axis =  120;
    word.y_axis = 35;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][6],&word);
    word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d",(uint16_t)((cod_user_sport_cast.step*60) / cod_user_sport_cast.time));
     word.x_axis =  120  + 16 + 4;
    word.y_axis = 35;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.x_axis =  120  + 16 + 4 + 19 - 16;
    word.y_axis += strlen(str)*10 - 0.5*10 + 8;
    word.kerning = 0;
   word.size = LCD_FONT_16_SIZE;
    LCD_SetString("spm",&word);



    //平均步幅
    charlen = strlen(sportdetailstrsgod[SetValue.Language][7]);
    word.x_axis = 120;
    word.y_axis = 130;
    word.kerning = 0;
	
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][7],&word);
    word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",(uint16_t)((cod_user_sport_cast.distance*100)/cod_user_sport_cast.step),
	(((cod_user_sport_cast.distance*100.f)/cod_user_sport_cast.step) - (uint16_t)((cod_user_sport_cast.distance*100.f)/cod_user_sport_cast.step))*100);
	#else
	
    sprintf(str,"%d",(uint16_t)((cod_user_sport_cast.distance*100)/cod_user_sport_cast.step));
	#endif
    word.x_axis = 120 + 16 + 4;
    word.y_axis = 130;
    word.kerning = 1;
	
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.y_axis += strlen(str)*10  + 8;
    word.kerning = 0;
	word.x_axis =  120  + 16 + 4 + 19 - 16;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("cm",&word);



    //累积爬升
    charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
    word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32; //179
    word.y_axis = 35;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
	word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);

    memset(str,0,20);
    sprintf(str,"%d",cod_user_sport_cast.total_altitude_increase);
    word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 + 16 - 19;
    word.y_axis += 8;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
	
     word.x_axis = 179;
     word.y_axis += strlen(str)*10  + 8;
	 word.kerning = 0;
	 word.size = LCD_FONT_16_SIZE;
	 LCD_SetString("m",&word);


    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
    
 
}

void gui_vice_sport_run_detail_paint(void)
{
	switch(sport_cast_detail_index)
		{
			case GUI_VICE_SPORT_DETAIL_1:
				
				gui_vice_run_save_detail_1_paint(LCD_BLACK);
				break;
			case GUI_VICE_SPORT_DETAIL_2:
				
				gui_vice_run_save_detail_2_paint(LCD_BLACK);
				break;
			
		
			default:
				break;
		}


}
 void gui_vice_sport_walk_detail_1_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	   
	   uint32_t second;
	
		
	
		//设置背景黑色
		LCD_SetBackgroundColor(backgroundcolor);
	
		if (backgroundcolor == LCD_BLACK)
		{
			//黑底白字
			word.forecolor = LCD_WHITE;
		}
		else
		{
			//白底黑字
			word.forecolor = LCD_BLACK;
		}
		word.x_axis = 16;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetString("户外健走", &word);
	
	
		
	   //结束时间
		/*memset(str,0,20);
		sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute);
		word.x_axis = 40+8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_13_SIZE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		LCD_SetNumber(str,&word);*/
	
	
		
	
		//距离
		charlen = strlen(sportdetailstrsgod[SetValue.Language][0]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][0],&word);
		//:
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
        #if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%02d",(uint8_t)(cod_user_sport_cast.distance/1000),cod_user_sport_cast.distance%1000);
        #else
		sprintf(str,"%d.%02d",((uint32_t)cod_user_sport_cast.distance/1000),(uint32_t)cod_user_sport_cast.distance/10%100);
        #endif

		word.x_axis = 120 - 4 - 19;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis = 120 - 4 - 16;
		word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("km",&word);
	
	
	//时长	   
		charlen = strlen(sportdetailstrsgod[SetValue.Language][1]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][1],&word);
	   word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
	
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 130;
		word.size = LCD_FONT_19_SIZE;
		//计算时长
		second = cod_user_sport_cast.time;
		memset(str,0,20);
		if(second >= 3600)
		{
			//时分秒
			sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//分秒
			sprintf(str,"%d:%02d",second/60%60,second%60);
		}
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
	
	//平均配速 
		charlen = strlen(sportdetailstrsgod[SetValue.Language][2]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][2],&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d:%02d",(cod_user_sport_cast.pace/60),(cod_user_sport_cast.pace%60));
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
		LCD_SetString("/km",&word);
	
	   
	   //平均心率
		charlen = strlen(sportdetailstrsgod[SetValue.Language][3]);
		word.x_axis = 120 + 4 + 12;
		word.y_axis = 130;
		word.kerning = 0;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][3],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d",cod_user_sport_cast.avg_heart_rate);
		
		word.x_axis = 120 + 4 + 12 + 16 + 4;
		word.y_axis = 130;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.y_axis += strlen(str)*10  + 8;
		word.kerning = 0;
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("bpm",&word);
	
	
		word.x_axis = 163+36;//框坐标
		LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_PERU,0,0,LCD_FILL_ENABLE);
	   
	   LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
	
		
	 
	}

//详细数据界面2
 void gui_vice_sport_walk_detail_2_paint(uint8_t backgroundcolor)
	{
		SetWord_t word = {0};
		uint32_t sec = 0;
		char str[20] = {0};
		
		char *colon = ":";
		uint8_t charlen;
		
		//设置背景黑色
	  LCD_SetBackgroundColor(backgroundcolor);
		
		if (backgroundcolor == LCD_BLACK)
		{
				//黑底白字
				word.forecolor = LCD_WHITE;
		}
		else
		{
				//白底黑字
				word.forecolor = LCD_BLACK;
		}
		
	
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
	
	
	//总步数
    charlen = strlen(sportdetailstrsgod[SetValue.Language][5]);
    word.x_axis = 120 - 8 - 19 - 4 - 16;
    word.y_axis = 120 - 2*16;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][5],&word);
	  word.y_axis =  120 - 2*16 + charlen*8 + 1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d",cod_user_sport_cast.step);
    word.x_axis = 120 - 8 - 19;
    word.y_axis = 120 - 2*16;
    word.size = LCD_FONT_19_SIZE;
    word.kerning = 1;
    LCD_SetNumber(str,&word);

	
	//累积爬升
    charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
    word.x_axis =  120 + 8; 
    word.y_axis = 120 - 2*16;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
	word.y_axis = 120 - 2*16 + charlen*8 + 1;
    LCD_SetString(colon,&word);

    memset(str,0,20);
    sprintf(str,"%d",(uint16_t)(cod_user_sport_cast.total_altitude_increase));
    word.x_axis =  120 + 8 + 16 + 4;
    word.y_axis = 120 - 2*16;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
	
     word.x_axis =  120 + 8 + 16 + 4 + 19 - 16;
     word.y_axis += strlen(str)*10  + 8;
	 word.kerning = 0;
	 word.size = LCD_FONT_16_SIZE;
	 LCD_SetString("m",&word);
	
		
		
	
    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
	
}

void gui_vice_sport_walk_detail_paint(void)
	{
		switch(sport_cast_detail_index)
		{
			case GUI_VICE_SPORT_DETAIL_1:
			
			
			gui_vice_sport_walk_detail_1_paint(LCD_BLACK);
			
				break;
			case GUI_VICE_SPORT_DETAIL_2:
				
				gui_vice_sport_walk_detail_2_paint(LCD_BLACK);
				break;
		
			
			
			default:
				break;
		}
	}
 void gui_vice_sport_cycling_detail_1_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	   
	   uint32_t second;
	
		
	
		//设置背景黑色
		LCD_SetBackgroundColor(backgroundcolor);
	
		if (backgroundcolor == LCD_BLACK)
		{
			//黑底白字
			word.forecolor = LCD_WHITE;
		}
		else
		{
			//白底黑字
			word.forecolor = LCD_BLACK;
		}
		word.x_axis = 16;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetString("骑行", &word);
	
	
		
	   //结束时间
	/*	memset(str,0,20);
		sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute);
		word.x_axis = 40+8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_13_SIZE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		LCD_SetNumber(str,&word);*/
	
	
		
	
		//距离
		charlen = strlen(sportdetailstrsgod[SetValue.Language][0]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][0],&word);
		//:
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
	    #if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%02d",(uint8_t)(cod_user_sport_cast.distance/1000),cod_user_sport_cast.distance%1000);
        #else
		sprintf(str,"%d.%02d",((uint32_t)cod_user_sport_cast.distance/1000),(uint32_t)cod_user_sport_cast.distance/10%100);
        #endif
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis = 120 - 4 - 16;
		word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("km",&word);
	
	
	//时长	   
		charlen = strlen(sportdetailstrsgod[SetValue.Language][1]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][1],&word);
	   word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
	
	
		
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 130;
		word.size = LCD_FONT_19_SIZE;
		
		second = cod_user_sport_cast.time;
		memset(str,0,20);
		if(second >= 3600)
		{
			//时分秒
			sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//分秒
			sprintf(str,"%d:%02d",second/60%60,second%60);
		}
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
	
	//平均速度
		charlen = strlen(sportdetailstrsgod[SetValue.Language][14]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][14],&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d.%02d",(uint32_t)(cod_user_sport_cast.avg_speed),(uint32_t)(cod_user_sport_cast.avg_speed*100)%100);
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
		LCD_SetString("km/h",&word);
	
	   
	   //平均心率
		charlen = strlen(sportdetailstrsgod[SetValue.Language][3]);
		word.x_axis = 120 + 4 + 12;
		word.y_axis = 130;
		word.kerning = 0;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][3],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d",cod_user_sport_cast.avg_heart_rate);
		
		word.x_axis = 120 + 4 + 12 + 16 + 4;
		word.y_axis = 130;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.y_axis += strlen(str)*10  + 8;
		word.kerning = 0;
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("bpm",&word);
	
	
		word.x_axis = 163+36;//框坐标
		LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_PERU,0,0,LCD_FILL_ENABLE);
	   
	   LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
	

}


//详细数据界面2
 void gui_vice_sport_cycling_detail_2_paint(uint8_t backgroundcolor)
	{
   SetWord_t word = {0};
   char str[20];
   char *colon = ":";
   uint8_t charlen;
  
    //设置背景黑色 
    LCD_SetBackgroundColor(backgroundcolor);

    if (backgroundcolor == LCD_BLACK)
    {
        //黑底白字
        word.forecolor = LCD_WHITE;
    }
    else
    {
        //白底黑字
        word.forecolor = LCD_BLACK;
    }
  
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
    
    //卡路里
    charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
    word.x_axis = 120 - 4 - 19 - 4 - 16 - 16;
	word.y_axis = 35;
    word.size = LCD_FONT_16_SIZE;
    word.kerning = 0;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
    word.y_axis += charlen*8 + 1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
   // sprintf(str,"%d",(uint16_t)(cod_user_sport_cast.calorie));
   sprintf(str,"%d.%01d",(uint32_t)(cod_user_sport_cast.calorie),(uint32_t)(cod_user_sport_cast.calorie*10)%10);
    word.x_axis = 120 - 4 - 19 - 16;
    word.y_axis = 35;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.x_axis = 120 - 4 - 16 - 16;
    word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("kcal",&word);


	//最高海拔
    charlen = strlen(sportdetailstrsgod[SetValue.Language][47]);
    word.x_axis = 120 - 4 - 19 - 4 - 16 - 16;
    word.y_axis = 130;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][47],&word);
	word.y_axis += charlen*8 + 1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d",(cod_user_sport_cast.max_altitude));
    word.x_axis = 120 - 4 - 19 - 16;
    word.y_axis = 130;
    word.size = LCD_FONT_19_SIZE;
    word.kerning = 1;
    LCD_SetNumber(str,&word);
    word.x_axis = 120 - 4 - 16 - 16;
	word.y_axis += strlen(str)*10 - 0.5*10 + 8;
	word.kerning = 0;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString("m",&word);

    //累积爬升
    charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
    word.x_axis =  120;
    word.y_axis = 35;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
    word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
    sprintf(str,"%d",cod_user_sport_cast.total_altitude_increase);
     word.x_axis =  120  + 16 + 4;
    word.y_axis = 35;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.x_axis =  120  + 16 + 4 + 19 - 16;
    word.y_axis += strlen(str)*10 - 0.5*10 + 8;
    word.kerning = 0;
   word.size = LCD_FONT_16_SIZE;
    LCD_SetString("m",&word);



   	//海拔上升
    charlen = strlen(sportdetailstrsgod[SetValue.Language][20]);
    word.x_axis = 120;
    word.y_axis = 130;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
   	LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][20],&word);
    word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);
    memset(str,0,20);
	sprintf(str,"%d",cod_user_sport_cast.altitude_diff);
    word.x_axis = 120 + 16 + 4;
    word.y_axis = 130;
    word.kerning = 1;
	
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
    word.y_axis += strlen(str)*10  + 8;
    word.kerning = 0;
	word.x_axis =  120  + 16 + 4 + 19 - 16;
    word.size = LCD_FONT_16_SIZE;
    LCD_SetString("m",&word);



      //上坡里程
    charlen = strlen(sportdetailstrsgod[SetValue.Language][48]);
    word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32; //179
    word.y_axis = 35;
    word.kerning = 0;
    word.size = LCD_FONT_16_SIZE;
   	LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][48],&word);
	word.y_axis += charlen*8+1;
    LCD_SetString(colon,&word);

    memset(str,0,20);
	sprintf(str,"%d.%02d",(cod_user_sport_cast.climb_distance/1000),cod_user_sport_cast.climb_distance/10%100);
    word.x_axis =  120 + 4 + 16 + 4 - 16 + 19 + 32 + 16 - 19;
    word.y_axis += 8;
    word.kerning = 1;
    word.size = LCD_FONT_19_SIZE;
    LCD_SetNumber(str,&word);
	
     word.x_axis = 179;
     word.y_axis += strlen(str)*10  + 8;
	 word.kerning = 0;
	 word.size = LCD_FONT_16_SIZE;
	LCD_SetString("km",&word);

    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
    
 
}

void gui_vice_sport_ride_detail_paint(void)
	{
		switch(sport_cast_detail_index)
		{
			case GUI_VICE_SPORT_DETAIL_1:
				
				gui_vice_sport_cycling_detail_1_paint(LCD_BLACK);
				break;
			case GUI_VICE_SPORT_DETAIL_2:
				
				gui_vice_sport_cycling_detail_2_paint(LCD_BLACK);
				break;
			
			
			default:
	
				break;
		}
	}
void gui_vice_climbing_save_detail_1_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	   
	   uint32_t second;
	

	
		//设置背景黑色
		LCD_SetBackgroundColor(backgroundcolor);
	
		if (backgroundcolor == LCD_BLACK)
		{
			//黑底白字
			word.forecolor = LCD_WHITE;
		}
		else
		{
			//白底黑字
			word.forecolor = LCD_BLACK;
		}
		word.x_axis = 16;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.bckgrndcolor = LCD_NONE;
		LCD_SetString("登山", &word);
	
	
		
	   //结束时间
		/*memset(str,0,20);
		sprintf(str,"%02d-%02d-%02d %02d:%02d",ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute);
		word.x_axis = 40+8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_13_SIZE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		LCD_SetNumber(str,&word);*/
	
	
		
	
		//距离
		charlen = strlen(sportdetailstrsgod[SetValue.Language][0]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][0],&word);
		//:
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
	    #if defined (SPRINTF_FLOAT_TO_INT)
		sprintf(str,"%d.%02d",(uint8_t)(cod_user_sport_cast.distance/1000),cod_user_sport_cast.distance%1000);
        #else
		sprintf(str,"%d.%02d",((uint32_t)cod_user_sport_cast.distance/1000),(uint32_t)cod_user_sport_cast.distance/10%100);
        #endif
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis = 120 - 4 - 16;
		word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("km",&word);
	
	
	//时长	   
		charlen = strlen(sportdetailstrsgod[SetValue.Language][1]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][1],&word);
	   word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
	
		
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 130;
		word.size = LCD_FONT_19_SIZE;
		
		second = cod_user_sport_cast.time;
		memset(str,0,20);
		if(second >= 3600)
		{
			//时分秒
			sprintf(str,"%d:%02d:%02d",second/3600,second/60%60,second%60);
		}
		else
		{
			//分秒
			sprintf(str,"%d:%02d",second/60%60,second%60);
		}
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
	
	//平均速度(登山使用垂直速度)
		charlen = strlen(sportdetailstrsgod[SetValue.Language][14]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][14],&word);
		
		//LCD_SetString("垂直速度",&word);
		
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		
		//sprintf(str,"%.1f",cod_user_sport_cast.avg_speed);
		sprintf(str,"%d.%02d",(uint32_t)cod_user_sport_cast.avg_speed,(uint32_t)(cod_user_sport_cast.avg_speed*100)%100);
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
		LCD_SetString("km/h",&word);
	
	   
	   //平均心率
		charlen = strlen(sportdetailstrsgod[SetValue.Language][3]);
		word.x_axis = 120 + 4 + 12;
		word.y_axis = 130;
		word.kerning = 0;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][3],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		
		memset(str,0,20);
		sprintf(str,"%d",cod_user_sport_cast.avg_heart_rate);
		
		word.x_axis = 120 + 4 + 12 + 16 + 4;
		word.y_axis = 130;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.y_axis += strlen(str)*10  + 8;
		word.kerning = 0;
	
		
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("bpm",&word);
	
	
		word.x_axis = 163+36;//框坐标
		LCD_SetRectangle(word.x_axis,4+8+4,0,240,LCD_PERU,0,0,LCD_FILL_ENABLE);
	   
	   LCD_SetPicture(word.x_axis+4, LCD_CENTER_JUSTIFIED, LCD_BLACK, LCD_NONE, &Img_Pointing_Down_12X8);
	
		
	 
}


//登山运动详细_2界面
void gui_vice_climbing_save_detail_2_paint(uint8_t backgroundcolor)
	{
	   SetWord_t word = {0};
	   char str[20];
	   char *colon = ":";
	   uint8_t charlen;
	  
		//设置背景黑色 
		LCD_SetBackgroundColor(backgroundcolor);
	
		if (backgroundcolor == LCD_BLACK)
		{
			//黑底白字
			word.forecolor = LCD_WHITE;
		}
		else
		{
			//白底黑字
			word.forecolor = LCD_BLACK;
		}
	  
		LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
		
		//卡路里
		charlen = strlen(sportdetailstrsgod[SetValue.Language][4]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 35;
		word.size = LCD_FONT_16_SIZE;
		word.kerning = 0;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][4],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		//sprintf(str,"%d",(uint16_t)(cod_user_sport_cast.calorie));
		sprintf(str,"%d.%01d",(uint32_t)(cod_user_sport_cast.calorie),(uint32_t)(cod_user_sport_cast.calorie*10)%10);
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis = 120 - 4 - 16;
		word.y_axis = 35 + strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("kcal",&word);
	
	
		//总步数
		charlen = strlen(sportdetailstrsgod[SetValue.Language][5]);
		word.x_axis = 120 - 4 - 19 - 4 - 16;
		word.y_axis = 130;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][5],&word);
		word.y_axis += charlen*8 + 1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",cod_user_sport_cast.step);
		word.x_axis = 120 - 4 - 19;
		word.y_axis = 130;
		word.size = LCD_FONT_19_SIZE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	
	
		//累积爬升
	   charlen = strlen(sportdetailstrsgod[SetValue.Language][8]);
		word.x_axis =  120 + 4 + 12;
		word.y_axis = 35;
		word.kerning = 0;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][8],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);
		sprintf(str,"%d",cod_user_sport_cast.total_altitude_increase);
		word.x_axis =  120 + 4 + 12 + 16 + 4;
		word.y_axis = 35;
		word.kerning = 1;
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.y_axis += strlen(str)*10 - 0.5*10 + 8;
		word.kerning = 0;
	   word.size = LCD_FONT_16_SIZE;
	   LCD_SetString("m",&word);
	
		
	
		//最高海拔
		charlen = strlen(sportdetailstrsgod[SetValue.Language][47]);
		word.x_axis = 120 + 4 + 12;
		word.y_axis = 130;
		word.kerning = 0;
		
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)sportdetailstrsgod[SetValue.Language][47],&word);
		word.y_axis += charlen*8+1;
		LCD_SetString(colon,&word);
		memset(str,0,20);

		sprintf(str,"%d",cod_user_sport_cast.max_altitude);
		word.x_axis = 120 + 4 + 12 + 16 + 4;
		word.y_axis = 130;
		word.kerning = 1;
		
		word.size = LCD_FONT_19_SIZE;
		LCD_SetNumber(str,&word);
		word.y_axis += strlen(str)*10  + 8;
		word.kerning = 0;
		word.x_axis =  120 + 4 + 12 + 16 + 4 + 19 - 16;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("m",&word);
	
	
	
		
	
	
		LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
		
	 
	}

void gui_vice_sport_climb_detail_paint(void)
	{
		switch(sport_cast_detail_index)
		{
			case GUI_VICE_SPORT_DETAIL_1:
				
				gui_vice_climbing_save_detail_1_paint(LCD_BLACK);
				break;
			case GUI_VICE_SPORT_DETAIL_2:
				
				gui_vice_climbing_save_detail_2_paint(LCD_BLACK);
				break;	
			
			
			default:
				break;
		}
	}


void gui_vice_sport_detail_paint(uint8_t sport_type)
{
   if(sport_type==SPORT_TYPE_OUTDOOR_RUN)//跑步
   	{
     	
	  gui_vice_sport_run_detail_paint();
    }
   else if(sport_type==SPORT_TYPE_OUTDOOR_WALK)//健走
   	{
        
	  gui_vice_sport_walk_detail_paint();
    }
    else if(sport_type==SPORT_TYPE_RIDE)//骑行
   	{
   	
	 gui_vice_sport_ride_detail_paint();

    }
	 else if(sport_type==SPORT_TYPE_CLIMB)//登山
   	{
      
	  gui_vice_sport_climb_detail_paint();
    }
	 else;


}

void gui_vice_screen_data_paint(uint8_t flag)
	{
   uint8_t i,font_size_flag=0;
   
   uint16_t number_len;
   //设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	am_hal_rtc_time_get(&RTC_time);

	SetWord_t word = {0};
	char str[20]={0};
	

    word.x_axis = 3;//12 ->3
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	memset(str,0,sizeof(str));
	sprintf(str,"%02d:%02d", RTC_time.ui32Hour, RTC_time.ui32Minute);
	LCD_SetNumber(str,&word);
	
	
    LCD_SetRectangle(80 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_CYAN,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(160 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_CYAN,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(80 + 4 ,80-4*2,LCD_LINE_CNT_MAX/2 - 1,2,LCD_CYAN,0,0,LCD_FILL_ENABLE);

	for(i=1;i<3;i++)
		{
		
		if(cod_user_sport_cast.sport_type==SPORT_TYPE_OUTDOOR_RUN)
			{
	          gui_app_sport_data_show_str(vice_screen_run_display[i],str);
			}
		 else if((cod_user_sport_cast.sport_type==SPORT_TYPE_OUTDOOR_WALK))
		 	{

              
	          gui_app_sport_data_show_str(vice_screen_walk_display[i],str);
		    }
          else if((cod_user_sport_cast.sport_type==SPORT_TYPE_RIDE))
		 	{

              
	          gui_app_sport_data_show_str(vice_screen_cycling_display[i],str);
		    }
		   else if((cod_user_sport_cast.sport_type==SPORT_TYPE_CLIMB))
		 	{

              
	          gui_app_sport_data_show_str(vice_screen_climbing_display[i],str);
		    }
		   else;
		number_len = strlen(str);
		if(number_len>=6)
			{
           font_size_flag = 1;
			 break;
			 }
		}
	

		for(i=0;i<4;i++)
			{
			 if(cod_user_sport_cast.sport_type==SPORT_TYPE_OUTDOOR_RUN)
				{
		          gui_sport_data_four_bar_display_paint(i+1,vice_screen_run_display[i],flag,font_size_flag);
				}
			 else if((cod_user_sport_cast.sport_type==SPORT_TYPE_OUTDOOR_WALK))
			 	{

	              
		          gui_sport_data_four_bar_display_paint(i+1,vice_screen_walk_display[i],flag,font_size_flag);
			    }
	          else if((cod_user_sport_cast.sport_type==SPORT_TYPE_RIDE))
			 	{

	              
		          gui_sport_data_four_bar_display_paint(i+1,vice_screen_cycling_display[i],flag,font_size_flag);
			    }
			   else if((cod_user_sport_cast.sport_type==SPORT_TYPE_CLIMB))
			 	{

	              
		          gui_sport_data_four_bar_display_paint(i+1,vice_screen_climbing_display[i],flag,font_size_flag);
			    }
			   else;
			 
			}
	
		



}
#endif

void gui_sport_data_four_bar_display_paint(uint8_t bar_order,uint8_t sport_item,uint8_t flag,uint8_t size_flag)
{
	SetWord_t word_n = {0};
    
	SetWord_t word_w = {0};
   uint16_t item_len,number_len;
   
   char str[20]={0};

    memset(str,0,sizeof(str));
#ifdef COD 
	if(flag==VICE_SCREEN_DATA_SHOW)//副屏模式
		{
         
		 gui_app_sport_data_show_str(sport_item,str);

	    }
	else if(flag==SPORT_DATA_PREVIEW)//预览界面
#else
	 if(flag==SPORT_DATA_PREVIEW)//预览界面
#endif
		{
	  // sprintf(str, (sportsatapreview[SetValue.Language][sport_item]));
	   
	   gui_sport_data_show_str(sport_item,str);
	 }
	else if(flag==SPORT_DATA_SHOW)
		{
		
		gui_sport_data_show_str(sport_item,str);

	   }
	
	
   number_len = strlen(str);
   item_len = strlen((char *)SportDataDisStrs[SetValue.Language][sport_item]);
  
   word_n.size = LCD_FONT_29_SIZE;
   word_w.size = LCD_FONT_16_SIZE;

if((bar_order==1)||(bar_order==4))
{
	word_n.y_axis = LCD_CENTER_JUSTIFIED;
    word_w.y_axis = LCD_CENTER_JUSTIFIED;
    word_n.size = LCD_FONT_37_SIZE;
    word_w.size = LCD_FONT_16_SIZE;

}
else if(bar_order==2)
{


word_w.y_axis = 60 - (item_len/2)*word_w.size*0.5;
if(size_flag==1)
   {
	
	word_n.y_axis = 60 - (number_len*Font_Number_29.width - 0.5*Font_Number_29.width)/2;
	
	word_n.size = LCD_FONT_29_SIZE;
   }
	else
	   {
   word_n.y_axis = 60 - (number_len*Font_Number_37.width - 0.5*Font_Number_37.width)/2;
   
   word_n.size = LCD_FONT_37_SIZE;
	   }


}

else
{
 
	word_w.y_axis =180 - (item_len/2)*word_w.size*0.5;
	if(size_flag==1)
  	{
     
	 word_n.y_axis = 180 - (number_len*Font_Number_29.width - 0.5*Font_Number_29.width)/2;
	 
	 word_n.size = LCD_FONT_29_SIZE;
    }
	 else
	 	{
	word_n.y_axis = 180 - (number_len*Font_Number_37.width - 0.5*Font_Number_37.width)/2;
	
	word_n.size = LCD_FONT_37_SIZE;
	 	}
}

if(bar_order==1)
{
	word_n.x_axis = 3 + Font_Number_13.height + 4;
	if(flag==SPORT_DATA_SHOW)
	{
    	word_n.forecolor = LCD_BLACK;
	}
	else
	{
		word_n.forecolor = LCD_WHITE;
	}

	word_n.bckgrndcolor = LCD_NONE;
	
    word_w.x_axis = 3 + Font_Number_13.height + 4 + Font_Number_37.height + 4;
    
	if(flag==SPORT_DATA_SHOW)
	{
		word_w.forecolor = LCD_BLACK;
	}
	else
	{
		word_w.forecolor = LCD_WHITE;
	}
    
	word_w.bckgrndcolor = LCD_NONE;
	

}
else if(bar_order==4)
{
	
	word_n.x_axis = 161 + 8;
	if(flag==SPORT_DATA_SHOW)
	{
		word_n.forecolor = LCD_BLACK;
	}
	else
	{
		word_n.forecolor = LCD_WHITE;
	}
	word_n.bckgrndcolor = LCD_NONE;
	
	word_w.x_axis = 161 + 8 + Font_Number_37.height + 4;
	if(flag==SPORT_DATA_SHOW)
	{
		word_w.forecolor = LCD_BLACK;
	}
	else
	{
		word_w.forecolor = LCD_WHITE;
	}
	word_w.bckgrndcolor = LCD_NONE;


}

else 
{
	
	word_w.x_axis = 81 + 8;
	if(flag==SPORT_DATA_SHOW)
	{
		word_w.forecolor = LCD_BLACK;
	}
	else
	{
		word_w.forecolor = LCD_WHITE;
	}
	word_w.bckgrndcolor = LCD_NONE;
	word_n.x_axis = 81 + 8 + 16 + 8;
	if(flag==SPORT_DATA_SHOW)
	{
		word_n.forecolor = LCD_BLACK;
	}
	else
	{
		word_n.forecolor = LCD_WHITE;
	}
	word_n.bckgrndcolor = LCD_NONE;

	
	

}



LCD_SetString((char *)SportDataDisStrs[SetValue.Language][sport_item],&word_w);

LCD_SetNumber(str,&word_n);


}






//飞腕数据显示设置界面
void gui_sport_data_display_set_paint(void)
{

	SetWord_t word = {0};
	
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
    LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
    LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);
	
	word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	if(m_sport_data_display_set_index == 0)
	{
		LCD_SetString((char *)SportDataDisStrs[SetValue.Language][m_sport_data_display_set_buf[m_sport_data_display_set_sum -1]],&word);
	}
	else
	{
		LCD_SetString((char *)SportDataDisStrs[SetValue.Language][m_sport_data_display_set_buf[m_sport_data_display_set_index-1]],&word);
	}
	


	//第二栏
	 word.x_axis = 120 - 24/2;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportDataDisStrs[SetValue.Language][m_sport_data_display_set_buf[m_sport_data_display_set_index]],&word);
	


	//第三栏
    word.x_axis = 120 + 24/2 + 36 + 24;
    word.forecolor = LCD_GRAY;
	word.size = LCD_FONT_16_SIZE;
	if(m_sport_data_display_set_index == (m_sport_data_display_set_sum -1))
	{
		LCD_SetString((char *)SportDataDisStrs[SetValue.Language][m_sport_data_display_set_buf[0]],&word);
	}
	else
	{
		LCD_SetString((char *)SportDataDisStrs[SetValue.Language][m_sport_data_display_set_buf[m_sport_data_display_set_index+1]],&word);	
	}

}
//飞腕运动提醒设置当前项开关状态,0-关闭;1-打开;2-无显示
uint8_t gui_sport_remind_set_status(void) 
{
	uint8_t ret = 0;
	switch( ScreenSportSave )
	{
		case DISPLAY_SCREEN_RUN:
			if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HAERT)
			{	
				ret = ((SetValue.SportSet.SwRun & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HRRECOVERY)
			{
				ret = SetValue.HRRecoverySet;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_PACE)
			{
				ret = ((SetValue.SportSet.SwRun & SW_PACE_MASK) == SW_PACE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_DISTANCE)
			{
				ret = ((SetValue.SportSet.SwRun & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? 1:0;
			}
			
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_TIME)
			{
				ret = ((SetValue.SportSet.SwRun & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_AUTOCIRCLE)
			{
				ret = ((SetValue.SportSet.SwRun & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? 1:0;
			}
			
		
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HAERT)
			{	
				ret = ((SetValue.SportSet.SwHike & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HRRECOVERY)
			{
				ret = SetValue.HRRecoverySet;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_PACE)
			{
				ret = ((SetValue.SportSet.SwHike & SW_PACE_MASK) == SW_PACE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_DISTANCE)
			{
				ret = ((SetValue.SportSet.SwHike & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_TIME)
			{
				ret = ((SetValue.SportSet.SwHike & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_AUTOCIRCLE)
			{
				ret = ((SetValue.SportSet.SwHike & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? 1:0;
			}
			
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HAERT)
			{	
				ret = ((SetValue.SportSet.SwCountryRace & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HRRECOVERY)
			{
				ret = SetValue.HRRecoverySet;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_PACE)
			{
				ret = ((SetValue.SportSet.SwCountryRace & SW_PACE_MASK) == SW_PACE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_DISTANCE)
			{
				ret = ((SetValue.SportSet.SwCountryRace & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_TIME)
			{
				ret = ((SetValue.SportSet.SwCountryRace & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_AUTOCIRCLE)
			{
				ret = ((SetValue.SportSet.SwCountryRace & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? 1:0;
			}
			break;
		case DISPLAY_SCREEN_WALK:
			if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_GOALKCAL)
			{	
				ret = ((SetValue.SportSet.SwWalking & SW_GOALKCAL_MASK) == SW_GOALKCAL_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_TIME)
			{
				ret = ((SetValue.SportSet.SwWalking & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_AUTOCIRCLE)
			{
				ret = ((SetValue.SportSet.SwWalking & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HRRECOVERY)
			{
				ret = SetValue.HRRecoverySet;
			}
			break;
		case DISPLAY_SCREEN_CYCLING:
			if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HAERT)
			{	
				ret = ((SetValue.SportSet.SwCycling & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HRRECOVERY)
			{
				ret = SetValue.HRRecoverySet;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_SPEED)
			{
				ret = ((SetValue.SportSet.SwCycling & SW_PACE_MASK) == SW_PACE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_DISTANCE)
			{
				ret = ((SetValue.SportSet.SwCycling & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? 1:0;
			}
				else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_TIME)
			{
				ret = ((SetValue.SportSet.SwCycling & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_AUTOCIRCLE)
			{
				ret = ((SetValue.SportSet.SwCycling & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? 1:0;
			}
		
			break;


		case DISPLAY_SCREEN_CLIMBING://登山
			if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HAERT)
			{	
				ret = ((SetValue.SportSet.SwClimbing & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? 1:0;
			}
		    else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HRRECOVERY)
			{
				ret = SetValue.HRRecoverySet;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_DISTANCE)
			{
				ret = ((SetValue.SportSet.SwClimbing & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_ALTITUDE)
			{
				ret = ((SetValue.SportSet.SwClimbing & SW_ALTITUDE_MASK) == SW_ALTITUDE_MASK) ? 1:0;
			}

			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_TIME)
			{
				ret = ((SetValue.SportSet.SwClimbing & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_AUTOCIRCLE)
			{
				ret = ((SetValue.SportSet.SwClimbing & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? 1:0;
			}
			
			break;
		case DISPLAY_SCREEN_SWIMMING:
			if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HAERT)
			{	
				ret = ((SetValue.SportSet.SwSwimmg & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_SWIMPOOL)
			{
				ret = 2;
			}

			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_DISTANCE)
			{
				ret = ((SetValue.SportSet.SwSwimmg & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_TIME)
			{
				ret = ((SetValue.SportSet.SwSwimmg & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_GOALCIRCLE)
			{
				ret = ((SetValue.SportSet.SwSwimmg & SW_GOALCIRCLE_MASK) == SW_GOALCIRCLE_MASK) ? 1:0;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_AUTOCIRCLE)
			{
				ret = ((SetValue.SportSet.SwSwimmg & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? 1:0;
			}
			
			break;
		default:
			break;
	}

	return ret;

}







//飞腕运动提醒设置界面
void gui_sport_remind_set_paint(void)
{	

	SetWord_t word = {0};
	uint8_t status = 0;
	
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
    LCD_SetRectangle(120 - 24/2 - 36 - 1 ,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetRectangle(120 + 24/2 + 36 - 1,2,LCD_LINE_CNT_MAX/2 - 90,180,LCD_LIGHTGRAY,0,0,LCD_FILL_ENABLE);
	LCD_SetPicture(10, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Up_12X8);
    LCD_SetPicture(225, LCD_CENTER_JUSTIFIED, LCD_PERU, LCD_NONE, &Img_Pointing_Down_12X8);

	//第一栏
	word.x_axis = 120 - 24/2 - 36 - 24 - 16;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	
	if(m_sport_remind_set_index == 0)
	{
		if(m_sport_remind_set_sum > 2)
		{
			LCD_SetString((char *)SportRemindStrs[SetValue.Language][m_sport_remind_set_buf[m_sport_remind_set_sum -1]],&word);
		}
	}
	else
	{
		if(m_sport_remind_set_sum > 1)
		{
			LCD_SetString((char *)SportRemindStrs[SetValue.Language][m_sport_remind_set_buf[m_sport_remind_set_index-1]],&word);
		}
	}


	
	
	//第二栏
	
	status = gui_sport_remind_set_status();


  if (status !=2)//有关闭或者打开状态
	{

	  word.x_axis = 120 - (24 + 8 + 12)/2;
      word.y_axis = LCD_CENTER_JUSTIFIED;
      word.size = LCD_FONT_24_SIZE;
      word.forecolor = LCD_CYAN;
      word.bckgrndcolor = LCD_NONE;
	   LCD_SetString((char *)SportRemindStrs[SetValue.Language][m_sport_remind_set_buf[m_sport_remind_set_index]],&word);
		word.x_axis += 24 + 8;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_LIGHTGRAY;
        word.bckgrndcolor = LCD_NONE;
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][6+status],&word);
	}
else
   {
    word.x_axis = 120 - 24/2;
    word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
    word.forecolor = LCD_CYAN;
    word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)SportRemindStrs[SetValue.Language][m_sport_remind_set_buf[m_sport_remind_set_index]],&word);
   }

	if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HRRECOVERY)
	{
		if (status == 1)
		{
			LCD_SetPicture(120 - img_v_switch_on.height/2, 200, LCD_NONE, LCD_NONE, &img_v_switch_on);
		}
		else
		{
			LCD_SetPicture(120 - img_v_switch_on.height/2, 200, LCD_NONE, LCD_NONE, &img_v_switch_off);
		}
	}
	

	//第三栏
    word.x_axis = 120 + 24/2 + 36 + 24;
    word.forecolor = LCD_GRAY;
	word.size = LCD_FONT_16_SIZE;
	if(m_sport_remind_set_index == (m_sport_remind_set_sum -1))
	{
		if(m_sport_remind_set_sum > 2)
		{
			LCD_SetString((char *)SportRemindStrs[SetValue.Language][m_sport_remind_set_buf[0]],&word);
		}
	}
	else
	{
		if(m_sport_remind_set_sum > 1)
		{
			LCD_SetString((char *)SportRemindStrs[SetValue.Language][m_sport_remind_set_buf[m_sport_remind_set_index+1]],&word);
		}			
	}


}
//飞腕心率提醒设置界面
void gui_sport_remind_set_heart_paint(void)
{
	uint8_t hdr_value;
	SetWord_t word = {0};
	 char str[10];

	LCD_SetBackgroundColor(LCD_BLACK);
	
		
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;

	LCD_SetString((char *)SportRemindStrs[SetValue.Language][0],&word);


	/*对称坐标点110*/
	word.x_axis = 110 -16/2;
	word.y_axis = 120 + 4;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;;	
	if(m_remind_heartrate.enable)
	{
        
		word.forecolor = LCD_WHITE;
	   LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
	   LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
	   hdr_value =  m_remind_heartrate.tens *10 +m_remind_heartrate.ones;
			
	}
	else
	{
	     
		word.forecolor = LCD_LIGHTGRAY;
		LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
		hdr_value = 0.9*get_hdr_max();
	
	}
	
	word.x_axis = 130;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	
	word.size = LCD_FONT_56_SIZE;
	memset(str,0,10);	
	sprintf(str,"%d",hdr_value);
	LCD_SetNumber(str,&word);	

	if (m_remind_heartrate.enable ==false)
	{
		word.x_axis = 130 + 56 + 14;
		word.forecolor = LCD_LIGHTGRAY;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][8],&word);
	}

	switch(m_gui_sport_remind_set_heart_index)
	{
		/*case GUI_SPORT_REMIND_SET_HEART_SWITCH:{	
			LCD_SetRectangle(128,6,96,48,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
		}break;*/
		case GUI_SPORT_REMIND_SET_HEART_TENS:{
			if (m_remind_heartrate.tens >=10)
			{
				LCD_SetRectangle(200,6,120 - 3*29/2 - 2,2*29+1,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
			else
			{
				LCD_SetRectangle(200,6,120 - 29 - 1,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
		}break;		
		case GUI_SPORT_REMIND_SET_HEART_ONES:{
			if (m_remind_heartrate.tens >=10)
			{
				LCD_SetRectangle(200,6,120 + 0.5*29 +1,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
			else
			{
				LCD_SetRectangle(200,6,121,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
		}break;	
		default:
			break;
		
	}

}
//飞腕心率提醒设置界面初始化数据
void gui_sport_remind_set_heart_init(void)
{
	switch( ScreenSportSave )
	{
		case DISPLAY_SCREEN_RUN:
			m_remind_heartrate.enable = ((SetValue.SportSet.SwRun & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? true:false;
			break;
		case DISPLAY_SCREEN_CYCLING:
			m_remind_heartrate.enable = ((SetValue.SportSet.SwCycling & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? true:false;
			break;
		case DISPLAY_SCREEN_CLIMBING:
			m_remind_heartrate.enable = ((SetValue.SportSet.SwClimbing & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? true:false;
			break;
		case DISPLAY_SCREEN_SWIMMING:
			m_remind_heartrate.enable = ((SetValue.SportSet.SwSwimmg & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? true:false;
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			m_remind_heartrate.enable = ((SetValue.SportSet.SwHike & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? true:false;
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			m_remind_heartrate.enable = ((SetValue.SportSet.SwCountryRace & SW_HEARTRATE_MASK) == SW_HEARTRATE_MASK) ? true:false;
			break;

		
		default:
			break;
	}

	m_remind_heartrate.tens = SetValue.AppSet.htr_zone_five_min/10;
	m_remind_heartrate.ones =  SetValue.AppSet.htr_zone_five_min%10;

}
//飞腕心率提醒设置界面保存数据
void gui_sport_remind_set_heart_save(void)
{
	switch( ScreenSportSave )
	{
		case DISPLAY_SCREEN_RUN:
			if(m_remind_heartrate.enable )
			{
				SetValue.SportSet.SwRun |=  SW_HEARTRATE_MASK;
			}
			else
			{
				SetValue.SportSet.SwRun &=  ~SW_HEARTRATE_MASK;
			}
			break;
		case DISPLAY_SCREEN_MARATHON:
			if(m_remind_heartrate.enable )
			{
				SetValue.SportSet.SwMarathon |=  SW_HEARTRATE_MASK;
			}
			else
			{
				SetValue.SportSet.SwMarathon &=  ~SW_HEARTRATE_MASK;
			}

			break;
		case DISPLAY_SCREEN_INDOORRUN:
			if(m_remind_heartrate.enable )
			{
				SetValue.SportSet.SwIndoorRun |=  SW_HEARTRATE_MASK;
			}
			else
			{
				SetValue.SportSet.SwIndoorRun &=  ~SW_HEARTRATE_MASK;
			}
			break;

		case DISPLAY_SCREEN_CYCLING:
			if(m_remind_heartrate.enable )
			{
				SetValue.SportSet.SwCycling |=  SW_HEARTRATE_MASK;
			}
			else
			{
				SetValue.SportSet.SwCycling &=  ~SW_HEARTRATE_MASK;
			}
			break;

		case DISPLAY_SCREEN_CLIMBING:
			if(m_remind_heartrate.enable )
			{
				SetValue.SportSet.SwClimbing |=  SW_HEARTRATE_MASK;
			}
			else
			{
				SetValue.SportSet.SwClimbing &=  ~SW_HEARTRATE_MASK;
			}
			break;
		case DISPLAY_SCREEN_SWIMMING:
			if(m_remind_heartrate.enable )
			{
				SetValue.SportSet.SwSwimmg |=  SW_HEARTRATE_MASK;
			}
			else
			{
				SetValue.SportSet.SwSwimmg &=  ~SW_HEARTRATE_MASK;
			}
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			if(m_remind_heartrate.enable )
			{
				SetValue.SportSet.SwHike |=  SW_HEARTRATE_MASK;
			}
			else
			{
				SetValue.SportSet.SwHike &=  ~SW_HEARTRATE_MASK;
			}
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			if(m_remind_heartrate.enable )
			{
				SetValue.SportSet.SwCountryRace |=  SW_HEARTRATE_MASK;
			}
			else
			{
				SetValue.SportSet.SwCountryRace &=  ~SW_HEARTRATE_MASK;
			}
			break;
		default:
			break;
	}
  	 if(m_remind_heartrate.enable )
  	 {
		SetValue.AppSet.htr_zone_five_min = m_remind_heartrate.tens*10+m_remind_heartrate.ones;
		SetValue.HeartRateSet = 1;
	 }

}

//飞腕配速提醒设置界面
void gui_sport_remind_set_pace_paint(void)
{
		SetWord_t word = {0};
		char str[10];
	
		
		//设置背景黑色
		LCD_SetBackgroundColor(LCD_BLACK);
		
			
	    word.x_axis = 36;
	    word.y_axis = LCD_CENTER_JUSTIFIED;
	    word.size = LCD_FONT_24_SIZE;
	    word.forecolor = LCD_CYAN;
	    word.bckgrndcolor = LCD_NONE;
	    word.kerning = 0;
		LCD_SetString((char *)SportRemindStrs[SetValue.Language][2],&word);
	
	
			/*对称坐标点110*/
	   word.x_axis = 110 -16/2;
	   word.y_axis = 120 + 4;
	   word.size = LCD_FONT_16_SIZE;
	   word.forecolor = LCD_LIGHTGRAY;
	   word.bckgrndcolor = LCD_NONE;;	
		if(m_remind_pace.enable)
		{
			word.forecolor = LCD_WHITE;
	        LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
			LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
					
		}
		else
		{
		    word.forecolor = LCD_LIGHTGRAY;
		   LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
			LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
				
		}
		
		 word.x_axis = 130;
	     word.y_axis = LCD_CENTER_JUSTIFIED;
		 
	     word.size = LCD_FONT_56_SIZE;
		memset(str,0,10);	
		sprintf(str,"%d:%02d",m_remind_pace.min,m_remind_pace.sec);
		LCD_SetNumber(str,&word);	

		word.x_axis = 130 + 56 + 8 + 6 +6;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		LCD_SetString("/km",&word);
		
		switch(m_gui_sport_remind_set_pace_index)
		{
			/*case GUI_SPORT_REMIND_SET_PACE_SWITCH:{	
				LCD_SetRectangle(128,6,96,48,LCD_BLUE,0,0,LCD_FILL_ENABLE); 
			}break;*/
			case GUI_SPORT_REMIND_SET_PACE_MIN:{
				if(m_remind_pace.min >=10)
				{
					LCD_SetRectangle(194,6,120 - 2.25*29,2*29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(194,6,120 - 1.75*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
			}break; 	
			case GUI_SPORT_REMIND_SET_PACE_SEC:{
				if (m_remind_pace.min >=10)
				{
					LCD_SetRectangle(194,6,120+0.25*29,2*29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(194,6,120 - 0.25*29,2*29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
			}break; 
			default:
				break;
		}

}



//飞腕配速提醒设置界面初始化数据
void gui_sport_remind_set_pace_init(void)
{
	switch( ScreenSportSave )
	{
		case DISPLAY_SCREEN_RUN:
			m_remind_pace.enable = ((SetValue.SportSet.SwRun & SW_PACE_MASK) == SW_PACE_MASK) ? true:false;
			m_remind_pace.min = SetValue.SportSet.DefHintSpeedRun/60;
			m_remind_pace.sec = SetValue.SportSet.DefHintSpeedRun%60;
			break;

	  	case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			m_remind_pace.enable = ((SetValue.SportSet.SwHike & SW_PACE_MASK) == SW_PACE_MASK) ? true:false;
			m_remind_pace.min = SetValue.SportSet.DefHintSpeedHike/60;
			m_remind_pace.sec = SetValue.SportSet.DefHintSpeedHike%60;
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			m_remind_pace.enable = ((SetValue.SportSet.SwCountryRace & SW_PACE_MASK) == SW_PACE_MASK) ? true:false;
			m_remind_pace.min = SetValue.SportSet.DefHintSpeedCountryRace/60;
			m_remind_pace.sec = SetValue.SportSet.DefHintSpeedCountryRace%60;
			break;
		default:
			break;
	}

}

//飞腕配速提醒设置界面保存数据
void gui_sport_remind_set_pace_save(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
				if(m_remind_pace.enable )
				{
					SetValue.SportSet.SwRun |=	SW_PACE_MASK;
					SetValue.SportSet.DefHintSpeedRun = m_remind_pace.min*60+m_remind_pace.sec;
				}
				else
				{
					SetValue.SportSet.SwRun &=	~SW_PACE_MASK;
				}
				break;
			case DISPLAY_SCREEN_MARATHON:
				if(m_remind_pace.enable )
				{
					SetValue.SportSet.SwMarathon |=  SW_PACE_MASK;
					SetValue.SportSet.DefHintSpeedMarathon = m_remind_pace.min*60+m_remind_pace.sec;
				}
				else
				{
					SetValue.SportSet.SwMarathon &=  ~SW_PACE_MASK;
				}
	
				break;
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
				if(m_remind_pace.enable )
				{
					SetValue.SportSet.SwHike |=  SW_PACE_MASK;
					SetValue.SportSet.DefHintSpeedHike = m_remind_pace.min*60+m_remind_pace.sec;
				}
				else
				{
					SetValue.SportSet.SwHike &=  ~SW_PACE_MASK;
				}
	
				break;
				case DISPLAY_SCREEN_CROSSCOUNTRY:
				if(m_remind_pace.enable )
				{
					SetValue.SportSet.SwCountryRace |=  SW_PACE_MASK;
					SetValue.SportSet.DefHintSpeedCountryRace = m_remind_pace.min*60+m_remind_pace.sec;
				}
				else
				{
					SetValue.SportSet.SwCountryRace &=  ~SW_PACE_MASK;
				}
	
				break;
			default:
				break;
		}

}

//距离提醒设置界面
void gui_sport_remind_set_distance_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	
	

	LCD_SetBackgroundColor(LCD_BLACK);
	
		
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportRemindStrs[SetValue.Language][3],&word);
	
	
	word.x_axis = 110 -16/2;
	word.y_axis = 120 + 4;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE; 
	if(m_remind_distance.enable)
	{
		word.forecolor = LCD_WHITE;
	   LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
				
	}
	else
	{   word.forecolor = LCD_LIGHTGRAY;
		LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
			
	}
	
	word.x_axis = 130;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	memset(str,0,10);	
	sprintf(str,"%d.%d",m_remind_distance.km,m_remind_distance.hm);
	LCD_SetNumber(str,&word);	
	
	word.x_axis = 130 + 56 + 8 + 6 + 6;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("km",&word);
	
	switch(m_gui_sport_remind_set_distance_index)
	{
		/*case GUI_SPORT_REMIND_SET_DISTANCE_SWITCH:{ 
			LCD_SetRectangle(128,6,96,48,LCD_BLUE,0,0,LCD_FILL_ENABLE); 
		}break;*/
		case GUI_SPORT_REMIND_SET_DISTANCE_KM:{
			if(m_remind_distance.km >=10)
			{
				LCD_SetRectangle(194,6,120-1.75*29,2*29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
			else
			{
				LCD_SetRectangle(194,6,120-1.25*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
		}break; 	
		case GUI_SPORT_REMIND_SET_DISTANCE_HM:{
			if (m_remind_distance.km >=10)
			{
				LCD_SetRectangle(194,6,120+0.75*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
			else
			{
				LCD_SetRectangle(194,6,120+0.25*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
		}break; 
		default:
			break;
	}

}

//距离提醒设置界面初始化数据
void gui_sport_remind_set_distance_init(void)
{
	switch( ScreenSportSave )
	{
		case DISPLAY_SCREEN_RUN:
			m_remind_distance.enable = ((SetValue.SportSet.SwRun & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? true:false;
			m_remind_distance.km = SetValue.SportSet.DefHintDistanceRun/1000;
			m_remind_distance.hm = SetValue.SportSet.DefHintDistanceRun/100%10;
			break;
		case DISPLAY_SCREEN_CYCLING:
			m_remind_distance.enable = ((SetValue.SportSet.SwCycling & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? true:false;
			m_remind_distance.km = SetValue.SportSet.DefHintDistanceCycling/1000;
			m_remind_distance.hm = SetValue.SportSet.DefHintDistanceCycling/100%10;
			break;
			/*登山*/
		case DISPLAY_SCREEN_CLIMBING:
			m_remind_distance.enable = ((SetValue.SportSet.SwClimbing & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? true:false;
			m_remind_distance.km = SetValue.SportSet.DefHintDistanceClimbing/1000;
			m_remind_distance.hm = SetValue.SportSet.DefHintDistanceClimbing/100%10;
			break;
		case DISPLAY_SCREEN_SWIMMING:
			m_remind_distance.enable = ((SetValue.SportSet.SwSwimmg & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? true:false;
			m_remind_distance.km = SetValue.SportSet.DefHintDistanceSwimmg/1000;
			m_remind_distance.hm = SetValue.SportSet.DefHintDistanceSwimmg/100%10;
			break;
       	case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			m_remind_distance.enable = ((SetValue.SportSet.SwHike & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? true:false;
			m_remind_distance.km = SetValue.SportSet.DefHintDistanceHike/1000;
			m_remind_distance.hm = SetValue.SportSet.DefHintDistanceHike/100%10;
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			m_remind_distance.enable = ((SetValue.SportSet.SwCountryRace & SW_DISTANCE_MASK) == SW_DISTANCE_MASK) ? true:false;
			m_remind_distance.km = SetValue.SportSet.DefHintDistanceCountryRace/1000;
			m_remind_distance.hm = SetValue.SportSet.DefHintDistanceCountryRace/100%10;
			break;
		
		default:
			break;
	}
}



//距离提醒设置界面保存数据
void gui_sport_remind_set_distance_save(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
				if(m_remind_distance.enable )
				{
					SetValue.SportSet.SwRun |=	SW_DISTANCE_MASK;
					SetValue.SportSet.DefHintDistanceRun = m_remind_distance.km*1000+m_remind_distance.hm*100;
				}
				else
				{
					SetValue.SportSet.SwRun &=	~SW_DISTANCE_MASK;
				}
				break;
			case DISPLAY_SCREEN_MARATHON:
				if(m_remind_distance.enable )
				{
					SetValue.SportSet.SwMarathon |=  SW_DISTANCE_MASK;
					SetValue.SportSet.DefHintDistanceMarathon = m_remind_distance.km*1000+m_remind_distance.hm*100;
				}
				else
				{
					SetValue.SportSet.SwMarathon &=  ~SW_DISTANCE_MASK;
				}
	
				break;
			case DISPLAY_SCREEN_SWIMMING:
				if(m_remind_distance.enable )
				{
					SetValue.SportSet.SwSwimmg |=  SW_DISTANCE_MASK;
					SetValue.SportSet.DefCircleDistancOutdoorSwimmg = m_remind_distance.km*1000+m_remind_distance.hm*100;//先用室外游泳代替
				}
				else
				{
					SetValue.SportSet.SwSwimmg &=  ~SW_DISTANCE_MASK;
				}
				break;
				
			case DISPLAY_SCREEN_CYCLING:
				if(m_remind_distance.enable )
				{
					SetValue.SportSet.SwCycling |=  SW_DISTANCE_MASK;
					SetValue.SportSet.DefHintDistanceCycling = m_remind_distance.km*1000+m_remind_distance.hm*100;
				}
				else
				{
					SetValue.SportSet.SwCycling &=  ~SW_DISTANCE_MASK;
				}
				break;
				case DISPLAY_SCREEN_CLIMBING:
				if(m_remind_distance.enable )
				{
					SetValue.SportSet.SwClimbing |=  SW_DISTANCE_MASK;
					SetValue.SportSet.DefHintDistanceClimbing = m_remind_distance.km*1000+m_remind_distance.hm*100;
				}
				else
				{
					SetValue.SportSet.SwClimbing &=  ~SW_DISTANCE_MASK;
				}
				break;


				case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
				if(m_remind_distance.enable )
				{
					SetValue.SportSet.SwHike |=  SW_DISTANCE_MASK;
					SetValue.SportSet.DefHintDistanceHike = m_remind_distance.km*1000+m_remind_distance.hm*100;
				}
				else
				{
					SetValue.SportSet.SwHike &=  ~SW_DISTANCE_MASK;
				}
				break;
				case DISPLAY_SCREEN_CROSSCOUNTRY:
				if(m_remind_distance.enable )
				{
					SetValue.SportSet.SwCountryRace |=  SW_DISTANCE_MASK;
					SetValue.SportSet.DefHintDistanceCountryRace = m_remind_distance.km*1000+m_remind_distance.hm*100;
				}
				else
				{
					SetValue.SportSet.SwCountryRace &=  ~SW_DISTANCE_MASK;
				}
				break;
			default:
				break;
		}

}
//自动计圈设置界面
void gui_sport_remind_set_autocircle_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	
	
	LCD_SetBackgroundColor(LCD_BLACK);

	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportRemindStrs[SetValue.Language][4],&word);
	
	
	word.x_axis = 110 -16/2;
	word.y_axis = 120 + 4;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;  
	if(m_remind_autocircle.enable)
	{
		word.forecolor = LCD_WHITE;
	   LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
	   LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
				
	}
	else
	{   word.forecolor = LCD_LIGHTGRAY;
		LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
			
	}
	
	word.x_axis = 130;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	
	word.size = LCD_FONT_56_SIZE;
	memset(str,0,10);	
	sprintf(str,"%d.%d",m_remind_autocircle.km,m_remind_autocircle.hm);
	LCD_SetNumber(str,&word);	
	
	word.x_axis = 130 + 56 + 8 + 6 + 6;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("km",&word);
	
	switch(m_gui_sport_remind_set_autocircle_index)
	{
		/*case GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH:{ 
			LCD_SetRectangle(128,6,96,48,LCD_BLUE,0,0,LCD_FILL_ENABLE); 
		}break;*/
		case GUI_SPORT_REMIND_SET_AUTOCIRCLE_KM:{
			if(m_remind_autocircle.km >=10)
			{
				LCD_SetRectangle(194,6,120-1.75*29,2*29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
			else
			{
				LCD_SetRectangle(194,6,120-1.25*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
		}break; 	
		case GUI_SPORT_REMIND_SET_AUTOCIRCLE_HM:{
			if (m_remind_autocircle.km >=10)
			{
				LCD_SetRectangle(194,6,120+0.75*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
			else
			{
				LCD_SetRectangle(194,6,120+0.25*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
			}
		}break; 
		default:
			break;
	}

}

//自动计圈设置界面初始化数据
void gui_sport_remind_set_autocircle_init(void)
{
	switch( ScreenSportSave )
	{
		case DISPLAY_SCREEN_RUN:
			m_remind_autocircle.enable = ((SetValue.SportSet.SwRun & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? true:false;
			m_remind_autocircle.km = SetValue.SportSet.DefCircleDistanceRun/1000;
			m_remind_autocircle.hm = SetValue.SportSet.DefCircleDistanceRun/100%10;
			break;
		case DISPLAY_SCREEN_CYCLING:
		    m_remind_autocircle.enable = ((SetValue.SportSet.SwCycling & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? true:false;
			m_remind_autocircle.km = SetValue.SportSet.DefCircleDistanceCycling/1000;
			m_remind_autocircle.hm = SetValue.SportSet.DefCircleDistanceCycling/100%10;
			break;
		case DISPLAY_SCREEN_CLIMBING:
		    m_remind_autocircle.enable = ((SetValue.SportSet.SwClimbing & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? true:false;
			m_remind_autocircle.km = SetValue.SportSet.DefCircleDistanceClimbing/1000;
			m_remind_autocircle.hm = SetValue.SportSet.DefCircleDistanceClimbing/100%10;
			break;
		case DISPLAY_SCREEN_SWIMMING:
			m_remind_autocircle.enable = ((SetValue.SportSet.SwSwimmg & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? true:false;
			m_remind_autocircle.km = SetValue.SportSet.DefCircleDistancSwimmg/1000;
			m_remind_autocircle.hm = SetValue.SportSet.DefCircleDistancSwimmg/100%10;
			break;

	case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			m_remind_autocircle.enable = ((SetValue.SportSet.SwHike & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? true:false;
			m_remind_autocircle.km = SetValue.SportSet.DefCircleDistanceHike/1000;
			m_remind_autocircle.hm = SetValue.SportSet.DefCircleDistanceHike/100%10;
			break;

	case DISPLAY_SCREEN_CROSSCOUNTRY:
			m_remind_autocircle.enable = ((SetValue.SportSet.SwCountryRace & SW_CIRCLEDISTANCE_MASK) == SW_CIRCLEDISTANCE_MASK) ? true:false;
			m_remind_autocircle.km = SetValue.SportSet.DefCircleDistanceCountryRace/1000;
			m_remind_autocircle.hm = SetValue.SportSet.DefCircleDistanceCountryRace/100%10;
			break;
			default:
				break;
		}
			
}
			
//自动计圈设置界面保存数据
void gui_sport_remind_set_autocircle_save(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
				if(m_remind_autocircle.enable )
				{
					SetValue.SportSet.SwRun |=	SW_CIRCLEDISTANCE_MASK;
					SetValue.SportSet.DefCircleDistanceRun = m_remind_autocircle.km*1000+m_remind_autocircle.hm*100;
				}
				else
				{
					SetValue.SportSet.SwRun &=	~SW_CIRCLEDISTANCE_MASK;
				}
				break;

		case DISPLAY_SCREEN_CYCLING:
				if(m_remind_autocircle.enable)
				{
					SetValue.SportSet.SwCycling |=  SW_CIRCLEDISTANCE_MASK;
					SetValue.SportSet.DefCircleDistanceCycling = m_remind_autocircle.km*1000+m_remind_autocircle.hm*100;
				}
				else
				{
					SetValue.SportSet.SwCycling &=  ~SW_CIRCLEDISTANCE_MASK;
				}
				break;

		 case DISPLAY_SCREEN_CLIMBING:
				if(m_remind_autocircle.enable )
				{
					SetValue.SportSet.SwClimbing |=  SW_CIRCLEDISTANCE_MASK;
					SetValue.SportSet.DefCircleDistanceClimbing = m_remind_autocircle.km*1000+m_remind_autocircle.hm*100;
				}
				else
				{
					SetValue.SportSet.SwClimbing &=  ~SW_CIRCLEDISTANCE_MASK;
				}
				break;
			case DISPLAY_SCREEN_SWIMMING:
				if(m_remind_autocircle.enable )
				{
					SetValue.SportSet.SwSwimmg |=  SW_CIRCLEDISTANCE_MASK;
					SetValue.SportSet.DefCircleDistancSwimmg = m_remind_autocircle.km*1000+m_remind_autocircle.hm*100;
				}
				else
				{
					SetValue.SportSet.SwSwimmg &=  ~SW_CIRCLEDISTANCE_MASK;
				}
	
				break;
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
				if(m_remind_autocircle.enable )
				{
					SetValue.SportSet.SwHike |=  SW_CIRCLEDISTANCE_MASK;
					SetValue.SportSet.DefCircleDistanceHike = m_remind_autocircle.km*1000+m_remind_autocircle.hm*100;
				}
				else
				{
					SetValue.SportSet.SwHike &=  ~SW_CIRCLEDISTANCE_MASK;
				}
	
				break;

			case DISPLAY_SCREEN_CROSSCOUNTRY:
				if(m_remind_autocircle.enable )
				{
					SetValue.SportSet.SwCountryRace |=	SW_CIRCLEDISTANCE_MASK;
					SetValue.SportSet.DefCircleDistanceCountryRace = m_remind_autocircle.km*1000+m_remind_autocircle.hm*100;
				}
				else
				{
					SetValue.SportSet.SwCountryRace &=	~SW_CIRCLEDISTANCE_MASK;
				}
				break;
			default:
				break;
		}		
}

//燃脂目标设置界面
void gui_sport_remind_set_goalkcal_paint(void)
{
		SetWord_t word = {0};
		char str[10];
		uint16_t kcal_value;
		
		//设置背景白色
		LCD_SetBackgroundColor(LCD_WHITE);
		//上半部黑色
		LCD_SetRectangle(0,80,0,LCD_LINE_CNT_MAX,LCD_BLACK,0,0,LCD_FILL_ENABLE);
			
		word.x_axis = 36;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
		
		LCD_SetString((char *)SportRemindStrs[SetValue.Language][4],&word);
		
		
		word.x_axis = 100;
		word.forecolor = LCD_BLACK; 
		if(m_remind_goalkcal.enable)
		{
		
			LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
					
		}
		else
		{
			LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
				
		}
		kcal_value = 100*m_remind_goalkcal.hundreds + 10*m_remind_goalkcal.tens + m_remind_goalkcal.ones;
		
		word.x_axis = 150;
		word.size = LCD_FONT_48_SIZE;
		memset(str,0,10);	
		sprintf(str,"%d",kcal_value);
		LCD_SetNumber(str,&word);	
		
		word.x_axis = 186;
		word.y_axis = 176;
		word.size = LCD_FONT_16_SIZE;
		LCD_SetString("kcal",&word);
		
		switch(m_gui_sport_remind_set_goalkcal_index)
		{
			case GUI_SPORT_REMIND_SET_GOALKCAL_SWITCH:{ 
				LCD_SetRectangle(128,6,96,48,LCD_BLUE,0,0,LCD_FILL_ENABLE); 
			}break;
			case GUI_SPORT_REMIND_SET_GOALKCAL_HUNDREDS:{
				if(m_remind_goalkcal.hundreds>=10)
				{
					LCD_SetRectangle(210,6,72,45,LCD_BLUE,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(210,6,80,25,LCD_BLUE,0,0,LCD_FILL_ENABLE);
				}
			}break; 	
			case GUI_SPORT_REMIND_SET_GOALKCAL_TENS:{
				if (m_remind_goalkcal.hundreds >=10)
				{
					LCD_SetRectangle(210,6,121,25,LCD_BLUE,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(210,6,107,25,LCD_BLUE,0,0,LCD_FILL_ENABLE);
				}
			
			}break; 
			case GUI_SPORT_REMIND_SET_GOALKCAL_ONES:{
				if (m_remind_goalkcal.hundreds >=10)
				{
					LCD_SetRectangle(210,6,147,25,LCD_BLUE,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(210,6,134,25,LCD_BLUE,0,0,LCD_FILL_ENABLE);
				}
			}break;
			default:
				break;
		}
	
}

//燃脂目标设置界面初始化数据
void gui_sport_remind_set_goalkcal_init(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_WALK:
				m_remind_goalkcal.enable = ((SetValue.SportSet.SwWalking & SW_GOALKCAL_MASK) == SW_GOALKCAL_MASK) ? true:false;
				m_remind_goalkcal.hundreds = SetValue.SportSet.DefWalkingFatBurnGoal/100;
				m_remind_goalkcal.tens = SetValue.SportSet.DefWalkingFatBurnGoal/10%10;
				m_remind_goalkcal.ones = SetValue.SportSet.DefWalkingFatBurnGoal%10;
				break;
			default:
				break;
		}		

}

//燃脂目标设置界面保存数据
void gui_sport_remind_set_goalkcal_save(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_WALK:
				if(m_remind_goalkcal.enable )
				{
					SetValue.SportSet.SwWalking |=  SW_GOALKCAL_MASK;
					SetValue.SportSet.DefWalkingFatBurnGoal = m_remind_goalkcal.hundreds*100+m_remind_goalkcal.tens*10 +m_remind_goalkcal.ones;
				}
				else
				{
					SetValue.SportSet.SwWalking &=  ~SW_GOALKCAL_MASK;
				}
	
				break;
			default:
				break;
		}	

}

//时间提醒设置界面
void gui_sport_remind_set_time_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	uint16_t time_value;
	

	LCD_SetBackgroundColor(LCD_BLACK);
	
		
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportRemindStrs[SetValue.Language][8],&word);
		
		
	word.x_axis = 110 -16/2;
	word.y_axis = 120 + 4;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE; 
	if(m_remind_time.enable)
		{
		
		word.forecolor = LCD_WHITE;
	   LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
					
		}
		else
		{
			word.forecolor = LCD_LIGHTGRAY;
		LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
				
		}
		
		time_value = 10*m_remind_time.tens + m_remind_time.ones;
		
		word.x_axis = 130;
	    word.y_axis = LCD_CENTER_JUSTIFIED;
		
	   word.size = LCD_FONT_56_SIZE;
		memset(str,0,10);	
		sprintf(str,"%d",time_value);
		LCD_SetNumber(str,&word);	
		
		word.x_axis = 130 + 56 + 8 + 6 + 6;
	    word.y_axis = LCD_CENTER_JUSTIFIED;
	    word.size = LCD_FONT_24_SIZE;
		LCD_SetString("min",&word);
		
		switch(m_gui_sport_remind_set_time_index)
		{
			/*case GUI_SPORT_REMIND_SET_TIME_SWITCH:{ 
				LCD_SetRectangle(128,6,96,48,LCD_BLUE,0,0,LCD_FILL_ENABLE); 
			}break;*/
		
			case GUI_SPORT_REMIND_SET_TIME_TENS:{
				if (m_remind_time.tens >=10)
				{
					LCD_SetRectangle(194,6,120 - 1 - 1.5*29,29*2,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(194,6,120 - 1 - 29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
			
			}break; 
			case GUI_SPORT_REMIND_SET_TIME_ONES:{
				if (m_remind_time.tens >=10)
				{
					LCD_SetRectangle(194,6,121+0.5*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(194,6,121,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
			}break;
			default:
				break;
		}
	
}

//时间提醒设置界面初始化数据
void gui_sport_remind_set_time_init(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
					m_remind_time.enable = ((SetValue.SportSet.SwRun & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? true:false;
					m_remind_time.tens = SetValue.SportSet.DefHintTimeRun/60/10;
					m_remind_time.ones = SetValue.SportSet.DefHintTimeRun/60%10;
				break;
			case DISPLAY_SCREEN_CYCLING:
					m_remind_time.enable = ((SetValue.SportSet.SwCycling & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? true:false;
					m_remind_time.tens = SetValue.SportSet.DefHintTimeCycling/60/10;
					m_remind_time.ones = SetValue.SportSet.DefHintTimeCycling/60%10;
				break;
			case DISPLAY_SCREEN_CLIMBING:
					m_remind_time.enable = ((SetValue.SportSet.SwClimbing & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? true:false;
					m_remind_time.tens = SetValue.SportSet.DefHintTimeClimbing/60/10;
					m_remind_time.ones = SetValue.SportSet.DefHintTimeClimbing/60%10;
				break;
			case DISPLAY_SCREEN_SWIMMING:
					m_remind_time.enable = ((SetValue.SportSet.SwSwimmg & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? true:false;
					m_remind_time.tens = SetValue.SportSet.DefHintTimeSwimming/60/10;
					m_remind_time.ones = SetValue.SportSet.DefHintTimeSwimming/60%10;
				break;
                  
				case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
					m_remind_time.enable = ((SetValue.SportSet.SwHike & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? true:false;
					m_remind_time.tens = SetValue.SportSet.DefHintTimeHike/60/10;
					m_remind_time.ones = SetValue.SportSet.DefHintTimeHike/60%10;
				break;
					case DISPLAY_SCREEN_CROSSCOUNTRY:
					m_remind_time.enable = ((SetValue.SportSet.SwCountryRace & SW_GOALTIME_MASK) == SW_GOALTIME_MASK) ? true:false;
					m_remind_time.tens = SetValue.SportSet.DefHintTimeCountryRace/60/10;
					m_remind_time.ones = SetValue.SportSet.DefHintTimeCountryRace/60%10;
				break;
				default:
						break;
	}		
				
}
				
//时间提醒设置界面保存数据
void gui_sport_remind_set_time_save(void)
{
	switch( ScreenSportSave )
	{

	     	case DISPLAY_SCREEN_RUN:
				if(m_remind_time.enable )
				{
					SetValue.SportSet.SwRun |=  SW_GOALTIME_MASK;
					SetValue.SportSet.DefHintTimeRun = (m_remind_time.tens*10 +m_remind_time.ones)*60;
				}
				else
				{
					SetValue.SportSet.SwRun &=  ~SW_GOALTIME_MASK;
				}
										
			break;
		case DISPLAY_SCREEN_WALK:
				if(m_remind_time.enable )
				{
					SetValue.SportSet.SwWalking |=  SW_GOALTIME_MASK;
					SetValue.SportSet.DefHintTimeWalking = (m_remind_time.tens*10 +m_remind_time.ones)*60;
				}
				else
				{
					SetValue.SportSet.SwWalking &=  ~SW_GOALTIME_MASK;
				}
					
			break;
		case DISPLAY_SCREEN_SWIMMING:
				if(m_remind_time.enable )
				{
					SetValue.SportSet.SwSwimmg |=  SW_GOALTIME_MASK;
					SetValue.SportSet.DefHintTimeSwimming = (m_remind_time.tens*10 +m_remind_time.ones)*60;
				}
				else
				{
					SetValue.SportSet.SwSwimmg &=  ~SW_GOALTIME_MASK;
				}
										
			break;
		
           case DISPLAY_SCREEN_CYCLING:
				if(m_remind_time.enable )
				{
					SetValue.SportSet.SwCycling |=  SW_GOALTIME_MASK;
					SetValue.SportSet.DefHintTimeCycling = (m_remind_time.tens*10 +m_remind_time.ones)*60;
				}
				else
				{
					SetValue.SportSet.SwCycling &=  ~SW_GOALTIME_MASK;
				}
										
			break;
			case DISPLAY_SCREEN_CLIMBING:
				if(m_remind_time.enable )
				{
					SetValue.SportSet.SwClimbing |=  SW_GOALTIME_MASK;
					SetValue.SportSet.DefHintTimeClimbing = (m_remind_time.tens*10 +m_remind_time.ones)*60;
				}
				else
				{
					SetValue.SportSet.SwClimbing &=  ~SW_GOALTIME_MASK;
				}
										
			break;
           
		   case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
				 if(m_remind_time.enable )
					 {
						SetValue.SportSet.SwHike |=  SW_GOALTIME_MASK;
						SetValue.SportSet.DefHintTimeHike = (m_remind_time.tens*10 +m_remind_time.ones)*60;
					  }
				  else
					 {
						SetValue.SportSet.SwHike &=  ~SW_GOALTIME_MASK;
					 }
												  
					  break;
			 case DISPLAY_SCREEN_CROSSCOUNTRY:
				if(m_remind_time.enable )
					 {
						 SetValue.SportSet.SwCountryRace |=  SW_GOALTIME_MASK;
						 SetValue.SportSet.DefHintDistanceCountryRace = (m_remind_time.tens*10 +m_remind_time.ones)*60;
					 }
				  else
					 {
						 SetValue.SportSet.SwCountryRace &=  ~SW_GOALTIME_MASK;
					 }
												  
				  break;
			
			
		default:
			break;
	}	
				
}
//速度提醒设置界面
void gui_sport_remind_set_speed_paint(void)
{
		SetWord_t word = {0};
	char str[10];
	
	

	LCD_SetBackgroundColor(LCD_BLACK);
	
		
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
		LCD_SetString((char *)SportRemindStrs[SetValue.Language][SPORT_REMIND_SET_SPEED],&word);
	
	
		word.x_axis = 110 -16/2;
	word.y_axis = 120 + 4;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE; 
		if(m_remind_speed.enable)
		{
	       word.forecolor = LCD_WHITE;
	       LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
			LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
					
		}
		else
		{
		     word.forecolor = LCD_LIGHTGRAY;
		   LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
			LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
				
		}
		
			word.x_axis = 130;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
		memset(str,0,10);	
		sprintf(str,"%d.%d",m_remind_speed.tens,m_remind_speed.ones);
		LCD_SetNumber(str,&word);	

		word.x_axis = 130 + 56 + 8 + 6 + 6;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
		LCD_SetString("km/h",&word);
		
		switch(m_gui_sport_remind_set_speed_index)
		{
			/*case GUI_SPORT_REMIND_SET_SPEED_SWITCH:{	
				LCD_SetRectangle(128,6,96,48,LCD_BLUE,0,0,LCD_FILL_ENABLE); 
			}break;*/
			case GUI_SPORT_REMIND_SET_SPEED_TENS:{
				if(m_remind_speed.tens>=10)
				{
					LCD_SetRectangle(194,6,120-1.75*29,2*29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(194,6,120-1.25*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
				
			}break; 	
			case GUI_SPORT_REMIND_SET_SPEED_ONES:{
				if(m_remind_speed.tens>=10)
				{
					LCD_SetRectangle(194,6,120+0.75*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
				else
				{
					LCD_SetRectangle(194,6,120+0.25*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				}
				
			}break; 
			default:
				break;
		}

}



//速度提醒设置界面初始化数据
void gui_sport_remind_set_speed_init(void)
{
	switch( ScreenSportSave )
	{
		case DISPLAY_SCREEN_CYCLING:
			m_remind_speed.enable = ((SetValue.SportSet.SwCycling & SW_PACE_MASK) == SW_PACE_MASK) ? true:false;
			m_remind_speed.tens = SetValue.SportSet.DefHintSpeedCycling/10;
			m_remind_speed.ones = SetValue.SportSet.DefHintSpeedCycling%10;
			break;
    
		default:
			break;
	}

}

//飞腕速度提醒设置界面保存数据
void gui_sport_remind_set_speed_save(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_CYCLING:
				if(m_remind_speed.enable )
				{
					SetValue.SportSet.SwCycling |=	SW_PACE_MASK;
					SetValue.SportSet.DefHintSpeedCycling = m_remind_speed.tens*10 + m_remind_speed.ones;
				}
				else
				{
					SetValue.SportSet.SwCycling &=	~SW_PACE_MASK;
				}
				break;
			default:
				break;
		}

}


//泳池长度设置界面
void gui_sport_remind_set_swimpool_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	uint16_t tmp_value;
	
	LCD_SetBackgroundColor(LCD_BLACK);

		
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
	LCD_SetString((char *)SportRemindStrs[SetValue.Language][SPORT_REMIND_SET_SWIMPOOL],&word);
	
	
	word.x_axis = 130;
    word.y_axis = LCD_CENTER_JUSTIFIED;
	
	word.forecolor = LCD_WHITE;
	word.size = LCD_FONT_56_SIZE;
    tmp_value = 10*m_remind_swimpool.tens + m_remind_swimpool.ones;
	memset(str,0,10);	
	sprintf(str,"%02d",tmp_value);
	LCD_SetNumber(str,&word);	

      

	switch(m_gui_sport_remind_set_swimpool_index)
	{

		case GUI_SPORT_REMIND_SET_SWIMPOOL_TENS:{
			
			//LCD_SetRectangle(200,6,81,38,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
			LCD_SetRectangle(194,6,120 - 1 - 29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
		
		}break; 
		case GUI_SPORT_REMIND_SET_SWIMPOOL_ONES:{
				
			//LCD_SetRectangle(200,6,121,38,LCD_BLUE,0,0,LCD_FILL_ENABLE);
			LCD_SetRectangle(194,6,121,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
		}break;
		default:
			break;
	}	
	       word.x_axis = 130 + 56 + 8 + 6 + 6;
		   word.y_axis = LCD_CENTER_JUSTIFIED;
		   word.size = LCD_FONT_24_SIZE;
		   LCD_SetString("m",&word);

}


//泳池长度设置界面初始化数据
void gui_sport_remind_set_swimpool_init(void)
{

	m_remind_swimpool.tens = SetValue.AppGet.LengthSwimmingPool/10;
	m_remind_swimpool.ones = SetValue.AppGet.LengthSwimmingPool%10;
}

//泳池长度设置界面保存数据
void gui_sport_remind_set_swimpool_save(void)
{
	SetValue.AppGet.LengthSwimmingPool = m_remind_swimpool.tens*10 + m_remind_swimpool.ones;	
}


//室内跑长度设置界面
void gui_sport_remind_set_indoorleng_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	uint16_t tmp_value;
	
	//设置背景白色
	LCD_SetBackgroundColor(LCD_WHITE);
	//上半部黑色
	LCD_SetRectangle(0,80,0,LCD_LINE_CNT_MAX,LCD_BLACK,0,0,LCD_FILL_ENABLE);
		
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	
	LCD_SetString((char *)SportRemindStrs[SetValue.Language][SPORT_REMIND_SET_INDOORRUNLENG],&word);
	
	
	word.x_axis = 100;
	word.forecolor = LCD_BLACK; 

	tmp_value = 10*m_remind_indoorleng.tens + m_remind_indoorleng.ones;
	
	word.x_axis = 110;
	word.size = LCD_FONT_88_SIZE;
	memset(str,0,10);	
	sprintf(str,"%d",tmp_value);
	LCD_SetNumber(str,&word);
	
	word.x_axis = 180;
	word.y_axis = 185;
	word.size = LCD_FONT_16_SIZE;
	word.kerning = 1;
	LCD_SetString("cm",&word);

	switch(m_gui_sport_remind_set_indoorleng_index)
	{

		case GUI_SPORT_REMIND_SET_INDOORLENG_TENS:{
			if(m_remind_indoorleng.tens >=10)
			{
				LCD_SetRectangle(200,6,61,76,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
			}
			else
			{
				LCD_SetRectangle(200,6,81,38,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
			}
		
		}break; 
		case GUI_SPORT_REMIND_SET_INDOORLENG_ONES:{
			if(m_remind_indoorleng.tens >=10)
			{
				LCD_SetRectangle(200,6,141,38,LCD_BLUE,0,0,LCD_FILL_ENABLE);	
			}
			else
			{	
			LCD_SetRectangle(200,6,121,38,LCD_BLUE,0,0,LCD_FILL_ENABLE);
			}
		}break;
		default:
			break;
	}	

}

//高度提醒设置界面
void gui_sport_remind_set_altitude_paint(void)
{
	SetWord_t word = {0};
	char str[10];
	
	uint16_t altit_value;
	

	LCD_SetBackgroundColor(LCD_BLACK);
	
		
	word.x_axis = 36;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 0;
	LCD_SetString((char *)SportRemindStrs[SetValue.Language][6],&word);
	
	
	word.x_axis = 110 -16/2;
	word.y_axis = 120 + 4;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE; 
	if(m_remind_altitude.enable)
	{
	    word.forecolor = LCD_CYAN;
	   LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
				
	}
	else
	{   word.forecolor = LCD_LIGHTGRAY;
		LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
		LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
			
	}
	
	word.x_axis = 130;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	altit_value = m_remind_altitude.thousand*1000 + m_remind_altitude.hundreds*100;
	memset(str,0,10);	
	sprintf(str,"%d",altit_value);
	LCD_SetNumber(str,&word);	
	
	word.x_axis = 130 + 56 + 8 + 6 + 6;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("m",&word);
	
	switch(m_gui_sport_remind_set_altitude_index)
	{
	
		case GUI_SPORT_REMIND_SET_ALTITUDE_THOUS:{
				LCD_SetRectangle(194,6,120 - 1 - 2*29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
		}break; 	
		case GUI_SPORT_REMIND_SET_ALTITUDE_HUND:{
				LCD_SetRectangle(194,6,120 - 1 - 29,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
		}break; 
		default:
			break;
	}

}

//登山高度提醒设置界面初始化数据
void gui_sport_remind_set_altitude_init(void)
{
			m_remind_altitude.enable = ((SetValue.SportSet.SwClimbing & SW_ALTITUDE_MASK) == SW_ALTITUDE_MASK) ? true:false;
			m_remind_altitude.thousand = SetValue.SportSet.DefHintClimbingHeight/1000;
			m_remind_altitude.hundreds = (SetValue.SportSet.DefHintClimbingHeight/100)%10;
		

	
}

void gui_sport_remind_set_altitude_save(void)
{
if(m_remind_altitude.enable)
{
	SetValue.SportSet.SwClimbing |=	SW_ALTITUDE_MASK;

   SetValue.SportSet.DefHintClimbingHeight = m_remind_altitude.thousand*1000 + m_remind_altitude.hundreds*100;
}
else
{
	SetValue.SportSet.SwClimbing &=	~	SW_ALTITUDE_MASK;


}




}

//室内跑步长设置界面初始化数据
void gui_sport_remind_set_indoorleng_init(void)
{

	m_remind_indoorleng.tens = SetValue.AppGet.StepLengthIndoorRun/10;
	m_remind_indoorleng.ones = SetValue.AppGet.StepLengthIndoorRun%10;
}

//室内跑步长设置界面保存数据
void gui_sport_remind_set_indoorleng_save(void)
{
	SetValue.AppGet.StepLengthIndoorRun = m_remind_indoorleng.tens*10 + m_remind_indoorleng.ones;
	SetValue.IndoorLengSet = 1;
}

//计圈提醒界面
void gui_sport_remind_set_goalcircle_paint(void)
	{
		SetWord_t word = {0};
		char str[10];
		uint8_t temp_value;
		
		LCD_SetBackgroundColor(LCD_BLACK);
			
		
		word.x_axis = 36;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.size = LCD_FONT_24_SIZE;
		word.forecolor = LCD_CYAN;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 0;
	
		LCD_SetString((char *)SportRemindStrs[SetValue.Language][9],&word);
		
		
		
		word.x_axis = 110 -16/2;
			word.y_axis = 120 + 4;
			word.size = LCD_FONT_16_SIZE;
			word.forecolor = LCD_LIGHTGRAY;
			word.bckgrndcolor = LCD_NONE;;	
	
		if(m_remind_goalcircle.enable)
		{
		
			  word.forecolor = LCD_WHITE;
			  LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_open);
			  LCD_SetString((char *)SportReadyStrs[SetValue.Language][7],&word);
					
		}
		else
		{
			word.forecolor = LCD_LIGHTGRAY;
			LCD_SetPicture(110 -11/2,120 - 4 - 27 ,LCD_NONE,LCD_NONE,&img_custom_sport_level_close);
			LCD_SetString((char *)SportReadyStrs[SetValue.Language][6],&word);
				
		}
		temp_value = m_remind_goalcircle.tens *10+m_remind_goalcircle.ones;
		
		
		word.x_axis = 130;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		
		//word.forecolor = LCD_WHITE;
		word.size = LCD_FONT_56_SIZE;
		memset(str,0,10);	
		sprintf(str,"%02d",temp_value);
		LCD_SetNumber(str,&word);
		
		switch(m_gui_sport_remind_set_goalcircle_index)
		{
			/*case GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH:{ 
				LCD_SetRectangle(128,6,96,48,LCD_BLUE,0,0,LCD_FILL_ENABLE); 
			}break;*/
			case GUI_SPORT_REMIND_SET_GOALCIRCLE_TENS:{
				
				LCD_SetRectangle(194,6,91,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				
			}break; 	
			case GUI_SPORT_REMIND_SET_GOALCIRCLE_ONES:{
				
				LCD_SetRectangle(194,6,120,29,LCD_CYAN,0,0,LCD_FILL_ENABLE);
				
			}break; 
			default:
				break;
		}
			word.x_axis = 208;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.size = LCD_FONT_16_SIZE;
			LCD_SetString((char *)SportReadyStrs[SetValue.Language][9],&word);
	
	}



//计圈提醒设置界面初始化数据
void gui_sport_remind_set_goalcircle_init(void)
{
	switch( ScreenSportSave )
	{
		case DISPLAY_SCREEN_SWIMMING:
			m_remind_goalcircle.enable = ((SetValue.SportSet.SwSwimmg & SW_GOALCIRCLE_MASK) == SW_GOALCIRCLE_MASK) ? true:false;
			m_remind_goalcircle.tens = SetValue.SportSet.DefHintCircleSwimming/10;
			m_remind_goalcircle.ones = SetValue.SportSet.DefHintCircleSwimming%10;
			break;
			default:
				break;
		}
			
}

//计圈提醒设置界面保存数据
void gui_sport_remind_set_goalcircle_save(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_SWIMMING:
				if(m_remind_goalcircle.enable )
				{
					SetValue.SportSet.SwSwimmg |=	SW_GOALCIRCLE_MASK;
					SetValue.SportSet.DefHintCircleSwimming = m_remind_goalcircle.tens*10+m_remind_goalcircle.ones;
				}
				else
				{
					SetValue.SportSet.SwSwimmg &=	~SW_GOALCIRCLE_MASK;
				}
				break;
			default:
				break;
		}		
}	


void gui_sport_start_init(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
				ScreenState = DISPLAY_SCREEN_RUN;
				g_sport_status = SPORT_STATUS_START;
			    gui_sport_run_init();
				break;
			case DISPLAY_SCREEN_WALK:
				ScreenState = DISPLAY_SCREEN_WALK;
				g_sport_status = SPORT_STATUS_START;
			    gui_sport_walk_init();
				break;
			case DISPLAY_SCREEN_MARATHON:
				ScreenState = DISPLAY_SCREEN_MARATHON;
				g_sport_status = SPORT_STATUS_START;
			    gui_sport_marathon_init();
				break;
			case DISPLAY_SCREEN_CYCLING://骑行
				ScreenState = DISPLAY_SCREEN_CYCLING;
				g_sport_status = SPORT_STATUS_START;
			    gui_sport_cycling_init();
				break;
				
			case DISPLAY_SCREEN_CLIMBING://登山
			   ScreenState = DISPLAY_SCREEN_CLIMBING;
				g_sport_status = SPORT_STATUS_START;
				gui_sport_climbing_init();
				break;

			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE://徒步越野
			   ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
				g_sport_status = SPORT_STATUS_START;
				gui_sport_crosscountry_hike_init();
				break;

			
			case DISPLAY_SCREEN_CROSSCOUNTRY://越野跑
			   ScreenState = DISPLAY_SCREEN_CROSSCOUNTRY;
				g_sport_status = SPORT_STATUS_START;
				gui_sport_crosscountry_init();
				
				break;
			case DISPLAY_SCREEN_SWIMMING:
				ScreenState = DISPLAY_SCREEN_SWIMMING;
				g_sport_status = SPORT_STATUS_START;
			
			    gui_sport_swim_init();
				if(drv_ublox_status_get()!=0)//如果开启则关闭
					{
				     CloseGPSTask();
					}
				else;
				
				break;
			case DISPLAY_SCREEN_INDOORRUN:
				ScreenState = DISPLAY_SCREEN_INDOORRUN;
				g_sport_status = SPORT_STATUS_START;
			    gui_sport_indoorrun_init();
					if(drv_ublox_status_get()!=0)
					{
				     CloseGPSTask();
					}
				else;
				break;
			default:
				break;
		}
		//刷新界面
		gui_swich_msg();
		//振动提示
		gui_sport_motor_prompt();
				
		//创建运动任务
       StartSport();
		
	GUI_SPORT_PRINTF(" StartSport\r\n");
}

//初始化各运动提醒设置项
void gui_sport_remind_set_init(void)
{
	uint8_t i;
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
				for (i=0;i < RUN_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_run_remind_set_option[i];
				}

				m_sport_remind_set_sum = RUN_REMIND_SET_SUM;
				
				break;
			/*case DISPLAY_SCREEN_WALK:
				for (i=0;i < WALK_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_walk_remind_set_option[i];
				}
				
				m_sport_remind_set_sum = WALK_REMIND_SET_SUM;

				break;
			case DISPLAY_SCREEN_MARATHON:
				for (i=0;i < MARATHON_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_marathon_remind_set_option[i];
				}
				
				m_sport_remind_set_sum = MARATHON_REMIND_SET_SUM;
				break;*/
			case DISPLAY_SCREEN_CYCLING:
				for (i=0;i < CYCLING_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_cycling_remind_set_option[i];
				}
				
				m_sport_remind_set_sum = CYCLING_REMIND_SET_SUM;
				break;
			case DISPLAY_SCREEN_SWIMMING:
				for (i=0;i < SWIM_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_swim_remind_set_option[i];
				}
							
				m_sport_remind_set_sum = SWIM_REMIND_SET_SUM;
		
				break;
            /*登山*/
			case DISPLAY_SCREEN_CLIMBING:
				for (i=0;i < CLIMBING_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_climbing_remind_set_option[i];
				}
				
				m_sport_remind_set_sum = CLIMBING_REMIND_SET_SUM;
				break;
			/*case DISPLAY_SCREEN_INDOORRUN:
				for (i=0;i < INDOORRUN_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_indoorrun_remind_set_option[i];
				}
				
				m_sport_remind_set_sum = INDOORRUN_REMIND_SET_SUM;
				break;*/
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
				for (i=0;i < HIKE_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_hike_remind_set_option[i];
				}
				
				m_sport_remind_set_sum = HIKE_REMIND_SET_SUM;
				break;
			case DISPLAY_SCREEN_CROSSCOUNTRY:
				for (i=0;i < CROSSCOUNTRY_REMIND_SET_SUM;i++)
				{
					m_sport_remind_set_buf[i] = m_crosscountry_remind_set_option[i];
				}
				
				m_sport_remind_set_sum = CROSSCOUNTRY_REMIND_SET_SUM;
				break;
			default:
				break;
		}

}


//初始化各运动数据显示项
void gui_sport_data_display_init(void)
{
	uint8_t i;
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
				/*先读取之前的配置*/

			
              
			
				for (i=0;i < SPORT_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_buf[i] = m_run_data_display_buf[i];
				}
					
				break;
			/*case DISPLAY_SCREEN_WALK:
				for (i=0;i < SPORT_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_buf[i] = m_walk_data_display_buf[i];
				}
				break;
			case DISPLAY_SCREEN_MARATHON:
				for (i=0;i < SPORT_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_buf[i] = m_marathon_data_display_buf[i];
				}
				break;*/
			case DISPLAY_SCREEN_CYCLING:
				
				for (i=0;i < SPORT_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_buf[i] = m_cycling_data_display_buf[i];
					
				}
				break;
			
			case DISPLAY_SCREEN_CLIMBING://登山
				for (i=0;i < SPORT_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_buf[i] = m_climbing_data_display_buf[i];
				}
				break;
			case DISPLAY_SCREEN_SWIMMING:
				for (i=0;i < SPORT_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_buf[i] = m_swim_data_display_buf[i];
				}
				break;
			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
				for (i=0;i < SPORT_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_buf[i] = m_hike_data_display_buf[i];
				}
				break;
			case DISPLAY_SCREEN_CROSSCOUNTRY:
				for (i=0;i < SPORT_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_buf[i] = m_crosscountry_data_display_buf[i];
				}
				break;
			default:
			
				break;
		}

}
void gui_sport_pause_swich(void)
{
	if (Get_PauseSta()==false)
	{
		g_sport_status = SPORT_STATUS_PAUSE;
		Set_PauseSta(true);
	}
	else
	{
		g_sport_status = SPORT_STATUS_START;
		Set_PauseSta(false);
	}
	
	switch(Save_Detail_Index)
	{
		case DISPLAY_SCREEN_RUN:
			Save_Detail_Index = DISPLAY_SCREEN_RUN_PAUSE;
			break;
		case DISPLAY_SCREEN_RUN_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_RUN;
			break;
		case DISPLAY_SCREEN_WALK:
			Save_Detail_Index = DISPLAY_SCREEN_WALK_PAUSE;
			break;
		case DISPLAY_SCREEN_WALK_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_WALK;
			break;
		case DISPLAY_SCREEN_MARATHON:
			Save_Detail_Index = DISPLAY_SCREEN_MARATHON_PAUSE;
			break;
		case DISPLAY_SCREEN_MARATHON_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_MARATHON;
			break;
		case DISPLAY_SCREEN_INDOORRUN:
			Save_Detail_Index = DISPLAY_SCREEN_INDOORRUN_PAUSE;
			break;
		case DISPLAY_SCREEN_INDOORRUN_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_INDOORRUN;
			break;
		case DISPLAY_SCREEN_CYCLING:
			Save_Detail_Index = DISPLAY_SCREEN_CYCLING_PAUSE;
			break;
		case DISPLAY_SCREEN_CYCLING_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_CYCLING;
			break;
		case DISPLAY_SCREEN_SWIMMING:
			Save_Detail_Index = DISPLAY_SCREEN_SWIM_PAUSE;
			break;
		case DISPLAY_SCREEN_SWIM_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_SWIMMING;
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE;
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY;
			break;
		case DISPLAY_SCREEN_CLIMBING:
			Save_Detail_Index = DISPLAY_SCREEN_CLIMBING_PAUSE;
			break;
		case DISPLAY_SCREEN_CLIMBING_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_CLIMBING;
			break;
				case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE;
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE:
			Save_Detail_Index = DISPLAY_SCREEN_CROSSCOUNTRY_HIKE;
			break;
	}
}
void gui_sport_pause_hint_btn_evt(uint32_t Key_Value)
{
		switch(Key_Value)
		{
			case (KEY_OK):
			{		
				gui_sport_pause_swich();
				timer_app_pasue_start();
				//振动提示
				gui_sport_motor_prompt();
			}break; 			
		}

}




//运动准备界面按键处理
void gui_sport_ready_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
	    switch(m_sport_ready_index)
	   	{
	   	case GUI_SPORT_READY_INDEX_DISPLAY:
	        //获取各运动数据显示项
			gui_sport_data_display_init();
			m_sport_data_display_index = 0;
			 ScreenState =DISPLAY_SCREEN_SPORT_DATA_DISPLAY_PREVIEW;
			gui_swich_msg();
	   		break;
	   
	   
	   	case GUI_SPORT_READY_INDEX_START:
			gui_sport_data_display_init();
			gui_sport_start_init();
		    progress_pre_timer = 0;
			location_timer_count = 0;
			progress_pre_timer = location_timer_count;
	   		break;
	   
	   	case GUI_SPORT_READY_INDEX_SET:
	   		//获取各运动提醒设置项
			gui_sport_remind_set_init();

			m_sport_remind_set_index = 0;
            ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
			gui_swich_msg();
	   		break;
	   
	      }
			

			break;
		case KEY_DOWN:
				if(m_sport_ready_index >= GUI_SPORT_READY_INDEX_MAX - 1)
			{
				m_sport_ready_index=0;
			}else{
				m_sport_ready_index++;
			}
			ScreenState = DISPLAY_SCREEN_SPORT_READY;
			gui_swich_msg();
			
		
			break;
		case KEY_UP:

				if(m_sport_ready_index <= 0)
			{
				m_sport_ready_index = GUI_SPORT_READY_INDEX_MAX - 1;
			}else{
				m_sport_ready_index--;
			}
	    	ScreenState = DISPLAY_SCREEN_SPORT_READY;
				gui_swich_msg();
	
            break; 
		case KEY_BACK:
		 	m_sport_ready_index = GUI_SPORT_READY_INDEX_START;
					
			progress_pre_timer = 0;
			location_timer_count = 0;
			progress_pre_timer = location_timer_count;
			Set_IsTrainPlanOnGoing(false);
			if(0 == ENTER_NAVIGATION && 0 == ENTER_MOTION_TRAIL)    //由导航界面进入运动ready界面的判断
			{
			#ifndef COD
			if (ScreenSportSave == DISPLAY_SCREEN_RUN) //跑步需判断训练计划
			{
				if(Get_IsCompleteTodayTrainPlan() != 1 && get_trainplan_valid_flag()
					 && Get_SomeDayofTrainPlan() > 0 && Get_TrainPlanBeforeDays() == 0 
				   && Is_TodayTimeTrainPlan() == true)
				{//有训练计划时
					if(get_enter_trainplan_way() == WAYS_ENTER_TRAINPLAN_SPORT_RUN)
					{
						//返回运动选择界面
						ScreenState = DISPLAY_SCREEN_TRAINPLAN_RUN_SELECT;
					}
					else if(get_enter_trainplan_way() == WAYS_ENTER_TRAINPLAN_SPORT_STANDBUY)
					{
					 //从待机快捷界面进入，则返回训练计划界面
					 ScreenState = DISPLAY_SCREEN_TRAINPLAN_MARATHON;
					}
					else
					{//提醒界面等进入的,均返回待机快捷界面
					 ScreenState = DISPLAY_SCREEN_SPORT;
					}
				}
				else
				{
					//返回运动选择界面
					ScreenState = DISPLAY_SCREEN_SPORT;
				}
			}
			else
			#endif
			if (ScreenSportSave == DISPLAY_SCREEN_SWIMMING)
			{
				task_hrt_start();
			}
			{//非训练计划跑步运动 返回运动选择界面
				ScreenState = DISPLAY_SCREEN_SPORT;
			}
			 ReadySportBack();
		
		  }
			else
			{
				if(1 == ENTER_NAVIGATION)
				{
					ScreenState = DISPLAY_SCREEN_NAVIGATION_OPS;
				}
				else
				{
					ScreenState = DISPLAY_SCREEN_MOTION_TRAIL_DETAIL;
				}
				
				ENTER_NAVIGATION = 0;
				ENTER_MOTION_TRAIL = 0;
			}
            	
				
			gui_swich_msg();
			break;
		default:
			break;
	}	

}
//各运动数据显示选项 赋值给 运动数据显示设置项
void gui_sport_data_display_set_init(void)
{
	uint8_t i;
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
				//跑步数据显示选项 赋值给 运动数据显示设置项
				for (i=0;i < RUN_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_set_buf[i] = m_run_data_display_option[i];
					
					//当前数据显示项
					if(m_sport_data_display_buf[m_sport_data_display_index] == m_sport_data_display_set_buf[i])
					{
						m_sport_data_display_set_index = i;
					}
				}
				//当前运动数据显示项个数
				m_sport_data_display_set_sum = RUN_DATA_DISPLAY_SUM;
				break;
		/*	case DISPLAY_SCREEN_WALK:
				//健走数据显示选项 赋值给 运动数据显示设置项
				for (i=0;i < WALK_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_set_buf[i] = m_walk_data_display_option[i];
						
					//当前数据显示项
					if(m_sport_data_display_buf[m_sport_data_display_index] == m_sport_data_display_set_buf[i])
					{
						m_sport_data_display_set_index = i;
					}
				}
				//当前运动数据显示项个数
				m_sport_data_display_set_sum = WALK_DATA_DISPLAY_SUM;
	
				break;
			case DISPLAY_SCREEN_MARATHON:
				//马拉松数据显示选项 赋值给 运动数据显示设置项
				for (i=0;i < MARATHON_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_set_buf[i] = m_marathon_data_display_option[i];
						
					//当前数据显示项
					if(m_sport_data_display_buf[m_sport_data_display_index] == m_sport_data_display_set_buf[i])
					{
						m_sport_data_display_set_index = i;
					}
				}
				//当前运动数据显示项个数
				m_sport_data_display_set_sum = MARATHON_DATA_DISPLAY_SUM;
				break;*/
			case DISPLAY_SCREEN_CYCLING:
				//骑行数据显示选项 赋值给 运动数据显示设置项
				for (i=0;i < CYCLING_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_set_buf[i] = m_cycling_data_display_option[i];
						
					//当前数据显示项
					if(m_sport_data_display_buf[m_sport_data_display_index] == m_sport_data_display_set_buf[i])
					{
						m_sport_data_display_set_index = i;
					}
				}
				//当前运动数据显示项个数
				m_sport_data_display_set_sum = CYCLING_DATA_DISPLAY_SUM;
				break;
			case DISPLAY_SCREEN_SWIMMING:
				//游泳数据显示选项 赋值给 运动数据显示设置项
				for (i=0;i < SWIM_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_set_buf[i] = m_swim_data_display_option[i];
						
					//当前数据显示项
					if(m_sport_data_display_buf[m_sport_data_display_index] == m_sport_data_display_set_buf[i])
					{
						m_sport_data_display_set_index = i;
					}
				}
				//当前运动数据显示项个数
				m_sport_data_display_set_sum =SWIM_DATA_DISPLAY_SUM;
		
				break;
			case DISPLAY_SCREEN_CLIMBING://登山
				for (i=0;i < CLIMBING_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_set_buf[i] = m_climbing_data_display_option[i];
						
					//当前数据显示项
					if(m_sport_data_display_buf[m_sport_data_display_index] == m_sport_data_display_set_buf[i])
					{
						m_sport_data_display_set_index = i;
					}
				}
				//当前运动数据显示项个数
				m_sport_data_display_set_sum = CLIMBING_DATA_DISPLAY_SUM;
				break;


			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE://徒步越野
				for (i=0;i < HIKE_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_set_buf[i] = m_hike_data_display_option[i];
						
					//当前数据显示项
					if(m_sport_data_display_buf[m_sport_data_display_index] == m_sport_data_display_set_buf[i])
					{
						m_sport_data_display_set_index = i;
					}
				}
				//当前运动数据显示项个数
				m_sport_data_display_set_sum =HIKE_DATA_DISPLAY_SUM;
				break;

			case DISPLAY_SCREEN_CROSSCOUNTRY://越野跑
				for (i=0;i < CROSSCOUNTRY_DATA_DISPLAY_SUM;i++)
				{
					m_sport_data_display_set_buf[i] = m_crosscountry_data_display_option[i];
						
					//当前数据显示项
					if(m_sport_data_display_buf[m_sport_data_display_index] == m_sport_data_display_set_buf[i])
					{
						m_sport_data_display_set_index = i;
					}
				}
				//当前运动数据显示项个数
				m_sport_data_display_set_sum = CROSSCOUNTRY_DATA_DISPLAY_SUM;
				break;
			default:
				break;
		}

}

/**/
void gui_sport_data_display_preview_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
		{
	case KEY_OK:
		ScreenState = DISPLAY_SCREEN_SPORT_DATA_DISPLAY;
	   
	   gui_swich_msg();
	   	
	break;


     case KEY_BACK:
		ScreenState = DISPLAY_SCREEN_SPORT_READY;
	   
	   gui_swich_msg();
	   	
	break;
	default:
		break;

		}


}


//数据显示界面按键处理
void gui_sport_data_display_btn_evt(uint32_t Key_Value)
{

	switch( Key_Value )
	{
		case KEY_OK:
			gui_sport_data_display_set_init();
			ScreenState = DISPLAY_SCREEN_SPORT_DATA_DISPLAY_SET;
			gui_swich_msg();
			break;
		case KEY_DOWN:
			 if (m_sport_data_display_index == (SPORT_DATA_DISPLAY_SUM -1))
			 {
				m_sport_data_display_index = 0;
			 }
			 else
			 {
				m_sport_data_display_index++;
			 }
			 gui_swich_msg();
			break;
		case KEY_UP:
			 if (m_sport_data_display_index == 0)
			 {
				m_sport_data_display_index = SPORT_DATA_DISPLAY_SUM -1;
			 }
			 else
			 {
				m_sport_data_display_index--;
			 }
			 gui_swich_msg();
            break; 
		case KEY_BACK:
 			ScreenState = DISPLAY_SCREEN_SPORT_READY;
			gui_swich_msg();
			break;
		default:
			break;
	}
   
}
//保存各运动数据显示项
void gui_sport_data_display_set_save(void)
{
	switch( ScreenSportSave )
		{
			case DISPLAY_SCREEN_RUN:
				//保存跑步数据显示项
				m_run_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
				if (m_sport_data_display_index == 0)
				{
					SetValue.RunData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.RunData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.RunData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 3)
				{
					SetValue.RunData4 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;
			/*case DISPLAY_SCREEN_WALK:
				//保存健走数据显示项
				//m_walk_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
				if (m_sport_data_display_index == 0)
				{
					SetValue.WlakData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.WlakData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.WlakData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;
			case DISPLAY_SCREEN_MARATHON:
				//保存马拉松数据显示项
				//m_marathon_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
				if (m_sport_data_display_index == 0)
				{
					SetValue.MarathonData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.MarathonData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.MarathonData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;*/
			case DISPLAY_SCREEN_CYCLING:
				//保存骑行数据显示项
				m_cycling_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
				if (m_sport_data_display_index == 0)
				{
					SetValue.CyclingData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.CyclingData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.CyclingData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 3)
				{
					SetValue.CyclingData4 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;
		case DISPLAY_SCREEN_SWIMMING:
				//保存游泳数据显示项
				m_swim_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
				if (m_sport_data_display_index == 0)
				{
					SetValue.SwimRunData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.SwimRunData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.SwimRunData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 3)
				{
					SetValue.SwimRunData4 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;
		/*	case DISPLAY_SCREEN_INDOORRUN:
				//保存室内跑数据显示项
			//	m_indoorrun_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
				if (m_sport_data_display_index == 0)
				{
					SetValue.IndoorRunData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.IndoorRunData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.IndoorRunData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;*/
			case DISPLAY_SCREEN_CLIMBING:
				//登山数据显示项
				m_climbing_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
			  if (m_sport_data_display_index == 0)
				{
					SetValue.ClimbingData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.ClimbingData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.ClimbingData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 3)
				{
					SetValue.ClimbingData4 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;

			case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
				//徒步越野数据显示项
				m_hike_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
			  if (m_sport_data_display_index == 0)
				{
					SetValue.HikeData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.HikeData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.HikeData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 3)
				{
					SetValue.HikeData4 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;


	       	case DISPLAY_SCREEN_CROSSCOUNTRY:
				//徒步越野数据显示项
				m_crosscountry_data_display_buf[m_sport_data_display_index]= m_sport_data_display_buf[m_sport_data_display_index];
			  if (m_sport_data_display_index == 0)
				{
					SetValue.CrosscountryData1 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 1)
				{
					SetValue.CrosscountryData2 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 2)
				{
					SetValue.CrosscountryData3 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				else if (m_sport_data_display_index == 3)
				{
					SetValue.CrosscountryData4 = m_sport_data_display_buf[m_sport_data_display_index];
				}
				break;
			default:
				break;
		}

}


//数据显示界面按键处理
void gui_sport_data_display_set_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			//选中数据显示项
			m_sport_data_display_buf[m_sport_data_display_index] = m_sport_data_display_set_buf[m_sport_data_display_set_index];
			
			//保存各运动数据显示项
			gui_sport_data_display_set_save();
			
			ScreenState = DISPLAY_SCREEN_SPORT_DATA_DISPLAY;
			gui_swich_msg();
			break;
		case KEY_DOWN:
			if (m_sport_data_display_set_index == (m_sport_data_display_set_sum -1))
			 {
				m_sport_data_display_set_index = 0;
			 }
			 else
			 {
				m_sport_data_display_set_index++;
			 }
			 gui_swich_msg();
			break;
		case KEY_UP:
			if (m_sport_data_display_set_index == 0)
			 {
				m_sport_data_display_set_index = (m_sport_data_display_set_sum -1);
			 }
			 else
			 {
				m_sport_data_display_set_index--;
			 }
			 gui_swich_msg();
            break; 
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_SPORT_DATA_DISPLAY;
			gui_swich_msg();
			break;
		default:
			break;
	}
	

}

//运动提醒设置按键处理
void gui_sport_remind_set_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HAERT)
			{	
				m_gui_sport_remind_set_heart_index = 0;
				gui_sport_remind_set_heart_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_HAERT;
			}
				else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_HRRECOVERY)
			{
				if (SetValue.HRRecoverySet == 0)
				{
					SetValue.HRRecoverySet = 1;
				}
				else
				{
					SetValue.HRRecoverySet = 0;
				}
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_PACE)
			{
				m_gui_sport_remind_set_pace_index = 0;
				gui_sport_remind_set_pace_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_PACE;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_DISTANCE)
			{
				m_gui_sport_remind_set_distance_index = 0;
				gui_sport_remind_set_distance_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_DISTANCE;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_TIME)
			{
				m_gui_sport_remind_set_time_index = 0;
				gui_sport_remind_set_time_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_TIME;
			}
           /*速度*/
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_SPEED)
			{
				m_gui_sport_remind_set_speed_index = 0;
				gui_sport_remind_set_speed_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_SPEED;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_AUTOCIRCLE)
			{
				m_gui_sport_remind_set_autocircle_index = 0;
				gui_sport_remind_set_autocircle_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_AUTOCIRCLE;
			}

			 /*高度提醒*/
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_ALTITUDE)
			{
				m_gui_sport_remind_set_altitude_index = 0;
				gui_sport_remind_set_altitude_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_ALTITUDE;
			}
		
			
				else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_GOALCIRCLE)
			{
				m_gui_sport_remind_set_goalcircle_index = 0;
				gui_sport_remind_set_goalcircle_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_GOALCIRCLE;
			}
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_GOALKCAL)
			{
				m_gui_sport_remind_set_goalkcal_index = 0;
				gui_sport_remind_set_goalkcal_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_COALKCAL;
			}
			
			
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_SWIMPOOL)			
			{
				m_gui_sport_remind_set_swimpool_index = 0;
				gui_sport_remind_set_swimpool_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_SWIMPOOL;
			}
		
			else if (m_sport_remind_set_buf[m_sport_remind_set_index] == SPORT_REMIND_SET_INDOORRUNLENG)
			{
				m_gui_sport_remind_set_indoorleng_index = 0;
				gui_sport_remind_set_indoorleng_init();
				ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET_INDOORLENG;
			}
		gui_swich_msg();
          break;    
		case KEY_DOWN:
			if (m_sport_remind_set_index == (m_sport_remind_set_sum -1))
			 {
				m_sport_remind_set_index = 0;
			 }
			 else
			 {
				m_sport_remind_set_index++;
			 }
			 gui_swich_msg();
			break;
		case KEY_UP:
			if (m_sport_remind_set_index == 0)
			 {
				m_sport_remind_set_index = (m_sport_remind_set_sum -1);
			 }
			 else
			 {
				m_sport_remind_set_index--;
			 }
			 gui_swich_msg();
            break; 
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_SPORT_READY;
			gui_swich_msg();
			break;
		default:
			break;
	}

}
//心率提醒设置按键处理
void gui_sport_remind_set_heart_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_sport_remind_set_heart_index)
			{
				case GUI_SPORT_REMIND_SET_HEART_SWITCH:{
					if (m_remind_heartrate.enable)
					{
						m_gui_sport_remind_set_heart_index = GUI_SPORT_REMIND_SET_HEART_TENS;
					}
					else
					{
						m_gui_sport_remind_set_heart_index = GUI_SPORT_REMIND_SET_HEART_SWITCH;
						gui_sport_remind_set_heart_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					}
				}break;
				case GUI_SPORT_REMIND_SET_HEART_TENS:{
					m_gui_sport_remind_set_heart_index = GUI_SPORT_REMIND_SET_HEART_ONES;						
				}break;		
				case GUI_SPORT_REMIND_SET_HEART_ONES:{
						m_gui_sport_remind_set_heart_index = GUI_SPORT_REMIND_SET_HEART_SWITCH;
						gui_sport_remind_set_heart_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_sport_remind_set_heart_index)
			{
				case GUI_SPORT_REMIND_SET_HEART_SWITCH:{
					m_remind_heartrate.enable = !m_remind_heartrate.enable;
				}break;
				case GUI_SPORT_REMIND_SET_HEART_TENS:{
					if(m_remind_heartrate.tens <= 4)
					{
						m_remind_heartrate.tens = 21;
					}else{
						m_remind_heartrate.tens--;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_HEART_ONES:{
					if(m_remind_heartrate.ones == 0)
					{
						m_remind_heartrate.ones = 9;
					}else{
						m_remind_heartrate.ones--;
					}		
		
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_sport_remind_set_heart_index)
			{
				case GUI_SPORT_REMIND_SET_HEART_SWITCH:{
					m_remind_heartrate.enable = !m_remind_heartrate.enable;
				}break;
				case GUI_SPORT_REMIND_SET_HEART_TENS:{
					if(m_remind_heartrate.tens >= 21)
					{
						m_remind_heartrate.tens = 4;
					}else{
						m_remind_heartrate.tens++;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_HEART_ONES:{
					if(m_remind_heartrate.ones >= 9)
					{
						m_remind_heartrate.ones = 0;
					}else{
						m_remind_heartrate.ones++;
					}		
		
				}break;		
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_sport_remind_set_heart_index)
			{
				case GUI_SPORT_REMIND_SET_HEART_SWITCH:{
				
					ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;
				case GUI_SPORT_REMIND_SET_HEART_TENS:{
					m_gui_sport_remind_set_heart_index = GUI_SPORT_REMIND_SET_HEART_SWITCH;						
				}break;		
				case GUI_SPORT_REMIND_SET_HEART_ONES:{
					m_gui_sport_remind_set_heart_index = GUI_SPORT_REMIND_SET_HEART_TENS;	
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}
//配速提醒设置按键处理
void gui_sport_remind_set_pace_btn_evt(uint32_t Key_Value)
{
		switch( Key_Value )
		{
			case KEY_OK:
				switch(m_gui_sport_remind_set_pace_index)
				{
					case GUI_SPORT_REMIND_SET_PACE_SWITCH:{
						if (m_remind_pace.enable)
						{
							m_gui_sport_remind_set_pace_index = GUI_SPORT_REMIND_SET_PACE_MIN;
						}
						else
						{
							m_gui_sport_remind_set_pace_index = GUI_SPORT_REMIND_SET_PACE_SWITCH;
							gui_sport_remind_set_pace_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
						}
					}break;
					case GUI_SPORT_REMIND_SET_PACE_MIN:{
						m_gui_sport_remind_set_pace_index = GUI_SPORT_REMIND_SET_PACE_SEC;						
					}break; 	
					case GUI_SPORT_REMIND_SET_PACE_SEC:{
							m_gui_sport_remind_set_pace_index = GUI_SPORT_REMIND_SET_PACE_SWITCH;
							gui_sport_remind_set_pace_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_DOWN:
				switch(m_gui_sport_remind_set_pace_index)
				{
					case GUI_SPORT_REMIND_SET_PACE_SWITCH:{
						m_remind_pace.enable = !m_remind_pace.enable;
					}break;
					case GUI_SPORT_REMIND_SET_PACE_MIN:{
						if(m_remind_pace.min <= 2)
						{
							m_remind_pace.min = 15;
						}else{
							m_remind_pace.min--;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_PACE_SEC:{
						if(m_remind_pace.sec <= 0)
						{
							m_remind_pace.sec = 59;
						}else{
							m_remind_pace.sec--;
						}		
			
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_UP:
				switch(m_gui_sport_remind_set_pace_index)
				{
					case GUI_SPORT_REMIND_SET_PACE_SWITCH:{
						m_remind_pace.enable = !m_remind_pace.enable;
					}break;
					case GUI_SPORT_REMIND_SET_PACE_MIN:{
						if(m_remind_pace.min >= 15)
						{
							m_remind_pace.min = 2;
						}else{
							m_remind_pace.min++;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_PACE_SEC:{
						if(m_remind_pace.sec >= 59)
						{
							m_remind_pace.sec = 0;
						}else{
							m_remind_pace.sec++;
						}		
			
					}break; 	
				}
				gui_swich_msg();
				break; 
			case KEY_BACK:
				switch(m_gui_sport_remind_set_pace_index)
				{
					case GUI_SPORT_REMIND_SET_PACE_SWITCH:{
					
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break;
					case GUI_SPORT_REMIND_SET_PACE_MIN:{
						m_gui_sport_remind_set_pace_index = GUI_SPORT_REMIND_SET_PACE_SWITCH; 					
					}break; 	
					case GUI_SPORT_REMIND_SET_PACE_SEC:{
						m_gui_sport_remind_set_pace_index = GUI_SPORT_REMIND_SET_PACE_MIN;	
					}break; 	
				}
				gui_swich_msg();
				break;
			default:
				break;
			}

}
//距离提醒设置按键处理
void gui_sport_remind_set_distance_btn_evt(uint32_t Key_Value)
{
		switch( Key_Value )
		{
			case KEY_OK:
				switch(m_gui_sport_remind_set_distance_index)
				{
					case GUI_SPORT_REMIND_SET_DISTANCE_SWITCH:{
						if (m_remind_distance.enable)
						{
							m_gui_sport_remind_set_distance_index = GUI_SPORT_REMIND_SET_DISTANCE_KM;
						}
						else
						{
							m_gui_sport_remind_set_distance_index = GUI_SPORT_REMIND_SET_DISTANCE_SWITCH;
							gui_sport_remind_set_distance_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
						}
					}break;
					case GUI_SPORT_REMIND_SET_DISTANCE_KM:{
						m_gui_sport_remind_set_distance_index = GUI_SPORT_REMIND_SET_DISTANCE_HM;						
					}break; 	
					case GUI_SPORT_REMIND_SET_DISTANCE_HM:{
							m_gui_sport_remind_set_distance_index = GUI_SPORT_REMIND_SET_DISTANCE_SWITCH;
							gui_sport_remind_set_distance_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_DOWN:
				switch(m_gui_sport_remind_set_distance_index)
				{
					case GUI_SPORT_REMIND_SET_DISTANCE_SWITCH:{
						m_remind_distance.enable = !m_remind_distance.enable;
					}break;
					case GUI_SPORT_REMIND_SET_DISTANCE_KM:{
						if(m_remind_distance.km == 0)
						{
							m_remind_distance.km = 99;
						}else{
							m_remind_distance.km--;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_DISTANCE_HM:{
						if(m_remind_distance.hm == 0)
						{
							m_remind_distance.hm = 9;
						}else{
							m_remind_distance.hm--;
						}		
			
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_UP:
				switch(m_gui_sport_remind_set_distance_index)
				{
					case GUI_SPORT_REMIND_SET_DISTANCE_SWITCH:{
						m_remind_distance.enable = !m_remind_distance.enable;
					}break;
					case GUI_SPORT_REMIND_SET_DISTANCE_KM:{
						if(m_remind_distance.km >= 99)
						{
							m_remind_distance.km = 0;
						}else{
							m_remind_distance.km++;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_DISTANCE_HM:{
						if(m_remind_distance.hm >= 9)
						{
							m_remind_distance.hm = 0;
						}else{
							m_remind_distance.hm++;
						}		
			
					}break; 	
				}
				gui_swich_msg();
				break; 
			case KEY_BACK:
				switch(m_gui_sport_remind_set_distance_index)
				{
					case GUI_SPORT_REMIND_SET_DISTANCE_SWITCH:{
					
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break;
					case GUI_SPORT_REMIND_SET_DISTANCE_KM:{
						m_gui_sport_remind_set_distance_index = GUI_SPORT_REMIND_SET_DISTANCE_SWITCH; 					
					}break; 	
					case GUI_SPORT_REMIND_SET_DISTANCE_HM:{
						m_gui_sport_remind_set_distance_index = GUI_SPORT_REMIND_SET_DISTANCE_KM;	
					}break; 	
				}
				gui_swich_msg();
				break;
			default:
				break;
			}

}
//自动计圈设置按键处理
void gui_sport_remind_set_autocircle_btn_evt(uint32_t Key_Value)
{
		switch( Key_Value )
		{
			case KEY_OK:
				switch(m_gui_sport_remind_set_autocircle_index)
				{
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH:{
						if (m_remind_autocircle.enable)
						{
							m_gui_sport_remind_set_autocircle_index = GUI_SPORT_REMIND_SET_AUTOCIRCLE_KM;
						}
						else
						{
							m_gui_sport_remind_set_autocircle_index = GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH;
							gui_sport_remind_set_autocircle_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
						}
					}break;
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_KM:{
						m_gui_sport_remind_set_autocircle_index = GUI_SPORT_REMIND_SET_AUTOCIRCLE_HM;						
					}break; 	
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_HM:{
							m_gui_sport_remind_set_autocircle_index = GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH;
							gui_sport_remind_set_autocircle_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_DOWN:
				switch(m_gui_sport_remind_set_autocircle_index)
				{
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH:{
						m_remind_autocircle.enable = !m_remind_autocircle.enable;
					}break;
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_KM:{
						if(m_remind_autocircle.km <= 0)
						{
							m_remind_autocircle.km = 99;
						}else{
							m_remind_autocircle.km--;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_HM:{
						if(m_remind_autocircle.hm == 0)
						{
							m_remind_autocircle.hm = 9;
						}else{
							m_remind_autocircle.hm--;
						}		
			
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_UP:
				switch(m_gui_sport_remind_set_autocircle_index)
				{
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH:{
						m_remind_autocircle.enable = !m_remind_autocircle.enable;
					}break;
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_KM:{
						if(m_remind_autocircle.km >= 99)
						{
							m_remind_autocircle.km = 0;
						}else{
							m_remind_autocircle.km++;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_HM:{
						if(m_remind_autocircle.hm >= 9)
						{
							m_remind_autocircle.hm = 0;
						}else{
							m_remind_autocircle.hm++;
						}		
			
					}break; 	
				}
				gui_swich_msg();
				break; 
			case KEY_BACK:
				switch(m_gui_sport_remind_set_autocircle_index)
				{
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH:{
					
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break;
					case GUI_SPORT_REMIND_SET_AUTOCIRCLE_KM:{
						m_gui_sport_remind_set_autocircle_index = GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH; 					
					}break; 	
					case GUI_SPORT_REMIND_SET_DISTANCE_HM:{
						m_gui_sport_remind_set_autocircle_index = GUI_SPORT_REMIND_SET_AUTOCIRCLE_KM;	
					}break; 	
				}
				gui_swich_msg();
				break;
			default:
				break;
			}
}
//燃脂目标设置按键处理
void gui_sport_remind_set_goalkcal_btn_evt(uint32_t Key_Value)
{
		switch( Key_Value )
		{
			case KEY_OK:
				switch(m_gui_sport_remind_set_goalkcal_index)
				{
					case GUI_SPORT_REMIND_SET_GOALKCAL_SWITCH:{
						if (m_remind_goalkcal.enable)
						{
							m_gui_sport_remind_set_goalkcal_index = GUI_SPORT_REMIND_SET_GOALKCAL_HUNDREDS;
						}
						else
						{
							m_gui_sport_remind_set_goalkcal_index = GUI_SPORT_REMIND_SET_GOALKCAL_SWITCH;
							gui_sport_remind_set_goalkcal_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
						}
					}break;
					case GUI_SPORT_REMIND_SET_GOALKCAL_HUNDREDS:{
						m_gui_sport_remind_set_goalkcal_index = GUI_SPORT_REMIND_SET_GOALKCAL_TENS;						
					}break; 
					case GUI_SPORT_REMIND_SET_GOALKCAL_TENS:{
						m_gui_sport_remind_set_goalkcal_index = GUI_SPORT_REMIND_SET_GOALKCAL_ONES;						
					}break; 
					case GUI_SPORT_REMIND_SET_GOALKCAL_ONES:{
							m_gui_sport_remind_set_goalkcal_index = GUI_SPORT_REMIND_SET_AUTOCIRCLE_SWITCH;
							gui_sport_remind_set_goalkcal_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_DOWN:
				switch(m_gui_sport_remind_set_goalkcal_index)
				{
					case GUI_SPORT_REMIND_SET_GOALKCAL_SWITCH:{
						m_remind_goalkcal.enable = !m_remind_goalkcal.enable;
					}break;
					case GUI_SPORT_REMIND_SET_GOALKCAL_HUNDREDS:{
						if(m_remind_goalkcal.hundreds <= 1)
						{
							m_remind_goalkcal.hundreds = 19;
						}else{
							m_remind_goalkcal.hundreds--;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_GOALKCAL_TENS:{
						if(m_remind_goalkcal.tens == 0)
						{
							m_remind_goalkcal.tens = 9;
						}else{
							m_remind_goalkcal.tens--;
						}		
					}break; 
					case GUI_SPORT_REMIND_SET_GOALKCAL_ONES:{
						if(m_remind_goalkcal.ones == 0)
						{
							m_remind_goalkcal.ones = 9;
						}else{
							m_remind_goalkcal.ones--;
						}		
				
					}break; 
				}
				gui_swich_msg();
				break;
			case KEY_UP:
				switch(m_gui_sport_remind_set_goalkcal_index)
				{
					case GUI_SPORT_REMIND_SET_GOALKCAL_SWITCH:{
						m_remind_goalkcal.enable = !m_remind_goalkcal.enable;
					}break;
					case GUI_SPORT_REMIND_SET_GOALKCAL_HUNDREDS:{
						if(m_remind_goalkcal.hundreds >= 19)
						{
							m_remind_goalkcal.hundreds = 1;
						}else{
							m_remind_goalkcal.hundreds++;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_GOALKCAL_TENS:{
						if(m_remind_goalkcal.tens >= 9)
						{
							m_remind_goalkcal.tens = 0;
						}else{
							m_remind_goalkcal.tens++;
						}
						
					}break; 
					case GUI_SPORT_REMIND_SET_GOALKCAL_ONES:{
						if(m_remind_goalkcal.ones >= 9)
						{
							m_remind_goalkcal.ones = 0;
						}else{
							m_remind_goalkcal.ones++;
						}		
						
					}break; 

				}
				gui_swich_msg();
				break; 
			case KEY_BACK:
				switch(m_gui_sport_remind_set_goalkcal_index)
				{
					case GUI_SPORT_REMIND_SET_GOALKCAL_SWITCH:{
					
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break;
					case GUI_SPORT_REMIND_SET_GOALKCAL_HUNDREDS:{
						m_gui_sport_remind_set_goalkcal_index = GUI_SPORT_REMIND_SET_GOALKCAL_SWITCH; 					
					}break; 	
					case GUI_SPORT_REMIND_SET_GOALKCAL_TENS:{
						m_gui_sport_remind_set_goalkcal_index = GUI_SPORT_REMIND_SET_GOALKCAL_HUNDREDS;	
					}break;
					case GUI_SPORT_REMIND_SET_GOALKCAL_ONES:{
						m_gui_sport_remind_set_goalkcal_index = GUI_SPORT_REMIND_SET_GOALKCAL_TENS;	
					}break; 
				}
				gui_swich_msg();
				break;
			default:
				break;
			}
}

//时间提醒设置按键处理
void gui_sport_remind_set_time_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_sport_remind_set_time_index)
			{
				case GUI_SPORT_REMIND_SET_TIME_SWITCH:{
					if (m_remind_time.enable)
					{
						m_gui_sport_remind_set_time_index = GUI_SPORT_REMIND_SET_TIME_TENS;
					}
					else
					{
						m_gui_sport_remind_set_time_index = GUI_SPORT_REMIND_SET_TIME_SWITCH;
						gui_sport_remind_set_time_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					}
				}break;
				case GUI_SPORT_REMIND_SET_TIME_TENS:{
					m_gui_sport_remind_set_time_index = GUI_SPORT_REMIND_SET_TIME_ONES;						
				}break;		
				case GUI_SPORT_REMIND_SET_TIME_ONES:{
						m_gui_sport_remind_set_time_index = GUI_SPORT_REMIND_SET_TIME_SWITCH;
						gui_sport_remind_set_time_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_sport_remind_set_time_index)
			{
				case GUI_SPORT_REMIND_SET_TIME_SWITCH:{
					m_remind_time.enable = !m_remind_time.enable;
				}break;
				case GUI_SPORT_REMIND_SET_TIME_TENS:{
					if(m_remind_time.tens <= 1)
					{
						m_remind_time.tens = 49;
					}else{
						m_remind_time.tens--;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_TIME_ONES:{
					if(m_remind_time.ones == 0)
					{
						m_remind_time.ones = 9;
					}else{
						m_remind_time.ones--;
					}		
		
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_sport_remind_set_time_index)
			{
				case GUI_SPORT_REMIND_SET_TIME_SWITCH:{
					m_remind_time.enable = !m_remind_time.enable;
				}break;
				case GUI_SPORT_REMIND_SET_TIME_TENS:{
					if(m_remind_time.tens >= 49)
					{
						m_remind_time.tens = 1;
					}else{
						m_remind_time.tens++;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_TIME_ONES:{
					if(m_remind_time.ones >= 9)
					{
						m_remind_time.ones = 0;
					}else{
						m_remind_time.ones++;
					}		
		
				}break;		
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_sport_remind_set_time_index)
			{
				case GUI_SPORT_REMIND_SET_TIME_SWITCH:{
				
					ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;
				case GUI_SPORT_REMIND_SET_TIME_TENS:{
					m_gui_sport_remind_set_time_index = GUI_SPORT_REMIND_SET_TIME_SWITCH;						
				}break;		
				case GUI_SPORT_REMIND_SET_TIME_ONES:{
					m_gui_sport_remind_set_time_index = GUI_SPORT_REMIND_SET_TIME_TENS;	
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}

//速度提醒设置按键处理
void gui_sport_remind_set_speed_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_sport_remind_set_speed_index)
			{
				case GUI_SPORT_REMIND_SET_SPEED_SWITCH:{
					if (m_remind_speed.enable)
					{
						m_gui_sport_remind_set_speed_index = GUI_SPORT_REMIND_SET_SPEED_TENS;
					}
					else
					{
						m_gui_sport_remind_set_speed_index = GUI_SPORT_REMIND_SET_SPEED_SWITCH;
						gui_sport_remind_set_speed_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					}
				}break;
				case GUI_SPORT_REMIND_SET_SPEED_TENS:{
					m_gui_sport_remind_set_speed_index = GUI_SPORT_REMIND_SET_SPEED_ONES;						
				}break;		
				case GUI_SPORT_REMIND_SET_SPEED_ONES:{
						m_gui_sport_remind_set_speed_index = GUI_SPORT_REMIND_SET_SPEED_SWITCH;
						gui_sport_remind_set_speed_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_sport_remind_set_speed_index)
			{
				case GUI_SPORT_REMIND_SET_SPEED_SWITCH:{
					m_remind_speed.enable = !m_remind_speed.enable;
				}break;
				case GUI_SPORT_REMIND_SET_SPEED_TENS:{
					if(m_remind_speed.tens <= 1)
					{
						m_remind_speed.tens = 99;
					}else{
						m_remind_speed.tens--;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_SPEED_ONES:{
					if(m_remind_speed.ones <= 0)
					{
						m_remind_speed.ones = 9;
					}else{
						m_remind_speed.ones--;
					}		
		
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_sport_remind_set_speed_index)
			{
				case GUI_SPORT_REMIND_SET_SPEED_SWITCH:{
					m_remind_speed.enable = !m_remind_speed.enable;
				}break;
				case GUI_SPORT_REMIND_SET_SPEED_TENS:{
					if(m_remind_speed.tens >= 99)
					{
						m_remind_speed.tens = 1;
					}else{
						m_remind_speed.tens++;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_SPEED_ONES:{
					if(m_remind_speed.ones >= 9)
					{
						m_remind_speed.ones = 0;
					}else{
						m_remind_speed.ones++;
					}		
		
				}break;		
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_sport_remind_set_speed_index)
			{
				case GUI_SPORT_REMIND_SET_SPEED_SWITCH:{
				
					ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;
				case GUI_SPORT_REMIND_SET_SPEED_TENS:{
					m_gui_sport_remind_set_speed_index = GUI_SPORT_REMIND_SET_SPEED_SWITCH;						
				}break;		
				case GUI_SPORT_REMIND_SET_SPEED_ONES:{
					m_gui_sport_remind_set_speed_index = GUI_SPORT_REMIND_SET_SPEED_TENS;	
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}

//泳池设置设置按键处理
void gui_sport_remind_set_swimpool_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_sport_remind_set_swimpool_index)
			{
			
				case GUI_SPORT_REMIND_SET_SWIMPOOL_TENS:{
					m_gui_sport_remind_set_swimpool_index = GUI_SPORT_REMIND_SET_SWIMPOOL_ONES;						
				}break;		
				case GUI_SPORT_REMIND_SET_SWIMPOOL_ONES:{
						m_gui_sport_remind_set_swimpool_index = GUI_SPORT_REMIND_SET_SWIMPOOL_TENS;
						gui_sport_remind_set_swimpool_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_sport_remind_set_swimpool_index)
			{
				case GUI_SPORT_REMIND_SET_SWIMPOOL_TENS:{
					if(m_remind_swimpool.tens <= 0)
					{
						m_remind_swimpool.tens = 9;
					}else{
						m_remind_swimpool.tens--;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_SWIMPOOL_ONES:{
					if(m_remind_swimpool.ones <= 0)
					{
						m_remind_swimpool.ones = 9;
					}else{
						m_remind_swimpool.ones--;
					}		
		
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_sport_remind_set_swimpool_index)
			{			
				case GUI_SPORT_REMIND_SET_SWIMPOOL_TENS:{
					if(m_remind_swimpool.tens >= 9)
					{
						m_remind_swimpool.tens = 0;
					}else{
						m_remind_swimpool.tens++;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_SWIMPOOL_ONES:{
					if(m_remind_swimpool.ones >= 9)
					{
						m_remind_swimpool.ones = 0;
					}else{
						m_remind_swimpool.ones++;
					}		
		
				}break;		
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_sport_remind_set_swimpool_index)
			{
				case GUI_SPORT_REMIND_SET_SWIMPOOL_TENS:{
					ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;						
				}break;		
				case GUI_SPORT_REMIND_SET_SWIMPOOL_ONES:{
					m_gui_sport_remind_set_swimpool_index = GUI_SPORT_REMIND_SET_SWIMPOOL_TENS;	
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}



//计圈提醒设置按键处理
void gui_sport_remind_set_goalcircle_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_sport_remind_set_goalcircle_index)
			{
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH:{
					if (m_remind_goalcircle.enable)
					{
						m_gui_sport_remind_set_goalcircle_index = GUI_SPORT_REMIND_SET_GOALCIRCLE_TENS;
					}
					else
					{
						m_gui_sport_remind_set_goalcircle_index = GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH;
						gui_sport_remind_set_goalcircle_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					}
				}break;
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_TENS:{
					m_gui_sport_remind_set_goalcircle_index = GUI_SPORT_REMIND_SET_GOALCIRCLE_ONES;						
				}break;		
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_ONES:{
						m_gui_sport_remind_set_goalcircle_index = GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH;
						gui_sport_remind_set_goalcircle_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_sport_remind_set_goalcircle_index)
			{
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH:{
					m_remind_goalcircle.enable = !m_remind_goalcircle.enable;
				}break;
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_TENS:{
					if(m_remind_goalcircle.tens <= 0)
					{
						m_remind_goalcircle.tens = 9;
					}else{
						m_remind_goalcircle.tens--;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_ONES:{
					if(m_remind_goalcircle.ones <= 0)
					{
						m_remind_goalcircle.ones = 9;
					}else{
						m_remind_goalcircle.ones--;
					}		
		
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_sport_remind_set_goalcircle_index)
			{
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH:{
					m_remind_goalcircle.enable = !m_remind_goalcircle.enable;
				}break;
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_TENS:{
					if(m_remind_goalcircle.tens >= 9)
					{
						m_remind_goalcircle.tens = 0;
					}else{
						m_remind_goalcircle.tens++;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_ONES:{
					if(m_remind_goalcircle.ones >= 9)
					{
						m_remind_goalcircle.ones = 0;
					}else{
						m_remind_goalcircle.ones++;
					}		
		
				}break;		
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_sport_remind_set_goalcircle_index)
			{
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH:{
				
					ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_TENS:{
					m_gui_sport_remind_set_goalcircle_index = GUI_SPORT_REMIND_SET_GOALCIRCLE_SWITCH;						
				}break;		
				case GUI_SPORT_REMIND_SET_GOALCIRCLE_ONES:{
					m_gui_sport_remind_set_goalcircle_index = GUI_SPORT_REMIND_SET_GOALCIRCLE_TENS;	
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}
#ifdef COD 
void gui_vice_sport_detail_btn_evt(uint32_t evt)
	{
		DISPLAY_MSG  msg = {0};
		
		switch( evt )
		{
			case KEY_UP:
				//上翻
				if(sport_cast_detail_index > 0)
				{
					sport_cast_detail_index--;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;
			case KEY_DOWN:
				//下翻
				if(sport_cast_detail_index < 1)
				{
					sport_cast_detail_index++;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				break;
			case KEY_BACK:
				timer_app_vice_pasue_stop(false);
						
				set_train_pause_time(); 
				//返回主界面
				ScreenState = DISPLAY_SCREEN_HOME;
				
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				break;
			default:
				break;
		}
	}
#endif

//室内跑步长设置设置按键处理
void gui_sport_remind_set_indoorleng_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
			switch(m_gui_sport_remind_set_indoorleng_index)
			{
			
				case GUI_SPORT_REMIND_SET_INDOORLENG_TENS:{
					m_gui_sport_remind_set_indoorleng_index = GUI_SPORT_REMIND_SET_INDOORLENG_ONES;						
				}break;		
				case GUI_SPORT_REMIND_SET_INDOORLENG_ONES:{
						m_gui_sport_remind_set_indoorleng_index = GUI_SPORT_REMIND_SET_INDOORLENG_TENS;
						gui_sport_remind_set_indoorleng_save();
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
				
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_DOWN:
			switch(m_gui_sport_remind_set_indoorleng_index)
			{
				case GUI_SPORT_REMIND_SET_INDOORLENG_TENS:{
					if(m_remind_indoorleng.tens <= 1)
					{
						m_remind_indoorleng.tens = 24;
					}else{
						m_remind_indoorleng.tens--;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_INDOORLENG_ONES:{
					if(m_remind_indoorleng.ones <= 0)
					{
						m_remind_indoorleng.ones = 9;
					}else{
						m_remind_indoorleng.ones--;
					}		
		
				}break;		
			}
			gui_swich_msg();
			break;
		case KEY_UP:
			switch(m_gui_sport_remind_set_indoorleng_index)
			{			
				case GUI_SPORT_REMIND_SET_INDOORLENG_TENS:{
					if(m_remind_indoorleng.tens >= 24)
					{
						m_remind_indoorleng.tens = 1;
					}else{
						m_remind_indoorleng.tens++;
					}						
				}break;		
				case GUI_SPORT_REMIND_SET_INDOORLENG_ONES:{
					if(m_remind_indoorleng.ones >= 9)
					{
						m_remind_indoorleng.ones = 0;
					}else{
						m_remind_indoorleng.ones++;
					}		
		
				}break;		
			}
			gui_swich_msg();
            break; 
		case KEY_BACK:
			switch(m_gui_sport_remind_set_indoorleng_index)
			{
				case GUI_SPORT_REMIND_SET_INDOORLENG_TENS:{
					ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;						
				}break;		
				case GUI_SPORT_REMIND_SET_INDOORLENG_ONES:{
					m_gui_sport_remind_set_indoorleng_index = GUI_SPORT_REMIND_SET_INDOORLENG_TENS;	
				}break;		
			}
			gui_swich_msg();
			break;
		default:
			break;
		}

}


//登山高度设置设置按键处理
void gui_sport_remind_set_altitude_btn_evt(uint32_t Key_Value)
	{
		switch( Key_Value )
		{
			case KEY_OK:
				switch(m_gui_sport_remind_set_altitude_index)
				{
					case GUI_SPORT_REMIND_SET_ALTITUDE_SWITCH:{
						if (m_remind_altitude.enable)
						{
							m_gui_sport_remind_set_altitude_index = GUI_SPORT_REMIND_SET_ALTITUDE_THOUS;
						}
						else
						{
							m_gui_sport_remind_set_altitude_index = GUI_SPORT_REMIND_SET_ALTITUDE_SWITCH;
							gui_sport_remind_set_altitude_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
						}
					}break;
					case GUI_SPORT_REMIND_SET_ALTITUDE_THOUS:{
						m_gui_sport_remind_set_altitude_index = GUI_SPORT_REMIND_SET_ALTITUDE_HUND; 					
					}break; 	
					case GUI_SPORT_REMIND_SET_ALTITUDE_HUND:{
							m_gui_sport_remind_set_altitude_index = GUI_SPORT_REMIND_SET_ALTITUDE_SWITCH;
							gui_sport_remind_set_altitude_save();
							ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_DOWN:
				switch(m_gui_sport_remind_set_altitude_index)
				{
					case GUI_SPORT_REMIND_SET_ALTITUDE_SWITCH:{
						m_remind_altitude.enable = !m_remind_altitude.enable;
					}break;
					case GUI_SPORT_REMIND_SET_ALTITUDE_THOUS:{
						if(m_remind_altitude.thousand <= 0)
						{
							m_remind_altitude.thousand = 9;
						}else{
							m_remind_altitude.thousand--;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_ALTITUDE_HUND:{
						if(m_remind_altitude.hundreds <= 0)
						{
							m_remind_altitude.hundreds = 9;
						}else{
							m_remind_altitude.hundreds--;
						}		
			
					}break; 	
				}
				gui_swich_msg();
				break;
			case KEY_UP:
				switch(m_gui_sport_remind_set_altitude_index)
				{
					case GUI_SPORT_REMIND_SET_ALTITUDE_SWITCH:{
						m_remind_altitude.enable = !m_remind_altitude.enable;
					}break;
					case GUI_SPORT_REMIND_SET_ALTITUDE_THOUS:{
						if(m_remind_altitude.thousand >= 9)
						{
							m_remind_altitude.thousand = 0;
						}else{
							m_remind_altitude.thousand++;
						}						
					}break; 	
					case GUI_SPORT_REMIND_SET_ALTITUDE_HUND:{
						if(m_remind_altitude.hundreds >= 9)
						{
							m_remind_altitude.hundreds = 0;
						}else{
							m_remind_altitude.hundreds++;
						}		
			
					}break; 	
				}
				gui_swich_msg();
				break; 
			case KEY_BACK:
				switch(m_gui_sport_remind_set_altitude_index)
				{
					case GUI_SPORT_REMIND_SET_ALTITUDE_SWITCH:{
					
						ScreenState = DISPLAY_SCREEN_SPORT_REMIND_SET;
					
					}break;
					case GUI_SPORT_REMIND_SET_ALTITUDE_THOUS:{
						m_gui_sport_remind_set_altitude_index = GUI_SPORT_REMIND_SET_ALTITUDE_SWITCH;						
					}break; 	
					case GUI_SPORT_REMIND_SET_ALTITUDE_HUND:{
						m_gui_sport_remind_set_altitude_index = GUI_SPORT_REMIND_SET_ALTITUDE_THOUS; 
					}break; 	
				}
				gui_swich_msg();
				break;
			default:
				break;
			}
	
	}


//#endif
//剩余距离 --所有运动都使用
void gui_sport_every_1_distance(uint8_t type, uint32_t distance)
{
	SetWord_t word = {0};
	char str[30] = {0};
	
	//里程数据
	memset(str,0,sizeof(str));
	sprintf(str,"%0.2fkm",distance/100000.f);	//厘米装换为千米，并保留2位小数
	
	word.x_axis = 184;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - (4*LCD_FONT_16_SIZE) - (strlen(str)*8) - 4)/2;
	word.bckgrndcolor = LCD_NONE;
	word.forecolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;
	if(type == 2)
	{
		//已在轨迹上
		word.y_axis = LCD_CENTER_JUSTIFIED;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][37], &word);
		
		return;
	}
	else if(type == 3)
	{
		//未在轨迹上
		word.y_axis = LCD_CENTER_JUSTIFIED;
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][38], &word);
		
		return;
	}
	else if(type == 0)
	{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		if(g_track_mark_data.track.close_time_index > 0 
			&& g_track_mark_data.is_close_time_valid == 1
		  && TRACE_BACK_DATA.is_traceback_flag == 0)
		{
			//距离CPX
			memset(str,0,sizeof(str));
			sprintf(str,"%s%d  %0.2fkm",(char *)GuiSportEventStrs[SetValue.Language][20]
												,g_track_mark_data.track.close_time_index
												,distance/100000.f);
			word.x_axis = 184;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.forecolor = LCD_WHITE;
			LCD_SetString(str, &word);
		}
		else
		{
			//剩余距离 xxkm
			memset(str,0,sizeof(str));
			sprintf(str,"%s %0.2fkm",(char *)SportDetailStrs[SetValue.Language][34]
												,distance/100000.f);
			word.x_axis = 184;
			word.y_axis = LCD_CENTER_JUSTIFIED;
			word.forecolor = LCD_WHITE;
			LCD_SetString(str, &word);
		}
#else
		//剩余距离
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][34], &word);
#endif
	}
	else
	{
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		
		//偏离轨迹
		LCD_SetString((char *)GuiSportEventStrs[SetValue.Language][22], &word);
		
		word.y_axis += (4*LCD_FONT_16_SIZE) + 4;
		LCD_SetString(str, &word);
#else
		//距离轨迹
		LCD_SetString((char *)SportDetailStrs[SetValue.Language][36], &word);
#endif
	}
#if !defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	word.y_axis += (4*LCD_FONT_16_SIZE) + 4;
	LCD_SetString(str, &word);
#endif
}
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
/*关门时间满足提醒条件
离关门时间还剩（1小时、半小时、十五分钟、十分钟、五分钟）
*/
void sport_cloud_start_notify(void)
{
	int32_t m_minute = 0,m_get_close_time = 0,m_real_time = 0;
	
	am_hal_rtc_time_get(&RTC_time);
	//以秒计算 关门时间提前1s提醒
	m_real_time = RTC_time.ui32Hour * 3600 + RTC_time.ui32Minute * 60
								+ RTC_time.ui32Second;
	
	m_get_close_time = g_track_mark_data.track.close_time_hour * 3600
										+ g_track_mark_data.track.close_time_minute * 60;
	
	m_minute = m_get_close_time - m_real_time + 1;
	
//	GUI_SPORT_PRINTF("[gui_sport]:sport_cloud_start_notify--m_minute=%d,hhmmss=%d:%02d:%02d\r\n"
//				,m_minute,RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second);
	if( (m_minute == 300 || m_minute == 600
			|| m_minute == 900 || m_minute == 1800
			|| m_minute == 3600))
	{//提醒 一次 
			if((ScreenState != DISPLAY_SCREEN_LOGO)
			&& (ScreenState != DISPLAY_SCREEN_NOTIFY_BAT_CHG)
			&& time_notify_is_ok())
				{
					if(ScreenState < DISPLAY_SCREEN_NOTIFY)
					{
					 ScreenStateSave = ScreenState;
					}
					timer_notify_display_start(5000,1,false);
					timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
					timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_NOTIFY_CLOUD_CLOSE_TIME_HINT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);							
				}	
	}
}
/*界面显示:关门时间提醒界面*/
void gui_notify_cloud_close_time_paint(void)
{
	SetWord_t word = {0};
	char str[30];
	int32_t m_remain_minute1 = 0,m_remain_minute2 = 0;
	
	m_remain_minute1 = RTC_time.ui32Hour * 60 + RTC_time.ui32Minute;
	m_remain_minute2 = g_track_mark_data.track.close_time_hour * 60 
										 + g_track_mark_data.track.close_time_minute;
	m_remain_minute2 = m_remain_minute2 - m_remain_minute1;
	
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	//图标
	LCD_SetPicture(30,LCD_CENTER_JUSTIFIED,LCD_RED,LCD_NONE,&Img_Countdown_28X28);
	
	if(m_remain_minute2 > 0)
	{
		//关门时间
		word.bckgrndcolor = LCD_NONE;
		word.forecolor = LCD_WHITE;
		word.kerning = 1;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis = 100;
		word.size = LCD_FONT_48_SIZE;

		memset(str,0,30);
		sprintf(str,"%02d:%02d",m_remain_minute2 / 60,m_remain_minute2 % 60);
		LCD_SetNumber(str,&word);
		
		//距离CP1 关门时间还剩
		memset(str,0,30);
		sprintf(str,"%s%d %s",(char *)GuiSportEventStrs[SetValue.Language][20]
												 ,g_track_mark_data.track.close_time_index
												 ,(char *)GuiSportEventStrs[SetValue.Language][21]);
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.x_axis += 68;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.kerning = 0;
		LCD_SetString(str,&word);
	}
}
/*按键处理:关门时间提醒界面*/
void gui_notify_cloud_close_time_btn_evt(uint32_t Key_Value)
{
	switch( Key_Value )
	{
		case KEY_OK:
		case KEY_BACK:
		{
			timer_notify_display_stop(false);
			timer_notify_motor_stop(false);
			timer_notify_buzzer_stop(false);
			DISPLAY_MSG  msg = {0,0};
			ScreenState = ScreenStateSave;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
		}
			break;
		default:
			break;
	}
}


#endif

