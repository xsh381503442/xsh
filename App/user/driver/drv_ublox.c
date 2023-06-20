/**
*Module:GPS
*Chip:UBLOX 8030
*Version: V 0.0.1
*Time: 2017/11/15
**/
#include "drv_config.h"
#include "drv_ublox.h"
#include "algo_time_calibration.h"
#include "bsp.h"
#include "bsp_iom.h"
#include "bsp_rtc.h"
#include "com_data.h"
#include "task_gps.h"
#include "task_step.h"

#include "com_dial.h"

#include <stdlib.h>
#include <stdio.h>
#include "drv_extFlash.h"


#if DEBUG_ENABLED == 1 && DRV_UBLOX_LOG_ENABLED == 1
	#define DRV_UBLOX_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_UBLOX_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define DRV_UBLOX_LOG_WRITESTRING(...)
	#define DRV_UBLOX_LOG_PRINTF(...)		        
#endif

static GPSState_t GPSStatus;	//GPS状态
///static GPSMode m_gps_data;		//GPS数据
am_hal_rtc_time_t s_time_gps;
//extern SemaphoreHandle_t Appolo_SPI_Semaphore;
extern SemaphoreHandle_t Appolo_I2C_Semaphore;
extern am_hal_rtc_time_t RTC_time;

extern SemaphoreHandle_t GPSSemaphore;

#if defined WATCH_GPS_ONLINE
uint8_t online_valid = 0;
#endif
static bool s_is_nmea_rmc_data_valid = false; //NMEA解析RMC数据 默认没有解析出有效时间
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
static uint32_t s_continuous_pps_count = 0;//为了保证授时前后的1pps连续
static int64_t m_gps_rtc_diff1 = 0,m_gps_rtc_diff2 = 0;
#endif
void set_nmea_rmc_data_valid(bool status)
{
	s_is_nmea_rmc_data_valid = status;
}
bool get_nmea_rmc_data_valid(void)
{
	return s_is_nmea_rmc_data_valid;
}

static uint8_t g_gps_1pps_pin_flag = GPS_PPS_NONE;/*1pps接收的次数*/
/*设置1pps串口中断*/
void SetGPS1PPSPinFlag(uint8_t isEnable)
{
	g_gps_1pps_pin_flag = isEnable;
}
/*获取1pps串口中断*/
uint8_t GetGPS1PPSPinFlag(void)
{
	return g_gps_1pps_pin_flag;
}

/*设置第一次1pps时的GPS时间*/
void Set_TimePPSGPSStr(am_hal_rtc_time_t status)
{
  s_time_gps = status;
}
/*获取第一次1pps时的GPS时间*/
am_hal_rtc_time_t Get_TimePPSGPSStr(void)
{
	return s_time_gps;
}
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
static bool s_is_nav_time_gps_valid = false;//ublox 是否下载了闰秒  NAV-TIMEGPS判断是否有效下载 默认无
void set_is_nav_time_gps_valid(bool status)
{
	s_is_nav_time_gps_valid = status;
}
bool get_is_nav_time_gps_valid(void)
{
	return s_is_nav_time_gps_valid;
}
am_hal_rtc_time_t m_before_rtc_ensure,m_before_gps_ensure;
static uint8_t s_timing_and_renewspace_status = TIME_GPS_NOT_TIMING_NOT_NEW_SPACE;//授时后未开辟新的flash空间
void set_timing_and_renewspace_status(uint8_t status)
{
	s_timing_and_renewspace_status = status;
}
uint8_t get_timing_and_renewspace_status(void)
{
	return s_timing_and_renewspace_status;
}
#endif

#if defined (WATCH_HUADA_GNSS) || defined WATCH_SONY_GNSS || defined WATCH_UBLOX_MOUDULE

uint8_t g_UartRxTimeoutFlag = 0;
static uint8_t g_Ublox_Tx_Buffer[UBLOX_TX_BUFFER_SIZE];
static uint8_t g_Ublox_Rx_Buffer[UBLOX_RX_BUFFER_SIZE];



static am_hal_uart_config_t g_Ublox_UartConfig =
{
	#if defined WATCH_UBLOX_MOUDULE
	.ui32BaudRate = 9600,
	#else
	.ui32BaudRate = 115200,
	#endif
	.ui32DataBits = AM_HAL_UART_DATA_BITS_8,
	.bTwoStopBits = false,
	.ui32Parity   = AM_HAL_UART_PARITY_NONE,
	.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,
};

#endif


const am_hal_iom_config_t g_ublox_spi_sIOMConfig =
{
	.ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
	.ui32ClockFrequency = AM_HAL_IOM_400KHZ,
	.bSPHA = 0,
	.bSPOL = 0,
	.ui8WriteThreshold = 4,
	.ui8ReadThreshold = 60,
};

void drv_ublox_enable(void)
{
	if (rtc_hw_flag == 0)
	{
	xSemaphoreTake( Appolo_I2C_Semaphore, portMAX_DELAY );
	bsp_iom_enable(BSP_UBLOX_IOM,g_ublox_spi_sIOMConfig);
	}
	//DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_enable\n");
}

void drv_ublox_disable(void)
{
	if (rtc_hw_flag == 0)
	{
	bsp_iom_disable(BSP_UBLOX_IOM);
	//DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_disable\n");
	xSemaphoreGive( Appolo_I2C_Semaphore );	
	}
}

void drv_ublox_poweron(void)
{
	//Power on Ublox
	am_hal_gpio_pin_config(BSP_GPIO_UBLOX_PWR,AM_HAL_PIN_OUTPUT);
	am_hal_gpio_out_bit_set(BSP_GPIO_UBLOX_PWR);
        #ifdef WATCH_HUADA_GNSS
		set_m_is_pc(false);
		am_hal_gpio_pin_config(BSP_GPIO_UART0_TX,BSP_GPIO_CFG_UART0_TX);
		am_hal_gpio_pin_config(BSP_GPIO_UART0_RX,BSP_GPIO_CFG_UART0_RX);
	  
		
		// Power on the selected UART
		am_hal_uart_pwrctrl_enable(0);
	
		// Start the UART interface, apply the desired configuration settings, and enable the FIFOs.
		 am_hal_uart_clock_enable(0);
		
		// Disable the UART before configuring it.
		am_hal_uart_disable(0);
	
		// Configure the UART.
		am_hal_uart_config(0, &g_Ublox_UartConfig);
	
		// Configure the UART FIFO.
		 am_hal_uart_fifo_config(0, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);
	
		// Initialize the UART queues.
		am_hal_uart_init_buffered(0, g_Ublox_Rx_Buffer, UBLOX_RX_BUFFER_SIZE,
								  g_Ublox_Tx_Buffer, UBLOX_TX_BUFFER_SIZE);
		am_hal_uart_int_clear(0,0xFFFFFFFF);
		am_hal_uart_int_enable(0, AM_HAL_UART_INT_RX_TMOUT|AM_HAL_UART_INT_RX|AM_HAL_UART_INT_TXCMP);
		am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART,AM_HAL_INTERRUPT_PRIORITY(5));
		am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART); 
		am_hal_uart_enable(0);
	
		vTaskDelay(300/portTICK_PERIOD_MS);
		#if 1 
		const char cmdonbuf[] = {0xF1,0xD9 ,0x02 ,0x01 ,0x01 ,0x00 ,0x00 ,0x04 ,0x11}; //关闭log
		for(uint8_t i = 0;i < sizeof(cmdonbuf);i++)
		{
				am_hal_uart_char_transmit_buffered(0, cmdonbuf[i]); 
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#endif
        #elif defined WATCH_SONY_GNSS
		if (rtc_hw_flag == 1)
		{
		set_m_is_pc(false);
		am_hal_gpio_pin_config(BSP_GPIO_UART0_TX,BSP_GPIO_CFG_UART0_TX);
		am_hal_gpio_pin_config(BSP_GPIO_UART0_RX,BSP_GPIO_CFG_UART0_RX);
	  
		
		// Power on the selected UART
		am_hal_uart_pwrctrl_enable(0);
	
		// Start the UART interface, apply the desired configuration settings, and enable the FIFOs.
		 am_hal_uart_clock_enable(0);
		
		// Disable the UART before configuring it.
		am_hal_uart_disable(0);
	
		// Configure the UART.
		am_hal_uart_config(0, &g_Ublox_UartConfig);
	
		// Configure the UART FIFO.
		 am_hal_uart_fifo_config(0, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);
	
		// Initialize the UART queues.
		am_hal_uart_init_buffered(0, g_Ublox_Rx_Buffer, UBLOX_RX_BUFFER_SIZE,
								  g_Ublox_Tx_Buffer, UBLOX_TX_BUFFER_SIZE);
		am_hal_uart_int_clear(0,0xFFFFFFFF);
		am_hal_uart_int_enable(0, AM_HAL_UART_INT_RX_TMOUT|AM_HAL_UART_INT_RX|AM_HAL_UART_INT_TXCMP);
		am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART,AM_HAL_INTERRUPT_PRIORITY(5));
		am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART); 
		am_hal_uart_enable(0);
	
		}
        #elif defined  WATCH_UBLOX_MOUDULE
		set_m_is_pc(false);
		am_hal_gpio_pin_config(BSP_GPIO_UART0_TX,BSP_GPIO_CFG_UART0_TX);
		am_hal_gpio_pin_config(BSP_GPIO_UART0_RX,BSP_GPIO_CFG_UART0_RX);
	  
		
		// Power on the selected UART
		am_hal_uart_pwrctrl_enable(0);
	
		// Start the UART interface, apply the desired configuration settings, and enable the FIFOs.
		 am_hal_uart_clock_enable(0);
		
		// Disable the UART before configuring it.
		am_hal_uart_disable(0);
	
		// Configure the UART.
		am_hal_uart_config(0, &g_Ublox_UartConfig);
	
		// Configure the UART FIFO.
		 am_hal_uart_fifo_config(0, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);
	
		// Initialize the UART queues.
		am_hal_uart_init_buffered(0, g_Ublox_Rx_Buffer, UBLOX_RX_BUFFER_SIZE,
								  g_Ublox_Tx_Buffer, UBLOX_TX_BUFFER_SIZE);
		am_hal_uart_int_clear(0,0xFFFFFFFF);
		am_hal_uart_int_enable(0, AM_HAL_UART_INT_RX_TMOUT|AM_HAL_UART_INT_RX|AM_HAL_UART_INT_TXCMP);
		am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART,AM_HAL_INTERRUPT_PRIORITY(5));
		am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART); 
		am_hal_uart_enable(0);
       #endif
}



void drv_ublox_poweroff(void)
{
	
#if defined  WATCH_HUADA_GNSS || defined WATCH_SONY_GNSS || defined  WATCH_UBLOX_MOUDULE
		if (rtc_hw_flag == 1)
		{
		  bsp_uart_disable(0);
		}
#endif

	//Power off Ublox
	am_hal_gpio_out_bit_clear(BSP_GPIO_UBLOX_PWR);
	am_hal_gpio_pin_config(BSP_GPIO_UBLOX_PWR,AM_HAL_PIN_DISABLE);	
}


#if defined WATCH_SONY_GNSS
//工作模式
void drv_set_GNSS_mode(uint8_t mode)
{
#if 1
	//vTaskDelay(500/portTICK_PERIOD_MS);
	
	#if WATCH_GPS_MODE == 1
  	  const char buf_gps_and_bd_mode[] = {"@GNS 0x69\r\n"};//双模  0x69  GPS + BDS + QZSS
	#elif  WATCH_GPS_MODE == 2  
	  const char buf_gps_and_bd_mode[] = {"@GNS 0x2B\r\n"};//双模  0x2B  GPS+GLONASS+QZSS
	#elif  WATCH_GPS_MODE == 3
	  const char buf_gps_and_bd_mode[] = {"@GNS 0xA9\r\n"};//双模  0xA9  GPS+QZSS+Galileo
        #endif
	
	const char buf_single_gps_mode[] = {"@GNS 0x09\r\n"};//单gps
	const char buf_single_bd_mode[] = {"@GNS 0x40\r\n"};//单bd
        
	switch(mode)
	{
		case UBLOX_GN:
		 	for(uint8_t i = 0;i < sizeof(buf_gps_and_bd_mode);i++)
			{
				am_hal_uart_char_transmit_buffered(0, buf_gps_and_bd_mode[i]);	
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
			}

 	                #ifdef STORE_GPS_LOG_DATA_TEST
	                  store_log_data(buf_gps_and_bd_mode,strlen(buf_gps_and_bd_mode));///
	                #endif 
			
			break;
			
		case UBLOX_GP:
			for(uint8_t i = 0;i < sizeof(buf_single_gps_mode);i++)
			{
				am_hal_uart_char_transmit_buffered(0, buf_single_gps_mode[i]);	
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
			}

      	                #ifdef STORE_GPS_LOG_DATA_TEST
	                  store_log_data(buf_single_gps_mode,strlen(buf_single_gps_mode));///
	                #endif 
			
			break;
			
		case UBLOX_GB:
			for(uint8_t i = 0;i < sizeof(buf_single_bd_mode);i++)
			{
				am_hal_uart_char_transmit_buffered(0, buf_single_bd_mode[i]);	
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
			}
			
       	                #ifdef STORE_GPS_LOG_DATA_TEST
	                  store_log_data(buf_single_bd_mode,strlen(buf_single_bd_mode));///
	                #endif 
			
			break;
		default:
			break;
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
#endif
}

void drv_set_clodstart(void)
{
    	const char cmdbuf[] = {"@GCD\r\n"};//冷启动;//冷启动
	
	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}

	xSemaphoreTake(GPSSemaphore,1000/portTICK_PERIOD_MS);

 	#ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
	#endif  	
}

void drv_set_hotstart(void)
{
    	const char cmdbuf[] = {"@GSR\r\n"};//热启动;
	
	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}
	
	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);

 	#ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
	#endif  	
}

void drv_set_idle(void)
{
	const char cmdbuf[] = {"@GSTP\r\n"};//The CXD5603GF transfers to the Idle state
	
	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);

	#ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
	#endif  
}

void drv_set_backup_data_save(void)
{
	const char cmdbuf[] = {"@BUP\r\n"};//
	
	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
			
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);

 	#ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
	#endif  	
}

void drv_set_aep_enable(void)
{

	const char cmdbuf[] = {"@AEPS 1\r\n"}; /**/
	

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	vTaskDelay(100/portTICK_PERIOD_MS);
}

void drv_set_operation_mode(void)
{

	const char cmdbuf[] = {"@GSOP 1 1000 0\r\n"}; /* */
	

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);

	#ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
	#endif  	
}

void drv_set_positioning_algo(void)
{

	const char cmdbuf[] = {"@GUSE 0x01\r\n"}; /* */
	

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);

      #ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
      #endif		
}

//输入当前时间
void drv_set_utc_time_set(void)
{
	rtc_time_t utc_time;
	uint8_t len;
	char cmdbuf[40] = {0};
	
	utc_time = RtcToUtc(3600*8);

		
	sprintf(cmdbuf,"@GTIM %d %d %d %d %d %d\r\n",utc_time.Year+2000,utc_time.Month,utc_time.Day,utc_time.Hour,utc_time.Minute,utc_time.Second); /* */

	len = strlen(cmdbuf);

	for(uint8_t i = 0;i < len;i++)
	{
		
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}

        xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
        
       #ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
       #endif	  
}

void drv_set_NMEA_RMC(void)
{
      #ifdef STORE_GPS_LOG_DATA_TEST
        const char cmdbuf[] = {"@BSSL 0x2EF\r\n"}; /* NMEA command, 输出RMC,GSA,GSV,频率1 */
      #else
        const char cmdbuf[] = {"@BSSL 0x20\r\n"}; /* NMEA command, 输出RMC，频率1 */
      #endif	

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
	
      #ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
      #endif		
}

void drv_set_NMEA_all(void)
{
     #ifdef STORE_GPS_LOG_DATA_TEST
       const char cmdbuf[] = {"@BSSL 0x2EF\r\n"}; /* NMEA command, 输出RMC,GSA,GSV,频率1 */
     #else
       const char cmdbuf[] = {"@BSSL 0xEF\r\n"}; /* NMEA command, 输出RMC GGA,GSA，频率1 */
     #endif  

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
 	
      #ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
      #endif	
}

void drv_set_NMEA_RMC_GGA_GSA(void)
{
     #ifdef STORE_GPS_LOG_DATA_TEST
       const char cmdbuf[] = {"@BSSL 0x2EF\r\n"}; /* NMEA command, 输出RMC,GSA,GSV,频率1 */
     #else	
       const char cmdbuf[] = {"@BSSL 0x25\r\n"}; /* NMEA command, 输出RMC GGA,GSA，频率1 */
     #endif	

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}

	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);
	
     #ifdef STORE_GPS_LOG_DATA_TEST
	store_log_data(cmdbuf,strlen(cmdbuf));///
     #endif	  
}

void drv_set_NMEA_RMC_GSA(void)
{
      #ifdef STORE_GPS_LOG_DATA_TEST
        const char cmdbuf[] = {"@BSSL 0x2EF\r\n"}; /* NMEA command, 输出RMC,GSA,GSV,频率1 */
      #else
	const char cmdbuf[] = {"@BSSL 0x24\r\n"}; /* NMEA command, 输出RMC,GSA，频率1 */
      #endif	

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);

	#ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
	#endif  	
}

void drv_set_NMEA_RMC_GSA_GSV(void)
{
        #ifdef STORE_GPS_LOG_DATA_TEST
          const char cmdbuf[] = {"@BSSL 0x2EF\r\n"}; /* NMEA command, 输出RMC,GSA,GSV,频率1 */
        #else
	  const char cmdbuf[] = {"@BSSL 0x2C\r\n"}; /* NMEA command, 输出RMC,GSA,GSV,频率1 */
	#endif  
	
	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	xSemaphoreTake(GPSSemaphore,50/portTICK_PERIOD_MS);

	#ifdef STORE_GPS_LOG_DATA_TEST
	  store_log_data(cmdbuf,strlen(cmdbuf));///
	#endif  	
}


#endif


/*
函数说明:1pps中断处理
1.可授时的时候 第一次接收到1pps,记录下gps时间,
2.当第二次接收到1pps的时候,记录当时的RTC时间,然后把GPS时间+1s赋给RTC,完成授时
*/
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
static void ublox_1ppx_handler(void)
{
	if( am_hal_gpio_input_bit_read(BSP_GPIO_1PPS) )//gps_1pps
	{
		am_hal_rtc_time_t m_pps_rtc_time;
		uint32_t m_get_rtc_status = am_hal_rtc_time_get(&m_pps_rtc_time);
		DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ublox_1ppx_handler set yes!m_get_rtc_status=%d,%d-%02d-%02d %02d:%02d:%02d.%02d0,s_continuous_pps_count=%d\n"
				,m_get_rtc_status,m_pps_rtc_time.ui32Year+2000,m_pps_rtc_time.ui32Month,m_pps_rtc_time.ui32DayOfMonth
				,m_pps_rtc_time.ui32Hour,m_pps_rtc_time.ui32Minute,m_pps_rtc_time.ui32Second,m_pps_rtc_time.ui32Hundredths,s_continuous_pps_count);
		if(g_gps_1pps_pin_flag == GPS_PPS_NONE && (GetGPSScreenStatus() == true))
		{
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_PPS_NONE!\n");
			m_gps_rtc_diff1 = 0;
			m_gps_rtc_diff2 = 0;
			s_continuous_pps_count = 0;
			g_gps_1pps_pin_flag = GPS_FILTER_INVALID;
		}
		else if(g_gps_1pps_pin_flag == GPS_FILTER_INVALID)
		{
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_FILTER_INVALID!\n");
		}
		else if(g_gps_1pps_pin_flag == GPS_FILTER_INVALID_TWO)
		{
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_FILTER_INVALID_TWO!\n");
			g_gps_1pps_pin_flag = GPS_FILTER_INVALID_THIRD;
		}
		else if(g_gps_1pps_pin_flag == GPS_FILTER_INVALID_THIRD)
		{
			s_continuous_pps_count ++;
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_FILTER_INVALID_THIRD!s_continuous_pps_count=%d\n",s_continuous_pps_count);
			g_gps_1pps_pin_flag = GPS_PPS_FIRST;
		}
		else if(g_gps_1pps_pin_flag == GPS_PPS_SECOND && (GetGPSScreenStatus() == true) && s_continuous_pps_count == 1
					&& get_nmea_rmc_data_valid() == true)
		{//校准期间如果闰秒已下载 授时才有效
			s_continuous_pps_count ++;
			s_time_gps.ui32Hundredths = 0;//gps的毫秒置0
			s_time_gps.ui32Second += 1;/*获取第二次1pps时的GPS时间+1s*/
			GetAddTime(&s_time_gps);
			
			m_gps_rtc_diff1 = DiffTimeMs(s_time_gps,m_pps_rtc_time);
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_PPS_SECOND!m_gps_rtc_diff1=%d\n",m_gps_rtc_diff1);
			if( ScreenState != DISPLAY_SCREEN_TEST_REAL_DIFF_TIME )
			{//在非测试比对时间界面授时
				m_before_rtc_ensure = m_pps_rtc_time;
				m_before_gps_ensure = s_time_gps;
			}
			g_gps_1pps_pin_flag = GPS_PPS_THIRD;			//并把GPS时间+1s赋给RTC RTC_time_Gps_Timing
#if defined(DEBUG_TASK_GPS)
			am_hal_rtc_time_t m_second_gps_time = Get_TimeGPSStr();
			am_hal_rtc_time_t m_second_rtc_time = Get_TimeRTCStr();
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: GPS_PPS_THIRD -->GPSTime %d-%02d-%02d %d:%02d:%02d.%02d0\n"
			 ,m_second_gps_time.ui32Year,m_second_gps_time.ui32Month,m_second_gps_time.ui32DayOfMonth
			 ,m_second_gps_time.ui32Hour,m_second_gps_time.ui32Minute,m_second_gps_time.ui32Second,m_second_gps_time.ui32Hundredths);
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: GPS_PPS_THIRD -->RTCTime before GPSTime %d-%02d-%02d %d:%02d:%02d.%02d0\n"
			 ,m_second_rtc_time.ui32Year,m_second_rtc_time.ui32Month,m_second_rtc_time.ui32DayOfMonth
			 ,m_second_rtc_time.ui32Hour,m_second_rtc_time.ui32Minute,m_second_rtc_time.ui32Second,m_second_rtc_time.ui32Hundredths);
#endif
		}
		else if(g_gps_1pps_pin_flag == GPS_PPS_FOURTH && (GetGPSScreenStatus() == true) && get_nmea_rmc_data_valid() == true && s_continuous_pps_count == 2)
		{//校准期间如果闰秒已下载 授时才有效
			s_continuous_pps_count = 0;
			s_time_gps.ui32Hundredths = 0;//gps的毫秒置0
			s_time_gps.ui32Second += 1;/*获取第二次1pps时的GPS时间+1s*/
			GetAddTime(&s_time_gps);
		
			int64_t gps_rtc_value = 0;
			if(get_open_difftime_status() == STATUS_OPEN_DIFFITME && ScreenState == DISPLAY_SCREEN_TEST_REAL_DIFF_TIME)
			{//测量界面不授时
				m_gps_rtc_diff2 = DiffTimeMs(s_time_gps,m_pps_rtc_time);
				DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_PPS_FOURTH!m_gps_rtc_diff2=%d\n",m_gps_rtc_diff2);
				gps_rtc_value = (m_gps_rtc_diff2 - m_gps_rtc_diff1);
			}
			else
			{//授时情况
				m_gps_rtc_diff2 = DiffTimeMs(s_time_gps,m_before_gps_ensure);
				DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_PPS_FOURTH!m_gps_rtc_diff2=%d\n",m_gps_rtc_diff2);
				gps_rtc_value = m_gps_rtc_diff2;
			}
			
			if( gps_rtc_value <= 1100 || gps_rtc_value > -1100)
			{//两次相差1100ms内认为解析正常 可以授时或比较
				DRV_UBLOX_LOG_PRINTF("[drv_ublox]: GPS_PPS_FIFTH--> success!gps_rtc_value=%d\n",(int32_t)gps_rtc_value);

				if(get_open_difftime_status() == STATUS_OPEN_DIFFITME && ScreenState == DISPLAY_SCREEN_TEST_REAL_DIFF_TIME)
				{//特定测试界面比对时间用  rtc时间与GPS时间相比
					DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_PPS_FOURTH!--->STATUS_OPEN_DIFFITME m_get_rtc_status=%d\n",m_get_rtc_status);
					DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_PPS_FOURTH!%d-%02d-%02d %02d:%02d:%02d.%02d0\n"
						,s_time_gps.ui32Year+2000,s_time_gps.ui32Month,s_time_gps.ui32DayOfMonth
						,s_time_gps.ui32Hour,s_time_gps.ui32Minute,s_time_gps.ui32Second,s_time_gps.ui32Hundredths);
					get_gps_rtc_ms_value(s_time_gps,m_pps_rtc_time);
					set_open_difftime_status(STATUS_OPEN_DIFFITME_COMPLETE);
				}
				else
				{
					if(is_set_real_time_to_rtc() == true)
					{
						RTC_time = s_time_gps; /*获取GPS准确时间并设到RTC中*/
						am_hal_rtc_time_set(&RTC_time);//设置rtc时间
						set_timing_and_renewspace_status(TIME_GPS_TIMING_NOT_NEW_SPACE);
						Set_TimeGPSStr(s_time_gps);
						Set_TimeRTCStr(m_pps_rtc_time); /*授时前rtc时间保存*/
						DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_PPS_FOURTH! -->RTC_time set success!%d-%02d-%02d %02d:%02d:%02d.%02d0\n"
							,RTC_time.ui32Year+2000,RTC_time.ui32Month,RTC_time.ui32DayOfMonth
							,RTC_time.ui32Hour,RTC_time.ui32Minute,RTC_time.ui32Second,RTC_time.ui32Hundredths);
					}
					else
					{
						DRV_UBLOX_LOG_PRINTF("[drv_ublox]: status is GPS_PPS_FOURTH! -->RTC_time set fail!!!\n");
					}
				}
				g_gps_1pps_pin_flag = GPS_PPS_FIFTH;//校验时间准确
			}
			else
			{
				if(get_open_difftime_status() == STATUS_OPEN_DIFFITME && ScreenState == DISPLAY_SCREEN_TEST_REAL_DIFF_TIME)
				{//时间校验不相等时候 计数
					uint32_t m_value = get_s_reset_gpsrmc_count();
					extern void set_s_reset_gpsrmc_count(uint32_t value);
					set_s_reset_gpsrmc_count(m_value++);
				}
				s_continuous_pps_count = 0;
				g_gps_1pps_pin_flag = GPS_PPS_NONE;//中间间断重置状态
				DRV_UBLOX_LOG_PRINTF("[drv_ublox]: GPS_PPS_FOURTH-->fail-status is not TIME_GETTED!gps_rtc_value=%d\n",(int32_t)gps_rtc_value);
			}
		}
		else
		{
			s_continuous_pps_count = 0;
			g_gps_1pps_pin_flag = GPS_PPS_NONE;//中间间断重置状态
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:status is not TIME_GETTED!s_continuous_pps_count=%d\n",s_continuous_pps_count);
		}
	}
	else
	{
		DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ublox_1ppx_handler set  no!\n");
	}
}
#endif
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
static void com_apollo2_gpio_int_init(uint32_t pin, uint32_t polarity)
{
	am_hal_gpio_pin_config(pin,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_polarity_bit_set(pin,polarity);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(pin));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(pin));	

}
/*1pps中断脚初始化配置*/
void drv_ublox_ppx_init(void)
{
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_ppx_init\n");
	am_hal_gpio_int_register(BSP_GPIO_1PPS, ublox_1ppx_handler);
	com_apollo2_gpio_int_init(BSP_GPIO_1PPS,AM_HAL_GPIO_RISING);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(BSP_GPIO_1PPS));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(BSP_GPIO_1PPS));
	g_gps_1pps_pin_flag = GPS_PPS_NONE;
	set_nmea_rmc_data_valid(false);
	s_continuous_pps_count = 0;
	set_timing_and_renewspace_status(TIME_GPS_NOT_TIMING_NOT_NEW_SPACE);
	vTaskDelay(20/portTICK_PERIOD_MS);
}
#endif
uint8_t drv_ublox_read(void)
{
	am_hal_iom_buffer(1) sData = {0};

	if (am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,1,AM_HAL_IOM_RAW)!=AM_HAL_IOM_SUCCESS)
    {
        return 0xFF;
    }    
	
	return sData.bytes[0];
}

void drv_ublox_set_platform( uint8_t PlatVal)
{
	
}

void drv_ublox_status_set(uint8_t status)
{
	GPSStatus = (GPSState_t)(status);
}

uint8_t drv_ublox_status_get(void)
{
	return GPSStatus;
}
void drv_ublox_GGA(bool status)
{
#ifdef WATCH_HUADA_GNSS
	
	if(status != false)
	{
		const char cmdonbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x00 ,0x01 ,0xFB ,0x10 }; //打开
		for(uint8_t i = 0;i < sizeof(cmdonbuf);i++)
		{
			am_hal_uart_char_transmit_buffered(0, cmdonbuf[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
	}
	else
	{
		const char cmdoffbuf[] = {0xF1, 0xD9, 0x06,0x01, 0x03, 0x00, 0xF0, 0x00, 0x00, 0xFA, 0x0F}; //关闭
		for(uint8_t i = 0;i < sizeof(cmdoffbuf);i++)
		{
			am_hal_uart_char_transmit_buffered(0, cmdoffbuf[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
	}
		
#else

	am_hal_iom_buffer(11) sData = {0};
	
	if(status != false)
	{
		//打开GGA
		
	}
	else
	{
		//关闭GGA
		sData.bytes[0] = 0xB5;
		sData.bytes[1] = 0x62;
		sData.bytes[2] = 0x06;
		sData.bytes[3] = 0x01;
		sData.bytes[4] = 0x03;
		sData.bytes[5] = 0x00;
		sData.bytes[6] = 0xF0;
		sData.bytes[7] = 0x00;
		sData.bytes[8] = 0x00;
		sData.bytes[9] = 0xFA;
		sData.bytes[10] = 0x0F;
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < 11;i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,11,AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
#endif
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_GGA status=%d\n",status);

	vTaskDelay(20/portTICK_PERIOD_MS);
}

void drv_ublox_GSV(bool status)
{
#ifdef WATCH_HUADA_GNSS
		
		if(status != false)
		{
			const char cmdonbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x04 ,0x01 ,0xFF ,0x18}; //打开
			for(uint8_t i = 0;i < sizeof(cmdonbuf);i++)
			{
				am_hal_uart_char_transmit_buffered(0, cmdonbuf[i]); 
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
			}
		}
		else
		{
			const char cmdoffbuf[] = {0xF1,0xD9,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x04 ,0x00 ,0xFE ,0x17}; //关闭
			for(uint8_t i = 0;i < sizeof(cmdoffbuf);i++)
			{
				am_hal_uart_char_transmit_buffered(0, cmdoffbuf[i]);	
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
			}
		}
			
#else

	am_hal_iom_buffer(11) sData = {0};
	
	if(status != false)
	{
		//打开GSV
		
	}
	else
	{
		//关闭GSV
		sData.bytes[0] = 0xB5;
		sData.bytes[1] = 0x62;
		sData.bytes[2] = 0x06;
		sData.bytes[3] = 0x01;
		sData.bytes[4] = 0x03;
		sData.bytes[5] = 0x00;
		sData.bytes[6] = 0xF0;
		sData.bytes[7] = 0x03;
		sData.bytes[8] = 0x00;
		sData.bytes[9] = 0xFD;
		sData.bytes[10] = 0x15;
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i <11;i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,11,AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_GSV status=%d\n",status);
#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
}

void drv_ublox_GLL(bool status)
{
#ifdef WATCH_HUADA_GNSS
			
			if(status != false)
			{
				const char cmdonbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x01 ,0x01 ,0xFC ,0x12}; //打开
				for(uint8_t i = 0;i < sizeof(cmdonbuf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, cmdonbuf[i]); 
			
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
			}
			else
			{
				const char cmdoffbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x01 ,0x00 ,0xFB ,0x11 }; //关闭
				for(uint8_t i = 0;i < sizeof(cmdoffbuf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, cmdoffbuf[i]);	
			
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
			}
				
#else

	am_hal_iom_buffer(11) sData = {0};
	
	if(status != false)
	{
		//打开GLL
		
	}
	else
	{
		//关闭GLL
		sData.bytes[0] = 0xB5;
		sData.bytes[1] = 0x62;
		sData.bytes[2] = 0x06;
		sData.bytes[3] = 0x01;
		sData.bytes[4] = 0x03;
		sData.bytes[5] = 0x00;
		sData.bytes[6] = 0xF0;
		sData.bytes[7] = 0x01;
		sData.bytes[8] = 0x00;
		sData.bytes[9] = 0xFB;
		sData.bytes[10] = 0x11;

		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i <11;i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,11,AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_GLL status=%d\n",status);
#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
}

void drv_ublox_GSA(bool status)
{

#ifdef WATCH_HUADA_GNSS
			
			if(status != false)
			{
				const char cmdonbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x02 ,0x01 ,0xFD ,0x14}; //打开
				for(uint8_t i = 0;i < sizeof(cmdonbuf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, cmdonbuf[i]); 
			
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
			}
			else
			{
				const char cmdoffbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x02 ,0x00 ,0xFC ,0x13}; //关闭
				for(uint8_t i = 0;i < sizeof(cmdoffbuf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, cmdoffbuf[i]);	
			
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
			}
				
#else

	am_hal_iom_buffer(11) sData = {0};
	
	if(status != false)
	{
		//打开GSA
		
	}
	else
	{
		//关闭GSA
		sData.bytes[0] = 0xB5;
		sData.bytes[1] = 0x62;
		sData.bytes[2] = 0x06;
		sData.bytes[3] = 0x01;
		sData.bytes[4] = 0x03;
		sData.bytes[5] = 0x00;
		sData.bytes[6] = 0xF0;
		sData.bytes[7] = 0x02;
		sData.bytes[8] = 0x00;
		sData.bytes[9] = 0xFC;
		sData.bytes[10] = 0x13;
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < 11;i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,11,AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_GSA status=%d\n",status);
#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
}

void drv_ublox_VTG(bool status)
{
#ifdef WATCH_HUADA_GNSS
				
			if(status != false)
			{
				const char cmdonbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x06 ,0x01 ,0x01 ,0x1C}; //打开
				for(uint8_t i = 0;i < sizeof(cmdonbuf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, cmdonbuf[i]); 
			
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
			}
			else
			{
				const char cmdoffbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x06 ,0x00 ,0x00 ,0x1B }; //关闭
				for(uint8_t i = 0;i < sizeof(cmdoffbuf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, cmdoffbuf[i]);	
			
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
			}
					
#else

	am_hal_iom_buffer(11) sData = {0};
	
	if(status != false)
	{
		//打开VTG
		
	}
	else
	{
		//关闭VTG
		sData.bytes[0] = 0xB5;
		sData.bytes[1] = 0x62;
		sData.bytes[2] = 0x06;
		sData.bytes[3] = 0x01;
		sData.bytes[4] = 0x03;
		sData.bytes[5] = 0x00;
		sData.bytes[6] = 0xF0;
		sData.bytes[7] = 0x05;
		sData.bytes[8] = 0x00;
		sData.bytes[9] = 0xFF;
		sData.bytes[10] = 0x19;
		
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < 11;i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,11,AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_VTG status=%d\n",status);
#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
}

void drv_ublox_RMC(bool status)
{

#ifdef WATCH_HUADA_GNSS
					
			if(status != false)
			{
				const char cmdonbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x05 ,0x01 ,0x00 ,0x1A}; //打开
				for(uint8_t i = 0;i < sizeof(cmdonbuf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, cmdonbuf[i]); 
				
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
			}
			else
			{
				const char cmdoffbuf[] = {0xF1 ,0xD9 ,0x06 ,0x01 ,0x03 ,0x00 ,0xF0 ,0x05 ,0x00 ,0xFF ,0x19 }; //关闭
				for(uint8_t i = 0;i < sizeof(cmdoffbuf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, cmdoffbuf[i]);	
				
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
			}
						
#else

	am_hal_iom_buffer(11) sData = {0};

	if(status != false)
	{
//		if(((IsSportMode(ScreenState) == true) || (ScreenState == DISPLAY_SCREEN_SPORT_FIRST) 
//			|| (ScreenState == DISPLAY_SCREEN_SPORT)) && (SetValue.Navigation_Mode == NORMAL))
//		{
//		
#if 0
        //打开RMC,5s
		sData.bytes[0] = 0xB5;
		sData.bytes[1] = 0x62;
		sData.bytes[2] = 0x06;
		sData.bytes[3] = 0x01;
		sData.bytes[4] = 0x03;
		sData.bytes[5] = 0x00;
		sData.bytes[6] = 0xF0;
		sData.bytes[7] = 0x04;
		sData.bytes[8] = 0x01;
		sData.bytes[9] = 0xFF;
		sData.bytes[10] = 0x18;
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,11,AM_HAL_IOM_RAW);
		drv_ublox_disable();
#endif
//		}		
	}
	else
	{
		//关闭RMC
		sData.bytes[0] = 0xB5;
		sData.bytes[1] = 0x62;
		sData.bytes[2] = 0x06;
		sData.bytes[3] = 0x01;
		sData.bytes[4] = 0x03;
		sData.bytes[5] = 0x00;
		sData.bytes[6] = 0xF0;
		sData.bytes[7] = 0x04;
	
		sData.bytes[8] = 0x00;
		sData.bytes[9] = 0xFE;
		sData.bytes[10] = 0x17;

		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < 11;i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,11,AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	

	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_RMC status=%d,%d\n",status,sizeof(sData));
#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
}

//设置ublox搜星系统
void drv_ublox_GNSS(uint8_t mode)
{
#ifdef WATCH_HUADA_GNSS
	const char buf_gps_and_bd_mode[] = {0xF1 ,0xD9 ,0x06 ,0x0C ,0x04 ,0x00 ,0x25 ,0x00 ,0x00 ,0x00 ,0x3B ,0x30};//双模
	const char buf_single_gps_mode[] = {0xF1 ,0xD9 ,0x06 ,0x0C ,0x04 ,0x00 ,0x21 ,0x00 ,0x00 ,0x00 ,0x37 ,0x20};//单gps
	const char buf_single_bd_mode[] = {0xF1 ,0xD9 ,0x06 ,0x0C ,0x04 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x1A ,0xAC};//单bd
	switch(mode)
	{
		case UBLOX_GN:
		 	for(uint8_t i = 0;i < sizeof(buf_gps_and_bd_mode);i++)
			{
				am_hal_uart_char_transmit_buffered(0, buf_gps_and_bd_mode[i]);	
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
			}
			break;
		case UBLOX_GP:
			for(uint8_t i = 0;i < sizeof(buf_single_gps_mode);i++)
			{
				am_hal_uart_char_transmit_buffered(0, buf_single_gps_mode[i]);	
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
			}
			break;
		case UBLOX_GB:
			for(uint8_t i = 0;i < sizeof(buf_single_bd_mode);i++)
			{
				am_hal_uart_char_transmit_buffered(0, buf_single_bd_mode[i]);	
		
				while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
			}
			break;
		default:
			break;
	}
#else
	//NMEA Version 4.1
	const uint8_t buf1[] = {0xB5,0x62,0x06,0x17,0x14,0x00,0x00,0x41,0x00,0x02,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x75,0x57};
	//GPS+BD
	const uint8_t buf2[] = {0xB5,0x62,0x06,0x3E,0x2C,0x00,0x00,0x00,0x20,0x05,
							0x00,0x08,0x10,0x00,0x01,0x00,0x01,0x01,
							0x01,0x01,0x03,0x00,0x00,0x00,0x01,0x01,
							0x03,0x08,0x10,0x00,0x01,0x00,0x01,0x01,
							0x05,0x00,0x03,0x00,0x00,0x00,0x01,0x01,
							0x06,0x08,0x0E,0x00,0x00,0x00,0x01,0x01,0xFD,0x25};
	//GPS
	const uint8_t buf3[] = {0xB5,0x62,0x06,0x3E,0x2C,0x00,0x00,0x00,0x20,0x05,
							0x00,0x08,0x10,0x00,0x01,0x00,0x01,0x01,
							0x01,0x01,0x03,0x00,0x00,0x00,0x01,0x01,
							0x03,0x08,0x10,0x00,0x00,0x00,0x01,0x01,
							0x05,0x00,0x03,0x00,0x00,0x00,0x01,0x01,
							0x06,0x08,0x0E,0x00,0x00,0x00,0x01,0x01,0xFC,0x11};
	
	//BD
	const uint8_t buf4[] = {0xB5,0x62,0x06,0x3E,0x3C,0x00,0x00,0x00,0x20,0x07,
	                        0x00,0x08,0x10,0x00,0x00,0x00,0x01,0x01,0x01,0x01,
                        	0x03,0x00,0x00,0x00,0x01,0x01,0x02,0x04,0x08,0x00,
                    	    0x00,0x00,0x01,0x01,0x03,0x08,0x10,0x00,0x01,0x00,
                          0x01,0x01,0x04,0x00,0x08,0x00,0x00,0x00,0x01,0x01,
                          0x05,0x00,0x03,0x00,0x00,0x00,0x01,0x01,0x06,0x08,
                  	      0x0E,0x00,0x00,0x00,0x01,0x01,0x2C,0x35,};
	am_hal_iom_buffer(70) sData;
	uint8_t i;
							
	//NMEA Version 设置为4.1						 
	memset(sData.bytes,0,70);
	for(i=0;i<(sizeof(buf1));i++)
	{
		sData.bytes[i] = buf1[i];
	}

	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < sizeof(buf1);i++)
	{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else 
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf1)),AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
	if(mode == UBLOX_GP)
	{
		//单GPS模式
		memset(sData.bytes,0,70);
		for(i=0;i<(sizeof(buf3));i++)
		{
			sData.bytes[i] = buf3[i];
		}
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf3);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf3)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
		vTaskDelay(40/portTICK_PERIOD_MS);
	}
	else if(mode == UBLOX_GB)
	{
		//单北斗模式
		memset(sData.bytes,0,70);
		for(i=0;i<(sizeof(buf4));i++)
		{
			sData.bytes[i] = buf4[i];
		}
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf4);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf4)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
		vTaskDelay(20/portTICK_PERIOD_MS);
	}
	else if(mode == UBLOX_GN)
	{
		//GPS+BD
		memset(sData.bytes,0,70);
		for(i=0;i<(sizeof(buf2));i++)
		{
			sData.bytes[i] = buf2[i];
		}

		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf2);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf2)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
		vTaskDelay(20/portTICK_PERIOD_MS);
	}
#endif
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_GNSS mode=%d\n",mode);
}

void drv_ublox_RXM(uint8_t mode)
{
	am_hal_iom_buffer(10) sData = {0};
	sData.bytes[0] = 0xB5;
	sData.bytes[1] = 0x62;
	sData.bytes[2] = 0x06;
	sData.bytes[3] = 0x11;
	sData.bytes[4] = 0x02;
	sData.bytes[5] = 0x00;
	sData.bytes[6] = 0x08;
	
	if(mode == UBLOX_CONTINUOUS)
	{
		//continuous mode
		sData.bytes[7] = 0x00;
		sData.bytes[8] = 0x21;
		sData.bytes[9] = 0x91;
	}
	else if(mode == UBLOX_POWERSAVE)
	{
		//power save mode
		sData.bytes[7] = 0x01;
		sData.bytes[8] = 0x22;
		sData.bytes[9] = 0x92;
	}
	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < 10;i++)
	{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,10,AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_RXM mode=%d\n",mode);
	vTaskDelay(20/portTICK_PERIOD_MS);
	
}

void drv_ublox_RXM_SAVE(uint8_t mode)
{
#ifdef WATCH_HUADA_GNSS
#else
    uint8_t i;
	//UBLOX_CONTINUOUS
	const uint8_t buf1[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00,0x08, 0x00, 0x21, 0x91, 0xB5, 0x62, 0x06, 0x09, 
	            0x0D, 0x00,0x00,0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1b, 0xA9};
	//UBLOX_POWERSAVE
    const uint8_t buf2[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00,0x08, 0x01, 0x22, 0x92, 0xB5, 0x62, 0x06, 0x09, 
                    0x0D, 0x00,0x00,0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1b, 0xA9};

	am_hal_iom_buffer(31) sData = {0};

	
	if(mode == UBLOX_CONTINUOUS)
	{			
		memset(sData.bytes,0,31);
		for(i=0;i<(sizeof(buf1));i++)
		{
			sData.bytes[i] = buf1[i];
		}
    }
	else if(mode == UBLOX_POWERSAVE)
	{
		//power save mode
		memset(sData.bytes,0,31);
		for(i=0;i<(sizeof(buf2));i++)
		{
			sData.bytes[i] = buf2[i];
		};
	}
	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < 31;i++)
	{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,31,AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_RXM mode=%d\n",mode);
	vTaskDelay(20/portTICK_PERIOD_MS);
#endif
	
}
void drv_ublox_REVERT(void)
{
#ifdef WATCH_HUADA_GNSS
#else

    uint8_t i;
	//恢复默认配置
	const uint8_t buf[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 
	            0x00, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x19, 0x98};
	
	am_hal_iom_buffer(21) sData = {0};

	for(i=0;i<(sizeof(buf));i++)
	{
		sData.bytes[i] = buf[i];
	}
	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < sizeof(buf);i++)
	{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,21,AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	
	vTaskDelay(30/portTICK_PERIOD_MS);
#endif
	
}

void drv_ublox_PWR(void)
{
    uint8_t i;
	//stop
	//const uint8_t buf[] = {0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x4F, 0x54, 0x53, 0xAC, 0x85};
	//backup
	const uint8_t buf[] = {0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x4B, 0x43, 0x42, 0x86, 0x46};
	am_hal_iom_buffer(16) sData = {0};

	for(i=0;i<(sizeof(buf));i++)
	{
		sData.bytes[i] = buf[i];
	}

	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < 16;i++)
	{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,16,AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	
	vTaskDelay(20/portTICK_PERIOD_MS);
	
}

void drv_ublox_PM2(void)
{
	//Cyclic tracking operation 5s
	const uint8_t buf[] = {0xB5, 0x62, 0x06, 0x3B, 0x2C, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x90, 0x42, 0x01,
		                 0x88, 0x13, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
							0x00, 0x00, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x4F, 0xC1, 0x03, 0x00, 0x87, 0x02,
							0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x64, 0x40, 0x01, 0x00, 0x88, 0x2E};
	am_hal_iom_buffer(52) sData;
	uint8_t i;
													 
	memset(sData.bytes,0,sizeof(52));
	for(i=0;i<(sizeof(buf));i++)
	{
		sData.bytes[i] = buf[i];
	}
	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < 52;i++)
	{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf)),AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
}
//RESET RTC TIME
void drv_ublox_reset_rtc(void)
{
#ifdef WATCH_HUADA_GNSS
#else

	uint8_t i;
	am_hal_iom_buffer(12) sData = {0};
	const uint8_t buf[] = {0xB5,0x62,0x06,0x04,0x04,0x00,0x00,0x01,0x02,0x00,0x11,0x6B};
	
	for(i=0;i<(sizeof(buf));i++)
	{
		sData.bytes[i] = buf[i];
	}
	
	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < 12;i++)
	{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf)),AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_reset_rtc !\n");
	vTaskDelay(80/portTICK_PERIOD_MS);
#endif
}

//nav_timegps
void drv_ublox_nav_timegps(bool flag)
{
	uint8_t i;
	am_hal_iom_buffer(12) sData = {0};
	const uint8_t buf1[] = {0xB5,0x62,0x06,0x01,0x03,0x00,0x01,0x20,0x01,0x2C,0x83};
	const uint8_t buf2[] = {0xB5,0x62,0x06,0x01,0x03,0x00,0x01,0x20,0x00,0x2B,0x82};
	if(flag)
	{
		for(i=0;i<(sizeof(buf1));i++)
		{
			sData.bytes[i] = buf1[i];
		}
		
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf1);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf1)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	else
	{
		for(i=0;i<(sizeof(buf2));i++)
		{
			sData.bytes[i] = buf2[i];
		}

		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i <sizeof(buf2);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf2)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}

	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_nav_timegps !flag=%d\n",flag);
	vTaskDelay(20/portTICK_PERIOD_MS);
}

//Hot start
void drv_ublox_hotstart(void)
{
#ifdef WATCH_HUADA_GNSS
	const char cmdbuf[] = {0xF1 ,0xD9 ,0x06 ,0x40 ,0x01 ,0x00 ,0x03 ,0x4A ,0x24};//热启动

	for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
		
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));	
	}
#else
	uint8_t i;
	am_hal_iom_buffer(12) sData = {0};
	const uint8_t buf[] = {0xB5,0x62,0x06,0x04,0x04,0x00,0x00,0x00,0x02,0x00,0x10,0x68};// Hot start Controlled Software reset (GNSS only)
	
	for(i=0;i<(sizeof(buf));i++)
	{
		sData.bytes[i] = buf[i];
	}
	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < 12;i++)
	{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf)),AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
#endif
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_hotstart !\n");
	vTaskDelay(20/portTICK_PERIOD_MS);
}


//cold start
void drv_ublox_clodstart(void)
{
#ifdef WATCH_HUADA_GNSS
		const char cmdbuf[] = {0xF1 ,0xD9 ,0x06 ,0x40 ,0x01 ,0x00 ,0x01 ,0x48 ,0x22};//冷启动
	
		for(uint8_t i = 0;i < sizeof(cmdbuf);i++)
		{
			am_hal_uart_char_transmit_buffered(0, cmdbuf[i]);	
			
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL)); 
		}
		
#else

	uint8_t i;
	am_hal_iom_buffer(12) sData = {0};
	const uint8_t buf[] = {0xB5,0x62,0x06,0x04,0x04,0x00,0xFF,0xB9,0x02,0x00,0xC8,0x8F};// cold start Controlled Software reset (GNSS only)
	
	for(i=0;i<(sizeof(buf));i++)
	{
		sData.bytes[i] = buf[i];
	}

	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < sizeof(buf);i++)
	{
		am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf)),AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_clodstart !\n");
#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
}


void drv_ublox_ZDA(bool status)
{
	am_hal_iom_buffer(11) sData = {0};
	
	if(status != false)
	{
		//打开ZDA
		sData.bytes[0] = 0xB5;
		sData.bytes[1] = 0x62;
		sData.bytes[2] = 0x06;
		sData.bytes[3] = 0x01;
		sData.bytes[4] = 0x03;
		sData.bytes[5] = 0x00;
		sData.bytes[6] = 0xF0;
		sData.bytes[7] = 0x08;
		sData.bytes[8] = 0x01;
		sData.bytes[9] = 0x03;
		sData.bytes[10] = 0x20;
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < 11;i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,11,AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	else
	{
		//关闭ZDA
	}
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_ZDA status=%d!\n",status);
	vTaskDelay(20/portTICK_PERIOD_MS);
}
/*Configuring time pulse with UBX-CFG-TP5 1pps
mode 1:The 1 Hz output is maintained whether or not the receiver is locked to
       UTC time.The alignment to TOW can only be maintained when GPS time is locked.
mode 2:The following example shows a 10 MHz TP signal generated on the TIMEPULSE2 output
       when the receiver is locked to UTC time. Without the lock to GPS time no frequency is output
mode 3:close 1pps
*/
void drv_ublox_TP5(uint8_t mode)
{
	uint8_t i = 0;
	am_hal_iom_buffer(40) sData = {0};
//	const uint8_t buf1[] = {0xB5,0x62,0x06,0x31,0x20,0x00,0x00,0x01,0x00,0x00,
//	                        0x00,0x00,0x00,0x00,0x40,0x42,0x0F,0x00,0x40,0x42,
//                       	  0x0F,0x00,0xA0,0x86,0x01,0x00,0x00,0x00,0x00,0x00, 
//	                        0x00,0x00,0x00,0x00,0x73,0x00,0x00,0x00,0x14,0xCA};// For UTC-time
#if defined(WATCH_SINGLE_BEIDOU)
	const uint8_t buf1[] = {0xB5,0x62,0x06,0x31,0x20,0x00,0x00,0x01,0x00,0x00,
                       	  0x32,0x00,0x00,0x00,0x40,0x42,0x0F,0x00,0x40,0x42,
	                        0x0F,0x00,0xA0,0x86,0x01,0x00,0xA0,0x86,0x01,0x00,
                         	0x00,0x00,0x00,0x00,0xF7,0x01,0x00,0x00,0xF2,0xA1};//For beidou-time
#else
	const uint8_t buf1[] = {0xB5,0x62,0x06,0x31,0x20,0x00,0x00,0x01,0x00,0x00,
		                      0x00,0x00,0x00,0x00,0x40,0x42,0x0F,0x00,0x40,0x42,
		                      0x0F,0x00,0xA0,0x86,0x01,0x00,0x00,0x00,0x00,0x00,
		                      0x00,0x00,0x00,0x00,0xF3,0x00,0x00,0x00,0x94,0xCA};//For gps-time
#endif

	const uint8_t buf2[] = {0xB5,0x62,0x06,0x31,0x20,0x00,0x01,0x01,0x00,0x00,
                        	0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x80,0x96,
                       	  0x98,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
                         	0x00,0x00,0x00,0x00,0x6F,0x00,0x00,0x00,0xF7,0x30};
	const uint8_t buf3[] = {0xB5,0x62,0x06,0x31,0x20,0x00,0x00,0x01,0x00,0x00,
                         	0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x80,0x96, 
	                        0x98,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
                    	    0x00,0x00,0x00,0x00,0x6E,0x00,0x00,0x00,0xF5,0x0C};
	/*Time pulse
	* mode:rising edge   pulse period:1000ms
	* pulse length:100ms time source:1UTC cable delay:820ns
	*/
#if defined(WATCH_SINGLE_BEIDOU)
	const uint8_t buf4[] = {0xB5,0x62,0x06,0x07,0x14,0x00,0x40,0x42,0x0F,0x00,
		                      0xA0,0x86,0x01,0x00,0x01,0x00,0x00,0x00,0x34,0x03,
                   	      0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x86};   // For UTC-time
#else
	const uint8_t buf4[] = {0xB5,0x62,0x06,0x07,0x14,0x00,0x40,0x42,0x0F,0x00,
													0xA0,0x86,0x01,0x00,0x01,0x01,0x00,0x00,0x34,0x03,
													0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x91};//For gps-time
#endif
	for(i=0;i<(sizeof(buf4));i++)
	{
		sData.bytes[i] = buf4[i];
	}
	#ifdef WATCH_UBLOX_MOUDULE
	for(uint8_t i = 0;i < sizeof(buf4);i++)
	{
		am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
	#else
	drv_ublox_enable();
	am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf4)),AM_HAL_IOM_RAW);
	drv_ublox_disable();
	#endif
	vTaskDelay(20/portTICK_PERIOD_MS);
	if(mode == TP5_MODE_TWO)
	{
		for(i=0;i<(sizeof(buf2));i++)
		{
			sData.bytes[i] = buf2[i];
		}
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf2);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf2)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	else if(mode == TP5_MODE_ONE)
	{//默认mode=1
		for(i=0;i<(sizeof(buf1));i++)
		{
			sData.bytes[i] = buf1[i];
		}
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf1);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf1)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	else if(mode == TP5_MODE_THREE)
	{
		for(i=0;i<(sizeof(buf3));i++)
		{
			sData.bytes[i] = buf3[i];
		}
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf3);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf3)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}

	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_TP5 ! mode=%d\n",mode);
	vTaskDelay(20/portTICK_PERIOD_MS);
}
/*SBAS Setting
For best time pulse performance it is recommended to disable the SBAS subsystem
*/
void drv_ublox_SBAS(bool flag)
{
	uint8_t i = 0;
	am_hal_iom_buffer(16) sData = {0};
	//enable  ranging(Use SBAS in NAV) 
	const uint8_t buf1[] = {0xB5,0x62,0x06,0x16,0x08,0x00,0x01,0x01,0x01,0x00,
	                        0x04,0xE0,0x04,0x00,0x0F,0x57};
	//disable
	const uint8_t buf2[] = {0xB5,0x62,0x06,0x16,0x08,0x00,0x00,0x01,0x01,0x00,
	                        0x00,0x00,0x00,0x00,0x26,0x97};

	if(flag == true)
	{//enable
		for(i=0;i<(sizeof(buf1));i++)
		{
			sData.bytes[i] = buf1[i];
		}
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf1);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf1)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}
	else
	{//disable
		for(i=0;i<(sizeof(buf2));i++)
		{
			sData.bytes[i] = buf2[i];
		}
		#ifdef WATCH_UBLOX_MOUDULE
		for(uint8_t i = 0;i < sizeof(buf2);i++)
		{
			am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
			while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
		}
		#else
		drv_ublox_enable();
		am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,(sizeof(buf2)),AM_HAL_IOM_RAW);
		drv_ublox_disable();
		#endif
	}

	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: drv_ublox_SBAS !flag=%d\n",flag);
	vTaskDelay(20/portTICK_PERIOD_MS);
}
//--------------------------------------------------
//检测校验和是否正确
//--------------------------------------------------
bool GPS_CheckCommand( void )
{
	//判断语句是否完整
	
	
	return true;
}
//hex to ascii
static uint8_t HexToAsc(char aChar){
  
	if((aChar>=0x30)&&(aChar<=0x39))
	{
		aChar -= 0x30;
	}
	else if((aChar>=0x41)&&(aChar<=0x46))
	{
		aChar -= 0x37;
	}
	else if((aChar>=0x61)&&(aChar<=0x66))
	{
		aChar -= 0x57;
	}
	else
	{
		aChar = 0xFF;
	}
  
return aChar;
  }
/*计算语句返回的校验值是否正确 
“$”为语句起始标志；“，”为域分隔符；“ *”为校验和识别符，其后面的两位数为校验和，
代表了“$”和“*”之间所有字符的按位异或值（不包括这两个字符）
最大帧长
GPRMC 70
GPGGA 72
GPGSA 65
GPGSV 210
*/
static bool GetGNRMCCrc(uint8_t *buf)
{
	int i,result1,result2,crcValue1,crcValue2;
	//RMC校验从$后开始 即GNRMC开始到*标志的校验和结束
//	char a[100]="$GNRMC,114007.00,A,2232.17895,N,11356.90528,E,1.195,,100518,,,A,V*13";
	char a[100],b[100];
	if (buf==NULL) return false;
	
	memcpy(b,buf,100);//找到RMC 然后补足$GPRMC 或$GNRMC
#if defined(WATCH_SINGLE_BEIDOU)
#if defined  WATCH_HUADA_GNSS || defined WATCH_SONY_GNSS
	if (rtc_hw_flag == 1)
	{
	a[0] = '$';
	a[1] = 'B';
	a[2] = 'D';
	}
	else
	{
	a[0] = '$';
	a[1] = 'G';
	a[2] = 'B';
	}
#endif

#else
	a[0] = '$';
	a[1] = 'G';
	a[2] = 'N';
#endif
	for(i=3;i<100;i++)
	{
		a[i] = b[i-3];
	}
	
	for(result1=a[1],i=2;a[i]!='*';i++)
	{
		result1^=a[i];
		if(i== 98)
		{//防止没有结束*
			break;
		}
	}
	crcValue1 = HexToAsc(a[i+1])<<4 | HexToAsc(a[i+2]);
	
	a[0] = '$';
	a[1] = 'G';
	a[2] = 'P';

	for(result2=a[1],i=2;a[i]!='*';i++)
	{
		result2^=a[i];
		if(i== 98)
		{//防止没有结束*
			break;
		}
	}
	crcValue2 = HexToAsc(a[i+1])<<4 | HexToAsc(a[i+2]);

#if defined  WATCH_HUADA_GNSS || defined WATCH_SONY_GNSS
	int result3,crcValue3;
	if (rtc_hw_flag == 1)
	{
	a[0] = '$';
	a[1] = 'B';
	a[2] = 'D';
	}
	else
	{
	a[0] = '$';
	a[1] = 'G';
	a[2] = 'B';
	}
	for(result3=a[1],i=2;a[i]!='*';i++)
	{
		result3^=a[i];
		if(i== 98)
		{//防止没有结束*
			break;
		}
	}
	crcValue3 = HexToAsc(a[i+1])<<4 | HexToAsc(a[i+2]);
   
#endif	
//	DRV_UBLOX_LOG_PRINTF("[drv_ublox]:result1=%X,crcValue1=%X,result2=%X,crcValue2=%X\n",result1,crcValue1,result2,crcValue2);
#if defined  WATCH_HUADA_GNSS || defined WATCH_SONY_GNSS
	if(crcValue1 == result1 || crcValue2 == result2 || crcValue3 == result3)
#else
	if(crcValue1 == result1 || crcValue2 == result2)
#endif
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}

uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{	 		    
	uint8_t *p=buf;
    if (buf==NULL) return 0XFF;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	if(*(buf) == ',')
	{
		return 0xFF;
	}
	return buf-p;	 
}

int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
	uint8_t *p=buf;
	uint32_t ires=0,fres=0;
	uint8_t ilen=0,flen=0,i;
	uint8_t mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}	


//分析GPRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
#if defined(WATCH_PRECISION_TIME_CALIBRATION)
void NMEA_GPRMC_New_Analysis(GPSMode *gpsx,uint8_t *buf,uint16_t buf_len)
{
	uint8_t *p1,dx,*p_buf_tail,*p_buf,*p_p1_head;			 
	uint8_t posx;     
	uint32_t temp;
	float rs;
	uint16_t BUF_LEN = 200;
	uint8_t nav_buf[200];
	
	memcpy(nav_buf,buf,BUF_LEN);
	for(int j = 0;j<BUF_LEN;j++)
	{
		if(nav_buf[j] == 0x62 && nav_buf[j+1] == 0x01 && nav_buf[j+2] == 0x20 
			 && nav_buf[j+3] == 0x10 && nav_buf[j+4] == 0x00 && j < BUF_LEN-19)
		{
			if(nav_buf[j+16] == 0x07)
			{
				set_is_nav_time_gps_valid(true);
				DRV_UBLOX_LOG_PRINTF("nav-timegps is valid-->=0x%x,leap second=0x%x\n",nav_buf[j+16],nav_buf[j+15]);
			}
			else
			{
				DRV_UBLOX_LOG_PRINTF("nav-timegps is invalid---->=0x%x,leap second=0x%x\n",nav_buf[j+16],nav_buf[j+15]);
			}
		}
	}
	set_nmea_rmc_data_valid(false);
	if(buf_len == 0 || buf_len == 0xFFFF)
	{
		return;
	}
	p_buf_tail = buf + buf_len;
	p_buf = buf;
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]:--1-buf_len=%d,buf_len/22=%d,p_buf_tail=%s\n",buf_len,buf_len/22,p_buf);
	for(uint16_t i = 0;i<(buf_len/22);i++)
	{//找出buf中存在的RMC
		p1=(uint8_t*)strstr((const char *)p_buf,"RMC");//"$GNRMC",经常有&和GPRMC分开的情况,故只判断GNRMC.
		if(p1 == NULL)
		{//未找到RMC
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:--2-p1=null\n");
			break;
		}
		p_p1_head = p1;//记录RMC位置
		if((p1 + 22) > p_buf_tail)
		{
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:-3-(p1 + 22) > p_buf_tail\n");
			break;
		}
		else
		{
			p_buf += 22;//RMC后最少有22个才完整
		}
	}
	p1 = p_p1_head;//找到最后一个RMC
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]:--4-p1=%s\n",p1);
	
	posx=NMEA_Comma_Pos(p1,12);//判断语句完整性
	if(posx!=0XFF)
	{
		//RMC校验值判断
		if(GetGNRMCCrc(p1) ==  false)
		{
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:NMEA_GPRMC_Analysis CRC wrong!\n");
			return;
		}
	}
	else
	{
			return;
	}
	posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;
	}	
	posx=NMEA_Comma_Pos(p1,2);								//得到GPS状态
	if(posx!=0XFF)gpsx->status=*(p1+posx);

	posx=NMEA_Comma_Pos(p1,3);								//得到纬度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,6)+(rs*NMEA_Pow(10,6-dx))/60;//转换为° 
		posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬 
		if(posx!=0XFF)gpsx->nshemi=*(p1+posx);	
		posx=NMEA_Comma_Pos(p1,5);								//得到经度
		if(posx!=0XFF)
		{												  
			temp=NMEA_Str2num(p1+posx,&dx);		 	 
			gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
			rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
			gpsx->longitude=gpsx->longitude*NMEA_Pow(10,6)+(rs*NMEA_Pow(10,6-dx))/60;//转换为° 
			posx=NMEA_Comma_Pos(p1,6);								//东经还是西经	
			if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);	
			if((GPSStatus == SEARCHING_SAT) || (GPSStatus == TIME_GETTED))
			{
				GPSStatus = POSITION_GETTED;
			}
		}		
	}
	posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
		gpsx->utc.date=temp/10000;
		gpsx->utc.month=(temp/100)%100;
		gpsx->utc.year=temp%100;
		if( GPSStatus == WAIT_FOR_TIME )
		{
			GPSStatus = TIME_GETTED;			
		}
		if(gpsx->status == 'A')
		{
			set_nmea_rmc_data_valid(true);
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:NMEA_GPRMC_New_Analysis gpsx->status == 'A'!\n");
		}
		else
		{
			set_nmea_rmc_data_valid(false);
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:NMEA_GPRMC_New_Analysis gpsx->status != 'A'!\n");
		}
	}
}
#else
void NMEA_GPRMC_Analysis(GPSMode *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;	
	uint8_t posx;     
	uint32_t temp;	   
	float rs;
	
	p1=(uint8_t*)strstr((const char *)buf,"RMC");//"$GNRMC",经常有&和GPRMC分开的情况,故只判断GNRMC.
	set_nmea_rmc_data_valid(false);
	
	posx=NMEA_Comma_Pos(p1,12);//判断语句完整性
	if(posx!=0XFF)
	{
		//RMC校验值判断
		if(GetGNRMCCrc(p1) ==  false)
		{
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:NMEA_GPRMC_Analysis CRC wrong!\n");
			return;
		}
	}
	else
	{
	    return;
	}
	posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;	 		
	}	
	posx=NMEA_Comma_Pos(p1,2);								//得到GPS状态
	if(posx!=0XFF)gpsx->status=*(p1+posx);

	posx=NMEA_Comma_Pos(p1,3);								//得到纬度
	#if defined WATCH_SONY_GNSS
	if(((posx!=0XFF && gpsx->status == 'A') && (rtc_hw_flag == 1)) || ((posx!=0XFF) && (rtc_hw_flag == 0)))
	#endif
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,6)+(rs*NMEA_Pow(10,6-dx))/60;//转换为° 
		posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬 
		if(posx!=0XFF)gpsx->nshemi=*(p1+posx);	
		posx=NMEA_Comma_Pos(p1,5);								//得到经度

		if(posx!=0XFF)
		
		{												  
			temp=NMEA_Str2num(p1+posx,&dx);		 	 
			gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
			rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
			gpsx->longitude=gpsx->longitude*NMEA_Pow(10,6)+(rs*NMEA_Pow(10,6-dx))/60;//转换为° 
			posx=NMEA_Comma_Pos(p1,6);								//东经还是西经	
			if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);	
			if((GPSStatus == SEARCHING_SAT) || (GPSStatus == TIME_GETTED))
			{
				#if defined WATCH_SONY_GNSS
			   if((( gpsx->status == 'A') && (rtc_hw_flag == 1)) ||(rtc_hw_flag == 0))
				#endif
				{
					GPSStatus = POSITION_GETTED;
				}
			}
		}		
	}
	posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
		gpsx->utc.date=temp/10000;
		gpsx->utc.month=(temp/100)%100;
		gpsx->utc.year=temp%100;
		if( GPSStatus == WAIT_FOR_TIME )
		{
			#if defined WATCH_SONY_GNSS
			if((( gpsx->status == 'A') && (rtc_hw_flag == 1)) ||(rtc_hw_flag == 0))
			#endif
			{
				GPSStatus = TIME_GETTED;
			}
		}
		if(gpsx->status == 'A')
		{
			set_nmea_rmc_data_valid(true);
		}
		else
		{
			set_nmea_rmc_data_valid(false);
		}
	} 		
}
#endif
void NMEA_GPGSV_Analysis(GPSMode *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,k,slx=0;
	uint8_t posx;   	 
	p=buf;
	p1=(uint8_t*)strstr((const char *)p,"$GPGSV");
	len=p1[7]-'0';								//得到GPGSV的条数
	posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
	if(posx!=0XFF)
	{
		gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
		if ((gpsx->svnum == 0) ||(gpsx->svnum > 12) )
		{
			if (gpsx->svnum == 0)
			{
				for(k = 0;k < 12;k++)
				{
					memset(&gpsx->slmsg[k],0,sizeof(nmea_slmsg));
				}
			}
			//不在范围内返回
			return;
		}
	}
	else 
	{
		//错误返回
		return;
	}
//	gpsx->slmsg[0].sn = 0;
//	gpsx->slmsg[1].sn = 0;
//	gpsx->slmsg[2].sn = 0;
	
	for(i=0;i<len;i++)
	{	 
		p1=(uint8_t*)strstr((const char *)p,"$GPGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
			//else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
			//else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
			//else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
			//else break;
			slx++;
			if((slx >= 12) ||(slx >= gpsx->svnum))//超出个数
			{
				for(k = gpsx->svnum;k < 12;k++)
				{
					memset(&gpsx->slmsg[k],0,sizeof(nmea_slmsg));
				}
				return;
			}
		}   
 		p=p1+1;//切换到下一个GPGSV信息
	}   
}


void NMEA_GBGSV_Analysis(GPSMode *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,k,slx=0;
	uint8_t posx; 

	p=buf;
#if defined  WATCH_HUADA_GNSS || defined WATCH_SONY_GNSS
	if(rtc_hw_flag == 1)
	{
	   #if  WATCH_GPS_MODE == 1
	    p1=(uint8_t*)strstr((const char *)p,"$BDGSV");
	   #elif  WATCH_GPS_MODE == 2
	    p1=(uint8_t*)strstr((const char *)p,"$GLGSV");
	   #elif  WATCH_GPS_MODE == 3
	    p1=(uint8_t*)strstr((const char *)p,"$GAGSV");
	   #endif
	}
	else
	{
	   p1=(uint8_t*)strstr((const char *)p,"$GBGSV");
	}
#endif
	len=p1[7]-'0';	

								//得到GPGSV的条数
	posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
	if(posx!=0XFF)
	{
		gpsx->bdnum=NMEA_Str2num(p1+posx,&dx);
		if ((gpsx->bdnum ==0))
		{
			if (gpsx->bdnum == 0)
			{
				for(k = 0;k < 12;k++)
				{
					memset(&gpsx->bdmsg[k],0,sizeof(nmea_slmsg));
				}
			}
			//不在范围内返回
			
			return;
		}
	
	}
	else 
	{
		
		//错误返回
		return;
	}
	
	for(i=0;i<len;i++)
	{	
		#if defined WATCH_HUADA_GNSS || defined WATCH_SONY_GNSS
		if(rtc_hw_flag == 1)
		{
		
		   #if  WATCH_GPS_MODE == 1
		    p1=(uint8_t*)strstr((const char *)p,"$BDGSV");
		   #elif  WATCH_GPS_MODE == 2
		    p1=(uint8_t*)strstr((const char *)p,"$GLGSV");
		   #elif  WATCH_GPS_MODE == 3
		    p1=(uint8_t*)strstr((const char *)p,"$GAGSV");
		   #endif		
		
		}
		else
		{
		p1=(uint8_t*)strstr((const char *)p,"$GBGSV");
		}
		#endif
		for(j=0;j<4;j++)
		{	
		
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->bdmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
			//else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->bdmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
			//else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->bdmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
			//else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->bdmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
			//else break;
			slx++;	 
			if((slx >= 12) ||(slx >= gpsx->bdnum))//超出个数
			{
			 	for(k = gpsx->bdnum;k < 12;k++)
				{
					memset(&gpsx->bdmsg[k],0,sizeof(nmea_slmsg));
				}
				return;
			}
		}   
 		p=p1+1;//切换到下一个GPGSV信息
	}   
}

//分析GPGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGGA_Analysis(GPSMode *gpsx,uint8_t *buf)
{
	u8 *p1,dx;			 
	u8 posx;
#if defined(WATCH_SINGLE_BEIDOU)
	#if defined  WATCH_HUADA_GNSS || defined WATCH_SONY_GNSS
	if(rtc_hw_flag == 1)
	{
	p1=(u8*)strstr((const char *)buf,"GGA");
	}
	else
	{
	p1=(u8*)strstr((const char *)buf,"$GBGGA");
	}
	#endif
#else
	p1=(u8*)strstr((const char *)buf,"GGA");
#endif
	posx=NMEA_Comma_Pos(p1,6);								//得到GPS状态
	if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);	
	posx=NMEA_Comma_Pos(p1,7);								//得到用于定位的卫星数
	if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx); 
	posx=NMEA_Comma_Pos(p1,9);								//得到海拔高度
	if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);  
}

//分析GPGSA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSA_Analysis(GPSMode *gpsx,uint8_t *buf)
{
	u8 *p,*p1,dx;			 
	u8 posx,id = 0; 
	u8 i,j;
	
//	gpsx->possl_gpsnum = 0;
//	gpsx->possl_bdnum = 0;
	
	p = buf;
	for(j=0;j<2;j++)
	{
		p1=(u8*)strstr((const char *)p,"GSA");
		posx=NMEA_Comma_Pos(p1,2);								//得到定位类型
		if(posx!=0XFF)
		{
			gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);
			if ((gpsx->fixmode == 0) ||(gpsx->fixmode >3))
			{
				//不在范围内返回
				return;
			}
		}
		else 
		{
			//错误返回
			return;
		}
		
		for(i=0;i<12;i++)										//得到定位卫星编号
		{
			posx=NMEA_Comma_Pos(p1,3+i);					 
			if(posx!=0XFF)
			{
				gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
			}
			else 
			{
				break;
			}			
		}
		posx=NMEA_Comma_Pos(p1,15);								//得到PDOP位置精度因子
		if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);  
		posx=NMEA_Comma_Pos(p1,16);								//得到HDOP位置精度因子
		if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);  
		posx=NMEA_Comma_Pos(p1,17);								//得到VDOP位置精度因子
		if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);  
		posx=NMEA_Comma_Pos(p1,18);								//得到GNSS 系统 ID
		if(posx!=0XFF)id=NMEA_Str2num(p1+posx,&dx); 
		if(id == 1)
		{
			//GPS
			gpsx->possl_gpsnum = i;
		}
		
		#if WATCH_GPS_MODE == 1
		  else if(id == 4)
		#elif  WATCH_GPS_MODE == 2
		  else if(id == 2)
		#elif  WATCH_GPS_MODE == 3
      	          else if(id == 3)
		#endif		
		{
			//BD
			gpsx->possl_bdnum = i;
		}
		
		p=p1+1;//切换到下一个GNGSA信息
	}
}
/*获取gps时间*/
void SatelliteGPSTime(GPSMode *gps)
{
#if defined WATCH_TIMEZONE_SET
	//UTC 0时区时间  TIMEZONE_Iceland
	am_hal_rtc_time_t m_time_utc_0_zone,m_local_time;//
	
	m_time_utc_0_zone.ui32Hour = gps->utc.hour;
	m_time_utc_0_zone.ui32Minute = gps->utc.min;
	m_time_utc_0_zone.ui32Second = gps->utc.sec;
	m_time_utc_0_zone.ui32DayOfMonth = gps->utc.date;
	m_time_utc_0_zone.ui32Month = gps->utc.month;
	m_time_utc_0_zone.ui32Year = gps->utc.year;
	m_time_utc_0_zone.ui32Weekday = CaculateWeekDay(gps->utc.year,gps->utc.month,gps->utc.date);
	m_time_utc_0_zone.ui32Hundredths = 0;
	m_time_utc_0_zone.ui32Century = 0;
	//转换成SetValue.TimeZoneRecord 
	m_local_time = get_local_time(m_time_utc_0_zone,TIMEZONE_ZERO,SetValue.TimeZoneRecord);
	s_time_gps = m_local_time;

#else
	//GPS授时
	uint8_t year,month,day,hour;
	
	//中国实际时间与UTC相差8小时
	year = gps->utc.year;
	month = gps->utc.month;
	day = gps->utc.date;
	hour = gps->utc.hour + 8;
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: SatelliteGPSTime year=%d,month=%d,day=%d,hour=%d,minute=%d,sec=%d\n"
	    ,year,month,day,hour,gps->utc.min,gps->utc.sec);
	if(hour > 23)
	{
		hour -= 24;
		if(month==1 || month==3 || month==5 || month==7 || month==8 || month==10 || month==12)
		{
			if(day < 31)
			{
				day++;
			}
			else
			{
				if(month != 12)
				{
					day = 1;
					month++;
				}
				else
				{
					day = 1;
					month = 1;
					year++;
				}
			}
		}
		else if(month==4 || month==6 || month==9 || month==11)
		{
			if(day < 30)
			{
				day++;
			}
			else
			{
				day = 1;
				month++;
			}
		}
		else if(month == 2)
		{
			if(day < 28)
			{
				day++;
			}
			else if(day == 28)
			{
				if(Is_Leap_Year(year))
				{
					day++;
				}
				else
				{
					day = 1;
					month++;
				}
			}
			else if(day == 29)
			{
				day = 1;
				month++;
			}					
		}
	}
	//暂时保存GPS授时的值
	s_time_gps.ui32Hour = hour;
	s_time_gps.ui32Minute = gps->utc.min;
	s_time_gps.ui32Second = gps->utc.sec;
	s_time_gps.ui32DayOfMonth = day;
	s_time_gps.ui32Month = month;
	s_time_gps.ui32Year = year;
	s_time_gps.ui32Weekday = CaculateWeekDay(year,month,day);
	s_time_gps.ui32Hundredths = 0;
	s_time_gps.ui32Century = 0;
#endif
}
//卫星时间授时
void SatelliteTime(GPSMode *gps)
{
#if defined WATCH_TIMEZONE_SET
	am_hal_rtc_time_t m_before_time,m_now_time; //授时前RTC时间和要设置到RTC的时间

	m_before_time = RTC_time;
	//UTC 0时区时间  TIMEZONE_Iceland
	am_hal_rtc_time_t m_time_utc_0_zone,m_local_time;
	
	m_time_utc_0_zone.ui32Hour = gps->utc.hour;
	m_time_utc_0_zone.ui32Minute = gps->utc.min;
	m_time_utc_0_zone.ui32Second = gps->utc.sec;
	m_time_utc_0_zone.ui32DayOfMonth = gps->utc.date;
	m_time_utc_0_zone.ui32Month = gps->utc.month;
	m_time_utc_0_zone.ui32Year = gps->utc.year;
	m_time_utc_0_zone.ui32Weekday = CaculateWeekDay(gps->utc.year,gps->utc.month,gps->utc.date);
	m_time_utc_0_zone.ui32Hundredths = 0;
	m_time_utc_0_zone.ui32Century = 0;
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]:SatelliteTime m_time_utc_0_zone %d-%02d-%02d %02d:%02d:%02d.%02d0 \n",
												m_time_utc_0_zone.ui32Year+2000,m_time_utc_0_zone.ui32Month,m_time_utc_0_zone.ui32DayOfMonth,
												m_time_utc_0_zone.ui32Hour,m_time_utc_0_zone.ui32Minute,m_time_utc_0_zone.ui32Second,
												m_time_utc_0_zone.ui32Hundredths);
	//转换成SetValue.TimeZoneRecord
	m_local_time = get_local_time(m_time_utc_0_zone,TIMEZONE_ZERO,SetValue.TimeZoneRecord);
	
	m_now_time = m_local_time;
	RTC_time = m_now_time;
	//设置RTC时间
	am_hal_rtc_time_set(&RTC_time);
	RenewOneDay( m_before_time,m_now_time,true );
#else

	am_hal_rtc_time_t m_before_time,m_now_time; //授时前RTC时间和要设置到RTC的时间
	//GPS授时
	uint8_t year,month,day,hour;
	
	//中国实际时间与UTC相差8小时
	year = gps->utc.year;
	month = gps->utc.month;
	day = gps->utc.date;
	hour = gps->utc.hour + 8;
	if(hour > 23)
	{
		hour -= 24;
		if(month==1 || month==3 || month==5 || month==7 || month==8 || month==10 || month==12)
		{
			if(day < 31)
			{
				day++;
			}
			else
			{
				if(month != 12)
				{
					day = 1;
					month++;
				}
				else
				{
					day = 1;
					month = 1;
					year++;
				}
			}
		}
		else if(month==4 || month==6 || month==9 || month==11)
		{
			if(day < 30)
			{
				day++;
			}
			else
			{
				day = 1;
				month++;
			}
		}
		else if(month == 2)
		{
			if(day < 28)
			{
				day++;
			}
			else if(day == 28)
			{
				if(Is_Leap_Year(year))
				{
					day++;
				}
				else
				{
					day = 1;
					month++;
				}
			}
			else if(day == 29)
			{
				day = 1;
				month++;
			}					
		}
	}
	
	m_before_time = RTC_time;
	m_now_time.ui32Year = year;
	m_now_time.ui32Month = month;
	m_now_time.ui32DayOfMonth = day;
	m_now_time.ui32Hour = hour;
	m_now_time.ui32Minute = gps->utc.min;
	m_now_time.ui32Second = gps->utc.sec;
	m_now_time.ui32Hundredths = 0;
	m_now_time.ui32Weekday = CaculateWeekDay(year,month,day);
	m_now_time.ui32Century = 0;
	
	RTC_time = m_now_time;
	//设置RTC时间
	am_hal_rtc_time_set(&RTC_time);
	RenewOneDay( m_before_time,m_now_time,true );
#endif
}

/*
void dev_ublox_utc_read_init(void)
{
	DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:dev_ublox_utc_read_init\n");
	drv_ublox_poweron();
	vTaskDelay(100/portTICK_PERIOD_MS);	
//	drv_ublox_enable();
	drv_ublox_reset_rtc();
	vTaskDelay(100/portTICK_PERIOD_MS);
	drv_ublox_GGA(false);
	vTaskDelay(100/portTICK_PERIOD_MS);
	drv_ublox_GSV(false);
	vTaskDelay(100/portTICK_PERIOD_MS);
	drv_ublox_GLL(false);
	vTaskDelay(100/portTICK_PERIOD_MS);
	drv_ublox_GSA(false);
	vTaskDelay(100/portTICK_PERIOD_MS);
	drv_ublox_VTG(false);
	vTaskDelay(100/portTICK_PERIOD_MS);
	drv_ublox_RMC(true);
	vTaskDelay(100/portTICK_PERIOD_MS);
	drv_ublox_GNSS(UBLOX_GN);	
	vTaskDelay(100/portTICK_PERIOD_MS);
//	drv_ublox_disable();
}
bool dev_ublox_utc_read(am_hal_rtc_time_t *time)
{
	DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:dev_ublox_utc_read\n");
	uint8_t *address_buf,end_buf;
	long temp_date;	
	long temp_time;
	bool error = false;
	am_hal_iom_buffer(300) sData = {0};
	am_hal_iom_buffer(1) temp_buf = {0}; 
	drv_ublox_enable();	
	for(uint32_t i=0;i<sizeof(sData.bytes);)
	{
		if(AM_HAL_IOM_SUCCESS == am_hal_iom_i2c_read(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,temp_buf.words,sizeof(temp_buf.bytes),AM_HAL_IOM_RAW))
		{
			sData.bytes[i] = temp_buf.bytes[0];
			i++;
		}
	}
	drv_ublox_disable();
	#if DRV_UBLOX_LOG_ENABLED == 1
	for(uint32_t i=0;i<32;i++)
	{
		DRV_UBLOX_LOG_PRINTF("%c",sData.bytes[i]);
	}	
	DRV_UBLOX_LOG_PRINTF("\n");
	#endif 
	address_buf = (uint8_t*)strstr((const char *)sData.bytes,"RMC");
	if(address_buf != NULL)
	{
		DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:RMC\n");
//		end_buf = NMEA_Comma_Pos(address_buf,2); //第二个逗号表示定位状态，'A'表示定位有效，'V'表示定位无效
		
//		if(end_buf != 0xFF)
		{
//			if (*(address_buf+end_buf) == 'A')
			{
//				DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:status == 'A'\n");
				end_buf = NMEA_Comma_Pos(address_buf,1);//第一个逗号表示UTC时间
				if(end_buf != 0XFF)
				{
					DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:time == good\n");
					temp_time = atol((const char *)(address_buf+end_buf));
					DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:temp --> %d\n",temp_time);
					time->ui32Hour = (temp_time)/10000 + 8;
					time->ui32Minute = ((temp_time)/100)%100;
					time->ui32Second = (temp_time)%100;		
					DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:%d:%d:%d\n",time->ui32Hour,time->ui32Minute,time->ui32Second);
					end_buf = NMEA_Comma_Pos(address_buf,9);//第九个逗号表示UTC日期
					if(end_buf != 0XFF)
					{
						DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:date == good\n");						
						temp_date = atol((const char *)(address_buf+end_buf));
						DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:temp --> %l\n",temp_date);
						time->ui32Year = temp_date%100;	
						time->ui32Month = (temp_date/100)%100;
						time->ui32DayOfMonth = temp_date/10000;	
						time->ui32Weekday = am_util_time_computeDayofWeek(time->ui32Year+2000, time->ui32Month, time->ui32DayOfMonth);			
						DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:%d/%d/%d.%d\n",time->ui32Year,time->ui32Month,time->ui32DayOfMonth,time->ui32Weekday);
						error = true;
					}
				}				
			}
		
		}
		
		
	}
	
	return error;
}
void dev_ublox_utc_read_uninit(void)
{
	DRV_UBLOX_LOG_PRINTF("[DRV_UBLOX]:dev_ublox_utc_read_uninit\n");
	drv_ublox_poweroff();
}
*/

#if defined WATCH_GPS_ONLINE

/*在线星历 获取完整星历可以使用更多卫星来改善DOP值,对快速定位/漂移问题也会有所改善*/
void drv_ublox_online_ephemeris(void)
{
	uint16_t i;//j;
	_ublox_offline_data ublox_offline_data;
	uint32_t addr = SATELLITE_DATA_START_ADDRESS + 1024 *80; //online 存储地址
	am_hal_iom_buffer(76) sData;
	bool on_flag = false;
	am_hal_iom_buffer(48) sData_config = {0};
	const uint8_t buf[] = {0xB5,0x62,0x06,0x23,0x28,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
						0x00,0x00,0x00,0x00,0x00,0x00,0x56,0x24};
	
	
	dev_extFlash_enable();
	dev_extFlash_read(addr,(uint8_t *)(&ublox_offline_data.len),4);
	dev_extFlash_disable();
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ublox_offline_data.len %d\n",ublox_offline_data.len);

	if(ublox_offline_data.len != 0xFFFFFFFF && ublox_offline_data.len < (30*1024))
	{
		ublox_offline_data.len &= 0x0000FFFF;
		if((ublox_offline_data.len%12) == 0)
		{
			if(GPSStatus == GPS_OFF)
			{//打开GPS
				on_flag = true;
			
				drv_ublox_poweron();
				vTaskDelay(100/portTICK_PERIOD_MS);
			}
				

			addr += sizeof(_ublox_offline_data);
			dev_extFlash_enable();	
			dev_extFlash_read(addr,sData.bytes,32);
			dev_extFlash_disable();
#if 0
			for (i=0;i<32;i++)
			{
				DRV_UBLOX_LOG_PRINTF("[drv_ublox]: sData.bytes[%d]= %x\n",i,sData.bytes[i]);
				vTaskDelay(1/portTICK_PERIOD_MS);
			}
#endif
			if (sData.bytes[2]==0x13 && sData.bytes[3]==0x40)
			{
				for(i=0;i<(sizeof(buf));i++)
				{
					sData_config.bytes[i] = buf[i];
				}
				
				#ifdef WATCH_UBLOX_MOUDULE
				for(uint8_t i = 0;i <sizeof(buf);i++)
				{
					am_hal_uart_char_transmit_buffered(0, sData_config.bytes[i]);	
	
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
				vTaskDelay(20/portTICK_PERIOD_MS);

				for(uint8_t i = 0;i <32;i++)
				{
					am_hal_uart_char_transmit_buffered(0, sData.bytes[i]);	
	
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
				vTaskDelay(20/portTICK_PERIOD_MS);
				#else
				drv_ublox_enable();
				am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData_config.words,(sizeof(buf)),AM_HAL_IOM_RAW);
				drv_ublox_disable();
				vTaskDelay(20/portTICK_PERIOD_MS);
				drv_ublox_enable();
				am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,32,AM_HAL_IOM_RAW);
				drv_ublox_disable();				
				vTaskDelay(20/portTICK_PERIOD_MS);
				#endif
			}
			else
			{
				return;
			}
			ublox_offline_data.len = (ublox_offline_data.len-32)/76;
			
			addr +=32;	
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ublox_offline_data.len %d,addr %x\n",ublox_offline_data.len,addr);
			for(i = 0;i < ublox_offline_data.len;i++)
			{
				
				dev_extFlash_enable();	
				dev_extFlash_read(addr,sData.bytes,76);
				dev_extFlash_disable();
#if 0
				for (j=0;j<76;j++)
				{
					DRV_UBLOX_LOG_PRINTF("[drv_ublox]: %d sData.bytes[%d]= %x\n",i,j,sData.bytes[j]);
					vTaskDelay(1/portTICK_PERIOD_MS);
				}
#endif
				#ifdef WATCH_UBLOX_MOUDULE
				for(uint8_t j = 0;j < 76;j++)
				{
					am_hal_uart_char_transmit_buffered(0, sData.bytes[j]);	
	
					while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
				}
				#else
				drv_ublox_enable();
				am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,76,AM_HAL_IOM_RAW);
				drv_ublox_disable();
				#endif
				vTaskDelay(20/portTICK_PERIOD_MS);
				addr +=76;
				
			}
			//vTaskDelay(20/portTICK_PERIOD_MS);
			if(on_flag)
			{
				//关闭GPS电源
				drv_ublox_poweroff();
			}
		}
	}
	else
	{
		DRV_UBLOX_LOG_PRINTF("drv_ublox_offline_ephemeris is empty!\n");
	}
}

bool ublox_is_online_data_in_flash_uptodate(void)
{
	uint32_t addr = SATELLITE_DATA_START_ADDRESS + 80*1024; //online 存储地址
	_ublox_offline_data ublox_offline_data;
	uint8_t time[2];
	int32_t time_diff;
	bool ret_val = false;
	rtc_time_t utc_time;
	rtc_time_t online_time;

	dev_extFlash_enable();
	dev_extFlash_read(addr,(uint8_t *)&ublox_offline_data,sizeof(_ublox_offline_data));
	dev_extFlash_disable();	
	
	
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ublox_offline_data.len --> 0x%08X\n",ublox_offline_data.len);
	
	addr += sizeof(_ublox_offline_data);
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: addr --> 0x%08X\n",addr);	
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ublox_offline_data.len --> %d\n",ublox_offline_data.len);
	
	if(ublox_offline_data.len != 0xFFFFFFFF && ublox_offline_data.len < (30*1024))
	{
			utc_time = RtcToUtc(3600*8);//东八区转utc时间	
		
			dev_extFlash_enable();
			dev_extFlash_read(addr+10,(uint8_t *)time,2);
			dev_extFlash_read(addr+11,(uint8_t *)&online_time,sizeof(rtc_time_t));
			dev_extFlash_disable();				
			online_time.Year = time[0] + time[1] *0x100 - 2000;

			time_diff  = Get_DiffTime(utc_time,online_time);
			
			if (time_diff > 0 && time_diff < 7200) //online 两小时内有效
			{
				ret_val = true;
			}
			
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: year  %d, mont %d, date %d,%d,%d,%d\n",online_time.Year,online_time.Month,online_time.Day,online_time.Hour,online_time.Minute,online_time.Second);
	}

	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ret_val --> %d\n",ret_val);

	return ret_val;
}

uint8_t ublox_online_offline_data_in_flash_uptodata(void)
{
	uint8_t ret_val = 0;

	if (ublox_is_offline_data_in_flash_uptodate ()== true) //不需更新离线星历
	{
		ret_val |= 0x01; //离线星历返回值mask
	}

	if (ublox_is_online_data_in_flash_uptodate ()== true) //不需更新在线星历 
	{
		ret_val |= 0x02; //离线星历返回值mask
	}

	return ret_val;

}

void drv_ublox_online_offline_ephemeris_select(void)
{

   if (ublox_is_online_data_in_flash_uptodate ()== true)//有有效在线星历
   {
	   drv_ublox_online_ephemeris();
	   online_valid = 1;

   }

   else
   {
		drv_ublox_offline_ephemeris();
		online_valid = 0;

   }

}

#endif

bool ublox_is_offline_data_in_flash_uptodate(void)
{
	bool ret_val = false;
        rtc_time_t utc_time;

    #ifdef WATCH_SONY_GNSS
        uint32_t agps_lle_time,current_time;
        
        utc_time = RtcToUtc(3600*8);//东八区转utc时间
	current_time = GetTimeSecond(utc_time);//当前时间 s
	agps_lle_time = GetTimeSecond(SetValue.agps_effective_time);//文件过期时间 s

        DRV_UBLOX_LOG_PRINTF("[current_time]: %d-%d-%d-%d-%d\n",utc_time.Year,utc_time.Month,utc_time.Day,utc_time.Hour,utc_time.Minute);
        DRV_UBLOX_LOG_PRINTF("[agps_lle_time]: %d-%d-%d-%d-%d\n",SetValue.agps_effective_time.Year,SetValue.agps_effective_time.Month,
        SetValue.agps_effective_time.Day,SetValue.agps_effective_time.Hour,SetValue.agps_effective_time.Minute);

	if(current_time < agps_lle_time)//2天
	{
	   DRV_UBLOX_LOG_PRINTF("[drv_ublox]: FOUND NEW ASTRO DATA\n");
	   ret_val = true;//有星历
	}
	else
	{
	   DRV_UBLOX_LOG_PRINTF("[drv_ublox]: NOT FOUND ASTRO DATA\n");
           ret_val = false;//无星历
	}
    
    #else
	uint32_t addr = SATELLITE_DATA_START_ADDRESS;
	_ublox_offline_data ublox_offline_data;
	uint8_t time[3];
	uint16_t i;

	#ifndef COD 
	dev_extFlash_enable();
	dev_extFlash_read(addr,(uint8_t *)&ublox_offline_data,sizeof(_ublox_offline_data));
	dev_extFlash_disable();	
	
	
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ublox_offline_data.len --> 0x%08X\n",ublox_offline_data.len);
	
	addr += sizeof(_ublox_offline_data);
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: addr --> 0x%08X\n",addr);
	//ublox_offline_data.len /= 84;	
	DRV_UBLOX_LOG_PRINTF("[drv_ublox]: ublox_offline_data.len --> %d\n",ublox_offline_data.len);
	#else
	ublox_offline_data.len = 20160;
	#endif
	if(ublox_offline_data.len != 0xFFFFFFFF && ublox_offline_data.len < (30*1024))
	{
		utc_time = RtcToUtc(3600*8);//东八区转utc时间
		#ifdef COD
		for(i = 0;i < (ublox_offline_data.len / 84);i++)//查询7天数据
		#else
		for(i = 0;i < 32;i++)//最多查询两天的数据
		#endif
		{
			dev_extFlash_enable();
			dev_extFlash_read(addr+10,(uint8_t *)time,3);
			dev_extFlash_disable();				
			
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]: year --> %d, month --> %d, date --> %d\n",time[0],time[1],time[2]);
			
			if(time[0] == utc_time.Year && time[1] == utc_time.Month && time[2] == utc_time.Day)
			{
				DRV_UBLOX_LOG_PRINTF("[drv_ublox]: FOUND NEW ASTRO DATA\n");
				ret_val = true;//
				break;
			}
			addr += 84;
		}
	}

   #endif
	return ret_val;
}


/*离线星历 获取完整星历可以使用更多卫星来改善DOP值,对快速定位/漂移问题也会有所改善*/
void drv_ublox_offline_ephemeris(void)
{
#ifdef WATCH_SONY_GNSS
	uint16_t i;
	_ublox_offline_data ublox_offline_data;
	uint32_t addr = SATELLITE_DATA_START_ADDRESS;
	am_hal_iom_buffer(84) sData;
	bool on_flag = false;
	uint8_t time[3];
	rtc_time_t utc_time;
	if (rtc_hw_flag == 0)
	{
	memset(time,0,sizeof(time));
	#ifndef COD 
	dev_extFlash_enable();
	dev_extFlash_read(addr,(uint8_t *)(&ublox_offline_data.len),4);
	dev_extFlash_disable();
		DRV_UBLOX_LOG_PRINTF("[drv_ublox]:ublox_offline_data.len %d\r\n",ublox_offline_data.len);
	#else 
	ublox_offline_data.len = 20160;
	#endif
	if(ublox_offline_data.len != 0xFFFFFFFF && ublox_offline_data.len < (30*1024))
	{
		ublox_offline_data.len &= 0x0000FFFF;
		if((ublox_offline_data.len%84) == 0)
		{
			if(GPSStatus == GPS_OFF)
			{//打开GPS
				on_flag = true;
			
				drv_ublox_poweron();
				vTaskDelay(100/portTICK_PERIOD_MS);
			}else{
				//热启动
				//drv_ublox_hotstart();
			}
			#ifndef COD 
			addr += sizeof(_ublox_offline_data);
			#endif
			ublox_offline_data.len /= 84;
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:ublox_offline_data.len %d\r\n",ublox_offline_data.len);
			
			utc_time = RtcToUtc(3600*8);//东八区转utc时间	
			
			DRV_UBLOX_LOG_PRINTF("[drv_ublox]:utc_time year %d, %d, %d,%d,%d\r\n",utc_time.Year,utc_time.Month,utc_time.Day,utc_time.Hour,utc_time.Minute);
			for(i = 0;i < ublox_offline_data.len;i++)
			{
				dev_extFlash_enable();	
				dev_extFlash_read(addr+10,(uint8_t *)time,3);		
				dev_extFlash_disable();
				
				//DRV_UBLOX_LOG_PRINTF("[drv_ublox]:offline_ephemeris year --> 0x%02X month --> 0x%02X date --> 0x%02X\r\n",time[0],time[1],time[2]);
				if(time[0] == utc_time.Year && time[1] == utc_time.Month && time[2] == utc_time.Day)
				{
					//DRV_UBLOX_LOG_PRINTF("The same day\r\n");
					dev_extFlash_enable();	
					dev_extFlash_read(addr,sData.bytes,84);
					dev_extFlash_disable();

#if 0					
					for(int j=0;j<84;j++)
					{
						DRV_UBLOX_LOG_PRINTF("0x%02X,",sData.bytes[j]);
					}
					DRV_UBLOX_LOG_PRINTF("\r\n");
#endif					

					#if defined  WATCH_UBLOX_MOUDULE
					for(uint8_t j = 0;j < 84;j++)
					{
						am_hal_uart_char_transmit_buffered(0, sData.bytes[j]);	
	
						while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
					}
					
					#else
					if (rtc_hw_flag == 0)
					{
						drv_ublox_enable();
						am_hal_iom_i2c_write(BSP_UBLOX_IOM,BSP_UBLOX_I2C_ADDRESS,sData.words,84,AM_HAL_IOM_RAW);
						drv_ublox_disable();
					}						
					#endif
					vTaskDelay(20/portTICK_PERIOD_MS);
				}
				addr += 84;
				
			}
			//vTaskDelay(100/portTICK_PERIOD_MS);
			if(on_flag)
			{
				//关闭GPS电源
				drv_ublox_poweroff();
			}
		}
	}
	else
	{
		DRV_UBLOX_LOG_PRINTF("drv_ublox_offline_ephemeris is empty!\n");
	}
	}
#endif
}




