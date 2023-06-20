#include "isr_uart.h"
#include "isr_config.h"
#include "task_uart.h"
#include "drv_heartrate.h"
#include "bsp.h"
#include "cmd_pc.h"
#if defined (WATCH_HUADA_GNSS) || defined WATCH_SONY_GNSS || defined WATCH_UBLOX_MOUDULE
#include "drv_ublox.h"
#endif


#if DEBUG_ENABLED == 1 && ISR_UART_LOG_ENABLED == 1
	#define ISR_UART_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define ISR_UART_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define ISR_UART_LOG_WRITESTRING(...)
	#define ISR_UART_LOG_PRINTF(...)		        
#endif
static bool g_heart_flag = false;
static uart0_callback m_uart0_cb;



/*设置心率串口中断标志*/
void SetHdrFlag(bool isEnable)
{
	g_heart_flag = isEnable;
}
/*获取心率串口中断标志*/
bool GetHdrFlag(void)
{
	return g_heart_flag;
}
static bool m_is_pc = false;
void am_uart_isr(void)
{

	uint32_t status;
	uint32_t rxSize, txSize;
	uint8_t mode = 0;

	status = am_hal_uart_int_status_get(0, false);
	am_hal_uart_int_clear(0, status);

	
	if (status & (AM_HAL_UART_INT_RX_TMOUT | AM_HAL_UART_INT_TX | AM_HAL_UART_INT_RX))
	{
			am_hal_uart_service_buffered_timeout_save(0, status);
	}

	if (status & (AM_HAL_UART_INT_RX_TMOUT))
	{
		mode = 1;
	}

	am_hal_uart_get_status_buffered(0, &rxSize, &txSize);

	if (rxSize >= 256)
	{
		mode = 1;
	}
	
	if(mode == 1)
	{
#if !defined (WATCH_HUADA_GNSS) && !defined WATCH_SONY_GNSS && !defined WATCH_UBLOX_MOUDULE
		static uint8_t data[256];
		uint32_t length = am_hal_uart_char_receive_buffered(0, (char *)data, 256);
#endif

//		ISR_UART_LOG_PRINTF("[ISR_UART]:Receive data_length = %d,data = {",length);
//		for(int i = 0;i < length;i++)
//		{
//		 ISR_UART_LOG_PRINTF("0x%02X,",data[i]);
//		}
//		ISR_UART_LOG_PRINTF("}\n");
		
		if(m_is_pc)
		{
#if defined (WATCH_HUADA_GNSS) || defined WATCH_SONY_GNSS || defined WATCH_UBLOX_MOUDULE
		static uint8_t data[256];
		uint32_t length = am_hal_uart_char_receive_buffered(0, (char *)data, 256);
#endif

#ifdef WATCH_CHIP_TEST
			if(data[0] == UART_PC_PACKET_HEADER)
#else
			if(data[0] == UART_PACKET_HEADER)
#endif
			{
				if(m_uart0_cb != NULL)
				{
					m_uart0_cb(data);
				}				
			}		
		}
		else
		{
	#if defined (WATCH_HUADA_GNSS) || defined WATCH_SONY_GNSS || defined WATCH_UBLOX_MOUDULE
			g_UartRxTimeoutFlag = 1;
	#else
			if(data[0] == PACKET_HDR)
			{//心率串口接收值头为0x50或0x81,PC串口接收值头为0xA5
				if(m_uart0_cb != NULL)
				{
					m_uart0_cb(data);
				}				
			}
	#endif

		}
		

	}	
}


void uart0_handler_register(uart0_callback uart0_cb, bool is_pc)
{
	m_uart0_cb = uart0_cb;
	m_is_pc = is_pc;
}
void uart0_handler_unregister(void)
{
	m_uart0_cb = NULL;
}

#if defined (WATCH_HUADA_GNSS) || defined WATCH_SONY_GNSS || defined WATCH_UBLOX_MOUDULE
void set_m_is_pc(bool is_pc )
{
	m_is_pc = is_pc;

}
#endif


