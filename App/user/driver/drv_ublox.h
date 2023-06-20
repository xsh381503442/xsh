#ifndef DRV_UBLOX_H
#define DRV_UBLOX_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "watch_config.h"


#define		UBLOX_GP		0x01	//单GPS模式
#define		UBLOX_GB		0x02	//单北斗模式
#define		UBLOX_GN		0x03	//GPS+BD模式

#define		UBLOX_CONTINUOUS	0x0	//continuous mode
#define		UBLOX_POWERSAVE		0x1	//power save mode

#define   TP5_MODE_ONE  (1)//time pulse mode 1模式
#define   TP5_MODE_TWO  (2)//time pulse mode 2模式
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
 	uint8_t num;		//卫星编号
	uint8_t eledeg;		//卫星仰角
	uint16_t azideg;	//卫星方位角
	uint8_t sn;			//信噪比		   
}nmea_slmsg;

typedef struct  
{										    
 	uint16_t year;	//年份
	uint8_t month;	//月份
	uint8_t date;	//日期
	uint8_t hour; 	//小时
	uint8_t min; 	//分钟
	uint8_t sec; 	//秒钟
}nmea_utc_time; 

typedef struct
{
 	uint8_t svnum;					//可见GPS卫星数
	uint8_t bdnum;					//可见北斗卫星数
	nmea_slmsg slmsg[12];			//最多12颗卫星
	nmea_slmsg bdmsg[12];			//最多12颗卫星
	nmea_utc_time utc;				//UTC时间
	uint8_t	status;					//定位状态 A=有效定位,V=无效定位
	uint32_t latitude;				//纬度 分扩大1000000倍,实际要除以1000000
	uint8_t nshemi;					//北纬/南纬,N:北纬;S:南纬				  
	uint32_t longitude;			    //经度 分扩大1000000倍,实际要除以1000000
	uint8_t ewhemi;					//东经/西经,E:东经;W:西经
	uint8_t gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.				  
 	uint8_t posslnum;				//用于定位的卫星数,0~12.
 	uint8_t possl[12];				//用于定位的卫星编号
	uint8_t fixmode;				//定位类型:1,没有定位;2,2D定位;3,3D定位
	uint16_t pdop;					//位置精度因子 0~9999,对应实际值0~99.99
	uint16_t hdop;					//水平精度因子 0~9999,对应实际值0~99.99
	uint16_t vdop;					//垂直精度因子 0~9999,对应实际值0~99.99 
	int altitude;			 		//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	uint16_t speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时
	uint8_t possl_gpsnum;			//用于定位的GPS星数
	uint8_t possl_bdnum;			//用于定位的北斗星数
}GPSMode;

#define GPS_SCALE_FLOAT (1000000.f)
#define GPS_SCALE_INT   (1000000)
#define GPS_SCALE_DECIMAL   (1000000.0)

typedef  enum
{
	GPS_OFF = 0,   		//GPS关闭
	INITIATION_NONE,	//未初始化	
	WAIT_FOR_TIME, 		//授时中
	TIME_GETTED,		//授时成功
	SEARCHING_SAT,		//搜星中
	POSITION_GETTED,    	//获取位置	
}GPSState_t;

typedef  enum
{
	GPS_PPS_NONE = 0,   		//GPS 1pps没有
	GPS_FILTER_INVALID,     //GPS 过滤定位到后第一次1pps无用
	GPS_FILTER_INVALID_TWO, //GPS 过滤定位到后第二次1pps无用
	GPS_FILTER_INVALID_THIRD, //GPS 过滤定位到后第二次1pps无用
	GPS_PPS_FIRST,          //GPS 1pps 第一次 触发
	GPS_PPS_SECOND,         //GPS 1pps 第二次  从GPS获取了时间
	GPS_PPS_THIRD,          //GPS 1pps 第三次  第二次触发1pps,把GPS时间+1作为最新时间
	GPS_PPS_FOURTH,
	GPS_PPS_FIFTH,
	GPS_PPS_COMPLETE,       //GPS 授时完成    把最新时间设置到rtc 关闭1pps
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
