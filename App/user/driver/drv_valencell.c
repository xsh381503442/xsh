/**
*Module:HeartRate valencell BW1.2
*Chip:
*Version: V 0.0.1
*Time: 2018/03/14
**/
#include <stdio.h>
#include "drv_heartrate.h"
#include "bsp_uart.h"
#include "drv_config.h"
#include "bsp.h"
#include "bsp_iom.h"
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


extern SemaphoreHandle_t Appolo_I2C_Semaphore;

#define		HEARTRATE_ADDRESS		0x44

//static uint8_t g_HR_Tx_Buffer[HDR_BUFFER_SIZE];
static uint8_t g_HR_Rx_Buffer[HDR_BUFFER_SIZE];
//static uint16_t packetLen;
//static uint16_t packetCrc;
//static uint8_t  packetIdx;
//static uint8_t  lenPrefix = 2;

static uint8_t m_first_receive_flag = 0;
static uint8_t  hdr_value = 0;//心率存放
//static bool enableHeartFlag = false;//使能心率标志
static char str[30];//心率版本号存放
//static hdr_callback m_hdr_cb;

//Get A3OS version  当前版本A3OS V5.4.8;build 20170113_1345
//static uint8_t p_cmd_get_version[] = {0x56};
//static uint8_t p_data_get_version[] = {0x01};
static uint8_t m_cmd_get_version[] = {0x44,0x04,0x04,0x1B,0x00,0x01};//running模式

static uint8_t m_cmd_enable_metric[] = {0x44,0x01,0x01};
static uint8_t m_cmd_disable_metric[] = {0x44,0x01,0x02};

static uint8_t m_cmd_get_updated_metric_values[] = {0x44,0x03,0x08,0x20,0x11};
//static uint8_t p_data_get_updated_skin_values[] = {0x01,0x22};

static  uint32_t hdr_zero_count = 0;//SignalQuality返回无效计数

const am_hal_iom_config_t g_HR_IOMConfig =
{
    .ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
    .ui32ClockFrequency = AM_HAL_IOM_100KHZ,
    .bSPHA = 0,
    .bSPOL = 0,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
};



static void drv_heartrate_enable(void)
{
	xSemaphoreTake( Appolo_I2C_Semaphore, portMAX_DELAY );
	
	// Enable IOM
	bsp_iom_enable(BSP_HEARTRATE_IOM,g_HR_IOMConfig);
}

static void drv_heartrate_disable(void)
{
	//DRV_NDOF_LOG_PRINTF(" drv_bmi160_disable\n");
	
	//Disable IOM
	bsp_iom_disable(BSP_HEARTRATE_IOM);
	
	xSemaphoreGive( Appolo_I2C_Semaphore );	
}

static void sendCmd(const uint8_t *p_data, uint16_t data_length)
{	
	 int i;

    am_hal_iom_buffer(32) sCommand ={0};
	am_hal_iom_buffer(32) sResult = {0};
    
    for (i = 0; i < data_length; i++)
    {
        sCommand.bytes[i] =  p_data[i];
    }
    
    drv_heartrate_enable();
	if( am_hal_iom_i2c_write(BSP_HEARTRATE_IOM,HEARTRATE_ADDRESS, sCommand.words, data_length,  AM_HAL_IOM_RAW) == AM_HAL_IOM_SUCCESS )
	{
		//vTaskDelay(150/portTICK_PERIOD_MS);
		if( p_data[2] == 0x08 )
		{
			data_length = 9;
		}
		else
		{
			data_length = 4;
		}
		am_hal_iom_i2c_read(BSP_HEARTRATE_IOM,HEARTRATE_ADDRESS,sResult.words,data_length,AM_HAL_IOM_RAW);
		drv_heartrate_disable();
		for (i = 0; i < 9; i++)
		{
			g_HR_Rx_Buffer[i] = sResult.bytes[i];
			//DRV_HEARTRATE_LOG_PRINTF("0x%02X,",g_HR_Rx_Buffer[i]);
		}
		//DRV_HEARTRATE_LOG_PRINTF("-----\n");
		if(sCommand.bytes[0] == 0x44 && sCommand.bytes[1] == 0x01 && sCommand.bytes[2] == 0x01)
		{//发送打开心率命令时 如果接受命令错误,下次重发
			if(!(g_HR_Rx_Buffer[0] == 0x44 && g_HR_Rx_Buffer[1] == 0x02 && g_HR_Rx_Buffer[2] == 0x10 && g_HR_Rx_Buffer[3] == 0x00))
			{
				m_first_receive_flag = 0;
				DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate910]:sendCmd open hdr ---->i2c_read is fail!\n");
			}
		}
		if( (g_HR_Rx_Buffer[2] == 0x20) && (g_HR_Rx_Buffer[3] == 0x20) && ( ( g_HR_Rx_Buffer[8] & 0x01 ) == 0 ) )
		{
			hdr_value = g_HR_Rx_Buffer[5];
			hdr_zero_count = 0;
		}
		else
		{
		   	hdr_zero_count++;
		    if (hdr_zero_count > 5)
		    {
				hdr_value = 0;
		    }
		}
	}
	else
	{
		if(sCommand.bytes[0] == 0x44 && sCommand.bytes[1] == 0x01 && sCommand.bytes[2] == 0x01)
		{//发送打开心率命令时 如果发送失败,下次重发
			m_first_receive_flag = 0;
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate910]:sendCmd open hdr -->i2c_write is fail!\n");
		}
		drv_heartrate_disable();
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
	//uint8_t status = RESP_PACKET_ACK;
	//bool stop = false;
	//uint8_t inData;
	//packetIdx = 0;
	//uint16_t tmpCrc,i = 0;
	return false;

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
//	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]: drv_sendHeartrateMetricData\n");
//	SendPacket(p_cmd_get_updated_metric_values,sizeof(p_cmd_get_updated_metric_values),
//		         p_data_get_updated_metric_values,sizeof(p_data_get_updated_metric_values));
	sendCmd(m_cmd_get_updated_metric_values,sizeof(m_cmd_get_updated_metric_values));
}

/*心率初始化 --对串口，硬件使能脚，输出脚配置*/
void drv_heartrate_init(void)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_heartrate_init!\n");


	//心率I2C初始化

	am_hal_gpio_out_bit_set(BSP_GPIO_HR_EN);

	//vTaskDelay(150/portTICK_PERIOD_MS);
	am_util_delay_ms(160);

	m_first_receive_flag = 0;
	hdr_zero_count = 0;
}
void drv_heartrate_uninit(void)
{
	m_first_receive_flag = 3;//关闭心率
	
	drv_sendHdrHandler();

	am_hal_gpio_out_bit_clear( BSP_GPIO_HR_EN );
		
    DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_heartrate_uninit!\n");
}
/*使能心率*/
static void drv_heartrate_start(void)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_heartrate_enable!\n");

//	SendPacket(p_cmd_enable_metric,sizeof(p_cmd_enable_metric),
//				 p_data_enable_metric,sizeof(p_data_enable_metric));
	sendCmd(m_cmd_enable_metric,sizeof(m_cmd_enable_metric));
}
static void drv_heartrate_close(void)
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


		if( m_first_receive_flag == 0 )
		{//第一次发送使能心率
			hdr_value = 0;//心率值清零
			m_first_receive_flag = 1;
			sendHeartrateVersion();//配置为running模式
			drv_heartrate_start();
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_sendHdrHandler ---drv_heartrate_enable\n");
		}
		else if( m_first_receive_flag == 1 )
		{//第二次
			hdr_value = 0;
			m_first_receive_flag = 2;
			//sendHeartrateVersion();//发送获取版本
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_sendHdrHandler ---sendHeartrateVersion\n");
		}
		else if( m_first_receive_flag == 2 )
		{
			m_first_receive_flag = 2;
			drv_sendHeartrateMetricData();//发送获取心率
//			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_sendHdrHandler ---sendHeartrateMetricData\n");
		}
		else if( m_first_receive_flag == 3 )
		{
			hdr_value = 0;
			m_first_receive_flag = 0;
			drv_heartrate_close();//关心率
			DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_sendHdrHandler ---drv_heartrate_disable\n");
		}
	
}

void drv_hdr_init(hdr_callback hdr_cb)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_hdr_init\n");
//	m_hdr_cb = hdr_cb;
    hdr_value = 0;//心率值清零
	drv_heartrate_init();
}
void drv_hdr_uninit(void)
{
	DRV_HEARTRATE_LOG_PRINTF("[drv_heartrate]:drv_hdr_uninit\n");
	drv_heartrate_uninit();
    hdr_value = 0;//心率值清零
}


