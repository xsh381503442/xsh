#include "task_config.h"
#include "task_sport.h"
#include "task_hrt.h"
#include "task_tool.h"
#include "task_timer.h"
#include "task_step.h"
#include "task_gps.h"

#include "img_sport.h"

#include "isr_clkgen.h"

#include "bsp_timer.h"
#include "lib_boot_setting.h"

#include "com_sport.h"
#include "com_data.h"

#include "drv_spiflash.h"
#include "drv_lsm6dsl.h"
#include "drv_heartrate.h"
#include "drv_extFlash.h"
#include "drv_ms5837.h"
#include "drv_lis3mdl.h"

#include "timer_traceback.h"
#include "timer_app.h"
#include "time_notify.h"
#include "time_progress.h"

#include "algo_sport.h"
#include "algo_trackoptimizing.h"
#include "algo_swimming.h"
#include "algo_time_calibration.h"
#include "algo_magn_cal.h"

//#include "algo_sport_cycling.h"

#include "gui_sport_config.h"
#include "gui_run.h"
#include "gui_sport.h"
#include "gui_sport_walk.h"
#include "gui_sport_marathon.h"
#include "gui_sport_cycling.h"
#include "gui_sport_triathlon.h"
#include "gui_swimming.h"
#include "gui_crosscountry.h"
#include "gui_accessory.h"
#include "gui_tool_gps.h"
#include "algo_step_count.h"
#include "gui_notify_sport.h"
/*HF*/
#include "algo_HF_swimming_main.h"
#if DEBUG_ENABLED == 1 && GUI_SPORT_LOG_ENABLED == 1
	#define GUI_SPORT_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_SPORT_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_SPORT_WRITESTRING(...)
	#define GUI_SPORT_PRINTF(...)		        
#endif

#ifdef WATCH_SONY_GNSS	 
  extern uint8_t GPS_START_FLAG;
#endif

SemaphoreHandle_t ActSemaphore = NULL;

extern uint8_t  step_pattern_flag;//日常or运动
extern GPSMode m_GPS_data; 	   //GPS数据
extern gps_data_out gps_data_output;
extern GpspointStr gps_point;

TaskHandle_t TaskSportHandle = NULL;

volatile ScreenState_t Sport_Index;	//运动界面子索引

uint8_t g_SportSensorSW;	        //手表传感器开关
static uint8_t SaveSta;             //保存状态，0放弃，1保存
static bool PauseSta;               //暂停状态，0运动状态，1暂停状态

bool TaskSportStatus = false;               //运动task状态，false 挂起，true运行

int32_t Last_GPS_Lon;               //上次经纬度
int32_t Last_GPS_Lat; 
uint16_t SportAngle;

#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
uint8_t is_crosscountry = 0;//徒步越野、越野跑

#endif
#if defined WATCH_SPORT_EVENT_SCHEDULE
uint8_t g_start_search_cloud_status = 0;//云迹导航搜索方式0 无运动限制 1运动限制搜索
#endif

	
static uint32_t begin_steps;        //运动开始时步数
static uint32_t steps_record_f;				//稍后继续前计步记录
static uint32_t steps_record_t;				//稍后继续期间计步记录

static uint32_t steps_pause_f = 0;			//暂停前计步记录
static uint32_t steps_pause_t = 0;			//暂停期间计步记录
uint8_t gps_located = 1; //gps定位成功标记
static uint16_t ave_frequency =0;		//平均步频
static uint32_t hdrtotal,hdrtcount; //心率累加值,心率累加次数
static uint32_t cadtotal,cadcount; 	//踏频累加值,踏频累加次数

static uint32_t hdrlast,hdrctlast,distancelast; //上次心率值缓存值,上次心率计算缓存值，上次距离缓存值
static uint32_t cadlast,cadctlast;
#ifdef COD 

static uint32_t har_total_cod,har_count_toal_cod,ave_hdr_cod;
#endif
static uint8_t SwimSta;             //游泳状态，0非游泳，1游泳，uint8_t类型可扩充
static uint16_t Last_laps;          //游泳上次趟数
static uint32_t Last_time;					//游泳上圈时间
//static uint32_t Now_time;					//游泳上圈时间
static uint32_t m_LastCrankRevolutions;		//踏频圈数
static uint32_t m_LastCrankEventTime;		//踏频时间

static uint8_t s_cyclingsta = 0;	//骑行状态，0是非骑行运动，1是骑行运动，uint8_t类型可扩充

static uint32_t PauseTotolTime;     //总暂停时间，单位秒

extern void drv_lsm6dsl_acc_fifo_init( void);
extern void  drv_lsm6dsl_acc_fifo_uninit(void);
static float sport_press = 0,sport_temp = 0;//气压，高度,温度
float sport_alt = 0;//高度,

static uint8_t Is_Need_Hint_Flag = 0;	//训练计划运动完成后提醒一次
static TrainRecordStr m_sport_record;	//运动的日期及存储区域记录

static bool IsInSportMode = false;//是否处于运动模式中
static uint32_t g_SportTimeCount = 0;//运动中时间计时

extern uint8_t del_gps_point_num ;


extern _ble_accessory g_ble_accessory;
static rtc_time_t m_time_sport_ready;//运动开始前时间记录

extern GpspointStr pre_gps_point[OUTPUT_DATA_SUM_PRE];
extern uint32_t pre_distance[OUTPUT_DATA_SUM_PRE];
extern gps_data_sm gps_data_get_sm;

extern last_circle_t last_circle_data;

DistanceInputStr DistanceInput = {0}; 

uint8_t start_point_num=0;

#ifdef COD 
static uint32_t last_run_step = 0;
static uint32_t last_walk_step = 0;
static uint8_t half_marathon_flag = 0;
static uint8_t full_marathon_flag = 0;
static uint32_t last_strokes = 0;//上次的划数
static uint32_t algo_swim_strokes = 0;//算法输出划数
static uint32_t last_lap_stroke = 0; //上趟的总划数
static uint32_t last_lap_time = 0; //上趟的总时间
static uint32_t last_distance;
static uint32_t sport_total_time ;//运动总时长包括暂停
static uint16_t last_RiseHeight;
int16_t sport_altitude;
static float sport_RiseHeight;
static float sport_risedrop;

static uint32_t last_SportEnergy;
static uint32_t last_SportDistance;

static uint8_t sport_Risestate;
static int16_t sport_RiseAlt;
static uint32_t swim_store[5];
extern uint8_t last_pausesta;
extern int32_t last_store_Lon;
extern uint32_t last_sport_dis;

extern int32_t last_store_Lat;
extern uint32_t gps_store_num;
extern float last_altitude;
extern float algo_last_press_and_altitude[2];

extern int16_t start_alt;
extern float temp_dis;
extern float last_temp_dis;

extern float temp_increase_dis;
extern uint8_t cod_gps_curpoint_num;


extern GpspointStr kf_last_gps_point;


void marathon_cal_time(void)
{
	if ((half_marathon_flag==0)&& (ActivityData.ActivityDetails.CommonDetail.Distance >=2109750))
	{
		ActivityData.half_marathon_time = ActivityData.ActTime;
		half_marathon_flag = 1;
	}

	if ((full_marathon_flag==0)&& (ActivityData.ActivityDetails.CommonDetail.Distance >= 4219500))
	{
		ActivityData.full_marathon_time = ActivityData.ActTime;
		full_marathon_flag = 1;
	} 
}
#endif

void Set_IsInSportMode(bool flag)
{
	IsInSportMode = flag;
}
bool Get_IsInSportMode(void)
{
	return IsInSportMode;
}
/*获取运动上升下降高度，一分钟获取一次
调用示例:
Get_Sport_Altitude(&ActivityData.ActivityDetails.ClimbingDetail.RiseHeight,&ActivityData.ActivityDetails.ClimbingDetail.DropHeight)
*/
void Get_Sport_Altitude(uint16_t* RiseHeight,uint16_t* DropHeight)
{
#if !defined WATCH_SPORT_NO_PRESS
	//气压高度
	drv_ms5837_data_get(&sport_press,&sport_alt,&sport_temp);

	CalculateAltitude(sport_press,RiseHeight,DropHeight);
#endif
}

#if defined WATCH_SPORT_EVENT_SCHEDULE
SportEventStatusStr g_sport_event_status;//赛事赛段状态
SportScheduleCalStr g_sport_event_schedule_detail_cal;//赛事赛段状态详情
static uint8_t m_sport_pause_event_status = 0;//0:一般运动赛事结束 , 1:赛事赛段结束
void set_sport_pause_event_status(uint8_t status)
{
	m_sport_pause_event_status = status;
}
uint8_t get_sport_pause_event_status(void)
{
	return m_sport_pause_event_status;
}
#endif
//获取运动中心率
uint8_t Get_Sport_Heart(void)
{
    uint8_t hdrvalue;

   	//自带传感器心率
    if  (g_ble_accessory.heartrate.status == ACCESSORY_STATUS_DISCONNECT)
    {
        hdrvalue = drv_getHdrValue();
    }
    else
    {
       if (g_ble_accessory.data.hr_data_valid)//配件心率
       {
			hdrvalue = g_ble_accessory.data.hr_value;
       }
		else if (g_SportSensorSW & HEARTRATE_SW_MASK)//自带传感器心率
		{
			hdrvalue = drv_getHdrValue();
		}
		else
	    {
	  	    hdrvalue = 0;
	    }
    }

    return hdrvalue;
  
}
//心率恢复率计算
void calculate_heartrate_recovery(void)
{

	if (sport_end_flag == 1)
	{

		sport_end_time++;
		
		//if (sport_end_time == 1)
		//{
		//	sport_end_hrvalue = get_10s_heartrate(); //获取前10s平均心率，
		//}

		if (sport_end_time == 60)
		{
			sport_min_hrvalue = get_10s_heartrate(); //获取前10s平均心率，
			if (sport_end_hrvalue >= sport_min_hrvalue)
			{
				ActivityData.ActivityDetails.CommonDetail.HRRecovery = sport_end_hrvalue - sport_min_hrvalue;
			}
			else
			{
				ActivityData.ActivityDetails.CommonDetail.HRRecovery = 0;
			}
		}
		else if (sport_end_time == 175)
		{
			timer_notify_buzzer_start(300,300,1,false,NOTIFY_MODE_SPORTS);
			timer_notify_motor_start(300,300,1,false,NOTIFY_MODE_SPORTS);
		}
		else if (sport_end_time == 180)
		{
			if (Get_TaskSportSta()==true)
			{
				ScreenState = DISPLAY_SCREEN_SPORT_SAVING;			
				Set_SaveSta(1);
				CloseSportTask();
				gui_swich_msg();
				timer_progress_start(200);
			}

		}

	}

}


//将数据每秒输入算法，计算距离、速度、轨迹点
void sport_data_input(uint8_t pausesta,uint8_t stride,uint32_t steps,uint8_t activity_type)
{
	

    memset(&DistanceInput,0,sizeof(DistanceInput));
	
	
	
    DistanceInput.lon = GetGpsLon();
	DistanceInput.lat = GetGpsLat();
	DistanceInput.pause_status = pausesta;

	DistanceInput.stride = stride;
	DistanceInput.steps = steps;
	DistanceInput.activity_type = activity_type;
	DistanceInput.time = ActivityData.Act_Stop_Time.Hour*3600 + ActivityData.Act_Stop_Time.Minute*60 + ActivityData.Act_Stop_Time.Second;
	

  
  DistanceInput.gps_status = GetGpsStatus();
 
	if(activity_type == ACT_CLIMB)
	{
		//登山运动需输入上升高度和下降高度
		DistanceInput.up_value = ActivityData.ActivityDetails.ClimbingDetail.RiseHeight;
		DistanceInput.down_value = ActivityData.ActivityDetails.ClimbingDetail.DropHeight;
	}
	

	  if((start_point_num<REMOVE_START_GPS_SUM)&&(DistanceInput.gps_status==1))
		   	{
		   		DistanceInput.lon = 0;				
				DistanceInput.lat = 0;
             	DistanceInput.gps_status = 0;				   
			   start_point_num++;

		    }
		   else;
	TrackOptimizingDataAccept(DistanceInput);
}

//结束运动时，将最后数据送入算法，进行计算
void sport_data_end(uint8_t activity_type)
{
	uint16_t laps;
	uint8_t hdrminute,j;		//一分钟平均心率
    uint16_t speedminute;	//一分钟平均速度/配速
	uint32_t act_time = g_SportTimeCount;
	

	switch(activity_type)
	{
		case ACT_RUN:
		case ACT_TRIATHLON_RUN:
		case ACT_TRAIN_PLAN_RUN:
		case ACT_INDOORRUN:
			if(activity_type == ACT_TRAIN_PLAN_RUN)
			{
				//训练计划的详细数据归为跑步一类保存
				activity_type =  ACT_RUN;
			}
		
			//将数据送入到算法中
			sport_data_input(2,SetValue.AppGet.StepLengthRun,ActivityData.ActivityDetails.RunningDetail.Steps,activity_type);
				
			//距离
			ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();

			#ifdef COD 
			marathon_cal_time();
			#endif
		
			//卡路里
			ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);		
		
			//计算平均速度
			ActivityData.ActivityDetails.RunningDetail.Pace = CalculateSpeed(act_time,0,ActivityData.ActivityDetails.RunningDetail.Distance,activity_type);
			
			//运动时长小于1分钟时的最佳配速
			CalculateOptimumPace(ActivityData.ActivityDetails.RunningDetail.Pace, &ActivityData.ActivityDetails.RunningDetail.OptimumPace);
		    ActivityData.ActivityDetails.RunningDetail.Steps = ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
			//平均步频
		//	ActivityData.ActivityDetails.RunningDetail.frequency = CalculateFrequency(act_time,ActivityData.ActivityDetails.RunningDetail.Steps);
		
			//将步数计入日常数据中
			DayMainData.SportSteps += ActivityData.ActivityDetails.RunningDetail.Steps;
			#ifdef COD
			if (act_time %60 != 0)
			{
				Store_cod_StepData(&ActivityData.ActivityDetails.RunningDetail.Steps,&sport_total_time);
				Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
			}
			if (act_time %20 != 0)
			{
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_cod_HeartRateData(&hdrminute,&sport_total_time);
				if(hdrminute!=0)
					{
					har_total_cod+=hdrminute;
					har_count_toal_cod++;
					}
				else;
				ave_hdr_cod = har_total_cod/har_count_toal_cod;
			}
			#else
			if ((act_time %60) > 10)
			{
				//最后一分钟平均配速
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//最后一分钟平均心率
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			#endif
			break;
		case ACT_WALK:
			//将数据送入到算法中
			sport_data_input(2,SetValue.AppGet.StepLengthWalking,ActivityData.ActivityDetails.WalkingDetail.Steps,ACT_WALK);
				
			//距离
			ActivityData.ActivityDetails.WalkingDetail.Distance = DistanceResult();
		
			//卡路里
			ActivityData.ActivityDetails.WalkingDetail.Calorie = CalculateWalkEnergy(ActivityData.ActivityDetails.WalkingDetail.Distance/100,SetValue.AppSet.Weight);		
		
			//计算平均速度
			ActivityData.ActivityDetails.WalkingDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.WalkingDetail.Distance,ACT_WALK);
		
			//运动时长小于1分钟时的最佳配速
			CalculateOptimumPace(ActivityData.ActivityDetails.WalkingDetail.Speed, &ActivityData.ActivityDetails.WalkingDetail.OptimumSpeed);
			ActivityData.ActivityDetails.WalkingDetail.Steps  = ActivityData.ActivityDetails.WalkingDetail.Steps - steps_pause_t;
			//平均步频
			ActivityData.ActivityDetails.WalkingDetail.frequency = CalculateFrequency(act_time,ActivityData.ActivityDetails.WalkingDetail.Steps);
		
			//将步数计入日常数据中
			DayMainData.SportSteps += ActivityData.ActivityDetails.WalkingDetail.Steps;
			if ((act_time %60) > 10)
			{
				//最后一分钟平均配速
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//最后一分钟平均心率
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			break;
		case ACT_MARATHON:
			//将数据送入到算法中
			sport_data_input(2,SetValue.AppGet.StepLengthMarathon,ActivityData.ActivityDetails.RunningDetail.Steps,ACT_MARATHON);
				
			//距离
			ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();
		
			//卡路里
			ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);		
		
			//计算平均速度
			ActivityData.ActivityDetails.RunningDetail.Pace = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.RunningDetail.Distance,ACT_MARATHON);
			
			//运动时长小于1分钟时的最佳配速
			CalculateOptimumPace(ActivityData.ActivityDetails.RunningDetail.Pace, &ActivityData.ActivityDetails.RunningDetail.OptimumPace);
			ActivityData.ActivityDetails.RunningDetail.Steps =ActivityData.ActivityDetails.RunningDetail.Steps -steps_pause_t;
			//平均步频
		//	ActivityData.ActivityDetails.RunningDetail.frequency = CalculateFrequency(act_time,ActivityData.ActivityDetails.RunningDetail.Steps);
		
			//将步数计入日常数据中
			DayMainData.SportSteps +=ActivityData.ActivityDetails.RunningDetail.Steps;
			if ((act_time %60) > 10)
			{
				//最后一分钟平均配速
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//最后一分钟平均心率
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			break;
		case ACT_SWIMMING:
			//结束时趟数可能增加，距离相关需要计算一次
			//laps = SwimDataCircleResult();
			//laps = SwimResultsOutputPort2_LengthCount();
			laps = SwimResultsPostProcessing();//post-processing
		
			//Average frequency
			ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency_swimming(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
			#ifdef COD 
			ActivityData.ActivityDetails.SwimmingDetail.pool_length = SetValue.AppGet.LengthSwimmingPool;
			ActivityData.ActivityDetails.SwimmingDetail.laps = laps;
			
			#endif
			//平均swolf
			ActivityData.ActivityDetails.SwimmingDetail.Swolf = CalculateSwolf(ActivityData.ActivityDetails.SwimmingDetail.Strokes,laps,act_time);
			//卡路里
			ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex);
			
			if(laps > Last_laps)
			{
				//距离
				ActivityData.ActivityDetails.SwimmingDetail.Distance = SetValue.AppGet.LengthSwimmingPool *100 *laps;

				//平均泳速
				ActivityData.ActivityDetails.SwimmingDetail.Speed = CalculateSpeed(act_time,0,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);

				//平均swolf
				//ActivityData.ActivityDetails.SwimmingDetail.Swolf = CalculateSwolf(ActivityData.ActivityDetails.SwimmingDetail.Strokes,laps,act_time);

				//卡路里
				//ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex); 

				//平均划频
				//ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);


				//划数存储
				Store_StepData(&ActivityData.ActivityDetails.SwimmingDetail.Strokes);

				//距离存储
				Store_DistanceData(&ActivityData.ActivityDetails.SwimmingDetail.Distance);

				//卡路里存储
				Store_EnergyData(& ActivityData.ActivityDetails.SwimmingDetail.Calorie);
							

				//计圈时间
				
#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
				Store_CircleTimeData(act_time ,SetValue.SportSet.SwSwimmg);
#endif
							
				//一趟平均速度
				speedminute = CalculateSpeed((act_time - Last_time),distancelast,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);
				
				Store_SpeedData(&speedminute);																											

				//一趟平均心率													
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount - hdrctlast);

				Store_HeartRateData(&hdrminute);
			}
			break;
		case ACT_CROSSCOUNTRY:
			//将数据送入到算法中
			sport_data_input(2,SetValue.AppGet.StepLengthCountryRace,ActivityData.ActivityDetails.CrosscountryDetail.Steps,ACT_CROSSCOUNTRY);
			
			//距离
			ActivityData.ActivityDetails.CrosscountryDetail.Distance = DistanceResult();

			#ifdef COD 
			marathon_cal_time();
			#endif
		
			//卡路里计算
			ActivityData.ActivityDetails.CrosscountryDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,act_time,SetValue.AppSet.Weight);	

			//计算平均速度
			ActivityData.ActivityDetails.CrosscountryDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.CrosscountryDetail.Distance,activity_type);
		
			//运动时长小于1分钟时的最佳配速
			CalculateOptimumPace(ActivityData.ActivityDetails.CrosscountryDetail.Speed, &ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed);
			ActivityData.ActivityDetails.CrosscountryDetail.Steps = ActivityData.ActivityDetails.CrosscountryDetail.Steps -steps_pause_t;
			//将步数计入日常数据中
			DayMainData.SportSteps += ActivityData.ActivityDetails.CrosscountryDetail.Steps;
			#ifdef COD
			if (act_time %60 != 0)
			{
				Store_cod_StepData(&ActivityData.ActivityDetails.RunningDetail.Steps,&sport_total_time);
				Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
			}
			if (act_time %20 != 0)
			{
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_cod_HeartRateData(&hdrminute,&sport_total_time);
				if(hdrminute!=0)
					{
					har_total_cod+=hdrminute;
					har_count_toal_cod++;
					}
				else;
				ave_hdr_cod = har_total_cod/har_count_toal_cod;
			}
			#else
			if ((act_time %60) > 10)
			{
				//最后一分钟平均配速
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//最后一分钟平均心率
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			#endif
			break;
		case ACT_HIKING:
			//将数据送入到算法中
			sport_data_input(2,SetValue.AppGet.StepLengthHike,ActivityData.ActivityDetails.CrosscountryDetail.Steps,ACT_HIKING);
			
			//距离
			ActivityData.ActivityDetails.CrosscountryDetail.Distance = DistanceResult();
			#ifdef COD 
			marathon_cal_time();
			#endif
			//卡路里
			ActivityData.ActivityDetails.CrosscountryDetail.Calorie = CalculateWalkEnergy(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,SetValue.AppSet.Weight);	
		
			//计算平均速度
			ActivityData.ActivityDetails.CrosscountryDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.CrosscountryDetail.Distance,activity_type);

		  //运动时长小于1分钟时的最佳配速
			CalculateOptimumPace(ActivityData.ActivityDetails.CrosscountryDetail.Speed, &ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed);
			ActivityData.ActivityDetails.CrosscountryDetail.Steps = ActivityData.ActivityDetails.CrosscountryDetail.Steps -steps_pause_t;
			//将步数计入日常数据中
			DayMainData.SportSteps += ActivityData.ActivityDetails.CrosscountryDetail.Steps;
			#ifdef COD
			if (act_time %60 != 0)
			{
				Store_cod_StepData(&ActivityData.ActivityDetails.RunningDetail.Steps,&sport_total_time);
				Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
			}
			if (act_time %20 != 0)
			{
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_cod_HeartRateData(&hdrminute,&sport_total_time);
				if(hdrminute!=0)
					{
					har_total_cod+=hdrminute;
					har_count_toal_cod++;
					}
				else;
				ave_hdr_cod = har_total_cod/har_count_toal_cod;
			}
			#else
			if ((act_time %60) > 10)
			{
				//最后一分钟平均配速
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//最后一分钟平均心率
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			#endif
			break;
		case ACT_CLIMB:
			//将数据送入到算法中
			sport_data_input(2,SetValue.AppGet.StepLengthClimbing,ActivityData.ActivityDetails.ClimbingDetail.Steps,ACT_CLIMB);
			
			//距离
			ActivityData.ActivityDetails.ClimbingDetail.Distance = DistanceResult();

			#ifdef COD 
			marathon_cal_time();
			#endif
		
			//登山计算垂直速度
			ActivityData.ActivityDetails.ClimbingDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.ClimbingDetail.DropHeight 
												+ ActivityData.ActivityDetails.ClimbingDetail.RiseHeight,ACT_CLIMB);

			ActivityData.ActivityDetails.ClimbingDetail.Steps = ActivityData.ActivityDetails.ClimbingDetail.Steps -steps_pause_t;
			//将步数计入日常数据中
			DayMainData.SportSteps += ActivityData.ActivityDetails.ClimbingDetail.Steps ;
			#ifdef COD
			if (act_time %60 != 0)
			{
				Store_cod_StepData(&ActivityData.ActivityDetails.RunningDetail.Steps,&sport_total_time);
				Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
			}
			if (act_time %20 != 0)
			{
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_cod_HeartRateData(&hdrminute,&sport_total_time);
				if(hdrminute!=0)
					{
					har_total_cod+=hdrminute;
					har_count_toal_cod++;
					}
				else;
				ave_hdr_cod = har_total_cod/har_count_toal_cod;
			}
			if (ActivityData.ActivityDetails.ClimbingDetail.OptimumSpeed < (CalculateSpeed(act_time,0,ActivityData.ActivityDetails.ClimbingDetail.Distance ,ACT_CYCLING)))
			{
				ActivityData.ActivityDetails.ClimbingDetail.OptimumSpeed = (CalculateSpeed(act_time,0,ActivityData.ActivityDetails.ClimbingDetail.Distance ,ACT_CYCLING));
			}
			#else
			if ((act_time %60) > 10)
			{
				//最后一分钟平均配速
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//最后一分钟平均心率
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			#endif
			break;
		case ACT_CYCLING:
		case ACT_TRIATHLON_CYCLING:
			//将数据送入到算法中
			sport_data_input(2,0,ActivityData.ActivityDetails.CyclingDetail.Cadence,ACT_CYCLING);
			
			//距离
			ActivityData.ActivityDetails.CyclingDetail.Distance = DistanceResult();
		
			//计算平均速度
			ActivityData.ActivityDetails.CyclingDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.CyclingDetail.Distance,activity_type);
		
			//小于1分钟的最佳速度
			CalculateOptimumSpeed(ActivityData.ActivityDetails.CyclingDetail.Speed, &ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed);
		
			//计算平均踏频
			if(cadcount != 0)
			{
				ActivityData.ActivityDetails.CyclingDetail.Cadence = cadtotal/cadcount;
			}
			#ifdef COD
			if (act_time %20 != 0)
			{
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_cod_HeartRateData(&hdrminute,&sport_total_time);
				if(hdrminute!=0)
					{
					har_total_cod+=hdrminute;
					har_count_toal_cod++;
					}
				else;
				ave_hdr_cod = har_total_cod/har_count_toal_cod;
			}
			#endif
			break;
		case ACT_TRIATHLON_SWIM:
			//将数据送入到算法中
			sport_data_input(2,0,ActivityData.ActivityDetails.SwimmingDetail.Strokes,ACT_TRIATHLON_SWIM);
		
			//距离
			ActivityData.ActivityDetails.SwimmingDetail.Distance = DistanceResult();
		
			//卡路里
			ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex);
		
			//计算平均速度
			ActivityData.ActivityDetails.SwimmingDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.SwimmingDetail.Distance,activity_type);
			break;
		default:
			break;
	}

	#ifdef COD	
	DayMainData.SportEnergy = last_SportEnergy +  ActivityData.ActivityDetails.CommonDetail.Calorie;
	if((activity_type != ACT_SWIMMING )&& (activity_type !=ACT_CYCLING))
	{
		DayMainData.SportDistance =last_SportDistance + ActivityData.ActivityDetails.CommonDetail.Distance ;
	}
	#else
	//所有运动的距离、卡路里数据都计入日常数据中
	DayMainData.SportEnergy += ActivityData.ActivityDetails.CommonDetail.Calorie;
	DayMainData.SportDistance += ActivityData.ActivityDetails.CommonDetail.Distance;
	#endif

	if (SPI_No_Semaphore == 0)
	{
		Send_Timer_Cmd(CMD_HOME_STEP);
   }
	if(g_SportSensorSW & GPS_SW_MASK)
	{
	#ifdef COD 

		if(gps_data_output.output_data_sum > OUTPUT_DATA_SUM_PRE)
			{
								
				gps_data_output.output_data_sum = OUTPUT_DATA_SUM_PRE;
			}
						
			for(j = 0; j < gps_data_output.output_data_sum; j++)
				{

				
							   
					Store_GPSData(pre_gps_point[j],pre_distance[j]);
				}
  
	
	#else
		//保存剩下的轨迹点

	#endif
	}
}

//获取骑行的平均踏频
uint32_t Get_Cycling_AvgCadence(void)
{
	uint32_t angCadence = 0;
	
	if(cadcount != 0)
	{
		angCadence = cadtotal/cadcount;
	}
	
	return angCadence;
}

//获取总暂停时间
uint32_t Get_PauseTotolTime(void)
{
    return PauseTotolTime;
}

//获取运动task状态
bool Get_TaskSportSta(void)
{
    return TaskSportStatus;
}
    
//设置运动task状态
void Set_TaskSportSta(bool sta)
{
    TaskSportStatus = sta;
    
}
uint16_t get_ave_frequency(void)
{
	return ave_frequency;
}

void set_ave_frequency(uint16_t frequency)
{
	ave_frequency = frequency;
}

//获取暂停状态
bool Get_PauseSta(void)
{
    return PauseSta;
}

 
//设置暂停状态
void Set_PauseSta(bool sta)
{
    PauseSta = sta;
	
//	//获取当前时间
//	am_hal_rtc_time_get(&RTC_time);
	
    if (PauseSta == true)
    {
        memset(&PauseTime,0,sizeof(PauseTime));
		steps_pause_f = drv_lsm6dsl_get_stepcount();
		#ifdef COD 
		sport_total_time = PauseTotolTime +g_SportTimeCount;
		#endif
//		//暂停开始时间
//		PauseTime.Pause_Start_Time.Year= RTC_time.ui32Year; 
//		PauseTime.Pause_Start_Time.Month= RTC_time.ui32Month;
//		PauseTime.Pause_Start_Time.Day= RTC_time.ui32DayOfMonth;
//		PauseTime.Pause_Start_Time.Hour = RTC_time.ui32Hour; 
//		PauseTime.Pause_Start_Time.Minute = RTC_time.ui32Minute;
//		PauseTime.Pause_Start_Time.Second = RTC_time.ui32Second;
    }
	else
	{
	 	steps_pause_t += (drv_lsm6dsl_get_stepcount() - steps_pause_f);
//		//暂停结束时间
//		PauseTime.Pause_Stop_Time.Year= RTC_time.ui32Year; 
//		PauseTime.Pause_Stop_Time.Month= RTC_time.ui32Month;
//		PauseTime.Pause_Stop_Time.Day= RTC_time.ui32DayOfMonth;
//		PauseTime.Pause_Stop_Time.Hour = RTC_time.ui32Hour; 
//		PauseTime.Pause_Stop_Time.Minute = RTC_time.ui32Minute;
//		PauseTime.Pause_Stop_Time.Second = RTC_time.ui32Second;
//		
//		//保存暂停时间
//		Store_PauseTimeData(&PauseTime);
	}
}

//获取游泳状态
uint8_t Get_SwimSta(void)
{
    return SwimSta;
}

//设置游泳状态
void Set_SwimSta(uint8_t sta)
{
    SwimSta = sta;
}


//获取骑行状态
uint8_t get_cyclingsta(void)
{
    return s_cyclingsta;
}

//设置骑行状态
void set_cyclingsta(uint8_t sta)
{
    s_cyclingsta = sta;
}



//获取保存状态
uint8_t Get_SaveSta(void)
{
    return SaveSta;
}

//设置保存状态
void Set_SaveSta(uint8_t sta)
{
    SaveSta = sta;
}

//创建/唤醒运动任务
void CreatSportTask(void)
{
    if( TaskSportHandle == NULL )
    {
        // ActSemaphore = xSemaphoreCreateBinary();  //创建二值信号量
        //如果未创建任务
        xTaskCreate(TaskSport,"Task Sport",TaskSport_StackDepth,0,TaskSport_Priority,&TaskSportHandle);
    } 
    else
    {
        vTaskResume(TaskSportHandle);

    }
}

//退出运动模式
void CloseSportTask(void)
{
    //退出运动模式
	TaskSportStatus = false;
	#if defined (WATCH_STEP_ALGO)
	#else
	move_int =1;
	#endif
	xSemaphoreGive(ActSemaphore);
   
	//如果是循迹返航,需删除循迹返航定时器或导航
	if((TRACE_BACK_DATA.is_traceback_flag == 1) ||(loadflag ==1))
	{
		trace_back_timer_delete();
	}
	if((ActivityData.Activity_Type >= ACT_TRIATHLON_SWIM) && (ActivityData.Activity_Type <= ACT_TRIATHLON_RUN))
	{
		//结束铁人三项运动中
		SetValue.TriathlonSport.IsTriathlonSport = 0;
		SetValue.TriathlonSport.IndexTriathlon = 0;
		SetValue.TriathlonSport.year = 0;
		SetValue.TriathlonSport.month = 0;
		SetValue.TriathlonSport.day = 0;
		SetValue.TriathlonSport.hour = 0;
		SetValue.TriathlonSport.minute = 0;
		SetValue.TriathlonSport.second = 0;
		TASK_SPORT_PRINTF("[task_sport]:TriathlonSport-Stop!!!IsTriathlonSport=%d\n",SetValue.TriathlonSport.IsTriathlonSport);
	}
	Set_IsInSportMode(false);
	Set_IsTrainPlanOnGoing(false);
}

//准备运动
void ReadySport(ScreenState_t index)
{
   	//清除运动的数据
	memset(&ActivityData,0,sizeof(ActivityData));
	g_SportTimeCount = 0;//运动计时时间计数清零
	//运动模式开启
  Set_IsInSportMode(true);
	switch(index)
	{
		case DISPLAY_SCREEN_RUN:
			//初始化数据
			ActivityData.Activity_Type = ACT_RUN;
			#ifdef COD 
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK |HEIGHT_SW_MASK;//计步、心率、、GPS
			#else
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//计步、心率、、GPS
			#endif
								
			break;
		case DISPLAY_SCREEN_TRAINPLAN_RUN:
			Set_IsTrainPlanOnGoing(true);
			ActivityData.Activity_Type = ACT_TRAIN_PLAN_RUN;
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//计步、心率、、GPS
			g_sport_status = SPORT_STATUS_READY;
			Is_Need_Hint_Flag	= 0;
			break;
		case DISPLAY_SCREEN_MARATHON:
			ActivityData.Activity_Type = ACT_MARATHON;
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//计步、心率、、GPS
			break;			
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			ActivityData.Activity_Type = ACT_CROSSCOUNTRY;
			g_SportSensorSW = STEP_SW_MASK | COMPASS_SW_MASK | HEARTRATE_SW_MASK |
								HEIGHT_SW_MASK | GPS_SW_MASK;//计步、指北针、心率、高度、GPS
			break;		
		case DISPLAY_SCREEN_INDOORRUN:
			ActivityData.Activity_Type = ACT_INDOORRUN;
			g_SportSensorSW =  STEP_SW_MASK | HEARTRATE_SW_MASK ;//计步、心率
			break;			
		case DISPLAY_SCREEN_WALK:
			ActivityData.Activity_Type = ACT_WALK;
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//计步、心率、、GPS
			
			break;
		case DISPLAY_SCREEN_CLIMBING:
			ActivityData.Activity_Type = ACT_CLIMB;
			g_SportSensorSW = STEP_SW_MASK | COMPASS_SW_MASK | HEARTRATE_SW_MASK |
								HEIGHT_SW_MASK | GPS_SW_MASK;//计步、指北针、心率、高度、GPS
			break;	
		case DISPLAY_SCREEN_CYCLING:
			ActivityData.Activity_Type = ACT_CYCLING;
			g_SportSensorSW = COMPASS_SW_MASK | HEARTRATE_SW_MASK |
								HEIGHT_SW_MASK | GPS_SW_MASK;//指北针、心率、高度、GPS
	
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			ActivityData.Activity_Type = ACT_HIKING;
			g_SportSensorSW = STEP_SW_MASK | COMPASS_SW_MASK | HEARTRATE_SW_MASK | 
                                HEIGHT_SW_MASK | GPS_SW_MASK ;//计步、指北针、心率、高度、GPS
			break;		
		case DISPLAY_SCREEN_SWIMMING:
			ActivityData.Activity_Type = ACT_SWIMMING;
			g_SportSensorSW = STEP_SW_MASK | GYRO_SW_MASK ;//计步、陀螺仪
			break;
         case DISPLAY_SCREEN_TRIATHLON:
			ActivityData.Activity_Type = ACT_TRIATHLON_SWIM;
			g_SportSensorSW =  STEP_SW_MASK | GYRO_SW_MASK | 
								HEARTRATE_SW_MASK | GPS_SW_MASK;//计步、陀螺仪、心率、GPS
			break;		
		case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			ActivityData.Activity_Type = ACT_TRIATHLON_CYCLING;;
			g_SportSensorSW = COMPASS_SW_MASK | HEARTRATE_SW_MASK |
								HEIGHT_SW_MASK | GPS_SW_MASK;//指北针、心率、高度、GPS
			break;
        case DISPLAY_SCREEN_TRIATHLON_RUNNING:
			ActivityData.Activity_Type = ACT_TRIATHLON_RUN;
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//计步、心率、、GPS
			break;	
		default:
			break;
     }
    //心率值清零
    drv_clearHdrValue();
    if(g_SportSensorSW & HEARTRATE_SW_MASK)
		{
			//心率初始操作
      task_hrt_start();
		}
    //GPS初始操作,不管是否开gps，都清零
		Last_GPS_Lon = 0;
    Last_GPS_Lat = 0;
		sport_alt = 0;
	
#if !defined WATCH_SIM_SPORT
    gps_value_clear();
#endif
    //初始化轨迹参数
    InitTrack();

//gps定位成功标记
	gps_located = 0;

#if defined WATCH_SIM_SPORT
		//平均步频清零
	set_ave_frequency(0);
#endif
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		//关门时间清零
	memset(&g_track_mark_data,0,sizeof(SportTrackMarkDataStr));
#endif
	if(g_SportSensorSW & GPS_SW_MASK)
	{

       if(drv_ublox_status_get() == GPS_OFF)
       {
           //如果GPS没有打开，重新开启
           CreateGPSTask();
       }
		
	}
    else
    {
        //对于不用开启GPS的运动任务，关闭GPS
 #if defined WATCH_SIM_SPORT
 #else
        //drv_ublox_status_set(GPS_OFF);
 #endif
    }
   
}
//退出运动准备界面
 void ReadySportBack(void)
{
  //清除运动的数据
  memset(&ActivityData,0,sizeof(ActivityData));

  if(g_SportSensorSW & GPS_SW_MASK)
	{
#if defined WATCH_SIM_SPORT
#else
	  //CloseGPSTask();
#endif		
	}
	if(g_SportSensorSW & HEARTRATE_SW_MASK)
	{
       // task_hrt_close();
	}
  g_SportSensorSW = 0;
	Set_IsInSportMode(false);
	Set_IsTrainPlanOnGoing(false);
}
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
//退出云迹导航运动准备界面 不用关GPS 如果是运动界面进入的不用关心率 工具界面进入的关心率及其他运动参数
void ReadyCloudSportBack(void)
{
	TASK_SPORT_PRINTF("[task_sport]:ReadyCloudSportBack screen=%d\r\n",g_tool_or_sport_goto_screen);
	if(g_tool_or_sport_goto_screen == DISPLAY_SCREEN_GPS_DETAIL)
	{
		//清除运动的数据
		memset(&ActivityData,0,sizeof(ActivityData));

		if(g_SportSensorSW & HEARTRATE_SW_MASK)
		{
			task_hrt_close();
		}
		g_SportSensorSW = 0;
		Set_IsInSportMode(false);
		Set_IsTrainPlanOnGoing(false);
	}
}
#endif

void OpenSensor(void)
{
 // float press;
  //uint16_t altitude;//气压，高度
  	//清除标志
	SportContinueLaterFlagClr();
 	step_pattern_flag = 1;
	if(g_SportSensorSW & STEP_SW_MASK)
	{
        steps_record_t += drv_lsm6dsl_get_stepcount() - steps_record_f;
		steps_pause_f = drv_lsm6dsl_get_stepcount();
	}
	if(g_SportSensorSW & HEARTRATE_SW_MASK)
	{
      task_hrt_start();
	}

    if(g_SportSensorSW & COMPASS_SW_MASK)
	{
#if !defined WATCH_SPORT_NO_COMPASS 
		//指北针初始操作
		drv_lis3mdl_reset();
        am_util_delay_ms(30);
       // drv_lis3mdl_init();
		magn_dev_para_init();
#endif		
	}
    
	if(g_SportSensorSW & HEIGHT_SW_MASK)
	{
        //press = 0;
        //altitude = 0;
        //CalculateAltitude(press,&altitude,&altitude);
        Get_Sport_Altitude(&ActivityData.ActivityDetails.CommonDetail.RiseHeight,&ActivityData.ActivityDetails.CommonDetail.DropHeight);
	}
	if(g_SportSensorSW & GYRO_SW_MASK)
	{
	}
		if(g_SportSensorSW & GPS_SW_MASK)
	{

       if(drv_ublox_status_get() == GPS_OFF)
       {
           //如果GPS没有打开，重新开启
           CreateGPSTask();
       }
		
	}
  else
  {
        //对于不用开启GPS的运动任务，关闭GPS
        #ifdef WATCH_SONY_GNSS   
          GPS_START_FLAG = 0;
        #endif
  	drv_ublox_status_set(GPS_OFF);
  }
 
    if ((ActivityData.Activity_Type == ACT_SWIMMING)||(ActivityData.Activity_Type == ACT_TRIATHLON_SWIM))
    {
				Last_laps =0;
			  Last_time =0;
        //SwimAlgorithmInit();
			 ParameterInitialization(SetValue.AppGet.LengthSwimmingPool);
			 drv_lsm6dsl_gyro_init();  
			 drv_lsm6dsl_fifo_init();  
		#if defined (WATCH_STEP_ALGO)
		
		#else
		drv_lsm6dsl_acc_fifo_init();
        drv_lsm6dsl_disable_pedometer();
        #endif
        Set_SwimSta(1);
    }
    else
    {
        Set_SwimSta(0);
    }
     if ((ActivityData.Activity_Type == ACT_CYCLING)||(ActivityData.Activity_Type == ACT_TRIATHLON_CYCLING))
    {
			  algo_sport_cycling_init();
			 
         //关闭传感器计步算法
        #if defined (WATCH_STEP_ALGO) 
	//		    drv_lsm6dsl_gyro_init();  
	//		    drv_lsm6dsl_fifo_init(); 
		    #else
		  	  drv_lsm6dsl_acc_fifo_init();
      		drv_lsm6dsl_disable_pedometer();
      	 #endif
			
			  set_cyclingsta(1);
    }
	else
	{
			  set_cyclingsta(0);
			 
	}
//    PauseTotolTime = 0;
//    memset(&PauseTime,0,sizeof(PauseTime));
		am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC); //设为秒中断
}
void CloseSensor(void)
{	
	//am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_MIN); 
	if(g_SportSensorSW & STEP_SW_MASK)
	{
	    steps_record_f = drv_lsm6dsl_get_stepcount();
		steps_pause_t += (drv_lsm6dsl_get_stepcount() - steps_pause_f);
		
	}
	step_pattern_flag = 0;
	
	if(g_SportSensorSW & GPS_SW_MASK)
	{
        CloseGPSTask();
	}
	if(g_SportSensorSW & HEARTRATE_SW_MASK)
	{
        task_hrt_close();
	}
	if(g_SportSensorSW & HEIGHT_SW_MASK)
	{
	}
	//关闭指北针
	if(g_SportSensorSW & COMPASS_SW_MASK)
	{
#if !defined WATCH_SPORT_NO_COMPASS 

        drv_lis3mdl_standby();
#endif
	}
	if(g_SportSensorSW & GYRO_SW_MASK)
	{
	}
    if ((ActivityData.Activity_Type == ACT_SWIMMING)||(ActivityData.Activity_Type == ACT_TRIATHLON_SWIM))
    {
        Set_SwimSta(0);
    	#if defined (WATCH_STEP_ALGO)
		#else
		drv_lsm6dsl_enable_pedometer();
		 drv_lsm6dsl_acc_fifo_uninit();
		#endif
        drv_lsm6dsl_fifo_uninit();
			  drv_lsm6dsl_gyro_powerdown();
			  drv_lsm6dsl_acc_init();
			  drv_lsm6dsl_acc_fifo_init();
        //SwimAlgorithmEnd();
			  SwimmingAlgorithmsEnd();
			ALGO_HF_SWIM_PRINTF("------------ Close Sensor--------------\n");
    }

    if ((ActivityData.Activity_Type == ACT_CYCLING)||(ActivityData.Activity_Type == ACT_TRIATHLON_CYCLING))
    {
			  set_cyclingsta(0);
			  
         //打开传感器计步算法
       	#if defined (WATCH_STEP_ALGO)
//		      drv_lsm6dsl_fifo_uninit();
//			    drv_lsm6dsl_gyro_powerdown();
//			    drv_lsm6dsl_acc_init();
//			    drv_lsm6dsl_acc_fifo_init();
		    #else
		      drv_lsm6dsl_acc_fifo_uninit();
		      drv_lsm6dsl_enable_pedometer();
		    #endif
    }
}

//铁人三项换项
void sport_triathlon_change_start(void)
{
	uint8_t type = ActivityData.Activity_Type;

	//暂停运动
	Set_PauseSta(true);
	
	//保存数据
	//SaveSport(1);
	
	//关闭传感器，不关闭GPS
	g_SportSensorSW &= (~GPS_SW_MASK);
	//CloseSensor();
	Set_SaveSta(1);
	TaskSportStatus = false;
#if defined (WATCH_STEP_ALGO)
#else
	move_int =1;
#endif
	xSemaphoreGive(ActSemaphore);
	vTaskDelay(300/portTICK_PERIOD_MS);
	
	//清除运动的数据
	memset(&ActivityData,0,sizeof(ActivityData));
	
	//开始下项运动
	ActivityData.Activity_Type = type + 1;
	if((ActivityData.Activity_Type == ACT_TRIATHLON_FIRST) 
		|| (ActivityData.Activity_Type == ACT_TRIATHLON_SECOND))
	{
		//换项轨迹
		g_SportSensorSW = GPS_SW_MASK;	//GPS
	}
	else if(ActivityData.Activity_Type == ACT_TRIATHLON_CYCLING)
	{
		//骑行
		g_SportSensorSW = COMPASS_SW_MASK | HEARTRATE_SW_MASK |
							HEIGHT_SW_MASK | GPS_SW_MASK;	//指北针、心率、高度、GPS
	}
	else
	{
		//跑步
		g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | 
							GPS_SW_MASK;	//计步、心率、GPS
	}
	
	//运动计时时间计数清零
	g_SportTimeCount = 0;

	//gps定位成功标记
	gps_located = 0;
	
	StartSport();
	if(g_SportSensorSW & HEARTRATE_SW_MASK)
	{
		//心率初始操作
		task_hrt_start();
	}
}

//铁人三项选择放弃后，并删除前面几项运动
static void sport_triathlon_cancel(void)
{
	uint8_t i,index,temp;
	uint8_t buf[2] = {0};
	
	index = m_sport_record.ActivityIndex;
	for(i = ActivityData.Activity_Type - 1; i >= ACT_TRIATHLON_SWIM; i--)
	{
		if(index <= 0)
		{
			//已是这天第一个活动
			break;
		}
		else
		{
			index -= 1;
		}
		dev_extFlash_enable();
		dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*m_sport_record.DayIndex+ACTIVITY_DATA_START_ADDRESS + 
															ACTIVITY_DAY_DATA_SIZE*(index-1),&temp,1);

		if(i == temp)
		{
			dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*m_sport_record.DayIndex+ACTIVITY_DATA_START_ADDRESS + 
								ACTIVITY_DAY_DATA_SIZE*(index-1),buf,2);
		}
		dev_extFlash_disable();
	}
}

//运动选择稍后继续
void sport_continue_later_start(void)
{
	//如果是循迹返航,需删除循迹返航定时器或导航
	if((TRACE_BACK_DATA.is_traceback_flag == 1)||(loadflag ==1))
	{
		trace_back_timer_delete();
	}
	//关闭传感器
	CloseSensor();
	
	//关闭传感器,并挂起运动任务
//	CloseSportTask();
	
	//置位标志
	SportContinueLaterFlagSet();
}

//运动结束稍后继续
void sport_continue_later_end(void)
{
	//清除标志
	SportContinueLaterFlagClr();
	
	//开启传感器
	OpenSensor();
	//唤醒运动任务
//	CreatSportTask();
	
	//如果是循迹返航或导航,需重新开始循迹返航定时器
	if((TRACE_BACK_DATA.is_traceback_flag == 1)||(loadflag ==1))
	{
		trace_back_timer_start();
	}
}
/**
  * @brief  设置运动记录的索引/年月日  保存到flash时用
* @param  save_starttime该运动真实的其实时间 save_stoptime结束运动时的时间
					beforetime  运动开始前的表中的时间
          is_stopsport  为了区分之前代码用
两个参数满足  运动时长=save_stoptime-save_starttime;
  * @retval
  */
static void SetSportRecord(rtc_time_t beforetime,rtc_time_t save_starttime,rtc_time_t save_stoptime,bool is_stopsport)
{
	if((ActivityData.Activity_Type > ACT_TRIATHLON_SWIM) && (ActivityData.Activity_Type <= ACT_TRIATHLON_RUN))
	{
		//铁人三项后两项运动
		m_sport_record.ActivityIndex = ActivityData.Activity_Index;
	}
	else
	{
		if(is_stopsport)
		{//运动停止后保存
			if( !(save_starttime.Year == beforetime.Year && save_starttime.Month == beforetime.Month
			  && save_starttime.Day == beforetime.Day) )
			{//运动开始前时间与运动真实的开始时间不一致 活动索引号重新获取 否则不变
				ActivityData.Activity_Index = SetValue.CurrentAddress.IndexActivity+1;
			}
			if(save_starttime.Year == save_stoptime.Year && save_starttime.Month == save_stoptime.Month
			 && save_starttime.Day == save_stoptime.Day)
			{//同一天运动 运动日索引和活动序号 
				m_sport_record.DayIndex = DayMainData.Index;
			}
			else
			{//不同天 这里主要是运动跨0点的活动;运动日索引0点被清零了,需要记录前天的
				m_sport_record.DayIndex = DayMainData.Index;
				if(m_sport_record.DayIndex == 0)
				{
					m_sport_record.DayIndex = 6;
				}
				else
				{
					m_sport_record.DayIndex --;
				}
			}
			m_sport_record.ActivityIndex = ActivityData.Activity_Index;
			m_sport_record.Year = save_starttime.Year;
			m_sport_record.Month = save_starttime.Month;
			m_sport_record.Date = save_starttime.Day;
		}
		else
		{//运动开始
			m_sport_record.DayIndex = DayMainData.Index;
			m_sport_record.ActivityIndex = ActivityData.Activity_Index;
			m_sport_record.Year = save_starttime.Year;
			m_sport_record.Month = save_starttime.Month;
			m_sport_record.Date = save_starttime.Day;
		}
		TASK_SPORT_PRINTF("[task_sport]:----SetSportRecord----m_sport_record.DayIndex=%d,ActivityIndex=%dyear=%d,month=%d,day=%d\n"
			,m_sport_record.DayIndex,m_sport_record.ActivityIndex,m_sport_record.Year,m_sport_record.Month,m_sport_record.Date);
	}
}
//开始运动   
void StartSport(void)
{
	float press;
	uint16_t altitude;//气压，高度
	steps_record_t = 0;    //运动开始时清零
	steps_record_f = 0;
	
	timer_timeout_back_stop(false);

	//清除运动的数据
	//memset(&ActivityData,0,sizeof(ActivityData));
	
	//如果是从云迹导航或者运动轨迹开始的运动，需要挂起工具任务
	if((ENTER_NAVIGATION == 1) || (ENTER_MOTION_TRAIL == 1))
	{
		//挂起工具任务
		tool_task_close();
	}
	
	SetValue.IsAllowBaseHDR = 0;//开启运动后不允许基础心率功能包括提醒保存等
	am_hal_rtc_time_get(&RTC_time); //获取当前时间
	ActivityData.Act_Start_Time.Year= RTC_time.ui32Year;
	ActivityData.Act_Start_Time.Month= RTC_time.ui32Month;
	ActivityData.Act_Start_Time.Day= RTC_time.ui32DayOfMonth;
	ActivityData.Act_Start_Time.Hour = RTC_time.ui32Hour;
	ActivityData.Act_Start_Time.Minute = RTC_time.ui32Minute;
	ActivityData.Act_Start_Time.Second = RTC_time.ui32Second;
	ActivityData.Act_Stop_Time.Hour = 0;  //活动过程中停止时间结构体保存持续时间
	ActivityData.Act_Stop_Time.Minute = 0;
	ActivityData.Act_Stop_Time.Second = 0;
	ActivityData.Activity_Index = SetValue.CurrentAddress.IndexActivity+1;	
	#ifdef COD
	if ((SetValue.CurrentAddress.AddressStep % 8) != 0)
	{
		SetValue.CurrentAddress.AddressStep += (8 - (SetValue.CurrentAddress.AddressStep % 8));
	}

	if ((SetValue.CurrentAddress.AddressHeartRate % 8) != 0)
	{
		SetValue.CurrentAddress.AddressHeartRate += (8 - (SetValue.CurrentAddress.AddressHeartRate % 8));
	}
	#endif
	ActivityData.Steps_Start_Address = SetValue.CurrentAddress.AddressStep;
	ActivityData.HeartRate_Start_Address = SetValue.CurrentAddress.AddressHeartRate;	
	ActivityData.Pressure_Start_Address = SetValue.CurrentAddress.AddressPressure;	
	ActivityData.GPS_Start_Address = SetValue.CurrentAddress.AddressGps;
	ActivityData.Distance_Start_Address = SetValue.CurrentAddress.AddressDistance;
	ActivityData.Energy_Start_Address = SetValue.CurrentAddress.AddressEnergy;
	ActivityData.Speed_Start_Address = SetValue.CurrentAddress.AddressSpeed;
	ActivityData.DistancePoint_Start_Address = SetValue.CurrentAddress.AddressDistancePoint;
	ActivityData.CircleTime_Start_Address = SetValue.CurrentAddress.AddressCircleTime;
#ifdef COD
#else
	ActivityData.VerticalSize_Start_Address = SetValue.CurrentAddress.AddressVerticalSize;
#if defined WATCH_SPORT_EVENT_SCHEDULE
	memset(&ActivityData.sport_event.U_EventSport.val,0,sizeof(uint64_t));
#else
	ActivityData.TouchDown_Start_Address = SetValue.CurrentAddress.AddressTouchDown;
#endif
	ActivityData.Pause_Start_Address = SetValue.CurrentAddress.AddressPause;//实际用于存储航迹打点数据
#endif
	//运动开始前把当前时间记录下来
	m_time_sport_ready = ActivityData.Act_Start_Time;
	//保存当前活动的索引数据，用于保存运动数据，避免跨天
	m_sport_record.ActivityType = ActivityData.Activity_Type;
	if(ActivityData.Activity_Type == ACT_TRIATHLON_SWIM)
	{
		//处于铁人三项运动中
		SetValue.TriathlonSport.IsTriathlonSport = 1;
		SetValue.TriathlonSport.IndexTriathlon = ActivityData.Activity_Index;
		SetValue.TriathlonSport.year = RTC_time.ui32Year;
		SetValue.TriathlonSport.month = RTC_time.ui32Month;
		SetValue.TriathlonSport.day = RTC_time.ui32DayOfMonth;
		SetValue.TriathlonSport.hour = RTC_time.ui32Hour;
		SetValue.TriathlonSport.minute = RTC_time.ui32Minute;
		SetValue.TriathlonSport.second = RTC_time.ui32Second;
		TASK_SPORT_PRINTF("[task_sport]:TriathlonSport-Start:SetValue.TriathlonSport.IndexTriathlon=%d,IsTriathlonSport=%d\n"
		 ,SetValue.TriathlonSport.IndexTriathlon,SetValue.TriathlonSport.IsTriathlonSport);
	}
	SetSportRecord(m_time_sport_ready,ActivityData.Act_Start_Time,ActivityData.Act_Stop_Time,false);
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	is_crosscountry = 0;
#endif
	
     switch(ActivityData.Activity_Type)
     {
        //计圈距离保存
        case ACT_RUN:
        case ACT_TRIATHLON_RUN:
				case ACT_TRAIN_PLAN_RUN:
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistanceRun;
            break;
        case ACT_WALK:
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistanceWalk;
            break;
        case ACT_MARATHON:
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistanceMarathon;
						break;
        case ACT_SWIMMING:
            ActivityData.CircleDistance = SetValue.AppGet.LengthSwimmingPool;
            break;
        case ACT_INDOORRUN:
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistanceIndoorRun;
            break;
        case ACT_CROSSCOUNTRY:
		{
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistanceCountryRace;
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
			is_crosscountry = 1;
#endif
		}
            break;
        case ACT_HIKING:
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistanceHike;
            break;
        case ACT_CLIMB:
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistanceClimbing;
						break;
        case ACT_CYCLING:
        case ACT_TRIATHLON_CYCLING:
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistanceCycling;
            break;
        case ACT_TRIATHLON_SWIM:
            ActivityData.CircleDistance = SetValue.SportSet.DefCircleDistancOutdoorSwimmg;
            break;        

     }
	//初始化心率恢复率为250，表示心率恢复率无效
  	ActivityData.ActivityDetails.CommonDetail.HRRecovery = 250;
	 
	//心率累加值清零，因可能心率配件中途加入，开始就清零
    hdrtotal = 0;
    hdrtcount = 0;
    hdrlast = 0;
    hdrctlast = 0;
	har_total_cod = 0;
	har_count_toal_cod = 0;
	ave_hdr_cod = 0;
	//踏频
	cadtotal = 0;
	cadcount = 0;
	cadlast = 0;
    cadctlast = 0;
	m_LastCrankRevolutions = 0;
	m_LastCrankEventTime = 0;

    //上次距离缓存清零
    distancelast = 0;
	
	//gps点存储顺序
	store_gps_nums = 0;

	//航迹打点初始化
	dot_track_nums = 0;
	dottrack_draw_time = 0;
	dottrack_out_time = 0;	//航迹打点到上限显示次数
	dottrack_nogps_time = 0;//航迹打点gps没有定位显示次数
	
	
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
			gps_store_flag = false;
#endif
	
	//心率恢复率相关参数
	 sport_end_flag = 0;  //是否按运动结束选项标志
	 sport_end_hrvalue = 0;   //运动结束时心率值
	 sport_min_hrvalue = 0;   //运动一分钟后心率值
	 sport_end_time = 0;    //运动结束
#ifdef COD 
	if ((ActivityData.Activity_Type == ACT_RUN)||(ActivityData.Activity_Type == ACT_CROSSCOUNTRY))
	{
		last_run_step = SetValue.run_step;
	}
	else if (ActivityData.Activity_Type == ACT_HIKING)
	{
		last_walk_step = SetValue.walk_step;
	}
	half_marathon_flag = 0;
    full_marathon_flag = 0;
	last_pausesta  = 0xFF;
	last_store_Lon = 0;
	last_store_Lat = 0;
	last_sport_dis = 0;
	gps_store_num = 0;
	last_strokes = 0;
	algo_swim_strokes = 0;
	last_lap_stroke = 0;
	last_lap_time = 0;
	last_distance = 0;
	last_RiseHeight = 0;
	sport_total_time = 0;
    last_altitude = 0;
	algo_last_press_and_altitude[0] = 101.325;
	
	algo_last_press_and_altitude[1] = 0;
	temp_dis = 0;
	last_temp_dis = 0;
	temp_increase_dis = 0;
    start_alt = 0;


	
	last_SportEnergy = DayMainData.SportEnergy;
	last_SportDistance = DayMainData.SportDistance;
	memset(&swim_store,0,sizeof(swim_store));
  
  memset(&kf_last_gps_point,0,sizeof(kf_last_gps_point));
  
  memset(&pre_gps_point,0,sizeof(pre_gps_point));
  
  memset(&gps_point,0,sizeof(gps_point));
	
 memset(&pre_distance,0,sizeof(pre_distance));

	

	sport_altitude = 0;
	sport_RiseHeight = 0;
	sport_risedrop = 0;
	sport_Risestate = 0;
	sport_RiseAlt = 0;
	cod_gps_curpoint_num = 0;
    del_gps_point_num = 0;
       
	
	kf_initial(); 
#endif
	 
  

#if defined WATCH_SPORT_EVENT_SCHEDULE	 
	 set_sport_pause_event_status(0);//赛事赛段初始化选择 默认选择一般运动结束
#endif
	//算法初始化
	TrackOptimizingInit();
     
	if(g_SportSensorSW & STEP_SW_MASK)
	{
      //计步初始操作
        begin_steps = drv_lsm6dsl_get_stepcount();
	
	}
	
	if(g_SportSensorSW & HEIGHT_SW_MASK)
	{
		//高度初始操作	
        press = 0;
        altitude = 0;
		ReadSeaLevelPressure(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
       //初始化算法
        CalculateAltitude(press,&altitude,&altitude);
        //第一个气压值
        Get_Sport_Altitude(&ActivityData.ActivityDetails.CommonDetail.RiseHeight,&ActivityData.ActivityDetails.CommonDetail.DropHeight);		
		
	}
	

	if(g_SportSensorSW & GYRO_SW_MASK)
	{
		//陀螺仪初始操作
		
	}

    if(g_SportSensorSW & COMPASS_SW_MASK)
	{
#if !defined WATCH_SPORT_NO_COMPASS 

		//指北针初始操作
		drv_lis3mdl_reset();
        am_util_delay_ms(30);
       // drv_lis3mdl_init();
		magn_dev_para_init();
#endif	
	}

    
    //游泳初始化操作
    if ((ActivityData.Activity_Type == ACT_SWIMMING)||(ActivityData.Activity_Type == ACT_TRIATHLON_SWIM))
    {
        Last_laps =0;
		Last_time =0;
       //SwimAlgorithmInit();
		ParameterInitialization(SetValue.AppGet.LengthSwimmingPool);
      	drv_lsm6dsl_gyro_init();  
		drv_lsm6dsl_fifo_init();  
		#if defined (WATCH_STEP_ALGO)
		#else
		drv_lsm6dsl_acc_fifo_init();
        drv_lsm6dsl_disable_pedometer();//关闭传感器计步算法
        #endif
        Set_SwimSta(1);
    }
    else
    {
        Set_SwimSta(0);
    }

    if ((ActivityData.Activity_Type == ACT_CYCLING)||(ActivityData.Activity_Type == ACT_TRIATHLON_CYCLING))
    {
			  algo_sport_cycling_init();
			
         //关闭传感器计步算法
        #if defined (WATCH_STEP_ALGO)
//			    drv_lsm6dsl_gyro_init();  
//			    drv_lsm6dsl_fifo_init(); 
		    #else
		 
		      drv_lsm6dsl_fifo_init();
          drv_lsm6dsl_disable_pedometer();
      	#endif 
		
			  set_cyclingsta(1);
    }
		else
		{
			  set_cyclingsta(0);
			 
		}
		
		//原始数据存储初始化
  #if defined STORE_ORG_DATA_TEST_VERSION
//    Store_OrgData_Start();
  #endif
		
    //暂停时间清零
    PauseTotolTime = 0;
    memset(&PauseTime,0,sizeof(PauseTime));

    //初始化运动提醒
    Remind_Sport_Init();
	
	//自动计圈提醒数据清零
	memset(&last_circle_data,0,sizeof(last_circle_data));
	
    //导航开启是否轨迹上循环显示定时器
    if (loadflag ==1)
    {
        trace_back_timer_start();
    }
    //开启运动task
    Set_PauseSta(false);
    Set_TaskSportSta(true);
	steps_pause_f = 0;
	steps_pause_t = 0;
	step_pattern_flag = 1;
	CreatSportTask();	
	am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC); //设为秒中断
	
}
#if defined WATCH_SPORT_EVENT_SCHEDULE
/*新建赛事 第1赛段*/
static void set_new_sport_event(void)
{
	g_sport_event_status.status = 1;
	g_sport_event_status.nums = 1;
	TASK_SPORT_PRINTF("[task_sport]:--set_new_sport_event.nums=%d\r\n",g_sport_event_status.nums);
	ActivityData.sport_event.U_EventSport.bits.Year = ActivityData.Act_Start_Time.Year;
	ActivityData.sport_event.U_EventSport.bits.Month = ActivityData.Act_Start_Time.Month;
	ActivityData.sport_event.U_EventSport.bits.Day = ActivityData.Act_Start_Time.Day;
	ActivityData.sport_event.U_EventSport.bits.Hour = ActivityData.Act_Start_Time.Hour;
	ActivityData.sport_event.U_EventSport.bits.Minute = ActivityData.Act_Start_Time.Minute;
	ActivityData.sport_event.U_EventSport.bits.Second_Start = ActivityData.Act_Start_Time.Second;
	ActivityData.sport_event.U_EventSport.bits.Second_End = ActivityData.Act_Stop_Time.Second;
	ActivityData.sport_event.U_EventSport.bits.Event_Status = g_sport_event_status.status;
	ActivityData.sport_event.U_EventSport.bits.Shedule_Nums = g_sport_event_status.nums;
	
	memcpy(&g_sport_event_status.event_str,&ActivityData.sport_event,sizeof(SportEventStr));
	g_sport_event_status.event_id = get_sport_event_id(ActivityData.sport_event);//新建赛事ID
	TASK_SPORT_PRINTF("[task_sport]:--set_new_sport_event sport_type=%d,status=%d,total_nums=%d,num=%d,event_id=%d\r\n"
	,g_sport_event_status.sport_type,g_sport_event_status.status,g_sport_event_status.total_nums,g_sport_event_status.nums,g_sport_event_status.event_id);
}
#endif
/*
功能：退出活动，改变相应保存值。
参数：sta    是否保存活动数据， 0 为放弃   ，非零 保存	
返回值:无
描述：
*/
void SaveSport( uint8_t	sta )
{
	if( sta )
	{
		ActivityData.ActTime = ActivityData.Act_Stop_Time.Hour*3600 + ActivityData.Act_Stop_Time.Minute*60 + ActivityData.Act_Stop_Time.Second;
		
		//训练计划的详细数据归为跑步一类保存
		if(ACT_TRAIN_PLAN_RUN == ActivityData.Activity_Type)
		{
			ActivityData.Activity_Type = ACT_RUN;
		}
		
		//结束运动时，将最后所有数据送入算法
		sport_data_end(ActivityData.Activity_Type);
		#ifdef COD 
		if ((ActivityData.Activity_Type == ACT_RUN)||(ActivityData.Activity_Type == ACT_CROSSCOUNTRY))
		{
			SetValue.run_step =  last_run_step + ActivityData.ActivityDetails.CommonDetail.Steps;
			ActivityData.ActivityDetails.CommonDetail.Speed = 
				CalculateSpeed(ActivityData.ActTime,0,(ActivityData.ActivityDetails.CommonDetail.Distance/100*100),ACT_RUN);
		}
		else if (ActivityData.Activity_Type == ACT_HIKING)
		{
			SetValue.walk_step =  last_walk_step + ActivityData.ActivityDetails.CommonDetail.Steps;
			ActivityData.ActivityDetails.CommonDetail.Speed = 
				CalculateSpeed(ActivityData.ActTime,0,(ActivityData.ActivityDetails.CommonDetail.Distance/100*100),ACT_RUN);
		}
		else if (ActivityData.Activity_Type == ACT_SWIMMING)
		{
			uint32_t swim_total_store = swim_store[Freestyle] + swim_store[Backstroke]+ swim_store[Breaststroke]+ swim_store[ButterflyStroke]+ swim_store[UnknownStyle];
			for (uint8_t swim_type_num = 0;swim_type_num <= UnknownStyle;swim_type_num++)
			{
				//主泳姿
				if ((swim_store[swim_type_num] *100/swim_total_store)> 50)
				{
					ActivityData.ActivityDetails.SwimmingDetail.Stroke = swim_type_num;
					break;
				}
				else
				{
					ActivityData.ActivityDetails.SwimmingDetail.Stroke = Medley;
				}
			}
			
			//划数
			ActivityData.ActivityDetails.SwimmingDetail.Strokes = 2*swim_store[Freestyle] + 2*swim_store[Backstroke]+ swim_store[Breaststroke]+ swim_store[ButterflyStroke]+ swim_store[UnknownStyle];
		}
		else if (ActivityData.Activity_Type == ACT_CYCLING)
		{
			ActivityData.ActivityDetails.CyclingDetail.Speed = (uint32_t)(ActivityData.ActivityDetails.CyclingDetail.Speed/36.f*10)/10.f*36;
			ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed = (uint32_t)(ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed/36.f*10)/10.f*36;
		}
		#endif
		//计算运动结束时数据
		//ActivityData.ActivityDetails.CommonDetail.AvgHeart = hdrtotal/hdrtcount;
		ActivityData.ActivityDetails.CommonDetail.AvgHeart = ave_hdr_cod;
		#ifndef COD 
		ActivityData.ActivityDetails.CommonDetail.AerobicEffect = GetAerobicIntensity(ActivityData.ActivityDetails.CommonDetail.HeartRateZone);
		ActivityData.ActivityDetails.CommonDetail.AnaerobicEffect = GetAnaerobicIntensity(ActivityData.ActivityDetails.CommonDetail.HeartRateZone);
		#endif
		//恢复时间
		switch(ActivityData.Activity_Type)
		{
			case ACT_RUN:
			case ACT_MARATHON:
			case ACT_CROSSCOUNTRY:
			case ACT_HIKING:
			case ACT_INDOORRUN:
			case ACT_WALK:
				ActivityData.ActivityDetails.CommonDetail.RecoveryTime = CalculateRecoveryTime(ActivityData.ActivityDetails.CommonDetail.Distance/100,
					ActivityData.ActivityDetails.CommonDetail,ActivityData.ActTime,ActivityData.Activity_Type);
				
				//当用户第二天的基础心率≤前一天基础心率，则当天再次运动，恢复时间不进行累积
				if ((DayMainData.BasedHeartrate != 0)&&(DayMainData.BasedHeartrate <=last_basedheartrate ))
				{
					//当次运动所需要的恢复时间≤当前的恢复时间时，显示当前恢复时间
					//当次运动所需要的恢复时间＞当前的恢复时间时，显示当次运动所需要恢复时间
					if (ActivityData.ActivityDetails.CommonDetail.RecoveryTime > SetValue.RecoverTime )
					{
						SetValue.RecoverTime = ActivityData.ActivityDetails.CommonDetail.RecoveryTime;
					}
				}
				else //用户未测试基础心率或第二天基础心率＞前一天基础心率，当天再次运动，恢复时间进行累积
				{
					SetValue.RecoverTime +=ActivityData.ActivityDetails.CommonDetail.RecoveryTime;
				}
				
				if (SetValue.RecoverTime > 20160) //大于14天
				{
					SetValue.RecoverTime  = 20160;
				}
				SetValue.RecoverTimeTotal = SetValue.RecoverTime;
				break;
			default:
				break;
		}
		
            
		am_hal_rtc_time_get(&RTC_time); //获取当前时间
		ActivityData.Act_Stop_Time.Year= RTC_time.ui32Year;  //保存活动停止时间
		ActivityData.Act_Stop_Time.Month= RTC_time.ui32Month;
		ActivityData.Act_Stop_Time.Day= RTC_time.ui32DayOfMonth;
		ActivityData.Act_Stop_Time.Hour = RTC_time.ui32Hour;
		ActivityData.Act_Stop_Time.Minute = RTC_time.ui32Minute;
		ActivityData.Act_Stop_Time.Second = RTC_time.ui32Second;
		
		//活动开始时间=结束时间-运动时长-运动暂停时长  单位秒
		unsigned int startTime = GetTimeSecond(ActivityData.Act_Stop_Time) - g_SportTimeCount - PauseTotolTime;
		
		ActivityData.Act_Start_Time = GetYmdhms(startTime);//秒换算成年月日  活动开始时间
		TASK_SPORT_PRINTF("[task_sport]:startTime=%d,g_SportTimeCount=%d,PauseTotolTime=%d,Start_Time %d-%d-%d %d:%d:%d GetTimeSecond()=%ld\n",
		     startTime,g_SportTimeCount,PauseTotolTime,ActivityData.Act_Start_Time.Year,ActivityData.Act_Start_Time.Month,
		     ActivityData.Act_Start_Time.Day,ActivityData.Act_Start_Time.Hour,ActivityData.Act_Start_Time.Minute,
		     ActivityData.Act_Start_Time.Second,GetTimeSecond(ActivityData.Act_Stop_Time));
		
		ActivityData.Steps_Stop_Address = SetValue.CurrentAddress.AddressStep;
		ActivityData.HeartRate_Stop_Address = SetValue.CurrentAddress.AddressHeartRate;
		ActivityData.Pressure_Stop_Address = SetValue.CurrentAddress.AddressPressure;
		ActivityData.GPS_Stop_Address = SetValue.CurrentAddress.AddressGps;
		ActivityData.Distance_Stop_Address = SetValue.CurrentAddress.AddressDistance;
		ActivityData.Energy_Stop_Address = SetValue.CurrentAddress.AddressEnergy;
		ActivityData.Speed_Stop_Address = SetValue.CurrentAddress.AddressSpeed;
		ActivityData.DistancePoint_Stop_Address = SetValue.CurrentAddress.AddressDistancePoint;
		ActivityData.CircleTime_Stop_Address = SetValue.CurrentAddress.AddressCircleTime;
#ifdef COD
		_boot_setting boot_setting;
		lib_boot_setting_read(&boot_setting);	
		ActivityData.cod_version = boot_setting.application_version;
		
		ActivityData.agps_update_time = GetTimeSecond(SetValue.agps_update_time);

#else
		ActivityData.VerticalSize_Stop_Address = SetValue.CurrentAddress.AddressVerticalSize;
#if defined WATCH_SPORT_EVENT_SCHEDULE
//		TASK_SPORT_PRINTF("[task_sport]:--00000->>g_sport_event_status sport_type=%d,status=%d,total_nums=%d,num=%d,event_id=%d\r\n"
//				,g_sport_event_status.sport_type,g_sport_event_status.status,g_sport_event_status.total_nums,g_sport_event_status.nums,g_sport_event_status.event_id);
		if(is_sport_type_valid(ActivityData.Activity_Type))
		{//属于运动赛事类型 保存赛事
			if(get_sport_pause_event_status() == 0)
			{//选择运动结束
				if(g_sport_event_status.event_id != 0)
				{//已有赛事
					if(g_sport_event_status.nums < SPORT_EVENT_SCHEDULE_NUM_MAX)
					{
						g_sport_event_status.status = 0;
						g_sport_event_status.nums += 1;
						//保存赛事ID
						ActivityData.sport_event.U_EventSport.bits.Year = g_sport_event_status.event_str.U_EventSport.bits.Year;
						ActivityData.sport_event.U_EventSport.bits.Month = g_sport_event_status.event_str.U_EventSport.bits.Month;
						ActivityData.sport_event.U_EventSport.bits.Day = g_sport_event_status.event_str.U_EventSport.bits.Day;
						ActivityData.sport_event.U_EventSport.bits.Hour = g_sport_event_status.event_str.U_EventSport.bits.Hour;
						ActivityData.sport_event.U_EventSport.bits.Minute = g_sport_event_status.event_str.U_EventSport.bits.Minute;
						ActivityData.sport_event.U_EventSport.bits.Second_Start = g_sport_event_status.event_str.U_EventSport.bits.Second_Start;
						ActivityData.sport_event.U_EventSport.bits.Second_End = g_sport_event_status.event_str.U_EventSport.bits.Second_End;
						ActivityData.sport_event.U_EventSport.bits.Event_Status = g_sport_event_status.status;
						ActivityData.sport_event.U_EventSport.bits.Shedule_Nums = g_sport_event_status.nums;
					}
					else
					{//超过赛程 重新新建赛事
						//新建赛事
						set_new_sport_event();
					}
				}
				else
				{//没有赛事或者重新开始 结束运动 云迹导航界面不保存赛事
					if(g_sport_event_is_restart_status == 1 && g_tool_or_sport_goto_screen != DISPLAY_SCREEN_GPS_DETAIL)
					{
						if(g_sport_event_status.nums < SPORT_EVENT_SCHEDULE_NUM_MAX)
						{
							g_sport_event_status.status = 0;
							g_sport_event_status.nums = 0;
							//保存赛事ID
							ActivityData.sport_event.U_EventSport.bits.Year = g_sport_event_status.event_str.U_EventSport.bits.Year;
							ActivityData.sport_event.U_EventSport.bits.Month = g_sport_event_status.event_str.U_EventSport.bits.Month;
							ActivityData.sport_event.U_EventSport.bits.Day = g_sport_event_status.event_str.U_EventSport.bits.Day;
							ActivityData.sport_event.U_EventSport.bits.Hour = g_sport_event_status.event_str.U_EventSport.bits.Hour;
							ActivityData.sport_event.U_EventSport.bits.Minute = g_sport_event_status.event_str.U_EventSport.bits.Minute;
							ActivityData.sport_event.U_EventSport.bits.Second_Start = g_sport_event_status.event_str.U_EventSport.bits.Second_Start;
							ActivityData.sport_event.U_EventSport.bits.Second_End = g_sport_event_status.event_str.U_EventSport.bits.Second_End;
							ActivityData.sport_event.U_EventSport.bits.Event_Status = g_sport_event_status.status;
							ActivityData.sport_event.U_EventSport.bits.Shedule_Nums = g_sport_event_status.nums;
						}
					}
				}
			}
			else if(get_sport_pause_event_status() == 1)
			{//选择赛段结束
				if(g_sport_event_status.event_id != 0)
				{//已有赛事
					if(g_sport_event_status.nums < SPORT_EVENT_SCHEDULE_NUM_MAX)
					{//保存赛事
						g_sport_event_status.status = 1;
						g_sport_event_status.nums += 1;
						//保存赛事
						ActivityData.sport_event.U_EventSport.bits.Year = g_sport_event_status.event_str.U_EventSport.bits.Year;
						ActivityData.sport_event.U_EventSport.bits.Month = g_sport_event_status.event_str.U_EventSport.bits.Month;
						ActivityData.sport_event.U_EventSport.bits.Day = g_sport_event_status.event_str.U_EventSport.bits.Day;
						ActivityData.sport_event.U_EventSport.bits.Hour = g_sport_event_status.event_str.U_EventSport.bits.Hour;
						ActivityData.sport_event.U_EventSport.bits.Minute = g_sport_event_status.event_str.U_EventSport.bits.Minute;
						ActivityData.sport_event.U_EventSport.bits.Second_Start = g_sport_event_status.event_str.U_EventSport.bits.Second_Start;
						ActivityData.sport_event.U_EventSport.bits.Second_End = g_sport_event_status.event_str.U_EventSport.bits.Second_End;
						ActivityData.sport_event.U_EventSport.bits.Event_Status = g_sport_event_status.status;
						ActivityData.sport_event.U_EventSport.bits.Shedule_Nums = g_sport_event_status.nums;
					}
					else
					{//新建赛事
						set_new_sport_event();
					}

				}
				else
				{//没有赛事 
						set_new_sport_event();
				}
				g_sport_event_status.sport_type = ActivityData.Activity_Type;
				g_sport_event_status.nums = ActivityData.sport_event.U_EventSport.bits.Shedule_Nums;
				g_sport_event_status.total_nums = ActivityData.sport_event.U_EventSport.bits.Shedule_Nums;
			}
			memcpy(&g_sport_event_status.event_str,&ActivityData.sport_event,sizeof(SportEventStr));
			TASK_SPORT_PRINTF("[task_sport]:--1->>event_status=%d,ActivityData.sport_event.U_EventSport.bits=%d-%02d-%02d %d:%02d:%02d-endsec:%02d,status=%d,num=%d\r\n"
				,get_sport_pause_event_status()
				,ActivityData.sport_event.U_EventSport.bits.Year,ActivityData.sport_event.U_EventSport.bits.Month,ActivityData.sport_event.U_EventSport.bits.Day
				,ActivityData.sport_event.U_EventSport.bits.Hour,ActivityData.sport_event.U_EventSport.bits.Minute,ActivityData.sport_event.U_EventSport.bits.Second_Start
				,ActivityData.sport_event.U_EventSport.bits.Second_End,ActivityData.sport_event.U_EventSport.bits.Event_Status,ActivityData.sport_event.U_EventSport.bits.Shedule_Nums
				);
			TASK_SPORT_PRINTF("[task_sport]:--2->>g_sport_event_status nums=%d,sport_type=%d,status=%d,total_nums=%d,event_id=%d\r\n"
				,g_sport_event_status.nums,g_sport_event_status.sport_type,g_sport_event_status.status,g_sport_event_status.total_nums,g_sport_event_status.event_id);
		}
#else
		ActivityData.TouchDown_Stop_Address = SetValue.CurrentAddress.AddressTouchDown;
#endif
		ActivityData.Pause_Stop_Address = SetValue.CurrentAddress.AddressPause;//实际用于存储航迹打点数据
#endif
		SetSportRecord(m_time_sport_ready,ActivityData.Act_Start_Time,ActivityData.Act_Stop_Time,true);

		//如果大于最大日活动数，则不保存活动
		if(m_sport_record.ActivityIndex <= DAY_ACTIVITY_MAX_NUM)
		{
			if(RTC_time.ui32DayOfMonth == m_sport_record.Date && RTC_time.ui32Month == m_sport_record.Month
				 && RTC_time.ui32Year == m_sport_record.Year)
			{//相同天 活动索引自增处理 
				//没有运动跨天
				SetValue.CurrentAddress.IndexActivity++;
			}
			SetValue.TrainRecordIndex[m_sport_record.DayIndex].ActivityNum++;

			if(SetValue.IsFirstSport == 1)
			{//运动保存后，第一次运动记录状态改变
				SetValue.IsFirstSport = 0;
			}
			#ifdef COD 
			SetValue.CurrentAddress.Current_cod_id++;
			ActivityData.cod_id = SetValue.CurrentAddress.Current_cod_id;
			#endif 
			dev_extFlash_enable();
			dev_extFlash_write( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*m_sport_record.DayIndex+ACTIVITY_DATA_START_ADDRESS + 
																ACTIVITY_DAY_DATA_SIZE*(m_sport_record.ActivityIndex-1),(uint8_t*)(&ActivityData),ACTIVITY_DAY_DATA_SIZE);
#if defined WATCH_SPORT_EVENT_SCHEDULE
//			ActivityDataStr m_ActivityData;
//			SportEventStr m_test_event;
//			int64_t m_event_id = 0,m_test_id = 0;
//			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*m_sport_record.DayIndex+ACTIVITY_DATA_START_ADDRESS + 
//																ACTIVITY_DAY_DATA_SIZE*(m_sport_record.ActivityIndex-1),(uint8_t*)(&m_ActivityData),ACTIVITY_DAY_DATA_SIZE);
//			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*m_sport_record.DayIndex+ACTIVITY_DATA_START_ADDRESS + 
//																ACTIVITY_DAY_DATA_SIZE*(m_sport_record.ActivityIndex-1) + SPORT_EVENT_BEFORE_BYTE,(uint8_t*)(&m_event_id),sizeof(int64_t));
//			dev_extFlash_read( DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*m_sport_record.DayIndex+ACTIVITY_DATA_START_ADDRESS + 
//																ACTIVITY_DAY_DATA_SIZE*(m_sport_record.ActivityIndex-1) + SPORT_EVENT_BEFORE_BYTE,(uint8_t*)(&m_test_event),sizeof(int64_t));
#endif
			dev_extFlash_disable();
#if defined WATCH_SPORT_EVENT_SCHEDULE
//			m_test_id = m_event_id >> SPORT_EVENT_ID_TO_LAST_BYTE;
//			m_event_id = get_sport_event_id(m_test_event);
//			TASK_SPORT_PRINTF("[task_sport]:*1*****>>ActivityData.sport_event.U_EventSport.bits=%d-%02d-%02d %d:%02d:%02d-endsec:%02d,status=%d,num=%d,event_id=%d,m_test_id=%d\n"
//			,m_ActivityData.sport_event.U_EventSport.bits.Year,m_ActivityData.sport_event.U_EventSport.bits.Month,m_ActivityData.sport_event.U_EventSport.bits.Day
//			,m_ActivityData.sport_event.U_EventSport.bits.Hour,m_ActivityData.sport_event.U_EventSport.bits.Minute,m_ActivityData.sport_event.U_EventSport.bits.Second_Start
//		  ,m_ActivityData.sport_event.U_EventSport.bits.Second_End,m_ActivityData.sport_event.U_EventSport.bits.Event_Status,m_ActivityData.sport_event.U_EventSport.bits.Shedule_Nums
//			,m_event_id,m_test_id);
//			TASK_SPORT_PRINTF("[task_sport]:*2******>>m_test_event.U_EventSport.bits=%d-%02d-%02d %d:%02d:%02d-endsec:%02d,status=%d,num=%d,event_id=%d,m_test_id=%d\n"
//			,m_test_event.U_EventSport.bits.Year,m_test_event.U_EventSport.bits.Month,m_test_event.U_EventSport.bits.Day
//			,m_test_event.U_EventSport.bits.Hour,m_test_event.U_EventSport.bits.Minute,m_test_event.U_EventSport.bits.Second_Start
//		  ,m_test_event.U_EventSport.bits.Second_End,m_test_event.U_EventSport.bits.Event_Status,m_test_event.U_EventSport.bits.Shedule_Nums
//			,m_event_id,m_test_id);
#endif
			TASK_SPORT_PRINTF("[task_sport]:save_add=0x%X,Activity_Type=0x%X,ActivityIndex=%d\n"
			,DAY_MAIN_DATA_START_ADDRESS+DAY_MAIN_DATA_SIZE*m_sport_record.DayIndex+ACTIVITY_DATA_START_ADDRESS + 
																ACTIVITY_DAY_DATA_SIZE*(m_sport_record.ActivityIndex-1)
			,ActivityData.Activity_Type,m_sport_record.ActivityIndex);
		}

		if((ActivityData.Activity_Type >= ACT_TRIATHLON_SWIM) && (ActivityData.Activity_Type <= ACT_TRIATHLON_RUN))
		{
			//如果是铁人三项运动，得到每项距离和时长
			gui_sport_triathlon_data_get();
		}
        //计算详情轨迹
        SaveGpsToXY();
	}
	else
	{
		if((ActivityData.Activity_Type > ACT_TRIATHLON_SWIM) && (ActivityData.Activity_Type <= ACT_TRIATHLON_RUN))
		{
			if(gui_sport_triathlon_status_get() != 1)
			{
				//如果是铁人三项运动，且放弃运动，还需删除前面几项运动
				sport_triathlon_cancel();
				gui_sport_triathlon_status_set(0);
			}
		}
		//放弃训练计划运动后 置未完成态
		Set_IsCompleteTodayTrainPlan(0);
	}
	//存储当前使用的地址空间
	Store_SetData();
}

//关闭运动
void CloseSport(void)
{
	//设为分中断

	am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_MIN); 
    	
	//关闭计步
	if(g_SportSensorSW & STEP_SW_MASK)
	{
	
	}
	
	//关闭GPS
	if(g_SportSensorSW & GPS_SW_MASK)
	{
        CloseGPSTask();
	}
	 
	//关闭心率
	if(g_SportSensorSW & HEARTRATE_SW_MASK)
	{
        task_hrt_close();
        
        
	}
	
	//关闭气压
	if(g_SportSensorSW & HEIGHT_SW_MASK)
	{

	}
	
	//关闭指北针
	if(g_SportSensorSW & COMPASS_SW_MASK)
	{
#if !defined WATCH_SPORT_NO_COMPASS 
        drv_lis3mdl_standby();
#endif
	}

    //关闭陀螺仪
	if(g_SportSensorSW & GYRO_SW_MASK)
	{

	}

	gps_located = 1; //gps定位成功标记

    //结束游泳时的处理
    if ((ActivityData.Activity_Type == ACT_SWIMMING)||(ActivityData.Activity_Type == ACT_TRIATHLON_SWIM))
    {
        Set_SwimSta(0);
       	#if defined (WATCH_STEP_ALGO)
		#else
		drv_lsm6dsl_enable_pedometer();
		drv_lsm6dsl_acc_fifo_uninit();
		#endif
		    drv_lsm6dsl_fifo_uninit();
			  drv_lsm6dsl_gyro_powerdown();
			  drv_lsm6dsl_acc_init();
			  drv_lsm6dsl_acc_fifo_init();
        //SwimAlgorithmEnd();
			  SwimmingAlgorithmsEnd();
				ALGO_HF_SWIM_PRINTF("------------ Close Sport--------------\n");
    }

     if ((ActivityData.Activity_Type == ACT_CYCLING)||(ActivityData.Activity_Type == ACT_TRIATHLON_CYCLING))
     {
		   set_cyclingsta(0);
         //打开传感器计步算法
    	 #if defined (WATCH_STEP_ALGO)
		//	   drv_lsm6dsl_fifo_uninit();
		//	   drv_lsm6dsl_gyro_powerdown();
		//	   drv_lsm6dsl_acc_init();
		//	   drv_lsm6dsl_acc_fifo_init();
		   #else
		     drv_lsm6dsl_enable_pedometer();
		     drv_lsm6dsl_acc_fifo_uninit();
		  #endif			  
    }
	
	//保存最后的暂停时间
	Set_PauseSta(false);

    InitLoadTrack();

	dottrack_draw_time = 0;
	dottrack_out_time = 0;	//航迹打点到上限显示次数
	dottrack_nogps_time = 0;//航迹打点gps没有定位显示次数
	//保存运动数据
    SaveSport(Get_SaveSta());
	
	if(ActivityData.Activity_Type == ACT_TRAIN_PLAN_RUN)
	{
		Is_Need_Hint_Flag = 0;
	}
	
	//清除云迹导航和运动轨迹的标志
	ENTER_NAVIGATION = 0;
	ENTER_MOTION_TRAIL = 0;

	step_pattern_flag = 0;
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	//云迹导航 状态清零
	set_share_track_back_discreen(GOTO_SHARE_TRACK_BY_NONE);
	g_tool_or_sport_goto_screen = DISPLAY_SCREEN_HOME;
	g_sport_event_is_restart_status = 0;
#endif
#if defined WATCH_SPORT_EVENT_SCHEDULE
	g_start_search_cloud_status = 0;//云迹导航搜索方式0 无运动限制 1运动限制搜索
#endif
}

//读取踏频配件数据
uint32_t sport_cycling_cadence(void)
{
	float crankCadence = 0;
	static float crankCadence_old = 0;
	
	extern _ble_accessory g_ble_accessory;
	
	if((g_ble_accessory.data.cycle_data_valid) && (g_ble_accessory.data.cycle_is_crank_revolution_data_present))
	{	
		//数据有效
		if((m_LastCrankRevolutions == 0) && (m_LastCrankEventTime == 0))
		{
			m_LastCrankRevolutions = g_ble_accessory.data.cycle_cumulative_crank_revolutions;
			m_LastCrankEventTime = g_ble_accessory.data.cycle_last_crank_event_time;
		}
		else if(m_LastCrankEventTime != g_ble_accessory.data.cycle_last_crank_event_time)
		{
			float timeDifference = 0;
			if (g_ble_accessory.data.cycle_last_crank_event_time < m_LastCrankEventTime)
			{
				timeDifference = (65535 + g_ble_accessory.data.cycle_last_crank_event_time - m_LastCrankEventTime) / 1024.0f;
			}
			else
			{
				timeDifference = (g_ble_accessory.data.cycle_last_crank_event_time - m_LastCrankEventTime) / 1024.0f;
			}

			crankCadence = (g_ble_accessory.data.cycle_cumulative_crank_revolutions - m_LastCrankRevolutions) * 60.0f / timeDifference;

//防止数据丢包时，踏频数据显示0
			if(crankCadence != 0){
				crankCadence_old = crankCadence;
			}else{
				crankCadence = crankCadence_old;
				crankCadence_old = 0;
			}
			
			m_LastCrankRevolutions = g_ble_accessory.data.cycle_cumulative_crank_revolutions;
			m_LastCrankEventTime = g_ble_accessory.data.cycle_last_crank_event_time;
		}
	}
	
	return ((uint32_t)crankCadence);
}

static void task_sport_press(void)
{
#if !defined WATCH_SPORT_NO_PRESS	
	int32_t temp_press;
	
	//上升下降高度
	Get_Sport_Altitude(&ActivityData.ActivityDetails.CommonDetail.RiseHeight,&ActivityData.ActivityDetails.CommonDetail.DropHeight);				
	//5分钟保存气压高度
	if( ( ActivityData.Act_Stop_Time.Minute % 5 ) == 0 )
	{
		temp_press = sport_press*100;
		#ifdef COD 
		#else
		Store_PressureData(&temp_press,&sport_alt);
		#endif
	}
#endif
}
#if defined WATCH_COM_SPORT
static void task_climbing_press(void)
{
#if !defined WATCH_SPORT_NO_PRESS	
	int32_t temp_press;
	
	
	drv_ms5837_data_get(&sport_press,&sport_alt,&sport_temp);
	TASK_SPORT_PRINTF("[task_sport]:task_climbing_press %d,%d,%d\r\n",(int32_t)sport_alt,(uint32_t)sport_press,(uint32_t)sport_temp);
	//一分钟计算一次上升下降高度
	if (ActivityData.Act_Stop_Time.Second  == 0)
	{
		CalculateAltitude(sport_press,&ActivityData.ActivityDetails.CommonDetail.RiseHeight,&ActivityData.ActivityDetails.CommonDetail.DropHeight);
	}
	
	//5分钟保存气压高度
	if( (( ActivityData.Act_Stop_Time.Minute % 5 ) == 0 ) && (ActivityData.Act_Stop_Time.Second  == 0))
	{
		temp_press = sport_press*100;
		#ifdef COD 
		#else
		Store_PressureData(&temp_press,&sport_alt);
		#endif
	}
#endif
}
#endif
void TaskSport(void* pvParameter)
{
    (void)pvParameter;

    uint32_t act_time;
    uint8_t hdrvalue = 0;	//心率值
    uint8_t pauseswitch;	//从暂停到运动状态标记
    uint16_t laps; 			//游泳趟数
    uint8_t hdrminute;		//一分钟平均心率
    uint16_t speedminute;	//一分钟平均速度/配速
	uint32_t cadenceminute;	//一分钟平均踏频
    uint32_t tempsteps;
    uint16_t last_circle_pace = 0;	//上圈配速
	uint32_t last_circle_time = 0;	//上圈时间
	uint32_t last_circle_dis = 0;	//上圈距离
	uint32_t last_circle_step = 0;//上一圈步数
	uint32_t last_circle_hdrtotal = 0;	//上一圈心率总值
	uint32_t last_circle_hdrtcount = 0; //上一圈心率次数
	uint16_t last_circle_riseheight = 0;	//上一圈上升高度
	uint16_t last_circle_dropheight = 0; //上一圈下降高度
	int32_t swim_type = 0;

	while(1)
	{
		//ALGO_HF_SWIM_PRINTF("3: %d\n",uxTaskGetStackHighWaterMark(TaskSportHandle));
		if( xSemaphoreTake( ActSemaphore, 2000/portTICK_PERIOD_MS ) == pdTRUE )
		{
			if(TaskSportStatus == false)
			{

				//退出相应运动模式
				CloseSport();
				//退出时清零，防止影响下次运动
				hdrvalue = 0;	//心率值
				last_circle_pace = 0;	//上圈配速
				last_circle_time = 0;	//上圈时间
				last_circle_dis = 0;	//上圈距离
				last_circle_step = 0;//上一圈步数
				last_circle_hdrtotal = 0;	//上一圈心率总值
   				last_circle_hdrtcount = 0; //上一圈心率次数
   				last_circle_riseheight = 0;	//上一圈上升高度
				last_circle_dropheight = 0; //上一圈下降高度
	
				//挂起任务
				vTaskSuspend(NULL);
                continue;
			}
            
			if(PauseSta == false)
            {
							g_SportTimeCount ++;
				//时间+1S
			    ActivityData.Act_Stop_Time.Second++;
			    if( ActivityData.Act_Stop_Time.Second >= 60 )
			    {	
				    ActivityData.Act_Stop_Time.Second = 0;				
				    ActivityData.Act_Stop_Time.Minute++; 
				    if( ActivityData.Act_Stop_Time.Minute >= 60 )
				    {
					    ActivityData.Act_Stop_Time.Minute = 0;
					    ActivityData.Act_Stop_Time.Hour++;
				    }   
			    }
            }
            else 
            {   
                //总暂停时间+1S
                PauseTotolTime++;
                
                //单次暂停时间+1S
                PauseTime.Pause_Stop_Time.Second++;
                if( PauseTime.Pause_Stop_Time.Second >= 60 )
			    {	
				    PauseTime.Pause_Stop_Time.Second = 0;				
				    PauseTime.Pause_Stop_Time.Minute++; 
				    if( PauseTime.Pause_Stop_Time.Minute >= 60 )
				    {
					    PauseTime.Pause_Stop_Time.Minute = 0;
					    PauseTime.Pause_Stop_Time.Hour++;
				    }   
			    }
            }
            
			if(PauseSta)
			{
				//暂停状态
				pauseswitch = 1; 
			}
			else
			{
				//运动状态
				pauseswitch = 0;
			}
#ifdef DEBUG_ERR           
            heartlight = 0;

#endif       
			//航迹打点显示
			if (dottrack_draw_time !=0)
			{
				dottrack_draw_time--;
			}
			if(dottrack_out_time !=0)
			{
				dottrack_out_time--;
			}
			if (dottrack_nogps_time !=0)
			{
				dottrack_nogps_time--;
			}

			//运动时间
			act_time = g_SportTimeCount;
			ActivityData.ActTime = g_SportTimeCount;
#if defined WATCH_SIM_SPORT ||defined WATCH_COM_SPORT
		
			if( (( PauseTime.Pause_Stop_Time.Minute % 5 ) == 0 ) 
				&& ( PauseTime.Pause_Stop_Time.Second == 0) 
				&& (PauseSta != false))
			{
				timer_notify_motor_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
				timer_notify_buzzer_start(1000,500,3,false,NOTIFY_MODE_SPORTS);
			}
#endif 
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
			if(get_share_track_back_discreen() == GOTO_SHARE_TRACK_BY_GPS
					&& is_sport_type_cloud_navigation_valid(ActivityData.Activity_Type)
					&& TRACE_BACK_DATA.is_traceback_flag == 0)
			{//仅云迹导航支持关门时间 循迹返航不支持
				is_sport_has_close_time(get_navigation_index(),ActivityData.Activity_Type);
				//云迹导航运动 有效关门时间提醒 更新关门时间等
				if(g_track_mark_data.is_close_time_valid == 1
					 && g_track_mark_data.track.close_time_index > 0)
				{
					//提醒
					sport_cloud_start_notify();
				}
			}
			
#endif
     
		
			switch(ActivityData.Activity_Type)
			{
				case ACT_RUN:
				case ACT_TRIATHLON_RUN:
				case ACT_TRAIN_PLAN_RUN:
					//步数
					ActivityData.ActivityDetails.RunningDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					#ifdef COD 
					SetValue.run_step = last_run_step + ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
					#endif
					
					//将数据送入到算法中
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthRun,ActivityData.ActivityDetails.RunningDetail.Steps,ACT_RUN);
				
					//配速
					ActivityData.ActivityDetails.RunningDetail.Pace = SpeedResult();
#if defined WATCH_SIM_SPORT
					//步频
					ActivityData.ActivityDetails.RunningDetail.frequency = get_steps_freq();
#endif 					
					if(PauseSta == false)
					{
						//运动状态
						//距离
						ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();
						
						//暂用平均配速
						ActivityData.ActivityDetails.RunningDetail.Pace = SpeedResult();
												
						//心率                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						#ifdef COD 
						marathon_cal_time();
						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20秒存储一次
						{
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_HeartRateData(&hdrminute,&sport_total_time);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;

							har_total_cod+=hdrminute;
							har_count_toal_cod++;
							ave_hdr_cod = har_total_cod/har_count_toal_cod;
							
						}
						#endif
#if defined WATCH_SIM_SPORT
						ActivityData.ActivityDetails.RunningDetail.AvgHeart = hdrtotal/hdrtcount;
						ave_frequency = CalculateFrequency(ActivityData.ActTime,ActivityData.ActivityDetails.RunningDetail.Steps -steps_pause_t );
#endif 

#ifdef DEBUG_TRAINPLAN
						//For Test 测试训练计划，达到20s后认为训练计划完成,不用请删掉
						if(act_time >= 60)
						{
							Set_IsCompleteTodayTrainPlan(1);
						}
#endif
						//训练计划提醒 训练计划正在进行中且达到目标值时提醒
#if defined WATCH_SIM_SPORT
						if(Get_IsTrainPlanOnGoing() == true)
#else
						if(ActivityData.Activity_Type == ACT_TRAIN_PLAN_RUN)
#endif
						{
#ifdef DEBUG_TRAINPLAN
							//训练计划
							Upload[1].Distance = 9000;//米
							Upload[1].Calories = 7000;//卡
							Upload[1].Hour = act_time;//秒
							Upload[1].RecordIndex = ActivityData.Activity_Index;//索引
#else
							//训练计划
							Upload[1].Distance = ActivityData.ActivityDetails.RunningDetail.Distance/100;//米
							Upload[1].Calories = ActivityData.ActivityDetails.RunningDetail.Calorie;//卡
							Upload[1].Hour = act_time;//秒
							Upload[1].RecordIndex = ActivityData.Activity_Index;//索引
#endif
							

#ifdef DEBUG_TRAINPLAN
							if(Get_IsTrainPlanOnGoing() && Is_TodayTimeTrainPlan() && Get_IsCompleteTodayTrainPlan() == 1
								  && Is_Need_Hint_Flag == 0 && get_trainplan_valid_flag())//仅提醒一次  目前测试用，替换完成目标依据直接依据结果,以达到20s完成作用
#else
							if(Get_IsTrainPlanOnGoing() && Is_TodayTimeTrainPlan() && Get_IsCompleteBothTodayTrainPlan(Download[1].Type) == 1
								  && Is_Need_Hint_Flag == 0 && get_trainplan_valid_flag())//仅提醒一次 正式版本用
#endif
							 {
								 Is_Need_Hint_Flag++;
								 Set_IsCompleteTodayTrainPlan(1);
								 Remind_TrainPlan_Complete();
							 }
						}

						//保存运动数据部分
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{
					   
							//计步存储
							tempsteps = ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&tempsteps);
							#endif
							//距离存储

							Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
							//一分钟平均速度
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.RunningDetail.Distance,ACT_RUN);
                            if((ActivityData.ActivityDetails.RunningDetail.Distance - distancelast) < 1000)
                            {
                                speedminute = 0;
                            }
							#endif
							speedminute = MeanSpeedResult();
							
							Store_SpeedData(&speedminute);
							distancelast = ActivityData.ActivityDetails.RunningDetail.Distance;
							
							//卡路里
							ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);
							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.RunningDetail.Calorie);
							
							//最佳配速，一分钟获取一次，提高精确度
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);
							#ifndef COD
							//一分钟平均心率
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							#endif
						}
					
						//gps存储
						
						if (GetGpsStatus())
						{
							store_gps_nums++;
						
						}
						

						
						//铁人三项暂不需要提醒
						if(ActivityData.Activity_Type != ACT_TRIATHLON_RUN)
						{
							//自动计圈时间 轨迹存储
							Store_AuotCirclle(ActivityData.ActivityDetails.RunningDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwRun);	
                       #if defined WATCH_SIM_SPORT
							if(Get_IsTrainPlanOnGoing() == true)
                       #else
							if(ActivityData.Activity_Type == ACT_TRAIN_PLAN_RUN)
                       #endif
							{
								//训练计划配速提醒
								Remind_Pace_TrainPlan(ActivityData.ActivityDetails.RunningDetail.Pace,ActivityData.ActivityDetails.RunningDetail.Distance,
													Download[1].TargetSpeedLimitUp,Download[1].TargetSpeedLimitDown);
							}
							else
							{
								//心率提醒
								Remind_Heartrate(hdrvalue,SetValue.SportSet.SwRun);
								//目标距离提醒
								Remind_GoalDistance(ActivityData.ActivityDetails.RunningDetail.Distance/100,SetValue.SportSet.DefHintDistanceRun,SetValue.SportSet.SwRun);
								//配速提醒
								Remind_Pace(ActivityData.ActivityDetails.RunningDetail.Pace,SetValue.SportSet.DefHintSpeedRun,SetValue.SportSet.SwRun);
								
								//目标时间提醒
								Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeRun,SetValue.SportSet.SwRun);
								
								//计圈提醒
								if (Remind_CircleDistance(ActivityData.ActivityDetails.RunningDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwRun))
								{
									#ifdef COD 
									last_circle_pace = CalculateSpeed(act_time,0,ActivityData.CircleDistance*100,ACT_RUN);
									#else
									//计算上圈平均配速
									last_circle_pace = CalculateSpeed(act_time - last_circle_time,0,ActivityData.CircleDistance*100,ACT_RUN);
									#endif

									last_circle_data.lap = (ActivityData.ActivityDetails.RunningDetail.Distance/100)/ActivityData.CircleDistance;
									last_circle_data.time = (act_time - last_circle_time);
									last_circle_data.pace= last_circle_pace;
									last_circle_data.heart =(hdrtotal- last_circle_hdrtotal)/(hdrtcount - last_circle_hdrtcount);

									remind_autocircle_display();
									
									last_circle_time = act_time;
									last_circle_dis = ActivityData.ActivityDetails.RunningDetail.Distance;
									last_circle_hdrtotal = hdrtotal;
									last_circle_hdrtcount = hdrtcount;

								}
							}
						}
					}
					#ifdef COD 

					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
						ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
						
						ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
                  
              
					}
					DayMainData.SportEnergy = last_SportEnergy +  ActivityData.ActivityDetails.CommonDetail.Calorie;
					DayMainData.SportDistance =last_SportDistance + ActivityData.ActivityDetails.CommonDetail.Distance ;
					#else
					//	TrackPointData = TrackPointsResult();
					
					#endif

					gps_data_get_sm.output_data_sum = 0;//重置数据
					break;
				case ACT_WALK:
					//步数
					ActivityData.ActivityDetails.WalkingDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					
					//将数据送入到算法中
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthWalking,ActivityData.ActivityDetails.WalkingDetail.Steps,ACT_WALK);
				
					//配速
					ActivityData.ActivityDetails.WalkingDetail.Speed = SpeedResult();
				
					//步频
					ActivityData.ActivityDetails.WalkingDetail.frequency = get_steps_freq();
					
					if(PauseSta == false)
					{
						//运动状态
						//距离
						ActivityData.ActivityDetails.WalkingDetail.Distance = DistanceResult();
												
						//心率                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
#if defined WATCH_SIM_SPORT
						ActivityData.ActivityDetails.WalkingDetail.AvgHeart = hdrtotal/hdrtcount;
						ave_frequency = CalculateFrequency(ActivityData.ActTime,ActivityData.ActivityDetails.WalkingDetail.Steps -steps_pause_t );
#endif 

						//保存运动数据部分
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{
							//计步存储
							tempsteps = ActivityData.ActivityDetails.WalkingDetail.Steps - steps_pause_t;
							Store_StepData(&tempsteps);
							
							//距离存储
							Store_DistanceData(&ActivityData.ActivityDetails.WalkingDetail.Distance);
							
							//一分钟平均速度
							#if 0 
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.WalkingDetail.Distance,ACT_WALK);
                            if((ActivityData.ActivityDetails.WalkingDetail.Distance - distancelast) < 1000)
                            {
                                speedminute = 0;
                            }
							#endif 
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);
							distancelast = ActivityData.ActivityDetails.WalkingDetail.Distance;
							
							//卡路里
							ActivityData.ActivityDetails.WalkingDetail.Calorie = CalculateWalkEnergy(ActivityData.ActivityDetails.WalkingDetail.Distance/100,SetValue.AppSet.Weight);

							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.WalkingDetail.Calorie);
							
							//最佳配速，一分钟获取一次，提高精确度
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.WalkingDetail.OptimumSpeed);
							
							//一分钟平均心率
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
					
						//gps存储
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
						
						//自动计圈时间 轨迹存储
						Store_AuotCirclle(ActivityData.ActivityDetails.WalkingDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwWalking);	
						
//						//心率提醒
//						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwWalking);
						//卡路里提醒
						Remind_GoalKcal(ActivityData.ActivityDetails.WalkingDetail.Calorie/1000,SetValue.SportSet.DefWalkingFatBurnGoal,SetValue.SportSet.SwWalking);
						//目标时间提醒
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeWalking,SetValue.SportSet.SwWalking);

						//计圈提醒
						if(Remind_CircleDistance(ActivityData.ActivityDetails.WalkingDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwWalking))
						{
						#if defined WATCH_SIM_SPORT
							//上一圈时间
							gui_sport_walk_last_time_set((act_time -last_circle_time));
						#endif
							
							last_circle_data.lap = (ActivityData.ActivityDetails.RunningDetail.Distance/100)/ActivityData.CircleDistance;
							last_circle_data.time = (act_time - last_circle_time);
							last_circle_data.frequency= CalculateFrequency((act_time - last_circle_time),(ActivityData.ActivityDetails.RunningDetail.Steps  - steps_pause_t - last_circle_step));
							last_circle_data.heart =(hdrtotal- last_circle_hdrtotal)/(hdrtcount - last_circle_hdrtcount);

							remind_autocircle_display();
							
						    last_circle_step = ActivityData.ActivityDetails.RunningDetail.Steps  - steps_pause_t;
							last_circle_time = act_time;
							last_circle_hdrtotal = hdrtotal;
							last_circle_hdrtcount = hdrtcount;
							

							
											
						}
#if defined WATCH_SIM_SPORT						
						if ((SetValue.SportSet.SwWalking & SW_CIRCLEDISTANCE_MASK) == 0)
						{
							//无自动计圈时，上一圈时间显示总时间
							gui_sport_walk_last_time_set(act_time);
						}

#endif						


					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
												ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
												
												ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
											
                 
					}
					#else
							//TrackPointData = TrackPointsResult();
					
					#endif
					
					gps_data_get_sm.output_data_sum = 0;
					break;
				case ACT_MARATHON:
					//步数
					ActivityData.ActivityDetails.RunningDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					
					//将数据送入到算法中
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthMarathon,ActivityData.ActivityDetails.RunningDetail.Steps,ACT_MARATHON);
				
					//配速
					ActivityData.ActivityDetails.RunningDetail.Pace = SpeedResult();
					
					//步频
				//	ActivityData.ActivityDetails.RunningDetail.frequency = get_steps_freq();
					
					if(PauseSta == false)
					{
						//运动状态
						//距离
						ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();
												
						//心率                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
#if defined WATCH_SIM_SPORT
						//平均心率
						ActivityData.ActivityDetails.WalkingDetail.AvgHeart = hdrtotal/hdrtcount;
						ave_frequency = CalculateFrequency(ActivityData.ActTime,ActivityData.ActivityDetails.RunningDetail.Steps -steps_pause_t );

#endif

						//保存运动数据部分
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{
							//计步存储
							tempsteps = ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
							Store_StepData(&tempsteps);
							//距离存储

							Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
							//一分钟平均速度
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.RunningDetail.Distance,ACT_MARATHON);
                            if((ActivityData.ActivityDetails.RunningDetail.Distance - distancelast) < 1000)
                            {
                                speedminute = 0;
                            }
							#endif 
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);
							distancelast = ActivityData.ActivityDetails.RunningDetail.Distance;
							
							//卡路里
							ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);
							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.RunningDetail.Calorie);
							
							//最佳配速，一分钟获取一次，提高精确度
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);
							
							//一分钟平均心率
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
					
						//gps存储
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
						
						//自动计圈时间 轨迹存储
						Store_AuotCirclle(ActivityData.ActivityDetails.RunningDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwMarathon);	
						
						//心率提醒
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwMarathon);
						//目标距离提醒
						Remind_GoalDistance(ActivityData.ActivityDetails.RunningDetail.Distance/100,SetValue.SportSet.DefHintDistanceMarathon,SetValue.SportSet.SwMarathon);
						//配速提醒
						Remind_Pace(ActivityData.ActivityDetails.RunningDetail.Pace,SetValue.SportSet.DefHintSpeedMarathon,SetValue.SportSet.SwMarathon);
						//计圈提醒
						if(Remind_CircleDistance(ActivityData.ActivityDetails.RunningDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwMarathon))
						{
							//计算上圈平均配速
							last_circle_pace = CalculateSpeed(act_time - last_circle_time,0,ActivityData.CircleDistance*100,ACT_MARATHON);
#if !defined WATCH_COM_SPORT
							gui_sport_marathon_last_time_set((act_time - last_circle_time));
#endif
							gui_sport_marathon_last_pace_set(last_circle_pace);
							
							last_circle_data.lap = (ActivityData.ActivityDetails.RunningDetail.Distance/100)/ActivityData.CircleDistance;
							last_circle_data.time = (act_time - last_circle_time);
							last_circle_data.pace= last_circle_pace;
							last_circle_data.heart =(hdrtotal- last_circle_hdrtotal)/(hdrtcount - last_circle_hdrtcount);

							remind_autocircle_display();
							
						    last_circle_step = ActivityData.ActivityDetails.RunningDetail.Steps;
							last_circle_time = act_time;
							last_circle_dis = ActivityData.ActivityDetails.RunningDetail.Distance;
							last_circle_hdrtotal = hdrtotal;
							last_circle_hdrtcount = hdrtcount;
							
						}
						
#if defined WATCH_SIM_SPORT
						if ((SetValue.SportSet.SwMarathon & SW_CIRCLEDISTANCE_MASK) == 0)
						{
							//无自动计圈时，上一圈时间显示总时间
							gui_sport_marathon_last_time_set(act_time);
						}				
#endif
					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
												ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
												
												ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
												
               
					}
					#else
							//TrackPointData = TrackPointsResult();
					
					#endif
					
					gps_data_get_sm.output_data_sum = 0;
					break;
				case ACT_SWIMMING:
					if(PauseSta == false)
					{
						//运动状态
						//划数
						//ActivityData.ActivityDetails.SwimmingDetail.Strokes = SwimDataWaveCountResult();
						#ifdef COD
						algo_swim_strokes = SwimResultsOutputPort1_StrokeCount();
						#else
						ActivityData.ActivityDetails.SwimmingDetail.Strokes = SwimResultsOutputPort1_StrokeCount();
						#endif
                        //心率 
						if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_CONNECT)
						{
						    hdrvalue = Get_Sport_Heart();
						    CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
							
#if defined WATCH_SIM_SPORT
							//平均心率
							ActivityData.ActivityDetails.SwimmingDetail.AvgHeart = hdrtotal/hdrtcount;
#endif 
						}
							
#ifndef COD
						//平均划频
						ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency_swimming(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
#endif 										
						//获取趟数
						//laps = SwimDataCircleResult();
						laps = SwimResultsOutputPort2_LengthCount();
#ifdef COD 
						
						if (laps > 0xFF)
						{
							laps = laps >> 8;
						}
						swim_type = SwimResultsOutputPort3_SwimmingStyle();
						if (last_strokes !=algo_swim_strokes)
						{
							if (swim_type > UnknownStyle)
							{
								swim_type = UnknownStyle;
							}
							swim_store[swim_type] +=1;			
							
							if (swim_type == Freestyle|| swim_type==Backstroke)
							{
								Stores_Stroke(swim_type,laps);
								Stores_Stroke(swim_type,laps);
								//ActivityData.ActivityDetails.SwimmingDetail.Strokes +=1;
							}
							else
							{
								Stores_Stroke(swim_type,laps);
							}
						last_strokes = algo_swim_strokes;
						}
						//趟数
						ActivityData.ActivityDetails.SwimmingDetail.laps = laps;
						//划数
						ActivityData.ActivityDetails.SwimmingDetail.Strokes = 2*swim_store[Freestyle] + 2*swim_store[Backstroke]+ swim_store[Breaststroke]+ swim_store[ButterflyStroke]+ swim_store[UnknownStyle];
						//平均划频
						ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency_swimming(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);

						#if 0 //没有心率存储
						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20秒存储一次
						{
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
						#endif
#endif
//						if (laps==0) {Last_GPS_Lat}					
						if(laps > Last_laps)
						{
							//距离
							ActivityData.ActivityDetails.SwimmingDetail.Distance = SetValue.AppGet.LengthSwimmingPool *100 *laps;

							//平均泳速
							ActivityData.ActivityDetails.SwimmingDetail.Speed = CalculateSpeed(act_time,0,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);

							//平均swolf
							#ifdef COD
							//cod 上一趟swolf
							ActivityData.ActivityDetails.SwimmingDetail.Swolf = CalculateSwolf(ActivityData.ActivityDetails.SwimmingDetail.Strokes - last_lap_stroke,1,act_time -last_lap_time);
							last_lap_stroke = ActivityData.ActivityDetails.SwimmingDetail.Strokes;
							last_lap_time = act_time;
							#else
							ActivityData.ActivityDetails.SwimmingDetail.Swolf = CalculateSwolf(ActivityData.ActivityDetails.SwimmingDetail.Strokes,laps,act_time);
							#endif
							//卡路里
							ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex);

//							//平均划频
//							ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);

							#ifndef COD
							//划数存储
							Store_StepData(&ActivityData.ActivityDetails.SwimmingDetail.Strokes);
							#endif

							//距离存储
							Store_DistanceData(&ActivityData.ActivityDetails.SwimmingDetail.Distance);

							//卡路里存储
							Store_EnergyData(& ActivityData.ActivityDetails.SwimmingDetail.Calorie);
							

							//计圈时间
							
#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
							Store_CircleTimeData(act_time ,SetValue.SportSet.SwSwimmg);
#endif
							
							//一趟平均速度
							speedminute = CalculateSpeed((act_time - Last_time),distancelast,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);
							distancelast = ActivityData.ActivityDetails.SwimmingDetail.Distance;
							Store_SpeedData(&speedminute);																											
#if defined WATCH_SIM_SPORT
							//趟数
							gui_sport_swim_laps_set(laps);
							//平均划数
							gui_sport_swim_ave_stroke_set(ActivityData.ActivityDetails.SwimmingDetail.Strokes/laps);
#endif 
							#ifndef COD
							//一趟平均心率													
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount - hdrctlast);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							Store_HeartRateData(&hdrminute);
							#endif
							
							Last_laps = laps;
							Last_time = act_time;
						}
						#ifdef COD
						DayMainData.SportEnergy = last_SportEnergy +  ActivityData.ActivityDetails.CommonDetail.Calorie;
						#endif
						
                        //心率告警提醒
						if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_CONNECT)
					    {
						    Remind_Heartrate(hdrvalue,SetValue.SportSet.SwSwimmg);
                        }

						//目标距离提醒
						Remind_GoalDistance(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,SetValue.SportSet.DefHintDistanceSwimmg,SetValue.SportSet.SwSwimmg);
						
						//目标时间提醒
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeSwimming,SetValue.SportSet.SwSwimmg);
						//目标圈数提醒
						Remind_GoalCircle(laps,SetValue.SportSet.DefHintCircleSwimming,SetValue.SportSet.SwSwimmg);
					}
					break;
				case ACT_INDOORRUN:
					//步数
					ActivityData.ActivityDetails.RunningDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					
					//将数据送入到算法中
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthIndoorRun,ActivityData.ActivityDetails.RunningDetail.Steps,ACT_INDOORRUN);
				
					//配速
					ActivityData.ActivityDetails.RunningDetail.Pace = SpeedResult();
				
					//步频
				//	ActivityData.ActivityDetails.RunningDetail.frequency = get_steps_freq();
					
					if(PauseSta == false)
					{
						//运动状态
						//距离
						ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();
												
						//心率                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						
#if defined WATCH_SIM_SPORT
						//平均心率
						ActivityData.ActivityDetails.RunningDetail.AvgHeart = hdrtotal/hdrtcount;
						ave_frequency = CalculateFrequency(ActivityData.ActTime,ActivityData.ActivityDetails.RunningDetail.Steps -steps_pause_t );
#endif

						//保存运动数据部分
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{
							//计步存储
							tempsteps = ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
							Store_StepData(&tempsteps);
							
							//距离存储
							Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
							
							//一分钟平均速度
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.RunningDetail.Distance,ACT_INDOORRUN);
                            if((ActivityData.ActivityDetails.RunningDetail.Distance - distancelast) < 1000)
                            {
                                speedminute = 0;
                            }
							#endif
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);
							distancelast = ActivityData.ActivityDetails.RunningDetail.Distance;
							
							//卡路里
							ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);
							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.RunningDetail.Calorie);
							
							//最佳配速，一分钟获取一次，提高精确度
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);
							
							//一分钟平均心率
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
						
						//心率提醒
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwIndoorRun);
					}
					break;
//Jason: Adding for handling cross-country running mode data 
				case ACT_CROSSCOUNTRY:
					//步数
					ActivityData.ActivityDetails.CrosscountryDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					#ifdef COD 
					SetValue.run_step = last_run_step + ActivityData.ActivityDetails.CrosscountryDetail.Steps -steps_pause_t;
					#endif
				
					//将数据送入到算法中
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthCountryRace,ActivityData.ActivityDetails.CrosscountryDetail.Steps,ACT_CROSSCOUNTRY);
					
					//配速
					ActivityData.ActivityDetails.CrosscountryDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//运动状态
						//static float sport_press,sport_alt,sport_temp;//气压，高度,温度
						
						//距离
						ActivityData.ActivityDetails.CrosscountryDetail.Distance = DistanceResult();					
						
						//心率                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						#ifdef COD 
						marathon_cal_time();
						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20秒存储一次
						{
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_HeartRateData(&hdrminute,&sport_total_time);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							har_total_cod+=hdrminute;
							har_count_toal_cod++;
							ave_hdr_cod = har_total_cod/har_count_toal_cod;
						}
						#endif
						//保存数据部分
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{  
							//计步存储
							tempsteps = ActivityData.ActivityDetails.CrosscountryDetail.Steps - steps_pause_t;
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&tempsteps);
							#endif

							//距离存储
							Store_DistanceData(&ActivityData.ActivityDetails.CrosscountryDetail.Distance);

							//卡路里计算
							ActivityData.ActivityDetails.CrosscountryDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,act_time,SetValue.AppSet.Weight);
							
							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.RunningDetail.Calorie);
							
							//一分钟平均速度
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.CrosscountryDetail.Distance,ACT_CROSSCOUNTRY);
                            if((ActivityData.ActivityDetails.CrosscountryDetail.Distance - distancelast) < 1000)
                            {
                                speedminute = 0;
                            }
							#endif
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);			
							distancelast = ActivityData.ActivityDetails.CrosscountryDetail.Distance;						
							//最佳配速
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed);
							#ifndef COD
							//一分钟平均心率													
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount - hdrctlast);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							Store_HeartRateData(&hdrminute);
							//气压高度
							task_sport_press();	
							#endif
						}
						
						//gps存储
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
						//自动计圈时间 轨迹存储
						Store_AuotCirclle(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwCountryRace);	
						
						//心率提醒
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwCountryRace);
						//目标距离提醒
						Remind_GoalDistance(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,SetValue.SportSet.DefHintDistanceCountryRace,SetValue.SportSet.SwCountryRace);
						//配速提醒
						Remind_Pace(ActivityData.ActivityDetails.CrosscountryDetail.Speed,SetValue.SportSet.DefHintSpeedCountryRace,SetValue.SportSet.SwCountryRace);

						//目标时间提醒
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeCountryRace,SetValue.SportSet.SwCountryRace);
						
						//计圈提醒
						if (Remind_CircleDistance(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwCountryRace))
						{

							#ifdef COD 
							last_circle_pace = CalculateSpeed(act_time,0,ActivityData.CircleDistance*100,ACT_CROSSCOUNTRY);
							#else
							//计算上圈平均配速
							last_circle_pace = CalculateSpeed(act_time - last_circle_time,0,ActivityData.CircleDistance*100,ACT_CROSSCOUNTRY);
							#endif
						
							last_circle_data.lap = (ActivityData.ActivityDetails.CrosscountryDetail.Distance/100)/ActivityData.CircleDistance;
							last_circle_data.time = (act_time - last_circle_time);
							last_circle_data.pace= last_circle_pace;
							last_circle_data.heart =(hdrtotal- last_circle_hdrtotal)/(hdrtcount - last_circle_hdrtcount);
						
							remind_autocircle_display();
															
							last_circle_time = act_time;
							last_circle_dis = ActivityData.ActivityDetails.CrosscountryDetail.Distance;
							last_circle_hdrtotal = hdrtotal;
							last_circle_hdrtcount = hdrtcount;

						}
					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
					cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
						ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
						
						ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
						
                
					}
					DayMainData.SportEnergy = last_SportEnergy +  ActivityData.ActivityDetails.CommonDetail.Calorie;
					DayMainData.SportDistance =last_SportDistance + ActivityData.ActivityDetails.CommonDetail.Distance ;
					#else
				
					#endif
					
					gps_data_get_sm.output_data_sum = 0;
					break;
//Jason's code end in here
				case ACT_HIKING:
					//步数
					ActivityData.ActivityDetails.CrosscountryDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					#ifdef COD 
					SetValue.walk_step = last_walk_step + ActivityData.ActivityDetails.CrosscountryDetail.Steps -steps_pause_t;
					#endif
					//将数据送入到算法中
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthHike,ActivityData.ActivityDetails.CrosscountryDetail.Steps,ACT_HIKING);
					
					//配速
					ActivityData.ActivityDetails.CrosscountryDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//运动状态
						//距离
						ActivityData.ActivityDetails.CrosscountryDetail.Distance = DistanceResult();
					
						
						//心率                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						#ifdef COD 
						marathon_cal_time();

						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20秒存储一次
						{
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_HeartRateData(&hdrminute,&sport_total_time);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							har_total_cod+=hdrminute;
							har_count_toal_cod++;
							ave_hdr_cod = har_total_cod/har_count_toal_cod;
						}
						#endif
						//保存运动数据部分
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{                
							//计步存储
							tempsteps = ActivityData.ActivityDetails.CrosscountryDetail.Steps - steps_pause_t;
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&tempsteps);
							#endif
							//距离存储
							Store_DistanceData(&ActivityData.ActivityDetails.CrosscountryDetail.Distance);
							//一分钟平均速度
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.CrosscountryDetail.Distance,ACT_HIKING);
							if((ActivityData.ActivityDetails.CrosscountryDetail.Distance - distancelast) < 1000)
							{
									speedminute = 0;
							}
							#endif 
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);
							distancelast = ActivityData.ActivityDetails.CrosscountryDetail.Distance;
							
							//卡路里
							ActivityData.ActivityDetails.CrosscountryDetail.Calorie = CalculateWalkEnergy(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,SetValue.AppSet.Weight);
							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.CrosscountryDetail.Calorie);
							#ifndef COD 
							//一分钟平均心率
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							//气压高度
							task_sport_press();	
							#endif
							//最佳配速
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed);							
							
							
						}
						//gps存储
						if (GetGpsStatus())
						{
							store_gps_nums++;
						
						}
						//自动计圈时间 轨迹存储
						Store_AuotCirclle(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwHike);	
					 
						//心率提醒
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwHike);
						
						#ifndef COD 
						//卡路里提醒
						Remind_GoalKcal(ActivityData.ActivityDetails.CrosscountryDetail.Calorie/1000,SetValue.SportSet.DefHikeFatBurnGoal,SetValue.SportSet.SwHike);
						#endif

						//目标距离提醒
						Remind_GoalDistance(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,SetValue.SportSet.DefHintDistanceHike,SetValue.SportSet.SwHike);

						//配速提醒
						Remind_Pace(ActivityData.ActivityDetails.CrosscountryDetail.Speed,SetValue.SportSet.DefHintSpeedHike,SetValue.SportSet.SwHike);

						//目标时间提醒
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeHike,SetValue.SportSet.SwHike);
					
						//计圈提醒
						if (Remind_CircleDistance(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwHike))
						{
							#ifdef COD 
							last_circle_pace = CalculateSpeed(act_time,0,ActivityData.CircleDistance*100,ACT_HIKING);
							#else
							//计算上圈平均配速
							last_circle_pace = CalculateSpeed(act_time - last_circle_time,0,ActivityData.CircleDistance*100,ACT_HIKING);
							#endif
						
							last_circle_data.lap = (ActivityData.ActivityDetails.CrosscountryDetail.Distance/100)/ActivityData.CircleDistance;
							last_circle_data.time = (act_time - last_circle_time);
							last_circle_data.pace= last_circle_pace;
							last_circle_data.heart =(hdrtotal- last_circle_hdrtotal)/(hdrtcount - last_circle_hdrtcount);
						
							remind_autocircle_display();
															
							last_circle_time = act_time;
							last_circle_dis = ActivityData.ActivityDetails.CrosscountryDetail.Distance;
							last_circle_hdrtotal = hdrtotal;
							last_circle_hdrtcount = hdrtcount;

						}					
					}
					#ifdef COD 
					
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
						ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
						
						ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
						
              
					}
					DayMainData.SportEnergy = last_SportEnergy +  ActivityData.ActivityDetails.CommonDetail.Calorie;
					DayMainData.SportDistance =last_SportDistance + ActivityData.ActivityDetails.CommonDetail.Distance ;
					#else
							//TrackPointData = TrackPointsResult();
					
					#endif
					gps_data_get_sm.output_data_sum = 0;
					break;
				case ACT_CLIMB:
					//步数
					ActivityData.ActivityDetails.ClimbingDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
#if defined WATCH_COM_SPORT
					#ifndef COD
			  		if(PauseSta == false && (ActivityData.Act_Stop_Time.Second %5) == 0) 
					{
					
							//气压高度
							task_climbing_press();
							
					}
					#endif
#else
				  if(PauseSta == false && ActivityData.Act_Stop_Time.Second  == 0) 
					{
							//气压高度
							task_sport_press();	
					}
#endif				  
					//将数据送入到算法中
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthClimbing,ActivityData.ActivityDetails.ClimbingDetail.Steps,ACT_CLIMB);
					
					//速率
					ActivityData.ActivityDetails.ClimbingDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//运动状态
						//距离
						ActivityData.ActivityDetails.ClimbingDetail.Distance = DistanceResult();
						
						//心率                
						hdrvalue = Get_Sport_Heart();
						#ifdef COD 
						CalculateHeartrateMax(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						marathon_cal_time();

						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20秒存储一次
						{
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_HeartRateData(&hdrminute,&sport_total_time);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							har_total_cod+=hdrminute;
							har_count_toal_cod++;
							ave_hdr_cod = har_total_cod/har_count_toal_cod;
						}
						#else 
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						#endif

						//最大心率记录
						if(hdrvalue > ActivityData.ActivityDetails.ClimbingDetail.MaxHeart)
						{							
							ActivityData.ActivityDetails.ClimbingDetail.MaxHeart = hdrvalue;
						}

						//保存运动数据部分
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{
							//计步存储
							tempsteps = ActivityData.ActivityDetails.ClimbingDetail.Steps - steps_pause_t;
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&tempsteps);
							#endif
							//距离存储
							Store_DistanceData(&ActivityData.ActivityDetails.ClimbingDetail.Distance);
							#ifndef COD 
							//一分钟平均心率
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							ActivityData.ActivityDetails.ClimbingDetail.AvgHeart = hdrminute;
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							#else
							//cod 最佳配速
							speedminute =  CalculateSpeed(60,last_distance,ActivityData.ActivityDetails.ClimbingDetail.Distance,ACT_CYCLING);  //速度10*km/h
							last_distance = ActivityData.ActivityDetails.ClimbingDetail.Distance;	
							CalculateOptimumSpeed(speedminute,&ActivityData.ActivityDetails.ClimbingDetail.OptimumSpeed);
							#endif
							//卡路里
							ActivityData.ActivityDetails.ClimbingDetail.Calorie += CalculateMountaineeringEnergy(hdrminute,60,SetValue.AppSet.Weight,SetValue.AppSet.Age,SetValue.AppSet.Sex);
							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.ClimbingDetail.Calorie);
							
							//气压高度
							//task_sport_press();	
							
							//一分钟平均速度
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.ClimbingDetail.DropHeight 
																			+ ActivityData.ActivityDetails.ClimbingDetail.RiseHeight,ACT_CLIMB);
							#endif
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);
							distancelast = ActivityData.ActivityDetails.ClimbingDetail.DropHeight 
												+ ActivityData.ActivityDetails.ClimbingDetail.RiseHeight;
						}
				  
						//gps存储
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
						
						//自动计圈时间，轨迹存储
						Store_AuotCirclle(ActivityData.ActivityDetails.ClimbingDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwClimbing);
						
						//心率提醒
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwClimbing);
						
						//目标距离提醒
						Remind_GoalDistance(ActivityData.ActivityDetails.ClimbingDetail.Distance/100,SetValue.SportSet.DefHintDistanceClimbing,SetValue.SportSet.SwClimbing);

						//目标时间提醒
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeClimbing,SetValue.SportSet.SwClimbing);
							
						//高度提醒
						#ifdef COD 
						Remind_Altitude(sport_altitude,SetValue.SportSet.DefHintClimbingHeight,SetValue.SportSet.SwClimbing);
						#else
						Remind_Altitude(sport_alt,SetValue.SportSet.DefHintClimbingHeight,SetValue.SportSet.SwClimbing);
						#endif

						//计圈提醒
						if (Remind_CircleDistance(ActivityData.ActivityDetails.ClimbingDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwClimbing))
						{
								
							
							last_circle_data.lap = (ActivityData.ActivityDetails.ClimbingDetail.Distance/100)/ActivityData.CircleDistance;
							last_circle_data.time = (act_time - last_circle_time);
							last_circle_data.heart =(hdrtotal- last_circle_hdrtotal)/(hdrtcount - last_circle_hdrtcount);
							if(ActivityData.ActivityDetails.ClimbingDetail.RiseHeight > last_circle_riseheight)
							{
								last_circle_data.riseheight = ActivityData.ActivityDetails.ClimbingDetail.RiseHeight - last_circle_riseheight;
							}
							else
							{
								last_circle_data.riseheight = 0;
							}
							if (ActivityData.ActivityDetails.ClimbingDetail.DropHeight > last_circle_dropheight)
							{
								last_circle_data.dropheight= ActivityData.ActivityDetails.ClimbingDetail.DropHeight - last_circle_dropheight;

							}
							else
							{
								last_circle_data.dropheight = 0;
							}
						
							
							remind_autocircle_display();
															
							last_circle_time = act_time;
							last_circle_hdrtotal = hdrtotal;
							last_circle_hdrtcount = hdrtcount;
							last_circle_riseheight = ActivityData.ActivityDetails.ClimbingDetail.RiseHeight;
							last_circle_dropheight = ActivityData.ActivityDetails.ClimbingDetail.DropHeight;

						}
					}

					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
					    
						//pauseswitch暂停状态(传参),sport_altitude实时海拔 ，sport_RiseHeight累计爬升(传参) ，sport_Risestate爬升状态 ,sport_RiseAlt海拔上升
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
						ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
						
						ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
						CalculateMaxAlt(sport_altitude,&ActivityData.ActivityDetails.ClimbingDetail.MaxAlt);
						
           
					}
					DayMainData.SportEnergy = last_SportEnergy +  ActivityData.ActivityDetails.CommonDetail.Calorie;
					DayMainData.SportDistance =last_SportDistance + ActivityData.ActivityDetails.CommonDetail.Distance ;
					#else				
						//	TrackPointData = TrackPointsResult();
				
					#endif
					
					gps_data_get_sm.output_data_sum = 0;
					break;
				case ACT_CYCLING:
				case ACT_TRIATHLON_CYCLING:
					
				//	GUI_SPORT_PRINTF("test_number:%d\r\n",test_number);
					//将数据送入到算法中
					sport_data_input(pauseswitch,0,ActivityData.ActivityDetails.CyclingDetail.Cadence,ACT_CYCLING);
					
#if 0
					test_number++;
					test_number = test_number%test_num;
#endif
					//速率
					ActivityData.ActivityDetails.CyclingDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//运动状态
						//踏频
						ActivityData.ActivityDetails.CyclingDetail.Cadence = sport_cycling_cadence();
						CalculateCadenceParam(ActivityData.ActivityDetails.CyclingDetail.Cadence,&cadtotal,&cadcount);
						
						//距离
						ActivityData.ActivityDetails.CyclingDetail.Distance = DistanceResult();
						
						//心率                
						hdrvalue = Get_Sport_Heart();
						
						#ifdef COD 
					
						CalculateHeartrateMax(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20秒存储一次
						{
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_HeartRateData(&hdrminute,&sport_total_time);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							har_total_cod+=hdrminute;
							har_count_toal_cod++;
							ave_hdr_cod = har_total_cod/har_count_toal_cod;
						}
						#else
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						#endif
#if defined WATCH_SIM_SPORT
						//平均心率
						ActivityData.ActivityDetails.CyclingDetail.AvgHeart = hdrtotal/hdrtcount;
#endif
						//最大心率记录
						if(hdrvalue > ActivityData.ActivityDetails.CyclingDetail.MaxHeart)
						{						
							ActivityData.ActivityDetails.CyclingDetail.MaxHeart = hdrvalue;
						}

						//保存运动数据部分
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{
							//踏频存储
							cadenceminute = (cadtotal - cadlast)/(cadcount - cadctlast);
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							tempsteps = 0;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&cadenceminute);
							cadlast = cadtotal;
							cadctlast = cadcount;
							#endif
							//距离存储
							Store_DistanceData(&ActivityData.ActivityDetails.CyclingDetail.Distance);
							
							//一分钟平均速度
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.CyclingDetail.Distance,ACT_CYCLING);
						
                            if((ActivityData.ActivityDetails.CyclingDetail.Distance - distancelast) < 1000)
                            {
                                speedminute = 0;
                            }
							#endif
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);
							
							CalculateOptimumSpeed(speedminute, &ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed);

							
							

							
							
							/*卡路里，暂定一分钟计算一次,输入distance距离，单位  米，除以100*/
							ActivityData.ActivityDetails.CyclingDetail.Calorie += CalculateCyclingEnergy(speedminute*100,60,SetValue.AppSet.Weight);
							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.CyclingDetail.Calorie);
							#ifndef COD 
							//一分钟平均心率
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							#endif 
							ActivityData.ActivityDetails.CyclingDetail.AvgHeart = hdrminute;
							//气压高度
							#ifndef COD 
							task_sport_press();
							#endif
						}
					  
						//gps存储
						if (GetGpsStatus())
						{
							store_gps_nums++;
						
						}
						
						//铁人三项暂不需要提醒
						if(ActivityData.Activity_Type != ACT_TRIATHLON_CYCLING)
						{
							//自动计圈时间，轨迹存储
							Store_AuotCirclle(ActivityData.ActivityDetails.CyclingDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwCycling);

							//心率提醒
							Remind_Heartrate(hdrvalue,SetValue.SportSet.SwCycling);
							//目标距离提醒
							Remind_GoalDistance(ActivityData.ActivityDetails.CyclingDetail.Distance/100,SetValue.SportSet.DefHintDistanceCycling,SetValue.SportSet.SwCycling);

							//速度提醒
							Remind_Speed(ActivityData.ActivityDetails.CyclingDetail.Speed,SetValue.SportSet.DefHintSpeedCycling,SetValue.SportSet.SwCycling);

								//目标时间提醒
							Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeCycling,SetValue.SportSet.SwCycling);
								
						//计圈提醒
						if (Remind_CircleDistance(ActivityData.ActivityDetails.CyclingDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwCycling))
						{
							#ifdef COD 
							last_circle_pace = CalculateSpeed(act_time,0,ActivityData.CircleDistance*100,ACT_CYCLING);
							#else
							//计算上圈平均配速
							last_circle_pace = CalculateSpeed(act_time - last_circle_time,0,ActivityData.CircleDistance*100,ACT_CYCLING);
							#endif
							#if defined WATCH_SIM_SPORT
							//上一圈时间
							gui_sport_cycling_last_time_set((act_time -last_circle_time));
							#endif
							last_circle_data.lap = (ActivityData.ActivityDetails.CyclingDetail.Distance/100)/ActivityData.CircleDistance;
							last_circle_data.time = (act_time - last_circle_time);
							last_circle_data.pace= last_circle_pace;
							last_circle_data.heart =(hdrtotal- last_circle_hdrtotal)/(hdrtcount - last_circle_hdrtcount);
						
							remind_autocircle_display();
															
							last_circle_time = act_time;
							last_circle_dis = ActivityData.ActivityDetails.CyclingDetail.Distance;
							last_circle_hdrtotal = hdrtotal;
							last_circle_hdrtcount = hdrtcount;

						}					
							
#if defined WATCH_SIM_SPORT
							if ((SetValue.SportSet.SwCycling & SW_CIRCLEDISTANCE_MASK) == 0)
							{
								//无自动计圈时，上一圈时间显示总时间
								gui_sport_cycling_last_time_set(act_time);
							}
#endif


						}
					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
						ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
						
						ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
						
						CalculateMaxAlt(sport_altitude,&ActivityData.ActivityDetails.CyclingDetail.MaxAlt);

						ActivityData.ActivityDetails.CyclingDetail.RiseAlt = sport_RiseAlt;

						//上坡里程
						/*if (sport_Risestate)
						{
							ActivityData.ActivityDetails.CyclingDetail.UphillDistance += (ActivityData.ActivityDetails.CyclingDetail.Distance -distancelast);
							
						}
						distancelast = ActivityData.ActivityDetails.CyclingDetail.Distance;*/

							ActivityData.ActivityDetails.CyclingDetail.UphillDistance += temp_increase_dis*100.f;//cm
							
            
					}
					DayMainData.SportEnergy = last_SportEnergy +  ActivityData.ActivityDetails.CommonDetail.Calorie;
					#else
						//	TrackPointData = TrackPointsResult();
			
					#endif
					
					gps_data_get_sm.output_data_sum = 0;
					break;
				case ACT_TRIATHLON_SWIM:
					//将数据送入到算法中
					sport_data_input(pauseswitch,0,ActivityData.ActivityDetails.SwimmingDetail.Strokes,ACT_TRIATHLON_SWIM);
					
					//泳速
					ActivityData.ActivityDetails.SwimmingDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//运动状态
						//划数
						//ActivityData.ActivityDetails.SwimmingDetail.Strokes = SwimDataWaveCountResult();
						ActivityData.ActivityDetails.SwimmingDetail.Strokes = SwimResultsOutputPort1_StrokeCount();
						
						//平均划频
						//ActivityData.ActivityDetails.SwimmingDetail.Frequency = CalculateFrequency(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
						ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency_swimming(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
						
						//距离
						ActivityData.ActivityDetails.SwimmingDetail.Distance = DistanceResult();
						
						//心率                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						
						//最大心率记录
						if(hdrvalue > ActivityData.ActivityDetails.SwimmingDetail.MaxHeart)
						{						
							ActivityData.ActivityDetails.SwimmingDetail.MaxHeart = hdrvalue;
						}
					
						if(ActivityData.Act_Stop_Time.Second == 0) //一分钟存储一次
						{
							//划数存储
							Store_StepData(&ActivityData.ActivityDetails.SwimmingDetail.Strokes);
							
							//距离存储
							Store_DistanceData(&ActivityData.ActivityDetails.SwimmingDetail.Distance);
							
							//一分钟平均速度
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_TRIATHLON_SWIM);
                            if((ActivityData.ActivityDetails.SwimmingDetail.Distance - distancelast) < 1000)
                            {
                                speedminute = 0;
                            } 
							#endif
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);
							distancelast = ActivityData.ActivityDetails.SwimmingDetail.Distance;
							
							//卡路里
							ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex);
							//卡路里存储
							Store_EnergyData(&ActivityData.ActivityDetails.SwimmingDetail.Calorie);
							
							//一分钟平均心率
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							ActivityData.ActivityDetails.SwimmingDetail.AvgHeart = hdrminute;
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
						
						//GPS储存
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
												ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
												
												ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
												
              
					}
					#else
						//	TrackPointData = TrackPointsResult();
					
					#endif
					break;
				case ACT_TRIATHLON_FIRST:
				case ACT_TRIATHLON_SECOND:
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2秒存储一次
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
												ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
												
												ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
					}
					#else
					if(PauseSta == false)
					{
						//运动状态
						//保存换项轨迹
						if (GetGpsStatus())
						{
							//TrackPointData = TrackPointsResult();
							Store_GPSData(GetGpsLon(),GetGpsLat());
						}
					}
					#endif
					
					gps_data_get_sm.output_data_sum = 0;
					break;
				default:
					break;
			}
			if (PauseSta == false)
            {
              if(g_SportSensorSW & GPS_SW_MASK)
	            {
#if defined (WATCH_SPORT_NEW_CLOUD_NAVIGATION)
								if (GetGpsStatus())
								{
									transform((double)(GetGpsLon()/GPS_SCALE_DECIMAL),(double)(GetGpsLat()/GPS_SCALE_DECIMAL));
									Last_GPS_Lon =(int32_t )(dLatLng.dlon*100000);//轨迹中gps点保留5位小数点，简化计算量第五位当做为米级
									Last_GPS_Lat = (int32_t )(dLatLng.dlat*100000);
								}
                if (act_time % 15 == 0 || act_time == 1)
#else
					if (act_time % 10 == 0 || act_time == 1)
#endif
								{
										 StoreTrackPoint();
								}
              }   

            } 

            if(g_SportSensorSW & COMPASS_SW_MASK)
	        {
#if !defined WATCH_SPORT_NO_COMPASS 

		      
                  SportAngle = magn_angle_and_cal(0);

#endif
		
	        }
			
			if (SetValue.HRRecoverySet == 1)
			{
				calculate_heartrate_recovery();
					
			}
			if((IsSportMode(ScreenState) == true )|| (ScreenState== DISPLAY_SCREEN_NOTIFY_HEARTRATE))
			{
				set_gps_flicker(1);
				//刷新
				gui_swich_msg();
			}
		}
	}
}



