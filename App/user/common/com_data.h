#ifndef COM_DATA_H
#define	COM_DATA_H

#include "task_display.h"
#include "bsp_rtc.h"
#include "watch_config.h"
#ifdef COD
#define		FLASH_USED_FLAG					      ((uint32_t)0xA5A5A5A5)
#else
#define		FLASH_USED_FLAG					      ((uint32_t)0xA6A6A6A6)
#endif
#define		SET_VALUE_ADDRESS				      ((uint32_t)0x00000000)
#define		DAY_MAIN_DATA_START_ADDRESS		((uint32_t)0x00001000)
#define		DAY_MAIN_DATA_SIZE				    ((uint32_t)0x00002000)

/**
日期+索引	0时	1时	2时	...	23时	    数据块大小	  地址	
第一天	 日常总步数					  4*24+4=100	  0x04	一天的存储大小0x2EC
	       日常总卡路里					4*24=96	      0x64	
	       日常总距离				    4*24=96	      0xC4	
	       活动总步数				    4*24=96	      0x124	
	       活动总卡路里					4*24=96	      0x184	
	       活动总距离				    4*24=96	      0x1E4	
	       基础心率				    	1*24=24	      0x244	
	       整点温度				    	2*24=48	      0x25C	
	       气压				        	4*24=96	      0x28C	
				 最大摄氧量           2*24=48       0x2EC
				 测定乳酸阈速度       2*24=48       0x31C
         测定乳酸阈心率值     1*24=24       0x34C
				 
				 活动数据索引表  第一个活动 从DAY_COMMON_DATA_SIZE开始，单个活动大小 sizeof(ActivityDataStr),
				                 第二个活动 地址DAY_COMMON_DATA_SIZE+sizeof(ActivityDataStr),依次类推
				 
				 
日期+索引	0时	1时	2时	...	23时	    数据块大小	  地址	
第二天	 日常总步数					  4*24+4=100	  0x04	一天的存储大小0x2EC
	       日常总卡路里					4*24=96	      0x64	
	       日常总距离				    4*24=96	      0xC4	
	       活动总步数				    4*24=96	      0x124	
	       活动总卡路里					4*24=96	      0x184	
	       活动总距离				    4*24=96	      0x1E4	
	       基础心率				    	1*24=24	      0x244	
	       整点温度				    	2*24=48	      0x25C	
	       气压				        	4*24=96	      0x28C	
				 最大摄氧量           2*24=48       0x2EC
				 测定乳酸阈速度       2*24=48       0x31C
         测定乳酸阈心率值     1*24=24       0x34C
...								
第七天	 日常总步数					  4*24+4=100	  0x04	一天的存储大小0x2EC
	       日常总卡路里					4*24=96	      0x64	
	       日常总距离				    4*24=96	      0xC4	
	       活动总步数				    4*24=96	      0x124	
	       活动总卡路里					4*24=96	      0x184	
	       活动总距离				    4*24=96	      0x1E4	
	       基础心率				    	1*24=24	      0x244	
	       整点温度				    	2*24=48	      0x25C	
	       气压				        	4*24=96	      0x28C			
				 最大摄氧量           2*24=48       0x2EC
				 测定乳酸阈速度       2*24=48       0x31C
         测定乳酸阈心率值     1*24=24       0x34C
								                                  七天存储总和0x1474
**/
#define		DAY_COMMON_STEPS_OFFSET				    0x04	  //日常总步数存储偏移地址
#define		DAY_COMMON_ENERGY_OFFSET				  0x64	  //日常总卡路里存储偏移地址
#define		DAY_COMMON_DISTANCE_OFFSET				0xC4	  //日常总距离存储偏移地址
#define		DAY_SPORTS_STEPS_OFFSET           0x124   //活动总步数存储偏移地址
#define		DAY_SPORTS_ENERGY_OFFSET          0x184   //活动总卡路里存储偏移地址
#define		DAY_SPORTS_DISTANCE_OFFSET        0x1E4   //活动总距离存储偏移地址
#define   DAY_BASE_HDR_OFFSET               0x244   //基础心率存储偏移地址
#define		DAY_TEMPRATURE_OFFSET			        0x25C	  //整点温度存储偏移地址
#define   DAY_PRESSURE_OFFSET               0x28C   //气压存储偏移地址
#define   DAY_VO2MAXMEASURING_OFFSET        0x2EC   //最大摄氧量偏移地址
#define   DAY_LTSPEEDMEASURING_OFFSET       0x31C   //测定乳酸阈速度 
#define   DAY_LTBMPMEASURING_OFFSET         0x34C   //测定乳酸阈心率值

#define   DAY_COMMON_DATA_SIZE              0x364   //每天(24小时)存储的日常数据大小
#define   DAY_COMMON_ACTIVITY_DATA_SIZE     (DAY_COMMON_DATA_SIZE + sizeof(ActivityDataStr))//每天的日常数据+活动数据总和大小

#define		DAY_COMMON_STEPS_LENGTH				    4
#define		DAY_COMMON_DISTANCE_LENGTH				4
#define		DAY_COMMON_ENERGY_LENGTH				  4	
#define		DAY_SPORTS_STEPS_LENGTH           4
#define		DAY_SPORTS_ENERGY_LENGTH          4
#define		DAY_SPORTS_DISTANCE_LENGTH        4
#define   DAY_BASE_HDR_LENGTH               1
#define   DAY_PRESSURE_LENGTH               4
#define		DAY_TEMPRATURE_LENGTH			        2
#define   DAY_VO2MAXMEASURING_LENGTH        2   //最大摄氧量偏移地址
#define   DAY_LTSPEEDMEASURING_LENGTH       2   //测定乳酸阈速度 
#define   DAY_LTBMPMEASURING_LENGTH         1  //测定乳酸阈心率值


/*
计步数据存储区,每个数据4字节，为当前总步数，单位为 步，每分钟存储一个数据;
当活动类型为骑行时存储数据为踏频，游泳时存储数据泳速
*/
#ifdef COD
//咕咚需要存储时间戳，扩大为16k
#define		STEP_DATA_START_ADDRESS			    ((uint32_t)0x0000F000)  //计步数据开始地址      //activity
#define		STEP_DATA_STOP_ADDRESS			    ((uint32_t)0x00012FFF)  //计步数据结束地址      //activity

/*
心率数据存储区,每个数据一字节，单位为次/分，每分钟存储一个数据(一分钟平均心率);
*/
//咕咚需要存储时间戳,且20一次，扩大为48k
#define		HEARTRATE_DATA_START_ADDRESS	    ((uint32_t)0x0002C000)  //心率数据开始地址    //activity
#define		HEARTRATE_DATA_STOP_ADDRESS	      ((uint32_t)0x00037FFF)  //心率数据结束地址    //activity

#else 
#define		STEP_DATA_START_ADDRESS			    ((uint32_t)0x0000F000)  //计步数据开始地址      //activity
#define		STEP_DATA_STOP_ADDRESS			    ((uint32_t)0x00010FFF)  //计步数据结束地址      //activity

/*
心率数据存储区,每个数据一字节，单位为次/分，每分钟存储一个数据(一分钟平均心率);
*/
#define		HEARTRATE_DATA_START_ADDRESS	    ((uint32_t)0x00011000)  //心率数据开始地址    //activity
#define		HEARTRATE_DATA_STOP_ADDRESS	      ((uint32_t)0x00012FFF)  //心率数据结束地址    //activity
#endif
/*
气压高度数据存储区,每个数据包括气压和高度两个值，每个值四个字节，气压为4字节浮点数，单位:hPa(百帕)
高度值为四字节浮点数，单位为米;
5分钟存储一个数据
*/
#define		PRESSURE_DATA_START_ADDRESS		    ((uint32_t)0x00013000)  //气压数据开始地址    //activity//咕咚项目实际用于公里牌、、泳姿点
#define		PRESSURE_DATA_STOP_ADDRESS		    ((uint32_t)0x00014FFF)  //气压数据结束地址    //activity

#define		DISTANCE_DATA_START_ADDRESS		    ((uint32_t)0x00015000)  //距离数据开始地址    //activity
#define		DISTANCE_DATA_STOP_ADDRESS		    ((uint32_t)0x00016FFF)  //距离数据结束地址    //activity

#define		ENERGY_DATA_START_ADDRESS		    ((uint32_t)0x00017000)  //热量数据开始地址        //activity
#define		ENERGY_DATA_STOP_ADDRESS		    ((uint32_t)0x00018FFF)  //热量数据结束地址        //activity

#define		WEATHER_DATA_START_ADDRESS			((uint32_t)0x00019000)	//天气存储开始地址        
#define		SPEED_DATA_START_ADDRESS		    ((uint32_t)0x0001B000)  //速度数据开始地址        //activity
#define		SPEED_DATA_STOP_ADDRESS		      ((uint32_t)0x0001CFFF)  //速度数据结束地址        //activity

/*
距离标点存储区,每个点数据为8字节，经度和纬度各占4字节，为带符号整数，实际值的1000000倍,泳池游泳为此数据

*/
#ifdef COD
#define		DISTANCEPOINT_START_ADDRESS		    ((uint32_t)0x00038000)  //暂停时间开始地址,咕咚实际用于gps点距离存储
#define		DISTANCEPOINT_STOP_ADDRESS		      ((uint32_t)0x00077FFF)  //暂停时间结束地址 ，咕咚实际用于gps点距离存储
#define 	DISTANCEPOINT_COD_SIZE 			  ((uint32_t)0x00040000)
#else
#define		DISTANCEPOINT_START_ADDRESS	        ((uint32_t)0x0001D000)  //距离标点开始地址    //activity
#define		DISTANCEPOINT_STOP_ADDRESS	        ((uint32_t)0x0001EFFF)  //距离标点结束地址    //activity
#endif


#if defined WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
#define		CIRCLETIME_START_ADDRESS		    ((uint32_t)0x00400000)  //计圈时间开始地址        //activity
#define		CIRCLETIME_STOP_ADDRESS		        ((uint32_t)0x004FFFFF)  //计圈时间结束地址        //activity
#else
#define		CIRCLETIME_START_ADDRESS		    ((uint32_t)0x0001F000)  //计圈时间开始地址        //activity
#define		CIRCLETIME_STOP_ADDRESS		      ((uint32_t)0x00020FFF)  //计圈时间结束地址        //activity
#endif

#define		PAUSE_DATA_START_ADDRESS		    ((uint32_t)0x00021000)  //暂停时间开始地址 ，实际用于存储航迹打点数据      //activity
#define		PAUSE_DATA_STOP_ADDRESS		      ((uint32_t)0x00022FFF)  //暂停时间结束地址 ，实际用于存储航迹打点数据      //activity

#define		AMBIENT_DATA_START_ADDRESS			((uint32_t)0x00023000)	//环境监测数据

#define		RESERVE1_DATA_START_ADDRESS		    ((uint32_t)0x00024000)  //保留1区域开始地址
#ifdef COD
#define		ALL_DAY_STEPS_START_ADDRESS		    ((uint32_t)0x00024000)  //全天计步开始地址
#define		ALL_DAY_STEPS_END_ADDRESS		    ((uint32_t)0x0002AFFF)  //全天计步结束地址
#define		ALL_DAY_STEPS_SIZE		    		((uint32_t)0x00001000)  //全天计步每天存储大小

#define		COD_BLE_DEV_START_ADDRESS		    ((uint32_t)0x0002B000)  //咕咚蓝牙设置信息开始地址
#define		COD_BLE_DEV_END_ADDRESS		    	((uint32_t)0x0002BFFF)  //咕咚蓝牙设置信息结束地址
#define		COD_BLE_DEV_SIZE		    		((uint32_t)0x00001000)  //咕咚蓝牙设置信息存储大小

#endif 
#define		GPS_DATA_START_ADDRESS			    ((uint32_t)0x00100000)  //GPS数据开始地址         //activity
#define		GPS_DATA_STOP_ADDRESS			      ((uint32_t)0x001FFFFF)  //GPS数据结束地址         //activity

#define		GPSTRACK_DATA_START_ADDRESS			((uint32_t)0x00200000) //共享轨迹GPS数据开始地址  //app

#define		SATELLITE_DATA_START_ADDRESS		((uint32_t)0x00278000)	//星历数据开始地址        //app


#define		REALTIME_STEP_START_ADDRESS		    ((uint32_t)0x0029C000)	//实时步数开始地址    //not app
#define		CUSTOMTHEME_START_ADDRESS		    ((uint32_t)0x002A0000)	//自定义表盘开始地址     //app
#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
#define		VERTICALSIZE_START_ADDRESS		    ((uint32_t)0x00400000)	//垂直幅度开始地址    //activity
#define		VERTICALSIZE_STOP_ADDRESS		      ((uint32_t)0x004FFFFF)	//垂直幅度结束地址    //activity
#endif
#define		TOUCHDOWN_START_ADDRESS		        ((uint32_t)0x00500000)	//触地时间开始地址     //activity
#define		TOUCHDOWN_STOP_ADDRESS		        ((uint32_t)0x005FFFFF)	//触地时间结束地址     //activity

#define		NOTIFY_DATA_START_ADDRESS		    ((uint32_t)0x00600000)	//通知记录开始地址
#define		TRAINING_DATA_START_ADDRESS		    ((uint32_t)0x00619000)	//训练计划开始地址      //app 手机到手表
#define   TRAINING_DATA_UPLOAD_ADDRESS      ((uint32_t)0x00629000)	//训练计划需要上传的数据的开始地址   //app 手表到手机

#define		TIME_CALIBRATION_ADJ_DATA_START_ADDRESS		    ((uint32_t)0x00639000)  //精密时间 Adj值及时间等参数保存 恢复出厂不可删除  不可擦写
#define		TIME_CALIBRATION_VALUE_DATA_START_ADDRESS		  ((uint32_t)0x0063A000)  //精密时间 Tcalibration值及时间等参数保存 恢复出厂不可删除 可擦写
#define		TIME_CALIBRATION_STSTUS_START_ADDRESS		  ((uint32_t)0x0063B000)//精密时间 状态值保存恢复出厂不可删除 可擦写
#if defined  WATCH_IMU_CALIBRATION
#define		ACC_CALIBRATION_PARAMETER_STORAGE_ADDRESS		  ((uint32_t)0x00651000)//加速度校准参数保存恢复出厂不可删除 可擦写
#define		GYRO_CALIBRATION_PARAMETER_STORAGE_ADDRESS		  ((uint32_t)0x00652000)//陀螺仪校准参数保存恢复出厂不可删除 可擦写
#endif
#define		MAGN_CALIBRATION_PARAMETER_STORAGE_ADDRESS		  ((uint32_t)0x00655000)//磁力计校准参数保存恢复出厂不可删除 可擦写

#define		UPGRADE_DATA_START_ADDRESS		    ((uint32_t)0x00700000)  //升级存储数据开始地址

/* 
运动开关mask
bit位定义如下:
0:心率告警 1:配速提醒2:距离提醒3自动计圈距离4:目标圈数提醒5:燃脂提醒6:目标时间7:高度提醒
*/
#define SW_HEARTRATE_MASK       0x01
#define SW_PACE_MASK            0x02
#define SW_DISTANCE_MASK        0x04
#define SW_CIRCLEDISTANCE_MASK  0x08
#define SW_GOALCIRCLE_MASK      0x10
#define SW_GOALKCAL_MASK        0x20
#define SW_GOALTIME_MASK        0x40
#define SW_ALTITUDE_MASK        0x80




#define LOCK_SCREEN_TIME   59     //自动锁屏时间

#define AMBIENT_DATA_MAX_NUM	90		//环境监测数据最大存储总数


//#define	DEFAULT_SPORT_MODE  0x3E5

#define	DEFAULT_SPORT_MODE  0x3F


//语言
enum
{
	L_CHINESE	= 0U,	//中文
	L_ENGLISH	= 1U	//英语
	
};
enum
{

  THEME_SIMPLENUMBER = 0U,//简约数字
  THEME_SIMPLEBUSINESS = 1U,//简约商务
	THEME_VINTAGEBUSINESS = 2U,//复古商务
	THEME_APPDOWNLOAD = 3U,//APP下载主题

};

typedef struct __attribute__((packed))
{
	uint8_t Hour;
	uint8_t Minute;
}TrainPlanHint;
typedef struct __attribute__((packed))
{
	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
}TrainPlanSport;
typedef enum
{
	Rest = 0U,        //休息  
	NORMAL_RUN,      //正常跑  暂时没使用
	PACE_RUN,        //配速跑  暂时没使用
	TEMPO_RUN,       //节奏跑  暂时没使用
	INTERVALS,       //间隙跑  暂时没使用
	AEROBIC_RUN,     //有氧跑  暂时没使用
	EASY_RUN,        //轻松跑  暂时没使用
	WALK_AND_RUN,    //走跑结合 暂时没使用
	BRISK_WALK,      //快走    暂时没使用
	SLOW_WALK,       //慢走    暂时没使用
	STRENGTH_TRAIN,  //力量训练 暂时没使用
	CROSS_TRAIN,     //交叉训练 暂时没使用
	CYCLING,         //骑行    暂时没使用
	SWIM,            //游泳    暂时没使用
	
	StrengthTraining,        //力量训练
	CrossTraining,           //交叉训练
	Rest_StrengthTraining,   //休息或力量训练
	Rest_CrossTraining,      //休息或交叉训练
	EasyToRun_Distance,     //轻松跑-距离
	EasyToRun_Time,          //轻松跑-时间
	NormalRun_Distance,      //跑步-距离
	NormalRun_Time,          //跑步-时间
	TempoRuns_Distance,      //节奏跑-距离
	TempoRuns_Time,          //节奏跑-时间
	PacetoRun_Distance,      //配速跑-距离
	PacetoRun_Time,          //配速跑-时间
	Intervals_Distance,      //间歇训练-距离
	Intervals_Time,          //间歇训练-时间
	Marathon_Distance,       //马拉松-距离
	Rest_Run_Distance,       //休息或跑步-距离
}TrainPlanType;

#define TRAIN_PLAN_MAX_DAYS        126
#define TRAIN_PLAN_DOWNLOAD_DATA_SIZE  (sizeof(TrainPlanDownStr)+sizeof(DownloadStr)*TRAIN_PLAN_MAX_DAYS)  //训练计划 app下发到手表的数据大小
typedef struct __attribute__((packed))
{
	TrainPlanType  Type;           //活动类型              0-休息1力量训练2交叉训练3休息或力量训练4休息或交叉训练5轻松跑-距离6轻松跑-时间7跑步-距离8跑步-时间9节奏跑-距离10节奏跑-时间11...
	char ContentStr[20];           //训练内容
	char SkillsStr[30];            //训练技巧
	uint16_t TargetHour;           //训练标准-时长              Min分钟，倒计时时间，比如“5分钟节奏跑”，则目标时长为5；等于0表示不做时间限制
	uint16_t TargetDistance;       //训练标准-里程            m米，倒数距离，比如“5公里轻松跑”，则目标距离为5000；等于0表示不做距离限制
	uint16_t TargetCalories;       //训练标准-卡路里            单位:卡
	uint16_t TargetSpeedLimitDown; //训练标准-配速下限          s/km秒/公里 ，等于0表示没有配速下限
	uint16_t TargetSpeedLimitUp;   //训练标准-配速上限         s/km秒/公里 ，等于0表示没有配速上限
	uint16_t RestTime;             //间歇跑/走跑等休息时间 单位:分
	uint8_t  IntermittentRun;      //单位:次  0代表没有间歇，例如2代表，间歇2次，单次时长=目标时长/(2)-间歇时长
	uint8_t Reserved[16];          //预留16个字节
}DownloadStr;//训练计划每天参数  APP下发到手表端


typedef struct __attribute__((packed))
{
	uint8_t  IsComplete;           //完成标志              0 未完成    1已完成
	uint8_t  RecordIndex;          //训练记录活动序号      1-40 ，训练计划数据的存储按照活动存储，这里记录存储的活动序号，用于链接训练记录的数据
	uint16_t Hour;                 //完成-时长      单位秒
	uint16_t Distance;             //完成-里程    单位米
	uint16_t Calories;             //完成-卡路里   单位卡
	uint8_t Reserved[16];          //预留16个字节
}UploadStr;//训练计划每天参数  手表端上传到APP端


typedef struct __attribute__((packed))
{
	uint8_t Md5Str[32];           //MD5 字符串
	uint8_t UUIDStr[32];           //UUID 字符串
	char NameStr[20];             //名称
	TrainPlanHint Hint;           //提醒时间 时分(24小时制)
	TrainPlanSport StartTime;     //年/月/日
	TrainPlanSport StopTime;      //年/月/日
	uint8_t Reserved[16];         //预留16个字节
}TrainPlanDownStr;//训练计划app下发数据头


typedef struct __attribute__((packed))
{
	uint8_t Md5Str[32];           //MD5 字符串
	uint8_t UUIDStr[32];           //UUID 字符串
	uint8_t Reserved[16];         //预留16个字节
}TrainPlanUploadStr;//训练计划手表上传数据头



//训练计划数据结果上传 --给蓝牙用
typedef struct __attribute__((packed))
{
	uint32_t Address;//从address地址开始起
	uint32_t Size;   //大小
	uint16_t Days;   //上传的天数
	uint8_t Is_HasTodayResult;//是否有数据上传 0 没有数据上传 1有数据上传
}TrainPlanUploadBLEData;
typedef struct __attribute__((packed))
{
	uint8_t Md5Str[32];           //MD5 字符串
	uint8_t UUIDStr[32];           //UUID 字符串
}_com_config_train_plan_MD5_UUID;

typedef struct __attribute__((packed))
{
	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
	uint8_t Msecond;//百毫秒
}TimeStr;
//精密时间 Adj值及时间等参数保存 恢复出厂不可删除  不可擦写
typedef struct __attribute__((packed))
{
	uint8_t IsWrite;//是否写入 1写入 0 未写入
	TimeStr AdjSaveTime;//保存Adj值的时间
	TimeStr GpsFirstTime;//保存Adj值的上一次GPS的有效授时，且满足时间AdjSaveTime-GpsFirstTime在[24,30*24]之间
	int32_t AdjValue;//Adj保存的值
	uint8_t Reserved[12];
}TimeAdjStr;
//精密时间 Tcalibration值及时间等参数保存 恢复出厂不可删除 可擦写
typedef struct __attribute__((packed))
{
	uint8_t IsWrite;//是否写入 1写入 0 未写入
	TimeStr CalibrationSaveTime;//保存Calibration值的时间
	TimeStr GpsSecondTime;//保存Calibration值的上一次GPS的有效授时，且满足CalibrationSaveTime-GpsSecondTime在[24,30*24]之间
	int64_t CalibrationValue;//calobration保存的值
	uint8_t Reserved[12];
}TimeCalibrationStr;
//精密时间 状态保存 存在区域3中
typedef struct __attribute__((packed))
{
	uint32_t status;//状态
	uint8_t Reserved[12];
}TimeCalibrationStatus;
#ifdef COD
typedef struct __attribute__ ((packed)){
	uint32_t command;
	uint8_t gender;  //性别
	uint8_t age;
	uint8_t height;         //身高:单位CM
	uint8_t weight;
}_ble_user_info;

/**
 * 设备当前的绑定状态
 * */
typedef enum {
    STATUS_BINDING,
    STATUS_BIND_SUCCESS,
    STATUS_BIND_FAIL
} cod_bind_status;
/**
 * 运动状态
 */
typedef enum{
    COD_SPORT_STATUS_START = 0,      //运动开始
    COD_SPORT_STATUS_SPORTING,       //运动中
    COD_SPORT_STATUS_PAUSE,          //运动暂停
    COD_SPORT_STATUS_STOP,           //运动结束
} cod_ble_sport_status;
/**
 * 运动类型
 */
typedef enum{
    SPORT_TYPE_OUTDOOR_RUN = 1,    //户外跑
    SPORT_TYPE_OUTDOOR_WALK,       //户外健走(徒步越野)
    SPORT_TYPE_RIDE,               //骑行
    SPORT_TYPE_CLIMB,              //登山
    SPORT_TYPE_INDOOR_RUN,         //室内跑
    SPORT_TYPE_INDOOR_WALK,        //室内健走
    SPORT_TYPE_SKI,                //滑雪
    SPORT_TYPE_SKATE,              //滑冰
} cod_ble_sport_type;


/**
 * 训练类型
 */
typedef enum{
    TRAINING_TYPE_UNKNOWN = 0,      //未知
    TRAINING_TYPE_NORMAL,           //普通训练
    TRAINING_TYPE_INTERACT,         //互动式课程
    TRAINING_TYPE_LIVE,             //直播训练
} cod_ble_training_type;

/**
 * 训练状态
 */
typedef enum{
    TRAINING_STATUS_UNKNOWN = 0,        //未知
    TRAINING_STATUS_PREPARE,            //准备
    TRAINING_STATUS_START,              //开始
    TRAINING_STATUS_INTRODUCT,          //讲解
    TRAINING_STATUS_TRAINING,           //训练中
    TRAINING_STATUS_REST,               //休息
    TRAINING_STATUS_PAUSE,              //暂停
    TRAINING_STATUS_RESUME,             //恢复
    TRAINING_STATUS_STOP,               //结束
} cod_ble_training_status;

/**
 * 投屏结果
 */
typedef enum{
    COD_BLE_CAST_RESULT_OK = 0,          //投屏成功
    COD_BLE_CAST_RESULT_NOT_SUPPORT = 1, //不支持投屏
    COD_BLE_CAST_RESULT_SPORTING = 0xff, //当前正在独立运动，则回复错误码0xFF
} cod_ble_cast_result;


/**
 * 训练动作类型
 */
typedef enum{
    TRAINING_ACTION_TYPE_TIME = 1,      //时间
    TRAINING_ACTION_TYPE_COUNT,         //个数
} cod_ble_training_action_type;

/**
 * 运动投屏信息
 */
typedef struct {
    cod_ble_sport_status sport_status;
    cod_ble_sport_type sport_type;
    float distance;                 //米
    uint32_t time;                  //时长，秒
    uint16_t pace;                  //配速，秒
    uint8_t heart_rate;             //实时心率
    uint8_t avg_heart_rate;         //平均心率
    uint32_t step;                  //步数
    float rt_speed;                 //实时速度, 千米/小时
    float avg_speed;                //平均速度, 千米/小时
    float calorie;                  //卡路里, 大卡
    int16_t altitude;              //海拔
    int16_t altitude_diff;         //海拔上升
    int16_t max_altitude;          //最高海拔
    uint32_t total_altitude_increase;//累积爬升
    uint32_t climb_distance;        //上坡里程
} cod_ble_sport_cast;


/**
 * 用户训练投屏信息
 */
typedef struct {
    cod_ble_training_status training_status;
    cod_ble_training_type training_type;
    cod_ble_training_action_type action_type;
    char course_name[20];          //课程名称，包括'\0'
    uint16_t course_name_len;   //课程名称长度，包括'\0'
    char action_name[20];          //动作名称，包括'\0'
    uint16_t action_name_len;   //动作名称长度，包括'\0'
    uint16_t action_target;     //动作目标值
    uint16_t action_actual;     //动作实际值
    float calorie;              //卡路里, 大卡
    uint32_t time;              //时长，秒
    uint8_t heart_rate;         //平均心率,结束状态时会传，范围0-255
} _cod_user_training_cast;

//全天计步存储信息
typedef struct  __attribute__ ((packed)){
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint32_t addr;
	uint32_t size;
} _all_day_steps;

extern _all_day_steps all_day_steps[7];
extern uint32_t last_store_com_step; //
extern uint32_t last_store_run_step;
extern uint32_t last_store_walk_step;
extern uint32_t last_store_cal;
extern uint32_t last_store_dis;

extern uint32_t last_sync_com_step; //
extern uint32_t last_sync_run_step ;
extern uint32_t last_sync_walk_step;
extern uint32_t last_sync_cal;
extern uint32_t last_sync_dis ;


typedef struct __attribute__ ((packed)){
    uint8_t weather;        //天气
    int8_t temperature;     //温度
    uint16_t aqi;			//空气质量指数
} cod_ble_weather;

typedef struct __attribute__ ((packed))
{
	uint8_t date[4]; //天气同步时间 :第一个字节年如果2018为18，第二个字节月，第三个字节日期.第四字节为小时
	uint8_t city[10];//城市名，中文用UNICODE最多5个汉字，英文用ASCII码最多有10个英文
	cod_ble_weather cod_weather[24]; //24小时天气
} CodWeatherSync; //APP天气数据结构体
/**
 * 咕咚数据类型
 */
typedef enum{
    DATA_TYPE_STEP = 1,      //全天计步
    DATA_TYPE_SLEEP,         //睡眠数据
    DATA_TYPE_HEART,         //心率数据
    DATA_TYPE_BP,            //血压数据
    DATA_TYPE_SPORT,         //运动数据
    DATA_TYPE_LOG = 10       //日志数据
} cod_ble_data_type;

void all_day_steps_reinit(void);
void Store_all_day_steps(void);
void Store_all_day_time(void);
void read_all_day_steps(void);
void delete_all_day_steps(void);


#endif
//App设置数据格式
typedef struct __attribute__((packed))
{
	uint8_t		Age;						//年龄 //APP设置
	uint8_t		Sex;						//性别  0女  1 男 //APP设置
	uint8_t		Height;						//身高  单位：cm //APP设置
	uint8_t		Weight;						//体重  单位：kg //APP设置
	uint8_t		Stride;						//步长(日常徒步步长) 单位：cm //APP设置
	uint16_t	Goal_Steps;					//目标步数 //APP设置
	uint16_t	Goal_Energy;				//目标卡路里 单位：大卡 //APP设置
	uint8_t		Goal_Distance;				//目标距离   单位：公里 //APP设置
	uint16_t	VO2Max;						//最大摄氧量 单位：ml/kg/min //APP设置
	uint8_t   LactateThresholdHeartrate; //乳酸阈心率值 //APP设置
	uint16_t  LactateThresholdSpeed;    //乳酸阈速度	 //APP设置
	uint8_t   htr_zone_one_min;       //心率区间1低值 //APP设置
	uint8_t   htr_zone_one_max;       //心率区间1高值 //APP设置
	uint8_t   htr_zone_two_min;       //心率区间2低值 //APP设置
	uint8_t   htr_zone_two_max;       //心率区间2低值 //APP设置
	uint8_t   htr_zone_three_min;     //心率区间3低值 //APP设置
	uint8_t   htr_zone_three_max;     //心率区间3低值 //APP设置
	uint8_t   htr_zone_four_min;      //心率区间4低值 //APP设置
	uint8_t   htr_zone_four_max;      //心率区间4低值 //APP设置
	uint8_t   htr_zone_five_min;      //心率区间5低值 //APP设置
	uint8_t   htr_zone_five_max;      //心率区间5低值 //APP设置
	uint32_t  Sea_Level_Pressure;	  //海平面大气压  通过app的天气插件获取
}AppSetStr;

//App获取数据格式
typedef struct __attribute__((packed))
{
	uint8_t    LengthSwimmingPool;            //泳池长度
	uint8_t    StepLengthRun;                 //跑步步长
	uint8_t    StepLengthCountryRace;         //越野步长
	uint8_t    StepLengthMarathon;            //马拉松步长
	uint8_t    StepLengthWalking;             //健走步长
	uint8_t    StepLengthClimbing;            //登山步长
	uint8_t    StepLengthIndoorRun;           //室内跑步长
	uint8_t    StepLengthHike;           	    //徒步越野步长
  uint16_t   VO2MaxMeasuring;               //测定最大摄氧量 单位：ml/kg/min
	uint16_t   LTSpeedMeasuring;              //测定乳酸阈速度  s/km
	uint8_t    LTBmpMeasuring;                //测定乳酸阈心率值 
	uint8_t    CalibrationStride;             //校准步长 单位cm步长 不能超过255cm
	uint8_t  	 Reserved[12];                  //预留
}AppGetStr;


//运动个性设置数据格式
typedef struct  __attribute__((packed))
{
	uint8_t    SwRun;                         //跑步开关(bit 0:心率告警 1:配速提醒2:距离提醒3自动计圈距离4:目标圈数提醒5:燃脂提醒6:目标时间7:高度提醒,下同)
	uint8_t    SwMarathon;                    //马拉松开关
	uint8_t    SwCountryRace;                 //越野跑开关
	uint8_t    SwSwimmg;                      //游泳开关
	uint8_t    SwCycling;                     //骑行开关
	uint8_t    SwClimbing;                    //登山开关
	uint8_t    SwWalking;                     //健走开关
	uint8_t    SwHike;                        //徒步越野开关
	uint8_t    SwIndoorRun;                   //室内跑开关
	uint8_t    SwOutdoorSwimmg;               //室外游泳开关
	
	uint16_t   DefHintSpeedRun;               //跑步配速提醒默认值 单位：秒/公里
	uint16_t   DefHintSpeedCountryRace;       //越野跑配速提醒默认值
	uint16_t   DefHintSpeedMarathon;          //马拉松配速提醒默认值
	uint16_t   DefHintSpeedCycling;           //骑行配速提醒默认值 骑行实际为速度。单位:10X km/h
	uint16_t   DefHintSpeedIndoorRun;         //室内跑配速提醒默认值
	
	uint16_t   DefHintDistanceRun;            //跑步距离提醒默认值 单位:米
	uint16_t   DefHintDistanceCountryRace;    //越野跑距离提醒默认值
	uint16_t   DefHintDistanceMarathon;       //马拉松距离提醒默认值
	uint16_t   DefHintDistanceCycling;        //骑行距离提醒默认值
	uint16_t   DefHintDistanceClimbing;       //登山距离提醒默认值
	uint16_t   DefHintDistanceIndoorRun;      //室内跑距离提醒值

	
	uint16_t   DefCircleDistanceRun;          //跑步默认计圈距离 单位:米
	uint16_t   DefCircleDistanceWalk;         //键走默认计圈距离 
	uint16_t   DefCircleDistanceCountryRace;  //越野跑默认计圈距离
	uint16_t   DefCircleDistanceMarathon;     //马拉松默认计圈距离
	uint16_t   DefCircleDistanceCycling;      //骑行默认计圈距离
	uint16_t   DefCircleDistanceIndoorRun;    //室内跑默认计圈距离
	uint16_t   DefCircleDistanceClimbing;      //登山默认计圈距离
	uint16_t   DefCircleDistanceHike;          //徒步越野默认计圈距离
	uint16_t   DefCircleDistancOutdoorSwimmg;  //室外游泳默认计圈距离

	
	uint16_t   DefHintTimeSwimming;           //游泳默认目标时间 单位:秒
    uint16_t   DefHintTimeWalking;            //健走默认目标认时间
	uint16_t   DefHintTimeOutdoorSwimmg;      //室外游泳默认目标时间

	
	uint16_t   DefWalkingFatBurnGoal;         //健走燃脂目标默认值 单位：大卡
	uint16_t   DefHikeFatBurnGoal;            //徒步越野燃脂目标默认值 
	uint16_t   DefHintClimbingHeight;         //登山高度提醒默认值 单位：米
	uint16_t   WheelDiameter;                 //车轮直径 单位:寸 X10

/*新增*/


 uint16_t   DefHintTimeRun;    //跑步默认目标时间
 uint16_t   DefHintTimeClimbing; //登山默认目标时间
 uint16_t   DefHintTimeCycling; //骑行默认目标时间
 uint16_t   DefHintTimeHike; //徒步越野默认目标时间
 uint16_t   DefHintTimeCountryRace; //徒步越野默认目标时间
 
 uint16_t	DefHintSpeedHike;		   //徒步越野配速提醒默认值


 uint16_t	DefCircleDistancSwimmg;	//游泳默认计圈距离
 uint16_t	DefHintDistanceHike;    //徒步越野距离提醒默认值
 uint16_t	DefHintDistanceSwimmg;	   //游泳距离提醒默认值值

   

 
	uint8_t    DefHintCircleSwimming;         //游泳目标圈数默认值
	uint8_t	   Reserved;
}SportPersonalSetStr;

//当前地址
typedef struct  __attribute__((packed))
{
	uint16_t		Index;							//最后存储的每天总数据索引号
	uint16_t		IndexActivity;	   		        //最后存储的活动序号
	uint32_t		AddressStep;					//最后存储的计步数据的地址
	uint32_t		AddressHeartRate;   	        //最后存储心率数据的地址
	uint32_t		AddressPressure;    	        //最后存储气压数据的地址，//咕咚项目实际用于公里牌、、泳姿点
	uint32_t		AddressDistance;		        //最后存储距离数据的地址
	uint32_t		AddressEnergy;					//最后存储热量数据的地址
	uint32_t		AddressSpeed;					//最后存储速度数据的地址
	uint32_t		AddressGps;						//最后存储GPS数据的地址
	uint32_t		AddressDistancePoint;			//最后存储距离标点的地址
	uint32_t		AddressCircleTime;			    //最后存储计圈时间的地址
	uint32_t    AddressPauseTime;          //最后暂停计圈时间的地址
	uint32_t    AddressGpsTrack;           //最后共享轨迹的地址
	uint32_t    AddressStatelite;          //最后星历的地址
	uint32_t    AddressTrainplan;          //最后训练计划的地址
	uint32_t    AddressTrainplanUpload;    //最后训练计划结果上传的地址
	#ifdef COD 
	uint32_t   		Current_cod_id;				//咕咚记录id
	
	#else
	uint32_t		AddressVerticalSize;			//最后存储垂直幅度的地址
	#endif
	uint32_t		AddressTouchDown;			    //最后存储触地时间的地址
	uint32_t		AddressPause;			        //最后存储暂停时间的地址，实际用于存储航迹打点数据
	ScreenState_t	Screen_Sta;						//最后一次使用的监测功能界面
	uint8_t			PowerOffHour;					//关机时的小时数
	uint8_t			ChangedHour;
	uint8_t			Reserved;
}CurrentAddressStr;

#define ALARM_REPITION_TYPE_SINGLE       0
#define ALARM_REPITION_TYPE_DAILY        1
#define ALARM_REPITION_TYPE_WEEK_DAY     2
#define ALARM_REPITION_TYPE_WEEKEND      3

typedef struct __attribute__((packed)){
	int8_t hour;
	int8_t minute;
} Alarm_time_t;

typedef struct  __attribute__((packed)){
	bool enable;
	Alarm_time_t time;
	uint8_t repetition_type;
	bool buzzer_enable;
	bool motor_enable;
	
} Alarm_t;
//按活动顺序存储当天的活动类型
typedef struct __attribute__((packed))
{	
	uint8_t		ActivityNum;					//活动数量 每次活动存储时加一
	uint8_t		ActivityType;                   //活动类型 健走 跑步 登山 游泳等						  
}TrainRecordIndexStr;
//工具 运动使用次数
typedef struct
{
	uint32_t NumTool;//工具使用的次数
	uint32_t NumSport;//运动使用的次数
}ToolSportRecordNum;
//一周七天  最大值不能超过7
typedef  enum
{
	Sun = 0U,//星期日
	Mon,
	Tues,
	Wed,
	Thur,
	Fri,
	Sat,//星期六
}Index_Week;
//工具 运动等使用项对应的数组序号 最大值不能超过32
typedef enum
{
	Location = 0U, 	//位置
	Compass,		//指北针
	Calendar,       //日历天气
	ambient,        //环境监测
	StopWatch,      //秒表
	FindPhone      	//寻找手机
}Index_UseItem;

typedef enum
{
	Run = 0U,    //跑步
	Marathon,   //马拉松
	CountryRun, //越野跑
	IndoorRun,  //室内跑
	Walking,    //健走
	Climbing,   //登山
	Cycling,    //骑行
	Hiking, 		//徒步
	Swimming,   //游泳	
	Triathlon,  //铁人三项
}Index_UseSport;

typedef struct __attribute__((packed))
{
	uint32_t	CommonSteps;			//日常步数
	uint32_t	CommonEnergy;			//日常卡路里,单位为卡
	uint32_t	CommonDistance;		//日常距离，单位为cm
	uint32_t	SportSteps;			//运动步数
	uint32_t	SportEnergy;			//运动卡路里,单位为卡
	uint32_t	SportDistance;		//运动距离，单位为cm
	uint8_t   BasedHeartrate;  //基础心率
	uint16_t  VO2MaxMeasuring;    //测定最大摄氧量 单位：ml/kg/min
	uint16_t  LTSpeedMeasuring;    //测定乳酸阈速度
	uint8_t   LTBmpMeasuring;     //测定乳酸阈心率值 
} PowerOffMianDataStr; //每天总数据结构体

//铁人三项运动中 运动中同步铁人三项时判断处理
typedef struct __attribute__((packed))
{
	uint8_t        IsTriathlonSport;//是否处于铁人三项运动中 1处于 0不处于
	uint8_t        IndexTriathlon;//记录正在运动时
	uint8_t        year;//正在运动时 年
	uint8_t        month;//正在运动时 月
	uint8_t        day;//正在运动时 日
	uint8_t        hour;
	uint8_t        minute;
	uint8_t        second;
}TriathlonSportMode;

//参数设置数据
typedef struct  __attribute__((packed))
{
	uint32_t      crc;					   //使用标志值
	AppSetStr     AppSet;					//详见app设置数据格式
	AppGetStr     AppGet;					//详见app获取数据格式
	SportPersonalSetStr SportSet;        //详见运动个性设置数据格式
	CurrentAddressStr   CurrentAddress;  //当前地址
	ScreenState_t	ShortcutScreenUP;			  //长按上键的快捷对应界面
	ScreenState_t	ShortcutScreenDown;			//长按下键的快捷对应界面
	ScreenState_t 	ShortcutScreenLight;		//长按背光键的快捷对应界面
	unsigned char 	Language;            		//语言
	Alarm_t         alarm_instance[4];      //4个ALARM闹钟实例
	uint32_t        backlight_timeout_mssec; //背光设置  10s/30s/60s
	uint8_t       	SwVibration;         		//振动开关     0关 1开
	uint8_t       	SwBuzzer;            		//蜂鸣器开关   0关 1开
	uint8_t       	SwRealtimeHdr;       		//实时心率开关 0关 1开
	uint8_t       	SwAutoLockScreen;    		//自动锁屏开关 0关 1开
	uint8_t       	SwBle;               		//蓝牙开关     0关 1开 

	uint8_t       	SwRaiseForLightScreen;		//抬手亮屏开关 0关 1开
	uint8_t       	SwNoDisturb;         		//勿扰模式开关 0关 1开
	uint8_t       	NoDisturbStartHour;  		//勿扰模式开始小时
	uint8_t       	NoDisturbStartMinute;		//勿扰模式开始分钟
	uint8_t       	NoDisturbEndHour;    		//勿扰模式结束小时
	uint8_t       	NoDisturbEndMinute;  		//勿扰模式结束分钟

	uint8_t       	SwBasedMonitorHdr;   		//基础心率监测开关
	uint8_t       	DefBasedMonitorHdrHour;     //基础心率监测默认小时
	uint8_t       	DefBasedMonitorHdrMinute;   //基础心率监测默认分钟
	uint8_t       	Theme;                      //主题
	float			Compass_Offset_x;			//指北针X校准值
	float			Compass_Offset_y;			//指北针Y校准值
	float			Compass_Offset_z;			//指北针Z校准值
	float			Altitude_Offset;			//高度校准值
	

	//自定义工具 bit定义顺序
	union UCustomTool 
	{ 
		uint32_t val;
		struct __attribute__((packed))
		{			
			//uint32_t step:1;       //步数
			//uint32_t calories:1;   //卡路里
			//uint32_t heartrate:1;  //心率
			//uint32_t motions_record:1; //运动记录
			//uint32_t notification:1;      //消息通知
			//uint32_t Calendar:1;       //日历天气
			
   		    uint32_t recover:1;//恢复时间
			uint32_t vo2max:1;//最大摄氧量
			uint32_t threshold:1;//乳酸阈
			uint32_t Countdown:1;		//倒计时
			uint32_t StopWatch:1;      //秒表
        	uint32_t Location:1;       //位置
			uint32_t Compass:1;        //指北针
			uint32_t ambient:1;        //环境监测
		}bits;
	} __attribute__((packed))U_CustomTool;
	//自定义运动 bit定义顺序
	union UCustomSport
	{ 
		uint32_t val;
		struct  __attribute__((packed))
		{
		   uint32_t	Run:1;        //跑步
		   uint32_t Cycling:1;    //骑行
	       uint32_t Climbing:1;   //登山
	       uint32_t Swimming:1;   //游泳	
	       uint32_t Hiking:1;//徒步越野
	       uint32_t CountryRun:1; //越野跑
	     //  uint32_t Marathon:1;   //马拉松
	     //  uint32_t IndoorRun:1;  //室内跑
	      // uint32_t Walking:1;    //健走
	     //  uint32_t Triathlon:1;  //铁人三项

		}bits;
	}__attribute__((packed))U_CustomSport;
	//自定义待机 bit定义顺序
	union UCustomMonitor 
	{
		uint32_t val;
		struct __attribute__((packed))
		{
		    uint32_t step:1;       //步数
			uint32_t calories:1;   //卡路里
			uint32_t heartrate:1;  //心率
			uint32_t motions_record:1; //运动记录
			uint32_t notification:1;      //消息通知
			uint32_t Calendar:1;       //日历天气
	
		}bits;
	}__attribute__((packed))U_CustomMonitor;
	
	TrainRecordIndexStr		TrainRecordIndex[7];		//7天的活动记录存储  数组序号对应见Index_Week
	uint32_t    			ToolRecordNumIndex[32];		//工具使用次数 数组序号对应见Index_UseTool
	uint32_t    			SportRecordNumIndex[32];    //运动使用次数 数组序号对应见Index_UseSport
	uint8_t 				AutoLockScreenFlag;			//自动锁屏标志 0 无锁屏 1已锁屏
	uint8_t         IsAllowBaseHDR;          //是否允许基础心率提醒保存等功能  1允许 0不允许
	uint8_t             TrainPlanHintFlag;    //训练计划是否提醒时间标志 0不需要提醒  1需要提醒
	uint8_t   					TrainPlanFlag;				//训练计划是否开启标志  0 未开启 1开启当app下发消息到手表端时置状态
	uint8_t             TrainPlanNotifyFlag;  //训练计划通知提醒开关 与app上开关提醒功能相呼应
	int32_t        		TimeCalibrationValue;       //精密时间校准补偿
	rtc_time_t				PowerOffTime;				//关机时间
	PowerOffMianDataStr     PowerOffMianData;           //关机运动、日常数据
	uint32_t          RecoverTime;//恢复时间
	uint32_t          RecoverTimeTotal;//仅恢复时间运动后保存,以计算百分比时用
	uint8_t           TrainPlanWriteUploadMD5Flag;//训练计划写上传结果中MD5标志 1已写0未写
	uint8_t					GPS_Gauss_Type;				//GPS高斯坐标类型
	uint8_t         PowerPercentRecord;        //电量百分比，防止电量不准确
	uint8_t         IsFirstCalibrationStep;//是否是第一次计步校准  恢复出厂设置后才认为第一次,用于图标闪烁,校准保存后图标不闪烁 1是 0不是
	uint8_t         IsFirstSport;//是否是第一次运动,为了区分在无运动记录界面,显示不同的提示语    1是 0不是
	TriathlonSportMode TriathlonSport;//是否正在铁人三项运动
	
	
	uint8_t     IsNeedClearTrainPlanDayHint;//是否需要清除训练日提醒界面  如果在提醒界面过了0点需要清除退出到主界面
#if  defined WATCH_COM_SHORTCUNT_UP
	uint8_t     IsMeasuredVO2max;     //最大摄氧量是否测量过  0从未测量 1测量无效 2测量有效
	uint8_t     IsMeasuredLthreshold;     //最大摄氧量是否测量过 0从未测量 1测量无效 2测量有效
	uint32_t    custom_shortcut_up_switch_val;//快捷上键界面切换
#endif
	uint8_t     RunData1;    	//跑步自定义数据显示第一栏 
	uint8_t     RunData2;    	//跑步自定义数据显示第二栏 
	uint8_t    	RunData3;		//跑步自定义数据显示第三栏 
	uint8_t    	RunData4;		//跑步自定义数据显示第四栏 

	uint8_t     WlakData1;    	//健走自定义数据显示第一栏 
	uint8_t     WlakData2;    	//健走自定义数据显示第二栏 
	uint8_t    	WlakData3;		//健走自定义数据显示第三栏 

	uint8_t     MarathonData1;    	//马拉松自定义数据显示第一栏 
	uint8_t     MarathonData2;    	//马拉松自定义数据显示第二栏 
	uint8_t    	MarathonData3;		//马拉松自定义数据显示第三栏 

	uint8_t     IndoorRunData1;    	//室内跑自定义数据显示第一栏 
	uint8_t     IndoorRunData2;    	//室内跑自定义数据显示第二栏 
	uint8_t    	IndoorRunData3;		//室内跑自定义数据显示第三栏 

	uint8_t     SwimRunData1;    	//游泳自定义数据显示第一栏 
	uint8_t     SwimRunData2;    	//游泳自定义数据显示第二栏 
	uint8_t    	SwimRunData3;		//游泳自定义数据显示第三栏 
    uint8_t    	SwimRunData4;       //游泳自定义数据显示第四栏

	
	uint8_t     CyclingData1;    	//骑行自定义数据显示第一栏 
	uint8_t     CyclingData2;    	//骑行自定义数据显示第二栏 
	uint8_t    	CyclingData3;		//骑行自定义数据显示第三栏 
	uint8_t    	CyclingData4;		//骑行自定义数据显示第四栏 



    uint8_t     ClimbingData1;    	//登山自定义数据显示第一栏 
	uint8_t     ClimbingData2;    	//登山自定义数据显示第二栏 
	uint8_t    	ClimbingData3;		//登山自定义数据显示第三栏 
	uint8_t    	ClimbingData4;		//登山自定义数据显示第四栏 


	uint8_t     HikeData1;    	//徒步越野自定义数据显示第一栏 
	uint8_t     HikeData2;    	//徒步越野自定义数据显示第二栏 
	uint8_t    	HikeData3;		//徒步越野自定义数据显示第三栏 
	uint8_t    	HikeData4;		//徒步越野自定义数据显示第四栏 

   uint8_t      CrosscountryData1;    	//越野跑自定义数据显示第一栏 
	uint8_t     CrosscountryData2;    	//越野跑自定义数据显示第二栏 
	uint8_t    	CrosscountryData3;		//越野跑自定义数据显示第三栏 
	uint8_t    	CrosscountryData4;		//越野跑自定义数据显示第四栏 

	
	
	uint8_t 	IndoorLengSet;		//手表端室内跑步长是否手动设置
	uint8_t 	HeartRateSet;		//手表端心率提醒是否手动设置




	uint8_t 		AutoLightStartHour; 		//抬手亮屏开始小时
	uint8_t 		AutoLightStartMinute;		//抬手亮屏开始分钟
	uint8_t 		AutoLightEndHour;			//抬手亮屏结束小时
	uint8_t 		AutoLightEndMinute; 		//抬手亮屏结束分钟

	uint8_t    HRRecoverySet;      //心率恢复率设置开关
	#if defined WATCH_TIMEZONE_SET
	uint16_t      TimeZoneRecord;//设置的世界时钟时区
		  
	#endif
	#ifdef COD
	rtc_time_t  agps_update_time; //更新agps后需要更新此时间
	uint8_t     phone_call_delay;//设备端来电震动延迟
	uint8_t     rt_today_summary;//实时当天步数，距离，卡路里数据传输标志
	uint8_t     rt_heart;		//心率，血压，血氧数据标志。没有采集到的某项则填0。
	uint32_t    run_step;		//跑步步数
	uint32_t    walk_step;		//健走步数
	uint32_t   all_day_step_address;//全天计步地址偏移
	uint8_t     rest_heart;//安静心率
	uint32_t    atuo_factory_reset; //测试阶段通过变化结构大小，自动复位，正式的去掉

        #ifdef WATCH_SONY_GNSS 
         rtc_time_t  agps_effective_time; //更新agps后需要更新此时间
         uint8_t   agps_update_flag; //AGPS更新标志
        #endif
	
	#endif

}SetValueStr;




typedef struct __attribute__((packed))
{
	uint8_t		Date;
	uint8_t		Month;
	uint8_t		Year;
	uint8_t		Index;			//索引号，代表当前的数据存储在7天数据空间的第几个空间  0-6	
	uint32_t	CommonSteps;			//日常步数
	uint32_t	CommonEnergy;			//日常卡路里,单位为卡
	uint32_t	CommonDistance;		//日常距离，单位为cm
	uint32_t	SportSteps;			//运动步数
	uint32_t	SportEnergy;			//运动卡路里,单位为卡
	uint32_t	SportDistance;		//运动距离，单位为cm
	uint8_t   BasedHeartrate;  //基础心率
	uint16_t	Temprature;		//温度值，单位为0.1摄氏度
	uint32_t 	Pressure;		//气压，单位Pa
	uint16_t    VO2MaxMeasuring;    //测定最大摄氧量 单位：ml/kg/min
	uint16_t    LTSpeedMeasuring;    //测定乳酸阈速度 s/km
	uint8_t     LTBmpMeasuring;     //测定乳酸阈心率值 
	uint8_t  Reserved[12];//预留
} DayMainDataStr; //每天总数据结构体



typedef enum
{
	SUNNY = 1,
	HEAVY_CLOUD,
	LIGHT_CLOUD,
	SUNNY_CLOUDY,
	OVERCAST,
	WIND,
	CALM,
	BREEZE,
	GENTLE_BREEZE,
	COOL_BREEZE,
	STRONG_BREEZE,
	INTENSE_GALE,
	HEAVY_GALE,
	STRONG_GLALE,
	STORM,
	TORNADO,
	HARRICANE,
	CYCLONE,
	TROPICAL_STORM,
	SHOWER,
	HEAVY_SHOWER,
	THUNDER_SHOWER,
	HEAVY_THUNDER_SHOWER,
	THUNDER_STORM_HAIL,
	LIGHT_RAIN,
	MILD_RAIN,
	HEAVY_RAIN,
	EXTREME_RAIN,
	LIGHT_SHOWER,
	RAIN_STORM,
	HEAVY_RAIN_STORM,
	SPECIAL_HEAVY_RAIN_STORM,
	FREEZE_RAIN,
	LIGHT_SNOW,
	MILD_SNOW,
	HEAVY_SNOW,
	INTENSE_SNOW,
	RAIN_SNOW,
	RAIN_SNOW_WEATHER,
	RAIN_SNOW_SHOWER,
	SNOW_SHOWER,
	LIGHT_FROG,
	FROG,
	HAZE,
	BLOWING_SAND,
	FLOATING_DUST,
	VOCANIC_ASH	,
	SAND_STORM,
	HEAVY_SAND_STORM,
	HOT,
	COLD,
	UNDEFINE,
}CloudType;
typedef enum
{
	NO_WIND = 1,
	LIGHT_AIR,
	MILD_AIR,
	LIGHT_WIND,
	WINDY,
	STRONG_WIND,
	INTENSE_WIND,
	SPEED_WIND,
	HEAVY_WIND,
	HUG_WIND,
	TORNADOY,
	STORMY,
	HARRICANEY,
	
}WindScale;

typedef enum
{
	GOOD_AIR_QUALITY = 1,
	WELL_AIR_QUALITY,
	LIGHT_POLUTION_AIR_QUALITY,
	MILD_POLUTION_AIR_QUALITY,
	HEAVY_POLUTION_AIR_QUALITY,
	INTENSE_POLLUTION_QUALITY,
	
}AirQuality;
typedef enum
{
	NORTH_WIND = 1, //北风
	NORTHEAST_WIND, //东北风
	EAST_WIND,      //东风
	SOUTHEAST_WIND, //东南风
	SOUTH_WIND,     //南风
	SOUTHWEST_WIND, //西南风
	WEST_WIND,      //西风
	NORTHWEST_WIND, //西北风
	
	UNPREDICATE_WIND = 0xFE, //无持续风向
}WindDirection;
typedef struct __attribute__ ((packed))
{
	uint8_t date[3];   //天气日期 第一个字节年如果2018为18，第二个字节月，第三个字节日期
	uint8_t sun_rise_time[2]; //日出时间，第一个字节小时，第二个字节分钟
	uint8_t sun_set_time[2]; //日落时间，第一个字节小时，第二个字节分钟	
	int32_t pressure;//大气压
	int8_t highest_temp;//最高温度
	int8_t lowest_temp;//最低温度
	CloudType cloud; //天气情况
	WindScale wind_scale;//风级别
	WindDirection wind_direction; 
} AppWeatherSyncDaily; //APP天气每天数据结构体

typedef struct __attribute__ ((packed))
{
	uint32_t crc;//CRC下面的数据用am_bootloader_fast_crc32接口算一下，如果不相等说明没有天气数据
	uint8_t city[10];//城市名，中文用UNICODE最多5个汉字，英文用ASCII码最多有10个英文
	uint16_t pm_2_5;  //PM2.5
	AirQuality air_quality; //空气质量
	int8_t current_temp; //摄氏度温度，负的128度到正的128度，平均温度
	uint8_t day_count; //APP传来的天气天数，例如7表示7天，有7天天气数据存入外部FLASH，用于轮询
	AppWeatherSyncDaily weather_array[7]; //目前定义最大有7天天气存入外部FLASH
} AppWeatherSync; //APP天气数据结构体
/** * 天气类型 */typedef enum
	{    
	WEATHER_TYPE_SUNNY = 0,         //晴   
	WEATHER_TYPE_SUNNY_NIGHT,       //晴-夜间  
	WEATHER_TYPE_CLOUDY,            //多云    
	WEATHER_TYPE_CLOUDY_NIGHT,      //多云-夜间   
	WEATHER_TYPE_OVERCAST,          //阴    
	WEATHER_TYPE_HEAVY_RAIN,        //大雨 
	WEATHER_TYPE_MODERATE_RAIN,     //中雨 
	WEATHER_TYPE_LIGHT_RAIN,        //小雨    
	WEATHER_TYPE_TORRENTIAL_RAIN,   //暴雨   
	WEATHER_TYPE_SNOW,              //雪    
	WEATHER_TYPE_DUST,              //尘   
	WEATHER_TYPE_FOGGY,             //雾  
	WEATHER_TYPE_HAZE,              //霾   
	WEATHER_TYPE_WINDY,             //风   
	WEATHER_TYPE_UNKNOWN          //未知
} cod_weather_type;



typedef struct
{
	uint16_t PressureNum;
	uint16_t PressureSum;
	uint16_t AltitudeNum;
	uint16_t AltitudeSum;
	uint16_t PressureData[AMBIENT_DATA_MAX_NUM];	//单位: Pa
	int16_t AltitudeData[AMBIENT_DATA_MAX_NUM];		//单位: m
}AmbientDataStr;

extern uint32_t StartSteps;
extern uint8_t last_basedheartrate;//前一天基础心率值
extern AmbientDataStr AmbientData;

extern DayMainDataStr  DayMainData;
extern UploadStr Upload[2];
extern DownloadStr Download[3];


extern void com_data_init(bool set_rtc);
extern SetValueStr SetValue;
extern void Store_SetData( void );
extern void StoreDayMainData(void);

extern void Store_AmbientData(void);
extern void AmbientData_Init(void);
extern void AmbientData_Read(void);

extern void RenewOneDay( am_hal_rtc_time_t before_time,am_hal_rtc_time_t now_time,uint8_t Act_Num_Clear);
extern void GetDayMainData_BasedHDRWeekData(uint8_t *hdrValue);
extern void GetDayMainData_CommonStepWeekData(uint32_t dayOfWeekValue[]);
extern void GetDayMainData_SportsStepWeekData(uint32_t dayOfWeekValue[]);
extern void GetDayMainData_SportsCaloriesWeekData(uint32_t dayOfWeekValue[]);
extern void Get_DayCommonDataData(void);

extern uint8_t Get_SomeDayofTrainPlan(void);

extern void AddTimetoOffSide(am_hal_rtc_time_t *AddRtc);
extern void Get_TrainPlanDayRecord(void);
extern void IsOpen_TrainPlan_Settings(bool flag);
extern void Store_TranPlanUploadData(void);
extern void Get_TranPlanUploadData(uint8_t days);
extern int16_t Get_TrainPlanBeforeDays(void);
extern uint16_t Get_TrainPlanTotalDays(void);
extern bool Is_ValidTimeTrainPlan(void);
extern bool Is_TodayTimeTrainPlan(void);
extern TrainPlanDownStr TrainPlanDownloadSettingStr;
extern TrainPlanUploadBLEData Get_TrainPlanResult(uint8_t year,uint8_t month,uint8_t day);
extern void Set_TrainPlan_Notify_IsOpen(bool flag);
extern bool Get_TrainPlan_Notify_IsOpen(void);
extern uint32_t Get_TrainPlanDownloadDataSize(void);
extern void Data_Log_Print(void);
extern uint32_t Get_TotalStep(void);
extern uint32_t Get_TotalEnergy(void);
extern uint32_t Get_TotalDistance(void);

extern uint32_t Get_CommonEnergy(void);

extern void GetDayMainData_CommonDistanceWeekData(uint32_t dayOfWeekValue[]);
extern void GetDayMainData_SportsDistanceWeekData(uint32_t dayOfWeekValue[]);
extern int32_t ReadSeaLevelPressure(uint8_t year,uint8_t month,uint8_t day);

extern void Factory_Reset(void);
extern void TimetoClearStepCount(void);
extern uint32_t Get_StepStride(void);
#endif






































