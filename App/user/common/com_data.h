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
����+����	0ʱ	1ʱ	2ʱ	...	23ʱ	    ���ݿ��С	  ��ַ	
��һ��	 �ճ��ܲ���					  4*24+4=100	  0x04	һ��Ĵ洢��С0x2EC
	       �ճ��ܿ�·��					4*24=96	      0x64	
	       �ճ��ܾ���				    4*24=96	      0xC4	
	       ��ܲ���				    4*24=96	      0x124	
	       ��ܿ�·��					4*24=96	      0x184	
	       ��ܾ���				    4*24=96	      0x1E4	
	       ��������				    	1*24=24	      0x244	
	       �����¶�				    	2*24=48	      0x25C	
	       ��ѹ				        	4*24=96	      0x28C	
				 ���������           2*24=48       0x2EC
				 �ⶨ�������ٶ�       2*24=48       0x31C
         �ⶨ����������ֵ     1*24=24       0x34C
				 
				 �����������  ��һ��� ��DAY_COMMON_DATA_SIZE��ʼ���������С sizeof(ActivityDataStr),
				                 �ڶ���� ��ַDAY_COMMON_DATA_SIZE+sizeof(ActivityDataStr),��������
				 
				 
����+����	0ʱ	1ʱ	2ʱ	...	23ʱ	    ���ݿ��С	  ��ַ	
�ڶ���	 �ճ��ܲ���					  4*24+4=100	  0x04	һ��Ĵ洢��С0x2EC
	       �ճ��ܿ�·��					4*24=96	      0x64	
	       �ճ��ܾ���				    4*24=96	      0xC4	
	       ��ܲ���				    4*24=96	      0x124	
	       ��ܿ�·��					4*24=96	      0x184	
	       ��ܾ���				    4*24=96	      0x1E4	
	       ��������				    	1*24=24	      0x244	
	       �����¶�				    	2*24=48	      0x25C	
	       ��ѹ				        	4*24=96	      0x28C	
				 ���������           2*24=48       0x2EC
				 �ⶨ�������ٶ�       2*24=48       0x31C
         �ⶨ����������ֵ     1*24=24       0x34C
...								
������	 �ճ��ܲ���					  4*24+4=100	  0x04	һ��Ĵ洢��С0x2EC
	       �ճ��ܿ�·��					4*24=96	      0x64	
	       �ճ��ܾ���				    4*24=96	      0xC4	
	       ��ܲ���				    4*24=96	      0x124	
	       ��ܿ�·��					4*24=96	      0x184	
	       ��ܾ���				    4*24=96	      0x1E4	
	       ��������				    	1*24=24	      0x244	
	       �����¶�				    	2*24=48	      0x25C	
	       ��ѹ				        	4*24=96	      0x28C			
				 ���������           2*24=48       0x2EC
				 �ⶨ�������ٶ�       2*24=48       0x31C
         �ⶨ����������ֵ     1*24=24       0x34C
								                                  ����洢�ܺ�0x1474
**/
#define		DAY_COMMON_STEPS_OFFSET				    0x04	  //�ճ��ܲ����洢ƫ�Ƶ�ַ
#define		DAY_COMMON_ENERGY_OFFSET				  0x64	  //�ճ��ܿ�·��洢ƫ�Ƶ�ַ
#define		DAY_COMMON_DISTANCE_OFFSET				0xC4	  //�ճ��ܾ���洢ƫ�Ƶ�ַ
#define		DAY_SPORTS_STEPS_OFFSET           0x124   //��ܲ����洢ƫ�Ƶ�ַ
#define		DAY_SPORTS_ENERGY_OFFSET          0x184   //��ܿ�·��洢ƫ�Ƶ�ַ
#define		DAY_SPORTS_DISTANCE_OFFSET        0x1E4   //��ܾ���洢ƫ�Ƶ�ַ
#define   DAY_BASE_HDR_OFFSET               0x244   //�������ʴ洢ƫ�Ƶ�ַ
#define		DAY_TEMPRATURE_OFFSET			        0x25C	  //�����¶ȴ洢ƫ�Ƶ�ַ
#define   DAY_PRESSURE_OFFSET               0x28C   //��ѹ�洢ƫ�Ƶ�ַ
#define   DAY_VO2MAXMEASURING_OFFSET        0x2EC   //���������ƫ�Ƶ�ַ
#define   DAY_LTSPEEDMEASURING_OFFSET       0x31C   //�ⶨ�������ٶ� 
#define   DAY_LTBMPMEASURING_OFFSET         0x34C   //�ⶨ����������ֵ

#define   DAY_COMMON_DATA_SIZE              0x364   //ÿ��(24Сʱ)�洢���ճ����ݴ�С
#define   DAY_COMMON_ACTIVITY_DATA_SIZE     (DAY_COMMON_DATA_SIZE + sizeof(ActivityDataStr))//ÿ����ճ�����+������ܺʹ�С

#define		DAY_COMMON_STEPS_LENGTH				    4
#define		DAY_COMMON_DISTANCE_LENGTH				4
#define		DAY_COMMON_ENERGY_LENGTH				  4	
#define		DAY_SPORTS_STEPS_LENGTH           4
#define		DAY_SPORTS_ENERGY_LENGTH          4
#define		DAY_SPORTS_DISTANCE_LENGTH        4
#define   DAY_BASE_HDR_LENGTH               1
#define   DAY_PRESSURE_LENGTH               4
#define		DAY_TEMPRATURE_LENGTH			        2
#define   DAY_VO2MAXMEASURING_LENGTH        2   //���������ƫ�Ƶ�ַ
#define   DAY_LTSPEEDMEASURING_LENGTH       2   //�ⶨ�������ٶ� 
#define   DAY_LTBMPMEASURING_LENGTH         1  //�ⶨ����������ֵ


/*
�Ʋ����ݴ洢��,ÿ������4�ֽڣ�Ϊ��ǰ�ܲ�������λΪ ����ÿ���Ӵ洢һ������;
�������Ϊ����ʱ�洢����Ϊ̤Ƶ����Ӿʱ�洢����Ӿ��
*/
#ifdef COD
//������Ҫ�洢ʱ���������Ϊ16k
#define		STEP_DATA_START_ADDRESS			    ((uint32_t)0x0000F000)  //�Ʋ����ݿ�ʼ��ַ      //activity
#define		STEP_DATA_STOP_ADDRESS			    ((uint32_t)0x00012FFF)  //�Ʋ����ݽ�����ַ      //activity

/*
�������ݴ洢��,ÿ������һ�ֽڣ���λΪ��/�֣�ÿ���Ӵ洢һ������(һ����ƽ������);
*/
//������Ҫ�洢ʱ���,��20һ�Σ�����Ϊ48k
#define		HEARTRATE_DATA_START_ADDRESS	    ((uint32_t)0x0002C000)  //�������ݿ�ʼ��ַ    //activity
#define		HEARTRATE_DATA_STOP_ADDRESS	      ((uint32_t)0x00037FFF)  //�������ݽ�����ַ    //activity

#else 
#define		STEP_DATA_START_ADDRESS			    ((uint32_t)0x0000F000)  //�Ʋ����ݿ�ʼ��ַ      //activity
#define		STEP_DATA_STOP_ADDRESS			    ((uint32_t)0x00010FFF)  //�Ʋ����ݽ�����ַ      //activity

/*
�������ݴ洢��,ÿ������һ�ֽڣ���λΪ��/�֣�ÿ���Ӵ洢һ������(һ����ƽ������);
*/
#define		HEARTRATE_DATA_START_ADDRESS	    ((uint32_t)0x00011000)  //�������ݿ�ʼ��ַ    //activity
#define		HEARTRATE_DATA_STOP_ADDRESS	      ((uint32_t)0x00012FFF)  //�������ݽ�����ַ    //activity
#endif
/*
��ѹ�߶����ݴ洢��,ÿ�����ݰ�����ѹ�͸߶�����ֵ��ÿ��ֵ�ĸ��ֽڣ���ѹΪ4�ֽڸ���������λ:hPa(����)
�߶�ֵΪ���ֽڸ���������λΪ��;
5���Ӵ洢һ������
*/
#define		PRESSURE_DATA_START_ADDRESS		    ((uint32_t)0x00013000)  //��ѹ���ݿ�ʼ��ַ    //activity//������Ŀʵ�����ڹ����ơ���Ӿ�˵�
#define		PRESSURE_DATA_STOP_ADDRESS		    ((uint32_t)0x00014FFF)  //��ѹ���ݽ�����ַ    //activity

#define		DISTANCE_DATA_START_ADDRESS		    ((uint32_t)0x00015000)  //�������ݿ�ʼ��ַ    //activity
#define		DISTANCE_DATA_STOP_ADDRESS		    ((uint32_t)0x00016FFF)  //�������ݽ�����ַ    //activity

#define		ENERGY_DATA_START_ADDRESS		    ((uint32_t)0x00017000)  //�������ݿ�ʼ��ַ        //activity
#define		ENERGY_DATA_STOP_ADDRESS		    ((uint32_t)0x00018FFF)  //�������ݽ�����ַ        //activity

#define		WEATHER_DATA_START_ADDRESS			((uint32_t)0x00019000)	//�����洢��ʼ��ַ        
#define		SPEED_DATA_START_ADDRESS		    ((uint32_t)0x0001B000)  //�ٶ����ݿ�ʼ��ַ        //activity
#define		SPEED_DATA_STOP_ADDRESS		      ((uint32_t)0x0001CFFF)  //�ٶ����ݽ�����ַ        //activity

/*
������洢��,ÿ��������Ϊ8�ֽڣ����Ⱥ�γ�ȸ�ռ4�ֽڣ�Ϊ������������ʵ��ֵ��1000000��,Ӿ����ӾΪ������

*/
#ifdef COD
#define		DISTANCEPOINT_START_ADDRESS		    ((uint32_t)0x00038000)  //��ͣʱ�俪ʼ��ַ,����ʵ������gps�����洢
#define		DISTANCEPOINT_STOP_ADDRESS		      ((uint32_t)0x00077FFF)  //��ͣʱ�������ַ ������ʵ������gps�����洢
#define 	DISTANCEPOINT_COD_SIZE 			  ((uint32_t)0x00040000)
#else
#define		DISTANCEPOINT_START_ADDRESS	        ((uint32_t)0x0001D000)  //�����㿪ʼ��ַ    //activity
#define		DISTANCEPOINT_STOP_ADDRESS	        ((uint32_t)0x0001EFFF)  //�����������ַ    //activity
#endif


#if defined WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
#define		CIRCLETIME_START_ADDRESS		    ((uint32_t)0x00400000)  //��Ȧʱ�俪ʼ��ַ        //activity
#define		CIRCLETIME_STOP_ADDRESS		        ((uint32_t)0x004FFFFF)  //��Ȧʱ�������ַ        //activity
#else
#define		CIRCLETIME_START_ADDRESS		    ((uint32_t)0x0001F000)  //��Ȧʱ�俪ʼ��ַ        //activity
#define		CIRCLETIME_STOP_ADDRESS		      ((uint32_t)0x00020FFF)  //��Ȧʱ�������ַ        //activity
#endif

#define		PAUSE_DATA_START_ADDRESS		    ((uint32_t)0x00021000)  //��ͣʱ�俪ʼ��ַ ��ʵ�����ڴ洢�����������      //activity
#define		PAUSE_DATA_STOP_ADDRESS		      ((uint32_t)0x00022FFF)  //��ͣʱ�������ַ ��ʵ�����ڴ洢�����������      //activity

#define		AMBIENT_DATA_START_ADDRESS			((uint32_t)0x00023000)	//�����������

#define		RESERVE1_DATA_START_ADDRESS		    ((uint32_t)0x00024000)  //����1����ʼ��ַ
#ifdef COD
#define		ALL_DAY_STEPS_START_ADDRESS		    ((uint32_t)0x00024000)  //ȫ��Ʋ���ʼ��ַ
#define		ALL_DAY_STEPS_END_ADDRESS		    ((uint32_t)0x0002AFFF)  //ȫ��Ʋ�������ַ
#define		ALL_DAY_STEPS_SIZE		    		((uint32_t)0x00001000)  //ȫ��Ʋ�ÿ��洢��С

#define		COD_BLE_DEV_START_ADDRESS		    ((uint32_t)0x0002B000)  //��������������Ϣ��ʼ��ַ
#define		COD_BLE_DEV_END_ADDRESS		    	((uint32_t)0x0002BFFF)  //��������������Ϣ������ַ
#define		COD_BLE_DEV_SIZE		    		((uint32_t)0x00001000)  //��������������Ϣ�洢��С

#endif 
#define		GPS_DATA_START_ADDRESS			    ((uint32_t)0x00100000)  //GPS���ݿ�ʼ��ַ         //activity
#define		GPS_DATA_STOP_ADDRESS			      ((uint32_t)0x001FFFFF)  //GPS���ݽ�����ַ         //activity

#define		GPSTRACK_DATA_START_ADDRESS			((uint32_t)0x00200000) //����켣GPS���ݿ�ʼ��ַ  //app

#define		SATELLITE_DATA_START_ADDRESS		((uint32_t)0x00278000)	//�������ݿ�ʼ��ַ        //app


#define		REALTIME_STEP_START_ADDRESS		    ((uint32_t)0x0029C000)	//ʵʱ������ʼ��ַ    //not app
#define		CUSTOMTHEME_START_ADDRESS		    ((uint32_t)0x002A0000)	//�Զ�����̿�ʼ��ַ     //app
#ifndef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
#define		VERTICALSIZE_START_ADDRESS		    ((uint32_t)0x00400000)	//��ֱ���ȿ�ʼ��ַ    //activity
#define		VERTICALSIZE_STOP_ADDRESS		      ((uint32_t)0x004FFFFF)	//��ֱ���Ƚ�����ַ    //activity
#endif
#define		TOUCHDOWN_START_ADDRESS		        ((uint32_t)0x00500000)	//����ʱ�俪ʼ��ַ     //activity
#define		TOUCHDOWN_STOP_ADDRESS		        ((uint32_t)0x005FFFFF)	//����ʱ�������ַ     //activity

#define		NOTIFY_DATA_START_ADDRESS		    ((uint32_t)0x00600000)	//֪ͨ��¼��ʼ��ַ
#define		TRAINING_DATA_START_ADDRESS		    ((uint32_t)0x00619000)	//ѵ���ƻ���ʼ��ַ      //app �ֻ����ֱ�
#define   TRAINING_DATA_UPLOAD_ADDRESS      ((uint32_t)0x00629000)	//ѵ���ƻ���Ҫ�ϴ������ݵĿ�ʼ��ַ   //app �ֱ��ֻ�

#define		TIME_CALIBRATION_ADJ_DATA_START_ADDRESS		    ((uint32_t)0x00639000)  //����ʱ�� Adjֵ��ʱ��Ȳ������� �ָ���������ɾ��  ���ɲ�д
#define		TIME_CALIBRATION_VALUE_DATA_START_ADDRESS		  ((uint32_t)0x0063A000)  //����ʱ�� Tcalibrationֵ��ʱ��Ȳ������� �ָ���������ɾ�� �ɲ�д
#define		TIME_CALIBRATION_STSTUS_START_ADDRESS		  ((uint32_t)0x0063B000)//����ʱ�� ״ֵ̬����ָ���������ɾ�� �ɲ�д
#if defined  WATCH_IMU_CALIBRATION
#define		ACC_CALIBRATION_PARAMETER_STORAGE_ADDRESS		  ((uint32_t)0x00651000)//���ٶ�У׼��������ָ���������ɾ�� �ɲ�д
#define		GYRO_CALIBRATION_PARAMETER_STORAGE_ADDRESS		  ((uint32_t)0x00652000)//������У׼��������ָ���������ɾ�� �ɲ�д
#endif
#define		MAGN_CALIBRATION_PARAMETER_STORAGE_ADDRESS		  ((uint32_t)0x00655000)//������У׼��������ָ���������ɾ�� �ɲ�д

#define		UPGRADE_DATA_START_ADDRESS		    ((uint32_t)0x00700000)  //�����洢���ݿ�ʼ��ַ

/* 
�˶�����mask
bitλ��������:
0:���ʸ澯 1:��������2:��������3�Զ���Ȧ����4:Ŀ��Ȧ������5:ȼ֬����6:Ŀ��ʱ��7:�߶�����
*/
#define SW_HEARTRATE_MASK       0x01
#define SW_PACE_MASK            0x02
#define SW_DISTANCE_MASK        0x04
#define SW_CIRCLEDISTANCE_MASK  0x08
#define SW_GOALCIRCLE_MASK      0x10
#define SW_GOALKCAL_MASK        0x20
#define SW_GOALTIME_MASK        0x40
#define SW_ALTITUDE_MASK        0x80




#define LOCK_SCREEN_TIME   59     //�Զ�����ʱ��

#define AMBIENT_DATA_MAX_NUM	90		//��������������洢����


//#define	DEFAULT_SPORT_MODE  0x3E5

#define	DEFAULT_SPORT_MODE  0x3F


//����
enum
{
	L_CHINESE	= 0U,	//����
	L_ENGLISH	= 1U	//Ӣ��
	
};
enum
{

  THEME_SIMPLENUMBER = 0U,//��Լ����
  THEME_SIMPLEBUSINESS = 1U,//��Լ����
	THEME_VINTAGEBUSINESS = 2U,//��������
	THEME_APPDOWNLOAD = 3U,//APP��������

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
	Rest = 0U,        //��Ϣ  
	NORMAL_RUN,      //������  ��ʱûʹ��
	PACE_RUN,        //������  ��ʱûʹ��
	TEMPO_RUN,       //������  ��ʱûʹ��
	INTERVALS,       //��϶��  ��ʱûʹ��
	AEROBIC_RUN,     //������  ��ʱûʹ��
	EASY_RUN,        //������  ��ʱûʹ��
	WALK_AND_RUN,    //���ܽ�� ��ʱûʹ��
	BRISK_WALK,      //����    ��ʱûʹ��
	SLOW_WALK,       //����    ��ʱûʹ��
	STRENGTH_TRAIN,  //����ѵ�� ��ʱûʹ��
	CROSS_TRAIN,     //����ѵ�� ��ʱûʹ��
	CYCLING,         //����    ��ʱûʹ��
	SWIM,            //��Ӿ    ��ʱûʹ��
	
	StrengthTraining,        //����ѵ��
	CrossTraining,           //����ѵ��
	Rest_StrengthTraining,   //��Ϣ������ѵ��
	Rest_CrossTraining,      //��Ϣ�򽻲�ѵ��
	EasyToRun_Distance,     //������-����
	EasyToRun_Time,          //������-ʱ��
	NormalRun_Distance,      //�ܲ�-����
	NormalRun_Time,          //�ܲ�-ʱ��
	TempoRuns_Distance,      //������-����
	TempoRuns_Time,          //������-ʱ��
	PacetoRun_Distance,      //������-����
	PacetoRun_Time,          //������-ʱ��
	Intervals_Distance,      //��Ъѵ��-����
	Intervals_Time,          //��Ъѵ��-ʱ��
	Marathon_Distance,       //������-����
	Rest_Run_Distance,       //��Ϣ���ܲ�-����
}TrainPlanType;

#define TRAIN_PLAN_MAX_DAYS        126
#define TRAIN_PLAN_DOWNLOAD_DATA_SIZE  (sizeof(TrainPlanDownStr)+sizeof(DownloadStr)*TRAIN_PLAN_MAX_DAYS)  //ѵ���ƻ� app�·����ֱ�����ݴ�С
typedef struct __attribute__((packed))
{
	TrainPlanType  Type;           //�����              0-��Ϣ1����ѵ��2����ѵ��3��Ϣ������ѵ��4��Ϣ�򽻲�ѵ��5������-����6������-ʱ��7�ܲ�-����8�ܲ�-ʱ��9������-����10������-ʱ��11...
	char ContentStr[20];           //ѵ������
	char SkillsStr[30];            //ѵ������
	uint16_t TargetHour;           //ѵ����׼-ʱ��              Min���ӣ�����ʱʱ�䣬���硰5���ӽ����ܡ�����Ŀ��ʱ��Ϊ5������0��ʾ����ʱ������
	uint16_t TargetDistance;       //ѵ����׼-���            m�ף��������룬���硰5���������ܡ�����Ŀ�����Ϊ5000������0��ʾ������������
	uint16_t TargetCalories;       //ѵ����׼-��·��            ��λ:��
	uint16_t TargetSpeedLimitDown; //ѵ����׼-��������          s/km��/���� ������0��ʾû����������
	uint16_t TargetSpeedLimitUp;   //ѵ����׼-��������         s/km��/���� ������0��ʾû����������
	uint16_t RestTime;             //��Ъ��/���ܵ���Ϣʱ�� ��λ:��
	uint8_t  IntermittentRun;      //��λ:��  0����û�м�Ъ������2������Ъ2�Σ�����ʱ��=Ŀ��ʱ��/(2)-��Ъʱ��
	uint8_t Reserved[16];          //Ԥ��16���ֽ�
}DownloadStr;//ѵ���ƻ�ÿ�����  APP�·����ֱ��


typedef struct __attribute__((packed))
{
	uint8_t  IsComplete;           //��ɱ�־              0 δ���    1�����
	uint8_t  RecordIndex;          //ѵ����¼����      1-40 ��ѵ���ƻ����ݵĴ洢���ջ�洢�������¼�洢�Ļ��ţ���������ѵ����¼������
	uint16_t Hour;                 //���-ʱ��      ��λ��
	uint16_t Distance;             //���-���    ��λ��
	uint16_t Calories;             //���-��·��   ��λ��
	uint8_t Reserved[16];          //Ԥ��16���ֽ�
}UploadStr;//ѵ���ƻ�ÿ�����  �ֱ���ϴ���APP��


typedef struct __attribute__((packed))
{
	uint8_t Md5Str[32];           //MD5 �ַ���
	uint8_t UUIDStr[32];           //UUID �ַ���
	char NameStr[20];             //����
	TrainPlanHint Hint;           //����ʱ�� ʱ��(24Сʱ��)
	TrainPlanSport StartTime;     //��/��/��
	TrainPlanSport StopTime;      //��/��/��
	uint8_t Reserved[16];         //Ԥ��16���ֽ�
}TrainPlanDownStr;//ѵ���ƻ�app�·�����ͷ


typedef struct __attribute__((packed))
{
	uint8_t Md5Str[32];           //MD5 �ַ���
	uint8_t UUIDStr[32];           //UUID �ַ���
	uint8_t Reserved[16];         //Ԥ��16���ֽ�
}TrainPlanUploadStr;//ѵ���ƻ��ֱ��ϴ�����ͷ



//ѵ���ƻ����ݽ���ϴ� --��������
typedef struct __attribute__((packed))
{
	uint32_t Address;//��address��ַ��ʼ��
	uint32_t Size;   //��С
	uint16_t Days;   //�ϴ�������
	uint8_t Is_HasTodayResult;//�Ƿ��������ϴ� 0 û�������ϴ� 1�������ϴ�
}TrainPlanUploadBLEData;
typedef struct __attribute__((packed))
{
	uint8_t Md5Str[32];           //MD5 �ַ���
	uint8_t UUIDStr[32];           //UUID �ַ���
}_com_config_train_plan_MD5_UUID;

typedef struct __attribute__((packed))
{
	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
	uint8_t Msecond;//�ٺ���
}TimeStr;
//����ʱ�� Adjֵ��ʱ��Ȳ������� �ָ���������ɾ��  ���ɲ�д
typedef struct __attribute__((packed))
{
	uint8_t IsWrite;//�Ƿ�д�� 1д�� 0 δд��
	TimeStr AdjSaveTime;//����Adjֵ��ʱ��
	TimeStr GpsFirstTime;//����Adjֵ����һ��GPS����Ч��ʱ��������ʱ��AdjSaveTime-GpsFirstTime��[24,30*24]֮��
	int32_t AdjValue;//Adj�����ֵ
	uint8_t Reserved[12];
}TimeAdjStr;
//����ʱ�� Tcalibrationֵ��ʱ��Ȳ������� �ָ���������ɾ�� �ɲ�д
typedef struct __attribute__((packed))
{
	uint8_t IsWrite;//�Ƿ�д�� 1д�� 0 δд��
	TimeStr CalibrationSaveTime;//����Calibrationֵ��ʱ��
	TimeStr GpsSecondTime;//����Calibrationֵ����һ��GPS����Ч��ʱ��������CalibrationSaveTime-GpsSecondTime��[24,30*24]֮��
	int64_t CalibrationValue;//calobration�����ֵ
	uint8_t Reserved[12];
}TimeCalibrationStr;
//����ʱ�� ״̬���� ��������3��
typedef struct __attribute__((packed))
{
	uint32_t status;//״̬
	uint8_t Reserved[12];
}TimeCalibrationStatus;
#ifdef COD
typedef struct __attribute__ ((packed)){
	uint32_t command;
	uint8_t gender;  //�Ա�
	uint8_t age;
	uint8_t height;         //���:��λCM
	uint8_t weight;
}_ble_user_info;

/**
 * �豸��ǰ�İ�״̬
 * */
typedef enum {
    STATUS_BINDING,
    STATUS_BIND_SUCCESS,
    STATUS_BIND_FAIL
} cod_bind_status;
/**
 * �˶�״̬
 */
typedef enum{
    COD_SPORT_STATUS_START = 0,      //�˶���ʼ
    COD_SPORT_STATUS_SPORTING,       //�˶���
    COD_SPORT_STATUS_PAUSE,          //�˶���ͣ
    COD_SPORT_STATUS_STOP,           //�˶�����
} cod_ble_sport_status;
/**
 * �˶�����
 */
typedef enum{
    SPORT_TYPE_OUTDOOR_RUN = 1,    //������
    SPORT_TYPE_OUTDOOR_WALK,       //���⽡��(ͽ��ԽҰ)
    SPORT_TYPE_RIDE,               //����
    SPORT_TYPE_CLIMB,              //��ɽ
    SPORT_TYPE_INDOOR_RUN,         //������
    SPORT_TYPE_INDOOR_WALK,        //���ڽ���
    SPORT_TYPE_SKI,                //��ѩ
    SPORT_TYPE_SKATE,              //����
} cod_ble_sport_type;


/**
 * ѵ������
 */
typedef enum{
    TRAINING_TYPE_UNKNOWN = 0,      //δ֪
    TRAINING_TYPE_NORMAL,           //��ͨѵ��
    TRAINING_TYPE_INTERACT,         //����ʽ�γ�
    TRAINING_TYPE_LIVE,             //ֱ��ѵ��
} cod_ble_training_type;

/**
 * ѵ��״̬
 */
typedef enum{
    TRAINING_STATUS_UNKNOWN = 0,        //δ֪
    TRAINING_STATUS_PREPARE,            //׼��
    TRAINING_STATUS_START,              //��ʼ
    TRAINING_STATUS_INTRODUCT,          //����
    TRAINING_STATUS_TRAINING,           //ѵ����
    TRAINING_STATUS_REST,               //��Ϣ
    TRAINING_STATUS_PAUSE,              //��ͣ
    TRAINING_STATUS_RESUME,             //�ָ�
    TRAINING_STATUS_STOP,               //����
} cod_ble_training_status;

/**
 * Ͷ�����
 */
typedef enum{
    COD_BLE_CAST_RESULT_OK = 0,          //Ͷ���ɹ�
    COD_BLE_CAST_RESULT_NOT_SUPPORT = 1, //��֧��Ͷ��
    COD_BLE_CAST_RESULT_SPORTING = 0xff, //��ǰ���ڶ����˶�����ظ�������0xFF
} cod_ble_cast_result;


/**
 * ѵ����������
 */
typedef enum{
    TRAINING_ACTION_TYPE_TIME = 1,      //ʱ��
    TRAINING_ACTION_TYPE_COUNT,         //����
} cod_ble_training_action_type;

/**
 * �˶�Ͷ����Ϣ
 */
typedef struct {
    cod_ble_sport_status sport_status;
    cod_ble_sport_type sport_type;
    float distance;                 //��
    uint32_t time;                  //ʱ������
    uint16_t pace;                  //���٣���
    uint8_t heart_rate;             //ʵʱ����
    uint8_t avg_heart_rate;         //ƽ������
    uint32_t step;                  //����
    float rt_speed;                 //ʵʱ�ٶ�, ǧ��/Сʱ
    float avg_speed;                //ƽ���ٶ�, ǧ��/Сʱ
    float calorie;                  //��·��, ��
    int16_t altitude;              //����
    int16_t altitude_diff;         //��������
    int16_t max_altitude;          //��ߺ���
    uint32_t total_altitude_increase;//�ۻ�����
    uint32_t climb_distance;        //�������
} cod_ble_sport_cast;


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
} _cod_user_training_cast;

//ȫ��Ʋ��洢��Ϣ
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
    uint8_t weather;        //����
    int8_t temperature;     //�¶�
    uint16_t aqi;			//��������ָ��
} cod_ble_weather;

typedef struct __attribute__ ((packed))
{
	uint8_t date[4]; //����ͬ��ʱ�� :��һ���ֽ������2018Ϊ18���ڶ����ֽ��£��������ֽ�����.�����ֽ�ΪСʱ
	uint8_t city[10];//��������������UNICODE���5�����֣�Ӣ����ASCII�������10��Ӣ��
	cod_ble_weather cod_weather[24]; //24Сʱ����
} CodWeatherSync; //APP�������ݽṹ��
/**
 * ������������
 */
typedef enum{
    DATA_TYPE_STEP = 1,      //ȫ��Ʋ�
    DATA_TYPE_SLEEP,         //˯������
    DATA_TYPE_HEART,         //��������
    DATA_TYPE_BP,            //Ѫѹ����
    DATA_TYPE_SPORT,         //�˶�����
    DATA_TYPE_LOG = 10       //��־����
} cod_ble_data_type;

void all_day_steps_reinit(void);
void Store_all_day_steps(void);
void Store_all_day_time(void);
void read_all_day_steps(void);
void delete_all_day_steps(void);


#endif
//App�������ݸ�ʽ
typedef struct __attribute__((packed))
{
	uint8_t		Age;						//���� //APP����
	uint8_t		Sex;						//�Ա�  0Ů  1 �� //APP����
	uint8_t		Height;						//���  ��λ��cm //APP����
	uint8_t		Weight;						//����  ��λ��kg //APP����
	uint8_t		Stride;						//����(�ճ�ͽ������) ��λ��cm //APP����
	uint16_t	Goal_Steps;					//Ŀ�경�� //APP����
	uint16_t	Goal_Energy;				//Ŀ�꿨·�� ��λ���� //APP����
	uint8_t		Goal_Distance;				//Ŀ�����   ��λ������ //APP����
	uint16_t	VO2Max;						//��������� ��λ��ml/kg/min //APP����
	uint8_t   LactateThresholdHeartrate; //����������ֵ //APP����
	uint16_t  LactateThresholdSpeed;    //�������ٶ�	 //APP����
	uint8_t   htr_zone_one_min;       //��������1��ֵ //APP����
	uint8_t   htr_zone_one_max;       //��������1��ֵ //APP����
	uint8_t   htr_zone_two_min;       //��������2��ֵ //APP����
	uint8_t   htr_zone_two_max;       //��������2��ֵ //APP����
	uint8_t   htr_zone_three_min;     //��������3��ֵ //APP����
	uint8_t   htr_zone_three_max;     //��������3��ֵ //APP����
	uint8_t   htr_zone_four_min;      //��������4��ֵ //APP����
	uint8_t   htr_zone_four_max;      //��������4��ֵ //APP����
	uint8_t   htr_zone_five_min;      //��������5��ֵ //APP����
	uint8_t   htr_zone_five_max;      //��������5��ֵ //APP����
	uint32_t  Sea_Level_Pressure;	  //��ƽ�����ѹ  ͨ��app�����������ȡ
}AppSetStr;

//App��ȡ���ݸ�ʽ
typedef struct __attribute__((packed))
{
	uint8_t    LengthSwimmingPool;            //Ӿ�س���
	uint8_t    StepLengthRun;                 //�ܲ�����
	uint8_t    StepLengthCountryRace;         //ԽҰ����
	uint8_t    StepLengthMarathon;            //�����ɲ���
	uint8_t    StepLengthWalking;             //���߲���
	uint8_t    StepLengthClimbing;            //��ɽ����
	uint8_t    StepLengthIndoorRun;           //�����ܲ���
	uint8_t    StepLengthHike;           	    //ͽ��ԽҰ����
  uint16_t   VO2MaxMeasuring;               //�ⶨ��������� ��λ��ml/kg/min
	uint16_t   LTSpeedMeasuring;              //�ⶨ�������ٶ�  s/km
	uint8_t    LTBmpMeasuring;                //�ⶨ����������ֵ 
	uint8_t    CalibrationStride;             //У׼���� ��λcm���� ���ܳ���255cm
	uint8_t  	 Reserved[12];                  //Ԥ��
}AppGetStr;


//�˶������������ݸ�ʽ
typedef struct  __attribute__((packed))
{
	uint8_t    SwRun;                         //�ܲ�����(bit 0:���ʸ澯 1:��������2:��������3�Զ���Ȧ����4:Ŀ��Ȧ������5:ȼ֬����6:Ŀ��ʱ��7:�߶�����,��ͬ)
	uint8_t    SwMarathon;                    //�����ɿ���
	uint8_t    SwCountryRace;                 //ԽҰ�ܿ���
	uint8_t    SwSwimmg;                      //��Ӿ����
	uint8_t    SwCycling;                     //���п���
	uint8_t    SwClimbing;                    //��ɽ����
	uint8_t    SwWalking;                     //���߿���
	uint8_t    SwHike;                        //ͽ��ԽҰ����
	uint8_t    SwIndoorRun;                   //�����ܿ���
	uint8_t    SwOutdoorSwimmg;               //������Ӿ����
	
	uint16_t   DefHintSpeedRun;               //�ܲ���������Ĭ��ֵ ��λ����/����
	uint16_t   DefHintSpeedCountryRace;       //ԽҰ����������Ĭ��ֵ
	uint16_t   DefHintSpeedMarathon;          //��������������Ĭ��ֵ
	uint16_t   DefHintSpeedCycling;           //������������Ĭ��ֵ ����ʵ��Ϊ�ٶȡ���λ:10X km/h
	uint16_t   DefHintSpeedIndoorRun;         //��������������Ĭ��ֵ
	
	uint16_t   DefHintDistanceRun;            //�ܲ���������Ĭ��ֵ ��λ:��
	uint16_t   DefHintDistanceCountryRace;    //ԽҰ�ܾ�������Ĭ��ֵ
	uint16_t   DefHintDistanceMarathon;       //�����ɾ�������Ĭ��ֵ
	uint16_t   DefHintDistanceCycling;        //���о�������Ĭ��ֵ
	uint16_t   DefHintDistanceClimbing;       //��ɽ��������Ĭ��ֵ
	uint16_t   DefHintDistanceIndoorRun;      //�����ܾ�������ֵ

	
	uint16_t   DefCircleDistanceRun;          //�ܲ�Ĭ�ϼ�Ȧ���� ��λ:��
	uint16_t   DefCircleDistanceWalk;         //����Ĭ�ϼ�Ȧ���� 
	uint16_t   DefCircleDistanceCountryRace;  //ԽҰ��Ĭ�ϼ�Ȧ����
	uint16_t   DefCircleDistanceMarathon;     //������Ĭ�ϼ�Ȧ����
	uint16_t   DefCircleDistanceCycling;      //����Ĭ�ϼ�Ȧ����
	uint16_t   DefCircleDistanceIndoorRun;    //������Ĭ�ϼ�Ȧ����
	uint16_t   DefCircleDistanceClimbing;      //��ɽĬ�ϼ�Ȧ����
	uint16_t   DefCircleDistanceHike;          //ͽ��ԽҰĬ�ϼ�Ȧ����
	uint16_t   DefCircleDistancOutdoorSwimmg;  //������ӾĬ�ϼ�Ȧ����

	
	uint16_t   DefHintTimeSwimming;           //��ӾĬ��Ŀ��ʱ�� ��λ:��
    uint16_t   DefHintTimeWalking;            //����Ĭ��Ŀ����ʱ��
	uint16_t   DefHintTimeOutdoorSwimmg;      //������ӾĬ��Ŀ��ʱ��

	
	uint16_t   DefWalkingFatBurnGoal;         //����ȼ֬Ŀ��Ĭ��ֵ ��λ����
	uint16_t   DefHikeFatBurnGoal;            //ͽ��ԽҰȼ֬Ŀ��Ĭ��ֵ 
	uint16_t   DefHintClimbingHeight;         //��ɽ�߶�����Ĭ��ֵ ��λ����
	uint16_t   WheelDiameter;                 //����ֱ�� ��λ:�� X10

/*����*/


 uint16_t   DefHintTimeRun;    //�ܲ�Ĭ��Ŀ��ʱ��
 uint16_t   DefHintTimeClimbing; //��ɽĬ��Ŀ��ʱ��
 uint16_t   DefHintTimeCycling; //����Ĭ��Ŀ��ʱ��
 uint16_t   DefHintTimeHike; //ͽ��ԽҰĬ��Ŀ��ʱ��
 uint16_t   DefHintTimeCountryRace; //ͽ��ԽҰĬ��Ŀ��ʱ��
 
 uint16_t	DefHintSpeedHike;		   //ͽ��ԽҰ��������Ĭ��ֵ


 uint16_t	DefCircleDistancSwimmg;	//��ӾĬ�ϼ�Ȧ����
 uint16_t	DefHintDistanceHike;    //ͽ��ԽҰ��������Ĭ��ֵ
 uint16_t	DefHintDistanceSwimmg;	   //��Ӿ��������Ĭ��ֵֵ

   

 
	uint8_t    DefHintCircleSwimming;         //��ӾĿ��Ȧ��Ĭ��ֵ
	uint8_t	   Reserved;
}SportPersonalSetStr;

//��ǰ��ַ
typedef struct  __attribute__((packed))
{
	uint16_t		Index;							//���洢��ÿ��������������
	uint16_t		IndexActivity;	   		        //���洢�Ļ���
	uint32_t		AddressStep;					//���洢�ļƲ����ݵĵ�ַ
	uint32_t		AddressHeartRate;   	        //���洢�������ݵĵ�ַ
	uint32_t		AddressPressure;    	        //���洢��ѹ���ݵĵ�ַ��//������Ŀʵ�����ڹ����ơ���Ӿ�˵�
	uint32_t		AddressDistance;		        //���洢�������ݵĵ�ַ
	uint32_t		AddressEnergy;					//���洢�������ݵĵ�ַ
	uint32_t		AddressSpeed;					//���洢�ٶ����ݵĵ�ַ
	uint32_t		AddressGps;						//���洢GPS���ݵĵ�ַ
	uint32_t		AddressDistancePoint;			//���洢������ĵ�ַ
	uint32_t		AddressCircleTime;			    //���洢��Ȧʱ��ĵ�ַ
	uint32_t    AddressPauseTime;          //�����ͣ��Ȧʱ��ĵ�ַ
	uint32_t    AddressGpsTrack;           //�����켣�ĵ�ַ
	uint32_t    AddressStatelite;          //��������ĵ�ַ
	uint32_t    AddressTrainplan;          //���ѵ���ƻ��ĵ�ַ
	uint32_t    AddressTrainplanUpload;    //���ѵ���ƻ�����ϴ��ĵ�ַ
	#ifdef COD 
	uint32_t   		Current_cod_id;				//���˼�¼id
	
	#else
	uint32_t		AddressVerticalSize;			//���洢��ֱ���ȵĵ�ַ
	#endif
	uint32_t		AddressTouchDown;			    //���洢����ʱ��ĵ�ַ
	uint32_t		AddressPause;			        //���洢��ͣʱ��ĵ�ַ��ʵ�����ڴ洢�����������
	ScreenState_t	Screen_Sta;						//���һ��ʹ�õļ�⹦�ܽ���
	uint8_t			PowerOffHour;					//�ػ�ʱ��Сʱ��
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
//���˳��洢����Ļ����
typedef struct __attribute__((packed))
{	
	uint8_t		ActivityNum;					//����� ÿ�λ�洢ʱ��һ
	uint8_t		ActivityType;                   //����� ���� �ܲ� ��ɽ ��Ӿ��						  
}TrainRecordIndexStr;
//���� �˶�ʹ�ô���
typedef struct
{
	uint32_t NumTool;//����ʹ�õĴ���
	uint32_t NumSport;//�˶�ʹ�õĴ���
}ToolSportRecordNum;
//һ������  ���ֵ���ܳ���7
typedef  enum
{
	Sun = 0U,//������
	Mon,
	Tues,
	Wed,
	Thur,
	Fri,
	Sat,//������
}Index_Week;
//���� �˶���ʹ�����Ӧ��������� ���ֵ���ܳ���32
typedef enum
{
	Location = 0U, 	//λ��
	Compass,		//ָ����
	Calendar,       //��������
	ambient,        //�������
	StopWatch,      //���
	FindPhone      	//Ѱ���ֻ�
}Index_UseItem;

typedef enum
{
	Run = 0U,    //�ܲ�
	Marathon,   //������
	CountryRun, //ԽҰ��
	IndoorRun,  //������
	Walking,    //����
	Climbing,   //��ɽ
	Cycling,    //����
	Hiking, 		//ͽ��
	Swimming,   //��Ӿ	
	Triathlon,  //��������
}Index_UseSport;

typedef struct __attribute__((packed))
{
	uint32_t	CommonSteps;			//�ճ�����
	uint32_t	CommonEnergy;			//�ճ���·��,��λΪ��
	uint32_t	CommonDistance;		//�ճ����룬��λΪcm
	uint32_t	SportSteps;			//�˶�����
	uint32_t	SportEnergy;			//�˶���·��,��λΪ��
	uint32_t	SportDistance;		//�˶����룬��λΪcm
	uint8_t   BasedHeartrate;  //��������
	uint16_t  VO2MaxMeasuring;    //�ⶨ��������� ��λ��ml/kg/min
	uint16_t  LTSpeedMeasuring;    //�ⶨ�������ٶ�
	uint8_t   LTBmpMeasuring;     //�ⶨ����������ֵ 
} PowerOffMianDataStr; //ÿ�������ݽṹ��

//���������˶��� �˶���ͬ����������ʱ�жϴ���
typedef struct __attribute__((packed))
{
	uint8_t        IsTriathlonSport;//�Ƿ������������˶��� 1���� 0������
	uint8_t        IndexTriathlon;//��¼�����˶�ʱ
	uint8_t        year;//�����˶�ʱ ��
	uint8_t        month;//�����˶�ʱ ��
	uint8_t        day;//�����˶�ʱ ��
	uint8_t        hour;
	uint8_t        minute;
	uint8_t        second;
}TriathlonSportMode;

//������������
typedef struct  __attribute__((packed))
{
	uint32_t      crc;					   //ʹ�ñ�־ֵ
	AppSetStr     AppSet;					//���app�������ݸ�ʽ
	AppGetStr     AppGet;					//���app��ȡ���ݸ�ʽ
	SportPersonalSetStr SportSet;        //����˶������������ݸ�ʽ
	CurrentAddressStr   CurrentAddress;  //��ǰ��ַ
	ScreenState_t	ShortcutScreenUP;			  //�����ϼ��Ŀ�ݶ�Ӧ����
	ScreenState_t	ShortcutScreenDown;			//�����¼��Ŀ�ݶ�Ӧ����
	ScreenState_t 	ShortcutScreenLight;		//����������Ŀ�ݶ�Ӧ����
	unsigned char 	Language;            		//����
	Alarm_t         alarm_instance[4];      //4��ALARM����ʵ��
	uint32_t        backlight_timeout_mssec; //��������  10s/30s/60s
	uint8_t       	SwVibration;         		//�񶯿���     0�� 1��
	uint8_t       	SwBuzzer;            		//����������   0�� 1��
	uint8_t       	SwRealtimeHdr;       		//ʵʱ���ʿ��� 0�� 1��
	uint8_t       	SwAutoLockScreen;    		//�Զ��������� 0�� 1��
	uint8_t       	SwBle;               		//��������     0�� 1�� 

	uint8_t       	SwRaiseForLightScreen;		//̧���������� 0�� 1��
	uint8_t       	SwNoDisturb;         		//����ģʽ���� 0�� 1��
	uint8_t       	NoDisturbStartHour;  		//����ģʽ��ʼСʱ
	uint8_t       	NoDisturbStartMinute;		//����ģʽ��ʼ����
	uint8_t       	NoDisturbEndHour;    		//����ģʽ����Сʱ
	uint8_t       	NoDisturbEndMinute;  		//����ģʽ��������

	uint8_t       	SwBasedMonitorHdr;   		//�������ʼ�⿪��
	uint8_t       	DefBasedMonitorHdrHour;     //�������ʼ��Ĭ��Сʱ
	uint8_t       	DefBasedMonitorHdrMinute;   //�������ʼ��Ĭ�Ϸ���
	uint8_t       	Theme;                      //����
	float			Compass_Offset_x;			//ָ����XУ׼ֵ
	float			Compass_Offset_y;			//ָ����YУ׼ֵ
	float			Compass_Offset_z;			//ָ����ZУ׼ֵ
	float			Altitude_Offset;			//�߶�У׼ֵ
	

	//�Զ��幤�� bit����˳��
	union UCustomTool 
	{ 
		uint32_t val;
		struct __attribute__((packed))
		{			
			//uint32_t step:1;       //����
			//uint32_t calories:1;   //��·��
			//uint32_t heartrate:1;  //����
			//uint32_t motions_record:1; //�˶���¼
			//uint32_t notification:1;      //��Ϣ֪ͨ
			//uint32_t Calendar:1;       //��������
			
   		    uint32_t recover:1;//�ָ�ʱ��
			uint32_t vo2max:1;//���������
			uint32_t threshold:1;//������
			uint32_t Countdown:1;		//����ʱ
			uint32_t StopWatch:1;      //���
        	uint32_t Location:1;       //λ��
			uint32_t Compass:1;        //ָ����
			uint32_t ambient:1;        //�������
		}bits;
	} __attribute__((packed))U_CustomTool;
	//�Զ����˶� bit����˳��
	union UCustomSport
	{ 
		uint32_t val;
		struct  __attribute__((packed))
		{
		   uint32_t	Run:1;        //�ܲ�
		   uint32_t Cycling:1;    //����
	       uint32_t Climbing:1;   //��ɽ
	       uint32_t Swimming:1;   //��Ӿ	
	       uint32_t Hiking:1;//ͽ��ԽҰ
	       uint32_t CountryRun:1; //ԽҰ��
	     //  uint32_t Marathon:1;   //������
	     //  uint32_t IndoorRun:1;  //������
	      // uint32_t Walking:1;    //����
	     //  uint32_t Triathlon:1;  //��������

		}bits;
	}__attribute__((packed))U_CustomSport;
	//�Զ������ bit����˳��
	union UCustomMonitor 
	{
		uint32_t val;
		struct __attribute__((packed))
		{
		    uint32_t step:1;       //����
			uint32_t calories:1;   //��·��
			uint32_t heartrate:1;  //����
			uint32_t motions_record:1; //�˶���¼
			uint32_t notification:1;      //��Ϣ֪ͨ
			uint32_t Calendar:1;       //��������
	
		}bits;
	}__attribute__((packed))U_CustomMonitor;
	
	TrainRecordIndexStr		TrainRecordIndex[7];		//7��Ļ��¼�洢  ������Ŷ�Ӧ��Index_Week
	uint32_t    			ToolRecordNumIndex[32];		//����ʹ�ô��� ������Ŷ�Ӧ��Index_UseTool
	uint32_t    			SportRecordNumIndex[32];    //�˶�ʹ�ô��� ������Ŷ�Ӧ��Index_UseSport
	uint8_t 				AutoLockScreenFlag;			//�Զ�������־ 0 ������ 1������
	uint8_t         IsAllowBaseHDR;          //�Ƿ���������������ѱ���ȹ���  1���� 0������
	uint8_t             TrainPlanHintFlag;    //ѵ���ƻ��Ƿ�����ʱ���־ 0����Ҫ����  1��Ҫ����
	uint8_t   					TrainPlanFlag;				//ѵ���ƻ��Ƿ�����־  0 δ���� 1������app�·���Ϣ���ֱ��ʱ��״̬
	uint8_t             TrainPlanNotifyFlag;  //ѵ���ƻ�֪ͨ���ѿ��� ��app�Ͽ������ѹ������Ӧ
	int32_t        		TimeCalibrationValue;       //����ʱ��У׼����
	rtc_time_t				PowerOffTime;				//�ػ�ʱ��
	PowerOffMianDataStr     PowerOffMianData;           //�ػ��˶����ճ�����
	uint32_t          RecoverTime;//�ָ�ʱ��
	uint32_t          RecoverTimeTotal;//���ָ�ʱ���˶��󱣴�,�Լ���ٷֱ�ʱ��
	uint8_t           TrainPlanWriteUploadMD5Flag;//ѵ���ƻ�д�ϴ������MD5��־ 1��д0δд
	uint8_t					GPS_Gauss_Type;				//GPS��˹��������
	uint8_t         PowerPercentRecord;        //�����ٷֱȣ���ֹ������׼ȷ
	uint8_t         IsFirstCalibrationStep;//�Ƿ��ǵ�һ�μƲ�У׼  �ָ��������ú����Ϊ��һ��,����ͼ����˸,У׼�����ͼ�겻��˸ 1�� 0����
	uint8_t         IsFirstSport;//�Ƿ��ǵ�һ���˶�,Ϊ�����������˶���¼����,��ʾ��ͬ����ʾ��    1�� 0����
	TriathlonSportMode TriathlonSport;//�Ƿ��������������˶�
	
	
	uint8_t     IsNeedClearTrainPlanDayHint;//�Ƿ���Ҫ���ѵ�������ѽ���  ��������ѽ������0����Ҫ����˳���������
#if  defined WATCH_COM_SHORTCUNT_UP
	uint8_t     IsMeasuredVO2max;     //����������Ƿ������  0��δ���� 1������Ч 2������Ч
	uint8_t     IsMeasuredLthreshold;     //����������Ƿ������ 0��δ���� 1������Ч 2������Ч
	uint32_t    custom_shortcut_up_switch_val;//����ϼ������л�
#endif
	uint8_t     RunData1;    	//�ܲ��Զ���������ʾ��һ�� 
	uint8_t     RunData2;    	//�ܲ��Զ���������ʾ�ڶ��� 
	uint8_t    	RunData3;		//�ܲ��Զ���������ʾ������ 
	uint8_t    	RunData4;		//�ܲ��Զ���������ʾ������ 

	uint8_t     WlakData1;    	//�����Զ���������ʾ��һ�� 
	uint8_t     WlakData2;    	//�����Զ���������ʾ�ڶ��� 
	uint8_t    	WlakData3;		//�����Զ���������ʾ������ 

	uint8_t     MarathonData1;    	//�������Զ���������ʾ��һ�� 
	uint8_t     MarathonData2;    	//�������Զ���������ʾ�ڶ��� 
	uint8_t    	MarathonData3;		//�������Զ���������ʾ������ 

	uint8_t     IndoorRunData1;    	//�������Զ���������ʾ��һ�� 
	uint8_t     IndoorRunData2;    	//�������Զ���������ʾ�ڶ��� 
	uint8_t    	IndoorRunData3;		//�������Զ���������ʾ������ 

	uint8_t     SwimRunData1;    	//��Ӿ�Զ���������ʾ��һ�� 
	uint8_t     SwimRunData2;    	//��Ӿ�Զ���������ʾ�ڶ��� 
	uint8_t    	SwimRunData3;		//��Ӿ�Զ���������ʾ������ 
    uint8_t    	SwimRunData4;       //��Ӿ�Զ���������ʾ������

	
	uint8_t     CyclingData1;    	//�����Զ���������ʾ��һ�� 
	uint8_t     CyclingData2;    	//�����Զ���������ʾ�ڶ��� 
	uint8_t    	CyclingData3;		//�����Զ���������ʾ������ 
	uint8_t    	CyclingData4;		//�����Զ���������ʾ������ 



    uint8_t     ClimbingData1;    	//��ɽ�Զ���������ʾ��һ�� 
	uint8_t     ClimbingData2;    	//��ɽ�Զ���������ʾ�ڶ��� 
	uint8_t    	ClimbingData3;		//��ɽ�Զ���������ʾ������ 
	uint8_t    	ClimbingData4;		//��ɽ�Զ���������ʾ������ 


	uint8_t     HikeData1;    	//ͽ��ԽҰ�Զ���������ʾ��һ�� 
	uint8_t     HikeData2;    	//ͽ��ԽҰ�Զ���������ʾ�ڶ��� 
	uint8_t    	HikeData3;		//ͽ��ԽҰ�Զ���������ʾ������ 
	uint8_t    	HikeData4;		//ͽ��ԽҰ�Զ���������ʾ������ 

   uint8_t      CrosscountryData1;    	//ԽҰ���Զ���������ʾ��һ�� 
	uint8_t     CrosscountryData2;    	//ԽҰ���Զ���������ʾ�ڶ��� 
	uint8_t    	CrosscountryData3;		//ԽҰ���Զ���������ʾ������ 
	uint8_t    	CrosscountryData4;		//ԽҰ���Զ���������ʾ������ 

	
	
	uint8_t 	IndoorLengSet;		//�ֱ�������ܲ����Ƿ��ֶ�����
	uint8_t 	HeartRateSet;		//�ֱ�����������Ƿ��ֶ�����




	uint8_t 		AutoLightStartHour; 		//̧��������ʼСʱ
	uint8_t 		AutoLightStartMinute;		//̧��������ʼ����
	uint8_t 		AutoLightEndHour;			//̧����������Сʱ
	uint8_t 		AutoLightEndMinute; 		//̧��������������

	uint8_t    HRRecoverySet;      //���ʻָ������ÿ���
	#if defined WATCH_TIMEZONE_SET
	uint16_t      TimeZoneRecord;//���õ�����ʱ��ʱ��
		  
	#endif
	#ifdef COD
	rtc_time_t  agps_update_time; //����agps����Ҫ���´�ʱ��
	uint8_t     phone_call_delay;//�豸���������ӳ�
	uint8_t     rt_today_summary;//ʵʱ���첽�������룬��·�����ݴ����־
	uint8_t     rt_heart;		//���ʣ�Ѫѹ��Ѫ�����ݱ�־��û�вɼ�����ĳ������0��
	uint32_t    run_step;		//�ܲ�����
	uint32_t    walk_step;		//���߲���
	uint32_t   all_day_step_address;//ȫ��Ʋ���ַƫ��
	uint8_t     rest_heart;//��������
	uint32_t    atuo_factory_reset; //���Խ׶�ͨ���仯�ṹ��С���Զ���λ����ʽ��ȥ��

        #ifdef WATCH_SONY_GNSS 
         rtc_time_t  agps_effective_time; //����agps����Ҫ���´�ʱ��
         uint8_t   agps_update_flag; //AGPS���±�־
        #endif
	
	#endif

}SetValueStr;




typedef struct __attribute__((packed))
{
	uint8_t		Date;
	uint8_t		Month;
	uint8_t		Year;
	uint8_t		Index;			//�����ţ�����ǰ�����ݴ洢��7�����ݿռ�ĵڼ����ռ�  0-6	
	uint32_t	CommonSteps;			//�ճ�����
	uint32_t	CommonEnergy;			//�ճ���·��,��λΪ��
	uint32_t	CommonDistance;		//�ճ����룬��λΪcm
	uint32_t	SportSteps;			//�˶�����
	uint32_t	SportEnergy;			//�˶���·��,��λΪ��
	uint32_t	SportDistance;		//�˶����룬��λΪcm
	uint8_t   BasedHeartrate;  //��������
	uint16_t	Temprature;		//�¶�ֵ����λΪ0.1���϶�
	uint32_t 	Pressure;		//��ѹ����λPa
	uint16_t    VO2MaxMeasuring;    //�ⶨ��������� ��λ��ml/kg/min
	uint16_t    LTSpeedMeasuring;    //�ⶨ�������ٶ� s/km
	uint8_t     LTBmpMeasuring;     //�ⶨ����������ֵ 
	uint8_t  Reserved[12];//Ԥ��
} DayMainDataStr; //ÿ�������ݽṹ��



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
	NORTH_WIND = 1, //����
	NORTHEAST_WIND, //������
	EAST_WIND,      //����
	SOUTHEAST_WIND, //���Ϸ�
	SOUTH_WIND,     //�Ϸ�
	SOUTHWEST_WIND, //���Ϸ�
	WEST_WIND,      //����
	NORTHWEST_WIND, //������
	
	UNPREDICATE_WIND = 0xFE, //�޳�������
}WindDirection;
typedef struct __attribute__ ((packed))
{
	uint8_t date[3];   //�������� ��һ���ֽ������2018Ϊ18���ڶ����ֽ��£��������ֽ�����
	uint8_t sun_rise_time[2]; //�ճ�ʱ�䣬��һ���ֽ�Сʱ���ڶ����ֽڷ���
	uint8_t sun_set_time[2]; //����ʱ�䣬��һ���ֽ�Сʱ���ڶ����ֽڷ���	
	int32_t pressure;//����ѹ
	int8_t highest_temp;//����¶�
	int8_t lowest_temp;//����¶�
	CloudType cloud; //�������
	WindScale wind_scale;//�缶��
	WindDirection wind_direction; 
} AppWeatherSyncDaily; //APP����ÿ�����ݽṹ��

typedef struct __attribute__ ((packed))
{
	uint32_t crc;//CRC�����������am_bootloader_fast_crc32�ӿ���һ�£���������˵��û����������
	uint8_t city[10];//��������������UNICODE���5�����֣�Ӣ����ASCII�������10��Ӣ��
	uint16_t pm_2_5;  //PM2.5
	AirQuality air_quality; //��������
	int8_t current_temp; //���϶��¶ȣ�����128�ȵ�����128�ȣ�ƽ���¶�
	uint8_t day_count; //APP��������������������7��ʾ7�죬��7���������ݴ����ⲿFLASH��������ѯ
	AppWeatherSyncDaily weather_array[7]; //Ŀǰ���������7�����������ⲿFLASH
} AppWeatherSync; //APP�������ݽṹ��
/** * �������� */typedef enum
	{    
	WEATHER_TYPE_SUNNY = 0,         //��   
	WEATHER_TYPE_SUNNY_NIGHT,       //��-ҹ��  
	WEATHER_TYPE_CLOUDY,            //����    
	WEATHER_TYPE_CLOUDY_NIGHT,      //����-ҹ��   
	WEATHER_TYPE_OVERCAST,          //��    
	WEATHER_TYPE_HEAVY_RAIN,        //���� 
	WEATHER_TYPE_MODERATE_RAIN,     //���� 
	WEATHER_TYPE_LIGHT_RAIN,        //С��    
	WEATHER_TYPE_TORRENTIAL_RAIN,   //����   
	WEATHER_TYPE_SNOW,              //ѩ    
	WEATHER_TYPE_DUST,              //��   
	WEATHER_TYPE_FOGGY,             //��  
	WEATHER_TYPE_HAZE,              //��   
	WEATHER_TYPE_WINDY,             //��   
	WEATHER_TYPE_UNKNOWN          //δ֪
} cod_weather_type;



typedef struct
{
	uint16_t PressureNum;
	uint16_t PressureSum;
	uint16_t AltitudeNum;
	uint16_t AltitudeSum;
	uint16_t PressureData[AMBIENT_DATA_MAX_NUM];	//��λ: Pa
	int16_t AltitudeData[AMBIENT_DATA_MAX_NUM];		//��λ: m
}AmbientDataStr;

extern uint32_t StartSteps;
extern uint8_t last_basedheartrate;//ǰһ���������ֵ
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






































