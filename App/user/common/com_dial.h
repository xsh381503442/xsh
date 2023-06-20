#ifndef COM_DIAL_H
#define	COM_DIAL_H


#include <stdint.h>
#include "watch_config.h"

#define	DIAL_DOWNLOAD_MAX_NUM				4	//最大可以下载表盘数量
#define	DIAL_DEFAULT_MAX_NUM				1	//默认表盘数量
#define	DIAL_MAX_NUM						(DIAL_DOWNLOAD_MAX_NUM + DIAL_DEFAULT_MAX_NUM)	//表盘总数					

#define	DIAL_DATA_START_ADDRESS				((uint32_t)0x002A0000)					//表盘外部flash起始地址
#define	DIAL_DATA_SIZE						(0x32000)								//一个表盘的大小200K
#define	DIAL_CUSTOM_DEFAULT_DATA_ADDRESS	((uint32_t)0x003FE000)					//其中4K用于保存自定义后的默认表盘数据

#define	DIAL_FESTIVAL_NUM					7										//节日表盘序号
#define	DIAL_FESTIVAL_START_ADDRESS			(DIAL_DATA_START_ADDRESS + (DIAL_DATA_SIZE * (DIAL_FESTIVAL_NUM - 1)))	//节日表盘地址

#define	DIAL_BACKGROUND_IMG_ADDRESS			(DIAL_DATA_START_ADDRESS + 0x400)		//背景图片地址
#define	DIAL_HOUR_IMG_ADDRESS				(DIAL_DATA_START_ADDRESS + 0xF000)		//小时数字图片地址
#define	DIAL_MIN_IMG_ADDRESS				(DIAL_HOUR_IMG_ADDRESS + 0x3000)		//分钟数字图片地址
#define	DIAL_SEC_IMG_ADDRESS				(DIAL_MIN_IMG_ADDRESS + 0x3000)			//秒钟数字图片地址
#define	DIAL_ALARM_IMG_ADDRESS				(DIAL_SEC_IMG_ADDRESS + 0x2000)			//闹钟图片地址
#define	DIAL_BLE_CONN_IMG_ADDRESS			(DIAL_ALARM_IMG_ADDRESS + 0x1000)		//蓝牙已连接图片地址
#define	DIAL_BLE_DISCONN_IMG_ADDRESS		(DIAL_BLE_CONN_IMG_ADDRESS + 0x1000)	//蓝牙未连接图片地址
#define	DIAL_DND_IMG_ADDRESS				(DIAL_BLE_DISCONN_IMG_ADDRESS + 0x1000)	//勿扰图片地址
#define	DIAL_MSG_IMG_ADDRESS				(DIAL_DND_IMG_ADDRESS + 0x800)			//未读信息图片地址
#define	DIAL_PLAN_IMG_ADDRESS				(DIAL_MSG_IMG_ADDRESS + 0x800)			//训练计划图片地址
#define	DIAL_STEP_IMG_ADDRESS				(DIAL_PLAN_IMG_ADDRESS + 0x1000)		//步数图片地址
#define	DIAL_STEPGOAL_IMG_ADDRESS			(DIAL_STEP_IMG_ADDRESS + 0x800)			//目标步数图片地址
#define	DIAL_DISTANCE_IMG_ADDRESS			(DIAL_STEPGOAL_IMG_ADDRESS + 0x800)		//里程图片地址
#define	DIAL_CALORIE_IMG_ADDRESS			(DIAL_DISTANCE_IMG_ADDRESS + 0x800)		//卡路里图片地址
#define	DIAL_BATTERY_IMG_ADDRESS			(DIAL_CALORIE_IMG_ADDRESS + 0x800)		//电池图片地址
#define	DIAL_HEART_IMG_ADDRESS				(DIAL_BATTERY_IMG_ADDRESS + 0x1000)		//心率图片地址
#define	DIAL_SUNRISE_IMG_ADDRESS			(DIAL_HEART_IMG_ADDRESS + 0x1000)		//日出图片地址
#define	DIAL_SUNSET_IMG_ADDRESS				(DIAL_SUNRISE_IMG_ADDRESS + 0x800)		//日落图片地址
#define	DIAL_ALTITUDE_IMG_ADDRESS			(DIAL_SUNSET_IMG_ADDRESS + 0x800)		//海拔图片地址
#define	DIAL_PRESSURE_IMG_ADDRESS			(DIAL_ALTITUDE_IMG_ADDRESS + 0x800)		//气压图片地址
#define	DIAL_TEMP_IMG_ADDRESS				(DIAL_PRESSURE_IMG_ADDRESS + 0x800)		//温度图片地址
#define	DIAL_STEPDEG_IMG_ADDRESS			(DIAL_TEMP_IMG_ADDRESS + 0x800)			//计步进度条图片地址

#define	DIAL_TYPE_NONE						0xFF	//无此项
#define	DIAL_DEFAULT_NUMBER					0x0		//默认表盘序号

//对齐方式
#define DIAL_ACTIVEX_ALIGN_CENTER_0			0	//合并居中对齐
#define	DIAL_ACTIVEX_ALIGN_LEFT_0			1	//合并左对齐
#define DIAL_ACTIVEX_ALIGN_RIGHT_0			2	//合并右对齐
#define DIAL_ACTIVEX_ALIGN_CENTER_1			3	//不同行居中对齐
#define DIAL_ACTIVEX_ALIGN_LEFT_1			4	//不同行左对齐
#define DIAL_ACTIVEX_ALIGN_RIGHT_1			5	//不同行右对齐
#define DIAL_ACTIVEX_ALIGN_6				6
#define DIAL_ACTIVEX_ALIGN_7				7
#define DIAL_ACTIVEX_ALIGN_8				8

//进度条
#define DIAL_PROGRESSBAR_TYPE_0				0	//环形进度圈(电量)
#define DIAL_PROGRESSBAR_TYPE_1				1	//环形进度圈(步数)
#define	DIAL_PROGRESSBAR_TYPE_2				2	//环形断点进度圈(电量)
#define DIAL_PROGRESSBAR_TYPE_3				3	//环形断点进度圈(步数)
#define DIAL_PROGRESSBAR_TYPE_4				4	//方形进度条(电量)
#define DIAL_PROGRESSBAR_TYPE_5				5	//方形进度条(步数)
#define	DIAL_PROGRESSBAR_TYPE_6				6	//环形进度圈(步数，带图标)

//字符串
#define DIAL_STRING_TYPE_SONG				0	//使用宋体数字字体
#define	DIAL_STRING_TYPE_CUSTOM				1	//使用自制数字字体



//背景
#define	DIAL_BACKGROUND_TYPE_DEFAULT		0	//不带图片的背景
#define DIAL_BACKGROUND_TYPE_IMG			1	//带图片的背景
#define	DIAL_BACKGROUND_TYPE_MANGOFUN		2	//芒果互娱定制背景

#define	DIAL_BACKGROUND_TYPE_BATTERY		4	//带电量的背景

//时间
#define	DIAL_TIME_TYPE_DEFAULT				0	//默认表盘数字
#define	DIAL_TIME_TYPE_NUMBER_SECOND		1	//带秒钟的数字表盘
#define DIAL_TIME_TYPE_NUMBER_NO_SECOND		2	//01:01
#define	DIAL_TIME_TYPE_WATCHHAND_1			3	//指针表盘1
#define	DIAL_TIME_TYPE_WATCHHAND_2			4	//指针表盘2
#define	DIAL_TIME_TYPE_WATCHHAND_3			5	//指针表盘3
#define DIAL_TIME_TYPE_NUMBER_NO_SECOND_1	6	//1:01
#define DIAL_TIME_TYPE_NUMBER_NO_SECOND_2	7	//1:1
#define DIAL_TIME_TYPE_NUMBER_NO_SECOND_3	8	//00 00


#define	DIAL_TIME_TYPE_WATCHHAND_4			9	//指针表盘4
#define	DIAL_TIME_TYPE_WATCHHAND_5			10	//指针表盘5
#define	DIAL_TIME_TYPE_WATCHHAND_6			11	//指针表盘6(商务运动指针)


//日期
#define DIAL_DATE_TYPE_DEFAULT				0	//默认表盘日期 1-19 周五 (自制字体)
#define	DIAL_DATE_TYPE_CENTER_1				1	//1-19 周五 (宋体)
#define DIAL_DATE_TYPE_CENTER_2				2	//1月19日 周五 (宋体)
#define DIAL_DATE_TYPE_CENTER_3				3	//1月19日 周五 (宋体)
#define DIAL_DATE_TYPE_CENTER_4				4	//1月19日 五 (宋体)
#define DIAL_DATE_TYPE_CENTER_5				5	//1-19 五 (自制字体)
#define DIAL_DATE_TYPE_CENTER_6				6	//JW913定制
#define DIAL_DATE_TYPE_CENTER_7				7	//19 五 (自制字体)
#define DIAL_DATE_TYPE_CENTER_8				8	//19 五 (宋体)
#define DIAL_DATE_TYPE_CENTER_9				9	//19 FIR (宋体)
#define DIAL_DATE_TYPE_CENTER_10			10	//商务指针主题日期(咕咚)
#define DIAL_DATE_TYPE_CENTER_11			11	//炫酷运动主题日期(咕咚)
#define DIAL_DATE_TYPE_CENTER_12			12



//工具栏
#define	DIAL_WIDGET_TYPE_DEFAULT			0	//默认图标 勿扰 闹钟 蓝牙 训练计划 (居中显示)
#define	DIAL_WIDGET_TYPE_1					1	//外部flash图标 勿扰 闹钟 蓝牙 训练计划 (居中显示)
#define	DIAL_WIDGET_TYPE_2					2	//蓝牙 电量(居中显示)
#define	DIAL_WIDGET_TYPE_3					3	//训练计划 蓝牙(右对齐显示)
#define DIAL_WIDGET_TYPE_4					4	

//状态栏
#define DIAL_STATUS_TYPE_DEFAULT			0	//默认 步数 完成度
#define	DIAL_STATUS_TYPE_1					1	//步数 里程
#define	DIAL_STATUS_TYPE_2					2	//步数 目标步数(JW600默认主题)
#define	DIAL_STATUS_TYPE_3					3	//步数 里程 卡路里(JW910默认主题)
#define	DIAL_STATUS_TYPE_4					4	//步数进度条 步数图标 步数数据 目标步数图标 目标步数 里程图标 里程数据 卡路里图标 卡路里数据
#define	DIAL_STATUS_TYPE_5					5	//无里程图标，有里程单位 (0.0km)
#define	DIAL_STATUS_TYPE_6					6	//步数/目标步数 (999/1000)
#define	DIAL_STATUS_TYPE_7					7	//步数 目标步数
#define	DIAL_STATUS_TYPE_8					8	//步数 目标步数环形圈
#define	DIAL_STATUS_TYPE_9					9	//步数 目标步数方形进度条
#define DIAL_STATUS_TYPE_DEFAULT_1			10	//默认 步数 完成度
#define DIAL_STATUS_TYPE_DEFAULT_2			11	//默认 步数


//天气
#define	DIAL_WEATHER_TYPE_NO_IMG			0	//不带天气图标
#define DIAL_WEATHER_TYPE_IMG				1	//带天气图标

//电量
#define	DIAL_BATTERY_TYPE_DEFAULT			0	//默认图标 百分比 图标
#define	DIAL_BATTERY_TYPE_1					1	//电量图标
#define	DIAL_BATTERY_TYPE_2					2	//进度条显示电量
#define	DIAL_BATTERY_TYPE_3					3	//进度条显示电量
#define	DIAL_BATTERY_TYPE_4					4	//插入工具栏中间的电量图标
#define	DIAL_BATTERY_TYPE_5					5	//数字运动对应的需绘制的电量图标
#define	DIAL_BATTERY_TYPE_6					6	//商务指针



//心率
#define	DIAL_HEART_TYPE_DEFAULT				0	//心率图标 心率数据
#define	DIAL_HEART_TYPE_1					1


#define	DIAL_KCAL_TYPE_1				1	//卡路里

//天气
#define DIAL_WEATHER_TYPE_0					0	//天气图标 气温

#define DIAL_WEATHER_TYPE_1					1	//天气图标 气温 日出 日落
#define DIAL_WEATHER_TYPE_2					2	//无天气时不显示
#define DIAL_WEATHER_TYPE_3					3	//与工具一起的天气
#define DIAL_WEATHER_TYPE_4					4	




//环境
#define DIAL_AMBIENT_TYPE_DEFAULT				0	//海拔 气压

#define DIAL_AMBIENT_TYPE_1					1	//海拔 气压 温度
#define DIAL_AMBIENT_TYPE_2				2	//气压,卡路里


//图表
#define DIAL_GRAPH_TYPE_WEEK_DISTANCE		1	//一周里程直方图	
#define	DIAL_GRAPH_TYPE_VO2MAX				2	//最大摄氧量图
#define DIAL_GRAPH_TYPE_12H_PRESSURE		3	//过去12H气压变化图
#define DIAL_GRAPH_TYPE_STEP_DEG			4	//步数进度条图标



#define DIAL_STYLE_DEFAULT 0
#define DIAL_STYLE_ELSE_1  1

#define DIAL_TEST_FLAG 0

//进度条
typedef struct
{
	uint8_t type;					//类型		//类型
	uint8_t star_x;					//起点行	//中心点行		
	uint8_t star_y;					//起点列    //中心点列
	uint8_t height;					//高        //半径
	uint8_t width;					//宽        //厚度
	uint8_t bckgrndcolor;			//背景色	//背景色
	uint8_t forecolor;				//前景色	//前景色
	uint8_t order;					//顺序		//顺序		0-顺时针 1-逆时针
	uint16_t star_ang;							//起点角度
	uint16_t range;								//角度范围
}ProgressBarStr;

//节日表盘时间
typedef struct
{
	uint8_t year;			//年
	uint8_t month;			//月
	uint8_t day;			//日
	uint8_t reserve;		//保留
}DialFestTimeStr;

//表盘基本信息
typedef struct
{
	uint32_t Addr;			//外部flash地址
	uint32_t Size;			//大小
	uint32_t CRC;			//CRC
	uint32_t version;		//版本号
	DialFestTimeStr star_time;	//节日表盘起始时间
	DialFestTimeStr end_time;	//节日表盘结束时间
	uint8_t reserve[8];
}DialInfo;

//表盘字符
typedef struct
{
	uint8_t type;			//类型
	uint8_t size;			//大小	
	uint8_t x_axis;			//起始行
	uint8_t y_axis;			//起始列
	uint8_t bckgrndcolor;	//背景色
	uint8_t forecolor;		//前景色
	int8_t kerning;			//字距
	uint8_t width;			//宽
	uint8_t height;			//高
}DialStrStr;

//表盘图片
typedef struct
{
	uint8_t x_axis;			//起始行
	uint8_t y_axis;			//起始列
	uint8_t bckgrndcolor;	//背景色
	uint8_t forecolor;		//前景色
	uint8_t width;			//宽
	uint8_t height;			//高
	uint8_t stride;			//跨度
	uint8_t imgtype;		//图片类型
}DialImgStr;

//表盘数字
typedef struct
{
	DialImgStr numImg;		//数字图片
	int8_t kerning;			//间距
}DialNumStr;

//蓝牙图标
typedef struct
{
	DialImgStr connectImg;		//蓝牙已连接图标
	DialImgStr disconnectImg;	//蓝牙未连接图标
}DialBleStr;

//电量图标
typedef struct
{
	DialImgStr batImg;		//电池图标
	uint8_t x_axis;			//填充起始行
	uint8_t y_axis;			//填充起始列
	uint8_t width;			//填充宽度
	uint8_t height;			//填充高度
	uint8_t normalColor;	//正常电量颜色
	uint8_t lowColor;		//低电量颜色
}DialBatteryImgStr;

//训练计划

typedef struct
{
	DialImgStr sportImg;	//运动类型图标
	DialStrStr timeStr;		//运动时间
	int8_t gap;				//间距
}DialTrainPlanStr;

typedef struct
{
	DialImgStr d_weatherImg;	//天气图标
	DialStrStr d_weatherStr;		//天气字符
	int8_t gap;				//间距
}DialDweatherStr;


//步数
typedef struct
{
	DialImgStr stepImg;		//计步图标
	DialStrStr stepStr;		//步数
	int8_t gap;				//间距
}DialSteptStr;

//表盘背景
typedef struct
{
	uint8_t type;			//类型
	uint8_t color;			//颜色
	DialImgStr img;			//图片
	uint8_t	reserve[16];	//保留
}DialBgStr;

//表盘时间
typedef struct
{
	uint8_t type;			//时间类型
	DialNumStr hourNum;		//小时图片
	DialNumStr minNum;		//分钟图片
	DialNumStr secNum;		//秒钟图片
	uint8_t gap;			//间距
	uint8_t colon;			//冒号	0-无 1-有
	uint8_t align;			//对齐方式
	uint8_t	reserve[13];	//保留
}DialTimeStr;

//日期
typedef struct
{
	uint8_t type;		
	DialStrStr dateStr;		//年月日
	DialStrStr weekStr;		//星期
	int8_t gap;				//间距
	uint8_t align;			//对齐方式
	uint8_t borderType;		//边框类型	0-无边框 1-日期边框 2-日期+星期边框
	uint8_t borderThick;	//边框厚度
	uint8_t borderColor;	//边框颜色
	uint8_t	reserve[12];	//保留
}DialDateStr;

//工具栏
typedef struct
{
	uint8_t type;			//类型
	DialImgStr alarmImg;	//闹钟
	DialBleStr ble;			//蓝牙
	DialImgStr DNDImg;		//勿扰
  #if defined COD
  DialDweatherStr dial_weather;
  #else
  DialDweatherStr dial_weather;
  DialTrainPlanStr plan;  //训练计划
  #endif
	int8_t gap;				//间距
	uint8_t align;			//对齐方式
	DialImgStr msgImg;		//信息
	uint8_t reserve[7];		//保留
}DialWidgetStr;

//状态栏1
typedef struct
{
	uint8_t type;					//类型
	DialSteptStr step;				//步数
	uint8_t align1;					//对齐方式
	uint8_t reserve1[2];			//保留
	DialSteptStr distance;			//里程
	ProgressBarStr stepDeg;			//步数进度条
	uint8_t reserve2[19];			//保留
	int8_t gap;						//间距
	uint8_t reserve3[16];			//保留
}DialStatus1Str;

//状态栏2
typedef struct
{
	uint8_t type;					//类型
	uint8_t align1;					//目标步数/步数 对齐方式
	uint8_t align2;					//卡路里/里程 对齐方式
	uint8_t reserve;				//保留
	DialSteptStr step;				//步数
	DialSteptStr calorie;			//卡路里
	DialSteptStr distance;			//里程
	DialSteptStr stepGoal;			//步数目标
	ProgressBarStr stepDeg;			//步数进度条
}DialStatus2Str;

//状态栏3
typedef struct
{
	uint8_t type;					//类型
	
	uint8_t reserve[87];			//保留
}DialStatus3Str;

//电量
typedef struct
{
	uint8_t type;				//类型
	DialStrStr batStr;			//数据
	DialBatteryImgStr batImg;	//图标
	int8_t gap;					//间距
	uint8_t align;				//对齐方式
	ProgressBarStr batDeg;		//电量进度条
	uint8_t reserve[2];			//保留
}DialBatteryStr;

//心率
typedef struct
{
	uint8_t type;			//类型
	DialImgStr heartImg;	//心率图标
	DialStrStr heartStr;	//心率数据
	int8_t gap;				//间距
	uint8_t align;			//对齐方式
	uint8_t reserve[8];		//保留
}DialHeartStr;

//天气
typedef struct
{
	uint8_t type;			//类型
	DialSteptStr weather;	//天气
	uint8_t align1;			//对齐方式（天气）
	DialSteptStr sunrise;	//日出
	DialSteptStr sunset;	//日落
	uint8_t align2;			//对齐方式（日落/日出）
	uint8_t reserve[7];		//保留
}DialWeatherStr;

//环境
typedef struct
{
	uint8_t type;			//类型
	DialSteptStr altitude;	//海拔
	uint8_t align1;			//对齐方式（海拔）
	DialSteptStr pressure;	//气压
	DialSteptStr temp;		//温度
	uint8_t align2;			//对齐方式（温度/气压）
	uint8_t reserve[7];		//保留
}DialAmbientStr;

//图表1
typedef struct
{
	uint8_t type;			//图表类型
	uint8_t star_x;			//起始行
	uint8_t star_y;			//起始列
	uint8_t height;			//高
	uint8_t width;			//宽
	uint8_t bckgrndcolor;	//背景颜色
	uint8_t forecolor;		//前景色
	uint8_t reserve2[9];	//保留
}DialGraph1Str;

//图表2
typedef struct
{
	uint8_t type;			//图表类型
	uint8_t reserve1[3];	//保留
	DialImgStr stepImg;		//计步目标图标
	uint8_t reserve2[4];	//保留
}DialGraph2Str;

//设置
typedef struct
{
	uint8_t type;			//类型
	uint8_t reserve1[3];	//保留
	union UDialSetting
	{ 
		uint32_t val;
		struct
		{
			uint32_t TimeHour:1;		//时
			uint32_t TimeMin:1;			//分
			uint32_t TimeSec:1;			//秒
			uint32_t Date:1;			//年月日
			uint32_t Week:1;			//星期
			uint32_t Step:1;			//步数
			uint32_t StepDegree:1;		//步数完成度
			uint32_t StepGoal:1;		//目标步数
			uint32_t Battery:1;			//电量
			uint32_t BatteryDegree:1;	//电量完成度
			uint32_t BckgrndImgColor:1;	//背景图片颜色
			uint32_t BckgrndColor:1;	//背景颜色
//			uint32_t Distance:1;		//里程
//			uint32_t DistanceDegree:1;	//里程完成度
//			uint32_t DistanceGoal:1;	//目标里程
//			uint32_t Calorie:1;			//热量
//			uint32_t CalorieDegree:1;	//热量完成度
//			uint32_t CalorieGoal:1;		//目标热量
			
		}bits;
	}U_DialSetting;
	
	uint8_t color;			//颜色
	uint8_t reserve2[11];	//保留
}DialSetting;

//表盘数据格式
typedef struct
{
	DialInfo		Info;			//表盘信息
	uint16_t 		Number;			//表盘序号
	char			Name[20];		//表盘名称
	DialBgStr		Background;		//表盘背景
	DialTimeStr 	Time;			//时间
	DialDateStr		Date;			//日期
	DialWidgetStr	Widget;			//工具栏
	union Dial_Status_U				//状态栏
	{
		uint8_t Dat[88]; 			//确定占据空间
		DialStatus1Str Type1;
		DialStatus2Str Type2;
		DialStatus3Str Type3;
	}Status;
	DialBatteryStr	Battery;		//电量
	DialHeartStr	Heart;			//心率
	DialWeatherStr	Weather;		//天气
	DialAmbientStr	Ambient;		//环境
	union Dial_Graph_U				//图表
	{
		uint8_t Dat[16]; 			//确定占据空间
		DialGraph1Str Type1;
		DialGraph2Str Type2;
	}Graph;
	DialSetting		Setting;		//设置
	
}DialDataStr;

#if DIAL_TEST_FLAG
//void com_dial_test1(DialDataStr *dial);
//void com_dial_test2(DialDataStr *dial);
//void com_dial_test3(DialDataStr *dial);
  void com_dial_cod_test1(DialDataStr *dial);


#endif
extern DialDataStr DialData;

extern void com_dial_init(void);
extern void com_dial_read(uint8_t num, DialDataStr *dial);
extern void com_dial_delete(void);
extern void com_dial_get(uint8_t number, DialDataStr *dial);
extern uint8_t com_dial_is_valid(uint8_t number);
extern void com_dial_festival_init(void);
extern uint8_t com_dial_festival_is_valid(void);
extern void com_dial_set_invalid(uint8_t num);
extern uint8_t com_dial_custom_is_valid(void);
extern void com_dial_custom_write(uint8_t num, DialDataStr *dial);
extern void com_dial_custom_delete(uint8_t num);

#endif
