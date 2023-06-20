#ifndef DRV_UBLOX_H
#define DRV_UBLOX_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "watch_config.h"


#define		UBLOX_GP		0x01	//��GPSģʽ
#define		UBLOX_GB		0x02	//������ģʽ
#define		UBLOX_GN		0x03	//GPS+BDģʽ

#define		UBLOX_CONTINUOUS	0x0	//continuous mode
#define		UBLOX_POWERSAVE		0x1	//power save mode

#define   TP5_MODE_ONE  (1)//time pulse mode 1ģʽ
#define   TP5_MODE_TWO  (2)//time pulse mode 2ģʽ
#define   TP5_MODE_THREE  (3)//time pulse mode close
//#define		PORTABLE	0
//#define

#if defined (WATCH_HUADA_GNSS) || defined WATCH_SONY_GNSS || defined WATCH_UBLOX_MOUDULE
#define UBLOX_TX_BUFFER_SIZE 128
#define UBLOX_RX_BUFFER_SIZE 1199
extern uint8_t g_UartRxTimeoutFlag;
#endif


typedef  unsigned char  u8;
typedef struct
{
    //
    // Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    // Chip Select number to use for IOM access.
    //
    uint32_t ui32ChipSelect;
}
ublox_t;

typedef struct  
{										    
 	uint8_t num;		//���Ǳ��
	uint8_t eledeg;		//��������
	uint16_t azideg;	//���Ƿ�λ��
	uint8_t sn;			//�����		   
}nmea_slmsg;

typedef struct  
{										    
 	uint16_t year;	//���
	uint8_t month;	//�·�
	uint8_t date;	//����
	uint8_t hour; 	//Сʱ
	uint8_t min; 	//����
	uint8_t sec; 	//����
}nmea_utc_time; 

typedef struct
{
 	uint8_t svnum;					//�ɼ�GPS������
	uint8_t bdnum;					//�ɼ�����������
	nmea_slmsg slmsg[12];			//���12������
	nmea_slmsg bdmsg[12];			//���12������
	nmea_utc_time utc;				//UTCʱ��
	uint8_t	status;					//��λ״̬ A=��Ч��λ,V=��Ч��λ
	uint32_t latitude;				//γ�� ������1000000��,ʵ��Ҫ����1000000
	uint8_t nshemi;					//��γ/��γ,N:��γ;S:��γ				  
	uint32_t longitude;			    //���� ������1000000��,ʵ��Ҫ����1000000
	uint8_t ewhemi;					//����/����,E:����;W:����
	uint8_t gpssta;					//GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.				  
 	uint8_t posslnum;				//���ڶ�λ��������,0~12.
 	uint8_t possl[12];				//���ڶ�λ�����Ǳ��
	uint8_t fixmode;				//��λ����:1,û�ж�λ;2,2D��λ;3,3D��λ
	uint16_t pdop;					//λ�þ������� 0~9999,��Ӧʵ��ֵ0~99.99
	uint16_t hdop;					//ˮƽ�������� 0~9999,��Ӧʵ��ֵ0~99.99
	uint16_t vdop;					//��ֱ�������� 0~9999,��Ӧʵ��ֵ0~99.99 
	int altitude;			 		//���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m	 
	uint16_t speed;					//��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ
	uint8_t possl_gpsnum;			//���ڶ�λ��GPS����
	uint8_t possl_bdnum;			//���ڶ�λ�ı�������
}GPSMode;

#define GPS_SCALE_FLOAT (1000000.f)
#define GPS_SCALE_INT   (1000000)
#define GPS_SCALE_DECIMAL   (1000000.0)

typedef  enum
{
	GPS_OFF = 0,   		//GPS�ر�
	INITIATION_NONE,	//δ��ʼ��	
	WAIT_FOR_TIME, 		//��ʱ��
	TIME_GETTED,		//��ʱ�ɹ�
	SEARCHING_SAT,		//������
	POSITION_GETTED,    	//��ȡλ��	
}GPSState_t;

typedef  enum
{
	GPS_PPS_NONE = 0,   		//GPS 1ppsû��
	GPS_FILTER_INVALID,     //GPS ���˶�λ�����һ��1pps����
	GPS_FILTER_INVALID_TWO, //GPS ���˶�λ����ڶ���1pps����
	GPS_FILTER_INVALID_THIRD, //GPS ���˶�λ����ڶ���1pps����
	GPS_PPS_FIRST,          //GPS 1pps ��һ�� ����
	GPS_PPS_SECOND,         //GPS 1pps �ڶ���  ��GPS��ȡ��ʱ��
	GPS_PPS_THIRD,          //GPS 1pps ������  �ڶ��δ���1pps,��GPSʱ��+1��Ϊ����ʱ��
	GPS_PPS_FOURTH,
	GPS_PPS_FIFTH,
	GPS_PPS_COMPLETE,       //GPS ��ʱ���    ������ʱ�����õ�rtc �ر�1pps
}GPS_1PPSFlag;

typedef struct __attribute__((packed))
{
	uint32_t len;

	#ifdef WATCH_SONY_GNSS
	  uint32_t allpacket_len;
	  uint32_t packet1_len;
	  uint32_t packet2_len;
	  uint8_t  reserve[4];
	#else
	
	  uint8_t reserve[16];
	#endif

}_ublox_offline_data;


#define TIME_GPS_NOT_TIMING_NOT_NEW_SPACE   (0)
#define TIME_GPS_TIMING_NOT_NEW_SPACE       (1)
#define TIME_GPS_TIMING_NEW_SPACE           (2)

extern void drv_ublox_enable(void);
extern void drv_ublox_disable(void);
extern void drv_ublox_poweron(void);
extern void drv_ublox_poweroff(void);
extern uint8_t drv_ublox_read(void);
extern void drv_ublox_GGA(bool status);
extern void drv_ublox_GSV(bool status);
extern void drv_ublox_GLL(bool status);
extern void drv_ublox_GSA(bool status);
extern void drv_ublox_VTG(bool status);
extern void drv_ublox_RMC(bool status);
extern void drv_ublox_GNSS(uint8_t mode);
extern void drv_ublox_RXM(uint8_t mode);
extern void drv_ublox_reset_rtc(void);
extern void drv_ublox_hotstart(void);
extern void drv_ublox_PM2(void);
extern void drv_ublox_ZDA(bool status);
extern void drv_ublox_RXM_SAVE(uint8_t mode);
extern void drv_ublox_REVERT(void);
extern void Page_Post_BeidouAnt(void);
extern void Update_Post_StarNum(GPSMode *beidou);
extern void Update_Post_LonLat(GPSMode *beidou);
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
extern void NMEA_GPRMC_New_Analysis(GPSMode *gpsx,uint8_t *buf,uint16_t buf_len);
#else
extern void NMEA_GPRMC_Analysis(GPSMode *gpsx,uint8_t *buf);
#endif
extern void NMEA_GPGSV_Analysis(GPSMode *gpsx,uint8_t *buf);
extern void NMEA_GBGSV_Analysis(GPSMode *gpsx,uint8_t *buf);
extern void NMEA_GPGGA_Analysis(GPSMode *gpsx,uint8_t *buf);
extern void NMEA_GPGSA_Analysis(GPSMode *gpsx,uint8_t *buf);

extern void SatelliteTime(GPSMode *gps);
#if defined WATCH_GPS_ONLINE
extern uint8_t ublox_online_offline_data_in_flash_uptodata(void);
extern void drv_ublox_online_offline_ephemeris_select(void);
#endif
extern void ublox_offline(void);
extern bool ublox_is_offline_data_in_flash_uptodate(void);

extern void drv_ublox_status_set(uint8_t status);
extern uint8_t drv_ublox_status_get(void);

extern void SetGPS1PPSPinFlag(uint8_t isEnable);
extern uint8_t GetGPS1PPSPinFlag(void);
extern void GetSatelliteTime(GPSMode *gps);
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
extern void drv_ublox_ppx_init(void);
extern void set_is_nav_time_gps_valid(bool status);
extern bool get_is_nav_time_gps_valid(void);
extern void set_timing_and_renewspace_status(uint8_t status);
extern uint8_t get_timing_and_renewspace_status(void);
#endif



void dev_ublox_utc_read_init(void);
bool dev_ublox_utc_read(am_hal_rtc_time_t *time);
void dev_ublox_utc_read_uninit(void);
extern void drv_ublox_offline_ephemeris(void);
extern void drv_ublox_TP5(uint8_t mode);
extern void drv_ublox_SBAS(bool flag);
void SatelliteGPSTime(GPSMode *gps);
extern void Set_TimePPSGPSStr(am_hal_rtc_time_t status);
extern am_hal_rtc_time_t Get_TimePPSGPSStr(void);
extern void drv_ublox_clodstart(void);
extern void set_nmea_rmc_data_valid(bool status);
extern bool get_nmea_rmc_data_valid(void);
extern void drv_ublox_nav_timegps(bool flag);
#endif
