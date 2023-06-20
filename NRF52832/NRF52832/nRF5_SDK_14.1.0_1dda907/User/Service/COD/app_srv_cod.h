#ifndef __APP_SRV_COD_H__
#define __APP_SRV_COD_H__



#include <stdint.h>
#include "ble_srv_common.h"
#include "cod_ble_api.h"



#define		DAY_MAIN_DATA_START_ADDRESS		((uint32_t)0x00001000)
#define		DAY_MAIN_DATA_SIZE				    ((uint32_t)0x00002000)
#define	    DAY_ACTIVITY_MAX_NUM			30		//每天最大活动数

#define		ACTIVITY_DATA_START_ADDRESS		((uint32_t)0x364)  			//存储活动数据的偏移地址
#define		ACTIVITY_DAY_DATA_SIZE			(sizeof(ActivityDataStr))  //活动数据的数据空间  

#define		ACT_RUN				        0xA1    //跑步
#define		ACT_SWIMMING			    0xA4    //游泳
#define		ACT_CROSSCOUNTRY	  		0xA6    //越野跑
#define		ACT_HIKING				    0xA7    //徒步越野
#define		ACT_CLIMB				    0xA8    //登山
#define		ACT_CYCLING				    0xA9    //骑行

#define		STEP_DATA_START_ADDRESS			    ((uint32_t)0x0000F000)  //计步数据开始地址      //activity
#define		STEP_DATA_STOP_ADDRESS			    ((uint32_t)0x00012FFF)  //计步数据结束地址      //activity

#define		HEARTRATE_DATA_START_ADDRESS	    ((uint32_t)0x0002C000)  //心率数据开始地址    //activity
#define		HEARTRATE_DATA_STOP_ADDRESS	      ((uint32_t)0x00037FFF)  //心率数据结束地址    //activity

#define		PRESSURE_DATA_START_ADDRESS		    ((uint32_t)0x00013000)  //气压数据开始地址    //activity//cod用于公路牌
#define		PRESSURE_DATA_STOP_ADDRESS		    ((uint32_t)0x00014FFF)  //气压数据结束地址    //activity

#define		DISTANCE_DATA_START_ADDRESS		    ((uint32_t)0x00015000)  //距离数据开始地址    //activity
#define		DISTANCE_DATA_STOP_ADDRESS		    ((uint32_t)0x00016FFF)  //距离数据结束地址    //activity

#define		GPS_DATA_START_ADDRESS			    ((uint32_t)0x00100000)  //GPS数据开始地址         //activity
#define		GPS_DATA_STOP_ADDRESS			      ((uint32_t)0x001FFFFF)  //GPS数据结束地址         //activity

#define		COD_BLE_DEV_START_ADDRESS		    ((uint32_t)0x0002B000)  //咕咚蓝牙设置信息开始地址
#define		COD_BLE_DEV_END_ADDRESS		    	((uint32_t)0x0002BFFF)  //咕咚蓝牙设置信息结束地址
#define		COD_BLE_DEV_SIZE		    		((uint32_t)0x00001000)  //咕咚蓝牙设置信息存储大小

#define		DISTANCEPOINT_START_ADDRESS		    ((uint32_t)0x00038000)  //暂停时间开始地址,咕咚实际用于gps点距离存储
#define		DISTANCEPOINT_STOP_ADDRESS		      ((uint32_t)0x00077FFF)  //暂停时间结束地址 ，咕咚实际用于gps点距离存储
#define 	DISTANCEPOINT_COD_SIZE 			  ((uint32_t)0x00040000)




typedef struct rtc_time_struct
{ 
    uint8_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
}rtc_time_t; //时间结构体

typedef struct __attribute__ ((packed)){
	uint8_t accuracy;
	uint8_t point_type;
	int16_t altitude;
	uint32_t timestamp; //时间戳 s
	int32_t Lon;
	int32_t Lat;
}GpspointStr;

//-----------------------------for stroke type identification--------------------
typedef enum//typedef is used to ensure compatibility with C(it's not necessary in C++)
{
	Freestyle = 0 ,//0
	Backstroke,//1
	Breaststroke,//2
	ButterflyStroke,//3
	UnknownStyle,//4,
	Medley,
}StrokeID;


typedef struct __attribute__ ((packed)){
    uint8_t stroke;
    uint8_t lap_id;             //趟序号,第一趟为0，255后又回到0即可
    uint32_t time;              //手表系统时间戳，而非gps时间
} store_swim_stroke_point;


typedef struct
{
	uint32_t	Steps;              //步数
	uint32_t	Distance;           //距离
	uint32_t	Calorie;            //卡路里
	uint16_t	Speed;              //平均速度
	uint16_t	OptimumSpeed;       //最佳速度
	uint8_t 	AvgHeart;           //平均心率
	uint8_t 	MaxHeart;           //最大心率
	uint8_t 	AerobicEffect;      //有氧效果
	uint8_t 	AnaerobicEffect;    //无氧效果
	uint32_t 	HeartRateZone[5];   //心率区间时间
	uint16_t 	RecoveryTime;       //恢复时间
	uint8_t 	SubjectiveEvaluate; //主观评价
	uint8_t	    HRRecovery;         //心率恢复率
	uint16_t	RiseHeight;         //上升高度
	uint16_t	DropHeight;         //下降高度

}CommonDetailStr;


typedef struct
{
	uint32_t	Strokes;             //划数
	uint32_t	Distance;           //距离
	uint32_t	Calorie;            //卡路里
	uint16_t	Speed;              //平均配速
	uint16_t	Swolf;              //平均swolf
	uint8_t 	AvgHeart;           //平均心率
	uint8_t 	MaxHeart;           //最大心率
	uint8_t 	AerobicEffect;      //有氧效果
#ifdef COD 
	uint8_t pool_length;   //泳池长度
#else
	uint8_t 	AnaerobicEffect;    //无氧效果
#endif
	uint32_t 	HeartRateZone[5];   //心率区间时间
#ifdef COD 
	uint16_t laps;//趟数
#else
	uint16_t 	RecoveryTime;       //恢复时间
#endif
	uint8_t 	SubjectiveEvaluate; //主观评价
	uint8_t 	HRRecovery;         //心率恢复率
	uint16_t	Frequency;          //平均划频
	uint8_t 	Stroke;             //泳姿
}SwimmingDetailStr;


typedef struct{
	uint8_t	Activity_Type;     				//活动类型  A1-B2分别对应跑步/健走/马拉松/游泳/室内跑/越野跑/徒步越野/登山/骑行/铁人三项游泳/铁人三项骑行/铁人三项跑步 ...
	uint8_t	Activity_Index;	   				//活动序号  从1开始递增，活动被删除后，序号不背删除，保留的活动保持原序号，新的活动继续往后递增
	rtc_time_t  Act_Start_Time;  			//活动开始时间
	rtc_time_t	Act_Stop_Time;   			//活动结束时间
	uint16_t	CircleDistance;             //计圈距离
	uint32_t	ActTime;                    //运动时长
	uint32_t	Steps_Start_Address;		//计步数据开始地址
	uint32_t	Steps_Stop_Address;			//计步数据结束地址
	uint32_t	HeartRate_Start_Address;	//心率数据开始地址
	uint32_t	HeartRate_Stop_Address;		//心率数据结束地址
	uint32_t	Pressure_Start_Address;		//气压数据开始地址
	uint32_t	Pressure_Stop_Address;		//气压数据结束地址
	uint32_t	GPS_Start_Address;			//GPS数据开始地址
	uint32_t	GPS_Stop_Address;			//GPS数据结束地址
	uint32_t	Distance_Start_Address;		//距离数据开始地址
	uint32_t	Distance_Stop_Address;		//距离数据结束地址
	uint32_t	Energy_Start_Address;     	//热量数据起始地址
	uint32_t	Energy_Stop_Address;     	//热量数据结束地址
	uint32_t	Speed_Start_Address;     	//速度数据起始地址
	uint32_t	Speed_Stop_Address;     	//速度数据结束地址
	uint32_t	DistancePoint_Start_Address;//距离标点开始地址
	uint32_t	DistancePoint_Stop_Address;	//距离标点结束地址
	uint32_t	CircleTime_Start_Address;   //计圈时间起始地址
	uint32_t	CircleTime_Stop_Address;    //计圈时间结束地址
	uint32_t    cod_id;				 	//记录ID，唯一的标明一条记录，自增。到最大值后+1回到0。
	uint32_t 	half_marathon_time;    //半马时间，不包括暂停，s
    uint32_t 	full_marathon_time;    //全马时间，不包括暂停，s
	uint32_t 	cod_version;              //运动记录时固件版本号(注意不是上传时版本号),大版本
	uint32_t 	agps_update_time;		 //离线星历更新时间戳s，如果没有离线星历，填2018年1月1日0时0分0秒
	uint32_t 	reserve;
	union   Activity_Details_U
	{
		uint8_t						Dat[48]; //确定占据空间
		CommonDetailStr			    CommonDetail;     //公共数据结构
		SwimmingDetailStr		    SwimmingDetail;    //游泳，铁人三项游泳
	}ActivityDetails;
} ActivityDataStr;//最大181字节



typedef struct __attribute__ ((packed)){
	uint32_t command;
	uint32_t crc;
	uint8_t year;
	uint8_t month;
	uint8_t date;
	uint8_t day_of_week;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	int8_t time_zone;
}_ble_current_time;

typedef struct __attribute__ ((packed)){
	uint32_t command;
	uint8_t gender;  //性别
	uint8_t age;
	uint8_t height;         //身高:单位CM
	uint8_t weight;  
}_ble_user_info;




typedef struct __attribute__((packed)){
	uint32_t command;
	uint8_t  data[200];
	uint16_t len;
}_ble_data_info;

typedef struct __attribute__((packed))
{
	uint32_t data_len;
	uint8_t  *data;
	uint32_t crc;
}cod_ota_data_info_t;

typedef struct __attribute__((packed)){
	uint32_t command;
	uint8_t  extra_info_len;
	uint8_t *extra_info_data;
	cod_ota_data_info_t apollo;
	uint16_t ble_id;
	cod_ota_data_info_t ble;
}cod_ota_file_info_t;

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
} cod_user_training_cast;

//全天计步存储信息
typedef struct __attribute__((packed)){
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint32_t addr;
	uint32_t size;
} _all_day_steps;


typedef struct __attribute__((packed))
{
	uint32_t crc;
    char name[20]; 
    char user_id[20]; 
    uint8_t name_len;
    uint8_t user_id_len;
    uint16_t product_type;
    uint8_t bind_status;
    uint8_t platform;
}cod_ble_dev_param;

typedef struct __attribute__((packed))
{
	uint32_t cmd;
	uint8_t mac[6];	
}_ble_mac;



extern cod_sport_data_converter sport_converter;
extern cod_swim_data_converter swim_converter;
extern cod_sport_data_convert_callback sport_callback;
extern uint8_t sport_buffer[50];
#if 0
extern uint8_t send_buffer[MTU_155];
extern uint8_t remain_buffer[MTU_155];
extern uint8_t send_buffer_len,remain_buffer_len,
#endif
extern uint8_t get_data_flag;
extern uint32_t total_size, last_total_size,sport_summary_size,swim_summary_size;
extern uint16_t cur_frame_id,total_frame_nums,cur_sport_id; 
extern uint32_t cur_total_size[210];




void cod_sdk_init(void);
uint32_t app_srv_cod_init(void);

//串口命令
void cmd_cod_on_check_agps(void);

//数据发送
void app_srv_bas_level_send(uint8_t *p_data, uint16_t length);



#endif //__APP_SRV_NRF52_H__
