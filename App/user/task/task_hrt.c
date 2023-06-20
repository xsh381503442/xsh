#include "task_hrt.h"
//#include "rtt_log.h"
#include "drv_hrt.h"
#include "task_step.h"
//#include "dev_acc_gyro.h"

#include "si117x_config.h"
#include "si117x_functions.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"

#include "Ohrdriver.h"
#include "com_ringbuffer.h"
#include "drv_lsm6dsl.h"
#include "timer_app.h"

static TimerHandle_t m_timer = NULL;
//static uint8_t m_timer_evt = 0;

#define MOUDLE_INFO_ENABLED 							1
#define MOUDLE_DEBUG_ENABLED   					  1
#define MOUDLE_NAME                     "[TASK_HRT]:"

#if DEBUG_ENABLED == 1 && MOUDLE_INFO_ENABLED == 1
#define INFO_PRINTF(...)                 SEGGER_RTT_printf(0,##__VA_ARGS__)     
#define INFO_HEXDUMP(p_data, len)        SEGGER_RTT_Write(0, (const char*)p_data, len);        
#else
#define INFO_PRINTF(...) 
#define INFO_HEXDUMP(p_data, len)             
#endif

#if DEBUG_ENABLED == 1 && MOUDLE_DEBUG_ENABLED == 1
#define DEBUG_PRINTF(...)                SEGGER_RTT_printf(0,##__VA_ARGS__)    
#define DEBUG_HEXDUMP(p_data, len)       SEGGER_RTT_Write(0, (const char*)p_data, len);        
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif

#define TASK_HRT_PRIORITY            5

static void task_ohr_timer_start(uint32_t timeout_msec, bool is_isr);
//static void task_ohr_timer_stop(bool is_isr);


static TaskHandle_t task_handle = NULL;
static uint32_t m_evt = 0;
static uint8_t m_buffer[256];
static uint32_t m_buffer_len = 0;

typ_fifodata StepAccFifoData;

static uint8_t acc_buffer[512]; ////�洢���ʼ�����Ҫ��acc����
struct ring_buffer_header acc_rb; //�洢���ʼ�����Ҫ��acc���Ͷ���


void task_hrt_handler(uint32_t evt, bool is_isr, uint8_t *p_data, uint32_t len)
{
	if(evt == HRT_EVT_HRT_DISABLE)//���̹ر������жϣ��жϲ��ٴ�Ϲرչ���
	#ifdef WATCH_COD_BSP
		drv_hrt_irq_disable();
	#endif
    m_evt = evt;
		m_buffer_len = 0;
		if(p_data != NULL && len != 0)
		{
			memset(m_buffer,0,sizeof(m_buffer));
			memcpy(m_buffer,p_data, len);
			m_buffer_len = len;		
		}	
    if (is_isr) {
    		BaseType_t yield_req = pdFALSE;
    		vTaskNotifyGiveFromISR(task_handle, &yield_req);
    		portYIELD_FROM_ISR(yield_req);	
    }
    else {
        xTaskNotifyGive( task_handle );
    }	  

}
bool hr_enabled = false;
static bool m_initialized = false;
#ifdef HRT_DEBUG_TEST
uint32_t hrt_int_num = 0;
uint32_t hrt_int_count = 0;
uint32_t hrt_int_count_callback = 0;
#endif
#ifdef HEARTRAT_FPC_TEST
extern uint32_t test_press;
extern uint32_t test_heart;
#endif
static void task( void *pvParameters )
{

	static uint8_t hr_int = 0;
	//static uint8_t hr_4hz = 0;
	#ifdef WATCH_COD_BSP
	drv_i2c_hw1_init();//����
	drv_hrt_init(task_hrt_handler);
	drv_hrt_irq_disable();
	#endif
	//Initial_Pedometer(0);
	//OHRL_init((uint32_t *)0xFB000, 0, NULL);
	hr_enabled =false;
	//task_ohr_timer_start(250, NULL);
	//vTaskDelay(1000);
	//task_hrt_handler(HRT_EVT_HRT_ENABLE, false, NULL, 0);
	//static uint16_t counter = 0;
	static uint8_t wristoff_timer = 0;
	while(1)
	{
		
		(void) ulTaskNotifyTake(pdTRUE,portMAX_DELAY); 
		 {
		 	#ifdef WATCH_COD_BSP
				switch(m_evt)
				{
					case HRT_EVT_TIMER_INTERRUPT:
						if(hr_enabled==false)
						{
							//ʹ�������㷨��δʹ�����ʿ�Ʋ��㷨
							StepAccFifoData.datalen = 2*dev_hrt_acc_fifo_data(StepAccFifoData.data.words); // gsensor�����ݳ������ֽ�����
							OHRL_pushXYZ(&StepAccFifoData);
							OHRL_algorithm();						
						
						}
						else
					    {
					   		 #if  0//���ʲ���
					   			uint8_t data1;
								 drv_i2c_hw1_enable();
								data1= Si117xReadFromRegister(REG_PART_ID);
								drv_i2c_hw1_disable();
								if ((data1 ==0x81))
								{

									OHRL_data.heartrate = 0xFF;	
								}
							#endif
						
							if(wristoff_timer<255)
							{
								++wristoff_timer;
							}
							
							//DEBUG_PRINTF("PPG value = %d, maxmin = %d, Heartrate = %d\r\n",OHRL_data.ppg,OHRL_data.ppg_maxmin,OHRL_data.heartrate);
							if(PedoSummary.PedoSleep_flag==1 && OHRL_data.onwrist>0)
							{
								    //�˴����Խ�gsensor�����ˣ����ѵ�ʱ����ִ�� drv_hrt_restart();			
									
										StepAccFifoData.datalen = 2*dev_hrt_acc_fifo_data(StepAccFifoData.data.words);
										OHRL_pushXYZ(&StepAccFifoData);
										OHRL_algorithm();
										if(PedoSummary.PedoSleep_flag == 0)//gsensor ����
										{
												drv_hrt_restart();	//�������ʴ�����										
											if(wristoff_timer>240)OHRL_reset();//���󳬹�1���ӣ������㷨
										}
							}
							else
							{
								if(hr_int==0)
								{
									drv_hrt_int_handler();
								}
								hr_int=0;
							}
#ifdef HEARTRAT_FPC_TEST

							//counter++;
							//if ((counter % 4) == 0 || (counter ==1))
							{
								float pres,temp,alt;
								pres = 0 ;
								drv_ms5837_data_get(&pres,&alt,&temp);

								if((pres >300)&& (pres < 1100))
								{
									test_press = 1;
								}
								else
								{
									test_press = 0;
								}
								 uint8_t data1;
								 drv_i2c_hw1_enable();
								data1= Si117xReadFromRegister(REG_PART_ID);
								drv_i2c_hw1_disable();
								if ((data1 ==0x81)&& (OHRL_data.ppg != 0))
								{

									test_heart = 1;	
								}
								else
								{
								    test_heart = 0;
									//OHRL_data.ppg = 0;
								}
								DISPLAY_MSG  msg = {0};		
								ScreenState = DISPLAY_SCREEN_TEST;
								msg.cmd = MSG_DISPLAY_SCREEN;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);	
							}
#endif
						}
						break;
					case HRT_EVT_PPG_INTERRUPT:{
						//DEBUG_PRINTF(MOUDLE_NAME"HRT_EVT_PPG_INTERRUPT\r\n");
						hr_int = 2;
						if (OHRL_data.onwrist == 0)
						{
							wristoff_timer = 0;
						}
						
						drv_hrt_int_handler();
						
						
						if(PedoSummary.PedoSleep_flag==1 && OHRL_data.onwrist>0)
						{
								drv_hrt_suspend();//������Ҿ�ֹ״̬��ֹͣppg������							
						}
						
					}break;   			
					case HRT_EVT_HRT_ENABLE:{
						DEBUG_PRINTF(MOUDLE_NAME"HRT_EVT_HRT_ENABLE\r\n");
						task_ohr_timer_start(250, NULL);
						OHRL_data.heartrate =0;
						OHRL_data.ppg = 100;
						OHRL_data.ppg_maxmin = 0;
						dev_hrt_acc_init();
						dev_hrt_acc_pwron();
						drv_hrt_enable();
						hr_enabled =true;
					}break; 			
					case HRT_EVT_HRT_DISABLE:{
						DEBUG_PRINTF(MOUDLE_NAME"HRT_EVT_HRT_DISABLE\r\n");
						drv_hrt_disable();
						OHRL_data.heartrate =0;
						OHRL_data.ppg =0;
						OHRL_data.ppg_maxmin = 0;
						//dev_acc_pwroff();
						drv_lsm6dsl_cod_close();
						//task_ohr_timer_stop(NULL);
						hr_enabled = false;
					}break; 		
				}				
		 #endif
		 }			 
	
	}

}

void task_ohrcounter_handler(uint32_t evt, bool is_isr, uint8_t *p_data, uint32_t len)
{
    m_evt = evt;
		m_buffer_len = 0;
		if(p_data != NULL && len != 0)
		{
			memset(m_buffer,0,sizeof(m_buffer));
			memcpy(m_buffer,p_data, len);
			m_buffer_len = len;		
		}	
    if (is_isr) {
    		
    		BaseType_t yield_req = pdFALSE;
    		vTaskNotifyGiveFromISR(task_handle, &yield_req);
    		portYIELD_FROM_ISR(yield_req);	
    }
    else {
        xTaskNotifyGive( task_handle );
    }	    
}

static void timer_callback(TimerHandle_t xTimer)
{
//	INFO_PRINTF(MOUDLE_NAME"timer_callback\r\n");
	task_ohrcounter_handler(HRT_EVT_TIMER_INTERRUPT, false, NULL, 0);
}

void task_ohr_timer_start(uint32_t timeout_msec, bool is_isr)
{
	if(is_isr) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerChangePeriodFromISR( m_timer,  timeout_msec / portTICK_PERIOD_MS ,&xHigherPriorityTaskWoken ) != pdPASS )
		{
//			ERROR_HANDLER(ERROR_NO_MEM);
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	} else {
	
		if( xTimerChangePeriod( m_timer, timeout_msec / portTICK_PERIOD_MS, 100 ) != pdPASS )
		{
//			ERROR_HANDLER(ERROR_NO_MEM);
		}
	}
}

void task_ohr_timer_stop(bool is_isr)
{
	if(is_isr)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if( xTimerStopFromISR( m_timer, &xHigherPriorityTaskWoken ) != pdPASS )
		{
//			ERROR_HANDLER(ERROR_NO_MEM);
		}
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}	
	}
	else{
	
		if (pdPASS != xTimerStop(m_timer, 100))
		{	
//			ERROR_HANDLER(ERROR_NO_MEM);
		}
	}	

}


void task_hrt_init(void)
{
	BaseType_t ret = xTaskCreate(task, "hrt", 1024, 0, TASK_HRT_PRIORITY, &task_handle);
	if(ret != pdPASS || task_handle == NULL)
	{

	}
	m_timer = xTimerCreate("ohr",
						 ( 1000 / portTICK_PERIOD_MS ),
						 pdTRUE,
						 NULL,
						 timer_callback);
	if (NULL == m_timer)
	{
//		ERROR_HANDLER(ERROR_NO_MEM);
	}
	
	//ring_buffer_init(&acc_rb, acc_buffer, 256);
}


void task_hrt_start(void)
{
#ifdef WATCH_COD_BSP
	if(m_initialized)
	{
		return;
	}
	 DEBUG_PRINTF(MOUDLE_NAME"task_hrt_start\r\n");
	OHRL_data.heartrate =0;
	m_initialized = true;
	task_hrt_handler(HRT_EVT_HRT_ENABLE, false, NULL, 0);
	timer_app_hdr_start();
	
	ring_buffer_init(&acc_rb, acc_buffer, 256);
	#ifdef HRT_DEBUG_TEST
	hrt_int_num = 0;
	hrt_int_count = 0;
	hrt_int_count_callback = 0;
	#endif
	g_HeartNum = 0;
#endif
	

}

void task_hrt_close(void)
{
#ifdef WATCH_COD_BSP
   if(!m_initialized)
	{
		return;
	}
    DEBUG_PRINTF(MOUDLE_NAME"task_hrt_close\r\n");
   	m_initialized = false;
	drv_hrt_irq_disable();
	task_ohr_timer_stop(NULL);
	timer_app_hdr_stop(false);
	task_hrt_handler(HRT_EVT_HRT_DISABLE, false, NULL, 0);
	for(int i = 0; i < HEARTBUF_SIZE; i++)
	{
		g_HeartBuf[i] = 0;
	}
	
	g_HeartNum = 0;
	OHRL_reset();
#endif


}

