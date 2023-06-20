#include "gui_gps_test.h"
#include "drv_lcd.h"
#include "drv_heartrate.h"
#include "drv_key.h"
#include "task_hrt.h"
#include "task_tool.h"
#include "task_display.h"
#include "task_gps.h"

#define GUI_GPS_TEST_LOG_ENABLED    1
#if DEBUG_ENABLED == 1 && GUI_GPS_TEST_LOG_ENABLED == 1
	#define GUI_GPS_TEST_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_GPS_TEST_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_GPS_TEST_WRITESTRING(...)
	#define GUI_GPS_TEST_PRINTF(...)		        
#endif
uint16_t g_Second = 0;//����ʱ�������ʱˢ�´�����1��1��,0~120��һ����ʱȦ
uint8_t g_status = 0;//����λ�ı�־

void DisplayTestGPS(GPSMode *gps)
{
	SetWord_t word = {0};
	char str[15];
	uint8_t latint,lonint;
	uint32_t latdec,londec;
//	uint8_t gps_n = 0,bds_n = 0;
	uint8_t a[12] ={0},t = 0;
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_WHITE);

	word.x_axis = 10;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("TEST",&word);
	
	word.x_axis += (24 + 5);
	LCD_SetRectangle(word.x_axis,2,0,218,LCD_BLACK,0,0,LCD_FILL_DISABLE);
	
	word.x_axis += 7;
	word.y_axis = 96;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	LCD_SetString("GPS",&word);
	
	word.y_axis += 26;
	LCD_SetString("BDS",&word);
	
		//�������������ź�ǿ��
//	if(gps->possl_gpsnum == 0)
//	{
//		gps_n = 0;
//	}
//	else if(gps->possl_gpsnum < 3)
//	{
//		gps_n = 1;
//	}
//	else if(gps->possl_gpsnum < 5)
//	{
//		gps_n = 2;
//	}
//	else if(gps->possl_gpsnum < 7)
//	{
//		gps_n = 3;
//	}
//	else if(gps->possl_gpsnum < 9)
//	{
//		gps_n = 4;
//	}
//	else
//	{
//		gps_n = 5;
//	}
//	
//	if(gps->possl_bdnum == 0)
//	{
//		bds_n = 0;
//	}
//	else if(gps->possl_bdnum < 3)
//	{
//		bds_n = 1;
//	}
//	else if(gps->possl_bdnum < 5)
//	{
//		bds_n = 2;
//	}
//	else if(gps->possl_bdnum < 7)
//	{
//		bds_n = 3;
//	}
//	else if(gps->possl_bdnum < 9)
//	{
//		bds_n = 4;
//	}
//	else
//	{
//		bds_n = 5;
//	}
	
	word.x_axis += 26;
	word.y_axis = 49 + 6;
	
//	//����
//	LCD_SetRectangle(word.x_axis,12,0,218,LCD_WHITE,0,0,LCD_FILL_DISABLE);
//	
//	//GPS�ź�ǿ��
//	for(i = 0;i < 5; i++)
//	{
//		LCD_SetRectangle(word.x_axis,1,word.y_axis,6,LCD_BLACK,0,0,LCD_FILL_DISABLE);
//		LCD_SetRectangle(word.x_axis,12,word.y_axis,1,LCD_BLACK,0,0,LCD_FILL_DISABLE);
//		LCD_SetRectangle(word.x_axis,12,word.y_axis+5,1,LCD_BLACK,0,0,LCD_FILL_DISABLE);
//		LCD_SetRectangle(word.x_axis+11,1,word.y_axis,6,LCD_BLACK,0,0,LCD_FILL_DISABLE);
//		if(i < gps_n)
//		{
//			LCD_SetRectangle(word.x_axis+1,10,word.y_axis+1,4,LCD_RED,0,0,LCD_FILL_DISABLE);
//		}
//		
//		word.y_axis += 8;
//	}
//	
//	word.y_axis += 4 + 20 + 6;
//	//BDS�ź�ǿ��
//	for(i = 0;i < 5; i++)
//	{
//		LCD_SetRectangle(word.x_axis,1,word.y_axis,6,LCD_BLACK,0,0,LCD_FILL_DISABLE);
//		LCD_SetRectangle(word.x_axis,12,word.y_axis,1,LCD_BLACK,0,0,LCD_FILL_DISABLE);
//		LCD_SetRectangle(word.x_axis,12,word.y_axis+5,1,LCD_BLACK,0,0,LCD_FILL_DISABLE);
//		LCD_SetRectangle(word.x_axis+11,1,word.y_axis,6,LCD_BLACK,0,0,LCD_FILL_DISABLE);
//		if(i < bds_n)
//		{
//			LCD_SetRectangle(word.x_axis+1,10,word.y_axis+1,4,LCD_RED,0,0,LCD_FILL_DISABLE);
//		}
//		
//		word.y_axis += 8;
//	}
//	word.forecolor = LCD_BLACK;
//	word.x_axis += 12+6;
//	LCD_SetRectangle(word.x_axis,108,88,2,LCD_BLACK,0,0,LCD_FILL_DISABLE);
	for(uint8_t i = 0;i < 12;i++)
	{
		a[i] = gps->slmsg[i].sn;
	}
	for(uint8_t i = 0;i < 11;i++)
	{
		for(uint8_t j = i+1;j < 12;j++)
		{
			if(a[i] < a[j])
			{
				t = a[i];
				a[i] = a[j];
				a[j] = t;
			}
		} 
	}
	word.y_axis = 26;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("Use",&word);
	word.y_axis +=38;
	memset(str,0,15);
//	sprintf(str,"%d,%d",gps->svnum,gps->bdnum);
	sprintf(str,"%d,%d",gps->possl_gpsnum,gps->possl_bdnum);
	LCD_SetString(str,&word);
	word.y_axis +=48;
	LCD_SetString("View",&word);
	word.y_axis +=48;
	memset(str,0,15);
	sprintf(str,"%d,%d",gps->svnum,gps->bdnum);
	LCD_SetString(str,&word);
	
	word.x_axis += 28;
	word.y_axis = 6;
	LCD_SetString("status:see the next",&word);
//	word.y_axis += 98;
//	memset(str,0,15);
//	sprintf(str,"%d,%d,%d",a[0],a[1],a[2]);
//	LCD_SetString(str,&word);
	
	word.x_axis += 28;
	word.y_axis = 26;
	LCD_SetString("lon",&word);
	word.y_axis += 78;
	memset(str,0,15);
	lonint = gps->longitude/GPS_SCALE_INT;
	londec = gps->longitude%GPS_SCALE_INT;
	sprintf(str,"%d.%05d",lonint,londec);
	LCD_SetString(str,&word);
	
	word.x_axis += 28;
	word.y_axis = 26;
	LCD_SetString("lat",&word);
	word.y_axis += 78;
	latint = gps->latitude/GPS_SCALE_INT;
	latdec = gps->latitude%GPS_SCALE_INT;
	sprintf(str,"%d.%05d",latint,latdec);
	LCD_SetString(str,&word);
	
	//����ˢ��
	word.x_axis += 28;
	word.forecolor = LCD_BLACK;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	memset(str,0,4);
	if(gps->status == 'A')
	{
		g_status = 1;
	}
	
	if ((gps->status == 'V' )&& (g_status ==0) )
	{
		g_Second += 1;
	}

	//if( g_Second >= 120 )
	//	g_Second = 0;
	sprintf(str,"second:%d",g_Second);
	LCD_SetString(str,&word);
}

void DisplayTestGPSDetail(GPSMode *gps)
{
	SetWord_t word = {0};
	char str[15];
	uint8_t a_slmsg[12] = {0},bd_slmsg[12] = {0};
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_WHITE);
	
	LCD_SetRectangle(word.x_axis, LCD_LINE_CNT_MAX, 0, LCD_LINE_CNT_MAX, LCD_WHITE, 0, 0, LCD_FILL_ENABLE);
	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("GPS Detail",&word);
	for(uint8_t i = 0;i < 12;i++)
	{
		a_slmsg[i] = 0;
		bd_slmsg[i] = 0;
	}
	for(uint8_t i = 0;i < 12;i++)
	{
		a_slmsg[i] = gps->slmsg[i].sn;
		bd_slmsg[i] = gps->bdmsg[i].sn;
	}
//	word.x_axis += 24;
//	word.y_axis = 40;
//	word.size = LCD_FONT_24_SIZE;
//	LCD_SetString("star  list",&word);
//	LCD_SetString(str,&word);
	word.x_axis += 24;
	word.y_axis = 36;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("gps:",&word);
	word.y_axis += 48;
	memset(str,0,15);
	sprintf(str,"%d,%d,%d,%d",a_slmsg[0],a_slmsg[1],a_slmsg[2],a_slmsg[3]);
	LCD_SetString(str,&word);
	
	word.x_axis += 28;
	word.y_axis = 36;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("gps:",&word);
	word.y_axis += 48;
	memset(str,0,15);
	sprintf(str,"%d,%d,%d,%d",a_slmsg[4],a_slmsg[5],a_slmsg[6],a_slmsg[7]);
	LCD_SetString(str,&word);
	word.x_axis += 28;
	word.y_axis = 36;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("gps:",&word);
	word.y_axis += 48;
	memset(str,0,15);
	sprintf(str,"%d,%d,%d,%d",a_slmsg[8],a_slmsg[9],a_slmsg[10],a_slmsg[11]);
	LCD_SetString(str,&word);

	word.x_axis += 28;
	word.y_axis = 36;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("bd :",&word);
	word.y_axis += 48;
	memset(str,0,15);
	sprintf(str,"%d,%d,%d,%d",bd_slmsg[0],bd_slmsg[1],bd_slmsg[2],bd_slmsg[3]);
	LCD_SetString(str,&word);
	word.x_axis += 28;
	word.y_axis = 36;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("bd :",&word);
	word.y_axis += 48;
	memset(str,0,15);
	sprintf(str,"%d,%d,%d,%d",bd_slmsg[4],bd_slmsg[5],bd_slmsg[6],bd_slmsg[7]);
	LCD_SetString(str,&word);
	word.x_axis += 28;
	word.y_axis = 36;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("bd :",&word);
	word.y_axis += 48;
	memset(str,0,15);
	sprintf(str,"%d,%d,%d,%d",bd_slmsg[8],bd_slmsg[9],bd_slmsg[10],bd_slmsg[11]);
	LCD_SetString(str,&word);
	
	//����ˢ��
	word.x_axis += 28;
	word.forecolor = LCD_BLACK;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	memset(str,0,4);
	if(gps->status == 'A')
	{
		g_status=1;
	}
	if ((gps->status == 'V')&& (g_status ==0))
	{
		g_Second += 1;
    }
	//if( g_Second >= 120 )
	//	g_Second = 0;
	sprintf(str,"second:%d",g_Second);
	LCD_SetString(str,&word);
}
#if defined(HARDWARE_TEST)
void MsgPostGPS(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_BACK:
		{
			ScreenState = DISPLAY_SCREEN_TEST_GPS;
			//���ش�������
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		default:
			break;
	}

}
#endif
/*����������ʾ����*/
void DisplayTestHdr( uint8_t hdr_value )
{
	SetWord_t word = {0};
	char str[30];
	char strVersion[30];
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_WHITE);
	
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("HEARTRATE",&word);
	
	memset(str,0,30);
	sprintf(str,"Heart:%d",hdr_value);
	word.x_axis = 92;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);
	
	memset(str,0,30);
	drv_getHeartrateVersion(strVersion,6);
	sprintf(str,"version:%s",strVersion);
	word.x_axis = 156;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);
}

/*����Flash��ʾ����*/
void DisplayTestFlash(bool isTrue)
{
	SetWord_t word = {0};
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_WHITE);
	
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("FLASH",&word);
	if(isTrue)
	{
		word.x_axis = 92;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
		word.size = LCD_FONT_24_SIZE;
		LCD_SetString("Flash is ok!",&word);
	}
	else
	{
		word.x_axis = 92;
		word.y_axis = LCD_CENTER_JUSTIFIED;
		word.forecolor = LCD_BLACK;
		word.bckgrndcolor = LCD_WHITE;
		word.size = LCD_FONT_24_SIZE;
		LCD_SetString("Flash is wrong!",&word);
	}

}
#if defined(DEBUG_FACTORY_HDR_AMBIENT_TEST)
/*��������������ѹ����*/
void DisplayTestHdrAmbient_paint(float pres,float alt,float temp,uint8_t hdr_value)
{
	SetWord_t word = {0};
	char str[30];
	char strVersion[30];
	
	//����������Ϊ��ɫ
	LCD_SetBackgroundColor(LCD_WHITE);
	
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("�Լ�",&word);
	
	memset(str,0,30);
	sprintf(str,"����:%d",hdr_value);
	word.x_axis +=  48;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);
	
	memset(str,0,30);
	drv_getHeartrateVersion(strVersion,6);
	sprintf(str,"���ʰ汾:%s",strVersion);
	word.x_axis += 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);
	
	memset(str,0,30);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"��ѹ:%d.%01d",(int32_t)pres,(int32_t)(fabsf(pres)*10)%10);
	#else
	sprintf(str,"��ѹ:%.1f",pres);
	#endif
	word.x_axis += 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);
	memset(str,0,30);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"�¶�:%d.%01d",(int32_t)temp,(int32_t)(fabsf(temp)*10)%10);
	#else
	sprintf(str,"�¶�:%.1f",temp);
	#endif
	word.x_axis += 32;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_WHITE;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString(str,&word);
}
void GoToDisplayTestHdrAmbient_btn_evt(void)
{
	DISPLAY_MSG  msg = {0};
	
	ScreenState = DISPLAY_SCREEN_FACTORY_HDR_AMBIENT_TEST;
	msg.cmd = MSG_DISPLAY_SCREEN;
	xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	task_hrt_start();//��ʼ������
}
void DisplayTestHdrAmbient_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch( Key_Value )
	{
		case KEY_BACK:{
			task_hrt_close();
			tool_task_close();
			ScreenState = DISPLAY_SCREEN_HOME;
			//���ش�������
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		default:
			break;
	}
}
#endif
/*�Լ�gps+BD���水������*/
void gui_post_gps_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_OK:
		{
			ScreenState = DISPLAY_SCREEN_POST_GPS_DETAILS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		case KEY_BACK:
		{
			CloseGPSTask();
			ScreenState = DISPLAY_SCREEN_POST_SETTINGS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		default:
			break;
	}
}
/*�Լ�GPS+BD������水������*/
void gui_post_gps_details_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	switch(Key_Value)
	{
		case KEY_BACK:
		{
			ScreenState = DISPLAY_SCREEN_POST_GPS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		}break;
		default:
			break;
	}
}
