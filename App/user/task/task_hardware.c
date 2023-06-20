#include "task_step.h"
#include "gui_gps_test.h"
#include "task_config.h"
//#include "ndof/drv_ndof.h"
#include "Pedometer/Pedometer.h"
#include "Pedometer/PedoSupport.h"
#include "task_hardware.h"
#include "task_display.h"
#include "task_key.h"
#include "task_uart.h"
#include "task_timer.h"
#include "bsp_timer.h"
#include "drv_lcd.h"
#include "drv_spiflash.h"
#include "drv_ms5837.h"
#include "drv_key.h"

#if defined(HARDWARE_TEST)
#include "drv_motor.h"

TaskHandle_t 	TaskHardHandle = NULL;  
SemaphoreHandle_t Hard_Delete_Semaphore = NULL;
SemaphoreHandle_t Hard_Start_Semaphore = NULL;

//static float m_pres,m_temp,m_alt;
extern int16_t		g_CompassCaliStatus;
extern int akm_cal_init(void );

uint16_t 	g_CompassCaliNum;		//指北针校准次数
void DisplayTestSTEP(void)
{
	SetWord_t word = {0};
	char str[15];

        

	//将背景设置为白色
	LCD_SetBackgroundColor(LCD_WHITE);

	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("STEP",&word);

    word.x_axis +=80;
	word.y_axis = 26;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("count:",&word);
	word.y_axis += 78;
	memset(str,0,15);
	sprintf(str,"%d ",GetStepCount());
	LCD_SetString(str,&word);
}
void DisplayTestCompassCalibration(void)
{
	SetWord_t word = {0};

		//将背景设置为白色
	LCD_SetBackgroundColor(LCD_WHITE);

	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("COMPASS",&word);
	
	word.x_axis += 80;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("calibration ...",&word);
}

void DisplayTestCOMPASS(uint16_t angle)
{
	SetWord_t word = {0};
	char str[15];
	
	//将背景设置为白色
	LCD_SetBackgroundColor(LCD_WHITE);

	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("COMPASS",&word);

    word.x_axis +=80;
	word.y_axis = 26;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("angle:",&word);
	word.y_axis += 78;
	memset(str,0,15);
	sprintf(str,"%d ",angle);
	LCD_SetString(str,&word);
}

void DisplayTestPressure(void)
{
	SetWord_t word = {0};
	char str[15];
	
	//将背景设置为白色
	LCD_SetBackgroundColor(LCD_WHITE);

	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("PRESSURE",&word);
	
	word.x_axis = 100;
	memset(str,0,sizeof(str));
	sprintf(str,"P %.1f",m_pres);
	LCD_SetString(str,&word);
	
	word.x_axis += 30;
	memset(str,0,sizeof(str));
	sprintf(str,"A %.1f",m_alt);
	LCD_SetString(str,&word);
	
	word.x_axis += 30;
	memset(str,0,sizeof(str));
	sprintf(str,"T %.1f",m_temp);
	LCD_SetString(str,&word);
}

void DisplayTestMotor(void)
{
	SetWord_t word = {0};
	
	//将背景设置为白色
	LCD_SetBackgroundColor(LCD_WHITE);

	word.x_axis = 100;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_DisplayGTString("马达+字库",&word);
}


void HardewareTaskClose( void)
{
	if (TaskHardHandle !=NULL)
	{
		 xSemaphoreGive(Hard_Delete_Semaphore);
	}
}

void HardewareTaskCreate( void)
{
  ScreenState = Menu_Index;
	if( TaskHardHandle != NULL )
	{
		xSemaphoreGive(	Hard_Start_Semaphore );
	}
}

void MsgHardware(ScreenState_t ScreenSta,uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	ScreenState_t MaxScreen = DISPLAY_SCREEN_TEST_MOTOR;

	switch( Key_Value )
	{
		case KEY_OK:
			if( Menu_Index == DISPLAY_SCREEN_TEST_GPS)
			{
				ScreenState = DISPLAY_SCREEN_TEST_GPS_DETAILS;
				msg.cmd = MSG_UPDATE_TEST_GPS;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			}break;
//			if( Menu_Index == DISPLAY_SCREEN_TEST_COMPASS)
//			{
//				DRV_NDOF_LOG_PRINTF("[MsgHardware]:KEY_OK\r\n" );
//			
//				ScreenState = DISPLAY_SCREEN_TEST_COMPASS_CALIBRATION;
//				g_CompassCaliNum = 0;
//				g_CompassCaliStatus = 0;
//				akm_cal_init();
//				msg.cmd = MSG_UPDATE_TEST_COMPASS_CALIBRATION;
//				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
//			}
			break;
//		case KEY_DOWN:
//			DRV_NDOF_LOG_PRINTF("[MsgHardware]:KEY_DOWN\r\n" );
//			Menu_Index++;
//			if( Menu_Index > MaxScreen )
//			{
//				Menu_Index = DISPLAY_SCREEN_TEST_STEP;
//			}
//			HardewareTaskClose();
//			vTaskDelay(2000/portTICK_PERIOD_MS);
//			HardewareTaskCreate();
//			break;
//		case KEY_UP:
//			Menu_Index--;
//			if( Menu_Index < DISPLAY_SCREEN_TEST_STEP )
//			{
//				Menu_Index = MaxScreen;
//			}
//			HardewareTaskClose();
//			vTaskDelay(2000/portTICK_PERIOD_MS);
//			HardewareTaskCreate();
//            
//			break;
		case KEY_BACK:
			HardewareTaskClose();
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
     }
}


ScreenState_t HardwareInit( ScreenState_t ScreenSta )
{
	DISPLAY_MSG  msg = {0};
	
	switch( ScreenSta )
	{
		case DISPLAY_SCREEN_TEST_STEP:

			break;

		case DISPLAY_SCREEN_TEST_COMPASS:
		case DISPLAY_SCREEN_TEST_COMPASS_CALIBRATION:

			break;
		case DISPLAY_SCREEN_TEST_GPS:
			CreateGPSTask();
			break;
		case DISPLAY_SCREEN_TEST_HDR:
			CreateUartTask();
			drv_heartrate_init();
			break;
		case DISPLAY_SCREEN_TEST_FLASH:
			drv_spiflash_enable();
			break;
		case DISPLAY_SCREEN_TEST_MOTOR:
			msg.cmd = MSG_UPDATE_TEST_MOTOR;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		
			drv_motor_enable();
			break;
		default:
			break;		
	}
	
	return ScreenSta;
}


void TaskHardware( void* pvParameter)
{
	(void)pvParameter;
	DISPLAY_MSG  msg = {0};
	ScreenState_t ScreenSta = ScreenState;
	BaseType_t pdreturn = pdFALSE;
	uint16_t		g_Angle;
	Hard_Start_Semaphore = xSemaphoreCreateCounting(10,0);
  Hard_Delete_Semaphore = xSemaphoreCreateCounting(10,0);

	while(1)
	{
		if( xSemaphoreTake(Hard_Start_Semaphore,portMAX_DELAY) == pdTRUE )
		{
		
			ScreenSta = HardwareInit(ScreenState);		
			pdreturn = pdFALSE;
		
			while( pdreturn == pdFALSE )
			{
				switch( ScreenSta )
				{
					case DISPLAY_SCREEN_TEST_STEP:	
            msg.cmd = MSG_UPDATE_TEST_STEP;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						break;
				
					case DISPLAY_SCREEN_TEST_COMPASS:
					case DISPLAY_SCREEN_TEST_COMPASS_CALIBRATION:
						ScreenSta = DISPLAY_SCREEN_TEST_COMPASS;
						if(ScreenState == DISPLAY_SCREEN_TEST_COMPASS)
						{
//							g_Angle = mag_calc_angle();
							
							msg.cmd = MSG_UPDATE_TEST_COMPASS;
							msg.value = g_Angle;
							xQueueSend(DisplayQueue, &msg,portMAX_DELAY);
							pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,350/portTICK_PERIOD_MS);
						}
						else if(ScreenState == DISPLAY_SCREEN_TEST_COMPASS_CALIBRATION)
						{
//							g_Angle = mag_calc_angle();
					
							if(g_CompassCaliStatus == 3)
							{
								//校准完成,马达振动
								drv_motor_enable();
								vTaskDelay(500/portTICK_PERIOD_MS);
								drv_motor_disable();
								//返回指北针界面
								ScreenState = DISPLAY_SCREEN_TEST_COMPASS;
								msg.cmd = MSG_UPDATE_TEST_COMPASS;
								xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
							}
							else
							{
								g_CompassCaliNum++;
								if(g_CompassCaliNum % 20 == 0)
								{
									if(g_CompassCaliNum == 160)
									{
										g_CompassCaliNum = 0; 
									}
									msg.value = g_CompassCaliNum / 20;
									msg.cmd = MSG_UPDATE_TEST_COMPASS_CALIBRATION;
									xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
								}
								pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,30/portTICK_PERIOD_MS);
							}
						}
						else
						{
							pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						}
						break;
					case DISPLAY_SCREEN_TEST_GPS:
					case DISPLAY_SCREEN_TEST_GPS_DETAILS:
						msg.cmd = MSG_UPDATE_TEST_GPS;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						break;
					case DISPLAY_SCREEN_TEST_HDR:
						msg.cmd = MSG_UPDATE_TEST_HEARTRATE;//心率值
						msg.value = drv_getHdrValue();
					  xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,1000/portTICK_PERIOD_MS);
						break;
					case DISPLAY_SCREEN_TEST_FLASH:
						msg.cmd = MSG_UPDATE_TEST_FLASH;//FLASH ID
						msg.value = drv_spiflash_id_get();
					  xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,2000/portTICK_PERIOD_MS);
						break;
					case DISPLAY_SCREEN_TEST_PRESSURE:
						drv_ms5837_data_get(&m_pres,&m_alt,&m_temp);
					
						msg.cmd = MSG_UPDATE_TEST_PRESSURE;
						xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
						pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,3000/portTICK_PERIOD_MS);
						break;
					case DISPLAY_SCREEN_TEST_MOTOR:
						drv_motor_toggle();
						
						pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,500/portTICK_PERIOD_MS);
						break;
					default:
						pdreturn = xSemaphoreTake(Hard_Delete_Semaphore,portMAX_DELAY);
						break;
				}
			}
		
			if(pdreturn == pdTRUE)
			{				
				//退出相应模式
				switch(ScreenSta)
				{
					case DISPLAY_SCREEN_TEST_STEP:
//						CloseStepTask();
						break;

					case DISPLAY_SCREEN_TEST_COMPASS:
					case DISPLAY_SCREEN_TEST_COMPASS_CALIBRATION:
						//地磁进入待机模式

						break;
					case DISPLAY_SCREEN_TEST_GPS:
						CloseGPSTask();
						break;
					case DISPLAY_SCREEN_TEST_HDR:
						drv_heartrate_uninit();
						CloseUartTask();
						break;
					case DISPLAY_SCREEN_TEST_FLASH:
						drv_spiflash_disable();
						break;
					case DISPLAY_SCREEN_TEST_MOTOR:
						drv_motor_disable();
						break;
					default:
						break;
				}
			}
		}
	}
}
#endif


