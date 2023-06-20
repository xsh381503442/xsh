/*
接口函数：
初始化void TrackOptimizingInit(float Lon, float Lat)
数据接收函数void TrackOptimizingDataAccept(DistanceInputStr DistanceInput)
轨迹点输出函数OutputTrackPointsStr TrackPointsResult(void);
里程输出函数uint32_t DistanceResult(void);
瞬时速度输出函数uint16_t SpeedResult(void);
均值速度输出函数uint16_t MeanSpeedResult(void);

步长校准后步长输出函数uint32_t StepStrideResult(void);

计算体能参数算法的初始化函数void PhysicalParamCalculateInit(void);
计算体能参数算法的数据接收函数void PhysicalParamCalDataAccept(int32_t Lon, int32_t Lat, uint32_t steps, uint32_t stepdiff);
*/

#include "algo_trackoptimizing.h"
#include "algo_config.h"
#include "drv_ublox.h"

#include "algo_step_count.h"

#if DEBUG_ENABLED == 1 && ALGO_TRACKOPTIMIZING_LOG_ENABLED == 1
	#define ALGO_TRACKOPTIMIZING_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ALGO_TRACKOPTIMIZING_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ALGO_TRACKOPTIMIZING_WRITESTRING(...)
	#define ALGO_TRACKOPTIMIZING_PRINTF(...)		        
#endif

extern GPSMode m_GPS_data;		//GPS数据

extern DistanceInputStr DistanceInput;


TrackOptimizingResultDataStr TrackOptimizingResultData ; //通过轨迹优化后的最终结果数据，有单个的经纬度和各种状态下的距离

instant_speed_t instant_speed; //计算瞬时速度的存储结构


#if defined COD

uint8_t del_gps_point_num = 0;
gps_point_str kf_last_gps_point,kf_pre_gps_point,kf_cur_gps_point;

#endif


float single_gpsdis_limit;    
uint8_t sport_status_flag = SPORT_FLAG_MOVE;  //运动状态标记(静止或者运动)

DistanceInputStr g_DistanceInput; //算法的输入数据，是个包含了经纬度、GPS状态、时间、暂停状态等的结构体

float g_stepthreshold;  //步数连续相同持续的时间，用于步行类判断静止的一个重要参数

float g_Speed;  //每秒速度

float per_sec_speed[60];
uint32_t g_speed_count;	//一分钟内累加的速度个数，防止暂停状态误算速度个数

const float para_mean[5] = {0.25f,0.30f,0.2f,0.15f,0.1f};

const float para_share_speed[5] = {0.5f,0.45f,0.4f,0.35f,0.3f};
StepsCompStr StepsComp;  //保存步数的存储结构，STEPSCOMP_SIZE是存储数据大小的上限值


int start_step_single[2] = {0};
float g_LastInstantSpeed; //前一秒的瞬时速度，用于稳定现在的瞬时速度

uint32_t g_FStime,g_LStime,g_FSup_value,g_LSup_value,g_FSdown_value,g_LSdown_value; //用于计算登山的一分钟平均速度
float g_InstantSpeedSum;  //用于除了登山运动的其他运动的分钟平均速度的计算
float g_minuMeanSpeed;  //一分钟平均速度
uint8_t g_minuSpeedNum;  //用于计算分钟平均数

uint8_t g_zeroSpeedCount; //骑行瞬时速度连续为0的计数

 
int32_t g_StepRecord; 
float g_stepstride; //the stride of calibrarion

uint16_t inst_step_freq; //实时步频
step_queue step_queue_data; //实时步频计算数组

uint16_t step_queue_temp_save_data[2];  //实时步频的暂存数组


float persec_step[PERSEC_STEP_SIZE] = {0};//每秒中的步频，开启运动后初始化为0
float mean_persec_step = 0;
uint8_t persec_step_point = 0;
uint8_t record_step_point = 0;
uint32_t last_step_num_sport = 0;



uint8_t g_minispeed_count;	//速度结果值出现0的数目

uint8_t g_NoGPS_flag;	//上一秒是否掉星标志(0:没有掉，1:掉星)

uint8_t g_firstGPS_flag=1;	//第一个gps的标志符，当是第一个gps时标志符为1，反之为0

extern uint8_t combo_speed_step ;//由加速度计识别出的运动状态标志

extern step_data  step_data_str;//运动计步结构体

uint8_t test_gps_flag = 1;
float ldv_oidpast = 0;	//基点与当前点的距离

uint32_t no_gps_num;	//掉星总数
uint32_t step_gps_exceed_num;	//步行类超过总数
uint32_t cycling_gps_exceed_num;	//步行类超过总数

extern uint8_t start_point_num;
extern GpspointStr pre_gps_point[OUTPUT_DATA_SUM_PRE];
extern uint32_t pre_distance[OUTPUT_DATA_SUM_PRE];

gps_data_rn gps_rawdata_rn;
gps_data_sm gps_rndata_sm,gps_data_get_sm;
gps_data_use_for_rn gps_point_pre, gps_point_cur, gps_point_next;
gps_data_out gps_data_output;

gps_data_distance gps_data_distance_sm;


/***************************卡尔曼滤波开始********************************/ 
double lastLocation_x; //上次位置
double currentLocation_x;//这次位置
double lastLocation_y; //上次位置
double currentLocation_y;//这次位置 
double estimate_x; //修正后数据 
double estimate_y; //修正后数据 
double pdelt_x; //自预估偏差 
double pdelt_y; //自预估偏差 
double mdelt_x; //上次模型偏差
double mdelt_y; //上次模型偏差
double gauss_x; //高斯噪音偏差
double gauss_y; //高斯噪音偏差
double kalmanGain_x; //卡尔曼增益
double kalmanGain_y; //卡尔曼增益
double m_R = 0;
double m_Q = 0;



void track_bubble_sort(float *ac, uint16_t n) 
{
    int i, j;
    float temp;
    for (j = 0; j < n - 1; j++)
    {
       for (i = 0; i < n - 1 - j; i++)
       {
          if(ac[i] > ac[i + 1])
          {
             temp=ac[i];
             ac[i]=ac[i+1]; 
             ac[i+1]=temp;
          }
       }
    }
}


/*
利用半正矢算法计算GPS的距离
*/
float GpsDistance(int32_t  LonA, int32_t  LatA, int32_t  LonB, int32_t  LatB)
{
    float distance = 0;
    double  lona = LonA / GPS_SCALE_DECIMAL;
    double  lata = LatA / GPS_SCALE_DECIMAL;
    double  lonb = LonB / GPS_SCALE_DECIMAL;
    double  latb = LatB / GPS_SCALE_DECIMAL;
    double	lv_a,lv_b;
    if ((lona == lonb) && (lata == latb))
    {
        distance = 0;
    }
	else if((LonA==0)||(LatA==0)||(LonB==0)||(LatB==0))
		{
         
		 distance = 0;

	    }
    else
    {
        lv_a= cos(lata * PI / 180) * cos(latb* PI / 180) * sin((lona - lonb)  * PI / 360) * sin((lona - lonb)  * PI / 360)
            + sin((lata-latb)  * PI / 360) * sin((lata-latb)  * PI / 360);
				lv_b = sqrt(1-lv_a);
				lv_a = sqrt(lv_a);
        distance = R_EARTH *(2*atan2(lv_a,lv_b)); 
    }

    return  distance;
}
 

/*
利用半正矢算法计算GPS的距离
*/
/*float gps_distance_f(double  LonA, double  LatA, double  LonB, double  LatB)
{
    double distance = 0;
    double  lona = LonA;
    double  lata = LatA;
    double  lonb = LonB;
    double  latb = LatB;

    if ((lona == lonb) && (lata == latb))
    {
        distance = 0;
    }
    else
    {
        double	lv_a,lv_b;
        lv_a= cos(lata * PI / 180) * cos(latb* PI / 180) * sin((lona - lonb)  * PI / 360) * sin((lona - lonb)  * PI / 360)
            + sin((lata-latb)  * PI / 360) * sin((lata-latb)  * PI / 360);
				lv_b = sqrt(1-lv_a);
				lv_a = sqrt(lv_a);
        distance = R_EARTH *(2*atan2(lv_a,lv_b)); 
    }

    return  (float)distance;
}*/




/*
计算体能参数算法：计算乳酸阈、最大摄氧量需要速度、里程，运用于特定跑步运动的速度、里程的计算。
下面是该算法的初始化函数。
*/
void PhysicalParamCalculateInit()
{
	
    memset(&gps_rawdata_rn, 0, sizeof(gps_rawdata_rn));
	memset(&gps_rndata_sm, 0, sizeof(gps_rndata_sm));
	memset(&TrackOptimizingResultData, 0, sizeof(TrackOptimizingResultData));
    memset(&instant_speed, 0, sizeof(instant_speed));
   // memset(&g_Speed, 0, sizeof(g_Speed));
     g_Speed = 0;
    memset(&g_DistanceInput, 0, sizeof(g_DistanceInput));
	TrackOptimizingResultData.Longitude = 0;
	TrackOptimizingResultData.Latitude = 0;
	g_DistanceInput.activity_type = ACT_RUN;
	g_DistanceInput.gps_status = 1;
	g_LastInstantSpeed = 0;
	g_zeroSpeedCount = 0;
	g_NoGPS_flag = 1;
	ldv_oidpast = 0;
	no_gps_num = 0;
	start_point_num = 0;
	step_gps_exceed_num = 0;
	cycling_gps_exceed_num = 0;
	//memset(&g_StepRecord, 0, sizeof(g_StepRecord));
	g_StepRecord = 0;
	g_stepstride = 0;
	
}
/*
体能参数计算的数据接收函数
*/
void PhysicalParamCalDataAccept(int32_t Lon, int32_t Lat, uint32_t steps, uint32_t stepdiff)
{
	g_DistanceInput.lon = Lon;
	g_DistanceInput.lat = Lat;
	

	if (g_DistanceInput.lon != 0 && g_DistanceInput.lat != 0)
	{
		if (TrackOptimizingResultData.Longitude == 0 && TrackOptimizingResultData.Latitude == 0)
		{
			TrackOptimizingResultData.Longitude = g_DistanceInput.lon;
			TrackOptimizingResultData.Latitude = g_DistanceInput.lat;
		}
	}
		
	g_StepRecord = steps - stepdiff;
	PhyParamCalculateDistSpeed();
}

void PhyParamCalculateDistSpeed()
{
	

	 gps_raw_data_rn_receive();
     SpeedOptimizingDataCollection(g_DistanceInput.lon,g_DistanceInput.lat);
	 gps_rn_data_sm_receive();
	 gps_data_distance_sm.sdistance_gps = gps_sm_data_distance_receive();
    if(gps_data_get_sm.data_tsum<SMOOTH_DATA_TSUM)
	{
		g_stepstride = 0;
	}
	else
	{
		if (g_StepRecord>0)
		{
			g_stepstride = gps_data_distance_sm.distance_sum/g_StepRecord;
		}
	}
	g_Speed = instant_speed.speed;
	
			
}

/*
初始化函数
*/
void TrackOptimizingInit()
{

	memset(&gps_rawdata_rn, 0, sizeof(gps_rawdata_rn));
    memset(&gps_rndata_sm, 0, sizeof(gps_rndata_sm));
    memset(&gps_data_get_sm, 0, sizeof(gps_data_get_sm));
	memset(&gps_data_output, 0, sizeof(gps_data_output));
    memset(&gps_data_distance_sm, 0, sizeof(gps_data_distance_sm));
    memset(&start_step_single, 0, sizeof(int)*2);
    memset(&kf_last_gps_point, 0, sizeof(kf_last_gps_point));
    memset(&TrackOptimizingResultData, 0, sizeof(TrackOptimizingResultData));
    memset(&instant_speed, 0, sizeof(instant_speed));
    memset(&StepsComp, 0, sizeof(StepsComp));
    memset(&g_DistanceInput, 0, sizeof(g_DistanceInput));
	memset(&step_queue_data, 0, sizeof(step_queue_data));
	memset(&step_queue_temp_save_data, 0, sizeof(step_queue_temp_save_data));
	memset(&per_sec_speed, 0, sizeof(per_sec_speed));
    g_Speed = 0;
	g_stepthreshold = 0;
	g_LastInstantSpeed = 0;
	g_FStime = 0;
	g_LStime = 0;
	g_FSup_value = 0;
	g_LSup_value = 0;
	g_FSdown_value = 0;
	g_LSdown_value = 0;
	g_InstantSpeedSum = 0;
	g_minuMeanSpeed = 0;
	g_minuSpeedNum = 0;
	g_zeroSpeedCount = 0;
	inst_step_freq = 0; 
	persec_step_point = 0;
	record_step_point = 0;
	last_step_num_sport = 0;
	g_minispeed_count = 0;
	g_NoGPS_flag = 1;
	g_firstGPS_flag = 1;
	g_speed_count = 0;
	ldv_oidpast = 0;
	no_gps_num = 0;
	start_point_num = 0;
	step_gps_exceed_num = 0;
	cycling_gps_exceed_num = 0;
	
#if 0
	test_number = 0;
	#endif
	//设置原始数据存储类型
  #if defined STORE_ORG_DATA_TEST_VERSION
    Set_StoreData_Flag(1);
  #endif	
}
//初始模型 
void kf_initial(void)
{	
 pdelt_x = 0.001;	
 pdelt_y = 0.001;	
 mdelt_x = 5.698402909980532E-4;
 mdelt_y = 5.698402909980532E-4;
}

gps_point_str kalmanFilter(double oldValue_x,double value_x,double oldValue_y,double value_y) 
	{	 
	gps_point_str latlng;	
	lastLocation_x = oldValue_x;//(1)	
	currentLocation_x = value_x;	
	gauss_x = sqrt(pdelt_x * pdelt_x + mdelt_x * mdelt_x) + m_Q;     //计算高斯噪音偏差(2)	
	kalmanGain_x = sqrt((gauss_x * gauss_x) / (gauss_x * gauss_x + pdelt_x * pdelt_x)) + m_R; //计算卡尔曼增益(3)	
	estimate_x = kalmanGain_x * (currentLocation_x - lastLocation_x) + lastLocation_x;    //修正定位点(4)	
	mdelt_x = sqrt((1 - kalmanGain_x) * gauss_x * gauss_x);      //修正模型偏差	(5)

	lastLocation_y = oldValue_y;	
	currentLocation_y = value_y;	
	gauss_y = sqrt(pdelt_y * pdelt_y + mdelt_y * mdelt_y) + m_Q;     //计算高斯噪音偏差	
	kalmanGain_y = sqrt((gauss_y * gauss_y) / (gauss_y * gauss_y + pdelt_y * pdelt_y)) + m_R; //计算卡尔曼增益	
	estimate_y = kalmanGain_y * (currentLocation_y - lastLocation_y) + lastLocation_y;    //修正定位点	
	mdelt_y = sqrt((1 - kalmanGain_y) * gauss_y * gauss_y);      //修正模型偏差	 
	latlng.Lon = estimate_x;	  
	latlng.Lat = estimate_y;	
	return latlng; 
	}
gps_point_str kalmanFilterPoint(gps_point_str lastLoc, gps_point_str curLoc, int intensity)
	 {
		 gps_point_str kalmanLatlng;
		 gps_point_str curLoc_data;
		 curLoc_data  = curLoc;
		 if (pdelt_x == 0 || pdelt_y == 0)	 
		 	{		
		 	kf_initial(); 
			}
		 for (int j = 0; j < intensity; j++)	
		 	{		
		 	kalmanLatlng = kalmanFilter(lastLoc.Lon, curLoc_data.Lon, lastLoc.Lat, curLoc_data.Lat);		
			curLoc_data = kalmanLatlng;			
			}
	
		 return kalmanLatlng;
	
	 
	 }

void kalmanfilter_path(gps_data_sm*gps_data_ori,uint8_t data_num)
{
uint8_t start_seq,i;
  gps_point_str temp_kfgps_lonlat;

  if((kf_last_gps_point.Lon==0)&&(kf_last_gps_point.Lat==0))
  	{
     kf_pre_gps_point.Lon = gps_data_ori->data[0][0];
   
     kf_pre_gps_point.Lat= gps_data_ori->data[0][1];

	 
	 kf_last_gps_point.Lon = gps_data_ori->data[0][0];
	 kf_last_gps_point.Lat = gps_data_ori->data[0][1];
    start_seq = 1;
  	}
  else
  	{
       kf_pre_gps_point.Lon = kf_last_gps_point.Lon;
   
       kf_pre_gps_point.Lat= kf_last_gps_point.Lat;
      
	  start_seq = 0;
    }


for(i=start_seq;i<data_num;i++)
   {

    
       kf_cur_gps_point.Lon = gps_data_ori->data[i][0];
   
       kf_cur_gps_point.Lat = gps_data_ori->data[i][1];

	temp_kfgps_lonlat =  kalmanFilterPoint(kf_pre_gps_point, kf_cur_gps_point, 5);

	//gps_data_ori->data[i][0] = (int32_t)(temp_kfgps_lonlat.Lon*100000);
	
	//gps_data_ori->data[i][1] = (int32_t)(temp_kfgps_lonlat.Lat*100000);

	gps_data_ori->data[i][0] = temp_kfgps_lonlat.Lon;
	
	gps_data_ori->data[i][1] = temp_kfgps_lonlat.Lat;

	  kf_pre_gps_point.Lon = temp_kfgps_lonlat.Lon;
   
     kf_pre_gps_point.Lat = temp_kfgps_lonlat.Lat;


	 kf_last_gps_point.Lon = temp_kfgps_lonlat.Lon;
     kf_last_gps_point.Lat = temp_kfgps_lonlat.Lat;

  } 
  




}

float calculateDistanceFromPoint(gps_data_use_for_rn pre, gps_data_use_for_rn cur,gps_data_use_for_rn next)
{
    float dis_point;
	double lona,lata,lonb,latb,lv_a,lv_b;
	double A = cur.data_lon - pre.data_lon;
	double B = cur.data_lat - pre.data_lat;
	double C = next.data_lon - pre.data_lon;
	double D = next.data_lat - pre.data_lat;

	double dot = A * C + B * D;
	double len_sq = C * C + D * D;
	double param = dot / len_sq;

	double xx, yy;
	
	
	if (param < 0 || (pre.data_lon == next.data_lon&& pre.data_lat == next.data_lat))
	{
		xx = pre.data_lon;
		yy = pre.data_lat;
		
	}
	else if (param > 1)
	{
		xx = next.data_lon;
		yy = next.data_lat;
		
	}
	else
	{
		xx = pre.data_lon + param * C;
		yy = pre.data_lat + param * D;
	}
	
	lona = cur.data_lon;
    lata = cur.data_lat;
    lonb = xx;
    latb = yy;
	lv_a = cos(lata * PI / 180) * cos(latb* PI / 180) * sin((lona - lonb)  * PI / 360) * sin((lona - lonb)  * PI / 360)  + sin((lata-latb) * PI / 360) * sin((lata-latb) * PI / 360);
	lv_b = sqrt(1-lv_a);
	lv_a = sqrt(lv_a);
    dis_point = R_EARTH *(2*atan2(lv_a,lv_b));

	return dis_point;
}

float gps_raw_data_process_rn(gps_data_rn*data_rn)
 {
    float dis_temp;
	gps_point_pre.data_lon = data_rn->data[0][0];
    gps_point_pre.data_lat = data_rn->data[0][1];

	gps_point_cur.data_lon = data_rn->data[1][0];
    gps_point_cur.data_lat = data_rn->data[1][1];

	
	gps_point_next.data_lon = data_rn->data[2][0];
    gps_point_next.data_lat = data_rn->data[2][1];

	dis_temp = calculateDistanceFromPoint(gps_point_pre,gps_point_cur,gps_point_next);
	return dis_temp;
   
 }
void gps_raw_data_rn_receive()
{
 float rn_distance;


 
 if(gps_rawdata_rn.data_sum<REMOVE_NOISE_DATA_SUM)
 	{
		gps_rawdata_rn.data[gps_rawdata_rn.data_number][0] = g_DistanceInput.lon/GPS_SCALE_DECIMAL;
		gps_rawdata_rn.data[gps_rawdata_rn.data_number][1] = g_DistanceInput.lat/GPS_SCALE_DECIMAL;
		gps_rawdata_rn.pausestatus[gps_rawdata_rn.data_number]	= g_DistanceInput.pause_status;

		gps_rawdata_rn.data_number++;
		gps_rawdata_rn.data_sum++;

	
 	}
 else
 	{
         
         gps_rawdata_rn.data[0][0] = gps_rawdata_rn.data[1][0];
	     gps_rawdata_rn.data[0][1] = gps_rawdata_rn.data[1][1];
	     gps_rawdata_rn.pausestatus[0] = gps_rawdata_rn.pausestatus[1];

		 gps_rawdata_rn.data[1][0] = gps_rawdata_rn.data[2][0];
	     gps_rawdata_rn.data[1][1] = gps_rawdata_rn.data[2][1];
	     gps_rawdata_rn.pausestatus[1] = gps_rawdata_rn.pausestatus[2];

		 gps_rawdata_rn.data[2][0] = g_DistanceInput.lon/GPS_SCALE_DECIMAL;
	     gps_rawdata_rn.data[2][1] = g_DistanceInput.lat/GPS_SCALE_DECIMAL;
	     gps_rawdata_rn.pausestatus[2]= g_DistanceInput.pause_status;

	
    }

    if(gps_rawdata_rn.data_sum>=REMOVE_NOISE_DATA_SUM)
	 	{
          rn_distance = gps_raw_data_process_rn(&gps_rawdata_rn);
          if(rn_distance>=REMOVE_NOISE_DATA_DISTANCE_VALUE)
          	{
             
             gps_rawdata_rn.data[1][0] = gps_rawdata_rn.data[2][0];
	         gps_rawdata_rn.data[1][1] = gps_rawdata_rn.data[2][1];
	         gps_rawdata_rn.pausestatus[1] = gps_rawdata_rn.pausestatus[2];
			
             gps_rawdata_rn.data_number--;
	         gps_rawdata_rn.data_sum--;
		    }
		  else;
		  
	    }
	 else;
  gps_rawdata_rn.middle_data_raw[0] = g_DistanceInput.lon;
  
  gps_rawdata_rn.middle_data_raw[1] = g_DistanceInput.lat;
  

}


void gps_rn_data_sm_receive()
{
 uint8_t i;
 uint16_t size;
 int32_t stepsdiff;
  if(gps_rawdata_rn.data_sum==REMOVE_NOISE_DATA_SUM)
   { 
	if(gps_rndata_sm.data_sum==0)
	   {
		  gps_rndata_sm.data[gps_rndata_sm.data_number][0] = gps_rawdata_rn.data[0][0];
		  gps_rndata_sm.data[gps_rndata_sm.data_number][1] = gps_rawdata_rn.data[0][1];

		  gps_rndata_sm.data_number++;
		  gps_rndata_sm.data_sum++;

		  gps_rndata_sm.data[gps_rndata_sm.data_number][0] = gps_rawdata_rn.data[1][0];
		  gps_rndata_sm.data[gps_rndata_sm.data_number][1] = gps_rawdata_rn.data[1][1];

		  gps_rndata_sm.data_number++;
		  gps_rndata_sm.data_sum++;

	   }
	else if( gps_rndata_sm.data_sum>=SMOOTH_DATA_SUM)
		  {
		    for(i=0;i<SMOOTH_DATA_SUM-1;i++)
		    	{
                   gps_rndata_sm.data[i][0] = gps_rndata_sm.data[i+1][0];
		           gps_rndata_sm.data[i][1] = gps_rndata_sm.data[i+1][1];

			   }

			gps_rndata_sm.data[SMOOTH_DATA_SUM-1][0] = gps_rawdata_rn.data[1][0];
		    gps_rndata_sm.data[SMOOTH_DATA_SUM-1][1] = gps_rawdata_rn.data[1][1];
			gps_rndata_sm.data_sum++;

	      }
	else
	   {
		 
		  gps_rndata_sm.data[gps_rndata_sm.data_number][0] = gps_rawdata_rn.data[1][0];
		  gps_rndata_sm.data[gps_rndata_sm.data_number][1] = gps_rawdata_rn.data[1][1];
		  
		  gps_rndata_sm.data_number++;
		
		  gps_rndata_sm.data_sum++;
		 	 

	   }
     gps_rndata_sm.data_smdata_get_flag = GET_SM_DATA;

   }
  else
  	{

     
     gps_rndata_sm.data_smdata_get_flag = DISCARD_SM_DATA;


     }
  /*平滑*/
  if(gps_rndata_sm.data_sum==(SMOOTH_DATA_SUM-4))//3
     	{
       
		 if(gps_rndata_sm.data_smdata_get_flag==GET_SM_DATA)
		 	{
		      gps_data_get_sm.data[gps_data_get_sm.data_number][0] = gps_rndata_sm.data[0][0];
		      gps_data_get_sm.data[gps_data_get_sm.data_number][1] = gps_rndata_sm.data[0][1];
		      gps_data_get_sm.data_number++;
		      gps_data_get_sm.data_number = gps_data_get_sm.data_number%SMOOTH_DATA_SUM;
		      gps_data_get_sm.data_sum++;
		      gps_data_get_sm.data_tsum++;

		      gps_data_get_sm.data[gps_data_get_sm.data_number][0] =  (gps_rndata_sm.data[0][0] + gps_rndata_sm.data[1][0] + gps_rndata_sm.data[2][0])/3.0;
		      gps_data_get_sm.data[gps_data_get_sm.data_number][1] =  (gps_rndata_sm.data[0][1] + gps_rndata_sm.data[1][1] + gps_rndata_sm.data[2][1])/3.0;
			  if(g_DistanceInput.activity_type != ACT_CYCLING && g_DistanceInput.activity_type != ACT_TRIATHLON_CYCLING)
			  
			  	{
		         gps_data_distance_sm.middle_gps_data[0] =  gps_rawdata_rn.data[2][0];
                 gps_data_distance_sm.middle_gps_data[1] =  gps_rawdata_rn.data[2][1];
			  	}
			  else
			  	{
                 gps_data_distance_sm.middle_gps_data[0] =  gps_data_get_sm.data[0][0];
                 gps_data_distance_sm.middle_gps_data[1] =  gps_data_get_sm.data[0][1];
			    }

			   // gps_data_distance_sm.middle_gps_data[0] =  gps_rawdata_rn.data[2][0];
               //  gps_data_distance_sm.middle_gps_data[1] =  gps_rawdata_rn.data[2][1];
              gps_data_get_sm.data_number++;
		      gps_data_get_sm.data_number = gps_data_get_sm.data_number%SMOOTH_DATA_SUM;
		      gps_data_get_sm.data_sum++;
		      gps_data_get_sm.data_tsum++;
		      gps_rndata_sm.data_smdata_get_flag = DISCARD_SM_DATA;
		 	}
		 else;
		  
		  if (g_DistanceInput.activity_type != ACT_CYCLING && g_DistanceInput.activity_type != ACT_TRIATHLON_CYCLING)
		  	{
             size = DistanceInput.time - start_step_single[1];
		     size = (size>1)?size:1;
		  	 
			stepsdiff = StepsComp.steps[StepsComp.rear] - start_step_single[0];
			if(stepsdiff < 0 || stepsdiff >=size* MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
		        gps_data_distance_sm.hdistance_step = stepsdiff*(g_DistanceInput.stride/100.0);
                gps_data_distance_sm.distance_sum += gps_data_distance_sm.hdistance_step;
                start_step_single[0] = StepsComp.steps[StepsComp.rear];
		        start_step_single[1] = DistanceInput.time;
		  	}
		  else;

	   }
 if(gps_rndata_sm.data_sum==(SMOOTH_DATA_SUM-2)) 
    	{
    	
		if(gps_rndata_sm.data_smdata_get_flag==GET_SM_DATA)
			{
    	     gps_data_get_sm.data[gps_data_get_sm.data_number][0] = (gps_rndata_sm.data[0][0] + gps_rndata_sm.data[1][0] + gps_rndata_sm.data[2][0]+ gps_rndata_sm.data[3][0] + gps_rndata_sm.data[4][0])/5.0;
	         gps_data_get_sm.data[gps_data_get_sm.data_number][1] = (gps_rndata_sm.data[0][1] + gps_rndata_sm.data[1][1] + gps_rndata_sm.data[2][1]+ gps_rndata_sm.data[3][1] + gps_rndata_sm.data[4][1])/5.0;
		     gps_data_distance_sm.middle_gps_data[0] =  gps_rawdata_rn.data[2][0];
             gps_data_distance_sm.middle_gps_data[1] =  gps_rawdata_rn.data[2][1];
	         gps_data_get_sm.data_number++;
		     gps_data_get_sm.data_number = gps_data_get_sm.data_number%SMOOTH_DATA_SUM;
		     gps_data_get_sm.data_sum++;
		     gps_data_get_sm.data_tsum++;
		     gps_rndata_sm.data_smdata_get_flag = DISCARD_SM_DATA;
			}
		else;

		
		if (g_DistanceInput.activity_type != ACT_CYCLING && g_DistanceInput.activity_type != ACT_TRIATHLON_CYCLING)
			{
             size = DistanceInput.time - start_step_single[1];
		     size = (size>1)?size:1;
		     stepsdiff = StepsComp.steps[StepsComp.rear] - start_step_single[0];
		    if(stepsdiff < 0 || stepsdiff >= size*MAX_STEPS_THRESHOLD)
			{
			 stepsdiff = 0;
			}
		    gps_data_distance_sm.hdistance_step = stepsdiff*(g_DistanceInput.stride/100.0);
            gps_data_distance_sm.distance_sum += gps_data_distance_sm.hdistance_step;
            start_step_single[0] = StepsComp.steps[StepsComp.rear];
            start_step_single[1] = DistanceInput.time;
			}
		else;
					 

	  }
	

     else if(gps_rndata_sm.data_sum>=SMOOTH_DATA_SUM)
     	{
     	
		if(gps_rndata_sm.data_smdata_get_flag==GET_SM_DATA)
			{
            gps_data_get_sm.data[gps_data_get_sm.data_number][0] = (gps_rndata_sm.data[0][0] + gps_rndata_sm.data[1][0] + gps_rndata_sm.data[2][0]
			                                                    + gps_rndata_sm.data[3][0] + gps_rndata_sm.data[4][0] + gps_rndata_sm.data[5][0] + gps_rndata_sm.data[6][0])/7.0;

		    gps_data_get_sm.data[gps_data_get_sm.data_number][1] =	(gps_rndata_sm.data[0][1] + gps_rndata_sm.data[1][1] + gps_rndata_sm.data[2][1]
			                                                    + gps_rndata_sm.data[3][1] + gps_rndata_sm.data[4][1]+ gps_rndata_sm.data[5][1] + gps_rndata_sm.data[6][1])/7.0;
	        gps_data_get_sm.data_number++;
		    gps_data_get_sm.data_number = gps_data_get_sm.data_number%SMOOTH_DATA_SUM;
		    gps_data_get_sm.data_sum++;	
		    gps_data_get_sm.data_tsum++;
			gps_rndata_sm.data_smdata_get_flag = DISCARD_SM_DATA;
			}
		else;
	   }
	 else;
	 	


}


float gps_data_distance_cal(gps_data_sm*gps_data,uint8_t sum,uint8_t flag)
{
  uint8_t i;
  float temp_dis,single_dis_total;
  temp_dis = 0;
  single_dis_total = 0;

     for(i=0;i<sum;i++)
	    {
		  if(i==0)
		  	{
			   temp_dis =  GpsDistance(gps_data_distance_sm.middle_gps_data[0]*GPS_SCALE_DECIMAL,gps_data_distance_sm.middle_gps_data[1]*GPS_SCALE_DECIMAL,gps_data->data[i][0]*GPS_SCALE_DECIMAL,gps_data->data[i][1]*GPS_SCALE_DECIMAL);
			  
			  
			}
		  else 
		  	{
             
			 temp_dis =  GpsDistance(gps_data->data[i-1][0]*GPS_SCALE_DECIMAL,gps_data->data[i-1][1]*GPS_SCALE_DECIMAL,gps_data->data[i][0]*GPS_SCALE_DECIMAL,gps_data->data[i][1]*GPS_SCALE_DECIMAL);
		     }
	
		   
		   single_dis_total+=temp_dis;
	 
	   }
 	 

   gps_data_distance_sm.middle_gps_data[0] = gps_data->data[sum-1][0];
   
   gps_data_distance_sm.middle_gps_data[1] = gps_data->data[sum-1][1];

  return single_dis_total;

}
float gps_sm_data_distance_receive()
{
	float temp_dis_total;
   temp_dis_total = 0;
   uint8_t i;
/*   if((gps_data_get_sm.data_tsum>=SMOOTH_DATA_TSUM)&&(gps_data_get_sm.data_sum>0))
	  	{
	 
		   temp_dis_total = gps_data_distance_cal(&gps_data_get_sm,gps_data_get_sm.data_sum,1);
	  

	    }
	  else;*/
  if(g_DistanceInput.activity_type != ACT_CYCLING && g_DistanceInput.activity_type != ACT_TRIATHLON_CYCLING)
  	{
	  if((gps_data_get_sm.data_tsum>=SMOOTH_DATA_TSUM)&&(gps_data_get_sm.data_sum>0))
	  	{
	 
		   temp_dis_total = gps_data_distance_cal(&gps_data_get_sm,gps_data_get_sm.data_sum,1);
	  

	    }
	  else;
  	}
   else
   	{
      if((gps_data_get_sm.data_tsum>=SMOOTH_DATA_TSUM-5)&&(gps_data_get_sm.data_sum>0))
	  	{
	 
		   temp_dis_total = gps_data_distance_cal(&gps_data_get_sm,gps_data_get_sm.data_sum,0);
	  

	    }
	  else;

    }
          

  
return temp_dis_total;


}

void gps_sm_data_kf_receive()
{
 uint8_t i;
  if(gps_data_get_sm.data_sum>0)
  	{
    kalmanfilter_path(&gps_data_get_sm,gps_data_get_sm.data_sum);

	 gps_data_output.output_data_sum = gps_data_get_sm.data_sum;
     for(i=0;i< gps_data_output.output_data_sum;i++)
     	{
        gps_data_output.data[i][0] = (int32_t)(gps_data_get_sm.data[i][0]*100000);
		gps_data_output.data[i][1] = (int32_t)(gps_data_get_sm.data[i][1]*100000);
  
	   }
  
	 
     gps_data_get_sm.data_sum = 0;
	 gps_data_get_sm.data_number = 0;
    }

  else;
  

}

void TrackOptimizingDataAccept(DistanceInputStr DistanceInput)
{
	g_DistanceInput = DistanceInput;
	
	int32_t stepsdiff;
	
	if (g_DistanceInput.pause_status == 2) //暂停状态为2时，表示运动停止了
	{
		if(g_DistanceInput.activity_type != ACT_INDOORRUN)
		{
		 if(gps_rndata_sm.data_sum>=SMOOTH_DATA_SUM)
			{
				sport_over_or_restart();
			}
		}

		return;
	}

		//将计步的数据存储起来，用于计算距离和速度
	if (StepsComp.size < STEPSCOMP_SIZE)
	{
		StepsComp.size++;
	}
	
	if (StepsComp.rear >= STEPSCOMP_SIZE - 1)
	{
		StepsComp.rear = 0;
	}
	else
	{
		StepsComp.rear++;
	}
	StepsComp.steps[StepsComp.rear] = g_DistanceInput.steps;	
	steps_freq();	
	if (g_DistanceInput.lon != 0 && g_DistanceInput.lat != 0)
	{
		//第一个gps有效数据且在非暂停状态下
		if (TrackOptimizingResultData.Longitude == 0 && TrackOptimizingResultData.Latitude == 0 && g_DistanceInput.pause_status == NON_PAUSE_STATUS)
		{
			g_firstGPS_flag = 1;
			g_NoGPS_flag = 0;
			TrackOptimizingResultData.Longitude = g_DistanceInput.lon;
			TrackOptimizingResultData.Latitude =  g_DistanceInput.lat;
			TrackOptimizingResultData.pausestatus = g_DistanceInput.pause_status;
          
		  if (g_DistanceInput.activity_type != ACT_CYCLING && g_DistanceInput.activity_type != ACT_TRIATHLON_CYCLING)
		  	{
            start_step_single[0] = StepsComp.steps[StepsComp.rear];
			start_step_single[1] = DistanceInput.time;
		  	}
		  else;
            gps_raw_data_rn_receive();

			
			if (g_DistanceInput.activity_type != ACT_CYCLING && g_DistanceInput.activity_type != ACT_TRIATHLON_CYCLING)
				{
			     stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + STEPSCOMP_SIZE - 1) % STEPSCOMP_SIZE)];
			    if(stepsdiff < 0 || stepsdiff >= MAX_STEPS_THRESHOLD)
				  {
					stepsdiff = 0;
				  }
			      gps_data_distance_sm.ndistance_step = stepsdiff*(g_DistanceInput.stride/100.0);
			      gps_data_distance_sm.distance_sum += gps_data_distance_sm.ndistance_step;
				}
			else;
			return;//返回之前计算一次距离

		}
		
	}

	//调用综合函数		
	SportsCalculateDistSpeed();
}
	
/*
GPS异常判断及对应的处理
*/
uint8_t gps_exception()
{
   uint8_t flag = GPS_NORMAL;
   float lfv_gapdist;
   uint8_t temp_num;
	if (gps_rndata_sm.data_sum >=1 && g_DistanceInput.gps_status == 1)
	{
		lfv_gapdist = GpsDistance(g_DistanceInput.lon,g_DistanceInput.lat,gps_rawdata_rn.middle_data_raw[0],gps_rawdata_rn.middle_data_raw[1]);

	
         if(no_gps_num<=5)
         	{
         	  
			  if(g_DistanceInput.activity_type == ACT_CYCLING)//骑行类
				{
				   if(lfv_gapdist >=(single_gpsdis_limit*8))
				   	{
				   	  if(cycling_gps_exceed_num<=8)
				   		{
                           cycling_gps_exceed_num++;
					       flag = GPS_DRIFT_EXCEPTION;
					   }
					else
						{
                         
						 nogps_maxgap_init(0);
						 
                         cycling_gps_exceed_num = 0;
						 
						 flag = GPS_DRIFT_EXCEPTION;
					    }
				 
				   	}
				   else if(lfv_gapdist >=(single_gpsdis_limit*5))
				   	{
                       
					  if(cycling_gps_exceed_num<=5)
				   		{
                           cycling_gps_exceed_num++;
					       flag = GPS_DRIFT_EXCEPTION;
					   }
					else
						{
                         
						 nogps_maxgap_init(0);
						 
                         cycling_gps_exceed_num = 0;
						 
						 flag = GPS_DRIFT_EXCEPTION;
					    }

				     }
				   else
				   	{
                      
					  cycling_gps_exceed_num = 0;


				     }
				   	
				}
			  else //步行类
			  	{
				   if(lfv_gapdist >=(single_gpsdis_limit*8))
				   	{
				   	  if(step_gps_exceed_num<=8)
				   		{
                           step_gps_exceed_num++;
					       flag = GPS_DRIFT_EXCEPTION;
					   }
					else
						{
                         
						 nogps_maxgap_init(0);
						 
                         step_gps_exceed_num = 0;
						 
						 flag = GPS_DRIFT_EXCEPTION;
					    }
				 
				   	}
				   else if(lfv_gapdist >=(single_gpsdis_limit*5))
				   	{
                       
					  if(step_gps_exceed_num<=5)
				   		{
                           step_gps_exceed_num++;
					       flag = GPS_DRIFT_EXCEPTION;
					   }
					else
						{
                         
						 nogps_maxgap_init(0);
						 
                         step_gps_exceed_num = 0;
						 
						 flag = GPS_DRIFT_EXCEPTION;
					    }

				     }
				   else
				   	{
                      
					  step_gps_exceed_num = 0;


				     }
				   	
				}

		    }

		 
        else if((no_gps_num<=30)&&(no_gps_num>5))
			{
         	  temp_num = (no_gps_num>10)?no_gps_num:10;
			  if(g_DistanceInput.activity_type == ACT_CYCLING)//骑行类
				{
				   if(lfv_gapdist >=(single_gpsdis_limit*temp_num))
				   	{
				   	  if(cycling_gps_exceed_num<=10)
				   		{
                           cycling_gps_exceed_num++;
					       flag = GPS_DRIFT_EXCEPTION;
					   }
					else
						{
                         
						 nogps_maxgap_init(0);
						 
                         cycling_gps_exceed_num = 0;
						 
						 flag = GPS_DRIFT_EXCEPTION;
					    }
				 
				   	}
				   else if(lfv_gapdist >=(single_gpsdis_limit*temp_num*0.7))
				   	{
                       
					  if(cycling_gps_exceed_num<=7)
				   		{
                           cycling_gps_exceed_num++;
					       flag = GPS_DRIFT_EXCEPTION;
					   }
					else
						{
                         
						 nogps_maxgap_init(0);
						 
                         cycling_gps_exceed_num = 0;
						 
						 flag = GPS_DRIFT_EXCEPTION;
					    }

				     }
				   else
				   	{
                      
					  cycling_gps_exceed_num = 0;


				     }
				   	
				}
			  else //步行类
			  	{
				   if(lfv_gapdist >=(single_gpsdis_limit*temp_num))
				   	{
				   	  if(step_gps_exceed_num<=10)
				   		{
                           step_gps_exceed_num++;
					       flag = GPS_DRIFT_EXCEPTION;
					   }
					else
						{
                         
						 nogps_maxgap_init(0);
						 
                         step_gps_exceed_num = 0;
						 
						 flag = GPS_DRIFT_EXCEPTION;
					    }
				 
				   	}
				   else if(lfv_gapdist >=(single_gpsdis_limit*temp_num*0.7))
				   	{
                       
					  if(step_gps_exceed_num<=7)
				   		{
                           step_gps_exceed_num++;
					       flag = GPS_DRIFT_EXCEPTION;
					   }
					else
						{
                         
						 nogps_maxgap_init(0);
						 
                         step_gps_exceed_num = 0;
						 
						 flag = GPS_DRIFT_EXCEPTION;
					    }

				     }
				   else
				   	{
                      
					  step_gps_exceed_num = 0;


				     }
				   	
				}

		    }
	    else 
			  {
                      
			  nogps_maxgap_init(0);

					  
			 flag = NOGPS_DIST_EXCEPTION;

			  }	

	}
	return flag;
}

/*
Gps掉星时，点间距出现很大的值，初始化各参数，从头开始优化轨迹
*/
void nogps_maxgap_init(uint8_t flag)
{
	if (gps_rndata_sm.data_sum>=SMOOTH_DATA_SUM)
	{
		sport_over_or_restart();
	}
	memset(&gps_rawdata_rn, 0, sizeof(gps_rawdata_rn));
	memset(&gps_rndata_sm, 0, sizeof(gps_rndata_sm));
    memset(&gps_data_get_sm, 0, sizeof(gps_data_get_sm));
    memset(&gps_data_output, 0, sizeof(gps_data_output));
    memset(&kf_last_gps_point, 0, sizeof(kf_last_gps_point));
	memset(&instant_speed, 0, sizeof(instant_speed));
	memset(&TrackOptimizingResultData, 0, sizeof(TrackOptimizingResultData));
    memset(&start_step_single, 0, sizeof(int)*2);
    g_firstGPS_flag = 1;
	g_NoGPS_flag = 1;
    start_point_num = 0; 
    kf_initial();
	
}
void sport_over_or_restart()
{
   uint8_t over_size;
   over_size = 4;

  
  pre_gps_point[0].Lon = gps_rndata_sm.data[4][0]*100000;
  pre_gps_point[0].Lat = gps_rndata_sm.data[4][1]*100000;
  pre_distance[0] = GpsDistance(gps_rndata_sm.data[3][0]*GPS_SCALE_DECIMAL,gps_rndata_sm.data[3][1]*GPS_SCALE_DECIMAL, gps_rndata_sm.data[4][0]*GPS_SCALE_DECIMAL, gps_rndata_sm.data[4][1]*GPS_SCALE_DECIMAL);

  pre_gps_point[1].Lon = gps_rndata_sm.data[5][0]*100000;
  pre_gps_point[1].Lat = gps_rndata_sm.data[5][1]*100000;
  pre_distance[1] = GpsDistance(gps_rndata_sm.data[4][0]*GPS_SCALE_DECIMAL,gps_rndata_sm.data[4][1]*GPS_SCALE_DECIMAL, gps_rndata_sm.data[5][0]*GPS_SCALE_DECIMAL, gps_rndata_sm.data[5][1]*GPS_SCALE_DECIMAL);

  pre_gps_point[2].Lon = gps_rndata_sm.data[6][0]*100000;
  pre_gps_point[2].Lat = gps_rndata_sm.data[6][1]*100000;
  pre_distance[2] = GpsDistance(gps_rndata_sm.data[5][0]*GPS_SCALE_DECIMAL,gps_rndata_sm.data[5][1]*GPS_SCALE_DECIMAL, gps_rndata_sm.data[6][0]*GPS_SCALE_DECIMAL, gps_rndata_sm.data[6][1]*GPS_SCALE_DECIMAL);

  pre_gps_point[3].Lon = gps_rawdata_rn.data[2][0]*100000;
  pre_gps_point[3].Lat = gps_rawdata_rn.data[2][1]*100000;
  pre_distance[3] = GpsDistance(gps_rndata_sm.data[6][0]*GPS_SCALE_DECIMAL,gps_rndata_sm.data[6][1]*GPS_SCALE_DECIMAL, gps_rawdata_rn.data[2][0]*GPS_SCALE_DECIMAL, gps_rawdata_rn.data[2][1]*GPS_SCALE_DECIMAL);
  
   gps_data_output.output_data_sum = over_size;
   
   gps_data_distance_sm.distance_sum +=(pre_distance[0]+pre_distance[1]+pre_distance[2]+pre_distance[3]);

}


//用于判断是否处于静止状态
uint8_t StaticJudgment()
{
    if (StepsComp.steps[((StepsComp.rear + (STEPSCOMP_SIZE -1)) % STEPSCOMP_SIZE)] < StepsComp.steps[StepsComp.rear])	//步数在增加，说明没有静止
    {
        g_stepthreshold = 0;
        return SPORT_FLAG_MOVE;
    }
    else
    { 
        g_stepthreshold++;
        if (g_stepthreshold < STATIC_STEPS_THRESHOLD)
        {
            return SPORT_FLAG_MOVE;
        }
        else
        {
            return SPORT_FLAG_STATIS;
        }
    }
}



void SportsCalculateDistSpeed()
{
	
	int32_t stepsdiff;
    
	uint8_t liv_gpsexcept;
	if ((g_DistanceInput.activity_type == ACT_CYCLING) || (g_DistanceInput.activity_type == ACT_TRIATHLON_CYCLING))   //骑行类去小漂移范围设定为30米
	{
		single_gpsdis_limit = NOGPS_MAX_DISTANCE_CYCLE;
	}
	else
	{
		single_gpsdis_limit = NOGPS_MAX_DISTANCE_FOOT;   
	}

	

	if (g_DistanceInput.activity_type != ACT_CYCLING && g_DistanceInput.activity_type != ACT_TRIATHLON_CYCLING)
	{
		sport_status_flag = StaticJudgment();
	}
	else
	{
		
		sport_status_flag = staticjudgment_cycling_result();
	}

	//GPS可用,计步可用的运动
	if (g_DistanceInput.activity_type == ACT_RUN || g_DistanceInput.activity_type == ACT_WALK || g_DistanceInput.activity_type == ACT_MARATHON
		|| g_DistanceInput.activity_type == ACT_CROSSCOUNTRY || g_DistanceInput.activity_type == ACT_HIKING || g_DistanceInput.activity_type == ACT_TRIATHLON_RUN
		|| g_DistanceInput.activity_type == ACT_TRAIN_PLAN_RUN)
	{
		//先判断gps是否异常
		liv_gpsexcept = gps_exception();
		if (liv_gpsexcept != GPS_NORMAL)
		{
	
           test_gps_flag = 0;
			
		}
		else
		{
           test_gps_flag = 1;

		}
	  if(test_gps_flag == 0)
      	{

		        stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + STEPSCOMP_SIZE - 1) % STEPSCOMP_SIZE)];
				if (stepsdiff < 0 || stepsdiff >= MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
				
				 gps_data_distance_sm.distance_sum += stepsdiff * (g_DistanceInput.stride / 100.0);
				
              stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + 1) % STEPSCOMP_SIZE)];
				if (stepsdiff < 0 || stepsdiff >= 4*MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
				g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/(STEPSCOMP_SIZE-1);
				if (sport_status_flag == SPORT_FLAG_STATIS)
				{
					g_Speed = 0;
				}

	    }

		//GPS在线的状态
		else if (g_DistanceInput.gps_status == 1)
		{
			if (sport_status_flag == SPORT_FLAG_STATIS)
			{
			    
              g_DistanceInput.lon = gps_rawdata_rn.middle_data_raw[0];
			  g_DistanceInput.lat = gps_rawdata_rn.middle_data_raw[1];
								   		
			}

            gps_raw_data_rn_receive();//去澡
            SpeedOptimizingDataCollection(g_DistanceInput.lon,g_DistanceInput.lat);//计算速度和配速
			gps_rn_data_sm_receive();//平滑
			gps_data_distance_sm.sdistance_gps = gps_sm_data_distance_receive();//计算距离
		    gps_sm_data_kf_receive();//卡尔曼滤波
			  #if defined STORE_ORG_DATA_TEST_VERSION
		   store_orggps_data((int32_t)(g_DistanceInput.lon),(int32_t)(g_DistanceInput.lat),(int32_t)( gps_data_distance_sm.middle_gps_data[0]*GPS_SCALE_DECIMAL),(int32_t)( gps_data_distance_sm.middle_gps_data[1]*GPS_SCALE_DECIMAL),

		  (int32_t)(gps_data_distance_sm.sdistance_gps*100),(int32_t)(instant_speed.speed*100));
		   #endif
	
          //配速
			if (g_DistanceInput.time < SPEED_DELAY_TIME)
			{
				g_Speed = 0;
			}
			else if (instant_speed.speed != 0)
			{
				g_Speed = instant_speed.speed;
			}
					 
	       gps_data_distance_sm.distance_sum +=  gps_data_distance_sm.sdistance_gps;//累加距离

					  						
		}
       
	   //GPS掉星,运动状态

	   
		else if (g_DistanceInput.gps_status == 0 && g_DistanceInput.pause_status == 0)
		{
			NoGPSSpeedParamInit();

		
			stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + STEPSCOMP_SIZE - 1) % STEPSCOMP_SIZE)];
			if (stepsdiff < 0 || stepsdiff >= MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
			gps_data_distance_sm.ndistance_step = stepsdiff *g_DistanceInput.stride/100.0;

           gps_data_distance_sm.distance_sum += gps_data_distance_sm.ndistance_step;
		   
			if (g_DistanceInput.time < SPEED_DELAY_TIME)
			{
				g_Speed = 0;
			}
			else
			{
				 stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + 1) % STEPSCOMP_SIZE)];

				 
				if (stepsdiff < 0 || stepsdiff >= 4*MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
			  if(g_DistanceInput.time<STEPSCOMP_SIZE-1)
					{
					
					g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/g_DistanceInput.time;
					}
			  else
					{
				      g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/(STEPSCOMP_SIZE-1);
					}
			}

		}
		//GPS掉星,暂停状态
		else if (g_DistanceInput.gps_status == 0 && g_DistanceInput.pause_status == 1)
		{
			NoGPSSpeedParamInit();
           
			if (g_DistanceInput.time < SPEED_DELAY_TIME)
			{
				g_Speed = 0;
			}
			else 
			{
				 stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + 1) % STEPSCOMP_SIZE)];
				if (stepsdiff < 0 || stepsdiff >= 4*MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
			if(g_DistanceInput.time<STEPSCOMP_SIZE-1)
				{
					
				 g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/g_DistanceInput.time;
				}
		   else
				{
				  g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/(STEPSCOMP_SIZE-1);
				}
			}
		}

		if (sport_status_flag == SPORT_FLAG_STATIS)
		{
			g_Speed = 0;
		}
		
	}
	//GPS可用,计步可用的运动
	else if (g_DistanceInput.activity_type == ACT_CLIMB)
	{
		uint32_t liv_heightvalue;
		//计算高度绝对值的累计变化
		liv_heightvalue = g_DistanceInput.up_value + g_DistanceInput.down_value;
		
		//先判断gps是否异常
		liv_gpsexcept = gps_exception();
		if (liv_gpsexcept != GPS_NORMAL)
		{
	
           test_gps_flag = 0;
			
		}
		else
		{
           test_gps_flag = 1;

		}
	  if(test_gps_flag == 0)
      	{

		   stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + STEPSCOMP_SIZE - 1) % STEPSCOMP_SIZE)];
			if (stepsdiff < 0 || stepsdiff >= MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
				
		    gps_data_distance_sm.distance_sum += stepsdiff * g_DistanceInput.stride/100.0;
				
         

	    }
		
		//GPS在线状态
		else if (g_DistanceInput.gps_status == 1)
		{
			if (sport_status_flag == SPORT_FLAG_STATIS)
			{
			    
              g_DistanceInput.lon = gps_rawdata_rn.middle_data_raw[0];
			  g_DistanceInput.lat = gps_rawdata_rn.middle_data_raw[1];
								   		
			}

            gps_raw_data_rn_receive();
            SpeedOptimizingDataCollection(g_DistanceInput.lon,g_DistanceInput.lat);
			gps_rn_data_sm_receive();
			gps_data_distance_sm.sdistance_gps = gps_sm_data_distance_receive();

		    gps_sm_data_kf_receive();
		
	
 
					 
	       gps_data_distance_sm.distance_sum +=  gps_data_distance_sm.sdistance_gps;

					  						
		}
         
		 //GPS掉星,运动状态
		else if (g_DistanceInput.gps_status == 0 && g_DistanceInput.pause_status == 0)
		{
			NoGPSSpeedParamInit();
				stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + STEPSCOMP_SIZE - 1) % STEPSCOMP_SIZE)];
			if (stepsdiff < 0 || stepsdiff >= MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
			gps_data_distance_sm.ndistance_step = stepsdiff *g_DistanceInput.stride/100.0;

           gps_data_distance_sm.distance_sum += gps_data_distance_sm.ndistance_step;
		
		}
		
		if (g_DistanceInput.time < SPEED_DELAY_TIME)
		{
			g_Speed = 0;
		}
		else
		{
			g_Speed = (float)liv_heightvalue / g_DistanceInput.time;
		}
	}
	//gps可用，计步不可用的运动
	else if (g_DistanceInput.activity_type == ACT_TRIATHLON_SWIM || g_DistanceInput.activity_type == ACT_CYCLING || g_DistanceInput.activity_type == ACT_TRIATHLON_CYCLING)
	{

		//先判断gps是否异常
		liv_gpsexcept = gps_exception();
		if (liv_gpsexcept != GPS_NORMAL)
		{
	
           test_gps_flag = 0;
			
		}
		else
		{
           test_gps_flag = 1;

		}
          if(test_gps_flag == 0);
		  	
		else if (g_DistanceInput.gps_status == 1)
		{
			if (sport_status_flag == SPORT_FLAG_STATIS)
			{
			    
                g_DistanceInput.lon = gps_rawdata_rn.middle_data_raw[0];
				g_DistanceInput.lat = gps_rawdata_rn.middle_data_raw[1];
								   	
			}
		    gps_raw_data_rn_receive();
		    SpeedOptimizingDataCollection(g_DistanceInput.lon,g_DistanceInput.lat);
          	gps_rn_data_sm_receive();
			gps_data_distance_sm.sdistance_gps = gps_sm_data_distance_receive();

		  #if defined STORE_ORG_DATA_TEST_VERSION
		   store_orggps_data((int32_t)(g_DistanceInput.lon),(int32_t)(g_DistanceInput.lat),(int32_t)( gps_data_distance_sm.middle_gps_data[0]*GPS_SCALE_DECIMAL),(int32_t)( gps_data_distance_sm.middle_gps_data[1]*GPS_SCALE_DECIMAL),

		  (int32_t)(gps_data_distance_sm.sdistance_gps*100),(int32_t)(instant_speed.speed*100));
		   #endif
			
		    gps_sm_data_kf_receive();
      
				
         
			if (g_DistanceInput.time < SPEED_DELAY_TIME)
			{
				g_Speed = 0;
			}
			else
			{
				if (instant_speed.speed <= SPEED_MINLIMIT)
				{
					g_zeroSpeedCount++;
					
					g_Speed = instant_speed.speed;
					if (g_zeroSpeedCount > 5)
					{
						g_Speed = 0;
					}
				}
				else
				{
					g_zeroSpeedCount = 0;
					g_Speed = instant_speed.speed;
				}
			}		
			
			gps_data_distance_sm.distance_sum +=  gps_data_distance_sm.sdistance_gps;

			
		}
		else
		{
			NoGPSSpeedParamInit();
		}
		if (sport_status_flag == SPORT_FLAG_STATIS)
		{
			g_Speed = 0;
		}
	}
	//GPS不可用,只能用计步
	else if (g_DistanceInput.activity_type == ACT_INDOORRUN && StepsComp.rear < STEPSCOMP_SIZE)
	{
		if (sport_status_flag == SPORT_FLAG_MOVE)
		{
			if (g_DistanceInput.pause_status == 0)  //非暂停状态
			{
			  stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + STEPSCOMP_SIZE - 1) % STEPSCOMP_SIZE)];
			  if(stepsdiff < 0 || stepsdiff >= MAX_STEPS_THRESHOLD)
				{
					stepsdiff = 0;
				}
			    gps_data_distance_sm.ndistance_step = stepsdiff *g_DistanceInput.stride/100.0;

                gps_data_distance_sm.distance_sum += gps_data_distance_sm.ndistance_step;


				
				if (g_DistanceInput.time < SPEED_DELAY_TIME)
				{
					g_Speed = 0;
				}
				else
				{
					stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + 1) % STEPSCOMP_SIZE)];
					if (stepsdiff < 0 || stepsdiff >= 4*MAX_STEPS_THRESHOLD)
					{
						stepsdiff = 0;
					}
				  if(g_DistanceInput.time<STEPSCOMP_SIZE-1)
					{
					
					g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/g_DistanceInput.time;
					}
			      else
					{
				      g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/(STEPSCOMP_SIZE-1);
					}
					
				}
			}
			else
			{
					
				if (g_DistanceInput.time < SPEED_DELAY_TIME)
				{
					g_Speed = 0;
				}
				else
				{
					stepsdiff = StepsComp.steps[StepsComp.rear] - StepsComp.steps[((StepsComp.rear + 1) % STEPSCOMP_SIZE)];
					if (stepsdiff < 0 || stepsdiff >= 4*MAX_STEPS_THRESHOLD)
					{
						stepsdiff = 0;
					}
				  if(g_DistanceInput.time<STEPSCOMP_SIZE-1)
					{
					
					g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/g_DistanceInput.time;
					}
			      else
					{
				      g_Speed = (stepsdiff*g_DistanceInput.stride/100.0)/(STEPSCOMP_SIZE-1);
					}
					
				}
			}
		}
		else  //静止状态
		{
			g_Speed = 0;
		}
	}

	//GPS是否在线标志，用于下一秒判断是否添加gps的距离
	if (g_DistanceInput.gps_status == 0)
	{
		g_NoGPS_flag = 1;
		
		no_gps_num++;
	}
	else
	{
		g_NoGPS_flag = 0;
		
		no_gps_num = 0;
	}

	//计算一分钟平均速度
	minuteMeanSpeedCalculate();

}



void NoGPSSpeedParamInit()
{
	instant_speed.speed = g_Speed;
	instant_speed.ori_count = 0;
	instant_speed.ori_dst_head = 0;
}

void SpeedOptimizingDataCollection(int32_t ori_lon, int32_t ori_lat)
{
	float total_dst = 0;
	float lfv_distlimit;
	float speed_change;
	static uint32_t lisv_lastgpsflag = DOWN_GPS_FLAG; //上一个数据的GPS状态标志，初始化为掉星状态
	uint8_t now_point = 0;
	int i,j,k;

	/*计算每秒步频*/
	persec_step[persec_step_point] = step_data_str.step_num_sport - last_step_num_sport;
	now_point = persec_step_point;
	persec_step_point++;
	persec_step_point%=PERSEC_STEP_SIZE;
	if (record_step_point < PERSEC_STEP_SIZE)
		{
		 record_step_point++;
		}
	else;
	if(record_step_point == PERSEC_STEP_SIZE)
		{
         mean_persec_step = 0;
		 for(i = 0;i < PERSEC_STEP_SIZE;i++)
		 	{
		 	 j = now_point-i;
			 
			if(j<0)
				{
                 j = j + PERSEC_STEP_SIZE;
			    }
			else;
            mean_persec_step += persec_step[j]*para_mean[i];
		 	}
		 
	    }

	else
		{

		j = now_point;
			 
			if(j<0)
				{
                 j = j + PERSEC_STEP_SIZE;
			    }
			else;
		 mean_persec_step = persec_step[j];
	    }
	if (ori_lon != 0 || ori_lat != 0)
	{
		//当且仅当上一秒与当前秒都是在星状态时计算瞬时速度
		if (instant_speed.ori_last_lon != 0 && instant_speed.ori_last_lat != 0 && lisv_lastgpsflag == UP_GPS_FLAG && g_DistanceInput.gps_status == UP_GPS_FLAG)
		{
			instant_speed.ori_dst[instant_speed.ori_dst_head] = GpsDistance(instant_speed.ori_last_lon, instant_speed.ori_last_lat, ori_lon, ori_lat);

			//过滤掉gps点间过大的距离值


			/*
             combo_speed_step:运动状态，从计步模块引入,0~5依次代表运动快慢程度由慢到快
             每个运动快慢程序赋值不同的速度上限阈值
			*/
			if(combo_speed_step == 0)
				{
                   lfv_distlimit = WALK_SPEED_MAXLIMIT_ZERO;
			      }
			else if(combo_speed_step == 1)
				{
			    lfv_distlimit = WALK_SPEED_MAXLIMIT_SLOW;
				}
			else if(combo_speed_step == 2)
				{
			    lfv_distlimit = WALK_SPEED_MAXLIMIT_GRNERAL;
				}
			else if(combo_speed_step == 3)
				{
			    lfv_distlimit = WALK_SPEED_MAXLIMIT_NORMAL;
				}
			else if(combo_speed_step == 4)
				{
			    lfv_distlimit = WALK_SPEED_MAXLIMIT_FAST;
				}
			else
				{

                 lfv_distlimit = RUN_SPEED_MAXLIMIT;

			    }
			if((g_DistanceInput.activity_type == ACT_CYCLING) || (g_DistanceInput.activity_type == ACT_TRIATHLON_CYCLING))
			{
				lfv_distlimit = CYCLING_SPEED_MAXLIMIT;
			}
			else;
			if(instant_speed.ori_dst[instant_speed.ori_dst_head] < lfv_distlimit)
			{
			    now_point = instant_speed.ori_dst_head;
				instant_speed.ori_dst_head++;
				instant_speed.ori_dst_head %= PERSEC_SPEED_SIZE;
				if (instant_speed.ori_count < PERSEC_SPEED_SIZE)
				{
					instant_speed.ori_count++;
				}
			}
			
			/*
            1、  距离/速度大于跑步类的速度上限值时,直接用5s钟内的平均步频*步长代替
            2、距离/速度大于阈值时，以步频与GPS计算出来的距离作加权和代替
            
			*/
			else if((instant_speed.ori_dst[instant_speed.ori_dst_head] >= lfv_distlimit)&&(g_DistanceInput.activity_type != ACT_CYCLING)
				  &&(g_DistanceInput.activity_type != ACT_TRIATHLON_CYCLING))
				{
                  if(instant_speed.ori_dst[instant_speed.ori_dst_head]>=RUN_SPEED_MAXLIMIT)
                  	{
                  	if(combo_speed_step==5)
                  		{
                           instant_speed.ori_dst[instant_speed.ori_dst_head] = mean_persec_step*(SetValue.AppGet.StepLengthRun/100);
                  		}
					else
						{
                           instant_speed.ori_dst[instant_speed.ori_dst_head] = mean_persec_step*(SetValue.AppGet.StepLengthWalking/100);
						   
					    }
					 now_point = instant_speed.ori_dst_head;
                     if(instant_speed.ori_dst[instant_speed.ori_dst_head] >= RUN_SPEED_MAXLIMIT);
                     	 
					 else
					 	{
                     
	                     instant_speed.ori_dst_head++;
						 instant_speed.ori_dst_head %= PERSEC_SPEED_SIZE;
						 if (instant_speed.ori_count < PERSEC_SPEED_SIZE)
							{
								instant_speed.ori_count++;
							}
					 	}
				    }
                   else 
                   	{
                       if(combo_speed_step==5)//不进入
                       	{
                          instant_speed.ori_dst[instant_speed.ori_dst_head] = mean_persec_step*(SetValue.AppGet.StepLengthRun/100)*0.7f
					      +instant_speed.ori_dst[instant_speed.ori_dst_head]*0.3f;//使用运动步长
					    }
					   else
					   	{
                           instant_speed.ori_dst[instant_speed.ori_dst_head] = mean_persec_step*(SetValue.AppGet.StepLengthWalking/100)*0.7f
					        +instant_speed.ori_dst[instant_speed.ori_dst_head]*0.3f;//使用行走步长
					     }

					    now_point = instant_speed.ori_dst_head;
					   	 if(instant_speed.ori_dst[instant_speed.ori_dst_head] >= RUN_SPEED_MAXLIMIT);
                     	 
					     else
					     	{
					   	
                               instant_speed.ori_dst_head++;
							  instant_speed.ori_dst_head %= PERSEC_SPEED_SIZE;
							  if(instant_speed.ori_count < PERSEC_SPEED_SIZE)
								{
									instant_speed.ori_count++;
								}
					     	}
				   }

			    }
			else
				{
                 
                   k = instant_speed.ori_dst_head - 1;
			 
			     if(k<0)
				   {
                    k = k + PERSEC_SPEED_SIZE;
			       }
                 instant_speed.ori_dst[instant_speed.ori_dst_head] = instant_speed.ori_dst[k];
				 if(instant_speed.ori_dst[instant_speed.ori_dst_head]>lfv_distlimit)
				 	{

                     instant_speed.ori_dst[instant_speed.ori_dst_head] = lfv_distlimit;
				    }
				 else;
                  now_point = instant_speed.ori_dst_head;

			   }
			
				

			    
		}
		instant_speed.ori_last_lat = ori_lat;
		instant_speed.ori_last_lon = ori_lon;
		lisv_lastgpsflag = g_DistanceInput.gps_status;

	 if(instant_speed.ori_count>=PERSEC_SPEED_SIZE)
	 	{
         instant_speed.speed = 0;

		 track_bubble_sort(instant_speed.ori_dst,PERSEC_SPEED_SIZE);
		 for(i = 0;i < PERSEC_SPEED_SIZE-2;i++)
		 	{
           
			 instant_speed.speed +=0.2f*instant_speed.ori_dst[i+1];	

		   }
		 
      	}
	    
	 else
	 	{
         
           if(instant_speed.ori_count>=3)
           	{

			 	track_bubble_sort(instant_speed.ori_dst,instant_speed.ori_count);
		 		 instant_speed.speed = 0;

		  		for( i = 0; i < instant_speed.ori_count; i++)
				{
					if ((i != 0) && (i != instant_speed.ori_count-1))
					{
						instant_speed.speed += instant_speed.ori_dst[i];
					}
				}
             
			 instant_speed.speed = instant_speed.speed/(instant_speed.ori_count - 2);
		    }
		   else
		   	{
		  
             instant_speed.speed = instant_speed.ori_dst[now_point];
		   	}
	    }
	 	/*
        
     根据前后每秒钟速度的变化，以前秒和此时的速度作加权和代替此时刻的速度

		
        */
	    if((instant_speed.speed!=0)&&(g_LastInstantSpeed!=0))
	    	{
             speed_change = fabs((1000.0f / instant_speed.speed + 0.5f)-(1000.0f / g_LastInstantSpeed + 0.5f));
			 if(speed_change<=30.0f)
			 	{
                   instant_speed.speed = instant_speed.speed * para_share_speed[0] + g_LastInstantSpeed * (1-para_share_speed[0]);
			    }
			 else if((speed_change<=60.0f)&&(speed_change>30.0f))

			 	{
                 instant_speed.speed = instant_speed.speed * para_share_speed[1] + g_LastInstantSpeed * (1-para_share_speed[1]);
			   }
			 else if((speed_change<=120.0f)&&(speed_change>60.0f))
			 	{
                  instant_speed.speed = instant_speed.speed * para_share_speed[2] + g_LastInstantSpeed * (1-para_share_speed[2]);
			    }
			 else if((speed_change<=180.0f)&&(speed_change>120.0f))
			 	{
                  instant_speed.speed = instant_speed.speed * para_share_speed[3] + g_LastInstantSpeed * (1-para_share_speed[3]);
			   }
			 else
			 	{
                  instant_speed.speed = instant_speed.speed * para_share_speed[4] + g_LastInstantSpeed * (1-para_share_speed[4]);
			   }
             
		   }
		else
			{
              instant_speed.speed = instant_speed.speed * (1 - DELAY_SMOOTH_ARGUMENT) + g_LastInstantSpeed * DELAY_SMOOTH_ARGUMENT;

		     }
		g_LastInstantSpeed = instant_speed.speed;
	}
	
	else
	{
		NoGPSSpeedParamInit();
	}
	last_step_num_sport = step_data_str.step_num_sport;
}

void minuteMeanSpeedCalculate()
{
	uint8_t i,max_size;
	if (g_DistanceInput.pause_status == NON_PAUSE_STATUS)
	{
		if (g_DistanceInput.activity_type == ACT_CLIMB)
		{
			g_LSdown_value = g_DistanceInput.down_value;
			g_LStime = g_DistanceInput.time;
			g_LSup_value = g_DistanceInput.up_value;
			if (g_LStime - g_FStime > 0)
			{
				g_minuMeanSpeed = (g_LSdown_value + g_LSup_value - g_FSdown_value - g_FSup_value) / (float)(g_LStime - g_FStime);
			}
			else
			{
				g_minuMeanSpeed = 0;
			}

			if ((g_DistanceInput.time % 60) == 0)
			{
				g_FSdown_value = g_DistanceInput.down_value;
				g_FStime = g_DistanceInput.time;
				g_FSup_value = g_DistanceInput.up_value;
			}
		}
		else
		{
			if ((g_DistanceInput.time % 60) == 1)
			{
				g_minispeed_count = 0;
               g_InstantSpeedSum = 0;
			 	g_speed_count = 1;
				//计算瞬时速度为0的个数
				if (SpeedResult() == 0)
				{
					g_minispeed_count++;
				}
				else
					{
				     g_InstantSpeedSum = g_Speed;
					   per_sec_speed[0] = g_Speed;
					}
				
			}
			else
			{
				//计算瞬时速度为0的个数
				g_speed_count++;
				if (SpeedResult() == 0)
				{
					g_minispeed_count++;
				}
				else
				{
				  g_InstantSpeedSum += g_Speed;
				
					if((g_speed_count - g_minispeed_count) >60)
					{
					   g_speed_count = 60;						
					}
					if(((g_speed_count-g_minispeed_count) >= 1) && ((g_speed_count-g_minispeed_count) <= 60))
					{
					    per_sec_speed[g_speed_count - g_minispeed_count - 1] = g_Speed;
					}
				}				
			}	
			if(g_speed_count==g_minispeed_count)
			 {
				 g_minuMeanSpeed = 0;
			 }
			else
			 {
			  if((g_speed_count - g_minispeed_count)<50)
			  {
			     g_minuMeanSpeed = g_InstantSpeedSum / (g_speed_count - g_minispeed_count);
			  }
			  else
			  {
                max_size = ((g_speed_count-g_minispeed_count) > 60)?60: g_speed_count-g_minispeed_count;
                 track_bubble_sort(per_sec_speed,max_size);
                  g_InstantSpeedSum = 0;
				 for(i = 0;i<(max_size-8);i++)
				 	{
                       g_InstantSpeedSum+=per_sec_speed[i+4];
                      
				    }

                   g_minuMeanSpeed = g_InstantSpeedSum / (max_size - 8);
			    }
			 }
		
		}
	}
}



uint32_t DistanceResult(void)
{
    return (uint32_t)(gps_data_distance_sm.distance_sum*100.f+0.5f);
}

uint16_t SpeedResult(void)
{
	if ((g_DistanceInput.activity_type == ACT_CYCLING) || (g_DistanceInput.activity_type == ACT_TRIATHLON_CYCLING))
	{
		//10*km/h
		if (g_Speed <= SPEED_MINLIMIT)
		{
			return 0;
		}
		else if (g_Speed >= CYCLING_SPEED_MAXLIMIT)
		{
			return (uint16_t)(CYCLING_SPEED_MAXLIMIT * 36.f + 0.5f);
		}
		else
		{
			return (uint16_t)(g_Speed * 36.f + 0.5f);
		}
	}
	else if ((g_DistanceInput.activity_type == ACT_WALK) || (g_DistanceInput.activity_type == ACT_HIKING) || (g_DistanceInput.activity_type == ACT_CROSSCOUNTRY)
		|| (g_DistanceInput.activity_type == ACT_RUN) || (g_DistanceInput.activity_type == ACT_MARATHON) || (g_DistanceInput.activity_type == ACT_TRIATHLON_RUN) 
	  || (g_DistanceInput.activity_type == ACT_INDOORRUN) || (g_DistanceInput.activity_type == ACT_TRAIN_PLAN_RUN))
	{
		//s/km
		if (g_Speed <= SPEED_MINLIMIT)
		{
			return 0;
		}
		else if (g_Speed >= RUN_SPEED_MAXLIMIT)
		{
			return (uint16_t)(1000.f / RUN_SPEED_MAXLIMIT + 0.5f);
		}
		else
		{
			return (uint16_t)(1000.f / g_Speed + 0.5f);
		}
	}
	else if (g_DistanceInput.activity_type == ACT_CLIMB)
	{
		//10*m/h
		return (uint16_t)(g_Speed * 36000.f + 0.5f);
	}
	else if ((g_DistanceInput.activity_type == ACT_SWIMMING) || (g_DistanceInput.activity_type == ACT_TRIATHLON_SWIM))
	{
		//  s/100m
		if (g_Speed == 0)
		{
			return 0;
		}
		else
		{
			return (uint16_t)(100.f / g_Speed + 0.5f);
		}
	}
	else
	{
		return 0;
	}
}

uint16_t MeanSpeedResult()
{
	if ((g_DistanceInput.activity_type == ACT_CYCLING) || (g_DistanceInput.activity_type == ACT_TRIATHLON_CYCLING))
	{
		//10*km/h
		if (g_minuMeanSpeed <= SPEED_MINLIMIT)
		{
			return 0;
		}
		else
		{
			return (uint16_t)(g_minuMeanSpeed*36.f + 0.5f);
		}
	}
	else if ((g_DistanceInput.activity_type == ACT_WALK) || (g_DistanceInput.activity_type == ACT_HIKING) || (g_DistanceInput.activity_type == ACT_CROSSCOUNTRY)
		|| (g_DistanceInput.activity_type == ACT_RUN) || (g_DistanceInput.activity_type == ACT_MARATHON) || (g_DistanceInput.activity_type == ACT_TRIATHLON_RUN) 
	  || (g_DistanceInput.activity_type == ACT_INDOORRUN) || g_DistanceInput.activity_type == ACT_TRAIN_PLAN_RUN)
	{
		//s/km
		if (g_minuMeanSpeed <= SPEED_MINLIMIT)
		{
			return 0;
		}
		else if (g_minuMeanSpeed >= RUN_SPEED_MAXLIMIT)
		{
			return (uint16_t)(1000.f / RUN_SPEED_MAXLIMIT + 0.5f);
		}
		else
		{
			return (uint16_t)(1000.f / g_minuMeanSpeed + 0.5f);
		}
	}
	else if (g_DistanceInput.activity_type == ACT_CLIMB)
	{
		//10*m/h
		return (uint16_t)(g_minuMeanSpeed * 36000.f + 0.5f);
	}
	else if ((g_DistanceInput.activity_type == ACT_SWIMMING) || (g_DistanceInput.activity_type == ACT_TRIATHLON_SWIM))
	{
		//  s/100m
		if (g_minuMeanSpeed == 0)
		{
			return 0;
		}
		else
		{
			return (uint16_t)(100.f / g_minuMeanSpeed + 0.5f);
		}
	}
	else
	{
		return 0;
	}

}

uint32_t StepStrideResult()
{
    return (uint32_t)(g_stepstride * 100.f + 0.5f);
}

void steps_freq()
{
	int liv_i;
	
	if (step_queue_data.rear < TIME_THS)
	{
	     step_queue_data.inst_step[step_queue_data.rear] = g_DistanceInput.steps;
		 step_queue_data.rear ++;
		 
	   if(step_queue_data.rear < (TIME_THS/2));
	  
	   else
	   	{
	   	  	//停止判断
			if(g_stepthreshold >= STATIC_STEPS_THRESHOLD)
			{
				inst_step_freq = 0;
			}
           else
           	{
				if (g_DistanceInput.steps > step_queue_data.inst_step[0])
				{
					inst_step_freq = (uint16_t)(((g_DistanceInput.steps - step_queue_data.inst_step[0]) * 60.0 / (step_queue_data.rear-1)) + 0.5f);
				}
				else
				{
					inst_step_freq = 0;
				}
           	}

	    }
     
		
	   
		
	}
	else 
	{
	    for (liv_i = 0; liv_i < TIME_THS - 1; liv_i++)
		{
			step_queue_data.inst_step[liv_i] = step_queue_data.inst_step[liv_i + 1];
		}
		step_queue_data.inst_step[step_queue_data.rear - 1] = g_DistanceInput.steps;
		//停止判断
		if(g_stepthreshold >= STATIC_STEPS_THRESHOLD)
		{
			inst_step_freq = 0;
		}
		else
		{
			if (g_DistanceInput.steps > step_queue_data.inst_step[0])
			{
				inst_step_freq = (uint16_t)(((g_DistanceInput.steps - step_queue_data.inst_step[0]) * 60.0 / (TIME_THS - 1)) + 0.5f);
			}
			else
			{
				inst_step_freq = 0;
			}
		}
		
	}
         step_queue_data.temp_sequence = step_queue_data.temp_sequence%2;
		 step_queue_temp_save_data[step_queue_data.temp_sequence] =  inst_step_freq;

	if((step_queue_temp_save_data[0]==0)||(step_queue_temp_save_data[1]==0))
		{
            
         inst_step_freq =  step_queue_temp_save_data[step_queue_data.temp_sequence];

	   }
    else
    	{

           if(abs(step_queue_temp_save_data[0] - step_queue_temp_save_data[1])<=3)
           	{
              
			  inst_step_freq =	step_queue_temp_save_data[step_queue_data.temp_sequence];


		     }
		   else if((abs(step_queue_temp_save_data[0] - step_queue_temp_save_data[1])>3)&&(abs(step_queue_temp_save_data[0] - step_queue_temp_save_data[1])<=7))
		   	{


              
			  inst_step_freq =(uint16_t)(0.5*step_queue_temp_save_data[step_queue_data.temp_sequence] + 0.5*step_queue_temp_save_data[(step_queue_data.temp_sequence+1)%2] + 0.5f);

		   }
           
		   else if((abs(step_queue_temp_save_data[0] - step_queue_temp_save_data[1])>7)&&(abs(step_queue_temp_save_data[0] - step_queue_temp_save_data[1])<=15))
					{
		   
		   
					  
					  inst_step_freq = (uint16_t)(0.45*step_queue_temp_save_data[step_queue_data.temp_sequence] + 0.55*step_queue_temp_save_data[(step_queue_data.temp_sequence+1)%2] + 0.5f);
		   
				   }
             else if((abs(step_queue_temp_save_data[0] - step_queue_temp_save_data[1])>15)&&(abs(step_queue_temp_save_data[0] - step_queue_temp_save_data[1])<=28))
             	{

                 
				 inst_step_freq =(uint16_t)(0.4*step_queue_temp_save_data[step_queue_data.temp_sequence] + 0.6*step_queue_temp_save_data[(step_queue_data.temp_sequence+1)%2] + 0.5f);

			   }
            else
            	{

                 
				 inst_step_freq =(uint16_t)(0.35*step_queue_temp_save_data[step_queue_data.temp_sequence] + 0.65*step_queue_temp_save_data[(step_queue_data.temp_sequence+1)%2] + 0.5f);

			   }

	   }

	    
		step_queue_data.temp_sequence++;
}

uint16_t get_steps_freq()
{
	return inst_step_freq;
}
