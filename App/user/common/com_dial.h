#ifndef COM_DIAL_H
#define	COM_DIAL_H


#include <stdint.h>
#include "watch_config.h"

#define	DIAL_DOWNLOAD_MAX_NUM				4	//���������ر�������
#define	DIAL_DEFAULT_MAX_NUM				1	//Ĭ�ϱ�������
#define	DIAL_MAX_NUM						(DIAL_DOWNLOAD_MAX_NUM + DIAL_DEFAULT_MAX_NUM)	//��������					

#define	DIAL_DATA_START_ADDRESS				((uint32_t)0x002A0000)					//�����ⲿflash��ʼ��ַ
#define	DIAL_DATA_SIZE						(0x32000)								//һ�����̵Ĵ�С200K
#define	DIAL_CUSTOM_DEFAULT_DATA_ADDRESS	((uint32_t)0x003FE000)					//����4K���ڱ����Զ�����Ĭ�ϱ�������

#define	DIAL_FESTIVAL_NUM					7										//���ձ������
#define	DIAL_FESTIVAL_START_ADDRESS			(DIAL_DATA_START_ADDRESS + (DIAL_DATA_SIZE * (DIAL_FESTIVAL_NUM - 1)))	//���ձ��̵�ַ

#define	DIAL_BACKGROUND_IMG_ADDRESS			(DIAL_DATA_START_ADDRESS + 0x400)		//����ͼƬ��ַ
#define	DIAL_HOUR_IMG_ADDRESS				(DIAL_DATA_START_ADDRESS + 0xF000)		//Сʱ����ͼƬ��ַ
#define	DIAL_MIN_IMG_ADDRESS				(DIAL_HOUR_IMG_ADDRESS + 0x3000)		//��������ͼƬ��ַ
#define	DIAL_SEC_IMG_ADDRESS				(DIAL_MIN_IMG_ADDRESS + 0x3000)			//��������ͼƬ��ַ
#define	DIAL_ALARM_IMG_ADDRESS				(DIAL_SEC_IMG_ADDRESS + 0x2000)			//����ͼƬ��ַ
#define	DIAL_BLE_CONN_IMG_ADDRESS			(DIAL_ALARM_IMG_ADDRESS + 0x1000)		//����������ͼƬ��ַ
#define	DIAL_BLE_DISCONN_IMG_ADDRESS		(DIAL_BLE_CONN_IMG_ADDRESS + 0x1000)	//����δ����ͼƬ��ַ
#define	DIAL_DND_IMG_ADDRESS				(DIAL_BLE_DISCONN_IMG_ADDRESS + 0x1000)	//����ͼƬ��ַ
#define	DIAL_MSG_IMG_ADDRESS				(DIAL_DND_IMG_ADDRESS + 0x800)			//δ����ϢͼƬ��ַ
#define	DIAL_PLAN_IMG_ADDRESS				(DIAL_MSG_IMG_ADDRESS + 0x800)			//ѵ���ƻ�ͼƬ��ַ
#define	DIAL_STEP_IMG_ADDRESS				(DIAL_PLAN_IMG_ADDRESS + 0x1000)		//����ͼƬ��ַ
#define	DIAL_STEPGOAL_IMG_ADDRESS			(DIAL_STEP_IMG_ADDRESS + 0x800)			//Ŀ�경��ͼƬ��ַ
#define	DIAL_DISTANCE_IMG_ADDRESS			(DIAL_STEPGOAL_IMG_ADDRESS + 0x800)		//���ͼƬ��ַ
#define	DIAL_CALORIE_IMG_ADDRESS			(DIAL_DISTANCE_IMG_ADDRESS + 0x800)		//��·��ͼƬ��ַ
#define	DIAL_BATTERY_IMG_ADDRESS			(DIAL_CALORIE_IMG_ADDRESS + 0x800)		//���ͼƬ��ַ
#define	DIAL_HEART_IMG_ADDRESS				(DIAL_BATTERY_IMG_ADDRESS + 0x1000)		//����ͼƬ��ַ
#define	DIAL_SUNRISE_IMG_ADDRESS			(DIAL_HEART_IMG_ADDRESS + 0x1000)		//�ճ�ͼƬ��ַ
#define	DIAL_SUNSET_IMG_ADDRESS				(DIAL_SUNRISE_IMG_ADDRESS + 0x800)		//����ͼƬ��ַ
#define	DIAL_ALTITUDE_IMG_ADDRESS			(DIAL_SUNSET_IMG_ADDRESS + 0x800)		//����ͼƬ��ַ
#define	DIAL_PRESSURE_IMG_ADDRESS			(DIAL_ALTITUDE_IMG_ADDRESS + 0x800)		//��ѹͼƬ��ַ
#define	DIAL_TEMP_IMG_ADDRESS				(DIAL_PRESSURE_IMG_ADDRESS + 0x800)		//�¶�ͼƬ��ַ
#define	DIAL_STEPDEG_IMG_ADDRESS			(DIAL_TEMP_IMG_ADDRESS + 0x800)			//�Ʋ�������ͼƬ��ַ

#define	DIAL_TYPE_NONE						0xFF	//�޴���
#define	DIAL_DEFAULT_NUMBER					0x0		//Ĭ�ϱ������

//���뷽ʽ
#define DIAL_ACTIVEX_ALIGN_CENTER_0			0	//�ϲ����ж���
#define	DIAL_ACTIVEX_ALIGN_LEFT_0			1	//�ϲ������
#define DIAL_ACTIVEX_ALIGN_RIGHT_0			2	//�ϲ��Ҷ���
#define DIAL_ACTIVEX_ALIGN_CENTER_1			3	//��ͬ�о��ж���
#define DIAL_ACTIVEX_ALIGN_LEFT_1			4	//��ͬ�������
#define DIAL_ACTIVEX_ALIGN_RIGHT_1			5	//��ͬ���Ҷ���
#define DIAL_ACTIVEX_ALIGN_6				6
#define DIAL_ACTIVEX_ALIGN_7				7
#define DIAL_ACTIVEX_ALIGN_8				8

//������
#define DIAL_PROGRESSBAR_TYPE_0				0	//���ν���Ȧ(����)
#define DIAL_PROGRESSBAR_TYPE_1				1	//���ν���Ȧ(����)
#define	DIAL_PROGRESSBAR_TYPE_2				2	//���ζϵ����Ȧ(����)
#define DIAL_PROGRESSBAR_TYPE_3				3	//���ζϵ����Ȧ(����)
#define DIAL_PROGRESSBAR_TYPE_4				4	//���ν�����(����)
#define DIAL_PROGRESSBAR_TYPE_5				5	//���ν�����(����)
#define	DIAL_PROGRESSBAR_TYPE_6				6	//���ν���Ȧ(��������ͼ��)

//�ַ���
#define DIAL_STRING_TYPE_SONG				0	//ʹ��������������
#define	DIAL_STRING_TYPE_CUSTOM				1	//ʹ��������������



//����
#define	DIAL_BACKGROUND_TYPE_DEFAULT		0	//����ͼƬ�ı���
#define DIAL_BACKGROUND_TYPE_IMG			1	//��ͼƬ�ı���
#define	DIAL_BACKGROUND_TYPE_MANGOFUN		2	//â�����鶨�Ʊ���

#define	DIAL_BACKGROUND_TYPE_BATTERY		4	//�������ı���

//ʱ��
#define	DIAL_TIME_TYPE_DEFAULT				0	//Ĭ�ϱ�������
#define	DIAL_TIME_TYPE_NUMBER_SECOND		1	//�����ӵ����ֱ���
#define DIAL_TIME_TYPE_NUMBER_NO_SECOND		2	//01:01
#define	DIAL_TIME_TYPE_WATCHHAND_1			3	//ָ�����1
#define	DIAL_TIME_TYPE_WATCHHAND_2			4	//ָ�����2
#define	DIAL_TIME_TYPE_WATCHHAND_3			5	//ָ�����3
#define DIAL_TIME_TYPE_NUMBER_NO_SECOND_1	6	//1:01
#define DIAL_TIME_TYPE_NUMBER_NO_SECOND_2	7	//1:1
#define DIAL_TIME_TYPE_NUMBER_NO_SECOND_3	8	//00 00


#define	DIAL_TIME_TYPE_WATCHHAND_4			9	//ָ�����4
#define	DIAL_TIME_TYPE_WATCHHAND_5			10	//ָ�����5
#define	DIAL_TIME_TYPE_WATCHHAND_6			11	//ָ�����6(�����˶�ָ��)


//����
#define DIAL_DATE_TYPE_DEFAULT				0	//Ĭ�ϱ������� 1-19 ���� (��������)
#define	DIAL_DATE_TYPE_CENTER_1				1	//1-19 ���� (����)
#define DIAL_DATE_TYPE_CENTER_2				2	//1��19�� ���� (����)
#define DIAL_DATE_TYPE_CENTER_3				3	//1��19�� ���� (����)
#define DIAL_DATE_TYPE_CENTER_4				4	//1��19�� �� (����)
#define DIAL_DATE_TYPE_CENTER_5				5	//1-19 �� (��������)
#define DIAL_DATE_TYPE_CENTER_6				6	//JW913����
#define DIAL_DATE_TYPE_CENTER_7				7	//19 �� (��������)
#define DIAL_DATE_TYPE_CENTER_8				8	//19 �� (����)
#define DIAL_DATE_TYPE_CENTER_9				9	//19 FIR (����)
#define DIAL_DATE_TYPE_CENTER_10			10	//����ָ����������(����)
#define DIAL_DATE_TYPE_CENTER_11			11	//�ſ��˶���������(����)
#define DIAL_DATE_TYPE_CENTER_12			12



//������
#define	DIAL_WIDGET_TYPE_DEFAULT			0	//Ĭ��ͼ�� ���� ���� ���� ѵ���ƻ� (������ʾ)
#define	DIAL_WIDGET_TYPE_1					1	//�ⲿflashͼ�� ���� ���� ���� ѵ���ƻ� (������ʾ)
#define	DIAL_WIDGET_TYPE_2					2	//���� ����(������ʾ)
#define	DIAL_WIDGET_TYPE_3					3	//ѵ���ƻ� ����(�Ҷ�����ʾ)
#define DIAL_WIDGET_TYPE_4					4	

//״̬��
#define DIAL_STATUS_TYPE_DEFAULT			0	//Ĭ�� ���� ��ɶ�
#define	DIAL_STATUS_TYPE_1					1	//���� ���
#define	DIAL_STATUS_TYPE_2					2	//���� Ŀ�경��(JW600Ĭ������)
#define	DIAL_STATUS_TYPE_3					3	//���� ��� ��·��(JW910Ĭ������)
#define	DIAL_STATUS_TYPE_4					4	//���������� ����ͼ�� �������� Ŀ�경��ͼ�� Ŀ�경�� ���ͼ�� ������� ��·��ͼ�� ��·������
#define	DIAL_STATUS_TYPE_5					5	//�����ͼ�꣬����̵�λ (0.0km)
#define	DIAL_STATUS_TYPE_6					6	//����/Ŀ�경�� (999/1000)
#define	DIAL_STATUS_TYPE_7					7	//���� Ŀ�경��
#define	DIAL_STATUS_TYPE_8					8	//���� Ŀ�경������Ȧ
#define	DIAL_STATUS_TYPE_9					9	//���� Ŀ�경�����ν�����
#define DIAL_STATUS_TYPE_DEFAULT_1			10	//Ĭ�� ���� ��ɶ�
#define DIAL_STATUS_TYPE_DEFAULT_2			11	//Ĭ�� ����


//����
#define	DIAL_WEATHER_TYPE_NO_IMG			0	//��������ͼ��
#define DIAL_WEATHER_TYPE_IMG				1	//������ͼ��

//����
#define	DIAL_BATTERY_TYPE_DEFAULT			0	//Ĭ��ͼ�� �ٷֱ� ͼ��
#define	DIAL_BATTERY_TYPE_1					1	//����ͼ��
#define	DIAL_BATTERY_TYPE_2					2	//��������ʾ����
#define	DIAL_BATTERY_TYPE_3					3	//��������ʾ����
#define	DIAL_BATTERY_TYPE_4					4	//���빤�����м�ĵ���ͼ��
#define	DIAL_BATTERY_TYPE_5					5	//�����˶���Ӧ������Ƶĵ���ͼ��
#define	DIAL_BATTERY_TYPE_6					6	//����ָ��



//����
#define	DIAL_HEART_TYPE_DEFAULT				0	//����ͼ�� ��������
#define	DIAL_HEART_TYPE_1					1


#define	DIAL_KCAL_TYPE_1				1	//��·��

//����
#define DIAL_WEATHER_TYPE_0					0	//����ͼ�� ����

#define DIAL_WEATHER_TYPE_1					1	//����ͼ�� ���� �ճ� ����
#define DIAL_WEATHER_TYPE_2					2	//������ʱ����ʾ
#define DIAL_WEATHER_TYPE_3					3	//�빤��һ�������
#define DIAL_WEATHER_TYPE_4					4	




//����
#define DIAL_AMBIENT_TYPE_DEFAULT				0	//���� ��ѹ

#define DIAL_AMBIENT_TYPE_1					1	//���� ��ѹ �¶�
#define DIAL_AMBIENT_TYPE_2				2	//��ѹ,��·��


//ͼ��
#define DIAL_GRAPH_TYPE_WEEK_DISTANCE		1	//һ�����ֱ��ͼ	
#define	DIAL_GRAPH_TYPE_VO2MAX				2	//���������ͼ
#define DIAL_GRAPH_TYPE_12H_PRESSURE		3	//��ȥ12H��ѹ�仯ͼ
#define DIAL_GRAPH_TYPE_STEP_DEG			4	//����������ͼ��



#define DIAL_STYLE_DEFAULT 0
#define DIAL_STYLE_ELSE_1  1

#define DIAL_TEST_FLAG 0

//������
typedef struct
{
	uint8_t type;					//����		//����
	uint8_t star_x;					//�����	//���ĵ���		
	uint8_t star_y;					//�����    //���ĵ���
	uint8_t height;					//��        //�뾶
	uint8_t width;					//��        //���
	uint8_t bckgrndcolor;			//����ɫ	//����ɫ
	uint8_t forecolor;				//ǰ��ɫ	//ǰ��ɫ
	uint8_t order;					//˳��		//˳��		0-˳ʱ�� 1-��ʱ��
	uint16_t star_ang;							//���Ƕ�
	uint16_t range;								//�Ƕȷ�Χ
}ProgressBarStr;

//���ձ���ʱ��
typedef struct
{
	uint8_t year;			//��
	uint8_t month;			//��
	uint8_t day;			//��
	uint8_t reserve;		//����
}DialFestTimeStr;

//���̻�����Ϣ
typedef struct
{
	uint32_t Addr;			//�ⲿflash��ַ
	uint32_t Size;			//��С
	uint32_t CRC;			//CRC
	uint32_t version;		//�汾��
	DialFestTimeStr star_time;	//���ձ�����ʼʱ��
	DialFestTimeStr end_time;	//���ձ��̽���ʱ��
	uint8_t reserve[8];
}DialInfo;

//�����ַ�
typedef struct
{
	uint8_t type;			//����
	uint8_t size;			//��С	
	uint8_t x_axis;			//��ʼ��
	uint8_t y_axis;			//��ʼ��
	uint8_t bckgrndcolor;	//����ɫ
	uint8_t forecolor;		//ǰ��ɫ
	int8_t kerning;			//�־�
	uint8_t width;			//��
	uint8_t height;			//��
}DialStrStr;

//����ͼƬ
typedef struct
{
	uint8_t x_axis;			//��ʼ��
	uint8_t y_axis;			//��ʼ��
	uint8_t bckgrndcolor;	//����ɫ
	uint8_t forecolor;		//ǰ��ɫ
	uint8_t width;			//��
	uint8_t height;			//��
	uint8_t stride;			//���
	uint8_t imgtype;		//ͼƬ����
}DialImgStr;

//��������
typedef struct
{
	DialImgStr numImg;		//����ͼƬ
	int8_t kerning;			//���
}DialNumStr;

//����ͼ��
typedef struct
{
	DialImgStr connectImg;		//����������ͼ��
	DialImgStr disconnectImg;	//����δ����ͼ��
}DialBleStr;

//����ͼ��
typedef struct
{
	DialImgStr batImg;		//���ͼ��
	uint8_t x_axis;			//�����ʼ��
	uint8_t y_axis;			//�����ʼ��
	uint8_t width;			//�����
	uint8_t height;			//���߶�
	uint8_t normalColor;	//����������ɫ
	uint8_t lowColor;		//�͵�����ɫ
}DialBatteryImgStr;

//ѵ���ƻ�

typedef struct
{
	DialImgStr sportImg;	//�˶�����ͼ��
	DialStrStr timeStr;		//�˶�ʱ��
	int8_t gap;				//���
}DialTrainPlanStr;

typedef struct
{
	DialImgStr d_weatherImg;	//����ͼ��
	DialStrStr d_weatherStr;		//�����ַ�
	int8_t gap;				//���
}DialDweatherStr;


//����
typedef struct
{
	DialImgStr stepImg;		//�Ʋ�ͼ��
	DialStrStr stepStr;		//����
	int8_t gap;				//���
}DialSteptStr;

//���̱���
typedef struct
{
	uint8_t type;			//����
	uint8_t color;			//��ɫ
	DialImgStr img;			//ͼƬ
	uint8_t	reserve[16];	//����
}DialBgStr;

//����ʱ��
typedef struct
{
	uint8_t type;			//ʱ������
	DialNumStr hourNum;		//СʱͼƬ
	DialNumStr minNum;		//����ͼƬ
	DialNumStr secNum;		//����ͼƬ
	uint8_t gap;			//���
	uint8_t colon;			//ð��	0-�� 1-��
	uint8_t align;			//���뷽ʽ
	uint8_t	reserve[13];	//����
}DialTimeStr;

//����
typedef struct
{
	uint8_t type;		
	DialStrStr dateStr;		//������
	DialStrStr weekStr;		//����
	int8_t gap;				//���
	uint8_t align;			//���뷽ʽ
	uint8_t borderType;		//�߿�����	0-�ޱ߿� 1-���ڱ߿� 2-����+���ڱ߿�
	uint8_t borderThick;	//�߿���
	uint8_t borderColor;	//�߿���ɫ
	uint8_t	reserve[12];	//����
}DialDateStr;

//������
typedef struct
{
	uint8_t type;			//����
	DialImgStr alarmImg;	//����
	DialBleStr ble;			//����
	DialImgStr DNDImg;		//����
  #if defined COD
  DialDweatherStr dial_weather;
  #else
  DialDweatherStr dial_weather;
  DialTrainPlanStr plan;  //ѵ���ƻ�
  #endif
	int8_t gap;				//���
	uint8_t align;			//���뷽ʽ
	DialImgStr msgImg;		//��Ϣ
	uint8_t reserve[7];		//����
}DialWidgetStr;

//״̬��1
typedef struct
{
	uint8_t type;					//����
	DialSteptStr step;				//����
	uint8_t align1;					//���뷽ʽ
	uint8_t reserve1[2];			//����
	DialSteptStr distance;			//���
	ProgressBarStr stepDeg;			//����������
	uint8_t reserve2[19];			//����
	int8_t gap;						//���
	uint8_t reserve3[16];			//����
}DialStatus1Str;

//״̬��2
typedef struct
{
	uint8_t type;					//����
	uint8_t align1;					//Ŀ�경��/���� ���뷽ʽ
	uint8_t align2;					//��·��/��� ���뷽ʽ
	uint8_t reserve;				//����
	DialSteptStr step;				//����
	DialSteptStr calorie;			//��·��
	DialSteptStr distance;			//���
	DialSteptStr stepGoal;			//����Ŀ��
	ProgressBarStr stepDeg;			//����������
}DialStatus2Str;

//״̬��3
typedef struct
{
	uint8_t type;					//����
	
	uint8_t reserve[87];			//����
}DialStatus3Str;

//����
typedef struct
{
	uint8_t type;				//����
	DialStrStr batStr;			//����
	DialBatteryImgStr batImg;	//ͼ��
	int8_t gap;					//���
	uint8_t align;				//���뷽ʽ
	ProgressBarStr batDeg;		//����������
	uint8_t reserve[2];			//����
}DialBatteryStr;

//����
typedef struct
{
	uint8_t type;			//����
	DialImgStr heartImg;	//����ͼ��
	DialStrStr heartStr;	//��������
	int8_t gap;				//���
	uint8_t align;			//���뷽ʽ
	uint8_t reserve[8];		//����
}DialHeartStr;

//����
typedef struct
{
	uint8_t type;			//����
	DialSteptStr weather;	//����
	uint8_t align1;			//���뷽ʽ��������
	DialSteptStr sunrise;	//�ճ�
	DialSteptStr sunset;	//����
	uint8_t align2;			//���뷽ʽ������/�ճ���
	uint8_t reserve[7];		//����
}DialWeatherStr;

//����
typedef struct
{
	uint8_t type;			//����
	DialSteptStr altitude;	//����
	uint8_t align1;			//���뷽ʽ�����Σ�
	DialSteptStr pressure;	//��ѹ
	DialSteptStr temp;		//�¶�
	uint8_t align2;			//���뷽ʽ���¶�/��ѹ��
	uint8_t reserve[7];		//����
}DialAmbientStr;

//ͼ��1
typedef struct
{
	uint8_t type;			//ͼ������
	uint8_t star_x;			//��ʼ��
	uint8_t star_y;			//��ʼ��
	uint8_t height;			//��
	uint8_t width;			//��
	uint8_t bckgrndcolor;	//������ɫ
	uint8_t forecolor;		//ǰ��ɫ
	uint8_t reserve2[9];	//����
}DialGraph1Str;

//ͼ��2
typedef struct
{
	uint8_t type;			//ͼ������
	uint8_t reserve1[3];	//����
	DialImgStr stepImg;		//�Ʋ�Ŀ��ͼ��
	uint8_t reserve2[4];	//����
}DialGraph2Str;

//����
typedef struct
{
	uint8_t type;			//����
	uint8_t reserve1[3];	//����
	union UDialSetting
	{ 
		uint32_t val;
		struct
		{
			uint32_t TimeHour:1;		//ʱ
			uint32_t TimeMin:1;			//��
			uint32_t TimeSec:1;			//��
			uint32_t Date:1;			//������
			uint32_t Week:1;			//����
			uint32_t Step:1;			//����
			uint32_t StepDegree:1;		//������ɶ�
			uint32_t StepGoal:1;		//Ŀ�경��
			uint32_t Battery:1;			//����
			uint32_t BatteryDegree:1;	//������ɶ�
			uint32_t BckgrndImgColor:1;	//����ͼƬ��ɫ
			uint32_t BckgrndColor:1;	//������ɫ
//			uint32_t Distance:1;		//���
//			uint32_t DistanceDegree:1;	//�����ɶ�
//			uint32_t DistanceGoal:1;	//Ŀ�����
//			uint32_t Calorie:1;			//����
//			uint32_t CalorieDegree:1;	//������ɶ�
//			uint32_t CalorieGoal:1;		//Ŀ������
			
		}bits;
	}U_DialSetting;
	
	uint8_t color;			//��ɫ
	uint8_t reserve2[11];	//����
}DialSetting;

//�������ݸ�ʽ
typedef struct
{
	DialInfo		Info;			//������Ϣ
	uint16_t 		Number;			//�������
	char			Name[20];		//��������
	DialBgStr		Background;		//���̱���
	DialTimeStr 	Time;			//ʱ��
	DialDateStr		Date;			//����
	DialWidgetStr	Widget;			//������
	union Dial_Status_U				//״̬��
	{
		uint8_t Dat[88]; 			//ȷ��ռ�ݿռ�
		DialStatus1Str Type1;
		DialStatus2Str Type2;
		DialStatus3Str Type3;
	}Status;
	DialBatteryStr	Battery;		//����
	DialHeartStr	Heart;			//����
	DialWeatherStr	Weather;		//����
	DialAmbientStr	Ambient;		//����
	union Dial_Graph_U				//ͼ��
	{
		uint8_t Dat[16]; 			//ȷ��ռ�ݿռ�
		DialGraph1Str Type1;
		DialGraph2Str Type2;
	}Graph;
	DialSetting		Setting;		//����
	
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
