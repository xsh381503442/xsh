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

extern uint8_t  step_pattern_flag;//�ճ�or�˶�
extern GPSMode m_GPS_data; 	   //GPS����
extern gps_data_out gps_data_output;
extern GpspointStr gps_point;

TaskHandle_t TaskSportHandle = NULL;

volatile ScreenState_t Sport_Index;	//�˶�����������

uint8_t g_SportSensorSW;	        //�ֱ���������
static uint8_t SaveSta;             //����״̬��0������1����
static bool PauseSta;               //��ͣ״̬��0�˶�״̬��1��ͣ״̬

bool TaskSportStatus = false;               //�˶�task״̬��false ����true����

int32_t Last_GPS_Lon;               //�ϴξ�γ��
int32_t Last_GPS_Lat; 
uint16_t SportAngle;

#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
uint8_t is_crosscountry = 0;//ͽ��ԽҰ��ԽҰ��

#endif
#if defined WATCH_SPORT_EVENT_SCHEDULE
uint8_t g_start_search_cloud_status = 0;//�Ƽ�����������ʽ0 ���˶����� 1�˶���������
#endif

	
static uint32_t begin_steps;        //�˶���ʼʱ����
static uint32_t steps_record_f;				//�Ժ����ǰ�Ʋ���¼
static uint32_t steps_record_t;				//�Ժ�����ڼ�Ʋ���¼

static uint32_t steps_pause_f = 0;			//��ͣǰ�Ʋ���¼
static uint32_t steps_pause_t = 0;			//��ͣ�ڼ�Ʋ���¼
uint8_t gps_located = 1; //gps��λ�ɹ����
static uint16_t ave_frequency =0;		//ƽ����Ƶ
static uint32_t hdrtotal,hdrtcount; //�����ۼ�ֵ,�����ۼӴ���
static uint32_t cadtotal,cadcount; 	//̤Ƶ�ۼ�ֵ,̤Ƶ�ۼӴ���

static uint32_t hdrlast,hdrctlast,distancelast; //�ϴ�����ֵ����ֵ,�ϴ����ʼ��㻺��ֵ���ϴξ��뻺��ֵ
static uint32_t cadlast,cadctlast;
#ifdef COD 

static uint32_t har_total_cod,har_count_toal_cod,ave_hdr_cod;
#endif
static uint8_t SwimSta;             //��Ӿ״̬��0����Ӿ��1��Ӿ��uint8_t���Ϳ�����
static uint16_t Last_laps;          //��Ӿ�ϴ�����
static uint32_t Last_time;					//��Ӿ��Ȧʱ��
//static uint32_t Now_time;					//��Ӿ��Ȧʱ��
static uint32_t m_LastCrankRevolutions;		//̤ƵȦ��
static uint32_t m_LastCrankEventTime;		//̤Ƶʱ��

static uint8_t s_cyclingsta = 0;	//����״̬��0�Ƿ������˶���1�������˶���uint8_t���Ϳ�����

static uint32_t PauseTotolTime;     //����ͣʱ�䣬��λ��

extern void drv_lsm6dsl_acc_fifo_init( void);
extern void  drv_lsm6dsl_acc_fifo_uninit(void);
static float sport_press = 0,sport_temp = 0;//��ѹ���߶�,�¶�
float sport_alt = 0;//�߶�,

static uint8_t Is_Need_Hint_Flag = 0;	//ѵ���ƻ��˶���ɺ�����һ��
static TrainRecordStr m_sport_record;	//�˶������ڼ��洢�����¼

static bool IsInSportMode = false;//�Ƿ����˶�ģʽ��
static uint32_t g_SportTimeCount = 0;//�˶���ʱ���ʱ

extern uint8_t del_gps_point_num ;


extern _ble_accessory g_ble_accessory;
static rtc_time_t m_time_sport_ready;//�˶���ʼǰʱ���¼

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
static uint32_t last_strokes = 0;//�ϴεĻ���
static uint32_t algo_swim_strokes = 0;//�㷨�������
static uint32_t last_lap_stroke = 0; //���˵��ܻ���
static uint32_t last_lap_time = 0; //���˵���ʱ��
static uint32_t last_distance;
static uint32_t sport_total_time ;//�˶���ʱ��������ͣ
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
/*��ȡ�˶������½��߶ȣ�һ���ӻ�ȡһ��
����ʾ��:
Get_Sport_Altitude(&ActivityData.ActivityDetails.ClimbingDetail.RiseHeight,&ActivityData.ActivityDetails.ClimbingDetail.DropHeight)
*/
void Get_Sport_Altitude(uint16_t* RiseHeight,uint16_t* DropHeight)
{
#if !defined WATCH_SPORT_NO_PRESS
	//��ѹ�߶�
	drv_ms5837_data_get(&sport_press,&sport_alt,&sport_temp);

	CalculateAltitude(sport_press,RiseHeight,DropHeight);
#endif
}

#if defined WATCH_SPORT_EVENT_SCHEDULE
SportEventStatusStr g_sport_event_status;//��������״̬
SportScheduleCalStr g_sport_event_schedule_detail_cal;//��������״̬����
static uint8_t m_sport_pause_event_status = 0;//0:һ���˶����½��� , 1:�������ν���
void set_sport_pause_event_status(uint8_t status)
{
	m_sport_pause_event_status = status;
}
uint8_t get_sport_pause_event_status(void)
{
	return m_sport_pause_event_status;
}
#endif
//��ȡ�˶�������
uint8_t Get_Sport_Heart(void)
{
    uint8_t hdrvalue;

   	//�Դ�����������
    if  (g_ble_accessory.heartrate.status == ACCESSORY_STATUS_DISCONNECT)
    {
        hdrvalue = drv_getHdrValue();
    }
    else
    {
       if (g_ble_accessory.data.hr_data_valid)//�������
       {
			hdrvalue = g_ble_accessory.data.hr_value;
       }
		else if (g_SportSensorSW & HEARTRATE_SW_MASK)//�Դ�����������
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
//���ʻָ��ʼ���
void calculate_heartrate_recovery(void)
{

	if (sport_end_flag == 1)
	{

		sport_end_time++;
		
		//if (sport_end_time == 1)
		//{
		//	sport_end_hrvalue = get_10s_heartrate(); //��ȡǰ10sƽ�����ʣ�
		//}

		if (sport_end_time == 60)
		{
			sport_min_hrvalue = get_10s_heartrate(); //��ȡǰ10sƽ�����ʣ�
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


//������ÿ�������㷨��������롢�ٶȡ��켣��
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
		//��ɽ�˶������������߶Ⱥ��½��߶�
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

//�����˶�ʱ����������������㷨�����м���
void sport_data_end(uint8_t activity_type)
{
	uint16_t laps;
	uint8_t hdrminute,j;		//һ����ƽ������
    uint16_t speedminute;	//һ����ƽ���ٶ�/����
	uint32_t act_time = g_SportTimeCount;
	

	switch(activity_type)
	{
		case ACT_RUN:
		case ACT_TRIATHLON_RUN:
		case ACT_TRAIN_PLAN_RUN:
		case ACT_INDOORRUN:
			if(activity_type == ACT_TRAIN_PLAN_RUN)
			{
				//ѵ���ƻ�����ϸ���ݹ�Ϊ�ܲ�һ�ౣ��
				activity_type =  ACT_RUN;
			}
		
			//���������뵽�㷨��
			sport_data_input(2,SetValue.AppGet.StepLengthRun,ActivityData.ActivityDetails.RunningDetail.Steps,activity_type);
				
			//����
			ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();

			#ifdef COD 
			marathon_cal_time();
			#endif
		
			//��·��
			ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);		
		
			//����ƽ���ٶ�
			ActivityData.ActivityDetails.RunningDetail.Pace = CalculateSpeed(act_time,0,ActivityData.ActivityDetails.RunningDetail.Distance,activity_type);
			
			//�˶�ʱ��С��1����ʱ���������
			CalculateOptimumPace(ActivityData.ActivityDetails.RunningDetail.Pace, &ActivityData.ActivityDetails.RunningDetail.OptimumPace);
		    ActivityData.ActivityDetails.RunningDetail.Steps = ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
			//ƽ����Ƶ
		//	ActivityData.ActivityDetails.RunningDetail.frequency = CalculateFrequency(act_time,ActivityData.ActivityDetails.RunningDetail.Steps);
		
			//�����������ճ�������
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
				//���һ����ƽ������
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//���һ����ƽ������
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			#endif
			break;
		case ACT_WALK:
			//���������뵽�㷨��
			sport_data_input(2,SetValue.AppGet.StepLengthWalking,ActivityData.ActivityDetails.WalkingDetail.Steps,ACT_WALK);
				
			//����
			ActivityData.ActivityDetails.WalkingDetail.Distance = DistanceResult();
		
			//��·��
			ActivityData.ActivityDetails.WalkingDetail.Calorie = CalculateWalkEnergy(ActivityData.ActivityDetails.WalkingDetail.Distance/100,SetValue.AppSet.Weight);		
		
			//����ƽ���ٶ�
			ActivityData.ActivityDetails.WalkingDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.WalkingDetail.Distance,ACT_WALK);
		
			//�˶�ʱ��С��1����ʱ���������
			CalculateOptimumPace(ActivityData.ActivityDetails.WalkingDetail.Speed, &ActivityData.ActivityDetails.WalkingDetail.OptimumSpeed);
			ActivityData.ActivityDetails.WalkingDetail.Steps  = ActivityData.ActivityDetails.WalkingDetail.Steps - steps_pause_t;
			//ƽ����Ƶ
			ActivityData.ActivityDetails.WalkingDetail.frequency = CalculateFrequency(act_time,ActivityData.ActivityDetails.WalkingDetail.Steps);
		
			//�����������ճ�������
			DayMainData.SportSteps += ActivityData.ActivityDetails.WalkingDetail.Steps;
			if ((act_time %60) > 10)
			{
				//���һ����ƽ������
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//���һ����ƽ������
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			break;
		case ACT_MARATHON:
			//���������뵽�㷨��
			sport_data_input(2,SetValue.AppGet.StepLengthMarathon,ActivityData.ActivityDetails.RunningDetail.Steps,ACT_MARATHON);
				
			//����
			ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();
		
			//��·��
			ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);		
		
			//����ƽ���ٶ�
			ActivityData.ActivityDetails.RunningDetail.Pace = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.RunningDetail.Distance,ACT_MARATHON);
			
			//�˶�ʱ��С��1����ʱ���������
			CalculateOptimumPace(ActivityData.ActivityDetails.RunningDetail.Pace, &ActivityData.ActivityDetails.RunningDetail.OptimumPace);
			ActivityData.ActivityDetails.RunningDetail.Steps =ActivityData.ActivityDetails.RunningDetail.Steps -steps_pause_t;
			//ƽ����Ƶ
		//	ActivityData.ActivityDetails.RunningDetail.frequency = CalculateFrequency(act_time,ActivityData.ActivityDetails.RunningDetail.Steps);
		
			//�����������ճ�������
			DayMainData.SportSteps +=ActivityData.ActivityDetails.RunningDetail.Steps;
			if ((act_time %60) > 10)
			{
				//���һ����ƽ������
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//���һ����ƽ������
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			break;
		case ACT_SWIMMING:
			//����ʱ�����������ӣ����������Ҫ����һ��
			//laps = SwimDataCircleResult();
			//laps = SwimResultsOutputPort2_LengthCount();
			laps = SwimResultsPostProcessing();//post-processing
		
			//Average frequency
			ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency_swimming(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
			#ifdef COD 
			ActivityData.ActivityDetails.SwimmingDetail.pool_length = SetValue.AppGet.LengthSwimmingPool;
			ActivityData.ActivityDetails.SwimmingDetail.laps = laps;
			
			#endif
			//ƽ��swolf
			ActivityData.ActivityDetails.SwimmingDetail.Swolf = CalculateSwolf(ActivityData.ActivityDetails.SwimmingDetail.Strokes,laps,act_time);
			//��·��
			ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex);
			
			if(laps > Last_laps)
			{
				//����
				ActivityData.ActivityDetails.SwimmingDetail.Distance = SetValue.AppGet.LengthSwimmingPool *100 *laps;

				//ƽ��Ӿ��
				ActivityData.ActivityDetails.SwimmingDetail.Speed = CalculateSpeed(act_time,0,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);

				//ƽ��swolf
				//ActivityData.ActivityDetails.SwimmingDetail.Swolf = CalculateSwolf(ActivityData.ActivityDetails.SwimmingDetail.Strokes,laps,act_time);

				//��·��
				//ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex); 

				//ƽ����Ƶ
				//ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);


				//�����洢
				Store_StepData(&ActivityData.ActivityDetails.SwimmingDetail.Strokes);

				//����洢
				Store_DistanceData(&ActivityData.ActivityDetails.SwimmingDetail.Distance);

				//��·��洢
				Store_EnergyData(& ActivityData.ActivityDetails.SwimmingDetail.Calorie);
							

				//��Ȧʱ��
				
#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
				Store_CircleTimeData(act_time ,SetValue.SportSet.SwSwimmg);
#endif
							
				//һ��ƽ���ٶ�
				speedminute = CalculateSpeed((act_time - Last_time),distancelast,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);
				
				Store_SpeedData(&speedminute);																											

				//һ��ƽ������													
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount - hdrctlast);

				Store_HeartRateData(&hdrminute);
			}
			break;
		case ACT_CROSSCOUNTRY:
			//���������뵽�㷨��
			sport_data_input(2,SetValue.AppGet.StepLengthCountryRace,ActivityData.ActivityDetails.CrosscountryDetail.Steps,ACT_CROSSCOUNTRY);
			
			//����
			ActivityData.ActivityDetails.CrosscountryDetail.Distance = DistanceResult();

			#ifdef COD 
			marathon_cal_time();
			#endif
		
			//��·�����
			ActivityData.ActivityDetails.CrosscountryDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,act_time,SetValue.AppSet.Weight);	

			//����ƽ���ٶ�
			ActivityData.ActivityDetails.CrosscountryDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.CrosscountryDetail.Distance,activity_type);
		
			//�˶�ʱ��С��1����ʱ���������
			CalculateOptimumPace(ActivityData.ActivityDetails.CrosscountryDetail.Speed, &ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed);
			ActivityData.ActivityDetails.CrosscountryDetail.Steps = ActivityData.ActivityDetails.CrosscountryDetail.Steps -steps_pause_t;
			//�����������ճ�������
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
				//���һ����ƽ������
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//���һ����ƽ������
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			#endif
			break;
		case ACT_HIKING:
			//���������뵽�㷨��
			sport_data_input(2,SetValue.AppGet.StepLengthHike,ActivityData.ActivityDetails.CrosscountryDetail.Steps,ACT_HIKING);
			
			//����
			ActivityData.ActivityDetails.CrosscountryDetail.Distance = DistanceResult();
			#ifdef COD 
			marathon_cal_time();
			#endif
			//��·��
			ActivityData.ActivityDetails.CrosscountryDetail.Calorie = CalculateWalkEnergy(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,SetValue.AppSet.Weight);	
		
			//����ƽ���ٶ�
			ActivityData.ActivityDetails.CrosscountryDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.CrosscountryDetail.Distance,activity_type);

		  //�˶�ʱ��С��1����ʱ���������
			CalculateOptimumPace(ActivityData.ActivityDetails.CrosscountryDetail.Speed, &ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed);
			ActivityData.ActivityDetails.CrosscountryDetail.Steps = ActivityData.ActivityDetails.CrosscountryDetail.Steps -steps_pause_t;
			//�����������ճ�������
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
				//���һ����ƽ������
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//���һ����ƽ������
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			#endif
			break;
		case ACT_CLIMB:
			//���������뵽�㷨��
			sport_data_input(2,SetValue.AppGet.StepLengthClimbing,ActivityData.ActivityDetails.ClimbingDetail.Steps,ACT_CLIMB);
			
			//����
			ActivityData.ActivityDetails.ClimbingDetail.Distance = DistanceResult();

			#ifdef COD 
			marathon_cal_time();
			#endif
		
			//��ɽ���㴹ֱ�ٶ�
			ActivityData.ActivityDetails.ClimbingDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.ClimbingDetail.DropHeight 
												+ ActivityData.ActivityDetails.ClimbingDetail.RiseHeight,ACT_CLIMB);

			ActivityData.ActivityDetails.ClimbingDetail.Steps = ActivityData.ActivityDetails.ClimbingDetail.Steps -steps_pause_t;
			//�����������ճ�������
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
				//���һ����ƽ������
				speedminute = MeanSpeedResult();
				Store_SpeedData(&speedminute);
				//���һ����ƽ������
				hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
				Store_HeartRateData(&hdrminute);

			}
			#endif
			break;
		case ACT_CYCLING:
		case ACT_TRIATHLON_CYCLING:
			//���������뵽�㷨��
			sport_data_input(2,0,ActivityData.ActivityDetails.CyclingDetail.Cadence,ACT_CYCLING);
			
			//����
			ActivityData.ActivityDetails.CyclingDetail.Distance = DistanceResult();
		
			//����ƽ���ٶ�
			ActivityData.ActivityDetails.CyclingDetail.Speed = 
				CalculateSpeed(act_time,0,ActivityData.ActivityDetails.CyclingDetail.Distance,activity_type);
		
			//С��1���ӵ�����ٶ�
			CalculateOptimumSpeed(ActivityData.ActivityDetails.CyclingDetail.Speed, &ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed);
		
			//����ƽ��̤Ƶ
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
			//���������뵽�㷨��
			sport_data_input(2,0,ActivityData.ActivityDetails.SwimmingDetail.Strokes,ACT_TRIATHLON_SWIM);
		
			//����
			ActivityData.ActivityDetails.SwimmingDetail.Distance = DistanceResult();
		
			//��·��
			ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex);
		
			//����ƽ���ٶ�
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
	//�����˶��ľ��롢��·�����ݶ������ճ�������
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
		//����ʣ�µĹ켣��

	#endif
	}
}

//��ȡ���е�ƽ��̤Ƶ
uint32_t Get_Cycling_AvgCadence(void)
{
	uint32_t angCadence = 0;
	
	if(cadcount != 0)
	{
		angCadence = cadtotal/cadcount;
	}
	
	return angCadence;
}

//��ȡ����ͣʱ��
uint32_t Get_PauseTotolTime(void)
{
    return PauseTotolTime;
}

//��ȡ�˶�task״̬
bool Get_TaskSportSta(void)
{
    return TaskSportStatus;
}
    
//�����˶�task״̬
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

//��ȡ��ͣ״̬
bool Get_PauseSta(void)
{
    return PauseSta;
}

 
//������ͣ״̬
void Set_PauseSta(bool sta)
{
    PauseSta = sta;
	
//	//��ȡ��ǰʱ��
//	am_hal_rtc_time_get(&RTC_time);
	
    if (PauseSta == true)
    {
        memset(&PauseTime,0,sizeof(PauseTime));
		steps_pause_f = drv_lsm6dsl_get_stepcount();
		#ifdef COD 
		sport_total_time = PauseTotolTime +g_SportTimeCount;
		#endif
//		//��ͣ��ʼʱ��
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
//		//��ͣ����ʱ��
//		PauseTime.Pause_Stop_Time.Year= RTC_time.ui32Year; 
//		PauseTime.Pause_Stop_Time.Month= RTC_time.ui32Month;
//		PauseTime.Pause_Stop_Time.Day= RTC_time.ui32DayOfMonth;
//		PauseTime.Pause_Stop_Time.Hour = RTC_time.ui32Hour; 
//		PauseTime.Pause_Stop_Time.Minute = RTC_time.ui32Minute;
//		PauseTime.Pause_Stop_Time.Second = RTC_time.ui32Second;
//		
//		//������ͣʱ��
//		Store_PauseTimeData(&PauseTime);
	}
}

//��ȡ��Ӿ״̬
uint8_t Get_SwimSta(void)
{
    return SwimSta;
}

//������Ӿ״̬
void Set_SwimSta(uint8_t sta)
{
    SwimSta = sta;
}


//��ȡ����״̬
uint8_t get_cyclingsta(void)
{
    return s_cyclingsta;
}

//��������״̬
void set_cyclingsta(uint8_t sta)
{
    s_cyclingsta = sta;
}



//��ȡ����״̬
uint8_t Get_SaveSta(void)
{
    return SaveSta;
}

//���ñ���״̬
void Set_SaveSta(uint8_t sta)
{
    SaveSta = sta;
}

//����/�����˶�����
void CreatSportTask(void)
{
    if( TaskSportHandle == NULL )
    {
        // ActSemaphore = xSemaphoreCreateBinary();  //������ֵ�ź���
        //���δ��������
        xTaskCreate(TaskSport,"Task Sport",TaskSport_StackDepth,0,TaskSport_Priority,&TaskSportHandle);
    } 
    else
    {
        vTaskResume(TaskSportHandle);

    }
}

//�˳��˶�ģʽ
void CloseSportTask(void)
{
    //�˳��˶�ģʽ
	TaskSportStatus = false;
	#if defined (WATCH_STEP_ALGO)
	#else
	move_int =1;
	#endif
	xSemaphoreGive(ActSemaphore);
   
	//�����ѭ������,��ɾ��ѭ��������ʱ���򵼺�
	if((TRACE_BACK_DATA.is_traceback_flag == 1) ||(loadflag ==1))
	{
		trace_back_timer_delete();
	}
	if((ActivityData.Activity_Type >= ACT_TRIATHLON_SWIM) && (ActivityData.Activity_Type <= ACT_TRIATHLON_RUN))
	{
		//�������������˶���
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

//׼���˶�
void ReadySport(ScreenState_t index)
{
   	//����˶�������
	memset(&ActivityData,0,sizeof(ActivityData));
	g_SportTimeCount = 0;//�˶���ʱʱ���������
	//�˶�ģʽ����
  Set_IsInSportMode(true);
	switch(index)
	{
		case DISPLAY_SCREEN_RUN:
			//��ʼ������
			ActivityData.Activity_Type = ACT_RUN;
			#ifdef COD 
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK |HEIGHT_SW_MASK;//�Ʋ������ʡ���GPS
			#else
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//�Ʋ������ʡ���GPS
			#endif
								
			break;
		case DISPLAY_SCREEN_TRAINPLAN_RUN:
			Set_IsTrainPlanOnGoing(true);
			ActivityData.Activity_Type = ACT_TRAIN_PLAN_RUN;
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//�Ʋ������ʡ���GPS
			g_sport_status = SPORT_STATUS_READY;
			Is_Need_Hint_Flag	= 0;
			break;
		case DISPLAY_SCREEN_MARATHON:
			ActivityData.Activity_Type = ACT_MARATHON;
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//�Ʋ������ʡ���GPS
			break;			
		case DISPLAY_SCREEN_CROSSCOUNTRY:
			ActivityData.Activity_Type = ACT_CROSSCOUNTRY;
			g_SportSensorSW = STEP_SW_MASK | COMPASS_SW_MASK | HEARTRATE_SW_MASK |
								HEIGHT_SW_MASK | GPS_SW_MASK;//�Ʋ���ָ���롢���ʡ��߶ȡ�GPS
			break;		
		case DISPLAY_SCREEN_INDOORRUN:
			ActivityData.Activity_Type = ACT_INDOORRUN;
			g_SportSensorSW =  STEP_SW_MASK | HEARTRATE_SW_MASK ;//�Ʋ�������
			break;			
		case DISPLAY_SCREEN_WALK:
			ActivityData.Activity_Type = ACT_WALK;
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//�Ʋ������ʡ���GPS
			
			break;
		case DISPLAY_SCREEN_CLIMBING:
			ActivityData.Activity_Type = ACT_CLIMB;
			g_SportSensorSW = STEP_SW_MASK | COMPASS_SW_MASK | HEARTRATE_SW_MASK |
								HEIGHT_SW_MASK | GPS_SW_MASK;//�Ʋ���ָ���롢���ʡ��߶ȡ�GPS
			break;	
		case DISPLAY_SCREEN_CYCLING:
			ActivityData.Activity_Type = ACT_CYCLING;
			g_SportSensorSW = COMPASS_SW_MASK | HEARTRATE_SW_MASK |
								HEIGHT_SW_MASK | GPS_SW_MASK;//ָ���롢���ʡ��߶ȡ�GPS
	
			break;
		case DISPLAY_SCREEN_CROSSCOUNTRY_HIKE:
			ActivityData.Activity_Type = ACT_HIKING;
			g_SportSensorSW = STEP_SW_MASK | COMPASS_SW_MASK | HEARTRATE_SW_MASK | 
                                HEIGHT_SW_MASK | GPS_SW_MASK ;//�Ʋ���ָ���롢���ʡ��߶ȡ�GPS
			break;		
		case DISPLAY_SCREEN_SWIMMING:
			ActivityData.Activity_Type = ACT_SWIMMING;
			g_SportSensorSW = STEP_SW_MASK | GYRO_SW_MASK ;//�Ʋ���������
			break;
         case DISPLAY_SCREEN_TRIATHLON:
			ActivityData.Activity_Type = ACT_TRIATHLON_SWIM;
			g_SportSensorSW =  STEP_SW_MASK | GYRO_SW_MASK | 
								HEARTRATE_SW_MASK | GPS_SW_MASK;//�Ʋ��������ǡ����ʡ�GPS
			break;		
		case DISPLAY_SCREEN_TRIATHLON_CYCLING:
			ActivityData.Activity_Type = ACT_TRIATHLON_CYCLING;;
			g_SportSensorSW = COMPASS_SW_MASK | HEARTRATE_SW_MASK |
								HEIGHT_SW_MASK | GPS_SW_MASK;//ָ���롢���ʡ��߶ȡ�GPS
			break;
        case DISPLAY_SCREEN_TRIATHLON_RUNNING:
			ActivityData.Activity_Type = ACT_TRIATHLON_RUN;
			g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | GPS_SW_MASK|COMPASS_SW_MASK;//�Ʋ������ʡ���GPS
			break;	
		default:
			break;
     }
    //����ֵ����
    drv_clearHdrValue();
    if(g_SportSensorSW & HEARTRATE_SW_MASK)
		{
			//���ʳ�ʼ����
      task_hrt_start();
		}
    //GPS��ʼ����,�����Ƿ�gps��������
		Last_GPS_Lon = 0;
    Last_GPS_Lat = 0;
		sport_alt = 0;
	
#if !defined WATCH_SIM_SPORT
    gps_value_clear();
#endif
    //��ʼ���켣����
    InitTrack();

//gps��λ�ɹ����
	gps_located = 0;

#if defined WATCH_SIM_SPORT
		//ƽ����Ƶ����
	set_ave_frequency(0);
#endif
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
		//����ʱ������
	memset(&g_track_mark_data,0,sizeof(SportTrackMarkDataStr));
#endif
	if(g_SportSensorSW & GPS_SW_MASK)
	{

       if(drv_ublox_status_get() == GPS_OFF)
       {
           //���GPSû�д򿪣����¿���
           CreateGPSTask();
       }
		
	}
    else
    {
        //���ڲ��ÿ���GPS���˶����񣬹ر�GPS
 #if defined WATCH_SIM_SPORT
 #else
        //drv_ublox_status_set(GPS_OFF);
 #endif
    }
   
}
//�˳��˶�׼������
 void ReadySportBack(void)
{
  //����˶�������
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
//�˳��Ƽ������˶�׼������ ���ù�GPS ������˶��������Ĳ��ù����� ���߽������Ĺ����ʼ������˶�����
void ReadyCloudSportBack(void)
{
	TASK_SPORT_PRINTF("[task_sport]:ReadyCloudSportBack screen=%d\r\n",g_tool_or_sport_goto_screen);
	if(g_tool_or_sport_goto_screen == DISPLAY_SCREEN_GPS_DETAIL)
	{
		//����˶�������
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
  //uint16_t altitude;//��ѹ���߶�
  	//�����־
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
		//ָ�����ʼ����
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
           //���GPSû�д򿪣����¿���
           CreateGPSTask();
       }
		
	}
  else
  {
        //���ڲ��ÿ���GPS���˶����񣬹ر�GPS
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
			 
         //�رմ������Ʋ��㷨
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
		am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC); //��Ϊ���ж�
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
	//�ر�ָ����
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
			  
         //�򿪴������Ʋ��㷨
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

//���������
void sport_triathlon_change_start(void)
{
	uint8_t type = ActivityData.Activity_Type;

	//��ͣ�˶�
	Set_PauseSta(true);
	
	//��������
	//SaveSport(1);
	
	//�رմ����������ر�GPS
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
	
	//����˶�������
	memset(&ActivityData,0,sizeof(ActivityData));
	
	//��ʼ�����˶�
	ActivityData.Activity_Type = type + 1;
	if((ActivityData.Activity_Type == ACT_TRIATHLON_FIRST) 
		|| (ActivityData.Activity_Type == ACT_TRIATHLON_SECOND))
	{
		//����켣
		g_SportSensorSW = GPS_SW_MASK;	//GPS
	}
	else if(ActivityData.Activity_Type == ACT_TRIATHLON_CYCLING)
	{
		//����
		g_SportSensorSW = COMPASS_SW_MASK | HEARTRATE_SW_MASK |
							HEIGHT_SW_MASK | GPS_SW_MASK;	//ָ���롢���ʡ��߶ȡ�GPS
	}
	else
	{
		//�ܲ�
		g_SportSensorSW = STEP_SW_MASK | HEARTRATE_SW_MASK | 
							GPS_SW_MASK;	//�Ʋ������ʡ�GPS
	}
	
	//�˶���ʱʱ���������
	g_SportTimeCount = 0;

	//gps��λ�ɹ����
	gps_located = 0;
	
	StartSport();
	if(g_SportSensorSW & HEARTRATE_SW_MASK)
	{
		//���ʳ�ʼ����
		task_hrt_start();
	}
}

//��������ѡ������󣬲�ɾ��ǰ�漸���˶�
static void sport_triathlon_cancel(void)
{
	uint8_t i,index,temp;
	uint8_t buf[2] = {0};
	
	index = m_sport_record.ActivityIndex;
	for(i = ActivityData.Activity_Type - 1; i >= ACT_TRIATHLON_SWIM; i--)
	{
		if(index <= 0)
		{
			//���������һ���
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

//�˶�ѡ���Ժ����
void sport_continue_later_start(void)
{
	//�����ѭ������,��ɾ��ѭ��������ʱ���򵼺�
	if((TRACE_BACK_DATA.is_traceback_flag == 1)||(loadflag ==1))
	{
		trace_back_timer_delete();
	}
	//�رմ�����
	CloseSensor();
	
	//�رմ�����,�������˶�����
//	CloseSportTask();
	
	//��λ��־
	SportContinueLaterFlagSet();
}

//�˶������Ժ����
void sport_continue_later_end(void)
{
	//�����־
	SportContinueLaterFlagClr();
	
	//����������
	OpenSensor();
	//�����˶�����
//	CreatSportTask();
	
	//�����ѭ�������򵼺�,�����¿�ʼѭ��������ʱ��
	if((TRACE_BACK_DATA.is_traceback_flag == 1)||(loadflag ==1))
	{
		trace_back_timer_start();
	}
}
/**
  * @brief  �����˶���¼������/������  ���浽flashʱ��
* @param  save_starttime���˶���ʵ����ʵʱ�� save_stoptime�����˶�ʱ��ʱ��
					beforetime  �˶���ʼǰ�ı��е�ʱ��
          is_stopsport  Ϊ������֮ǰ������
������������  �˶�ʱ��=save_stoptime-save_starttime;
  * @retval
  */
static void SetSportRecord(rtc_time_t beforetime,rtc_time_t save_starttime,rtc_time_t save_stoptime,bool is_stopsport)
{
	if((ActivityData.Activity_Type > ACT_TRIATHLON_SWIM) && (ActivityData.Activity_Type <= ACT_TRIATHLON_RUN))
	{
		//��������������˶�
		m_sport_record.ActivityIndex = ActivityData.Activity_Index;
	}
	else
	{
		if(is_stopsport)
		{//�˶�ֹͣ�󱣴�
			if( !(save_starttime.Year == beforetime.Year && save_starttime.Month == beforetime.Month
			  && save_starttime.Day == beforetime.Day) )
			{//�˶���ʼǰʱ�����˶���ʵ�Ŀ�ʼʱ�䲻һ�� ����������»�ȡ ���򲻱�
				ActivityData.Activity_Index = SetValue.CurrentAddress.IndexActivity+1;
			}
			if(save_starttime.Year == save_stoptime.Year && save_starttime.Month == save_stoptime.Month
			 && save_starttime.Day == save_stoptime.Day)
			{//ͬһ���˶� �˶��������ͻ��� 
				m_sport_record.DayIndex = DayMainData.Index;
			}
			else
			{//��ͬ�� ������Ҫ���˶���0��Ļ;�˶�������0�㱻������,��Ҫ��¼ǰ���
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
		{//�˶���ʼ
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
//��ʼ�˶�   
void StartSport(void)
{
	float press;
	uint16_t altitude;//��ѹ���߶�
	steps_record_t = 0;    //�˶���ʼʱ����
	steps_record_f = 0;
	
	timer_timeout_back_stop(false);

	//����˶�������
	//memset(&ActivityData,0,sizeof(ActivityData));
	
	//����Ǵ��Ƽ����������˶��켣��ʼ���˶�����Ҫ���𹤾�����
	if((ENTER_NAVIGATION == 1) || (ENTER_MOTION_TRAIL == 1))
	{
		//���𹤾�����
		tool_task_close();
	}
	
	SetValue.IsAllowBaseHDR = 0;//�����˶�������������ʹ��ܰ������ѱ����
	am_hal_rtc_time_get(&RTC_time); //��ȡ��ǰʱ��
	ActivityData.Act_Start_Time.Year= RTC_time.ui32Year;
	ActivityData.Act_Start_Time.Month= RTC_time.ui32Month;
	ActivityData.Act_Start_Time.Day= RTC_time.ui32DayOfMonth;
	ActivityData.Act_Start_Time.Hour = RTC_time.ui32Hour;
	ActivityData.Act_Start_Time.Minute = RTC_time.ui32Minute;
	ActivityData.Act_Start_Time.Second = RTC_time.ui32Second;
	ActivityData.Act_Stop_Time.Hour = 0;  //�������ֹͣʱ��ṹ�屣�����ʱ��
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
	ActivityData.Pause_Start_Address = SetValue.CurrentAddress.AddressPause;//ʵ�����ڴ洢�����������
#endif
	//�˶���ʼǰ�ѵ�ǰʱ���¼����
	m_time_sport_ready = ActivityData.Act_Start_Time;
	//���浱ǰ����������ݣ����ڱ����˶����ݣ��������
	m_sport_record.ActivityType = ActivityData.Activity_Type;
	if(ActivityData.Activity_Type == ACT_TRIATHLON_SWIM)
	{
		//�������������˶���
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
        //��Ȧ���뱣��
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
	//��ʼ�����ʻָ���Ϊ250����ʾ���ʻָ�����Ч
  	ActivityData.ActivityDetails.CommonDetail.HRRecovery = 250;
	 
	//�����ۼ�ֵ���㣬��������������;���룬��ʼ������
    hdrtotal = 0;
    hdrtcount = 0;
    hdrlast = 0;
    hdrctlast = 0;
	har_total_cod = 0;
	har_count_toal_cod = 0;
	ave_hdr_cod = 0;
	//̤Ƶ
	cadtotal = 0;
	cadcount = 0;
	cadlast = 0;
    cadctlast = 0;
	m_LastCrankRevolutions = 0;
	m_LastCrankEventTime = 0;

    //�ϴξ��뻺������
    distancelast = 0;
	
	//gps��洢˳��
	store_gps_nums = 0;

	//��������ʼ��
	dot_track_nums = 0;
	dottrack_draw_time = 0;
	dottrack_out_time = 0;	//������㵽������ʾ����
	dottrack_nogps_time = 0;//�������gpsû�ж�λ��ʾ����
	
	
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
			gps_store_flag = false;
#endif
	
	//���ʻָ�����ز���
	 sport_end_flag = 0;  //�Ƿ��˶�����ѡ���־
	 sport_end_hrvalue = 0;   //�˶�����ʱ����ֵ
	 sport_min_hrvalue = 0;   //�˶�һ���Ӻ�����ֵ
	 sport_end_time = 0;    //�˶�����
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
	 set_sport_pause_event_status(0);//�������γ�ʼ��ѡ�� Ĭ��ѡ��һ���˶�����
#endif
	//�㷨��ʼ��
	TrackOptimizingInit();
     
	if(g_SportSensorSW & STEP_SW_MASK)
	{
      //�Ʋ���ʼ����
        begin_steps = drv_lsm6dsl_get_stepcount();
	
	}
	
	if(g_SportSensorSW & HEIGHT_SW_MASK)
	{
		//�߶ȳ�ʼ����	
        press = 0;
        altitude = 0;
		ReadSeaLevelPressure(RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth);
       //��ʼ���㷨
        CalculateAltitude(press,&altitude,&altitude);
        //��һ����ѹֵ
        Get_Sport_Altitude(&ActivityData.ActivityDetails.CommonDetail.RiseHeight,&ActivityData.ActivityDetails.CommonDetail.DropHeight);		
		
	}
	

	if(g_SportSensorSW & GYRO_SW_MASK)
	{
		//�����ǳ�ʼ����
		
	}

    if(g_SportSensorSW & COMPASS_SW_MASK)
	{
#if !defined WATCH_SPORT_NO_COMPASS 

		//ָ�����ʼ����
		drv_lis3mdl_reset();
        am_util_delay_ms(30);
       // drv_lis3mdl_init();
		magn_dev_para_init();
#endif	
	}

    
    //��Ӿ��ʼ������
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
        drv_lsm6dsl_disable_pedometer();//�رմ������Ʋ��㷨
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
			
         //�رմ������Ʋ��㷨
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
		
		//ԭʼ���ݴ洢��ʼ��
  #if defined STORE_ORG_DATA_TEST_VERSION
//    Store_OrgData_Start();
  #endif
		
    //��ͣʱ������
    PauseTotolTime = 0;
    memset(&PauseTime,0,sizeof(PauseTime));

    //��ʼ���˶�����
    Remind_Sport_Init();
	
	//�Զ���Ȧ������������
	memset(&last_circle_data,0,sizeof(last_circle_data));
	
    //���������Ƿ�켣��ѭ����ʾ��ʱ��
    if (loadflag ==1)
    {
        trace_back_timer_start();
    }
    //�����˶�task
    Set_PauseSta(false);
    Set_TaskSportSta(true);
	steps_pause_f = 0;
	steps_pause_t = 0;
	step_pattern_flag = 1;
	CreatSportTask();	
	am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC); //��Ϊ���ж�
	
}
#if defined WATCH_SPORT_EVENT_SCHEDULE
/*�½����� ��1����*/
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
	g_sport_event_status.event_id = get_sport_event_id(ActivityData.sport_event);//�½�����ID
	TASK_SPORT_PRINTF("[task_sport]:--set_new_sport_event sport_type=%d,status=%d,total_nums=%d,num=%d,event_id=%d\r\n"
	,g_sport_event_status.sport_type,g_sport_event_status.status,g_sport_event_status.total_nums,g_sport_event_status.nums,g_sport_event_status.event_id);
}
#endif
/*
���ܣ��˳�����ı���Ӧ����ֵ��
������sta    �Ƿ񱣴����ݣ� 0 Ϊ����   ������ ����	
����ֵ:��
������
*/
void SaveSport( uint8_t	sta )
{
	if( sta )
	{
		ActivityData.ActTime = ActivityData.Act_Stop_Time.Hour*3600 + ActivityData.Act_Stop_Time.Minute*60 + ActivityData.Act_Stop_Time.Second;
		
		//ѵ���ƻ�����ϸ���ݹ�Ϊ�ܲ�һ�ౣ��
		if(ACT_TRAIN_PLAN_RUN == ActivityData.Activity_Type)
		{
			ActivityData.Activity_Type = ACT_RUN;
		}
		
		//�����˶�ʱ��������������������㷨
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
				//��Ӿ��
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
			
			//����
			ActivityData.ActivityDetails.SwimmingDetail.Strokes = 2*swim_store[Freestyle] + 2*swim_store[Backstroke]+ swim_store[Breaststroke]+ swim_store[ButterflyStroke]+ swim_store[UnknownStyle];
		}
		else if (ActivityData.Activity_Type == ACT_CYCLING)
		{
			ActivityData.ActivityDetails.CyclingDetail.Speed = (uint32_t)(ActivityData.ActivityDetails.CyclingDetail.Speed/36.f*10)/10.f*36;
			ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed = (uint32_t)(ActivityData.ActivityDetails.CyclingDetail.OptimumSpeed/36.f*10)/10.f*36;
		}
		#endif
		//�����˶�����ʱ����
		//ActivityData.ActivityDetails.CommonDetail.AvgHeart = hdrtotal/hdrtcount;
		ActivityData.ActivityDetails.CommonDetail.AvgHeart = ave_hdr_cod;
		#ifndef COD 
		ActivityData.ActivityDetails.CommonDetail.AerobicEffect = GetAerobicIntensity(ActivityData.ActivityDetails.CommonDetail.HeartRateZone);
		ActivityData.ActivityDetails.CommonDetail.AnaerobicEffect = GetAnaerobicIntensity(ActivityData.ActivityDetails.CommonDetail.HeartRateZone);
		#endif
		//�ָ�ʱ��
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
				
				//���û��ڶ���Ļ������ʡ�ǰһ��������ʣ������ٴ��˶����ָ�ʱ�䲻�����ۻ�
				if ((DayMainData.BasedHeartrate != 0)&&(DayMainData.BasedHeartrate <=last_basedheartrate ))
				{
					//�����˶�����Ҫ�Ļָ�ʱ��ܵ�ǰ�Ļָ�ʱ��ʱ����ʾ��ǰ�ָ�ʱ��
					//�����˶�����Ҫ�Ļָ�ʱ�䣾��ǰ�Ļָ�ʱ��ʱ����ʾ�����˶�����Ҫ�ָ�ʱ��
					if (ActivityData.ActivityDetails.CommonDetail.RecoveryTime > SetValue.RecoverTime )
					{
						SetValue.RecoverTime = ActivityData.ActivityDetails.CommonDetail.RecoveryTime;
					}
				}
				else //�û�δ���Ի������ʻ�ڶ���������ʣ�ǰһ��������ʣ������ٴ��˶����ָ�ʱ������ۻ�
				{
					SetValue.RecoverTime +=ActivityData.ActivityDetails.CommonDetail.RecoveryTime;
				}
				
				if (SetValue.RecoverTime > 20160) //����14��
				{
					SetValue.RecoverTime  = 20160;
				}
				SetValue.RecoverTimeTotal = SetValue.RecoverTime;
				break;
			default:
				break;
		}
		
            
		am_hal_rtc_time_get(&RTC_time); //��ȡ��ǰʱ��
		ActivityData.Act_Stop_Time.Year= RTC_time.ui32Year;  //����ֹͣʱ��
		ActivityData.Act_Stop_Time.Month= RTC_time.ui32Month;
		ActivityData.Act_Stop_Time.Day= RTC_time.ui32DayOfMonth;
		ActivityData.Act_Stop_Time.Hour = RTC_time.ui32Hour;
		ActivityData.Act_Stop_Time.Minute = RTC_time.ui32Minute;
		ActivityData.Act_Stop_Time.Second = RTC_time.ui32Second;
		
		//���ʼʱ��=����ʱ��-�˶�ʱ��-�˶���ͣʱ��  ��λ��
		unsigned int startTime = GetTimeSecond(ActivityData.Act_Stop_Time) - g_SportTimeCount - PauseTotolTime;
		
		ActivityData.Act_Start_Time = GetYmdhms(startTime);//�뻻���������  ���ʼʱ��
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
		{//�����˶��������� ��������
			if(get_sport_pause_event_status() == 0)
			{//ѡ���˶�����
				if(g_sport_event_status.event_id != 0)
				{//��������
					if(g_sport_event_status.nums < SPORT_EVENT_SCHEDULE_NUM_MAX)
					{
						g_sport_event_status.status = 0;
						g_sport_event_status.nums += 1;
						//��������ID
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
					{//�������� �����½�����
						//�½�����
						set_new_sport_event();
					}
				}
				else
				{//û�����»������¿�ʼ �����˶� �Ƽ��������治��������
					if(g_sport_event_is_restart_status == 1 && g_tool_or_sport_goto_screen != DISPLAY_SCREEN_GPS_DETAIL)
					{
						if(g_sport_event_status.nums < SPORT_EVENT_SCHEDULE_NUM_MAX)
						{
							g_sport_event_status.status = 0;
							g_sport_event_status.nums = 0;
							//��������ID
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
			{//ѡ�����ν���
				if(g_sport_event_status.event_id != 0)
				{//��������
					if(g_sport_event_status.nums < SPORT_EVENT_SCHEDULE_NUM_MAX)
					{//��������
						g_sport_event_status.status = 1;
						g_sport_event_status.nums += 1;
						//��������
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
					{//�½�����
						set_new_sport_event();
					}

				}
				else
				{//û������ 
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
		ActivityData.Pause_Stop_Address = SetValue.CurrentAddress.AddressPause;//ʵ�����ڴ洢�����������
#endif
		SetSportRecord(m_time_sport_ready,ActivityData.Act_Start_Time,ActivityData.Act_Stop_Time,true);

		//�����������ջ�����򲻱���
		if(m_sport_record.ActivityIndex <= DAY_ACTIVITY_MAX_NUM)
		{
			if(RTC_time.ui32DayOfMonth == m_sport_record.Date && RTC_time.ui32Month == m_sport_record.Month
				 && RTC_time.ui32Year == m_sport_record.Year)
			{//��ͬ�� ������������� 
				//û���˶�����
				SetValue.CurrentAddress.IndexActivity++;
			}
			SetValue.TrainRecordIndex[m_sport_record.DayIndex].ActivityNum++;

			if(SetValue.IsFirstSport == 1)
			{//�˶�����󣬵�һ���˶���¼״̬�ı�
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
			//��������������˶����õ�ÿ������ʱ��
			gui_sport_triathlon_data_get();
		}
        //��������켣
        SaveGpsToXY();
	}
	else
	{
		if((ActivityData.Activity_Type > ACT_TRIATHLON_SWIM) && (ActivityData.Activity_Type <= ACT_TRIATHLON_RUN))
		{
			if(gui_sport_triathlon_status_get() != 1)
			{
				//��������������˶����ҷ����˶�������ɾ��ǰ�漸���˶�
				sport_triathlon_cancel();
				gui_sport_triathlon_status_set(0);
			}
		}
		//����ѵ���ƻ��˶��� ��δ���̬
		Set_IsCompleteTodayTrainPlan(0);
	}
	//�洢��ǰʹ�õĵ�ַ�ռ�
	Store_SetData();
}

//�ر��˶�
void CloseSport(void)
{
	//��Ϊ���ж�

	am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_MIN); 
    	
	//�رռƲ�
	if(g_SportSensorSW & STEP_SW_MASK)
	{
	
	}
	
	//�ر�GPS
	if(g_SportSensorSW & GPS_SW_MASK)
	{
        CloseGPSTask();
	}
	 
	//�ر�����
	if(g_SportSensorSW & HEARTRATE_SW_MASK)
	{
        task_hrt_close();
        
        
	}
	
	//�ر���ѹ
	if(g_SportSensorSW & HEIGHT_SW_MASK)
	{

	}
	
	//�ر�ָ����
	if(g_SportSensorSW & COMPASS_SW_MASK)
	{
#if !defined WATCH_SPORT_NO_COMPASS 
        drv_lis3mdl_standby();
#endif
	}

    //�ر�������
	if(g_SportSensorSW & GYRO_SW_MASK)
	{

	}

	gps_located = 1; //gps��λ�ɹ����

    //������Ӿʱ�Ĵ���
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
         //�򿪴������Ʋ��㷨
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
	
	//����������ͣʱ��
	Set_PauseSta(false);

    InitLoadTrack();

	dottrack_draw_time = 0;
	dottrack_out_time = 0;	//������㵽������ʾ����
	dottrack_nogps_time = 0;//�������gpsû�ж�λ��ʾ����
	//�����˶�����
    SaveSport(Get_SaveSta());
	
	if(ActivityData.Activity_Type == ACT_TRAIN_PLAN_RUN)
	{
		Is_Need_Hint_Flag = 0;
	}
	
	//����Ƽ��������˶��켣�ı�־
	ENTER_NAVIGATION = 0;
	ENTER_MOTION_TRAIL = 0;

	step_pattern_flag = 0;
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
	//�Ƽ����� ״̬����
	set_share_track_back_discreen(GOTO_SHARE_TRACK_BY_NONE);
	g_tool_or_sport_goto_screen = DISPLAY_SCREEN_HOME;
	g_sport_event_is_restart_status = 0;
#endif
#if defined WATCH_SPORT_EVENT_SCHEDULE
	g_start_search_cloud_status = 0;//�Ƽ�����������ʽ0 ���˶����� 1�˶���������
#endif
}

//��ȡ̤Ƶ�������
uint32_t sport_cycling_cadence(void)
{
	float crankCadence = 0;
	static float crankCadence_old = 0;
	
	extern _ble_accessory g_ble_accessory;
	
	if((g_ble_accessory.data.cycle_data_valid) && (g_ble_accessory.data.cycle_is_crank_revolution_data_present))
	{	
		//������Ч
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

//��ֹ���ݶ���ʱ��̤Ƶ������ʾ0
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
	
	//�����½��߶�
	Get_Sport_Altitude(&ActivityData.ActivityDetails.CommonDetail.RiseHeight,&ActivityData.ActivityDetails.CommonDetail.DropHeight);				
	//5���ӱ�����ѹ�߶�
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
	//һ���Ӽ���һ�������½��߶�
	if (ActivityData.Act_Stop_Time.Second  == 0)
	{
		CalculateAltitude(sport_press,&ActivityData.ActivityDetails.CommonDetail.RiseHeight,&ActivityData.ActivityDetails.CommonDetail.DropHeight);
	}
	
	//5���ӱ�����ѹ�߶�
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
    uint8_t hdrvalue = 0;	//����ֵ
    uint8_t pauseswitch;	//����ͣ���˶�״̬���
    uint16_t laps; 			//��Ӿ����
    uint8_t hdrminute;		//һ����ƽ������
    uint16_t speedminute;	//һ����ƽ���ٶ�/����
	uint32_t cadenceminute;	//һ����ƽ��̤Ƶ
    uint32_t tempsteps;
    uint16_t last_circle_pace = 0;	//��Ȧ����
	uint32_t last_circle_time = 0;	//��Ȧʱ��
	uint32_t last_circle_dis = 0;	//��Ȧ����
	uint32_t last_circle_step = 0;//��һȦ����
	uint32_t last_circle_hdrtotal = 0;	//��һȦ������ֵ
	uint32_t last_circle_hdrtcount = 0; //��һȦ���ʴ���
	uint16_t last_circle_riseheight = 0;	//��һȦ�����߶�
	uint16_t last_circle_dropheight = 0; //��һȦ�½��߶�
	int32_t swim_type = 0;

	while(1)
	{
		//ALGO_HF_SWIM_PRINTF("3: %d\n",uxTaskGetStackHighWaterMark(TaskSportHandle));
		if( xSemaphoreTake( ActSemaphore, 2000/portTICK_PERIOD_MS ) == pdTRUE )
		{
			if(TaskSportStatus == false)
			{

				//�˳���Ӧ�˶�ģʽ
				CloseSport();
				//�˳�ʱ���㣬��ֹӰ���´��˶�
				hdrvalue = 0;	//����ֵ
				last_circle_pace = 0;	//��Ȧ����
				last_circle_time = 0;	//��Ȧʱ��
				last_circle_dis = 0;	//��Ȧ����
				last_circle_step = 0;//��һȦ����
				last_circle_hdrtotal = 0;	//��һȦ������ֵ
   				last_circle_hdrtcount = 0; //��һȦ���ʴ���
   				last_circle_riseheight = 0;	//��һȦ�����߶�
				last_circle_dropheight = 0; //��һȦ�½��߶�
	
				//��������
				vTaskSuspend(NULL);
                continue;
			}
            
			if(PauseSta == false)
            {
							g_SportTimeCount ++;
				//ʱ��+1S
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
                //����ͣʱ��+1S
                PauseTotolTime++;
                
                //������ͣʱ��+1S
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
				//��ͣ״̬
				pauseswitch = 1; 
			}
			else
			{
				//�˶�״̬
				pauseswitch = 0;
			}
#ifdef DEBUG_ERR           
            heartlight = 0;

#endif       
			//���������ʾ
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

			//�˶�ʱ��
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
			{//���Ƽ�����֧�ֹ���ʱ�� ѭ��������֧��
				is_sport_has_close_time(get_navigation_index(),ActivityData.Activity_Type);
				//�Ƽ������˶� ��Ч����ʱ������ ���¹���ʱ���
				if(g_track_mark_data.is_close_time_valid == 1
					 && g_track_mark_data.track.close_time_index > 0)
				{
					//����
					sport_cloud_start_notify();
				}
			}
			
#endif
     
		
			switch(ActivityData.Activity_Type)
			{
				case ACT_RUN:
				case ACT_TRIATHLON_RUN:
				case ACT_TRAIN_PLAN_RUN:
					//����
					ActivityData.ActivityDetails.RunningDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					#ifdef COD 
					SetValue.run_step = last_run_step + ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
					#endif
					
					//���������뵽�㷨��
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthRun,ActivityData.ActivityDetails.RunningDetail.Steps,ACT_RUN);
				
					//����
					ActivityData.ActivityDetails.RunningDetail.Pace = SpeedResult();
#if defined WATCH_SIM_SPORT
					//��Ƶ
					ActivityData.ActivityDetails.RunningDetail.frequency = get_steps_freq();
#endif 					
					if(PauseSta == false)
					{
						//�˶�״̬
						//����
						ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();
						
						//����ƽ������
						ActivityData.ActivityDetails.RunningDetail.Pace = SpeedResult();
												
						//����                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						#ifdef COD 
						marathon_cal_time();
						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20��洢һ��
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
						//For Test ����ѵ���ƻ����ﵽ20s����Ϊѵ���ƻ����,������ɾ��
						if(act_time >= 60)
						{
							Set_IsCompleteTodayTrainPlan(1);
						}
#endif
						//ѵ���ƻ����� ѵ���ƻ����ڽ������ҴﵽĿ��ֵʱ����
#if defined WATCH_SIM_SPORT
						if(Get_IsTrainPlanOnGoing() == true)
#else
						if(ActivityData.Activity_Type == ACT_TRAIN_PLAN_RUN)
#endif
						{
#ifdef DEBUG_TRAINPLAN
							//ѵ���ƻ�
							Upload[1].Distance = 9000;//��
							Upload[1].Calories = 7000;//��
							Upload[1].Hour = act_time;//��
							Upload[1].RecordIndex = ActivityData.Activity_Index;//����
#else
							//ѵ���ƻ�
							Upload[1].Distance = ActivityData.ActivityDetails.RunningDetail.Distance/100;//��
							Upload[1].Calories = ActivityData.ActivityDetails.RunningDetail.Calorie;//��
							Upload[1].Hour = act_time;//��
							Upload[1].RecordIndex = ActivityData.Activity_Index;//����
#endif
							

#ifdef DEBUG_TRAINPLAN
							if(Get_IsTrainPlanOnGoing() && Is_TodayTimeTrainPlan() && Get_IsCompleteTodayTrainPlan() == 1
								  && Is_Need_Hint_Flag == 0 && get_trainplan_valid_flag())//������һ��  Ŀǰ�����ã��滻���Ŀ������ֱ�����ݽ��,�Դﵽ20s�������
#else
							if(Get_IsTrainPlanOnGoing() && Is_TodayTimeTrainPlan() && Get_IsCompleteBothTodayTrainPlan(Download[1].Type) == 1
								  && Is_Need_Hint_Flag == 0 && get_trainplan_valid_flag())//������һ�� ��ʽ�汾��
#endif
							 {
								 Is_Need_Hint_Flag++;
								 Set_IsCompleteTodayTrainPlan(1);
								 Remind_TrainPlan_Complete();
							 }
						}

						//�����˶����ݲ���
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{
					   
							//�Ʋ��洢
							tempsteps = ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&tempsteps);
							#endif
							//����洢

							Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
							//һ����ƽ���ٶ�
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
							
							//��·��
							ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);
							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.RunningDetail.Calorie);
							
							//������٣�һ���ӻ�ȡһ�Σ���߾�ȷ��
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);
							#ifndef COD
							//һ����ƽ������
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							#endif
						}
					
						//gps�洢
						
						if (GetGpsStatus())
						{
							store_gps_nums++;
						
						}
						

						
						//���������ݲ���Ҫ����
						if(ActivityData.Activity_Type != ACT_TRIATHLON_RUN)
						{
							//�Զ���Ȧʱ�� �켣�洢
							Store_AuotCirclle(ActivityData.ActivityDetails.RunningDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwRun);	
                       #if defined WATCH_SIM_SPORT
							if(Get_IsTrainPlanOnGoing() == true)
                       #else
							if(ActivityData.Activity_Type == ACT_TRAIN_PLAN_RUN)
                       #endif
							{
								//ѵ���ƻ���������
								Remind_Pace_TrainPlan(ActivityData.ActivityDetails.RunningDetail.Pace,ActivityData.ActivityDetails.RunningDetail.Distance,
													Download[1].TargetSpeedLimitUp,Download[1].TargetSpeedLimitDown);
							}
							else
							{
								//��������
								Remind_Heartrate(hdrvalue,SetValue.SportSet.SwRun);
								//Ŀ���������
								Remind_GoalDistance(ActivityData.ActivityDetails.RunningDetail.Distance/100,SetValue.SportSet.DefHintDistanceRun,SetValue.SportSet.SwRun);
								//��������
								Remind_Pace(ActivityData.ActivityDetails.RunningDetail.Pace,SetValue.SportSet.DefHintSpeedRun,SetValue.SportSet.SwRun);
								
								//Ŀ��ʱ������
								Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeRun,SetValue.SportSet.SwRun);
								
								//��Ȧ����
								if (Remind_CircleDistance(ActivityData.ActivityDetails.RunningDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwRun))
								{
									#ifdef COD 
									last_circle_pace = CalculateSpeed(act_time,0,ActivityData.CircleDistance*100,ACT_RUN);
									#else
									//������Ȧƽ������
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

					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
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

					gps_data_get_sm.output_data_sum = 0;//��������
					break;
				case ACT_WALK:
					//����
					ActivityData.ActivityDetails.WalkingDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					
					//���������뵽�㷨��
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthWalking,ActivityData.ActivityDetails.WalkingDetail.Steps,ACT_WALK);
				
					//����
					ActivityData.ActivityDetails.WalkingDetail.Speed = SpeedResult();
				
					//��Ƶ
					ActivityData.ActivityDetails.WalkingDetail.frequency = get_steps_freq();
					
					if(PauseSta == false)
					{
						//�˶�״̬
						//����
						ActivityData.ActivityDetails.WalkingDetail.Distance = DistanceResult();
												
						//����                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
#if defined WATCH_SIM_SPORT
						ActivityData.ActivityDetails.WalkingDetail.AvgHeart = hdrtotal/hdrtcount;
						ave_frequency = CalculateFrequency(ActivityData.ActTime,ActivityData.ActivityDetails.WalkingDetail.Steps -steps_pause_t );
#endif 

						//�����˶����ݲ���
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{
							//�Ʋ��洢
							tempsteps = ActivityData.ActivityDetails.WalkingDetail.Steps - steps_pause_t;
							Store_StepData(&tempsteps);
							
							//����洢
							Store_DistanceData(&ActivityData.ActivityDetails.WalkingDetail.Distance);
							
							//һ����ƽ���ٶ�
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
							
							//��·��
							ActivityData.ActivityDetails.WalkingDetail.Calorie = CalculateWalkEnergy(ActivityData.ActivityDetails.WalkingDetail.Distance/100,SetValue.AppSet.Weight);

							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.WalkingDetail.Calorie);
							
							//������٣�һ���ӻ�ȡһ�Σ���߾�ȷ��
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.WalkingDetail.OptimumSpeed);
							
							//һ����ƽ������
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
					
						//gps�洢
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
						
						//�Զ���Ȧʱ�� �켣�洢
						Store_AuotCirclle(ActivityData.ActivityDetails.WalkingDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwWalking);	
						
//						//��������
//						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwWalking);
						//��·������
						Remind_GoalKcal(ActivityData.ActivityDetails.WalkingDetail.Calorie/1000,SetValue.SportSet.DefWalkingFatBurnGoal,SetValue.SportSet.SwWalking);
						//Ŀ��ʱ������
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeWalking,SetValue.SportSet.SwWalking);

						//��Ȧ����
						if(Remind_CircleDistance(ActivityData.ActivityDetails.WalkingDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwWalking))
						{
						#if defined WATCH_SIM_SPORT
							//��һȦʱ��
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
							//���Զ���Ȧʱ����һȦʱ����ʾ��ʱ��
							gui_sport_walk_last_time_set(act_time);
						}

#endif						


					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
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
					//����
					ActivityData.ActivityDetails.RunningDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					
					//���������뵽�㷨��
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthMarathon,ActivityData.ActivityDetails.RunningDetail.Steps,ACT_MARATHON);
				
					//����
					ActivityData.ActivityDetails.RunningDetail.Pace = SpeedResult();
					
					//��Ƶ
				//	ActivityData.ActivityDetails.RunningDetail.frequency = get_steps_freq();
					
					if(PauseSta == false)
					{
						//�˶�״̬
						//����
						ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();
												
						//����                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
#if defined WATCH_SIM_SPORT
						//ƽ������
						ActivityData.ActivityDetails.WalkingDetail.AvgHeart = hdrtotal/hdrtcount;
						ave_frequency = CalculateFrequency(ActivityData.ActTime,ActivityData.ActivityDetails.RunningDetail.Steps -steps_pause_t );

#endif

						//�����˶����ݲ���
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{
							//�Ʋ��洢
							tempsteps = ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
							Store_StepData(&tempsteps);
							//����洢

							Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
							//һ����ƽ���ٶ�
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
							
							//��·��
							ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);
							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.RunningDetail.Calorie);
							
							//������٣�һ���ӻ�ȡһ�Σ���߾�ȷ��
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);
							
							//һ����ƽ������
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
					
						//gps�洢
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
						
						//�Զ���Ȧʱ�� �켣�洢
						Store_AuotCirclle(ActivityData.ActivityDetails.RunningDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwMarathon);	
						
						//��������
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwMarathon);
						//Ŀ���������
						Remind_GoalDistance(ActivityData.ActivityDetails.RunningDetail.Distance/100,SetValue.SportSet.DefHintDistanceMarathon,SetValue.SportSet.SwMarathon);
						//��������
						Remind_Pace(ActivityData.ActivityDetails.RunningDetail.Pace,SetValue.SportSet.DefHintSpeedMarathon,SetValue.SportSet.SwMarathon);
						//��Ȧ����
						if(Remind_CircleDistance(ActivityData.ActivityDetails.RunningDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwMarathon))
						{
							//������Ȧƽ������
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
							//���Զ���Ȧʱ����һȦʱ����ʾ��ʱ��
							gui_sport_marathon_last_time_set(act_time);
						}				
#endif
					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
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
						//�˶�״̬
						//����
						//ActivityData.ActivityDetails.SwimmingDetail.Strokes = SwimDataWaveCountResult();
						#ifdef COD
						algo_swim_strokes = SwimResultsOutputPort1_StrokeCount();
						#else
						ActivityData.ActivityDetails.SwimmingDetail.Strokes = SwimResultsOutputPort1_StrokeCount();
						#endif
                        //���� 
						if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_CONNECT)
						{
						    hdrvalue = Get_Sport_Heart();
						    CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
							
#if defined WATCH_SIM_SPORT
							//ƽ������
							ActivityData.ActivityDetails.SwimmingDetail.AvgHeart = hdrtotal/hdrtcount;
#endif 
						}
							
#ifndef COD
						//ƽ����Ƶ
						ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency_swimming(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
#endif 										
						//��ȡ����
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
						//����
						ActivityData.ActivityDetails.SwimmingDetail.laps = laps;
						//����
						ActivityData.ActivityDetails.SwimmingDetail.Strokes = 2*swim_store[Freestyle] + 2*swim_store[Backstroke]+ swim_store[Breaststroke]+ swim_store[ButterflyStroke]+ swim_store[UnknownStyle];
						//ƽ����Ƶ
						ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency_swimming(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);

						#if 0 //û�����ʴ洢
						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20��洢һ��
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
							//����
							ActivityData.ActivityDetails.SwimmingDetail.Distance = SetValue.AppGet.LengthSwimmingPool *100 *laps;

							//ƽ��Ӿ��
							ActivityData.ActivityDetails.SwimmingDetail.Speed = CalculateSpeed(act_time,0,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);

							//ƽ��swolf
							#ifdef COD
							//cod ��һ��swolf
							ActivityData.ActivityDetails.SwimmingDetail.Swolf = CalculateSwolf(ActivityData.ActivityDetails.SwimmingDetail.Strokes - last_lap_stroke,1,act_time -last_lap_time);
							last_lap_stroke = ActivityData.ActivityDetails.SwimmingDetail.Strokes;
							last_lap_time = act_time;
							#else
							ActivityData.ActivityDetails.SwimmingDetail.Swolf = CalculateSwolf(ActivityData.ActivityDetails.SwimmingDetail.Strokes,laps,act_time);
							#endif
							//��·��
							ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex);

//							//ƽ����Ƶ
//							ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);

							#ifndef COD
							//�����洢
							Store_StepData(&ActivityData.ActivityDetails.SwimmingDetail.Strokes);
							#endif

							//����洢
							Store_DistanceData(&ActivityData.ActivityDetails.SwimmingDetail.Distance);

							//��·��洢
							Store_EnergyData(& ActivityData.ActivityDetails.SwimmingDetail.Calorie);
							

							//��Ȧʱ��
							
#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
							Store_CircleTimeData(act_time ,SetValue.SportSet.SwSwimmg);
#endif
							
							//һ��ƽ���ٶ�
							speedminute = CalculateSpeed((act_time - Last_time),distancelast,ActivityData.ActivityDetails.SwimmingDetail.Distance,ACT_SWIMMING);
							distancelast = ActivityData.ActivityDetails.SwimmingDetail.Distance;
							Store_SpeedData(&speedminute);																											
#if defined WATCH_SIM_SPORT
							//����
							gui_sport_swim_laps_set(laps);
							//ƽ������
							gui_sport_swim_ave_stroke_set(ActivityData.ActivityDetails.SwimmingDetail.Strokes/laps);
#endif 
							#ifndef COD
							//һ��ƽ������													
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
						
                        //���ʸ澯����
						if(g_ble_accessory.heartrate.status == ACCESSORY_STATUS_CONNECT)
					    {
						    Remind_Heartrate(hdrvalue,SetValue.SportSet.SwSwimmg);
                        }

						//Ŀ���������
						Remind_GoalDistance(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,SetValue.SportSet.DefHintDistanceSwimmg,SetValue.SportSet.SwSwimmg);
						
						//Ŀ��ʱ������
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeSwimming,SetValue.SportSet.SwSwimmg);
						//Ŀ��Ȧ������
						Remind_GoalCircle(laps,SetValue.SportSet.DefHintCircleSwimming,SetValue.SportSet.SwSwimmg);
					}
					break;
				case ACT_INDOORRUN:
					//����
					ActivityData.ActivityDetails.RunningDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					
					//���������뵽�㷨��
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthIndoorRun,ActivityData.ActivityDetails.RunningDetail.Steps,ACT_INDOORRUN);
				
					//����
					ActivityData.ActivityDetails.RunningDetail.Pace = SpeedResult();
				
					//��Ƶ
				//	ActivityData.ActivityDetails.RunningDetail.frequency = get_steps_freq();
					
					if(PauseSta == false)
					{
						//�˶�״̬
						//����
						ActivityData.ActivityDetails.RunningDetail.Distance = DistanceResult();
												
						//����                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						
#if defined WATCH_SIM_SPORT
						//ƽ������
						ActivityData.ActivityDetails.RunningDetail.AvgHeart = hdrtotal/hdrtcount;
						ave_frequency = CalculateFrequency(ActivityData.ActTime,ActivityData.ActivityDetails.RunningDetail.Steps -steps_pause_t );
#endif

						//�����˶����ݲ���
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{
							//�Ʋ��洢
							tempsteps = ActivityData.ActivityDetails.RunningDetail.Steps - steps_pause_t;
							Store_StepData(&tempsteps);
							
							//����洢
							Store_DistanceData(&ActivityData.ActivityDetails.RunningDetail.Distance);
							
							//һ����ƽ���ٶ�
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
							
							//��·��
							ActivityData.ActivityDetails.RunningDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.RunningDetail.Distance/100,act_time,SetValue.AppSet.Weight);
							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.RunningDetail.Calorie);
							
							//������٣�һ���ӻ�ȡһ�Σ���߾�ȷ��
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.RunningDetail.OptimumPace);
							
							//һ����ƽ������
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
						
						//��������
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwIndoorRun);
					}
					break;
//Jason: Adding for handling cross-country running mode data 
				case ACT_CROSSCOUNTRY:
					//����
					ActivityData.ActivityDetails.CrosscountryDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					#ifdef COD 
					SetValue.run_step = last_run_step + ActivityData.ActivityDetails.CrosscountryDetail.Steps -steps_pause_t;
					#endif
				
					//���������뵽�㷨��
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthCountryRace,ActivityData.ActivityDetails.CrosscountryDetail.Steps,ACT_CROSSCOUNTRY);
					
					//����
					ActivityData.ActivityDetails.CrosscountryDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//�˶�״̬
						//static float sport_press,sport_alt,sport_temp;//��ѹ���߶�,�¶�
						
						//����
						ActivityData.ActivityDetails.CrosscountryDetail.Distance = DistanceResult();					
						
						//����                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						#ifdef COD 
						marathon_cal_time();
						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20��洢һ��
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
						//�������ݲ���
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{  
							//�Ʋ��洢
							tempsteps = ActivityData.ActivityDetails.CrosscountryDetail.Steps - steps_pause_t;
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&tempsteps);
							#endif

							//����洢
							Store_DistanceData(&ActivityData.ActivityDetails.CrosscountryDetail.Distance);

							//��·�����
							ActivityData.ActivityDetails.CrosscountryDetail.Calorie = CalculateRunningEnergy(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,act_time,SetValue.AppSet.Weight);
							
							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.RunningDetail.Calorie);
							
							//һ����ƽ���ٶ�
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
							//�������
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed);
							#ifndef COD
							//һ����ƽ������													
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount - hdrctlast);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							Store_HeartRateData(&hdrminute);
							//��ѹ�߶�
							task_sport_press();	
							#endif
						}
						
						//gps�洢
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
						//�Զ���Ȧʱ�� �켣�洢
						Store_AuotCirclle(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwCountryRace);	
						
						//��������
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwCountryRace);
						//Ŀ���������
						Remind_GoalDistance(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,SetValue.SportSet.DefHintDistanceCountryRace,SetValue.SportSet.SwCountryRace);
						//��������
						Remind_Pace(ActivityData.ActivityDetails.CrosscountryDetail.Speed,SetValue.SportSet.DefHintSpeedCountryRace,SetValue.SportSet.SwCountryRace);

						//Ŀ��ʱ������
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeCountryRace,SetValue.SportSet.SwCountryRace);
						
						//��Ȧ����
						if (Remind_CircleDistance(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwCountryRace))
						{

							#ifdef COD 
							last_circle_pace = CalculateSpeed(act_time,0,ActivityData.CircleDistance*100,ACT_CROSSCOUNTRY);
							#else
							//������Ȧƽ������
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
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
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
					//����
					ActivityData.ActivityDetails.CrosscountryDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
					#ifdef COD 
					SetValue.walk_step = last_walk_step + ActivityData.ActivityDetails.CrosscountryDetail.Steps -steps_pause_t;
					#endif
					//���������뵽�㷨��
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthHike,ActivityData.ActivityDetails.CrosscountryDetail.Steps,ACT_HIKING);
					
					//����
					ActivityData.ActivityDetails.CrosscountryDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//�˶�״̬
						//����
						ActivityData.ActivityDetails.CrosscountryDetail.Distance = DistanceResult();
					
						
						//����                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						#ifdef COD 
						marathon_cal_time();

						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20��洢һ��
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
						//�����˶����ݲ���
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{                
							//�Ʋ��洢
							tempsteps = ActivityData.ActivityDetails.CrosscountryDetail.Steps - steps_pause_t;
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&tempsteps);
							#endif
							//����洢
							Store_DistanceData(&ActivityData.ActivityDetails.CrosscountryDetail.Distance);
							//һ����ƽ���ٶ�
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
							
							//��·��
							ActivityData.ActivityDetails.CrosscountryDetail.Calorie = CalculateWalkEnergy(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,SetValue.AppSet.Weight);
							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.CrosscountryDetail.Calorie);
							#ifndef COD 
							//һ����ƽ������
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							//��ѹ�߶�
							task_sport_press();	
							#endif
							//�������
							CalculateOptimumPace(speedminute,&ActivityData.ActivityDetails.CrosscountryDetail.OptimumSpeed);							
							
							
						}
						//gps�洢
						if (GetGpsStatus())
						{
							store_gps_nums++;
						
						}
						//�Զ���Ȧʱ�� �켣�洢
						Store_AuotCirclle(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwHike);	
					 
						//��������
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwHike);
						
						#ifndef COD 
						//��·������
						Remind_GoalKcal(ActivityData.ActivityDetails.CrosscountryDetail.Calorie/1000,SetValue.SportSet.DefHikeFatBurnGoal,SetValue.SportSet.SwHike);
						#endif

						//Ŀ���������
						Remind_GoalDistance(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,SetValue.SportSet.DefHintDistanceHike,SetValue.SportSet.SwHike);

						//��������
						Remind_Pace(ActivityData.ActivityDetails.CrosscountryDetail.Speed,SetValue.SportSet.DefHintSpeedHike,SetValue.SportSet.SwHike);

						//Ŀ��ʱ������
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeHike,SetValue.SportSet.SwHike);
					
						//��Ȧ����
						if (Remind_CircleDistance(ActivityData.ActivityDetails.CrosscountryDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwHike))
						{
							#ifdef COD 
							last_circle_pace = CalculateSpeed(act_time,0,ActivityData.CircleDistance*100,ACT_HIKING);
							#else
							//������Ȧƽ������
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
					
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
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
					//����
					ActivityData.ActivityDetails.ClimbingDetail.Steps = drv_lsm6dsl_get_stepcount() - begin_steps - steps_record_t;
#if defined WATCH_COM_SPORT
					#ifndef COD
			  		if(PauseSta == false && (ActivityData.Act_Stop_Time.Second %5) == 0) 
					{
					
							//��ѹ�߶�
							task_climbing_press();
							
					}
					#endif
#else
				  if(PauseSta == false && ActivityData.Act_Stop_Time.Second  == 0) 
					{
							//��ѹ�߶�
							task_sport_press();	
					}
#endif				  
					//���������뵽�㷨��
					sport_data_input(pauseswitch,SetValue.AppGet.StepLengthClimbing,ActivityData.ActivityDetails.ClimbingDetail.Steps,ACT_CLIMB);
					
					//����
					ActivityData.ActivityDetails.ClimbingDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//�˶�״̬
						//����
						ActivityData.ActivityDetails.ClimbingDetail.Distance = DistanceResult();
						
						//����                
						hdrvalue = Get_Sport_Heart();
						#ifdef COD 
						CalculateHeartrateMax(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						marathon_cal_time();

						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20��洢һ��
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

						//������ʼ�¼
						if(hdrvalue > ActivityData.ActivityDetails.ClimbingDetail.MaxHeart)
						{							
							ActivityData.ActivityDetails.ClimbingDetail.MaxHeart = hdrvalue;
						}

						//�����˶����ݲ���
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{
							//�Ʋ��洢
							tempsteps = ActivityData.ActivityDetails.ClimbingDetail.Steps - steps_pause_t;
							#ifdef COD
							sport_total_time = PauseTotolTime +g_SportTimeCount;
							Store_cod_StepData(&tempsteps,&sport_total_time);
							#else
							Store_StepData(&tempsteps);
							#endif
							//����洢
							Store_DistanceData(&ActivityData.ActivityDetails.ClimbingDetail.Distance);
							#ifndef COD 
							//һ����ƽ������
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							ActivityData.ActivityDetails.ClimbingDetail.AvgHeart = hdrminute;
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							#else
							//cod �������
							speedminute =  CalculateSpeed(60,last_distance,ActivityData.ActivityDetails.ClimbingDetail.Distance,ACT_CYCLING);  //�ٶ�10*km/h
							last_distance = ActivityData.ActivityDetails.ClimbingDetail.Distance;	
							CalculateOptimumSpeed(speedminute,&ActivityData.ActivityDetails.ClimbingDetail.OptimumSpeed);
							#endif
							//��·��
							ActivityData.ActivityDetails.ClimbingDetail.Calorie += CalculateMountaineeringEnergy(hdrminute,60,SetValue.AppSet.Weight,SetValue.AppSet.Age,SetValue.AppSet.Sex);
							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.ClimbingDetail.Calorie);
							
							//��ѹ�߶�
							//task_sport_press();	
							
							//һ����ƽ���ٶ�
							#if 0
							speedminute = CalculateSpeed(60,distancelast,ActivityData.ActivityDetails.ClimbingDetail.DropHeight 
																			+ ActivityData.ActivityDetails.ClimbingDetail.RiseHeight,ACT_CLIMB);
							#endif
							speedminute = MeanSpeedResult();
							Store_SpeedData(&speedminute);
							distancelast = ActivityData.ActivityDetails.ClimbingDetail.DropHeight 
												+ ActivityData.ActivityDetails.ClimbingDetail.RiseHeight;
						}
				  
						//gps�洢
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
						
						//�Զ���Ȧʱ�䣬�켣�洢
						Store_AuotCirclle(ActivityData.ActivityDetails.ClimbingDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwClimbing);
						
						//��������
						Remind_Heartrate(hdrvalue,SetValue.SportSet.SwClimbing);
						
						//Ŀ���������
						Remind_GoalDistance(ActivityData.ActivityDetails.ClimbingDetail.Distance/100,SetValue.SportSet.DefHintDistanceClimbing,SetValue.SportSet.SwClimbing);

						//Ŀ��ʱ������
						Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeClimbing,SetValue.SportSet.SwClimbing);
							
						//�߶�����
						#ifdef COD 
						Remind_Altitude(sport_altitude,SetValue.SportSet.DefHintClimbingHeight,SetValue.SportSet.SwClimbing);
						#else
						Remind_Altitude(sport_alt,SetValue.SportSet.DefHintClimbingHeight,SetValue.SportSet.SwClimbing);
						#endif

						//��Ȧ����
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
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
					{
					    
						//pauseswitch��ͣ״̬(����),sport_altitudeʵʱ���� ��sport_RiseHeight�ۼ�����(����) ��sport_Risestate����״̬ ,sport_RiseAlt��������
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
					//���������뵽�㷨��
					sport_data_input(pauseswitch,0,ActivityData.ActivityDetails.CyclingDetail.Cadence,ACT_CYCLING);
					
#if 0
					test_number++;
					test_number = test_number%test_num;
#endif
					//����
					ActivityData.ActivityDetails.CyclingDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//�˶�״̬
						//̤Ƶ
						ActivityData.ActivityDetails.CyclingDetail.Cadence = sport_cycling_cadence();
						CalculateCadenceParam(ActivityData.ActivityDetails.CyclingDetail.Cadence,&cadtotal,&cadcount);
						
						//����
						ActivityData.ActivityDetails.CyclingDetail.Distance = DistanceResult();
						
						//����                
						hdrvalue = Get_Sport_Heart();
						
						#ifdef COD 
					
						CalculateHeartrateMax(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						if(ActivityData.Act_Stop_Time.Second % 20 == 0) //20��洢һ��
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
						//ƽ������
						ActivityData.ActivityDetails.CyclingDetail.AvgHeart = hdrtotal/hdrtcount;
#endif
						//������ʼ�¼
						if(hdrvalue > ActivityData.ActivityDetails.CyclingDetail.MaxHeart)
						{						
							ActivityData.ActivityDetails.CyclingDetail.MaxHeart = hdrvalue;
						}

						//�����˶����ݲ���
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{
							//̤Ƶ�洢
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
							//����洢
							Store_DistanceData(&ActivityData.ActivityDetails.CyclingDetail.Distance);
							
							//һ����ƽ���ٶ�
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

							
							

							
							
							/*��·��ݶ�һ���Ӽ���һ��,����distance���룬��λ  �ף�����100*/
							ActivityData.ActivityDetails.CyclingDetail.Calorie += CalculateCyclingEnergy(speedminute*100,60,SetValue.AppSet.Weight);
							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.CyclingDetail.Calorie);
							#ifndef COD 
							//һ����ƽ������
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
							#endif 
							ActivityData.ActivityDetails.CyclingDetail.AvgHeart = hdrminute;
							//��ѹ�߶�
							#ifndef COD 
							task_sport_press();
							#endif
						}
					  
						//gps�洢
						if (GetGpsStatus())
						{
							store_gps_nums++;
						
						}
						
						//���������ݲ���Ҫ����
						if(ActivityData.Activity_Type != ACT_TRIATHLON_CYCLING)
						{
							//�Զ���Ȧʱ�䣬�켣�洢
							Store_AuotCirclle(ActivityData.ActivityDetails.CyclingDetail.Distance/100,ActivityData.CircleDistance,act_time,SetValue.SportSet.SwCycling);

							//��������
							Remind_Heartrate(hdrvalue,SetValue.SportSet.SwCycling);
							//Ŀ���������
							Remind_GoalDistance(ActivityData.ActivityDetails.CyclingDetail.Distance/100,SetValue.SportSet.DefHintDistanceCycling,SetValue.SportSet.SwCycling);

							//�ٶ�����
							Remind_Speed(ActivityData.ActivityDetails.CyclingDetail.Speed,SetValue.SportSet.DefHintSpeedCycling,SetValue.SportSet.SwCycling);

								//Ŀ��ʱ������
							Remind_GoalTime(act_time,SetValue.SportSet.DefHintTimeCycling,SetValue.SportSet.SwCycling);
								
						//��Ȧ����
						if (Remind_CircleDistance(ActivityData.ActivityDetails.CyclingDetail.Distance/100,ActivityData.CircleDistance,SetValue.SportSet.SwCycling))
						{
							#ifdef COD 
							last_circle_pace = CalculateSpeed(act_time,0,ActivityData.CircleDistance*100,ACT_CYCLING);
							#else
							//������Ȧƽ������
							last_circle_pace = CalculateSpeed(act_time - last_circle_time,0,ActivityData.CircleDistance*100,ACT_CYCLING);
							#endif
							#if defined WATCH_SIM_SPORT
							//��һȦʱ��
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
								//���Զ���Ȧʱ����һȦʱ����ʾ��ʱ��
								gui_sport_cycling_last_time_set(act_time);
							}
#endif


						}
					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
						ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
						
						ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
						
						CalculateMaxAlt(sport_altitude,&ActivityData.ActivityDetails.CyclingDetail.MaxAlt);

						ActivityData.ActivityDetails.CyclingDetail.RiseAlt = sport_RiseAlt;

						//�������
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
					//���������뵽�㷨��
					sport_data_input(pauseswitch,0,ActivityData.ActivityDetails.SwimmingDetail.Strokes,ACT_TRIATHLON_SWIM);
					
					//Ӿ��
					ActivityData.ActivityDetails.SwimmingDetail.Speed = SpeedResult();
					
					if(PauseSta == false)
					{
						//�˶�״̬
						//����
						//ActivityData.ActivityDetails.SwimmingDetail.Strokes = SwimDataWaveCountResult();
						ActivityData.ActivityDetails.SwimmingDetail.Strokes = SwimResultsOutputPort1_StrokeCount();
						
						//ƽ����Ƶ
						//ActivityData.ActivityDetails.SwimmingDetail.Frequency = CalculateFrequency(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
						ActivityData.ActivityDetails.SwimmingDetail.Frequency  = CalculateFrequency_swimming(act_time,ActivityData.ActivityDetails.SwimmingDetail.Strokes);
						
						//����
						ActivityData.ActivityDetails.SwimmingDetail.Distance = DistanceResult();
						
						//����                
						hdrvalue = Get_Sport_Heart();
						CalculateHeartrateParam(hdrvalue,&ActivityData.ActivityDetails.CommonDetail,&hdrtotal,&hdrtcount);
						
						//������ʼ�¼
						if(hdrvalue > ActivityData.ActivityDetails.SwimmingDetail.MaxHeart)
						{						
							ActivityData.ActivityDetails.SwimmingDetail.MaxHeart = hdrvalue;
						}
					
						if(ActivityData.Act_Stop_Time.Second == 0) //һ���Ӵ洢һ��
						{
							//�����洢
							Store_StepData(&ActivityData.ActivityDetails.SwimmingDetail.Strokes);
							
							//����洢
							Store_DistanceData(&ActivityData.ActivityDetails.SwimmingDetail.Distance);
							
							//һ����ƽ���ٶ�
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
							
							//��·��
							ActivityData.ActivityDetails.SwimmingDetail.Calorie = CalculateSwimmingEnergy(ActivityData.ActivityDetails.SwimmingDetail.Distance/100,act_time,SetValue.AppSet.Sex);
							//��·��洢
							Store_EnergyData(&ActivityData.ActivityDetails.SwimmingDetail.Calorie);
							
							//һ����ƽ������
							hdrminute = (hdrtotal - hdrlast)/(hdrtcount -hdrctlast);
							ActivityData.ActivityDetails.SwimmingDetail.AvgHeart = hdrminute;
							Store_HeartRateData(&hdrminute);
							hdrlast = hdrtotal;
							hdrctlast = hdrtcount;
						}
						
						//GPS����
						if (GetGpsStatus())
						{
							store_gps_nums++;

						}
					}
					#ifdef COD 
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
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
					if((PauseTotolTime +g_SportTimeCount) % 2 == 1) //2��洢һ��
					{
						cod_store_gpsdata(pauseswitch,&sport_altitude,&sport_RiseHeight,&sport_risedrop,&sport_Risestate,&sport_RiseAlt,&ActivityData);
												ActivityData.ActivityDetails.CommonDetail.RiseHeight = sport_RiseHeight;
												
												ActivityData.ActivityDetails.CommonDetail.DropHeight = sport_risedrop;
					}
					#else
					if(PauseSta == false)
					{
						//�˶�״̬
						//���滻��켣
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
									Last_GPS_Lon =(int32_t )(dLatLng.dlon*100000);//�켣��gps�㱣��5λС���㣬�򻯼���������λ����Ϊ�׼�
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
				//ˢ��
				gui_swich_msg();
			}
		}
	}
}



