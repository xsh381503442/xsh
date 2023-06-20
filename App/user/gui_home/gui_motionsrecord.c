/*
*待机功能界面:运动记录
*/
#include "gui_motionsrecord.h"
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui_sports_record.h"
#include "gui.h"
#include "com_sport.h"
#include "gui_run.h"
#include "task_sport.h"
#include "gui_sport_triathlon.h"
#include "drv_extFlash.h"

#if DEBUG_ENABLED == 1 && GUI_MOTION_RECORD_LOG_ENABLED == 1
	#define GUI_MOTION_RECORD_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_MOTION_RECORD_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_MOTION_RECORD_WRITESTRING(...)
	#define GUI_MOTION_RECORD_PRINTF(...)		        
#endif

_gui_sports_record g_gui_sports_record[2];
uint32_t g_motion_record_addr = 0;
extern TriathlonDetailStr TriathlonData;			//用于保存铁人三项数据
extern SWITCH_HOME_SCREEN_T switch_home_screen;
extern HOME_OR_MINOR_SCREEN_T home_or_minor_switch;
extern uint16_t s_switch_screen_count;
extern ScreenState_t back_monitor_screen;
/*获取运动图标 返回值与Img_Sport[]*/
uint8_t getImgSport(uint8_t type)
{
	uint8_t ImgNum = 0;
	switch(type)
	/*{
		case ACT_RUN:
			ImgNum = 0;
		break;
		case ACT_WALK:
			ImgNum = 4;
		break;
		case ACT_MARATHON:
			ImgNum = 1;
		break;
		case ACT_SWIMMING:
			ImgNum = 8;
		break;
		case ACT_INDOORRUN:
			ImgNum = 3;
		break;
		case ACT_CROSSCOUNTRY:
			ImgNum = 2;
		break;
		case ACT_HIKING:
			ImgNum = 7;
		break;
		case ACT_CLIMB:
			ImgNum = 5;
		break;
		case ACT_CYCLING:
			ImgNum = 6;
		break;
		case ACT_TRIATHLON_SWIM:
		case ACT_TRIATHLON_CYCLING:
		case ACT_TRIATHLON_RUN:
			ImgNum = 9;
		break;
		default:
			break;
	}*/


	{
		case ACT_RUN:
			ImgNum = 0;
		break;
		/*case ACT_WALK:
			ImgNum = 4;
		break;*/
		/*case ACT_MARATHON:
			ImgNum = 1;
		break;*/
		case ACT_SWIMMING:
			ImgNum = 3;
		break;
		/*case ACT_INDOORRUN:
			ImgNum = 3;
		break;*/
		case ACT_CROSSCOUNTRY:
			ImgNum = 5;
		break;
		case ACT_HIKING:
			ImgNum = 4;
		break;
		case ACT_CLIMB:
			ImgNum = 2;
		break;
		case ACT_CYCLING:
			ImgNum = 1;
		break;
		case ACT_TRIATHLON_SWIM:
		case ACT_TRIATHLON_CYCLING:
		case ACT_TRIATHLON_RUN:
			ImgNum = 5;
		break;
		default:
			break;
	}
	return ImgNum;
}
/*获取心率区间*/
static void GetHeartratePara(void)
{
	uint32_t MaxHeartRateZone = 0;
	uint32_t HDRZone[5];
	uint8_t  HDRPercent[5];
	
	memset(HDRZone,0,sizeof(HDRZone));
	memset(HDRPercent,0,sizeof(HDRPercent));

	for (int i = 0; i < 5;i++)
	{
		memcpy(&HDRZone[i],&ActivityRecordData.ActivityDetails.CommonDetail.HeartRateZone[i],5);
		if (MaxHeartRateZone < HDRZone[i])
		{
			MaxHeartRateZone = HDRZone[i];
		}
	}
	if(MaxHeartRateZone != 0)
	{
		for(int i=0;i<5;i++)
		{
			HDRPercent[i] = (HDRZone[i]*80/MaxHeartRateZone);//参考RUN运动
		}
	}
	else
	{
		memset(HDRPercent,0,sizeof(HDRPercent));
	}
	g_gui_sports_record[0].hr_section_one.hour = HDRZone[0]/60;
	g_gui_sports_record[0].hr_section_one.minute = HDRZone[0]%60;
	g_gui_sports_record[0].hr_section_one.percentage = HDRPercent[0];
	g_gui_sports_record[0].hr_section_two.hour = HDRZone[1]/60;
	g_gui_sports_record[0].hr_section_two.minute = HDRZone[1]%60;
	g_gui_sports_record[0].hr_section_two.percentage = HDRPercent[1];
	g_gui_sports_record[0].hr_section_three.hour = HDRZone[2]/60;
	g_gui_sports_record[0].hr_section_three.minute = HDRZone[2]%60;
	g_gui_sports_record[0].hr_section_three.percentage = HDRPercent[2];
	g_gui_sports_record[0].hr_section_four.hour = HDRZone[3]/60;
	g_gui_sports_record[0].hr_section_four.minute = HDRZone[3]%60;
	g_gui_sports_record[0].hr_section_four.percentage = HDRPercent[3];
	g_gui_sports_record[0].hr_section_five.hour = HDRZone[4]/60;
	g_gui_sports_record[0].hr_section_five.minute = HDRZone[4]%60;
	g_gui_sports_record[0].hr_section_five.percentage = HDRPercent[4];
}

/*用作运动记录下一项记录预览*/
static void GetNextSportColumn(uint32_t addr2)
{
	if(addr2 == 0)
	{
		g_gui_sports_record[1].year = 0;
		g_gui_sports_record[1].month = 0;
		g_gui_sports_record[1].month = 0;
	}
	else
	{
		g_gui_sports_record[1].year = ActivityRecordDataDown.Act_Start_Time.Year + 2000;
		g_gui_sports_record[1].month = ActivityRecordDataDown.Act_Start_Time.Month;
		g_gui_sports_record[1].date = ActivityRecordDataDown.Act_Start_Time.Day;
	}
}
/*获取活动数据详情
如果宏定义了WATCH_SPORT_RECORD_TEST：调用该函数后,可以获取到总活动记录个数,以及第addr1个和第addr2个的运动详情
addr1和addr2分别是在数组s_sport_record_select_str的位置
addr1=0,代表最新的运动(日期)add2=0,代表不需要查询*/
void GetSportDetail(uint32_t addr1,uint32_t addr2)
{
	uint32_t second = 0;
	uint8_t m_TRIATHLON_status = 0;
#if defined(WATCH_SPORT_RECORD_TEST)
//		memset(&g_gui_sports_record[0],0,sizeof(_gui_sports_record));
//		memset(&g_gui_sports_record[1],0,sizeof(_gui_sports_record));
	
		bool m_status = get_sport_detail_record(addr1,addr2);
		
		if(ActivityRecordData.Activity_Type == ACT_TRIATHLON_SWIM)
		{
			m_TRIATHLON_status = 1;
		}
		else
		{
			m_TRIATHLON_status = 0;
		}
		GUI_MOTION_RECORD_PRINTF("[gui_motionsrecord]:m_status=%d,Activity_Type=%d,m_TRIATHLON_status=%d,time %d-%02d-%02d %d:%02d:%02d\n"
		  ,m_status,ActivityRecordData.Activity_Type,m_TRIATHLON_status
		  ,ActivityRecordData.Act_Start_Time.Year,ActivityRecordData.Act_Start_Time.Month,ActivityRecordData.Act_Start_Time.Day
		  ,ActivityRecordData.Act_Start_Time.Hour,ActivityRecordData.Act_Start_Time.Minute,ActivityRecordData.Act_Start_Time.Second);
#else
		m_TRIATHLON_status = Read_ActivityData(addr1,addr2);
#endif

#if defined(WATCH_SPORT_RECORD_TEST)
	if(m_TRIATHLON_status == 0 && m_status)
#else
	if(m_TRIATHLON_status == 0)
#endif
	{
		GUI_MOTION_RECORD_PRINTF("[gui_motionsrecord]:g_gui_sports_record[0]==\n");
  second = ActivityRecordData.ActTime;
	//图标及时间
	g_gui_sports_record[0].type = ActivityRecordData.Activity_Type;
	g_gui_sports_record[0].year = ActivityRecordData.Act_Start_Time.Year + 2000;
	g_gui_sports_record[0].month = ActivityRecordData.Act_Start_Time.Month;
	g_gui_sports_record[0].date = ActivityRecordData.Act_Start_Time.Day;
	g_gui_sports_record[0].time_stamp.hour = ActivityRecordData.Act_Start_Time.Hour;
	g_gui_sports_record[0].time_stamp.minute = ActivityRecordData.Act_Start_Time.Minute;
	g_gui_sports_record[0].time_elapse.hour = second/3600;
	g_gui_sports_record[0].time_elapse.minute = second/60%60;
	g_gui_sports_record[0].time_elapse.second = second%60;
	switch(g_gui_sports_record[0].type)
	{
		case ACT_RUN://跑步  
			//第一页数据:里程 时长 平均速度 最大速度
		  //第二页数据:卡路里 平均心率 最大心率 有氧效果 无氧效果 恢复时间
		  //第三页轨迹:RunningDetailStr
			g_gui_sports_record[0].distance = ActivityRecordData.ActivityDetails.RunningDetail.Distance/100000.f;
			g_gui_sports_record[0].avg_speed = ActivityRecordData.ActivityDetails.RunningDetail.Pace;
			g_gui_sports_record[0].max_speed = ActivityRecordData.ActivityDetails.RunningDetail.OptimumPace;
			g_gui_sports_record[0].steps = ActivityRecordData.ActivityDetails.RunningDetail.Steps;
			g_gui_sports_record[0].riseHeight = ActivityRecordData.ActivityDetails.RunningDetail.RiseHeight;
		  g_gui_sports_record[0].calory = ActivityRecordData.ActivityDetails.RunningDetail.Calorie/1000.f;
		  g_gui_sports_record[0].avg_hr = ActivityRecordData.ActivityDetails.RunningDetail.AvgHeart;
		  g_gui_sports_record[0].max_hr = ActivityRecordData.ActivityDetails.RunningDetail.MaxHeart;
		  g_gui_sports_record[0].oxygen_efficiency = ActivityRecordData.ActivityDetails.RunningDetail.AerobicEffect/10.f;
		  g_gui_sports_record[0].non_oxygen_efficiency = ActivityRecordData.ActivityDetails.RunningDetail.AnaerobicEffect/10.f;
		  g_gui_sports_record[0].time_recovery.hour = ActivityRecordData.ActivityDetails.RunningDetail.RecoveryTime/60.f;
//			g_gui_sports_record[0].time_recovery.minute = ActivityRecordData.ActivityDetails.RunningDetail.RecoveryTime%60;
			break;
		case ACT_WALK://健走
			//第一页数据:里程 时长 平均速度 最大速度
		  //第二页数据:卡路里 平均心率 最大心率 有氧效果 无氧效果 恢复时间
		  //第三页轨迹:
			g_gui_sports_record[0].distance = ActivityRecordData.ActivityDetails.WalkingDetail.Distance/100000.f;
			g_gui_sports_record[0].avg_speed = ActivityRecordData.ActivityDetails.WalkingDetail.Speed;
			g_gui_sports_record[0].max_speed = ActivityRecordData.ActivityDetails.WalkingDetail.OptimumSpeed;
			g_gui_sports_record[0].steps = ActivityRecordData.ActivityDetails.WalkingDetail.Steps;
		  g_gui_sports_record[0].calory = ActivityRecordData.ActivityDetails.WalkingDetail.Calorie/1000.f;
		  g_gui_sports_record[0].avg_hr = ActivityRecordData.ActivityDetails.WalkingDetail.AvgHeart;
		  g_gui_sports_record[0].max_hr = ActivityRecordData.ActivityDetails.WalkingDetail.MaxHeart;
		  g_gui_sports_record[0].oxygen_efficiency = ActivityRecordData.ActivityDetails.WalkingDetail.AerobicEffect/10.f;
		  g_gui_sports_record[0].non_oxygen_efficiency = ActivityRecordData.ActivityDetails.WalkingDetail.AnaerobicEffect/10.f;
		  g_gui_sports_record[0].time_recovery.hour = ActivityRecordData.ActivityDetails.WalkingDetail.RecoveryTime/60.f;
//			g_gui_sports_record[0].time_recovery.minute = ActivityRecordData.ActivityDetails.WalkingDetail.RecoveryTime%60;
			break;
		case ACT_MARATHON://马拉松
			//第一页数据:里程 时长 平均速度 最大速度
		  //第二页数据:卡路里 平均心率 最大心率 有氧效果 无氧效果 恢复时间
		  //第三页轨迹:
			g_gui_sports_record[0].distance = ActivityRecordData.ActivityDetails.RunningDetail.Distance/100000.f;
			g_gui_sports_record[0].avg_speed = ActivityRecordData.ActivityDetails.RunningDetail.Pace;
			g_gui_sports_record[0].max_speed = ActivityRecordData.ActivityDetails.RunningDetail.OptimumPace;
			g_gui_sports_record[0].steps = ActivityRecordData.ActivityDetails.RunningDetail.Steps;
		  g_gui_sports_record[0].calory = ActivityRecordData.ActivityDetails.RunningDetail.Calorie/1000.f;
		  g_gui_sports_record[0].avg_hr = ActivityRecordData.ActivityDetails.RunningDetail.AvgHeart;
		  g_gui_sports_record[0].max_hr = ActivityRecordData.ActivityDetails.RunningDetail.MaxHeart;
		  g_gui_sports_record[0].oxygen_efficiency = ActivityRecordData.ActivityDetails.RunningDetail.AerobicEffect/10.f;
		  g_gui_sports_record[0].non_oxygen_efficiency = ActivityRecordData.ActivityDetails.RunningDetail.AnaerobicEffect/10.f;
		  g_gui_sports_record[0].time_recovery.hour = ActivityRecordData.ActivityDetails.RunningDetail.RecoveryTime/60.f;
//			g_gui_sports_record[0].time_recovery.minute = ActivityRecordData.ActivityDetails.RunningDetail.RecoveryTime%60;
			break;
		case ACT_SWIMMING://游泳
			//第一页数据:里程 时长 平均配速 平均swolf
		  //第二页数据:卡路里 平均心率 最大心率 平均滑频 有氧效果 无氧效果 (恢复时间)
		  //第三页轨迹:
			g_gui_sports_record[0].distance = ActivityRecordData.ActivityDetails.SwimmingDetail.Distance/100000.f;
			g_gui_sports_record[0].avg_speed = ActivityRecordData.ActivityDetails.SwimmingDetail.Speed;
			g_gui_sports_record[0].max_speed = ActivityRecordData.ActivityDetails.SwimmingDetail.Swolf;//HOOPP:待定
		
		  g_gui_sports_record[0].calory = ActivityRecordData.ActivityDetails.SwimmingDetail.Calorie/1000.f;
		  g_gui_sports_record[0].avg_hr = ActivityRecordData.ActivityDetails.SwimmingDetail.AvgHeart;
		  g_gui_sports_record[0].max_hr = ActivityRecordData.ActivityDetails.SwimmingDetail.MaxHeart;
		  //HOOPP:最大滑频待定
		#ifdef COD 
		#else
		  g_gui_sports_record[0].oxygen_efficiency = ActivityRecordData.ActivityDetails.SwimmingDetail.AerobicEffect/10.f;
		  g_gui_sports_record[0].non_oxygen_efficiency = ActivityRecordData.ActivityDetails.SwimmingDetail.AnaerobicEffect/10.f;
		  g_gui_sports_record[0].time_recovery.hour = ActivityRecordData.ActivityDetails.SwimmingDetail.RecoveryTime/60.f;
		#endif
//			g_gui_sports_record[0].time_recovery.minute = ActivityRecordData.ActivityDetails.SwimmingDetail.RecoveryTime%60;
		 
			break;
		case ACT_INDOORRUN://室内跑
			//第一页数据:里程 时长 平均速度 最大速度
		  //第二页数据:卡路里 平均心率 最大心率 有氧效果 无氧效果 恢复时间
		  //第三页轨迹:
			g_gui_sports_record[0].distance = ActivityRecordData.ActivityDetails.RunningDetail.Distance/100000.f;
			g_gui_sports_record[0].avg_speed = ActivityRecordData.ActivityDetails.RunningDetail.Pace;
			g_gui_sports_record[0].max_speed = ActivityRecordData.ActivityDetails.RunningDetail.OptimumPace;
			g_gui_sports_record[0].steps = ActivityRecordData.ActivityDetails.RunningDetail.Steps;
		  g_gui_sports_record[0].calory = ActivityRecordData.ActivityDetails.RunningDetail.Calorie/1000.f;
		  g_gui_sports_record[0].avg_hr = ActivityRecordData.ActivityDetails.RunningDetail.AvgHeart;
		  g_gui_sports_record[0].max_hr = ActivityRecordData.ActivityDetails.RunningDetail.MaxHeart;
		  g_gui_sports_record[0].oxygen_efficiency = ActivityRecordData.ActivityDetails.RunningDetail.AerobicEffect/10.f;
		  g_gui_sports_record[0].non_oxygen_efficiency = ActivityRecordData.ActivityDetails.RunningDetail.AnaerobicEffect/10.f;
		  g_gui_sports_record[0].time_recovery.hour = ActivityRecordData.ActivityDetails.RunningDetail.RecoveryTime/60.f;
//			g_gui_sports_record[0].time_recovery.minute = ActivityRecordData.ActivityDetails.RunningDetail.RecoveryTime%60;
			break;
		case ACT_CROSSCOUNTRY://越野跑
		case ACT_HIKING://徒步越野
			//第一页数据:里程 时长 平均速度 最大速度
		  //第二页数据:卡路里 平均心率 最大心率 有氧效果 无氧效果 恢复时间
		  //第三页轨迹:
			g_gui_sports_record[0].distance = ActivityRecordData.ActivityDetails.CrosscountryDetail.Distance/100000.f;
			g_gui_sports_record[0].avg_speed = ActivityRecordData.ActivityDetails.CrosscountryDetail.Speed;
			g_gui_sports_record[0].max_speed = ActivityRecordData.ActivityDetails.CrosscountryDetail.OptimumSpeed;	
			g_gui_sports_record[0].steps = ActivityRecordData.ActivityDetails.CrosscountryDetail.Steps;//步频 待定
			g_gui_sports_record[0].riseHeight = ActivityRecordData.ActivityDetails.CrosscountryDetail.RiseHeight;
			g_gui_sports_record[0].dropHeight = ActivityRecordData.ActivityDetails.CrosscountryDetail.DropHeight;
		  g_gui_sports_record[0].calory = ActivityRecordData.ActivityDetails.CrosscountryDetail.Calorie/1000.f;
		  g_gui_sports_record[0].avg_hr = ActivityRecordData.ActivityDetails.CrosscountryDetail.AvgHeart;
		  g_gui_sports_record[0].max_hr = ActivityRecordData.ActivityDetails.CrosscountryDetail.MaxHeart;
		  g_gui_sports_record[0].oxygen_efficiency = ActivityRecordData.ActivityDetails.CrosscountryDetail.AerobicEffect/10.f;
		  g_gui_sports_record[0].non_oxygen_efficiency = ActivityRecordData.ActivityDetails.CrosscountryDetail.AnaerobicEffect/10.f;
		  g_gui_sports_record[0].time_recovery.hour = ActivityRecordData.ActivityDetails.CrosscountryDetail.RecoveryTime/60.f;
//			g_gui_sports_record[0].time_recovery.minute = ActivityRecordData.ActivityDetails.CrosscountryDetail.RecoveryTime%60;
			
			break;
		case ACT_CLIMB://登山
			//第一页数据:里程 时长 累计上升 累计下降
		  //第二页数据:卡路里 垂直速度 平均心率 最大心率 有氧效果 无氧效果 (恢复时间)
		  //第三页轨迹:
			g_gui_sports_record[0].distance = ActivityRecordData.ActivityDetails.ClimbingDetail.Distance/100000.f;
			g_gui_sports_record[0].avg_speed = ActivityRecordData.ActivityDetails.ClimbingDetail.Speed/10;
			g_gui_sports_record[0].riseHeight = ActivityRecordData.ActivityDetails.ClimbingDetail.RiseHeight;
			g_gui_sports_record[0].dropHeight = ActivityRecordData.ActivityDetails.ClimbingDetail.DropHeight;
		  
		  g_gui_sports_record[0].calory = ActivityRecordData.ActivityDetails.ClimbingDetail.Calorie/1000.f;
	  	g_gui_sports_record[0].max_speed = ActivityRecordData.ActivityDetails.ClimbingDetail.Speed;//垂直速度 HOOPP:待定
		  g_gui_sports_record[0].avg_hr = ActivityRecordData.ActivityDetails.ClimbingDetail.AvgHeart;
		  g_gui_sports_record[0].max_hr = ActivityRecordData.ActivityDetails.ClimbingDetail.MaxHeart;
		  g_gui_sports_record[0].oxygen_efficiency = ActivityRecordData.ActivityDetails.ClimbingDetail.AerobicEffect/10.f;
		  g_gui_sports_record[0].non_oxygen_efficiency = ActivityRecordData.ActivityDetails.ClimbingDetail.AnaerobicEffect/10.f;
		  g_gui_sports_record[0].time_recovery.hour = ActivityRecordData.ActivityDetails.ClimbingDetail.RecoveryTime/60.f;
//			g_gui_sports_record[0].time_recovery.minute = ActivityRecordData.ActivityDetails.ClimbingDetail.RecoveryTime%60;
			break;
		case ACT_CYCLING://骑行
      //待定
			g_gui_sports_record[0].distance = ActivityRecordData.ActivityDetails.CyclingDetail.Distance/100000.f;
	  		#ifdef COD
			g_gui_sports_record[0].avg_speed = (uint32_t)(ActivityRecordData.ActivityDetails.CyclingDetail.Speed/36.f*10)/10.f*36;
			g_gui_sports_record[0].max_speed = (uint32_t)(ActivityRecordData.ActivityDetails.CyclingDetail.OptimumSpeed/36.f*10)/10.f*36;
			GUI_MOTION_RECORD_PRINTF("[gui_motionsrecord]:g_gui_sports_record[0].avg_speed %d,%d,F:"LOG_FLOAT_MARKER"\r\n",(uint32_t)g_gui_sports_record[0].avg_speed,ActivityRecordData.cod_id,LOG_FLOAT((uint32_t)(ActivityRecordData.ActivityDetails.CyclingDetail.Speed/36.f*10)/10.f));
			#else
			g_gui_sports_record[0].avg_speed = ActivityRecordData.ActivityDetails.CyclingDetail.Speed;
			g_gui_sports_record[0].max_speed = ActivityRecordData.ActivityDetails.CyclingDetail.OptimumSpeed;
			#endif
			g_gui_sports_record[0].steps = ActivityRecordData.ActivityDetails.CyclingDetail.Cadence;//最佳踏频HOOPP:待定
			g_gui_sports_record[0].riseHeight = ActivityRecordData.ActivityDetails.CrosscountryDetail.RiseHeight;
			g_gui_sports_record[0].dropHeight = ActivityRecordData.ActivityDetails.CrosscountryDetail.DropHeight;
		  g_gui_sports_record[0].calory = ActivityRecordData.ActivityDetails.CyclingDetail.Calorie/1000.f;
		  g_gui_sports_record[0].avg_hr = ActivityRecordData.ActivityDetails.CyclingDetail.AvgHeart;
		  g_gui_sports_record[0].max_hr = ActivityRecordData.ActivityDetails.CyclingDetail.MaxHeart;
		  g_gui_sports_record[0].oxygen_efficiency = ActivityRecordData.ActivityDetails.CyclingDetail.AerobicEffect/10.f;
		  g_gui_sports_record[0].non_oxygen_efficiency = ActivityRecordData.ActivityDetails.CyclingDetail.AnaerobicEffect/10.f;
		  g_gui_sports_record[0].time_recovery.hour = ActivityRecordData.ActivityDetails.CyclingDetail.RecoveryTime/60.f;
//			g_gui_sports_record[0].time_recovery.minute = ActivityRecordData.ActivityDetails.CyclingDetail.RecoveryTime%60;
			break;
		case ACT_TRIATHLON_SWIM:
		case ACT_TRIATHLON_CYCLING:
		case ACT_TRIATHLON_RUN://铁人三项
		{

		}
			break;
		default:	
			break;
	}
	}
	else
	{
#if defined(WATCH_SPORT_RECORD_TEST)
		if(m_status)
		{
#endif
		uint16_t heartAvg = 0;
		memset(&TriathlonData, 0, sizeof(TriathlonDetailStr));
		//图标及时间
		g_gui_sports_record[0].distance = 0;
		g_gui_sports_record[0].type = ActivityRecordData.Activity_Type;
		g_gui_sports_record[0].year = ActivityRecordData.Act_Start_Time.Year + 2000;
		g_gui_sports_record[0].month = ActivityRecordData.Act_Start_Time.Month;
		g_gui_sports_record[0].date = ActivityRecordData.Act_Start_Time.Day;
		g_gui_sports_record[0].time_stamp.hour = ActivityRecordData.Act_Start_Time.Hour;
		g_gui_sports_record[0].time_stamp.minute = ActivityRecordData.Act_Start_Time.Minute;
		TriathlonData.StartTime.Year = ActivityRecordData.Act_Start_Time.Year + 2000;
		TriathlonData.StartTime.Month = ActivityRecordData.Act_Start_Time.Month;
		TriathlonData.StartTime.Day = ActivityRecordData.Act_Start_Time.Day;
		TriathlonData.StartTime.Hour = ActivityRecordData.Act_Start_Time.Hour;
		TriathlonData.StartTime.Minute = ActivityRecordData.Act_Start_Time.Minute;
		TriathlonData.FininshNum = 0;
	
		second = 0;
		dev_extFlash_enable();
		for(uint8_t m=0;m<5;m++)
		{
#if defined(WATCH_SPORT_RECORD_TEST)
			dev_extFlash_read( s_sport_record_select_str[addr1].address+m*sizeof(ActivityDataStr),(uint8_t*)(&ActivityRecordData),ACTIVITY_DAY_DATA_SIZE);
			GUI_MOTION_RECORD_PRINTF("[gui_motionsrecord]:ACT_TRIATHLON -->address=0X%X,Activity_Type=0x%X,sizeof(ActivityDataStr)=%d\n"
		  ,s_sport_record_select_str[addr1].address+m*sizeof(ActivityDataStr),ActivityRecordData.Activity_Type,sizeof(ActivityDataStr));
#else
			dev_extFlash_read( addr1+m*sizeof(ActivityDataStr),(uint8_t*)(&ActivityRecordData),ACTIVITY_DAY_DATA_SIZE);
#endif
			
			if( ActivityRecordData.Activity_Type == ACT_TRIATHLON_SWIM)
			{
				if(m != 0) break;
				TriathlonData.FininshNum = 1;
				TriathlonData.HeartAvg = ActivityRecordData.ActivityDetails.SwimmingDetail.AvgHeart;
				TriathlonData.SwimmingTime = ActivityRecordData.ActTime;
				TriathlonData.SwimmingDistance = ActivityRecordData.ActivityDetails.SwimmingDetail.Distance;
				TriathlonData.HeartMax = ActivityRecordData.ActivityDetails.SwimmingDetail.MaxHeart;
				g_gui_sports_record[0].distance += TriathlonData.SwimmingDistance;
			}
			else if( ActivityRecordData.Activity_Type == ACT_TRIATHLON_FIRST)
			{
				if(m != 1) break;
				
				//已开始骑行运动
				if(TriathlonData.FininshNum >= 1)
				{
					//第一次转项时长
					TriathlonData.FirstWaitTime = ActivityRecordData.ActTime;
				}
			}
			else if( ActivityRecordData.Activity_Type == ACT_TRIATHLON_CYCLING)
			{
				if(m != 2) break;
				TriathlonData.FininshNum++;
				TriathlonData.CyclingTime = ActivityRecordData.ActTime;
				TriathlonData.CyclingDistance = ActivityRecordData.ActivityDetails.CyclingDetail.Distance;
				g_gui_sports_record[0].distance += TriathlonData.CyclingDistance;
				
				heartAvg = TriathlonData.HeartAvg;
				if((heartAvg == 0) || (ActivityRecordData.ActivityDetails.CyclingDetail.AvgHeart == 0))
				{
					TriathlonData.HeartAvg = heartAvg + ActivityRecordData.ActivityDetails.CyclingDetail.AvgHeart;
				}
				else
				{
					TriathlonData.HeartAvg = (heartAvg + ActivityRecordData.ActivityDetails.CyclingDetail.AvgHeart)/2;
				}
				if(ActivityRecordData.ActivityDetails.CyclingDetail.MaxHeart > TriathlonData.HeartMax)
					TriathlonData.HeartMax = ActivityRecordData.ActivityDetails.CyclingDetail.MaxHeart;
			}
			else if( ActivityRecordData.Activity_Type == ACT_TRIATHLON_SECOND)
			{
				if(m != 3) break;
				
				//已开始跑步运动
				if(TriathlonData.FininshNum >= 2)
				{
					//第二次转项时长
					TriathlonData.SecondWaitTime = ActivityRecordData.ActTime;
				}
			}
			else if( ActivityRecordData.Activity_Type == ACT_TRIATHLON_RUN)
			{
				if(m != 4) break;
				TriathlonData.FininshNum++;
				TriathlonData.RunningTime = ActivityRecordData.ActTime;
				TriathlonData.RunningDistance = ActivityRecordData.ActivityDetails.RunningDetail.Distance;
				g_gui_sports_record[0].distance += TriathlonData.RunningDistance;
				if(ActivityRecordData.ActivityDetails.RunningDetail.MaxHeart > TriathlonData.HeartMax)
					TriathlonData.HeartMax = ActivityRecordData.ActivityDetails.RunningDetail.MaxHeart;
					
				heartAvg = TriathlonData.HeartAvg;
				if((heartAvg == 0) || (ActivityRecordData.ActivityDetails.RunningDetail.AvgHeart == 0))
				{
					TriathlonData.HeartAvg = heartAvg + ActivityRecordData.ActivityDetails.RunningDetail.AvgHeart;
				}
				else
				{
					TriathlonData.HeartAvg = (heartAvg + ActivityRecordData.ActivityDetails.RunningDetail.AvgHeart)/2;
				}
			}
			else
			{
				break;
			}
	
			second += ActivityRecordData.ActTime;
			
		}
    dev_extFlash_disable();
		g_gui_sports_record[0].distance /= 100000.f;
		g_gui_sports_record[0].time_elapse.hour = second/3600;
		g_gui_sports_record[0].time_elapse.minute = second/60%60;
		g_gui_sports_record[0].time_elapse.second = second%60;
		g_gui_sports_record[0].avg_speed = 0;
#if defined(WATCH_SPORT_RECORD_TEST)
		}
#endif
	}
	
#if defined(WATCH_SPORT_RECORD_TEST)
	if(addr2 == 0)
	{
		GetNextSportColumn(0);
	}
	else
	{
		GetNextSportColumn(s_sport_record_select_str[addr2].address);
	}
	
#else
	GetNextSportColumn(addr2);
#endif
	//第四页:心率区间
	GetHeartratePara();
}
/*界面显示:运动记录里程/时长/配速 界面*/
static void gui_motionsrecord_mile_time_pace_paint(void)
{
	SetWord_t word = {0};
	char str[10];

	memset(str,0,10);
	//#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",(uint32_t)g_gui_sports_record[0].distance,(uint32_t)(g_gui_sports_record[0].distance*100)%100);
	//#else
	//sprintf(str,"%.2f",g_gui_sports_record[0].distance);
	//#endif
	word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_49.height/2;
	word.y_axis = (LCD_LINE_CNT_MAX - strlen(str)*Font_Number_49.width)/2;
	word.size = LCD_FONT_49_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);//6.66
	
	word.x_axis += (Font_Number_49.height - Font_Number_24.height) + 5;
	word.y_axis += strlen(str)*Font_Number_49.width+4;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("km",&word);//km




	
	if(g_gui_sports_record[0].type == ACT_TRIATHLON_SWIM)
	{
		word.x_axis = LCD_LINE_CNT_MAX/2 + Font_Number_49.height/2 + 24;
		word.y_axis = 52;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("总时长:",&word);//总时长
	}
	else
	{
		word.x_axis = LCD_LINE_CNT_MAX/2 + Font_Number_49.height/2 + 24;
		word.y_axis = 60;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetString("时长:",&word);//时长
	}

	memset(str,0,10);
	sprintf(str,"%02d:%02d:%02d",g_gui_sports_record[0].time_elapse.hour, g_gui_sports_record[0].time_elapse.minute, g_gui_sports_record[0].time_elapse.second);
	word.x_axis += (16-19);
	word.y_axis = 60 + 2.5*16 + 8;
	word.size = LCD_FONT_19_SIZE;
	word.forecolor = LCD_PERU;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	if(g_gui_sports_record[0].type != ACT_TRIATHLON_SWIM && g_gui_sports_record[0].type != ACT_CLIMB)
	{
		word.x_axis += Font_Number_19.height + 10;
		word.y_axis = 60;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		if(g_gui_sports_record[0].type == ACT_CYCLING
		|| g_gui_sports_record[0].type == ACT_TRIATHLON_CYCLING)
		{
			LCD_SetString("速度:",&word);//速度
		}
		else
		{
			LCD_SetString("配速:",&word);//配速
		}
		
		
		memset(str,0,10);
		if(g_gui_sports_record[0].type == ACT_CYCLING
		|| g_gui_sports_record[0].type == ACT_TRIATHLON_CYCLING)
		{
			#if defined (SPRINTF_FLOAT_TO_INT)
			sprintf(str,"%d.%01d",(uint32_t)(g_gui_sports_record[0].avg_speed/10),(uint32_t)(g_gui_sports_record[0].avg_speed)%10);
			#else
			sprintf(str,"%.1f",g_gui_sports_record[0].avg_speed/10.f);
			#endif
		}
		else
		{
			sprintf(str,"%d:%02d",(int)(g_gui_sports_record[0].avg_speed)/60,(int)(g_gui_sports_record[0].avg_speed)%60);
		}
        
		word.x_axis += Font_Number_16.height - Font_Number_19.height;
		word.y_axis = 60 + 2.5*16 + 8;
		word.size = LCD_FONT_19_SIZE;
		word.forecolor = LCD_PERU;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		word.y_axis += strlen(str)*Font_Number_19.width + 8;
		
		word.x_axis += Font_Number_19.height - Font_Number_16.height;
		word.size = LCD_FONT_16_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		if(g_gui_sports_record[0].type == ACT_CYCLING
		|| g_gui_sports_record[0].type == ACT_TRIATHLON_CYCLING)
		{
			LCD_SetString("km/h",&word);
		}
		else
		{
			LCD_SetString("/km",&word);
		}
	}
	
}
/*界面显示:运动记录标题*/
static void gui_motionsrecord_title_paint(void)
{
	SetWord_t word = {0};
	
	char p_str[30];
	word.x_axis = 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	if(g_ActivityData_Total_Num == 0)//不会调
	{//无活动时,默认显示跑步界面
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_BLACK, &img_custom_sports[0]);
	}
	else
	{//有活动时
		
		LCD_SetPicture(word.x_axis, word.y_axis, LCD_PERU, LCD_BLACK, &img_custom_sports[getImgSport(g_gui_sports_record[0].type)]);
      word.x_axis += img_custom_sports[0].height - 4;
	  word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	   memset(p_str,0,sizeof(p_str));
	   sprintf(p_str,"%d-%02d-%02d %02d:%02d",g_gui_sports_record[0].year, g_gui_sports_record[0].month,g_gui_sports_record[0].date,
			g_gui_sports_record[0].time_stamp.hour, g_gui_sports_record[0].time_stamp.minute);
	   LCD_SetNumber(p_str,&word);	
	}

/*	word.x_axis = 60;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("上次运动",&word);*/
}
/*界面显示:运动记录上次运动界面*/
void gui_motionsrecord_last_paint(void)
{
	//设置背景黑色
	LCD_SetBackgroundColor(LCD_BLACK);
	if(g_ActivityData_Total_Num != 0)
	{
		gui_motionsrecord_title_paint();//标题
		gui_motionsrecord_mile_time_pace_paint();//里程 km 时长 00:43:00 配速 06:26/km
	}
	else
	{
	
		GUI_MOTION_RECORD_PRINTF("motionsrecord_last_paint\r\n");
		List_None();
	}
}

/*按键处理:运动记录上次运动界面*/
void gui_motionsrecord_last_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	home_or_minor_switch = MINOR_SCREEN_SWITCH;

	
	switch( Key_Value )
	{
		case KEY_OK:


		//查询遍历运动记录 活动总数 
		if(search_sport_record() != 0)
		{
		
			g_SportRecord_Going_Screen_Flag = 1;
			data_up_down_flag = 0;
			ScreenState = DISPLAY_SCREEN_SPORTS_RECORD;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}
			
			break;
		case KEY_UP:
			if(((SetValue.U_CustomMonitor.val & 0x01)== 0) &&(((SetValue.U_CustomMonitor.val>>1) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>2) & 0x01)== 0))
				{
				   
					s_switch_screen_count = 0;
					switch_home_screen = MONITOR_HOME_SCREEN;
					
					back_monitor_screen = DISPLAY_SCREEN_MOTIONS_RECORD;
                    ScreenState = DISPLAY_SCREEN_HOME;
			         //返回待机界面
			         msg.cmd = MSG_DISPLAY_SCREEN;
			         xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			    }
			else
				{
                  
				set_switch_screen_count_evt(Key_Value);
			    }
			break;
		case KEY_DOWN:
               
			if((((SetValue.U_CustomMonitor.val>>4) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>5) & 0x01)== 0))
						{
							switch_home_screen = MONITOR_HOME_SCREEN;
							
							back_monitor_screen = DISPLAY_SCREEN_MOTIONS_RECORD;
							ScreenState = DISPLAY_SCREEN_HOME;
							 //返回待机界面
							 msg.cmd = MSG_DISPLAY_SCREEN;
							 xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						}
					else
						{
						  
						set_switch_screen_count_evt(Key_Value);
						}

			break;
			
	
		case KEY_BACK:
			
			switch_home_screen = BACK_HOME_SCREEN;
			ScreenState = DISPLAY_SCREEN_HOME;
		
			back_monitor_screen = DISPLAY_SCREEN_MOTIONS_RECORD;
			//返回待机界面
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}
