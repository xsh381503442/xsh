#ifndef _TASK_DISPLAY_H__
#define _TASK_DISPLAY_H__

#include <stdint.h>
#include "app_config.h"
#include "watch_config.h"

// Defines for the possible messages to be sent
#define MSG_DEFAULT						0	//do nothing
#define MSG_DISPLAY_SCREEN				1	//��ʾ����
#define	MSG_UPDATE_TIME					2	//ÿ���Ӹ���ʱ��
#define MSG_START_TOOL              	3	//����������

#define MSG_UPDATE_COMPASS_VALUE        6   //���½Ƕ�
#define	MSG_UPDATE_COMPASS_CALIBRATION	7	//����ָ����У׼����
#define MSG_UPDATE_AMBIENT_VALUE		8	//������ѹ����ֵ
#define	MSG_UPDATE_GPS_VALUE			9	//����λ�ý���ֵ
#define	MSG_UPDATE_STEP_VALUE			10	//���¼Ʋ�����ֵ
#define MSG_UPDATE_HDR_VALUE     		11	//�������ʽ�������ֵ
#define	MSG_UPDATE_STOPWATCH_TIME		12	//��������ʱʱ��
#define	MSG_UPDATE_STOPWATCH_RECORD		13	//��������¼

#define MSG_UPDATE_COUNTDOWN      14//����ʱ���½���
#define MSG_UPDATE_VO2MAX_VALUE    15//���������12���ӵ���ʱ�������
#define	MSG_UPDATE_STEP_CALIBRETION		16	//���²���У׼���沽����̵�����
#define MSG_UPDATE_VO2MAX_LACTATE_THRESHOLD  17  //��������һ�� ����������������в�������
#define MSG_UPDATE_PACE_TRAINPLAN_SPORT  18 //�˶�������ѵ���ƻ��˶�����������
#define MSG_UPDATE_INTERVAL_TRAINPLAN_SPORT  19//ѵ���ƻ��˶��м�Ъѵ�������� ��4x400
#define MSG_UPDATE_TRAINPLAN_SW_HINT  20//ѵ���ƻ�����
#define MSG_UPDATE_REALTIME_CALIBRATION   21 //�Լ����ʵʱʱ����ʾ����
#define MSG_UPDATE_ICON_CALIBRTIONSTEP    22//��̲�������У׼ͼ����˸����
#if defined(HARDWARE_TEST)
#define MSG_UPDATE_TEST_GPS      	92  //GPS���Ը���
#define	MSG_UPDATE_TEST_STEP     	93
#define	MSG_UPDATE_TEST_COMPASS  	94
#define MSG_UPDATE_TEST_HEARTRATE 	95 	//���ʲ��Ը���
#define MSG_UPDATE_TEST_FLASH    	96
#define	MSG_UPDATE_TEST_PRESSURE 	97
#define MSG_UPDATE_TEST_MOTOR	   	98
#define MSG_UPDATE_TEST_COMPASS_CALIBRATION 99

#endif
#define MSG_UPDATE_TIME_CALIBRATION   100
typedef struct{
	uint32_t cmd;
	uint32_t value;
}DISPLAY_MSG;

typedef enum
{
	DISPLAY_SCREEN_LOGO = 0,  	//��������

	DISPLAY_SCREEN_HOME,		//������
	DISPLAY_SCREEN_WEEK_AVERAGE_HDR, //��ȥһ��ƽ���������ʽ���
	DISPLAY_SCREEN_STEP,             //�Ʋ�����
	DISPLAY_SCREEN_VO2MAX,    //���������OK���������
	DISPLAY_SCREEN_MOTIONS_RECORD,   //�˶���¼���� //5
	
	DISPLAY_SCREEN_NOTIFICATION,     //֪ͨ��Ϣ���ͽ���
	DISPLAY_SCREEN_CALORIES,         //��·����ֵ��ʾ���� �˶�/����
	DISPLAY_SCREEN_LACTATE_THRESHOLD,//��������ֵ��ʾ���� bmp | min/km
	DISPLAY_SCREEN_TRAINPLAN_MARATHON,//ѵ���ƻ�֮��������ʾ���� ״̬:ǰһ��/����/��һ��
	DISPLAY_SCREEN_RECOVERY,          //�ָ�ʱ�䵹��ʱ���� h/min  //10
	
	DISPLAY_SCREEN_LARGE_LOCK,   //������ʾ����
	DISPLAY_SCREEN_CUSTOM_ADD_SUB_MAIN,//�Զ�����������������
	DISPLAY_SCREEN_CUSTOM_ADD_SUB,//�Զ��������ӽ���
	DISPLAY_SCREEN_TIME_IS_MONITOR_HDR_EXPLAIN,//�������ʱ�䵽�������
	
	DISPLAY_SCREEN_HEARTRATE_HINT,//���ʲ������ѽ���			//15
	DISPLAY_SCREEN_HEARTRATE,   //���ʲ�������
	DISPLAY_SCREEN_HEARTRATE_STABLE,//�ȶ����ʽ���
	
	DISPLAY_SCREEN_STEP_WEEK_STEPS,//�Ʋ��ܼƲ�������
	DISPLAY_SCREEN_STEP_WEEK_MILEAGE,//�Ʋ�����̽���
	DISPLAY_SCREEN_STEP_CALIBRATION_HINT,//�Ʋ�У׼��ʾ����		//20
	DISPLAY_SCREEN_STEP_CALIBRATION_SHOW,//�Ʋ�У׼����ʾ����
	DISPLAY_SCREEN_STEP_CALIBRATION_CONFIRM_SAVE,//�Ʋ�У׼ȷ���Ƿ񱣴����
	
	DISPLAY_SCREEN_VO2MAX_HINT,       //�������������ǰ���ѽ���
	DISPLAY_SCREEN_CUTDOWN,           //����ʱ����
	DISPLAY_SCREEN_VO2MAX_MEASURING,  //�����������������			//25
	DISPLAY_SCREEN_END_UNFINISH_SPORT,//Ҫ�����δ����˶����ѽ��� �Ժ��������������������ģʽʱ��������
  

	
	DISPLAY_SCREEN_CALORIES_WEEK_SPORTS,//�˶���·����ͳ�ƽ���
	DISPLAY_SCREEN_LACTATE_THRESHOLD_HINT,//�����в���ǰ���ѽ���
	DISPLAY_SCREEN_LACTATE_THRESHOLD_MEASURING,//�����в��Խ���
	DISPLAY_SCREEN_TIME_DONE,//����ʱ����ɲ������ѽ���
	DISPLAY_SCREEN_SPORTS_RECORD_TRACK_DETAIL,//�˶���¼���˶��켣��ϸ����
	

	DISPLAY_SCREEN_TOOL,		//����ѡ�����				//30
	DISPLAY_SCREEN_GPS,        	//��λ(��γ��)
	DISPLAY_SCREEN_COMPASS,    	//ָ����
	DISPLAY_SCREEN_CALENDAR,   	//��������
	DISPLAY_SCREEN_AMBIENT,    	//�������(������ѹ)				
	DISPLAY_SCREEN_STOPWATCH,  	//����ʱ				//35
	DISPLAY_SCREEN_COUNTDOWN,   //����ʱ����
	DISPLAY_SCREEN_FINDPHONE,  	//Ѱ���ֻ�

	
	DISPLAY_SCREEN_COMPASS_CALIBRATION, //�ش�У׼���棬����ӽ��濪ʼ�������������ӽ����ڴ˺�(���޸�)
	
	DISPLAY_SCREEN_AMBIENT_MENU,		//�������˵�����
	DISPLAY_SCREEN_AMBIENT_PRESSURE,	//���������ѹ����
	DISPLAY_SCREEN_AMBIENT_ALTITUDE,	//�������߶Ƚ���		//40
	DISPLAY_SCREEN_AMBIENT_ALTITUDE_CALIBRATION,	//�������߶�У׼����

#if defined (WATCH_TOOL_COUNTDOWN)
	DISPLAY_SCREEN_FINDPHONE_START,		//��ʼѰ���ֻ�

	DISPLAY_SCREEN_COUNTDOWN_SETTING,	//����ʱ����
	DISPLAY_SCREEN_COUNTDOWN_MENU,		//����ʱ�˵�
	DISPLAY_SCREEN_COUNTDOWN_TIME,		//����ʱʱ��
	DISPLAY_SCREEN_COUNTDOWN_EXIT,		//����ʱ�˳�
	DISPLAY_SCREEN_COUNTDOWN_FINISH,	//����ʱ����
#endif
	DISPLAY_SCREEN_STOPWATCH_MENU,		//���˵�����
	DISPLAY_SCREEN_STOPWATCH_RECORD,	//����¼����				
	
	DISPLAY_SCREEN_GPS_DETAIL,			//GPS���ý���
	DISPLAY_SCREEN_PROJECTION_ZONING,   //ͶӰ�ִ�����
	DISPLAY_SCREEN_CLOUD_NAVIGATION,    //�Ƽ���������
	DISPLAY_SCREEN_MOTION_TRAIL,        //�˶��켣����
	DISPLAY_SCREEN_MOTION_TRAIL_DETAIL, //�˶��켣��ϸ���ݽ���
	DISPLAY_SCREEN_VIEW_MOTION_TRAIL,	//�鿴�˶��켣����
	DISPLAY_SCREEN_NAVIGATION_OPS,      //�Ƽ�����ѡ�����
	DISPLAY_SCREEN_CLOUD_NAVIGATION_TRAIL,  //�Ƽ������鿴�켣����
	DISPLAY_SCREEN_CLOUD_NAVIGATION_DEL,    //�Ƽ�����ɾ������
	DISPLAY_SCREEN_START_NAVIGATION_READY,  //��ʼ����׼������
	DISPLAY_SCREEN_GPS_TRAIL_REMIND,    //�����˶��Ժ����ʱ����켣����ʾ����
	DISPLAY_SCREEN_TRACK_LOAD,              //�켣���ؽ���
	
	DISPLAY_SCREEN_WEATHER_INFO, //������Ϣ����
	
	DISPLAY_SCREEN_APP_MSG_INFO, //������Ϣ�˵�����
	
    DISPLAY_SCREEN_SPORT,				//�˶�ѡ�����
	
	DISPLAY_SCREEN_RUN,			        //�ܲ�
	DISPLAY_SCREEN_MARATHON,            //������				
	DISPLAY_SCREEN_CROSSCOUNTRY,    //ԽҰ��
	DISPLAY_SCREEN_INDOORRUN,           //������
	DISPLAY_SCREEN_WALK,        //����
	DISPLAY_SCREEN_CLIMBING,		    //��ɽ
	DISPLAY_SCREEN_CYCLING,			    //����					
	DISPLAY_SCREEN_CROSSCOUNTRY_HIKE,	//ͽ��ԽҰ
	DISPLAY_SCREEN_SWIMMING,	            //������Ӿ
	DISPLAY_SCREEN_TRIATHLON,		    //��������
	DISPLAY_SCREEN_TRIATHLON_SWIMMING,			//���������Ӿ��
	DISPLAY_SCREEN_TRIATHLON_CYCLING,			//����������У�		
	DISPLAY_SCREEN_TRIATHLON_RUNNING,			//��������ܲ���
	DISPLAY_SCREEN_TRIATHLON_FIRST_CHANGE,		//���������һ�������
	DISPLAY_SCREEN_TRIATHLON_SECOND_CHANGE,		//��������ڶ��������
	DISPLAY_SCREEN_TRIATHLON_FINISH,			//����������ɽ���
	DISPLAY_SCREEN_TRIATHLON_PAUSE,				//����������ͣ�˵�����			
	DISPLAY_SCREEN_TRIATHLON_CANCEL,			//��������ȡ���˵�����
	DISPLAY_SCREEN_TRIATHLON_DETAIL,			//����������ϸ���ݽ���
	
	
	
	DISPLAY_SCREEN_TRAINPLAN_RUN,   //ѵ���ƻ��ܲ�
	DISPLAY_SCREEN_TRAINPLAN_CYCLING,//ѵ���ƻ�����
	DISPLAY_SCREEN_TRAINPLAN_SWIMMING,//ѵ���ƻ���Ӿ
	DISPLAY_SCREEN_TRAINPLAN_HOME_RUN_HINT,//ѵ���ƻ��������浱��������ʾ
	DISPLAY_SCREEN_TRAINPLAN_SPORT_RUN_HINT,//ѵ���ƻ��˶����浱��������ʾ
	DISPLAY_SCREEN_TRAINPLAN_IS_COMPLETE,//ѵ���ƻ����������Ƿ����ȷ�Ͻ���
	DISPLAY_SCREEN_TRAINPLAN_RUN_SELECT,//ѵ���ƻ��˶�ģʽ����ʱ����ѡ����ִ��ѵ���ƻ������ܲ�����
#if defined WATCH_SIM_SPORT

#elif defined WATCH_COM_SPORT
	DISPLAY_SCREEN_RUN_PAUSE,				//�ܲ���ͣ�˵�����
	DISPLAY_SCREEN_RUN_CANCEL,				//�ܲ�ȡ���˵�����(����)
	DISPLAY_SCREEN_RUN_DETAIL,				//�ܲ���ϸ���ݽ���


      /*����*/
	DISPLAY_SCREEN_SPORT_READY, 				//�˶� ׼������
	DISPLAY_SCREEN_SPORT_DATA_DISPLAY_PREVIEW,  //�˶� ������ʾԤ������
	DISPLAY_SCREEN_SPORT_DATA_DISPLAY,			//�˶� ������ʾ����(��ʾ������)
	DISPLAY_SCREEN_SPORT_DATA_DISPLAY_SET,		//�˶� ������ʾ���ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET,			//�˶����� ���ý���
    DISPLAY_SCREEN_SPORT_REMIND_SET_HAERT,					//���ʸ澯���ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_PACE,					//�����������ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_DISTANCE,				//�����������ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_AUTOCIRCLE, 			//�Զ���Ȧ���ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_COALKCAL,				//ȼ֬Ŀ�����ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_TIME,					//ʱ���������ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_SPEED,					//�ٶ��������ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_SWIMPOOL,				//Ӿ�س������ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_GOALCIRCLE, 			//��Ȧ�������ý���
	DISPLAY_SCREEN_SPORT_REMIND_SET_INDOORLENG, 			//�����ܲ������ý���
    DISPLAY_SCREEN_SPORT_REMIND_SET_ALTITUDE, 			//(��ɽ)�߶����ý���
 

#else	
	DISPLAY_SCREEN_RUN_READY,			        //�ܲ�׼��
	DISPLAY_SCREEN_RUN_DATA,			        //�ܲ�����
	DISPLAY_SCREEN_RUN_TRACK,			        //�ܲ��켣					
	DISPLAY_SCREEN_RUN_PAUSE_DATA,        //�ܲ���ͣ���ݣ�����ͣ����ͨ����back�����أ�ע�ⲻҪ������֮�����ö�٣� 		
	DISPLAY_SCREEN_RUN_PAUSE_TRACK,       //�ܲ���ͣ�켣���ݣ�����ͣ����ͨ����back�����أ�
	DISPLAY_SCREEN_RUN_PAUSE,                   //�ܲ���ͣ
	DISPLAY_SCREEN_RUN_PAUSE_BACK_DATA_PAUSE,           //�ܲ���ͣ����������ͣ����
	DISPLAY_SCREEN_RUN_PAUSE_BACK_TRACK_PAUSE,           //�ܲ���ͣ���ع켣��ͣ����
		
	DISPLAY_SCREEN_RUN_DATA_TRACKBACK,		//ѭ���������ݣ�ע�ⲻҪ������֮�����ö�٣�
	DISPLAY_SCREEN_RUN_TRACK_TRACKBACK,		//ѭ�������켣				
	DISPLAY_SCREEN_RUN_TRACKBACK_PAUSE,		//�ܲ�ѭ��������ͣ
#endif
	//Jason_V1.00:Addin Swimming Interface Display Strings in the following section:

#if defined WATCH_SIM_SPORT
	DISPLAY_SCREEN_SWIM_PAUSE,				//��Ӿ��ͣ�˵�����
	DISPLAY_SCREEN_SWIM_CANCEL, 			//��Ӿȡ���˵�����
	DISPLAY_SCREEN_SWIM_DETAIL, 			//��Ӿ��ϸ���ݽ���
#elif defined WATCH_COM_SPORT
	
    DISPLAY_SCREEN_SWIM_PAUSE,				//��Ӿ��ͣ�˵�����
	DISPLAY_SCREEN_SWIM_CANCEL, 			//��Ӿȡ���˵�����
	DISPLAY_SCREEN_SWIM_DETAIL, 			//��Ӿ��ϸ���ݽ���
#else
	DISPLAY_SCREEN_SWIM_READY,			        //��Ӿ׼��
	DISPLAY_SCREEN_SWIM_DATA,			        //��Ӿ����
	DISPLAY_SCREEN_SWIM_STATS,			        //��Ӿ����
	DISPLAY_SCREEN_SWIM_PAUSE,                  //��Ӿ��ͣ			
	DISPLAY_SCREEN_SWIM_POOL_SET,				//��Ӿ���ȸ���					
	DISPLAY_SCREEN_SWIM_DATA_PAUSE,			//��Ӿ������ͣ����
	DISPLAY_SCREEN_SWIM_STATS_PAUSE,			//��Ӿ������ͣ����
	
	DISPLAY_SCREEN_SWIM_SAVE_DETIAL_1,           //�˶���ϸ����1
	DISPLAY_SCREEN_SWIM_SAVE_DETIAL_2,           //�˶���ϸ����2
	DISPLAY_SCREEN_SWIM_SAVE_HEARTRATEZONE,       //�˶���ϸ�����������
#endif
	//Jason_V1.10:Addin CrossCountryRunning Interface Display Strings in the following section:
#if defined WATCH_COM_SPORT
	DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE,				//ԽҰ����ͣ�˵�����
	DISPLAY_SCREEN_CROSSCOUNTRY_CANCEL,				//ԽҰ��ȡ���˵�����
	DISPLAY_SCREEN_CROSSCOUNTRY_DETAIL,				//ԽҰ����ϸ���ݽ���
#else
	DISPLAY_SCREEN_CROSSCOUNTRY_READY,		        //ԽҰ��׼�� ԽҰ��
	DISPLAY_SCREEN_CROSSCOUNTRY_TRACK,			        //ԽҰ�ܹ켣
	DISPLAY_SCREEN_CROSSCOUNTRY_DATA,			        //ԽҰ������				
	DISPLAY_SCREEN_CROSSCOUNTRY_PAUSE,                   //ԽҰ����ͣ
	DISPLAY_SCREEN_CROSSCOUNTRY_TRACK_PAUSE,           //ԽҰ����ͣ���ؽ���
	DISPLAY_SCREEN_CROSSCOUNTRY_DATA_PAUSE,           //ԽҰ����ͣ���ؽ���
	
	DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_1,       //ԽҰ����ϸ����1
	DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_2,			 //ԽҰ����ϸ����2
	DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_DETIAL_3,			 //ԽҰ����ϸ����2
	DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_TRACK,               //ԽҰ����ϸ�켣����
	DISPLAY_SCREEN_CROSSCOUNTRY_SAVE_HEARTRATEZONE,       //ԽҰ����ϸ�����������			
#endif
	//Jason's code end in here
	DISPLAY_SCREEN_SPORT_CANCEL,                //�˶�����
	DISPLAY_SCREEN_SPORT_FEEL,                  //�˶�����
	DISPLAY_SCREEN_SPORT_HRRECROVY,                  //���ʻָ�
	DISPLAY_SCREEN_SPORT_SAVING,                //�˶����ڱ���				
	DISPLAY_SCREEN_SPORT_SAVED,                 //�˶�����ɹ�
#if defined WATCH_SIM_SPORT || defined WATCH_COM_SPORT
	DISPLAY_SCREEN_REMIND_PAUSE,	//��ͣ���ѽ���

#else
	DISPLAY_SCREEN_RUN_SAVE_DETIAL_1,            //�˶���ϸ����1
	DISPLAY_SCREEN_RUN_SAVE_DETIAL_2,            //�˶���ϸ����2
	DISPLAY_SCREEN_RUN_SAVE_TRACK,               //�˶���ϸ�켣����
	DISPLAY_SCREEN_RUN_SAVE_HEARTRATEZONE,       //�˶���ϸ�����������			
#endif
	DISPLAY_SCREEN_CYCLING_PAUSE,				//������ͣ�˵�����
	DISPLAY_SCREEN_CYCLING_CANCEL,				//����ȡ���˵�����
	DISPLAY_SCREEN_CYCLING_DETAIL,				//������ϸ���ݽ���
	
	DISPLAY_SCREEN_INDOORRUN_PAUSE,				//��������ͣ�˵�����
	DISPLAY_SCREEN_INDOORRUN_CANCEL,			//������ȡ���˵�����
	DISPLAY_SCREEN_INDOORRUN_DETAIL,			//��������ϸ���ݽ���
#if defined WATCH_COM_SPORT
	//DISPLAY_SCREEN_SPORT_REMIND_SET_INDOORLENG,				//�����ܲ������ý���
#endif
#if defined WATCH_SPORT_EVENT_SCHEDULE //�������ν���
	DISPLAY_SCREEN_SPORT_EVENT_RESTART_SELECT,//�˶���ʼ�Ƿ�����Ƿ����¿�ʼѡ�����
#endif
	DISPLAY_SCREEN_WALK_PAUSE,					//������ͣ�˵�����
	DISPLAY_SCREEN_WALK_CANCEL,					//����ȡ���˵�����
	DISPLAY_SCREEN_WALK_DETAIL,					//������ϸ���ݽ���
	
	DISPLAY_SCREEN_MARATHON_PAUSE,				//��������ͣ�˵�����
	DISPLAY_SCREEN_MARATHON_CANCEL,				//������ȡ���˵�����
	DISPLAY_SCREEN_MARATHON_DETAIL,				//��������ϸ���ݽ���

	DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_PAUSE,				//ͽ��ԽҰ��ͣ�˵�����
	DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_CANCEL,				//ͽ��ԽҰȡ���˵�����		
	DISPLAY_SCREEN_CROSSCOUNTRY_HIKE_DETAIL,				//ͽ��ԽҰ��ϸ���ݽ���
#if defined WATCH_COM_SPORT
	DISPLAY_SCREEN_CLIMBING_PAUSE,				//��ɽͣ�˵�����
	DISPLAY_SCREEN_CLIMBING_CANCEL, 			//��ɽȡ���˵�����
	DISPLAY_SCREEN_CLIMBING_DETAIL, 			//��ɽ��ϸ���ݽ���
#else
	DISPLAY_SCREEN_CLIMBING_READY,			        //��ɽ׼�� 
	DISPLAY_SCREEN_CLIMBING_TRACK,			        //��ɽ�켣
	DISPLAY_SCREEN_CLIMBING_DATA,			        //��ɽ����
	DISPLAY_SCREEN_CLIMBING_PAUSE,                   //��ɽ��ͣ					
	//DISPLAY_SCREEN_CLIMBING_CANCEL,           //��ɽ��������
	//DISPLAY_SCREEN_CLIMBING_FEEL,                //��ɽ��������
	DISPLAY_SCREEN_CLIMBING_PAUSE_BACK_TRACK,           //��ɽ��ͣ����
	DISPLAY_SCREEN_CLIMBING_PAUSE_BACK_DATA,           //��ɽ��ͣ����
	DISPLAY_SCREEN_CLIMBING_PAUSE_BACK_TRACK_TARCEBACK,           //��ɽ��ͣ���ع켣��ͣ����
	DISPLAY_SCREEN_CLIMBING_PAUSE_BACK_DATA_TRACEBACK,           //��ɽ��ͣ����������ͣ����
	DISPLAY_SCREEN_CLIMBING_TRACK_TRACKBACK,		//��ɽѭ�������켣	
	DISPLAY_SCREEN_CLIMBING_DATA_TRACKBACK,		//��ɽѭ���������ݣ�ע�ⲻҪ������֮�����ö�٣�			
	DISPLAY_SCREEN_CLIMBING_TRACKBACK_PAUSE,        //��ɽѭ��������ͣ

	DISPLAY_SCREEN_CLIMBING_SAVING,              //��ɽ�����������
	DISPLAY_SCREEN_CLIMBING_SAVE_DETIAL_1,       //��ɽ��ϸ����1
	DISPLAY_SCREEN_CLIMBING_SAVE_DETIAL_2,			 //��ɽ��ϸ����2				
	DISPLAY_SCREEN_CLIMBING_SAVE_TRACK,               //��ɽ��ϸ�켣����
	DISPLAY_SCREEN_CLIMBING_SAVE_HEARTRATEZONE,       //��ɽ��ϸ�����������
#endif	
	DISPLAY_SCREEN_TEST_CALIBRATION_VIEW_STATUS,//����ʱ��У׼����״̬�鿴
	DISPLAY_SCREEN_TEST_REAL_DIFF_TIME, //ʵʱʱ�����ͳ�ƽ��� RTC��GPSʱ��ȶ�
	DISPLAY_SCREEN_MENU,                                              //���˵�
    DISPLAY_SCREEN_THEME = DISPLAY_SCREEN_MENU +1,                    //����
    DISPLAY_SCREEN_TIME,                     //ʱ��
	DISPLAY_SCREEN_SPORTS_RECORD,            //�˶���¼
	DISPLAY_SCREEN_CUSTOMIZED_TOOLS,         //�Զ��幤��
	DISPLAY_SCREEN_CUSTOMIZED_SPORTS,        //�Զ����˶�			
    DISPLAY_SCREEN_ACCESSORY,                //�������
    DISPLAY_SCREEN_SYSTEM,                   //ϵͳ
	DISPLAY_SCREEN_WATCH_INFO,               //�����ֱ�	
	DISPLAY_SCREEN_THEME_APP_MENU,			//����Ӧ�ò˵�
	DISPLAY_SCREEN_THEME_APP_CUSTOM,		//�����Զ������



	
#if defined WATCH_TIMEZONE_SET	
	DISPLAY_SCREEN_TIMEZONE_SET,                    //ʱ������	
#endif
	DISPLAY_SCREEN_UTC_GET,                    //������ʱ	
	DISPLAY_SCREEN_ALARM,                      //����
	DISPLAY_SCREEN_ALARM_SET = DISPLAY_SCREEN_ALARM + 1,                      //����ѡ��
	DISPLAY_SCREEN_ALARM_SET_TIME,                      //����ʱ������
	DISPLAY_SCREEN_ALARM_SET_SOUND_N_VIBERATE,                      //��������������
	DISPLAY_SCREEN_ALARM_SET_REPITION,                      //�����ظ�����
	#if defined WATCH_TIMEZONE_SET
	DISPLAY_SCREEN_TIMEZONE_SELECT,						//ʱ������ѡ�����
	#endif
	DISPLAY_SCREEN_SPORTS_RECORD_DETAIL,                      //���¼����	
	DISPLAY_SCREEN_SPORTS_RECORD_DELETE,                      //���¼ɾ��
	DISPLAY_SCREEN_ACCESSORY_HEARTRATE,    						 //���������������   
	DISPLAY_SCREEN_ACCESSORY_RUNNING,    						 //�ܲ������������   
	DISPLAY_SCREEN_ACCESSORY_CYCLING_WHEEL,    						 //���г����������������   		
	DISPLAY_SCREEN_ACCESSORY_CYCLING_CADENCE,    						 //���г�̤Ƶ�����������   
	DISPLAY_SCREEN_SYSTEM_BASE_HEARTRATE,    						 //��������ѡ���������   
	DISPLAY_SCREEN_SYSTEM_LANGUAGE,    						 //��Ӣ��ѡ���������  
	DISPLAY_SCREEN_SYSTEM_BACKLIGHT,    						 //����ʱ��ѡ���������    
	DISPLAY_SCREEN_SYSTEM_SOUND,    						 //������ѡ��������� 
	DISPLAY_SCREEN_SYSTEM_SILENT,                              //����ģʽ���ý�������
    DISPLAY_SCREEN_SYSTEM_SILENT_TIME,                         //����ģʽʱ�����ý���
   	DISPLAY_SCREEN_SYSTEM_AUTOLIGHT,                              //̧���������ý�������
    DISPLAY_SCREEN_SYSTEM_AUTOLIGHT_TIME,                         //̧������ʱ�����ý���
 	DISPLAY_SCREEN_NOTIFY_ALARM,          //����֪ͨ��������                        
    DISPLAY_SCREEN_SOS,					//SOS����
	DISPLAY_SCREEN_SOS_MENU,			//SOS�˵�����
#ifdef COD 
	DISPLAY_SCREEN_TRAIN_CAST,		   //ѵ��Ͷ������
    DISPLAY_SCREEN_TRAIN_CAST_PAUSE_REMIND,	//ѵ����ͣ����
    DISPLAY_VICE_SCREEN_OUTDOOR_RUN,//����������
    DISPLAY_VICE_SCREEN_OUTDOOR_WALK,//�������⽡��(ͽ��ԽҰ)
	DISPLAY_VICE_SCREEN_RIDE,//��������
	DISPLAY_VICE_SCREEN_CLIMB,//������ɽ
	DISPLAY_SCREEN_SPORT_CAST_PAUSE_REMIND, //�˶�Ͷ����ͣ�˵�����
	DISPLAY_VICE_SCREEN_SPORT_DATA,//�˶�Ͷ�����������������
	DISPLAY_SCREEN_UPDATE_BP ,      //����������ʾ����
#endif
	
	DISPLAY_SCREEN_POST_SETTINGS,//�Լ�����     
	DISPLAY_SCREEN_POST_HDR,//�����Լ�,©�����	
	DISPLAY_SCREEN_POST_HDR_SNR,//�����Լ�,����Ȳ���		
	DISPLAY_SCREEN_POST_FACTORY_RESET,//�ָ���������
	DISPLAY_SCREEN_POST_WATCH_INFO,//�Լ�������ʾ�ֱ����Ӳ����Ϣ
	DISPLAY_SCREEN_POST_GPS,   //�Լ�GPS+BD
	DISPLAY_SCREEN_POST_GPS_DETAILS,//�Լ�GPS����
#if defined(WATCH_AUTO_BACK_HOME)
	DISPLAY_SCREEN_POST_AUTO_BACK_HOME,//�Լ� ������ˢ��ҳ�泬��10�������κβ������ް������������ش������� ״̬��ʾ
#endif
	DISPLAY_SCREEN_BLE_DFU ,             //�ֻ�APP������������
	DISPLAY_SCREEN_PC_HWT ,             //PC��PCBӲ�����Խ���

	DISPLAY_SCREEN_NOTIFY ,            //֪ͨ����������ʼ
	DISPLAY_SCREEN_NOTIFY_SPORTS_RECORD_DELETE = DISPLAY_SCREEN_NOTIFY,    //ɾ�������֪ͨ��������   
	DISPLAY_SCREEN_NOTIFY_BLE_CONNECT,    //��������֪ͨ��������  
	DISPLAY_SCREEN_NOTIFY_BLE_DISCONNECT, //�����Ͽ�֪ͨ��������   
	DISPLAY_SCREEN_NOTIFY_BLE_BOND,       //������֪ͨ��������  
	DISPLAY_SCREEN_NOTIFY_BLE_PASSKEY,    //���������֪ͨ��������  
	DISPLAY_SCREEN_NOTIFY_TIME_IS_MONITOR_HDR,//�������ʱ�䵽���������
	DISPLAY_SCREEN_NOTIFY_LOW_BAT,        //�͵������ѽ�������     
	DISPLAY_SCREEN_NOTIFY_APP_MSG,        //APP������Ϣ���ѽ�������
	DISPLAY_SCREEN_NOTIFY_BAT_CHG,        //��س�����	

    DISPLAY_SCREEN_NOTIFY_HEARTRATE,      //���ʹ������ѽ���
	DISPLAY_SCREEN_NOTIFY_CALORY,		  //ÿ�տ�·�����ѽ��棨�ܹ���
	DISPLAY_SCREEN_NOTIFY_STEP,           //�������ѽ���		���ܹ���	
	
	DISPLAY_SCREEN_NOTIFY_GOALKCAL,       //Ŀ��ȼ֬���ѽ��棨�˶���
	DISPLAY_SCREEN_NOTIFY_GOALCIRCLE,     //Ŀ��Ȧ�����ѽ���			
	DISPLAY_SCREEN_NOTIFY_GOALTIME,       //Ŀ��ʱ�����ѽ���
	DISPLAY_SCREEN_NOTIFY_GOALDISTANCE,	  //�������ѽ���
	DISPLAY_SCREEN_NOTIFY_CIRCLEDISTANCE, //��Ȧ�������ѽ���
	DISPLAY_SCREEN_NOTIFY_ALTITUDE,       //�߶����ѽ���
	DISPLAY_SCREEN_NOTIFY_SPEED, 		  //�ٶ����ѽ���
	DISPLAY_SCREEN_NOTIFY_PACE,           //�������ѽ���
    DISPLAY_SCREEN_NOTIFY_LIFE_DATA,      //������������
	
	DISPLAY_SCREEN_NOTIFY_TRAINING_DAY,   //ѵ���ƻ� ѵ��������
	DISPLAY_SCREEN_NOTIFY_TRAINING_COMPLETE,   //ѵ���ƻ� ѵ��Ŀ��������
	DISPLAY_SCREEN_NOTIFY_PACE_TRAINPLAN,      //ѵ���ƻ���������
	DISPLAY_SCREEN_NOTIFY_PACE_INTER_TRAINPLAN,//ѵ���ƻ���Ъѵ��4x400+4x200���Ƶ�����
	DISPLAY_SCREEN_NOTIFY_SW_HINT,
	DISPLAY_SCREEN_NOTIFY_WARNING,        //�����ֱ����ѽ���
	DISPLAY_SCREEN_NOTIFY_VO2MAX_LACTATE_ERALY_END,//�������������������ǰ����ʱ���ѽ���
	DISPLAY_SCREEN_NOTIFY_FIND_WATCH,				//Ѱ���ֱ����
#ifdef COD 
	DISPLAY_SCREEN_TRAIN_CAST_START_REMIND,	//ѵ��������ʼ�˶�����
	DISPLAY_SCREEN_TRAIN_CAST_PRESS_REMIND,	//ѵ����ͣ��������
	
	DISPLAY_SCREEN_REMIND_VICE_PAUSE_CONTINUE,//�˶�Ͷ����ͣ���ѽ���
	DISPLAY_VICE_SCREEN_SPORT_HINT,//�˶�����������ʼ����
	DISPLAY_SCREEN_TRAIN_CAST_END_REMIND,	//ѵ����������
	DISPLAY_SCREEN_NOTIFY_ON_BIND_DEVICE,	//��ȷ�Ͻ���
	DISPLAY_SCREEN_NOTIFY_BIND_STATUS,		//��ȷ�Ͻ���
	
     DISPLAY_SCREEN_NOTIFY_LOCATION_OK,	//��λ�ɹ�����
#endif


	DISPLAY_SCREEN_TRIATHLON_CYCLING_READY,		//������������׼������
	DISPLAY_SCREEN_TRIATHLON_RUNNING_READY,		//���������ܲ�׼������
	DISPLAY_SCREEN_THEME_PREVIEW,				//����Ԥ������
	DISPLAY_SCREEN_THEME_APP_SUCCESS,			//����Ӧ�óɹ�����
	DISPLAY_SCREEN_THEME_INVALID,				//������Ч����
	
	DISPLAY_SCREEN_TIME_CALIBRATION,//����ʱ���������
	DISPLAY_SCREEN_TIME_CALIBRATION_DETAIL,//����ʱ���������������
	DISPLAY_SCREEN_TIME_CALIBRATION_REALTIME,//����ʱ��ʵʱʱ�����

   //�Զ������ٱ���һ������
    DISPLAY_SCREEN_CUSTOM_MON_HINT, 
    DISPLAY_SCREEN_CUSTOM_TOOL_HINT,
    DISPLAY_SCREEN_CUSTOM_SPORT_HINT,

  

	/*����½���ʱ�ڸý���֮�����*/
	DISPLAY_SCREEN_END_TAIL,//�������һ������  
}ScreenState_t;

extern QueueHandle_t DisplayQueue;
extern ScreenState_t ScreenState;
extern ScreenState_t ScreenStateSave;
//#if defined WATCH_SIM_SPORT
extern ScreenState_t ScreenSportSave;
//#endif

extern volatile ScreenState_t Menu_Index;	//������
extern TaskHandle_t	TaskDisplayHandle;
//Display
extern void CreatDisplayTask(void);
extern void TaskDisplay(void* pvParameter);
extern void DisplayScreen(uint32_t index);

#endif
































































































































































