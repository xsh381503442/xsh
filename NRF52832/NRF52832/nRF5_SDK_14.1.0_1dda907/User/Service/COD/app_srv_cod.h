#ifndef __APP_SRV_COD_H__
#define __APP_SRV_COD_H__



#include <stdint.h>
#include "ble_srv_common.h"
#include "cod_ble_api.h"



#define		DAY_MAIN_DATA_START_ADDRESS		((uint32_t)0x00001000)
#define		DAY_MAIN_DATA_SIZE				    ((uint32_t)0x00002000)
#define	    DAY_ACTIVITY_MAX_NUM			30		//ÿ�������

#define		ACTIVITY_DATA_START_ADDRESS		((uint32_t)0x364)  			//�洢����ݵ�ƫ�Ƶ�ַ
#define		ACTIVITY_DAY_DATA_SIZE			(sizeof(ActivityDataStr))  //����ݵ����ݿռ�  

#define		ACT_RUN				        0xA1    //�ܲ�
#define		ACT_SWIMMING			    0xA4    //��Ӿ
#define		ACT_CROSSCOUNTRY	  		0xA6    //ԽҰ��
#define		ACT_HIKING				    0xA7    //ͽ��ԽҰ
#define		ACT_CLIMB				    0xA8    //��ɽ
#define		ACT_CYCLING				    0xA9    //����

#define		STEP_DATA_START_ADDRESS			    ((uint32_t)0x0000F000)  //�Ʋ����ݿ�ʼ��ַ      //activity
#define		STEP_DATA_STOP_ADDRESS			    ((uint32_t)0x00012FFF)  //�Ʋ����ݽ�����ַ      //activity

#define		HEARTRATE_DATA_START_ADDRESS	    ((uint32_t)0x0002C000)  //�������ݿ�ʼ��ַ    //activity
#define		HEARTRATE_DATA_STOP_ADDRESS	      ((uint32_t)0x00037FFF)  //�������ݽ�����ַ    //activity

#define		PRESSURE_DATA_START_ADDRESS		    ((uint32_t)0x00013000)  //��ѹ���ݿ�ʼ��ַ    //activity//cod���ڹ�·��
#define		PRESSURE_DATA_STOP_ADDRESS		    ((uint32_t)0x00014FFF)  //��ѹ���ݽ�����ַ    //activity

#define		DISTANCE_DATA_START_ADDRESS		    ((uint32_t)0x00015000)  //�������ݿ�ʼ��ַ    //activity
#define		DISTANCE_DATA_STOP_ADDRESS		    ((uint32_t)0x00016FFF)  //�������ݽ�����ַ    //activity

#define		GPS_DATA_START_ADDRESS			    ((uint32_t)0x00100000)  //GPS���ݿ�ʼ��ַ         //activity
#define		GPS_DATA_STOP_ADDRESS			      ((uint32_t)0x001FFFFF)  //GPS���ݽ�����ַ         //activity

#define		COD_BLE_DEV_START_ADDRESS		    ((uint32_t)0x0002B000)  //��������������Ϣ��ʼ��ַ
#define		COD_BLE_DEV_END_ADDRESS		    	((uint32_t)0x0002BFFF)  //��������������Ϣ������ַ
#define		COD_BLE_DEV_SIZE		    		((uint32_t)0x00001000)  //��������������Ϣ�洢��С

#define		DISTANCEPOINT_START_ADDRESS		    ((uint32_t)0x00038000)  //��ͣʱ�俪ʼ��ַ,����ʵ������gps�����洢
#define		DISTANCEPOINT_STOP_ADDRESS		      ((uint32_t)0x00077FFF)  //��ͣʱ�������ַ ������ʵ������gps�����洢
#define 	DISTANCEPOINT_COD_SIZE 			  ((uint32_t)0x00040000)




typedef struct rtc_time_struct
{ 
    uint8_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
}rtc_time_t; //ʱ��ṹ��

typedef struct __attribute__ ((packed)){
	uint8_t accuracy;
	uint8_t point_type;
	int16_t altitude;
	uint32_t timestamp; //ʱ��� s
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
    uint8_t lap_id;             //�����,��һ��Ϊ0��255���ֻص�0����
    uint32_t time;              //�ֱ�ϵͳʱ���������gpsʱ��
} store_swim_stroke_point;


typedef struct
{
	uint32_t	Steps;              //����
	uint32_t	Distance;           //����
	uint32_t	Calorie;            //��·��
	uint16_t	Speed;              //ƽ���ٶ�
	uint16_t	OptimumSpeed;       //����ٶ�
	uint8_t 	AvgHeart;           //ƽ������
	uint8_t 	MaxHeart;           //�������
	uint8_t 	AerobicEffect;      //����Ч��
	uint8_t 	AnaerobicEffect;    //����Ч��
	uint32_t 	HeartRateZone[5];   //��������ʱ��
	uint16_t 	RecoveryTime;       //�ָ�ʱ��
	uint8_t 	SubjectiveEvaluate; //��������
	uint8_t	    HRRecovery;         //���ʻָ���
	uint16_t	RiseHeight;         //�����߶�
	uint16_t	DropHeight;         //�½��߶�

}CommonDetailStr;


typedef struct
{
	uint32_t	Strokes;             //����
	uint32_t	Distance;           //����
	uint32_t	Calorie;            //��·��
	uint16_t	Speed;              //ƽ������
	uint16_t	Swolf;              //ƽ��swolf
	uint8_t 	AvgHeart;           //ƽ������
	uint8_t 	MaxHeart;           //�������
	uint8_t 	AerobicEffect;      //����Ч��
#ifdef COD 
	uint8_t pool_length;   //Ӿ�س���
#else
	uint8_t 	AnaerobicEffect;    //����Ч��
#endif
	uint32_t 	HeartRateZone[5];   //��������ʱ��
#ifdef COD 
	uint16_t laps;//����
#else
	uint16_t 	RecoveryTime;       //�ָ�ʱ��
#endif
	uint8_t 	SubjectiveEvaluate; //��������
	uint8_t 	HRRecovery;         //���ʻָ���
	uint16_t	Frequency;          //ƽ����Ƶ
	uint8_t 	Stroke;             //Ӿ��
}SwimmingDetailStr;


typedef struct{
	uint8_t	Activity_Type;     				//�����  A1-B2�ֱ��Ӧ�ܲ�/����/������/��Ӿ/������/ԽҰ��/ͽ��ԽҰ/��ɽ/����/����������Ӿ/������������/���������ܲ� ...
	uint8_t	Activity_Index;	   				//����  ��1��ʼ���������ɾ������Ų���ɾ���������Ļ����ԭ��ţ��µĻ�����������
	rtc_time_t  Act_Start_Time;  			//���ʼʱ��
	rtc_time_t	Act_Stop_Time;   			//�����ʱ��
	uint16_t	CircleDistance;             //��Ȧ����
	uint32_t	ActTime;                    //�˶�ʱ��
	uint32_t	Steps_Start_Address;		//�Ʋ����ݿ�ʼ��ַ
	uint32_t	Steps_Stop_Address;			//�Ʋ����ݽ�����ַ
	uint32_t	HeartRate_Start_Address;	//�������ݿ�ʼ��ַ
	uint32_t	HeartRate_Stop_Address;		//�������ݽ�����ַ
	uint32_t	Pressure_Start_Address;		//��ѹ���ݿ�ʼ��ַ
	uint32_t	Pressure_Stop_Address;		//��ѹ���ݽ�����ַ
	uint32_t	GPS_Start_Address;			//GPS���ݿ�ʼ��ַ
	uint32_t	GPS_Stop_Address;			//GPS���ݽ�����ַ
	uint32_t	Distance_Start_Address;		//�������ݿ�ʼ��ַ
	uint32_t	Distance_Stop_Address;		//�������ݽ�����ַ
	uint32_t	Energy_Start_Address;     	//����������ʼ��ַ
	uint32_t	Energy_Stop_Address;     	//�������ݽ�����ַ
	uint32_t	Speed_Start_Address;     	//�ٶ�������ʼ��ַ
	uint32_t	Speed_Stop_Address;     	//�ٶ����ݽ�����ַ
	uint32_t	DistancePoint_Start_Address;//�����㿪ʼ��ַ
	uint32_t	DistancePoint_Stop_Address;	//�����������ַ
	uint32_t	CircleTime_Start_Address;   //��Ȧʱ����ʼ��ַ
	uint32_t	CircleTime_Stop_Address;    //��Ȧʱ�������ַ
	uint32_t    cod_id;				 	//��¼ID��Ψһ�ı���һ����¼�������������ֵ��+1�ص�0��
	uint32_t 	half_marathon_time;    //����ʱ�䣬��������ͣ��s
    uint32_t 	full_marathon_time;    //ȫ��ʱ�䣬��������ͣ��s
	uint32_t 	cod_version;              //�˶���¼ʱ�̼��汾��(ע�ⲻ���ϴ�ʱ�汾��),��汾
	uint32_t 	agps_update_time;		 //������������ʱ���s�����û��������������2018��1��1��0ʱ0��0��
	uint32_t 	reserve;
	union   Activity_Details_U
	{
		uint8_t						Dat[48]; //ȷ��ռ�ݿռ�
		CommonDetailStr			    CommonDetail;     //�������ݽṹ
		SwimmingDetailStr		    SwimmingDetail;    //��Ӿ������������Ӿ
	}ActivityDetails;
} ActivityDataStr;//���181�ֽ�



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
	uint8_t gender;  //�Ա�
	uint8_t age;
	uint8_t height;         //���:��λCM
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
 * �û�ѵ��Ͷ����Ϣ
 */
typedef struct {
    cod_ble_training_status training_status;
    cod_ble_training_type training_type;
    cod_ble_training_action_type action_type;
    char course_name[20];          //�γ����ƣ�����'\0'
    uint16_t course_name_len;   //�γ����Ƴ��ȣ�����'\0'
    char action_name[20];          //�������ƣ�����'\0'
    uint16_t action_name_len;   //�������Ƴ��ȣ�����'\0'
    uint16_t action_target;     //����Ŀ��ֵ
    uint16_t action_actual;     //����ʵ��ֵ
    float calorie;              //��·��, ��
    uint32_t time;              //ʱ������
    uint8_t heart_rate;         //ƽ������,����״̬ʱ�ᴫ����Χ0-255
} cod_user_training_cast;

//ȫ��Ʋ��洢��Ϣ
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

//��������
void cmd_cod_on_check_agps(void);

//���ݷ���
void app_srv_bas_level_send(uint8_t *p_data, uint16_t length);



#endif //__APP_SRV_NRF52_H__
