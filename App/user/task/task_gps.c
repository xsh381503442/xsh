#include "task_gps.h"
#include "task_display.h"
#include "task_key.h"
#include "task_config.h"
#include "task_sport.h"
#include "task_ble.h"
#include "task_step.h"

#include "bsp_rtc.h"
#include "drv_ublox.h"
#include "drv_extflash.h"
#include "drv_lsm6dsl.h"
#include "bsp.h"

#include "time_notify.h"
#include "timer_app.h"

#include "gui_time.h"
#include "gui_run.h"
#include "gui_sport.h"
#include "gui_gps_test.h"
#include "gui_step.h"

#include "com_sport.h"
#include "com_data.h"
#include "algo_trackoptimizing.h"
#include "algo_time_calibration.h"
#include "watch_config.h"


#if DEBUG_ENABLED == 1 && TASK_GPS_LOG_ENABLED == 1
	#define TASK_GPS_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define TASK_GPS_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define TASK_GPS_WRITESTRING(...)
	#define TASK_GPS_PRINTF(...)		        
#endif

#undef DEBUG_TASK_GPS    //����ʱ��log��ӡ����

static uint8_t m_GPSTimeFlag;	//GPS��ʱ״̬
GPSMode m_GPS_data;		//GPS����
TaskHandle_t	GPSTimeHandle = NULL;
SemaphoreHandle_t GPSSemaphore = NULL;
extern uint8_t gps_located;

#ifdef WATCH_SONY_GNSS

extern void drv_set_idle(void);
extern void drv_set_operation_mode(void);
extern void drv_set_NMEA_RMC_GSA(void);
extern void drv_set_NMEA_RMC(void);
extern void drv_set_NMEA_all(void);
extern void drv_set_NMEA_RMC_GGA_GSA(void);
extern void drv_set_positioning_algo(void);
extern void drv_set_GNSS_mode(uint8_t mode);
extern void drv_set_utc_time_set(void);
extern void drv_set_hotstart(void);
extern void drv_set_backup_data_save(void);
extern void drv_set_clodstart(void);
extern void drv_set_NMEA_RMC_GSA_GSV(void);

#ifdef STORE_GPS_LOG_DATA_TEST
  extern void store_log_data(uint8_t *log ,uint32_t len);
#endif

static uint8_t lle_data[2048] = {0};
const  uint8_t cep_header[4] = {0xA0, 0x00, 0x08, 0x00};
const  uint8_t cep_footer[4] = {0x00, 0x00, 0x00, 0xB0};
uint8_t GPS_START_FLAG = 0;

#endif

#if defined WATCH_GPS_ONLINE
extern uint8_t online_valid ;
#endif
#ifdef COD 
uint32_t location_timer_count = 0;
#endif

static bool gps_hint_motor_flag = false;//�����
uint32_t g_VO2max_Step = 0;//����������Ĳ���
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
static bool s_is_pps_rmc_task_valid = false;//GPS������ЧRMCʱ ������ʱ���Ƿ���Ч ����������First-pps,Ȼ�����
static uint32_t s_reset_gpsrmc_count = 0;//gps setrtcʱ���У��ʱ���Ƿ���RMC����һ��һ��,��������settime,��¼����
uint32_t get_s_reset_gpsrmc_count(void)
{
	return s_reset_gpsrmc_count;
}
void set_s_reset_gpsrmc_count(uint32_t value)
{
	s_reset_gpsrmc_count = value;
}
uint8_t s_is_open_nav_time_gps = TIME_NAV_GPS_NONE;//��¼�Ƿ�ʹ�ܷ���NAV-TIMEGPS
#endif

void set_gps_hint_motor_flag(bool flag)
{
	gps_hint_motor_flag = flag;
}
bool get_gps_hint_motor_flag(void)
{
	return gps_hint_motor_flag;
}
void gps_value_get(GPSMode *p_gps)
{
	memcpy(p_gps, &m_GPS_data, sizeof(GPSMode));
}

//���gps����
void gps_value_clear(void)
{
	memset(&m_GPS_data, 0, sizeof(m_GPS_data));
}


//��ȡ����
int32_t GetGpsLon(void)
{
    int32_t lon = 0;
    if( m_GPS_data.ewhemi == 'E' )
    {
        lon = m_GPS_data.longitude;
    }
    else
    {
        lon = 0-m_GPS_data.longitude;
    }

    return lon;
}

//��ȡγ��
int32_t GetGpsLat(void)
{
    int32_t lat = 0;
    if( m_GPS_data.nshemi == 'N' )
    {
        lat = m_GPS_data.latitude;
    }
    else
    {
        lat = 0-m_GPS_data.latitude;
    }
    return lat;
  
}

//��ȡgps״̬
bool GetGpsStatus (void)
{
   if(m_GPS_data.status == 'A') 
    {
        return true;
    }
   
    return false;
}

//��ȡgpsλ�þ�������
uint16_t GetGpspdop (void)
{
   
    return m_GPS_data.pdop;
}

int32_t GetGpsAlt(void)
{
	return m_GPS_data.altitude;
}

#if defined STORE_ALT_TEST_VERTION


#define MAX_SN_NUMBER 12
#define SWAP(x,y) {int32_t t; t = x; x = y; y = t;}

void quicksort(uint8_t number[], int32_t left, int32_t right) {
	int32_t i, j, s;
	if(left < right) {
		s = number[left];
		i = left;
		j = right + 1;
		while(1) {
		// ������
		while(i + 1 < MAX_SN_NUMBER && number[++i] > s) ;
		// ������
		while(j -1 > -1 && number[--j] < s) ;
		if(i >= j)
		break;
		SWAP(number[i], number[j]);
		}
	number[left] = number[j];
	number[j] = s;
	quicksort(number, left, j-1); // ����߽��еݻ�
	quicksort(number, j+1, right); // ���ұ߽��еݻ�
	}
}

//�����ź���ǿ��6��GPS�ź�
void GetGpsSn(uint8_t *sn)
{
	uint8_t temp_sn[MAX_SN_NUMBER];

	uint8_t i;
	for(i=0; i<MAX_SN_NUMBER; i++){
		temp_sn[i] = m_GPS_data.slmsg[i].sn;
	}
	quicksort(temp_sn,0,MAX_SN_NUMBER-1);
	for(uint8_t j=0; j<MAX_SN_NUMBER; j++){
		*sn = temp_sn[j];
		sn++;
	}	
}

uint16_t GetGpsHdop(void)
{
	return m_GPS_data.hdop;
}
#endif

/*���������������У׼�������ʱ�������㷨������ȡ*/
static void SetSteptoAlgo(void)
{
	if (GetGpsStatus())
	{
		if(CalibrationStep_GPSFlag == false)
		{//��һ�ζ���λ���¼����
			CalibrationStep_GPSFlag = true;
			Step_VO2CalInit = drv_lsm6dsl_get_stepcount();
		}
		Step_VO2maxOrCalibration = drv_lsm6dsl_get_stepcount() - Step_VO2CalInit;
		PhysicalParamCalDataAccept(GetGpsLon(),GetGpsLat(),Step_VO2maxOrCalibration,StepDiff_VO2maxOrCalibration);
	}
	else
	{//������GPSʱ����
		if(CalibrationStep_GPSFlag)
		{//��һ�ζ���λ��
			StepDiff_VO2maxOrCalibration += (drv_lsm6dsl_get_stepcount() - Step_VO2CalInit) - Step_VO2maxOrCalibration;
			Step_VO2maxOrCalibration = drv_lsm6dsl_get_stepcount() - Step_VO2CalInit;
		}
		else
		{//��һ��δ����λ
			Step_VO2maxOrCalibration = 0;
		}
	}
}
/*��ȡgps״̬���ж��Ƿ������ʱ*/
 bool GetGPSScreenStatus(void)
{
	if(ScreenState == DISPLAY_SCREEN_UTC_GET)
	{
		if(drv_ublox_status_get() >= TIME_GETTED)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if(drv_ublox_status_get() == POSITION_GETTED)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
//����GPS����
void CreateGPSTask(void)
{
        #ifdef WATCH_SONY_GNSS	 
          GPS_START_FLAG = 1;
        #endif
	drv_ublox_status_set(INITIATION_NONE);
	m_GPSTimeFlag = false;
	
	memset(&m_GPS_data,0,sizeof(m_GPS_data));

	if (GPSSemaphore == NULL)
	{
	    GPSSemaphore = xSemaphoreCreateBinary();  //������ֵ�ź���
	}
	
	if(GPSTimeHandle == NULL)
	{
		//TASK_GPS_PRINTF("[task_gps:] gps->Create\r\n");
		//����GPS��ʱ����
		xTaskCreate(TaskGPSTime,"Task GPS Time",TaskGPSTime_StackDepth,0,TaskGPSTime_Priority,&GPSTimeHandle);
	}
	else
	{
		vTaskResume(GPSTimeHandle);
	}
	set_gps_hint_motor_flag(false);
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
	s_is_pps_rmc_task_valid = false;
	s_reset_gpsrmc_count = 0;
	if(s_is_open_nav_time_gps == TIME_NAV_GPS_SEND_AND_INVALID)
	{//δ��������
		s_is_open_nav_time_gps = TIME_NAV_GPS_NONE;
		//TASK_GPS_PRINTF("[task_gps]: CreateGPSTask-->TIME_NAV_GPS_SEND_AND_INVALID!!!\n");
	}
	else if(s_is_open_nav_time_gps == TIME_NAV_GPS_VALID)
	{//�Ѿ������������
		set_is_nav_time_gps_valid(true);
		//TASK_GPS_PRINTF("[task_gps]: CreateGPSTask-->TIME_NAV_GPS_VALID!!!\n");
	}
	
#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
	if(s_is_open_nav_time_gps != TIME_NAV_GPS_VALID)
	{
		//TASK_GPS_PRINTF("[task_gps]: CreateGPSTask-->timer_app_detect_leap_second_start!!!\n");
		timer_app_detect_leap_second_start();
	}
#endif
	uint8_t m_time_calibration_status = ReadCalibrationValueAndConfigAdjValue();
	Set_TimeCalibrationStatus(m_time_calibration_status);//��GPSǰ���У׼״̬
#endif
}

#ifdef WATCH_SONY_GNSS
void drv_set_lle_search(void)
{

	const char cmdbuf[] = {"@CEPA\r\n"};//The CXD5603GF transfers to the Idle state
	
	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
}

void drv_set_lle_check(void)
{

	const char cmdbuf[] = {"@CEPC\r\n"};//The CXD5603GF transfers to the Idle state
	
	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);//500
}

void drv_set_lle_erase(void)
{
	const char cmdbuf[] = {"@CEPE\r\n"};//The CXD5603GF transfers to the Idle state

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}
	
	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);//400
}

void drv_set_lle_write(uint8_t num)
{
	char llenum[5] = {0};
	char cmdbuf[] = {"@CEPW "};//The CXD5603GF transfers to the Idle state
	 
	sprintf(llenum,"%d",num); 
	strcat(cmdbuf,llenum);
	strcat(cmdbuf,"\r\n\0");
	
	for(uint8_t i = 0;i < strlen(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
	TASK_GPS_PRINTF("%s\n",cmdbuf);
}

void drv_set_lle_header(void)
{	
	for(uint8_t i = 0;i < sizeof(cep_header);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cep_header[i]);	
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}
}

void drv_set_lle_date(void)
{	
	for(uint16_t i = 0;i < sizeof(lle_data);i++)
	{
		am_hal_uart_char_transmit_buffered(0, lle_data[i]);	
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}

}

void drv_set_lle_footer(void)
{	
	for(uint8_t i = 0;i < sizeof(cep_footer);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cep_footer[i]);
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}
	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
}

int agps_compare_time(char* agps_time_buf)
{
    char  timer_buf[5][20] = {0};
    uint8_t  i = 0,j = 0,k = 0;
    uint32_t  valuetime0 = 0;
    rtc_time_t start;

    //TASK_GPS_PRINTF("%s\n",gps_init_buf);
    for(j = 0; j < 5; j++)//5�ֶ�λϵͳʱ��
    { 
      if(agps_time_buf[k] == '0')//û������������
      {
        k += 3;   //0\r\nҪ3�ֽڵ���һ��ʱ��
        continue;
      }

      i = 0;//20�ֽ�ʱ����Ϣ2020 04 29 00 00 14'\0'
      while(agps_time_buf[k] != '\r')
      {
        timer_buf[j][i] = agps_time_buf[k];
        k++;
        i++;
      }
      timer_buf[j][i] = 0;
      k += 2;//\r\nҪ���ֽڵ���һ��

      //TASK_GPS_PRINTF("[time:%d] %s\n",j,timer_buf[j]);
    }

    sscanf(&timer_buf[0][17],"%d",&valuetime0);//GPS
    memset(&timer_buf[0][16],0,4);
  /*sscanf(&timer_buf[4][17],"%d",&valuetime4);
    memset(&timer_buf[4][16],0,4);
    
    if(strcmp(timer_buf[0], timer_buf[4]) > 0)//timer_buf[4]ʱ����
    {
      memcpy(timer_buf[4],&timer_buf[4][2],18);
      sscanf(timer_buf[4],"%d %d %d %d %d",(int *)&start.Year,(int *)&start.Month,
                         (int *)&start.Day,(int *)&start.Hour,(int *)&start.Minute);
      
      TASK_GPS_PRINTF("[time4:]%d-%d-%d-%d-%d %d\r\n",start.Year,start.Month,start.Day,start.Hour,start.Minute,valuetime4);
     
      valuetime4 = (valuetime4 - 1)*24*3600;
      valuetime4 = GetTimeSecond(start) + valuetime4;

      start = GetYmdhms(valuetime4);
      
      TASK_GPS_PRINTF("[effective time4:]%d-%d-%d-%d-%d\r\n",start.Year,start.Month,start.Day,start.Hour,start.Minute);
    }
    else
    {*/
       memcpy(timer_buf[0],&timer_buf[0][2],18);
       sscanf(timer_buf[0],"%d %d %d %d %d",(int *)&start.Year,(int *)&start.Month,
			       (int *)&start.Day,(int *)&start.Hour,(int *)&start.Minute);
          
       TASK_GPS_PRINTF("[time0:]%d-%d-%d-%d-%d %d\r\n",start.Year,start.Month,start.Day,start.Hour,start.Minute,valuetime0);

       if(valuetime0 >= 3)//������ 3�� 7�� 14��
       {
         valuetime0 = (valuetime0 - 1)*24*3600;
         valuetime0 = GetTimeSecond(start) + valuetime0;
         start = GetYmdhms(valuetime0);
       }
       else
       {
         valuetime0 = 0;
       }
       
       TASK_GPS_PRINTF("[effective time0:]%d-%d-%d-%d-%d\r\n",start.Year,start.Month,start.Day,start.Hour,start.Minute);
    //}

    if(start.Year && start.Month && start.Day && valuetime0)
    {
	//ʱ���
	SetValue.agps_effective_time.Year = start.Year;
	SetValue.agps_effective_time.Month= start.Month;
	SetValue.agps_effective_time.Day= start.Day;
	SetValue.agps_effective_time.Hour= start.Hour;
	SetValue.agps_effective_time.Minute= start.Minute;
	SetValue.agps_effective_time.Second= start.Second;
	return 1;
    }
    else
    {
        return 0;
    }

}

void agps_lle_import(void)
{
    char  agps_init_buf[20] = {0},agps_time_buf[130] = {0};
    uint8_t  agps_init_buf_Length = 0,agps_timers = 0;
    uint8_t  agps_restart_num = 0,packet_i = 0,packet_num = 0;

    uint32_t addr = SATELLITE_DATA_START_ADDRESS;
    _ublox_offline_data agps_header_data;
    dev_extFlash_enable();
    dev_extFlash_read(addr,(uint8_t *)&agps_header_data,sizeof(_ublox_offline_data));
    dev_extFlash_disable(); 
    addr += sizeof(_ublox_offline_data);
    TASK_GPS_PRINTF("[AGPS_LLE_SIZE] %d\n[AGPS_PAK1_SIZE] %d\n[AGPS_PAK2_SIZE] %d\n",agps_header_data.allpacket_len,agps_header_data.packet1_len,agps_header_data.packet2_len);

    if(agps_header_data.allpacket_len != 0xFFFFFFFF && agps_header_data.packet1_len != 0xFFFFFFFF && agps_header_data.packet2_len != 0xFFFFFFFF)//FLASH�����а�
    {
         while(GPS_START_FLAG)
	 {
	     if(agps_restart_num > 2)//һ���� 3 ��
	      break;
	     agps_restart_num++;
	            
	     drv_ublox_poweroff();
	     xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
	     drv_ublox_poweron();
	     xSemaphoreTake(GPSSemaphore,1050/portTICK_PERIOD_MS);//1Sһ��Ҫ

	     memset(agps_init_buf,0,sizeof(agps_init_buf));
	     drv_set_idle();
	     agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
	     if(agps_init_buf_Length)
	       agps_init_buf[agps_init_buf_Length] = 0;

	     agps_timers = 0;
	     while(strstr((const char*)agps_init_buf, "[GSTP] Done") == NULL)
	     {
		  xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
		  agps_timers++;
		  if(agps_timers >= 100 || !GPS_START_FLAG)
		   break;

		  agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
		  if(agps_init_buf_Length)
		    agps_init_buf[agps_init_buf_Length] = 0;
	     }
	     if(agps_timers >= 100 || !GPS_START_FLAG)//1s���ط�
	     {
		  TASK_GPS_PRINTF("[GSTP] ERR\n");
		  continue;
	     }
	     TASK_GPS_PRINTF("[GSTP] OK\n");

	     memset(agps_init_buf,0,sizeof(agps_init_buf));
	     drv_set_lle_erase();
	     agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
	     if(agps_init_buf_Length)
	       agps_init_buf[agps_init_buf_Length] = 0;

	     agps_timers = 0;
	     while(strstr((const char*)agps_init_buf, "[CEPE] Done") == NULL)
	     {
	         xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
	         agps_timers++;
	         if(agps_timers >= 100 || !GPS_START_FLAG)
		  break;
	       
	         agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
	         if(agps_init_buf_Length)
		   agps_init_buf[agps_init_buf_Length] = 0;
	     }
	     if(agps_timers >= 100 || !GPS_START_FLAG)//1s���ط�
	     {
	          TASK_GPS_PRINTF("[CEPE] ERR\n");
	          continue;
	     }
	     TASK_GPS_PRINTF("[CEPE] OK\n");
	 
	     for(packet_i = 0; packet_i < agps_header_data.allpacket_len/2048; packet_i++)//82K
	     {
	          packet_num = packet_i; 

                  if(packet_i % 2 == 0)
                  {
		      location_timer_count++;// GPS ������ 
		      DISPLAY_MSG  msg = {0,0};
		      msg.cmd = MSG_DISPLAY_SCREEN;
		      xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	          }   
	         
	          if(packet_i >= (agps_header_data.packet1_len + agps_header_data.packet2_len)/2048)//�ڶ����ļ�����
	            packet_num = packet_i - (agps_header_data.packet1_len + agps_header_data.packet2_len)/2048;
	          else
	            if(packet_i >= agps_header_data.packet1_len/2048)//��һ���ļ�����
	               packet_num = packet_i - agps_header_data.packet1_len/2048;
	           
		  memset(agps_init_buf,0,sizeof(agps_init_buf));	 
		  drv_set_lle_write(packet_num);
		  agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
		  if(agps_init_buf_Length)
	            agps_init_buf[agps_init_buf_Length] = 0;

		  agps_timers = 0;
	          while(strstr((const char*)agps_init_buf, "[CEPW] Ready") == NULL)
		  {
		    xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
		    agps_timers++;
		    if(agps_timers >= 100 || !GPS_START_FLAG)
		     break;
		  
		    agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
		    if(agps_init_buf_Length)
		      agps_init_buf[agps_init_buf_Length] = 0;
		  }
		  if(agps_timers >= 100 || !GPS_START_FLAG)//1s���ط�
		  {
		     TASK_GPS_PRINTF("[CEPW] ERR\n");
		     break;
		  } 
	          TASK_GPS_PRINTF("%s\n",agps_init_buf);
	          
	          memset(lle_data,0,sizeof(lle_data));
	          dev_extFlash_enable();
	          dev_extFlash_read(addr + packet_i*2048, lle_data ,2048);
	          dev_extFlash_disable();

	          for(uint16_t i = 0; i < 2048; i++)
	           TASK_GPS_PRINTF("%X",lle_data[i]);
	          TASK_GPS_PRINTF("\r\n");
				 
		  memset(agps_init_buf,0,sizeof(agps_init_buf));	 
		  drv_set_lle_header();
		  drv_set_lle_date();
		  drv_set_lle_footer();
		  agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
		  if(agps_init_buf_Length)
	            agps_init_buf[agps_init_buf_Length] = 0;

		 agps_timers = 0;
	         while(strstr((const char*)agps_init_buf, "[CEPW] Done") == NULL)
		 {
		   xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
		   agps_timers++;
		   if(agps_timers >= 100 || !GPS_START_FLAG)
		    break;
		 
		   agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
		   if(agps_init_buf_Length)
		     agps_init_buf[agps_init_buf_Length] = 0;
		 }
		 if(agps_timers >= 100 || !GPS_START_FLAG)//1s���ط�
	         {
	           TASK_GPS_PRINTF("[WRITE] ERR\n");
	           break;
	         }
	         TASK_GPS_PRINTF("%s\n",agps_init_buf);
				 
	     }

	     if(packet_i < (agps_header_data.allpacket_len/2048))//���°�����
	      continue;
	         
	     memset(agps_init_buf,0,sizeof(agps_init_buf));
	     drv_set_lle_check();
	     agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
	     if(agps_init_buf_Length)
	      agps_init_buf[agps_init_buf_Length] = 0;

	     agps_timers = 0;
	     while(strstr((const char*)agps_init_buf, "[CEPC] Done") == NULL)
	     {
		 xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
		 agps_timers++;
		 if(agps_timers >= 100 || !GPS_START_FLAG)
		  break;
	       
		 agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_init_buf, sizeof(agps_init_buf));
		 if(agps_init_buf_Length)
		   agps_init_buf[agps_init_buf_Length] = 0;
	     }
	     if(agps_timers >= 100 || !GPS_START_FLAG)//1s���ط�
	     {
	          TASK_GPS_PRINTF("[CEPC] ERR\n");
	          continue;
	     }

	     memset(agps_time_buf,0,sizeof(agps_time_buf));
	     drv_set_lle_search();
	     agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_time_buf, sizeof(agps_time_buf));
	     if(agps_init_buf_Length)
              agps_time_buf[agps_init_buf_Length] = 0;

	     agps_timers = 0;
	     while(strstr((const char*)agps_time_buf, "[CEPA] Done") == NULL)
	     {
		 xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
		 agps_timers++;
		 if(agps_timers >= 100 || !GPS_START_FLAG)
		  break;
	       
	         agps_init_buf_Length = am_hal_uart_char_receive_buffered(0, agps_time_buf, sizeof(agps_time_buf));
	         if(agps_init_buf_Length)
		   agps_time_buf[agps_init_buf_Length] = 0;
	     }
	     if(agps_timers >= 100 || !GPS_START_FLAG)//1s���ط�
	     {
	          TASK_GPS_PRINTF("[CEPA] ERR\n");
	          continue;
	     }

	     if(agps_compare_time(agps_time_buf))
	     {
	         TASK_GPS_PRINTF("[CEPC] OK\nAGPS_LLE_SUCCESS!!!\n");
	         SetValue.agps_update_flag = 0;
	         break;
	     }
	     else
	     {
		 TASK_GPS_PRINTF("[CEPC] ERR\n");
		 continue;
	     }
	 }
    }
    else//FLASH����û�а����ܸ���
    {
         SetValue.agps_update_flag = 0;
         TASK_GPS_PRINTF("No AGPS packets found\n");
    }
}

uint8_t ublox_start(uint8_t select)
{
   char  gps_init_buf[20] = {0};
   uint8_t  gps_init_buf_Length = 0, GPS_CMD_STATE = 0,restart_num = 0,timers = 0;  
   DISPLAY_MSG	msg = {0,0};

   if(SetValue.agps_update_flag)//��Ҫ����AGPS
    agps_lle_import();

   while(GPS_START_FLAG)
   {
     switch(GPS_CMD_STATE)
     {
       case 0:

	location_timer_count++;// GPS ������
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

 	drv_ublox_poweroff();
 	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
	drv_ublox_poweron();
	xSemaphoreTake(GPSSemaphore,1050/portTICK_PERIOD_MS);//1Sһ��Ҫ

	if(restart_num > 14)// 15 ��
	{
	  restart_num = 0;
	  drv_set_clodstart();//������
	  drv_set_GNSS_mode(UBLOX_GN);
	  xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
	  drv_set_NMEA_RMC_GSA_GSV(); 
	  xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
          TASK_GPS_PRINTF("[GCD] GPS Clodstart\n");
	  GPS_START_FLAG = 0;
	  break;
	}
	restart_num++;

        memset(gps_init_buf,0,sizeof(gps_init_buf));
        drv_set_idle();
        gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));
        if(gps_init_buf_Length)
	 gps_init_buf[gps_init_buf_Length] = 0;
        
        timers = 0;
        while(strstr((const char*)gps_init_buf, "[GSTP] Done") == NULL)
        {
          xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
          timers++;
          if(timers >= 100 || !GPS_START_FLAG)
           break;

	  gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));
	  if(gps_init_buf_Length)
	    gps_init_buf[gps_init_buf_Length] = 0;
        }

	#ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(gps_init_buf,gps_init_buf_Length);///
	#endif	

        if(timers >= 100 || !GPS_START_FLAG)//1s���ط�
        {
	   TASK_GPS_PRINTF("[GSTP] ERR\n");
	   GPS_CMD_STATE = 0;
        }
        else
        {
           TASK_GPS_PRINTF("[GSTP] OK\n");
           GPS_CMD_STATE = 1;
        }

        break;

       case 1:

        memset(gps_init_buf,0,sizeof(gps_init_buf));
        drv_set_operation_mode();
        gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));

        if(gps_init_buf_Length)
	 gps_init_buf[gps_init_buf_Length] = 0;

        timers = 0;
	while(strstr((const char*)gps_init_buf, "[GSOP] Done") == NULL)
        {
	  xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
	  timers++;
          if(timers >= 100 || !GPS_START_FLAG)
	   break;

          gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));
          if(gps_init_buf_Length)
	    gps_init_buf[gps_init_buf_Length] = 0;
        }
        
        #ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(gps_init_buf,gps_init_buf_Length);///
	#endif  
        
	if(timers >= 100 || !GPS_START_FLAG)//1s���ط�
        {
	  TASK_GPS_PRINTF("[GSOP] ERR\n");
	  GPS_CMD_STATE = 0;
        }
        else
        {
	  TASK_GPS_PRINTF("[GSOP] OK\n");
	  GPS_CMD_STATE = 2;
        } 
        break;

     case 2:

      memset(gps_init_buf,0,sizeof(gps_init_buf));
      if(select == 1)//��λ
      {
	 drv_set_NMEA_RMC_GSA();
	 TASK_GPS_PRINTF("GPS_TOOL\n");
      }
      else if(select == 2)//������ʱ
      {
          drv_set_NMEA_RMC();
          TASK_GPS_PRINTF("GPS_TIME\n"); 
      }
      else//�˶�
      {
 	#if defined  WATCH_GPS_HAEDWARE_ANALYSIS_INFOR
           drv_set_NMEA_all();
	#else
	   drv_set_NMEA_RMC_GGA_GSA();
	#endif
	TASK_GPS_PRINTF("GPS_SPORT\n");
      }
	  
      gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));

      if(gps_init_buf_Length)
       gps_init_buf[gps_init_buf_Length] = 0;

      timers = 0; 
      while(strstr((const char*)gps_init_buf, "[BSSL] Done") == NULL)
      {
	xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
	timers++;
	if(timers >= 100 || !GPS_START_FLAG)
	 break;
      
	gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));
	if(gps_init_buf_Length)
	  gps_init_buf[gps_init_buf_Length] = 0;
      }

    #ifdef STORE_GPS_LOG_DATA_TEST
      store_log_data(gps_init_buf,gps_init_buf_Length);///
    #endif
      
      if(timers >= 100 || !GPS_START_FLAG)//1s���ط�
      {
        TASK_GPS_PRINTF("[BSSL] ERR\n");
        GPS_CMD_STATE = 0;
      }
      else
      {
        TASK_GPS_PRINTF("[BSSL] OK\n");
        GPS_CMD_STATE = 3;
      } 
      break;

     case 3:

      memset(gps_init_buf,0,sizeof(gps_init_buf));
      drv_set_positioning_algo();
      gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, 20);

      if(gps_init_buf_Length)
       gps_init_buf[gps_init_buf_Length] = 0;

      timers = 0;       
      while(strstr((const char*)gps_init_buf, "[GUSE] Done") == NULL)
      {
 	xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
	timers++;
	if(timers >= 100 || !GPS_START_FLAG)
	 break;
      
	gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));
	if(gps_init_buf_Length)
	  gps_init_buf[gps_init_buf_Length] = 0;
      }

     #ifdef STORE_GPS_LOG_DATA_TEST
        store_log_data(gps_init_buf,gps_init_buf_Length);///
     #endif
	
      if(timers >= 100 || !GPS_START_FLAG)//1s���ط�
      {
        TASK_GPS_PRINTF("[GUSE] ERR\n"); 
        GPS_CMD_STATE = 0; 
      }
      else
      {
        TASK_GPS_PRINTF("[GUSE] OK\n");
        GPS_CMD_STATE = 4;
      } 
      break;

     case 4:

      memset(gps_init_buf,0,sizeof(gps_init_buf));
      drv_set_GNSS_mode(UBLOX_GN);
      gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, 20);

      if(gps_init_buf_Length)
       gps_init_buf[gps_init_buf_Length] = 0;

      timers = 0;       
      while(strstr((const char*)gps_init_buf, "[GNS] Done") == NULL)
      {
	xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
	timers++;
	if(timers >= 100 || !GPS_START_FLAG)
	 break;

	gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));
	if(gps_init_buf_Length)
	  gps_init_buf[gps_init_buf_Length] = 0;
      }
      
    #ifdef STORE_GPS_LOG_DATA_TEST
      store_log_data(gps_init_buf,gps_init_buf_Length);///
    #endif  
      
      if(timers >= 100 || !GPS_START_FLAG)//1s���ط�
      {
	TASK_GPS_PRINTF("[GNS] ERR\n");	      
	GPS_CMD_STATE = 0;
      }
      else
      {
	TASK_GPS_PRINTF("[GNS] OK\n");
	GPS_CMD_STATE = 5;
      } 
      break;      

     case 5:

      memset(gps_init_buf,0,sizeof(gps_init_buf));
      drv_set_utc_time_set();
      gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, 20);

      if(gps_init_buf_Length)
       gps_init_buf[gps_init_buf_Length] = 0;

      timers = 0;   
      while(strstr((const char*)gps_init_buf, "[GTIM] Done") == NULL)
      {
	xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
	timers++;
	if(timers >= 100 || !GPS_START_FLAG)
	 break;

	gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));
	if(gps_init_buf_Length)
	  gps_init_buf[gps_init_buf_Length] = 0;
      }

    #ifdef STORE_GPS_LOG_DATA_TEST
      store_log_data(gps_init_buf,gps_init_buf_Length);///
    #endif  

      if(timers >= 100 || !GPS_START_FLAG)//1s���ط�
      {
        TASK_GPS_PRINTF("[GTIM] ERR\n");	 
        GPS_CMD_STATE = 0;
      }
      else
      {
        TASK_GPS_PRINTF("[GTIM] OK\n");
        GPS_CMD_STATE = 6;
      } 
      break;    

      case 6:

      memset(gps_init_buf,0,sizeof(gps_init_buf));
      drv_set_hotstart();
      gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, 20);

      if(gps_init_buf_Length)
       gps_init_buf[gps_init_buf_Length] = 0;
      GPS_CMD_STATE = 0; 

      timers = 0;  
      while(strstr((const char*)gps_init_buf, "[GSR] Done") == NULL)
      {
	xSemaphoreTake(GPSSemaphore,10/portTICK_PERIOD_MS);
	timers++;
	if(timers >= 100 || !GPS_START_FLAG)
	 break;

	gps_init_buf_Length = am_hal_uart_char_receive_buffered(0, gps_init_buf, sizeof(gps_init_buf));
	if(gps_init_buf_Length)
	  gps_init_buf[gps_init_buf_Length] = 0;
      }

    #ifdef STORE_GPS_LOG_DATA_TEST
      store_log_data(gps_init_buf,gps_init_buf_Length);///
    #endif  
      
      if(timers >= 100 || !GPS_START_FLAG)//1s���ط�
      {
        TASK_GPS_PRINTF("[GSR] ERR\n"); 
      }
      else
      {
        TASK_GPS_PRINTF("[GSR] OK\n");
        GPS_START_FLAG = 0;
	return 1;
      } 
      break;       

      default:    
        GPS_START_FLAG = 0;
      break;
     }
   }
   return 0;
}

#endif

void CloseGPSTask(void)
{
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
	drv_ublox_TP5(TP5_MODE_THREE); /*�ر�1pps*/
	set_is_nav_time_gps_valid(false);
	if(get_timing_and_renewspace_status() == TIME_GPS_TIMING_NOT_NEW_SPACE)
	{//��ʱ����û�п��ռ� ��Ҫ�ر�ʱ�򿪱�
		TASK_GPS_PRINTF("[task_gps]: CloseGPSTask-->TIME_GPS_TIMING_NOT_NEW_SPACE!!!\n");
		am_hal_rtc_time_t m_before_time,m_now_time; //��ʱǰRTCʱ���Ҫ���õ�RTC��ʱ��
		m_before_time = Get_TimeRTCStr();
		am_hal_rtc_time_get(&m_now_time);
		RenewOneDay(m_before_time,m_now_time,true);//���￪�ٵĿռ��ʱ����ʱ���Բ���
	}
	else
	{
		TASK_GPS_PRINTF("[task_gps]: CloseGPSTask--> DONE!!!\n");
	}
#endif
	memset(&m_GPS_data,0,sizeof(GPSMode));
        #ifdef WATCH_SONY_GNSS   
          GPS_START_FLAG = 0;
        #endif
	drv_ublox_status_set(GPS_OFF);
	TASK_GPS_PRINTF("drv_ublox_status_set\r\n");
	
	xSemaphoreGive(GPSSemaphore);
	TASK_GPS_PRINTF("xSemaphoreGive\r\n");
	set_gps_hint_motor_flag(false);
	
	TASK_GPS_PRINTF("set_gps_hint_motor_flag\r\n");
#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
	timer_app_detect_leap_second_stop(false);
#endif
	TASK_GPS_PRINTF("timer_app_detect_leap_second_stop\r\n");

}

/*����ʱ�䷽�� WATCH_PRECISION_TIME_CALIBRATION*/
void TaskGPSTime(void* pvParameter)
{
	(void)pvParameter;
	uint16_t i = 0;
  	uint8_t gps_buf[BUFFER_GPS_LEN_MAX] = {0};
	uint16_t m_buf_total_len = 0;//ͳ�Ʒ����ӵ�buf����
	int16_t m_surplus_len = 0;//ͳ�Ƴ���gps_buf��������С�ĳ���

       #ifdef WATCH_SONY_GNSS
        uint8_t exti_num = 0,timers = 0;
       #endif
	  
	DISPLAY_MSG  msg = {0};

    #if defined WATCH_GPS_STREAM_READ
	uint16_t m_i2c_read_max = BUFFER_I2C_REAS_LEN_MAX;//i2cÿ�ζ����ݴ�С
	am_hal_iom_buffer(2) data_len = {0};
	am_hal_iom_buffer(BUFFER_I2C_REAS_LEN_MAX) data_buf = {0};
	uint8_t ui8Offset;
	uint16_t gps_len;
	uint16_t m_buf_gps_len = 0;//ͳ������GPS�����������ݴ�С
    #else
	uint8_t data = 0;
    #endif
	while(1)
	{
	    location_timer_count++;
		if( drv_ublox_status_get() == GPS_OFF )
		{
		    #ifdef WATCH_SONY_GNSS
			if (rtc_hw_flag == 1)
			{
			   exti_num = 3;//�ط�����
			   while(exti_num--)
			   {
			         memset(gps_buf,0,sizeof(gps_buf));
				 drv_set_idle();
				 m_buf_total_len = am_hal_uart_char_receive_buffered(0, (char*)gps_buf, UBLOX_RX_BUFFER_SIZE);

				 if(m_buf_total_len)
				  gps_buf[m_buf_total_len] = 0;

			         timers = 0;
				 while(strstr((const char*)gps_buf, "[GSTP] Done") == NULL)
				 {
                                   vTaskDelay(10/portTICK_PERIOD_MS);
                                   timers++;
                                   if(timers >= 100)
                                    break;
				   m_buf_total_len = am_hal_uart_char_receive_buffered(0, (char*)gps_buf, UBLOX_RX_BUFFER_SIZE);
				   if(m_buf_total_len)
				    gps_buf[m_buf_total_len] = 0;
				 }
				 
				#ifdef STORE_GPS_LOG_DATA_TEST
				 store_log_data(gps_buf,m_buf_total_len);///
				#endif 
				 
				 if(timers >= 100)//1s���ط�
				 {
				    TASK_GPS_PRINTF("[GSTP] ERR\n");
				 }
				 else
				 {
				    TASK_GPS_PRINTF("[GSTP] OK\n");

                                    memset(gps_buf,0,sizeof(gps_buf));
				    drv_set_backup_data_save();   
				    m_buf_total_len = am_hal_uart_char_receive_buffered(0, (char*)gps_buf, UBLOX_RX_BUFFER_SIZE);

				    if(m_buf_total_len)
				     gps_buf[m_buf_total_len] = 0;

			            timers = 0;
				    while(strstr((const char*)gps_buf, "[BUP] Done") == NULL)
				    {
				      vTaskDelay(10/portTICK_PERIOD_MS);
				      timers++;
				      if(timers >= 100)
				       break;
				      m_buf_total_len = am_hal_uart_char_receive_buffered(0, (char*)gps_buf, UBLOX_RX_BUFFER_SIZE);
				      if(m_buf_total_len)
				       gps_buf[m_buf_total_len] = 0;
				    }
				    
                                   #ifdef STORE_GPS_LOG_DATA_TEST
				    store_log_data(gps_buf,m_buf_total_len);///
				   #endif  
				    
				    if(timers >= 100)//1s���ط�
				    {
			              TASK_GPS_PRINTF("[BUP] ERR\n");  
				    }
				    else
				    {
				      TASK_GPS_PRINTF("[BUP] OK\n");
				      break; 
				    } 
				 }
			   }
			}
			else
			{
			   drv_ublox_REVERT();
			}
		      #endif
		      
			//�ر�GPS��Դ
			drv_ublox_poweroff();

			//��������
		       if( drv_ublox_status_get() == GPS_OFF )//��ֹƵ������������λ
			vTaskSuspend(NULL);
			//TASK_GPS_PRINTF("[task_gps]: gps->GPS_OFF\n");
		}

		if(drv_ublox_status_get() == INITIATION_NONE)
		{
			//��δ��ʼ��,��GPS��Դ
                        if(rtc_hw_flag == 0)
                        {
			  drv_ublox_poweron();
			  vTaskDelay(150/portTICK_PERIOD_MS);	
			  TASK_GPS_PRINTF("[ublox]: GPS_POWER_ON\n");
			}  
	        
			ublox_init();
			 
			if(drv_ublox_status_get() != GPS_OFF)
			{
			   drv_ublox_status_set(WAIT_FOR_TIME);
			}
			else
			{
			   drv_ublox_status_set(GPS_OFF);
			}
			
			//TASK_GPS_PRINTF("[task_gps]: gps->INITIATION_NONE\n");
		}
		else
		{
			//TASK_GPS_PRINTF("[task_gps]: gps->WAIT_FOR_TIME\n");
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
			//��ȡGPS����ǰȷ�� ��ЧPPS״̬ ��ֹ������������ǰ�ڼ�,��pps
			if((GetGPS1PPSPinFlag() == GPS_PPS_FIRST || GetGPS1PPSPinFlag() == GPS_PPS_THIRD)&& (GetGPSScreenStatus() == true))
			{
				s_is_pps_rmc_task_valid = true;
			}
			else
			{
				s_is_pps_rmc_task_valid = false;
			}
			//TASK_GPS_PRINTF("[task_gps]: gps->WAIT_FOR_TIME s_is_pps_rmc_task_valid=%d\n",s_is_pps_rmc_task_valid);
#endif
				memset(gps_buf,0x0,sizeof(gps_buf));
				drv_ublox_enable();
#if defined WATCH_GPS_STREAM_READ   //JW600����ʵ��������
				ui8Offset = 0xFD;
				am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,data_len.words,2,AM_HAL_IOM_OFFSET(ui8Offset));
				gps_len =(data_len.bytes[0] <<8  | data_len.bytes[1]);
				//TASK_GPS_PRINTF("[task_gps]: gps_len %d\n",gps_len);
//				TASK_GPS_PRINTF("[task_gps]:--1-GPS_BUFFER len=%d---\n",gps_len);
				m_buf_total_len = 0;//ͳ�Ʒ����ӵ�buf����
				m_surplus_len = 0;//ͳ�Ƴ���gps_buf��������С�ĳ���

				//���¶���GPS��buffer����
				while(gps_len > BUFFER_GPS_LEN_MAX)
				{//������������ݳ���gps_buf�����С
					m_surplus_len = gps_len - BUFFER_GPS_LEN_MAX;
					if(m_surplus_len >= m_i2c_read_max)
					{
						if (am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,data_buf.words,m_i2c_read_max,AM_HAL_IOM_RAW)==AM_HAL_IOM_SUCCESS)
						{//֮ǰ�����ݶ�������
							gps_len -= m_i2c_read_max;
						}
					}
					else
					{
						if (am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,data_buf.words,m_surplus_len,AM_HAL_IOM_RAW)==AM_HAL_IOM_SUCCESS)
						{//֮ǰ�����ݶ�������
							gps_len -= m_surplus_len;
						}
					}
				}
				if(gps_len != 0 && m_i2c_read_max != 0)
				{
					for(int j = 0;j < (gps_len/m_i2c_read_max)+1;j++)//���ѭ������
					{
						if(gps_len <= BUFFER_GPS_LEN_MAX)
						{//�����������ݲ�����gps_buf�����С
							if(gps_len >= m_i2c_read_max)
							{//���buffer����255,i2c��������
								if (am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,data_buf.words,m_i2c_read_max,AM_HAL_IOM_RAW)==AM_HAL_IOM_SUCCESS)
								{
									for (i=0; i< m_i2c_read_max; i++)
									{
										if(data_buf.bytes[i] != 0xFF)
										{
											gps_buf[i+m_buf_total_len] = data_buf.bytes[i];
										}
										else
										{
											break;
										}
									}
									m_buf_total_len += i;
									gps_len -= m_i2c_read_max;
								}
							}
							else
							{//���buffer����255i2cһ�ζ���
								if(gps_len != 0)
								{//��buffer��С��Ϊ0,һ�ζ���
									if (am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,data_buf.words,gps_len,AM_HAL_IOM_RAW)==AM_HAL_IOM_SUCCESS)
									{
										for (i=0; i< gps_len; i++)
										{
											if(data_buf.bytes[i] != 0xFF)
											{
												gps_buf[i+m_buf_total_len] = data_buf.bytes[i];
											}
											else
											{
												break;
											}
										}
										m_buf_total_len += i;
										gps_len = 0;
									}
								}
							}
						}
					}
				}

				ui8Offset = 0xFD;
				am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,data_len.words,2,AM_HAL_IOM_OFFSET(ui8Offset));
				gps_len =(data_len.bytes[0] <<8  | data_len.bytes[1]);
				if(gps_len + m_buf_total_len > BUFFER_GPS_LEN_MAX)
				{//����GPSbuf
					m_buf_gps_len = m_buf_total_len;
				}
				else
				{//����GPSbuf
					m_buf_gps_len = gps_len + m_buf_total_len;//��ȡ
				}
				//TASK_GPS_PRINTF("[task_gps]: m_buf_total_len %d gps_len %d\n",m_buf_total_len,gps_len);
				 if(((gps_len+m_buf_total_len) <= BUFFER_GPS_LEN_MAX)&&( gps_len != 0))
				 {
						if(gps_len >= m_i2c_read_max)
						{//���buffer����255,i2c��������
							if (am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,data_buf.words,m_i2c_read_max,AM_HAL_IOM_RAW)==AM_HAL_IOM_SUCCESS)
							{
								for (i=0; i< m_i2c_read_max; i++)
								{
									if(data_buf.bytes[i] != 0xFF)
									{
										gps_buf[i+m_buf_total_len] = data_buf.bytes[i];
									}
									else
									{
										break;
									}
								}
									m_buf_total_len += i;
									gps_len -= m_i2c_read_max;
							}
						}
						else
						{//���buffer����255i2cһ�ζ���
						   if(gps_len != 0)
							{//��buffer��С��Ϊ0,һ�ζ���
								if (am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,data_buf.words,gps_len,AM_HAL_IOM_RAW)==AM_HAL_IOM_SUCCESS)
								{
									for (i=0; i< gps_len; i++)
									{
										if(data_buf.bytes[i] != 0xFF)
										{
											gps_buf[i+m_buf_total_len] = data_buf.bytes[i];
										}
										else
										{
											break;
										}
									}
										m_buf_total_len += i;
										gps_len = 0;
								}
							}
						}
				 }
#else
#if defined  WATCH_HUADA_GNSS || defined WATCH_SONY_GNSS || defined WATCH_UBLOX_MOUDULE

				uint16_t ui16Length;

				if (rtc_hw_flag == 1)
				{
					if (g_UartRxTimeoutFlag)
					{
						g_UartRxTimeoutFlag = 0;
				
						ui16Length = am_hal_uart_char_receive_buffered(0, (char*)gps_buf, UBLOX_RX_BUFFER_SIZE);
								
						if (ui16Length)
						{
							// We have overallocated the gps_buf, to make sure ui32Length==UBLOX_RX_BUFFER_SIZE is ok
							gps_buf[ui16Length] = 0;
				
						}
							
					}
				}
				else
				{
				
				for(i=0;i<BUFFER_GPS_LEN_MAX;i++)
				{
					data = drv_ublox_read();
					if(data != 0xFF)
					{
						gps_buf[i] = data;
					}
					else
					{
						break;
					}
				}
				}
#endif
#endif
				drv_ublox_disable();
				
                              #ifdef STORE_GPS_LOG_DATA_TEST
                                store_log_data(gps_buf,ui16Length);///
                              #endif     
				
				am_hal_rtc_time_get(&RTC_time);
				TASK_GPS_PRINTF("[task_gps]: %d-%02d-%02d %02d:%02d:%02d.%02d0 -->buf=%s\n",
												RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth,
												RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second,
												RTC_time.ui32Hundredths,gps_buf);
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
				NMEA_GPRMC_New_Analysis(&m_GPS_data,gps_buf,m_buf_gps_len);
#else
				NMEA_GPRMC_Analysis(&m_GPS_data,gps_buf);
                
#endif

#if defined STORE_ALT_TEST_VERTION
				NMEA_GPGGA_Analysis(&m_GPS_data,gps_buf);
				NMEA_GPGSV_Analysis(&m_GPS_data,gps_buf);
				NMEA_GPGSA_Analysis(&m_GPS_data,gps_buf);
#endif

#if defined(WATCH_PRECISION_TIME_CALIBRATION)
			if(GetGPS1PPSPinFlag() == GPS_FILTER_INVALID)
			{//�������
				SetGPS1PPSPinFlag(GPS_FILTER_INVALID_TWO);
			}
			else if(GetGPS1PPSPinFlag() == GPS_PPS_FIRST && (GetGPSScreenStatus() == true))
			{//��һ��1pps	
				//��¼GPSʱ�� ���ڶ���ʱ��+1s��ʱ��RTC
				if(IsTriathlonMode(ScreenState) == false && m_GPSTimeFlag != true && get_nmea_rmc_data_valid() == true
					 && (s_is_pps_rmc_task_valid == true))
				{
					s_is_pps_rmc_task_valid = false;
					//��¼��һ��1pps�յ���GPSʱ��,�ڵڶ���1pps����ʱ+1����׼ȷ��GPSʱ�� ��NMEA��������Чgpsʱ�� 
					SatelliteGPSTime(&m_GPS_data);
					SetGPS1PPSPinFlag(GPS_PPS_SECOND);//��gps��ȡʱ��
					//TASK_GPS_PRINTF("[task_gps]: GPS_PPS_FIRST-->SetGPS1PPSPinFlag(GPS_PPS_SECOND)----s_is_pps_rmc_task_valid=%d\n",s_is_pps_rmc_task_valid);
				}
				else
				{
					SetGPS1PPSPinFlag(GPS_PPS_NONE);//����1pps״̬���¿�ʼ
					//TASK_GPS_PRINTF("[task_gps]: GPS_PPS_FIRST-->SetGPS1PPSPinFlag(GPS_PPS_NONE)!!!s_is_pps_rmc_task_valid=%d\n",s_is_pps_rmc_task_valid);
				}
			}
			else if(GetGPS1PPSPinFlag() == GPS_PPS_THIRD && (GetGPSScreenStatus() == true) && (s_is_pps_rmc_task_valid == true))
			{//�ڶ���1pps  ��ȡGPS׼ȷʱ�������ʱ
				s_is_pps_rmc_task_valid = false;
				if(IsTriathlonMode(ScreenState) == false && m_GPSTimeFlag != true)
				{//�����������˶�ģʽ������ʱ�ɹ�
					am_hal_rtc_time_t m_now_time;
					
					if(get_nmea_rmc_data_valid() == true)
					{
						//TASK_GPS_PRINTF("[task_gps]:get_open_difftime_status -->GPS_PPS_THIRD--!!!\n");
						am_hal_rtc_time_t m_last_rmc_time;
						
						SatelliteGPSTime(&m_GPS_data);//����RMCʱ��
						m_last_rmc_time = Get_TimePPSGPSStr();//��ȡ����RMC��GPSʱ��
						am_hal_rtc_time_get(&m_now_time);//��ȡ�Ѿ�set��rtcʱ��
						//TASK_GPS_PRINTF("[task_gps]:-->GPS_PPS_THIRD--m_last_rmc_time: %d-%02d-%02d %d:%02d:%02d.%02d0,rtc:%d-%02d-%02d %d:%02d:%02d.%02d0\n"								 						 
						 ,m_last_rmc_time.ui32Year,m_last_rmc_time.ui32Month,m_last_rmc_time.ui32DayOfMonth
						 ,m_last_rmc_time.ui32Hour,m_last_rmc_time.ui32Minute,m_last_rmc_time.ui32Second,m_last_rmc_time.ui32Hundredths
						 ,m_now_time.ui32Year,m_now_time.ui32Month,m_now_time.ui32DayOfMonth
						 ,m_now_time.ui32Hour,m_now_time.ui32Minute,m_now_time.ui32Second,m_now_time.ui32Hundredths);
						SetGPS1PPSPinFlag(GPS_PPS_FOURTH);
					}
					else
					{
						//TASK_GPS_PRINTF("[task_gps]:get_open_difftime_status -->GPS_PPS_THIRD--get_nmea_rmc_data_valid=false!!!\n");
						SetGPS1PPSPinFlag(GPS_PPS_NONE);//����1pps״̬���¿�ʼ
					}
#if defined(DEBUG_TASK_GPS)
					am_hal_rtc_time_t m_second_gps_time = Get_TimeGPSStr();
					am_hal_rtc_time_t m_second_rtc_time = Get_TimeRTCStr();
					//TASK_GPS_PRINTF("[task_gps]: GPS_PPS_COMPLETE -->GetGPS1PPSPinFlag=%d,GPSTime %d-%02d-%02d %d:%02d:%02d.%02d0\n"
				   ,GetGPS1PPSPinFlag(),m_second_gps_time.ui32Year,m_second_gps_time.ui32Month,m_second_gps_time.ui32DayOfMonth
				   ,m_second_gps_time.ui32Hour,m_second_gps_time.ui32Minute,m_second_gps_time.ui32Second,m_second_gps_time.ui32Hundredths);
					//TASK_GPS_PRINTF("[task_gps]: GPS_PPS_COMPLETE -->GetGPS1PPSPinFlag=%d,RTCTime before GPSTime %d-%02d-%02d %d:%02d:%02d.%02d0\n"
				   ,GetGPS1PPSPinFlag(),m_second_rtc_time.ui32Year,m_second_rtc_time.ui32Month,m_second_rtc_time.ui32DayOfMonth
				   ,m_second_rtc_time.ui32Hour,m_second_rtc_time.ui32Minute,m_second_rtc_time.ui32Second,m_second_rtc_time.ui32Hundredths);
#endif
				}
			}
				else if( GetGPS1PPSPinFlag() == GPS_PPS_FIFTH && (GetGPSScreenStatus() == true) )
				{
					//TASK_GPS_PRINTF("[task_gps]:GPS_PPS_FIFTH -->\n");
						if(IsTriathlonMode(ScreenState) == false && m_GPSTimeFlag != true)
						{//�����������˶�ģʽ������ʱ�ɹ�
							am_hal_rtc_time_t m_before_time,m_now_time; //��ʱǰRTCʱ���Ҫ���õ�RTC��ʱ��
							
							m_before_time = Get_TimeRTCStr();
							am_hal_rtc_time_get(&RTC_time);
							m_now_time = RTC_time;//��ȡ�Ѿ�set��rtcʱ��
						if(get_open_difftime_status() == STATUS_OPEN_DIFFITME_COMPLETE && ScreenState == DISPLAY_SCREEN_TEST_REAL_DIFF_TIME)
						{//�Լ���� ʵʱʱ�����ȡ��GPSʱ��ȽϽ��� ��ȡ��GPSʱ���ر�GPS����
							//TASK_GPS_PRINTF("[task_gps]:get_open_difftime_status -->STATUS_OPEN_DIFFITME_COMPLETE--!!!\n");
							drv_ublox_TP5(TP5_MODE_THREE); /*��ʱ��ɺ� �ر�1pps*/
							SetGPS1PPSPinFlag(GPS_PPS_COMPLETE);//��ʱ���
							CloseGPSTask();
#if defined(WATCH_PRECISION_TIME_JUMP_TEST)
							//��������������ʱʱ��ͻ��Ķ�ʱ�� ������Ϊ�˻�ȡGPSʱ�� ���ǲ�����ʱ��RTC �������ο��Ƚ�ʱ��
							timer_app_precision_time_jump_test_start();
#endif
						}
						else
						{
							//��ʱ��ľ���ʱ�䴦��ҵ��
							if(is_set_real_time_to_rtc() == true)
							{
								RenewOneDay(m_before_time,m_now_time,true);//���￪�ٵĿռ��ʱ����ʱ���Բ���
								set_timing_and_renewspace_status(TIME_GPS_TIMING_NEW_SPACE);
								//TASK_GPS_PRINTF("[task_gps]: GPS_PPS_COMPLETE -->RTC_time: %d-%02d-%02d %d:%02d:%02d.%02d0\n"
									 ,RTC_time.ui32Year,RTC_time.ui32Month,RTC_time.ui32DayOfMonth
									 ,RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second,RTC_time.ui32Hundredths);
#if defined(WATCH_NOT_LEAP_SECOND_SET_TIME)
								if(s_is_open_nav_time_gps == TIME_NAV_GPS_VALID)
								{
									//TASK_GPS_PRINTF("[task_gps]:Get leap success and time is to calibration !\n");
									TimeCalibrationInGps();
								}
								else
								{
									//TASK_GPS_PRINTF("[task_gps]:Get leap fail and time is not to calibration !\n");
								}
#else
								TimeCalibrationInGps();
#endif
								set_s_is_mcu_restart_and_need_set_time(false);
							}
							m_GPSTimeFlag = true; /*������ʱ�ɹ�*/
#if defined(WATCH_GPS_OR_NO_GPS_RESOURCE_SET_TIME)
							set_s_set_time_resource(SET_TIME_RESOURCE_BY_GPS);//����RTCʱ����Դ--GPS��ʱ
#endif
							drv_ublox_TP5(TP5_MODE_THREE); /*��ʱ��ɺ� �ر�1pps*/
							SetGPS1PPSPinFlag(GPS_PPS_COMPLETE);//��ʱ���	
						}
					}
					else
					{//��������ģʽ������ʱ
	#if defined(DEBUG_TASK_GPS)
						if(m_GPSTimeFlag == true)
						{
							//TASK_GPS_PRINTF("[task_gps]: GPS_PPS_COMPLETE-->Now is >= TIME_GETTED!!!\n");
						}
						else
						{
							//TASK_GPS_PRINTF("[task_gps]: GPS_PPS_COMPLETE-->Now is TriathlonMode!!!\n");
						}
	#endif
					}
			}
#endif
				if((ScreenState == DISPLAY_SCREEN_GPS) 
					|| (ScreenState == DISPLAY_SCREEN_GPS_DETAIL)
					|| (ScreenState == DISPLAY_SCREEN_PROJECTION_ZONING))
				{
				    
					set_gps_flicker(1);
					//λ�ù��߽���,��λ�ɹ�������Զ���ʱ
					if((m_GPSTimeFlag != true) && (drv_ublox_status_get() == POSITION_GETTED))
					{
						//��ʱ�ɹ����޸�ʱ��
						SatelliteTime(&m_GPS_data);
						m_GPSTimeFlag = true;
					}
					NMEA_GPGSA_Analysis(&m_GPS_data,gps_buf);
				//	TASK_GPS_PRINTF("[task_gps]: possl_gpsnum %d\n",m_GPS_data.possl_gpsnum);

				}
				else if(ScreenState == DISPLAY_SCREEN_LACTATE_THRESHOLD_HINT ||
								ScreenState == DISPLAY_SCREEN_VO2MAX_HINT            ||
								ScreenState == DISPLAY_SCREEN_STEP_CALIBRATION_HINT)
				{
					set_gps_flicker(1);
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					if(m_GPS_data.status == 'A' && !get_gps_hint_motor_flag())
					{
						set_gps_hint_motor_flag(true);
						//�����λ�ɹ��������в��Խ���1��ˢ�£���λ������ʾ
						timer_notify_motor_start(1000,500,1,false,NOTIFY_MODE_SPORTS);
						timer_notify_buzzer_start(1000,500,1,false,NOTIFY_MODE_SPORTS);
					}
				}
				else if(ScreenState == DISPLAY_SCREEN_UTC_GET)
				{
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
#else
#if defined WATCH_GPS_ONLINE
					if (online_valid ==1)
					{
						if((m_GPSTimeFlag != true) && (drv_ublox_status_get() == POSITION_GETTED))
						{
							//��ʱ�ɹ����޸�ʱ��
							SatelliteTime(&m_GPS_data);
							m_GPSTimeFlag = true;
						}
						
					}
					else
					{
						if((m_GPSTimeFlag != true) && (drv_ublox_status_get() >= TIME_GETTED))
						{
							//��ʱ�ɹ����޸�ʱ��
							SatelliteTime(&m_GPS_data);
							m_GPSTimeFlag = true;
						}

					}
				
#else
					if((m_GPSTimeFlag != true) && (drv_ublox_status_get() >= TIME_GETTED))
					{
						//��ʱ�ɹ����޸�ʱ��
						SatelliteTime(&m_GPS_data);
						m_GPSTimeFlag = true;
					}
#endif

#endif
					extern _gui_time_utc_get_param g_gui_time_utc_get_param;
					if(m_GPSTimeFlag == true)
					{
						//��ʱ�ɹ�
						g_gui_time_utc_get_param.evt = GUI_TIME_UTC_GET_EVT_SUCCESS;
						
						CloseGPSTask();
					}
					else if(g_gui_time_utc_get_param.progress >= (GUI_TIME_UTC_TIMEOUT-1))
					{
						//��ʱʧ��
						g_gui_time_utc_get_param.evt = GUI_TIME_UTC_GET_EVT_FAIL;
						
						CloseGPSTask();
					}
					else
					{
						g_gui_time_utc_get_param.progress += 1;
					}
					
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}


					else if (IsSportReady(ScreenState) == true ||ScreenState == DISPLAY_SCREEN_SPORT )
					{
					
						NMEA_GPGGA_Analysis(&m_GPS_data,gps_buf);
						NMEA_GPGSA_Analysis(&m_GPS_data,gps_buf);
						if((m_GPSTimeFlag != true) && (drv_ublox_status_get() == POSITION_GETTED))
						{
							//��ʱ�ɹ����޸�ʱ��
							SatelliteTime(&m_GPS_data);
							m_GPSTimeFlag = true;
						}
						
						set_gps_flicker(1);
						gui_swich_msg();
						
					}

					else if(ScreenState == DISPLAY_SCREEN_VO2MAX_MEASURING)
					{//����������������� gps�������
						set_gps_flicker(1);
						//���������뵽�㷨��
							SetSteptoAlgo();

						if((m_GPSTimeFlag != true) && (drv_ublox_status_get() == POSITION_GETTED))
						{
							//��ʱ�ɹ����޸�ʱ��
							SatelliteTime(&m_GPS_data);
							m_GPSTimeFlag = true;
						}
					}
					else if(ScreenState == DISPLAY_SCREEN_STEP_CALIBRATION_SHOW)
					{//����У׼����ˢ�� ��ȡ����
						set_gps_flicker(1);
						if(GetCalDistanceStatus())
						{//��ok��У׼��ſ�ʼ�������
							//���������뵽�㷨��
							SetSteptoAlgo();
						}
						msg.cmd = MSG_UPDATE_STEP_CALIBRETION;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);

						if((m_GPSTimeFlag != true) && (drv_ublox_status_get() == POSITION_GETTED))
						{
							//��ʱ�ɹ����޸�ʱ��
							SatelliteTime(&m_GPS_data);
							m_GPSTimeFlag = true;
						}
					}

					else if (IsSportMode(ScreenState) == true)
					{
						NMEA_GPGGA_Analysis(&m_GPS_data,gps_buf);
						NMEA_GPGSA_Analysis(&m_GPS_data,gps_buf);
						if((m_GPSTimeFlag != true) && (drv_ublox_status_get() == POSITION_GETTED))
						{
							//��ʱ�ɹ����޸�ʱ��
							SatelliteTime(&m_GPS_data);
							m_GPSTimeFlag = true;
						}
					}
#if defined(WATCH_PRECISION_TIME_CALIBRATION)					
					else if(ScreenState == DISPLAY_SCREEN_TEST_REAL_DIFF_TIME)
					{//��ʾ�Լ���� ʵʱʱ���������
						ScreenState = DISPLAY_SCREEN_TEST_REAL_DIFF_TIME;
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
#endif					
					else if(ScreenState == DISPLAY_SCREEN_POST_GPS || ScreenState == DISPLAY_SCREEN_POST_GPS_DETAILS)
					{
						//�Լ챱������
						NMEA_GPGSA_Analysis(&m_GPS_data,gps_buf);
						NMEA_GPGSV_Analysis(&m_GPS_data,gps_buf);
						NMEA_GBGSV_Analysis(&m_GPS_data,gps_buf);
					#ifdef WATCH_GPS_SNR_TEST
						ble_uart_ccommand_application_send_gps_test_data(&m_GPS_data);
					#endif		
						//TASK_GPS_PRINTF("**************WATCH_GPS_SNR_TEST*********");
						msg.cmd = MSG_DISPLAY_SCREEN;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					}
	#ifdef HARDWARE_TEST	
				if(ScreenState == DISPLAY_SCREEN_TEST_GPS || ScreenState == DISPLAY_SCREEN_TEST_GPS_DETAILS)
				{
					//�Լ챱������
					NMEA_GPGSA_Analysis(&m_GPS_data,gps_buf);
					NMEA_GPGSV_Analysis(&m_GPS_data,gps_buf);
					NMEA_GBGSV_Analysis(&m_GPS_data,gps_buf);
					//TASK_GPS_PRINTF("**************MSG_UPDATE_TEST_GPS*********");
				}
	#endif

			 if (gps_located == 0)
			  {		  	
			  	if (GetGpsStatus() == true &&((IsSportReady(ScreenState) == true) ||(IsSportMode(ScreenState) == true)))
			  	{
			  		//��λ�ɹ�����������
			  		if((ScreenSportSave!=DISPLAY_SCREEN_SWIMMING)&&(ScreenSportSave!=DISPLAY_SCREEN_INDOORRUN))
			  			{
					timer_notify_motor_start(500,500,1,false,NOTIFY_MODE_ALARM);
					timer_notify_buzzer_start(500,500,1,false,NOTIFY_MODE_ALARM);
           #ifdef COD 
					ScreenStateSave = ScreenState;
					timer_notify_display_start(1500,1,false);

					DISPLAY_MSG  msg = {0,0};
					ScreenState = DISPLAY_SCREEN_NOTIFY_LOCATION_OK;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			#endif
			  			}
					gps_located = 1;
				}

			  }

				//ÿ���ȡһ�α�������
				//

				xSemaphoreTake(GPSSemaphore,1000/portTICK_PERIOD_MS);
//				vTaskDelay(1000/portTICK_PERIOD_MS);
			}
	}

}

void ublox_init(void)
{
	//drv_ublox_offline_ephemeris();
      #if defined(WATCH_PRECISION_TIME_CALIBRATION)
	drv_ublox_ppx_init();//��ʼ��gps 1pps�ж�����
      #endif

	switch(ScreenState)
	{
		case DISPLAY_SCREEN_UTC_GET:
		
              #if defined WATCH_SONY_GNSS	
		if (rtc_hw_flag == 1)
		{
		   ublox_start(2);
		}
		else
		{
			drv_ublox_reset_rtc();
                      #if defined(WATCH_SINGLE_BEIDOU)
		        drv_ublox_GNSS(UBLOX_GB);
                      #endif
                      #if defined(WATCH_PRECISION_TIME_CALIBRATION)
			drv_ublox_SBAS(false);
			drv_ublox_TP5(TP5_MODE_ONE);
                      #endif
			drv_ublox_GGA(false);
			drv_ublox_GSV(false);
			drv_ublox_GLL(false);
			drv_ublox_GSA(false);
			drv_ublox_VTG(false);
			drv_ublox_RMC(true);
                     #if defined(WATCH_SINGLE_BEIDOU)
                     #else
		        drv_ublox_GNSS(UBLOX_GN);
                     #endif	
		}
              #endif	
                break;
                
		case DISPLAY_SCREEN_GPS:
		case DISPLAY_SCREEN_GPS_DETAIL:
		case DISPLAY_SCREEN_PROJECTION_ZONING:

              #ifdef WATCH_SONY_GNSS
		if (rtc_hw_flag == 1)
		{
		   ublox_start(1);
		}	
		else
		{
			drv_ublox_hotstart();
 		      #if defined(WATCH_SINGLE_BEIDOU)
		        drv_ublox_GNSS(UBLOX_GB);
		      #endif
		      #if defined(WATCH_PRECISION_TIME_CALIBRATION)
			drv_ublox_SBAS(false);
			drv_ublox_TP5(TP5_MODE_ONE);
		      #endif
			drv_ublox_GGA(false);
			drv_ublox_GSV(false);
			drv_ublox_GLL(false);
			drv_ublox_GSA(true);
			drv_ublox_VTG(false);
			drv_ublox_RMC(true);
                      #if defined(WATCH_SINGLE_BEIDOU)
                      #else
		        drv_ublox_GNSS(UBLOX_GN);
                      #endif
		}
              #endif
		break;
		
              #if defined(HARDWARE_TEST)
		case DISPLAY_SCREEN_TEST_GPS:
		case DISPLAY_SCREEN_TEST_GPS_DETAILS:
			//GPS�Լ����
			drv_ublox_clodstart();
                      #if defined(WATCH_SINGLE_BEIDOU)
		        drv_ublox_GNSS(UBLOX_GB);
                      #endif
                      #if defined(WATCH_PRECISION_TIME_CALIBRATION)
			s_is_open_nav_time_gps = TIME_NAV_GPS_NONE;//������ʱ��Ҫ�������
			drv_ublox_SBAS(false);
			drv_ublox_TP5(TP5_MODE_ONE);
                      #endif
			drv_ublox_GGA(false);
			drv_ublox_GSV(true);
			drv_ublox_GLL(false);
			drv_ublox_GSA(true);
			drv_ublox_VTG(false);
			drv_ublox_RMC(true);
                     #if defined(WATCH_SINGLE_BEIDOU)
                     #else
		        drv_ublox_GNSS(UBLOX_GN);
                     #endif
			break;
               #endif
               
		case DISPLAY_SCREEN_POST_GPS:
		case DISPLAY_SCREEN_POST_GPS_DETAILS:
               #ifdef WATCH_SONY_GNSS
		if (rtc_hw_flag == 1)
		{
			drv_ublox_poweron();
			xSemaphoreTake(GPSSemaphore,1050/portTICK_PERIOD_MS);//1Sһ��Ҫ

			drv_set_clodstart();
			drv_set_GNSS_mode(UBLOX_GN);
			xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
			drv_set_NMEA_RMC_GSA_GSV();
			xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
		}
		else
		{
			//GPS�Լ����
			drv_ublox_clodstart();
                      #if defined(WATCH_SINGLE_BEIDOU)
		        drv_ublox_GNSS(UBLOX_GB);
		      #endif
		      #if defined(WATCH_PRECISION_TIME_CALIBRATION)
			s_is_open_nav_time_gps = TIME_NAV_GPS_NONE;//������ʱ��Ҫ�������
			drv_ublox_SBAS(false);
			drv_ublox_TP5(TP5_MODE_ONE);
		      #endif
			drv_ublox_GGA(false);
			drv_ublox_GSV(true);
			drv_ublox_GLL(false);
			drv_ublox_GSA(true);
			drv_ublox_VTG(false);
			drv_ublox_RMC(true);
		     #if defined(WATCH_SINGLE_BEIDOU)
		     #else
		        drv_ublox_GNSS(UBLOX_GN);
                     #endif
		}
               #endif
	        break;
	        
               #if defined WATCH_HAS_SOS
		case DISPLAY_SCREEN_SOS:
		case DISPLAY_SCREEN_SOS_MENU:
			//SOS����
			drv_ublox_hotstart();
		      #if defined(WATCH_SINGLE_BEIDOU)
		        drv_ublox_GNSS(UBLOX_GB);
		      #endif
			drv_ublox_GGA(false);
			drv_ublox_GSV(false);
			drv_ublox_GLL(false);
			drv_ublox_GSA(false);
			drv_ublox_VTG(false);
			drv_ublox_RMC(true);
                      #if defined(WATCH_SINGLE_BEIDOU)
                      #else
			drv_ublox_GNSS(UBLOX_GN);
                      #endif
		  break;
                #endif
                 
		default:
		//if((IsSportMode(ScreenState) == true) || (IsSportReady(ScreenState) == true))
                {      
			//�˶�gps����
			//drv_ublox_reset_rtc();
                     #ifdef WATCH_SONY_GNSS
			if (rtc_hw_flag == 1)
			{		
			   ublox_start(0);
			}
			else
			{
                            #if defined(WATCH_SINGLE_BEIDOU)
		                drv_ublox_GNSS(UBLOX_GB);
          		    #endif
                            #if defined(WATCH_PRECISION_TIME_CALIBRATION)
				drv_ublox_SBAS(false);
				drv_ublox_TP5(TP5_MODE_ONE);
                            #endif
                            #if defined STORE_ALT_TEST_VERTION
				drv_ublox_GGA(true);
				drv_ublox_GSV(false);
				drv_ublox_GLL(false);
				drv_ublox_GSA(true);
				drv_ublox_VTG(false);
				drv_ublox_RMC(true);
                             #else			
			      #ifdef COD
				drv_ublox_GGA(true);
				drv_ublox_GSV(true);
				drv_ublox_GLL(false);
				drv_ublox_GSA(true);
				drv_ublox_VTG(false);
				drv_ublox_RMC(true);
			      #else
				drv_ublox_GGA(false);
				drv_ublox_GSV(false);
				drv_ublox_GLL(false);
				drv_ublox_GSA(false);
				drv_ublox_VTG(false);
				drv_ublox_RMC(true);
			      #endif
                            #endif
                            #if defined(WATCH_SINGLE_BEIDOU)
                            #else
				drv_ublox_GNSS(UBLOX_GP);
			 	//drv_ublox_RXM(UBLOX_POWERSAVE);
				drv_ublox_RXM_SAVE(UBLOX_POWERSAVE);
                            #endif
			}
                     #endif				
                }
		break;
	}

	if(rtc_hw_flag == 0)
	{
          #if defined WATCH_GPS_ONLINE
	   drv_ublox_online_offline_ephemeris_select();
          #else
	   drv_ublox_offline_ephemeris();
          #endif
        }
}
