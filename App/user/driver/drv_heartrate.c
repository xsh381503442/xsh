/**
*Module:HeartRate Philips
*Chip:Philips WeST HDR A3OS
*Version: V 0.0.1
*Time: 2017/11/16
**/
#include <stdio.h>
#include "drv_heartrate.h"
#include "bsp_uart.h"
#include "drv_config.h"
#include "bsp.h"
#include "isr_gpio.h"
#include "task_uart.h"
#include "bsp_timer.h"

#if DEBUG_ENABLED == 1 && DRV_HEARTRATE_LOG_ENABLED == 1
	#define DRV_HEARTRATE_LOG_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define DRV_HEARTRATE_LOG_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else       
	#define DRV_HEARTRATE_LOG_WRITESTRING(...)
	#define DRV_HEARTRATE_LOG_PRINTF(...)		        
#endif


static uint8_t g_HR_Tx_Buffer[HDR_BUFFER_SIZE];
static uint8_t g_HR_Rx_Buffer[HDR_BUFFER_SIZE];
static uint16_t packetLen;
static uint16_t packetCrc;
static uint8_t  packetIdx;
//static uint8_t  lenPrefix = 2;

static uint8_t m_first_receive_flag = 0;
static uint8_t  hdr_value = 0;//心率存放
//static bool enableHeartFlag = false;//使能心率标志
static char str[30];//心率版本号存放
static hdr_callback m_hdr_cb;

//Get A3OS version  当前版本A3OS V5.4.8;build 20170113_1345
//static uint8_t p_cmd_get_version[] = {0x56};
//static uint8_t p_data_get_version[] = {0x01};
static uint8_t m_cmd_get_version[] = {0x00,0x00,0x81,0x02,0x00,0x56,0x01,0x37,0xA9};
//Enable Metric Heart Rate (0x20) and Skin Proximity (0x22)
//static uint8_t p_cmd_enable_metric[] = {0x45};
//static uint8_t p_data_enable_metric[] = {0x01,0x02,0x20,0x22};
//static uint8_t p_data_disable_metric[] = {0x00,0x02,0x20,0x22};
static uint8_t m_cmd_enable_metric[] = {0x00,0x00,0x81,0x05,0x00,0x45,0x01,0x02,0x20,0x22,0x21,0x39};
static uint8_t m_cmd_disable_metric[] = {0x00,0x00,0x81,0x05,0x00,0x45,0x00,0x02,0x20,0x22,0x95,0x4F};
//static uint8_t p_data_enable_heart[] = {0x01,0x01,0x20};
//static uint8_t p_data_disable_heart[] = {0x00,0x01,0x20};
//static uint8_t p_data_enable_skin[] = {0x01,0x01,0x22};
//static uint8_t p_data_disable_skin[] = {0x00,0x01,0x22};
//State Power down
//static uint8_t p_cmd_state_power_down[] = {0x50};
//static uint8_t p_data_state_power_down[] = {0x00};
//static uint8_t m_cmd_state_power_down[] = {0x00,0x00,0x81,0x02,0x00,0x50,0x00,0xB0,0x13};
//static uint8_t p_data_state_power_active[] = {0x02};
//static uint8_t p_data_state_power_diagnosing[] = {0x64};
//static uint8_t p_data_state_power_diagnosing_state[] = {0x40};
//Get state (while in Active state)
//static uint8_t p_cmd_get_active_state[] = {0x50};
//Get updater version (in Updating state)
//static uint8_t p_cmd_get_update_state[] = {0x56};
//static uint8_t p_data_get_update_state[] = {0x02};
//Metric Heart Rate (0x20) and Skin Proximity (0x22)
//static uint8_t p_cmd_get_updated_metric_values[] = {0x47};
//static uint8_t p_data_get_updated_metric_values[] = {0x02,0x20,0x22};
static uint8_t m_cmd_get_updated_metric_values[] = {0x00,0x00,0x81,0x04,0x00,0x47,0x02,0x20,0x22,0xD7,0xD7};
//static uint8_t p_data_get_updated_skin_values[] = {0x01,0x22};

static am_hal_uart_config_t g_HR_UartConfig =
{
	.ui32BaudRate = 115200,
	.ui32DataBits = AM_HAL_UART_DATA_BITS_8,
	.bTwoStopBits = false,
	.ui32Parity   = AM_HAL_UART_PARITY_NONE,
	.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,
};

/*计算CRC*/
static void CalculateCrc(uint16_t * const pCrc, uint8_t const data)
{
	uint16_t x;
	
	x = (( *pCrc >> 8 ) ^ data) & 0xFF;
	x ^= x >> 4;
	*pCrc = ( *pCrc << 8 ) ^ ( x << 12 ) ^ ( x << 5 ) ^ x;
}
//static void SendPacket
//(
//	uint8_t const *pCmd,
//	uint8_t const lenCmd,
//	uint8_t const *pData,
//	uint8_t const lenData
//)
//{
//	uint16_t i = 0;
//	//send prefixes:0
//	for( ; lenPrefix > 0; lenPrefix-- )
//	{
//		am_hal_uart_char_transmit_buffered( 0,PACKET_HDR_PREFIX );
//	}
//	//send header:H
//	am_hal_uart_char_transmit_buffered( 0,PACKET_HDR );
//	
//	//send length command: N = lenCmd +lenData
//	uint16_t len = lenCmd + lenData;
//	am_hal_uart_char_transmit_buffered(0,( len     ) & 0xFF);
//	am_hal_uart_char_transmit_buffered(0,( len >> 8) & 0xFF);
//	
//	//init crc
//	uint16_t crc = 0xFFFF;
//	
//	//send cmd bytes: CMD (+calc crc)
//	for(uint16_t i = 0; i < lenCmd; i++)
//	{
//		CalculateCrc(&crc,pCmd[i]);
//		am_hal_uart_char_transmit_buffered(0,pCmd[i]);
//	}
//	
//	//send data bytes: D(+calc crc)
//	for(uint16_t i = 0; i < lenData; i++)
//	{
//		CalculateCrc(&crc,pData[i]);
//		am_hal_uart_char_transmit_buffered(0,pData[i]);
//	}
//	
//	//send calculated CRC
//	am_hal_uart_char_transmit_buffered( 0,crc       & 0xFF);
//	am_hal_uart_char_transmit_buffered( 0,(crc >> 8) & 0xFF);
//	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:SendPacket = {0x00,0x00,0x81,0x%02X,0x%02X,",(len) & 0xFF,( len >> 8) & 0xFF);
//	for(i = 0;i < lenCmd;i++)
//	{
//		DRV_HEARTRATE_LOG_PRINTF("0x%02X,",pCmd[i]);
//	}
//		for(i = 0;i < lenData;i++)
//	{
//		DRV_HEARTRATE_LOG_PRINTF("0x%02X,",pData[i]);
//	}
//	DRV_HEARTRATE_LOG_PRINTF("0x%02X,0x%02X",(crc&0xFF),((crc >> 8) & 0xFF));
//	DRV_HEARTRATE_LOG_PRINTF("}\n");
//}
static void sendCmd(const uint8_t *p_data, uint16_t data_length)
{
	for(int i = 0;i < data_length;i++)
	{
		am_hal_uart_char_transmit_buffered(0, p_data[i]);	
		while ( am_hal_uart_flags_get(0) & (AM_HAL_UART_FR_BUSY | AM_HAL_UART_FR_TX_FULL));
	}
}
/*****************************************************/
//!①手表在手，现象有：此时skin=0，心率的quality为4；
//!②手表离手，现象有：此时skin=254,刚离手时心率的quality在0~3,短时间离手到戴手上会到4，长时间离手一直为4；
//!③在手时skin从on到off,需要5s，心率的quality从其他值到等于4，需要10s；
//!④离手时从off到on，至少需要30s，心率的quality从4到其他值，需要5s;
//!⑤串口返回会多一个0x51返回数据，会导致ReceivePacket状态值不准
/*****************************************************/
bool drv_receivePacket(void)
{
	uint8_t status = RESP_PACKET_ACK;
	bool stop = false;
	uint8_t inData;
	packetIdx = 0;
	uint16_t tmpCrc,i = 0;
	
	while( stop == false)
	{
		inData = am_hal_uart_char_receive_buffered(0,(char *)(g_HR_Rx_Buffer),HDR_BUFFER_SIZE);
		
		if(packetIdx == 0)           //header byte(s)
		{
			if(inData != PACKET_HDR_PREFIX)    //skip all 0x00 bytes
			{
				if(inData != PACKET_HDR)
				{
					status = RESP_HEADER_INCORRECT;
					stop = true;
				}
				else
				{
					packetIdx ++;
				}
			}
			else
			{
				//skip all 0x00 bytes
			}
		}
		else if(packetIdx == 1)
		{
			packetLen = inData;
			packetIdx ++;
		}
		else if(packetIdx == 2)
		{
			packetLen += ((uint16_t)inData) << 8;
			
			if(packetLen == 0)
			{
				status = RESP_PACKET_ZERO;
				stop = true;
			}
			else if(packetLen > HDR_BUFFER_SIZE)
			{
				status = RESP_PACKET_TOO_BIG;
				stop = true;
			}
			else
			{
				packetIdx++;
				tmpCrc = 0xFFFF;  //init crc of received data
			}
		}
		else if(packetIdx == packetLen + 3)//CRC LSB
		{
			packetCrc = inData;
			packetIdx ++;
		}
		else if(packetIdx == packetLen + 4) //CRC MSB
		{
			packetCrc += ((uint16_t)inData) << 8;
			if(packetCrc != tmpCrc)
			{
				status = RESP_CRC_INCORRECT;
			}
			else
			{
				status = RESP_PACKET_ACK;
				packetIdx ++;
			}
			stop = true;
		}
		else    //store data
		{
			g_HR_Rx_Buffer[packetIdx - 3] = inData;  //-3: 1 header + 2 length
			CalculateCrc(&tmpCrc,inData);
			packetIdx ++;
		}
	}
	g_HR_Rx_Buffer[inData] = 0;
	am_hal_uart_string_transmit_buffered(0, (char *)g_HR_Rx_Buffer);
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:Receive heart data_length = %d,data = {",inData);
	for(i = 0;i < inData;i++)
	{
	 DRV_HEARTRATE_LOG_PRINTF("0x%02X,",g_HR_Rx_Buffer[i]);
	}
	DRV_HEARTRATE_LOG_PRINTF("}\n");
  //测试
	if(g_HR_Rx_Buffer[0] == 0x50)
	{//返回值判断
		if( g_HR_Rx_Buffer[0] == 0x50 && g_HR_Rx_Buffer[1] == 0x81 && g_HR_Rx_Buffer[2] == 0x12 && g_HR_Rx_Buffer[3] == 0x00 &&
		 g_HR_Rx_Buffer[4] == 0x56 && g_HR_Rx_Buffer[5] == 0x01 )
		{//版本号
			sprintf((char *)str,"v%d.%d.%d;build %d%d%d%d%d%d%d%d_%d%d%d%d",
			g_HR_Rx_Buffer[6],g_HR_Rx_Buffer[7],g_HR_Rx_Buffer[8],
			g_HR_Rx_Buffer[9]-48,g_HR_Rx_Buffer[10]-48,g_HR_Rx_Buffer[11]-48,g_HR_Rx_Buffer[12]-48,
			g_HR_Rx_Buffer[13]-48,g_HR_Rx_Buffer[14]-48,g_HR_Rx_Buffer[15]-48,g_HR_Rx_Buffer[16]-48,
			g_HR_Rx_Buffer[18]-48,g_HR_Rx_Buffer[19]-48,g_HR_Rx_Buffer[20]-48,g_HR_Rx_Buffer[21]-48);
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: 1-HeartrateVersion:%s\n",str);
		}
		else if( g_HR_Rx_Buffer[0] == 0x50 && g_HR_Rx_Buffer[1] == 0x81 && g_HR_Rx_Buffer[2] == 0x0E && g_HR_Rx_Buffer[3] == 0x00 &&
			 g_HR_Rx_Buffer[4] == 0x47 && g_HR_Rx_Buffer[5] == 0x02 && g_HR_Rx_Buffer[6] == 0x20 )
		{//心率值
			hdr_value = g_HR_Rx_Buffer[11];
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: drv_getHdrValue--Sensor is skin on hdr_value = %d!!!!\r\n",hdr_value);
		}
		if(m_first_receive_flag == 0)
		{//使能心率返回命令判断是否使能ok
			if(	!(g_HR_Rx_Buffer[0] == 0x50 && g_HR_Rx_Buffer[1] == 0x81 && g_HR_Rx_Buffer[2] == 0x01 && g_HR_Rx_Buffer[3] == 0x00 &&
					g_HR_Rx_Buffer[4] == 0x45 && g_HR_Rx_Buffer[5] == 0x91 && g_HR_Rx_Buffer[6] == 0xF9) )
			{//[50 81]01 00 45 91 F9
				m_first_receive_flag = 0;
			}
		}
	}
	else if(g_HR_Rx_Buffer[0] == 0x81)
	{
		if( g_HR_Rx_Buffer[0] == 0x81 && g_HR_Rx_Buffer[1] == 0x12 && g_HR_Rx_Buffer[2] == 0x00 &&
			g_HR_Rx_Buffer[3] == 0x56 && g_HR_Rx_Buffer[4] == 0x01 )
		{//版本号
			sprintf((char *)str,"v%d.%d.%d;build %d%d%d%d%d%d%d%d_%d%d%d%d",
			g_HR_Rx_Buffer[5],g_HR_Rx_Buffer[6],g_HR_Rx_Buffer[7],
			g_HR_Rx_Buffer[8]-48,g_HR_Rx_Buffer[9]-48,g_HR_Rx_Buffer[10]-48,g_HR_Rx_Buffer[11]-48,
			g_HR_Rx_Buffer[12]-48,g_HR_Rx_Buffer[13]-48,g_HR_Rx_Buffer[14]-48,g_HR_Rx_Buffer[15]-48,
			g_HR_Rx_Buffer[17]-48,g_HR_Rx_Buffer[18]-48,g_HR_Rx_Buffer[19]-48,g_HR_Rx_Buffer[20]-48);
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: 2-HeartrateVersion:%s\n",str);
		}
		else if( g_HR_Rx_Buffer[0] == 0x81 && g_HR_Rx_Buffer[1] == 0x0E && g_HR_Rx_Buffer[2] == 0x00 &&
			 g_HR_Rx_Buffer[3] == 0x47 && g_HR_Rx_Buffer[4] == 0x02 && g_HR_Rx_Buffer[5] == 0x20 )
		{//心率值
			hdr_value = g_HR_Rx_Buffer[10];
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: drv_getHdrValue--Sensor is skin on hdr_value = %d!!!!\r\n",hdr_value);
		}
	}
	if(m_first_receive_flag == 0)
	{//使能心率返回命令判断是否使能ok
		if(	!(g_HR_Rx_Buffer[0] == 0x81 && g_HR_Rx_Buffer[1] == 0x01 && g_HR_Rx_Buffer[2] == 0x00 &&
				g_HR_Rx_Buffer[3] == 0x45 && g_HR_Rx_Buffer[4] == 0x91 && g_HR_Rx_Buffer[5] == 0xF9) )
		{//[81]01 00 45 91 F9
			m_first_receive_flag = 0;
		}
	}
	
	return (status == RESP_PACKET_ACK);
}

void sendHeartrateVersion(void)
{
//	SendPacket(p_cmd_get_version,sizeof(p_cmd_get_version),
//				 p_data_get_version,sizeof(p_data_get_version));
	sendCmd(m_cmd_get_version,sizeof(m_cmd_get_version));
}

/*获取心率指标等数据*/
void drv_sendHeartrateMetricData(void)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: drv_sendHeartrateMetricData\n");
//	SendPacket(p_cmd_get_updated_metric_values,sizeof(p_cmd_get_updated_metric_values),
//		         p_data_get_updated_metric_values,sizeof(p_data_get_updated_metric_values));
	sendCmd(m_cmd_get_updated_metric_values,sizeof(m_cmd_get_updated_metric_values));
}
static void hr_d_handler(void)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: hr_d_handler\n");
	if(am_hal_gpio_input_bit_read(BSP_GPIO_HR_D))//心率 D_AVAIL脚 显示心率有数据
	{
		SetHdrPinFlag(true);
	}
}
static void com_apollo2_gpio_int_init(uint32_t pin, uint32_t polarity)
{
	am_hal_gpio_pin_config(pin,AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_polarity_bit_set(pin,polarity);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(pin));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(pin));	

}
/*心率初始化 --对串口，硬件使能脚，输出脚配置*/
void drv_heartrate_init(void)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_heartrate_init!\n");
	//心率串口初始化
	bsp_uart_init(0,g_HR_UartConfig,g_HR_Rx_Buffer,g_HR_Tx_Buffer,HDR_BUFFER_SIZE);
//	bsp_uart_enable(0);

	am_hal_gpio_int_register(BSP_GPIO_HR_D, hr_d_handler);
	com_apollo2_gpio_int_init(BSP_GPIO_HR_D, AM_HAL_GPIO_RISING);

	am_hal_gpio_pin_config(BSP_GPIO_HR_EN,AM_HAL_PIN_OUTPUT);
	am_hal_gpio_out_bit_set(BSP_GPIO_HR_EN);
	vTaskDelay(200/portTICK_PERIOD_MS);
	m_first_receive_flag = 0;
}
void drv_heartrate_uninit(void)
{
	m_first_receive_flag = 3;//关闭心率
	drv_sendHdrHandler();
	bsp_uart_disable(0);
	
	
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(BSP_GPIO_HR_D));
	am_hal_gpio_pin_config(BSP_GPIO_HR_D,AM_HAL_PIN_DISABLE);
	am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(BSP_GPIO_HR_D));
	
	
	
	am_hal_gpio_pin_config(BSP_GPIO_HR_EN,AM_HAL_PIN_OUTPUT);
	am_hal_gpio_out_bit_clear(BSP_GPIO_HR_EN);
		
  DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_heartrate_uninit!\n");
}
/*使能心率*/
static void drv_heartrate_enable(void)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_heartrate_enable!\n");

//	SendPacket(p_cmd_enable_metric,sizeof(p_cmd_enable_metric),
//				 p_data_enable_metric,sizeof(p_data_enable_metric));
	sendCmd(m_cmd_enable_metric,sizeof(m_cmd_enable_metric));
}
static void drv_heartrate_disable(void)
{
//	SendPacket(p_cmd_enable_metric,sizeof(p_cmd_enable_metric),
//				 p_data_disable_metric,sizeof(p_data_disable_metric));
	sendCmd(m_cmd_disable_metric,sizeof(m_cmd_disable_metric));

	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_heartrate_disable!\n");
}
//static void drv_heartrate_powerdown(void)
//{
//	sendCmd(m_cmd_state_power_down,sizeof(m_cmd_state_power_down));

//	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_heartrate_powerdown!\n");
//}
/*获取心率版本号，存放的数组长度不要超过30
//! 如A3OS V5.4.8;build 20170113_1345 /A3OS V5.4.6;build 20161025_0822
*/
void drv_getHeartrateVersion(char strVersion[],uint8_t length)
{
	uint8_t i = length;
	
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: drv_getHeartrateVersion\n");
//	SendPacket(p_cmd_get_version,sizeof(p_cmd_get_version),
//			 p_data_get_version,sizeof(p_data_get_version));
	for(i = 0; i < length;i++)
		strVersion[i] = str[i];
}
/*
//!获取心率值
//!注意：使用此函数前必须使能心率,发送获取心率命令
*/
uint8_t drv_getHdrValue(void)
{
	return hdr_value;
}

void drv_clearHdrValue(void)
{
    hdr_value = 0;

}

/*发送命令 有序处理*/
void drv_sendHdrHandler(void)
{
	if(GetHdrPinFlag())
	{//D_ABAIL 有脉冲时，提示有数据，这个时候获取心率数值最佳
		SetHdrPinFlag(false);
		if( m_first_receive_flag == 0 )
		{//第一次发送使能心率
			hdr_value = 0;//心率值清零
			drv_heartrate_enable();
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_sendHdrHandler ---drv_heartrate_enable\n");
		}
		else if( m_first_receive_flag == 1 )
		{//第二次
			hdr_value = 0;
			m_first_receive_flag = 2;
			sendHeartrateVersion();//发送获取版本
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_sendHdrHandler ---sendHeartrateVersion\n");
		}
		else if( m_first_receive_flag == 2 )
		{
			m_first_receive_flag = 2;
			drv_sendHeartrateMetricData();//发送获取心率
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_sendHdrHandler ---sendHeartrateMetricData\n");
		}
		else if( m_first_receive_flag == 3 )
		{
			hdr_value = 0;
			m_first_receive_flag = 0;
			drv_heartrate_disable();//关心率
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_sendHdrHandler ---drv_heartrate_disable\n");
		}
	}
}
static void uart_cb(uint8_t *p_data)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:uart_cb\n");
	m_hdr_cb(DRV_HRD_EVT_UART_DATA, p_data, DRV_HDR_UART_RX_BUF_SIZE);
}
void drv_hdr_init(hdr_callback hdr_cb)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_hdr_init\n");
	uart0_handler_register(uart_cb,false);	
	m_hdr_cb = hdr_cb;
    hdr_value = 0;//心率值清零
	drv_heartrate_init();
}
void drv_hdr_uninit(void)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_hdr_uninit\n");
	drv_heartrate_uninit();
    hdr_value = 0;//心率值清零
	uart0_handler_unregister();
}
/*心率串口返回数据处理，主要保存版本号及心率值*/
void drv_receive_handler(uint8_t *p_data, uint8_t length)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_receive_handler\n");
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:Receive heart data_length = %d,data = {",length);
	for(int i = 0;i < 50;i++)
	{
	 DRV_HEARTRATE_LOG_PRINTF("0x%02X,",p_data[i]);
	}
	DRV_HEARTRATE_LOG_PRINTF("}\n");
	
  if(p_data[0] == 0x81)
	{
		if( p_data[0] == 0x81 && p_data[1] == 0x12 && p_data[2] == 0x00 &&
			p_data[3] == 0x56 && p_data[4] == 0x01 )
		{//版本号
			sprintf((char *)str,"v%d.%d.%d;build %d%d%d%d%d%d%d%d_%d%d%d%d",
			p_data[5],p_data[6],p_data[7],
			p_data[8]-48,p_data[9]-48,p_data[10]-48,p_data[11]-48,
			p_data[12]-48,p_data[13]-48,p_data[14]-48,p_data[15]-48,
			p_data[17]-48,p_data[18]-48,p_data[19]-48,p_data[20]-48);
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: 2-HeartrateVersion:%s\n",str);
		}
		else if( p_data[0] == 0x81 && p_data[1] == 0x0E && p_data[2] == 0x00 &&
			 p_data[3] == 0x47 && p_data[4] == 0x02 && p_data[5] == 0x20 )
		{//心率值
			hdr_value = p_data[10];
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: drv_getHdrValue--Sensor is skin on hdr_value = %d!!!!\r\n",hdr_value);
		}
		if(m_first_receive_flag == 0)
		{//使能心率返回命令判断是否使能ok
			if(	p_data[0] == 0x81 && p_data[1] == 0x01 && p_data[2] == 0x00 &&
					p_data[3] == 0x45 && p_data[4] == 0x91 && p_data[5] == 0xF9 )
			{//[81]01 00 45 91 F9 心率初始化ok后心率灯亮
				m_first_receive_flag = 1;
				DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:Sending heart rate initialization success!\n");
			}
			else
			{//心率使能不成功,再次发送
				m_first_receive_flag = 0;
				DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:Sending heart rate initialization failed!\n");
			}
		}
	}
}

