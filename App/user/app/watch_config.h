#ifndef WATCH_CONFIG_H
#define	WATCH_CONFIG_H


/*���ܿ���*/
#undef WACTH_THEME_APP_DOWNLOAD			//�Զ�������̹���
#undef WATCH_SPORT_RUNNING_TYPE			//�ܲ����ֱ� ��ԽҰ���˶�
#undef WATCH_UNLOCK_BACK_UP				//BACK+UP����
#undef WATCH_GPS_STREAM_READ			//gpsһ�ζȶ������
#undef WATCH_STEP_ALGO          		//ʹ�üƲ��㷨
#undef WATCH_PRECISION_TIME_CALIBRATION       //������ʱ���ܿ���
#undef WATCH_PRECISION_TIME_JUMP_TEST  //������ʱʱ��ͻ��������� ǰ�ᾫ����ʱ���ش�
#undef WATCH_SINGLE_BEIDOU           //����������
#undef WATCH_GPS_SET_TIME_AUTOMATICALLY  //�Զ���ʱ����
#undef WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME //����RTCʱ����Դ ��������ʱ��������ʱ��������ʱ����Դ
#undef WATCH_RAISE_BRIGHT_SCREEN_ALGO         	//ʹ��̧�������㷨,����Ʋ��ſ���Ч
#undef WATCH_GPS_ONLINE							//gps online����

#undef WATCH_GPS_SNR_TEST           //����GPS�����

#undef WATCH_COM_SPORT			//����ϵ���˶���UI������
#undef WATCH_COM_SHORTCUNT_UP 	//�����ֱ��ϼ���ݹ���
#undef WATCH_COM_SETTING			//�����ֱ����ù���
#undef WATCH_NOT_LEAP_SECOND_SET_TIME   //����ȡ����ֱ����ʱ ������ʱУ׼����Ҫ�ж�������Ч
#undef WATCH_SPORT_RECORD_TEST     //�˶���¼��ѯ��ʽ����
#undef WATCH_IMU_CALIBRATION //���ٶ�У׼���ܱ��뿪��
#undef WATCH_AUTO_BACK_HOME  //������ˢ��ҳ�泬��10�������κβ������ް������������ش������� �˶������С���λ�С���������������С������в����С����ʲ����С���������С�����г���
#undef WATCH_NO_ACCESSORY
#undef STORE_ORG_DATA_TEST_VERSION		//ð�̰汾ԭʼgps�Ĵ洢�������㷨����
#undef WATCH_NO_FONT_CN_16_24		//��ʹ�ô����Դ�����16��24������
#undef WATCH_SPORT_EVENT_SCHEDULE   //ԽҰ�ܵ��������ι���
#undef WATCH_SPORT_NEW_CLOUD_NAVIGATION  //���˶����Ƽ�����
#undef HRT_DEBUG_TEST  //���ʵ��Թ���
#undef WATCH_SONY_GNSS //sony��λоƬ


/*Ӳ������*/
#undef WACTH_HEARTRATE_VERSION			//philips���ʰ汾��
#undef WATCH_HAS_NO_BLE					//��������������
#undef WATCH_I2C_RESET					//���ʿ���ʱ�����ֱ�
#undef WATCH_SPORT_NO_COMPASS			//�˶�����ָ����
#undef WATCH_SPORT_NO_PRESS				//�˶�������ѹ�߶�
#undef WATCH_TOOL_COUNTDOWN				//����ʱ����(freeone����)
#undef WATCH_MODEL_PLUS
#undef WATCH_COD_BSP
#undef WATCH_CHIP_TEST //��Ƭ����

#undef WATCH_HUADA_GNSS //����λоƬ
#undef WATCH_GPS_HAEDWARE_ANALYSIS_INFOR //GPSӲ����λ������Ϣ

/*��UI�ͽ���*/
#undef WATCH_SIM_SPORT				//�ֱ��˶�����
#undef WATCH_SIM_UNLOCK				//��������
#undef WATCH_SIM_LIGHTS_ON			//���ⳣ��
#undef WATCH_SIM_NUMBER				//�ֱ�����
#undef STORE_ORG_DATA_TEST_VERSION //ԭʼGPS����,�洢�ռ�����ݴ�����GPS�ռ��ص�(���WATCH_GPS_HAEDWARE_ANALYSIS_INFOR�겢�رմ�GPS���湦��)





#if defined COD
#define	WACTH_THEME_APP_DOWNLOAD		//�Զ�����̹���
#define WATCH_STEP_ALGO    				//�Ʋ��㷨
#define WATCH_RAISE_BRIGHT_SCREEN_ALGO  //ʹ��̧�������㷨,����Ʋ��ſ���Ч

//#define WACTH_HEARTRATE_VERSION			//��ȡ���ʰ汾

//#define WATCH_GPS_ONLINE				//gps online����
//#define WATCH_GPS_STREAM_READ			//gpsһ�ζȶ������

#define WATCH_TOOL_COUNTDOWN			//����ʱ����(freeone����)


#define	WATCH_UNLOCK_BACK_UP			//BACK+UP����
#define	WATCH_SIM_UNLOCK			//��������
#define WATCH_SIM_LIGHTS_ON			//���ⳣ��

#define WATCH_SIM_NUMBER			//�ֱ�����
#define WATCH_COM_SPORT			//�˶���UI������
#define WATCH_COM_SHORTCUNT_UP 	//�ϼ���ݹ���
#define WATCH_COM_SETTING			//���ù���

#define WATCH_AUTO_BACK_HOME  //������ˢ��ҳ�泬��10�������κβ������ް������������ش�������
//#define STORE_ALT_TEST_VERTION

//#define WATCH_MODEL_PLUS				//������ʹ��vallencell�ļ�
#define WATCH_NO_FONT_CN_16_24
#define WATCH_COD_BSP

//#define WATCH_LCD_ESD  //������Իָ�
#define WATXCH_LCD_TRULY
//#define WATCH_LCD_LPM
//#define WATCH_HUADA_GNSS //����λģ��
#define WATCH_SONY_GNSS //sony��λģ��

//#define WATCH_UBLOX_MOUDULE //ubox��λģ��
#define WATCH_RTC_RV8803

//�洢GPSԭʼ����
//#define WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
//#define STORE_ORG_DATA_TEST_VERSION

//�洢GPS��LOG����
//#define STORE_GPS_LOG_DATA_TEST

//��λϵͳѡ��
#define WATCH_GPS_MODE  1  // 1 (GPS + BDS + QZSS) 2 (GPS + GLONASS + QZSS) 3 (GPS + QZSS + Galileo)

#endif 

#if defined HY100
#define	WACTH_THEME_APP_DOWNLOAD		//�Զ�����̹���
#define WATCH_STEP_ALGO    				//�Ʋ��㷨
#define WATCH_RAISE_BRIGHT_SCREEN_ALGO  //ʹ��̧�������㷨,����Ʋ��ſ���Ч

//#define WACTH_HEARTRATE_VERSION			//��ȡ���ʰ汾

#define WATCH_GPS_ONLINE				//gps online����
//#define WATCH_GPS_STREAM_READ			//gpsһ�ζȶ������

#define WATCH_TOOL_COUNTDOWN			//����ʱ����(freeone����)


#define	WATCH_UNLOCK_BACK_UP			//BACK+UP����
#define	WATCH_SIM_UNLOCK			//��������
#define WATCH_SIM_LIGHTS_ON			//���ⳣ��

#define WATCH_SIM_NUMBER			//�ֱ�����
#define WATCH_COM_SPORT			//�˶���UI������
#define WATCH_COM_SHORTCUNT_UP 	//�ϼ���ݹ���
#define WATCH_COM_SETTING			//���ù���

#define WATCH_AUTO_BACK_HOME  //������ˢ��ҳ�泬��10�������κβ������ް������������ش�������
//#define STORE_ORG_DATA_TEST_VERSION		//ð�̰汾ԭʼgps�Ĵ洢�������㷨����
//#define STORE_ALT_TEST_VERTION

#define WATCH_NO_FONT_CN_16_24
#define WATCH_COD_BSP

//#define WATCH_LCD_ESD  //������Իָ�
#endif 

#if defined WATCH_SPORT_EVENT_SCHEDULE //ԽҰ�ܵ��������ι���
#define WATCH_SPORT_NEW_CLOUD_NAVIGATION  //���˶����Ƽ�����
#endif


#if defined WATCH_SIM_SPORT
#define BLE_PROTOCAL_VERSION_CODE          0x00000103      //,֧���˶������޸ģ���ͬ��Ŀ�����в�ͬ�汾�ţ�APP����ͨ���汾�������ݣ���������Э��汾��
#else
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION
#define BLE_PROTOCAL_VERSION_CODE          0x00000004      //��ͬ��Ŀ�����в�ͬ�汾�ţ�APP����ͨ���汾�������ݣ���������Э��汾��
#else
#define BLE_PROTOCAL_VERSION_CODE          0x00000003      //��ͬ��Ŀ�����в�ͬ�汾�ţ�APP����ͨ���汾�������ݣ���������Э��汾��
#endif
#endif

#if defined WATCH_PRECISION_TIME_CALIBRATION
#define WATCH_GPS_STREAM_READ			//gpsһ�ζȶ������
#define WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME //����ʱ������GPS�ͷ�GPS������Դ����
#define WATCH_NOT_LEAP_SECOND_SET_TIME  //����ȡ����ֱ����ʱ ������ʱУ׼����Ҫ�ж�������Ч
#endif

#define WATCH_GPS_SNR_TEST  //Ĭ�Ͽ���GPS����Ȳ���
#define WATCH_SPORT_RECORD_TEST     //�˶���¼��ѯ��ʽ����

#endif
