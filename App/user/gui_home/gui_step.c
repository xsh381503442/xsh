/*
*�������ܽ���:�Ʋ���ʵʱ������ʵʱ���룬Ŀ��ֵ�����������룩����ɶȣ�һ�ܲ���������ֱ��ͼ��
*/
#include "gui_config.h"
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui_tool.h"
#include "gui.h"
#include "gui_step.h"
#include "gui_run.h"
#include "gui_sport.h"
#include "gui_VO2max.h"
#include "gui_heartrate.h"
#include "drv_lcd.h"
#include "drv_key.h"

#include "task_display.h"
#include "task_key.h"
#include "task_timer.h"
#include "task_gps.h"

#include "bsp_rtc.h"

#include "com_data.h"
#include "com_sport.h"
#include "font_config.h"
#include "drv_lsm6dsl.h"
#include "algo_trackoptimizing.h"

#if DEBUG_ENABLED == 1 && GUI_STEP_LOG_ENABLED == 1
	#define GUI_STEP_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_STEP_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_STEP_WRITESTRING(...)
	#define GUI_STEP_PRINTF(...)		        
#endif
#define STEP_MAX_VALUE (100000)

extern SetValueStr SetValue;
static uint32_t weekCommonSteps[7] = {0},weekSportsSteps[7] = {0},
          weekCommonDistance[7] = {0},weekSportsDistance[7] = {0};
extern void set_gps_hint_motor_flag(bool flag);
extern SWITCH_HOME_SCREEN_T switch_home_screen;
extern HOME_OR_MINOR_SCREEN_T home_or_minor_switch;
extern ScreenState_t back_monitor_screen;
uint8_t menuIndex = 0;
float stepStride = 0;
static bool CalDistanceStatus = false;
void SetCalDistanceStatus(bool flag)
{
	CalDistanceStatus = flag;
}
bool GetCalDistanceStatus(void)
{
	return CalDistanceStatus;
}
/*�����ճ�/����ݲ�������ʾ��*/
void get_weekSteps(void)
{
	GetDayMainData_CommonStepWeekData(weekCommonSteps);
	GetDayMainData_SportsStepWeekData(weekSportsSteps);
	GetDayMainData_CommonDistanceWeekData(weekCommonDistance);
	GetDayMainData_SportsDistanceWeekData(weekSportsDistance);
}

uint32_t get_week_common_distance(uint8_t day)
{
	return weekCommonDistance[day];
}

uint32_t get_week_sport_distance(uint8_t day)
{
	return weekSportsDistance[day];
}

/*��ȡһ���������ճ������ͻ�����ܺ͵����ֵ*/
uint32_t get_weekmain_max_steps(void)
{
	int weekStepsMax[7] = {0};
	int weekStepsMaxValue = 0;
	
	weekCommonSteps[RTC_time.ui32Weekday] = DayMainData.CommonSteps;
	weekSportsSteps[RTC_time.ui32Weekday] = DayMainData.SportSteps;
	
	for(uint8_t i = 0;i < 7;i++)
	{
		weekStepsMax[i] = weekCommonSteps[i] + weekSportsSteps[i];
	}
	weekStepsMaxValue = weekStepsMax[0];
	for(uint8_t i = 0;i < 7;i++)
	{
		if(weekStepsMax[i] >= STEP_MAX_VALUE)
		{
			weekStepsMax[i] = 0;
		}
		if(weekStepsMax[i] >= weekStepsMaxValue)
		{
			weekStepsMaxValue = weekStepsMax[i];
		}
	}
	if(SetValue.AppSet.Goal_Steps == 0)
	{
		SetValue.AppSet.Goal_Steps = 6000;
	}
	if(SetValue.AppSet.Goal_Steps > weekStepsMaxValue)
	{
		weekStepsMaxValue = SetValue.AppSet.Goal_Steps;
	}
	GUI_STEP_PRINTF("---weekStepsMaxValue=%d--\n",weekStepsMaxValue);
	
	return weekStepsMaxValue;
}
/*��ȡһ���������ճ���̺ͻ����ܺ͵����ֵ*/
uint32_t get_weekmain_max_distance(void)
{
	uint32_t weekDistanceMax[7] = {0};
	uint32_t weekDistanceMaxValue = 0;
	
	weekCommonDistance[RTC_time.ui32Weekday] = DayMainData.CommonDistance;
	weekSportsDistance[RTC_time.ui32Weekday] = DayMainData.SportDistance;
	
	for(uint8_t i = 0;i < 7;i++)
	{
		weekDistanceMax[i] = weekCommonDistance[i] + weekSportsDistance[i];
	}
	weekDistanceMaxValue = weekDistanceMax[0];
	for(uint8_t i = 0;i < 7;i++)
	{
		if(weekDistanceMax[i] >= weekDistanceMaxValue)
		{
			weekDistanceMaxValue = weekDistanceMax[i];
		}
	}
//	if(SetValue.AppSet.Goal_Distance == 0)
//	{
//		SetValue.AppSet.Goal_Distance = 5;
//	}
//	if(SetValue.AppSet.Goal_Distance*100000 > weekDistanceMaxValue)
//	{
//		weekDistanceMaxValue = SetValue.AppSet.Goal_Distance*100000;//km
//	}

	GUI_STEP_PRINTF("---weekDistanceMaxValue=%d--weekDistanceMaxValue/100000=%d\n",weekDistanceMaxValue,weekDistanceMaxValue/100000);
	
	return weekDistanceMaxValue;
}

/* ���⼰�������ʾ ������� 
*type 0:����  1:���
*max ���ֵ
*����ֵ �������ص���������
*/
static uint32_t gui_sidebar_step_paint(uint8_t type,uint32_t max)
{
	SetWord_t word = {0};
	uint32_t lowValue = 0;//����һ�̶ȵ�ֵ���
	uint8_t level = 0,multipleValue = 0;
	char str[10];
	
	//������ʾ
	word.x_axis = 20;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	if(type == 0)
	{
		level = 20;
		LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][7],&word);//"����"
		if(max < 1000 && max > 0)
		{
			lowValue = max / 1000.0;
		}
		else if(max >= 1000 && max < 100000)
		{
			lowValue = max / 1000;
		}
		else if(max == 0)
		{
			lowValue = 1;
		}
		else
		{
			lowValue = 99;
		}
	}
	else
	{
		level = 20;
		LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][8],&word);//"���"
		if(max < 1000 && max > 0)
		{
			lowValue = max;
		}
		else if(max >= 1000 && max < 100000)
		{
			lowValue = max;
		}
		else if(max == 0)
		{
			lowValue = 1;
		}
		else
		{
			lowValue = 99000;
		}
	}

	GUI_STEP_PRINTF("---max=%d,lowValue=%d\n",max,lowValue);
	if(lowValue <= 3)//<4k
	{
		multipleValue = 1;
	}
	else if(lowValue > 3 && lowValue < 100)//<100k
	{
		multipleValue = lowValue/3;
		if((int)(lowValue)%3 != 0)
		{
			multipleValue += 1;
		}
	}
	GUI_STEP_PRINTF("multipleValue=%d\n",multipleValue);
	memset(str,0,6);
	//��������ֶ���
	if(3 * multipleValue >= 100)//>=100k
	{
		if(type == 0)
		{
			sprintf(str,"%d",99);
		}
		else
		{
			sprintf(str,"%d",99);
		}
	}
	else if((3 * multipleValue) >= 10 && (3 * multipleValue) < 100)//10k~100k
	{	
		if(type == 0)
		{
			sprintf(str,"%d",3 * multipleValue);
		}
		else
		{
			sprintf(str,"%d",3 * multipleValue);
		}
	}
	else if((3 * multipleValue) < 10)//<10k;lowValue<10;
	{
		if(type == 0)
		{
			sprintf(str,"%d",3 * multipleValue);
		}
		else
		{
			sprintf(str,"%d",3 * multipleValue);
		}
		
	}
	word.x_axis = 190 - 10 - 4*LCD_FONT_13_SIZE - (3 * level);
	word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
    memset(str,0,6);
	if(type == 0)
	{
		sprintf(str,"%d",2 * multipleValue);
	}
	else
	{
		sprintf(str,"%d",2 * multipleValue);
	}
	if((2 * multipleValue) >= 10 && (2 * multipleValue) < 100)
	{
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
	}
	else if((2 * multipleValue) < 10)
	{
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
	}
	word.x_axis = 190 - 10 - 3*LCD_FONT_13_SIZE - (2* level);
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	

   	memset(str,0,6);
	if(type == 0)
	{
		sprintf(str,"%d",1 * multipleValue);
	}
	else
	{
		sprintf(str,"%d",1 * multipleValue);
	}
	if((1 * multipleValue) >= 10 && (1 * multipleValue) < 100)
	{
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
	}
	else if((1 * multipleValue) < 10)
	{
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
	}
	word.x_axis = 190 - 10 - 2*LCD_FONT_13_SIZE - (1 * level);
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);

	
	memset(str,0,6);
	sprintf(str,"%d",0);
	word.x_axis = 190 - 10 - LCD_FONT_13_SIZE -(0 * level);
	word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_LIGHTGRAY;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);

	return multipleValue;
}


/*����/��� ��״ͼ��ʾ�������������
*day:�����еڼ��죬��η�Χ0~6
*type��0Ϊ������1Ϊ���
*/
static void gui_histogram_step_paint(uint8_t day,uint8_t type)
{
	SetWord_t word = {0};
	uint32_t max_steps = 0,max_mile = 0,multipleValue;
	uint32_t stepsCommon = 0,stepsSports = 0,stepsTarget = 0;//�������������
	uint8_t mDayNum = 0,mNewDayNum = 0;
	uint8_t areaValue = 108,level = 0;//��״ͼ�������ű���


	
	
	LCD_SetPicture(58,205,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//���ϽǼ�ͷ
//	LCD_SetPicture(120-img_step_arrow_left.height/2, 10, LCD_RED, LCD_NONE, &img_step_arrow_left);
	//gui_step_img_setting_blink();//����ͼ����˸
	LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_LINE, 120, 4, 225, 135, LCD_DARKTURQUOISE);//�ϻ�Ȧ
	
	gui_daysofweek_paint(RTC_time.ui32Weekday);

	if(day == Sat)
	{
		day = Sun;
	}
	else if( day >= 7 )
	{//�����δ�����day=1
		mDayNum = 1;
	}
	else
	{
		day += 1;
	}
	if(type == 0)
	{
		max_steps = get_weekmain_max_steps();
	}
	else if(type == 1)
	{//�����״ͼ����ʾĿ�����
		max_mile = get_weekmain_max_distance()/100000;
	}
	GUI_STEP_PRINTF("--------------max_steps=%d,max_mile=%d,SetValue.AppGet.CalibrationStride=%d\n",max_steps,max_mile,SetValue.AppGet.CalibrationStride);
	//���˳�Ԥ�ڵ�ֵ,������ʾ��״ͼ
	if(max_steps >= 97000)
		max_steps = 96999;
	if(max_mile >= 97000)
		max_mile = 96999;

	if(type == 0)
	{//������״ͼ
		//��ʾ�����������,������������
		multipleValue = gui_sidebar_step_paint(0,max_steps);
		level = areaValue/(3*multipleValue);

		for(int i = 0;i < 7;i++)
		{
			mDayNum = day + i;
			if(mDayNum >= 7)
			{
				mNewDayNum = mDayNum - 7;
			}
			else
			{
				mNewDayNum = mDayNum;
			}
			stepsCommon = ( weekCommonSteps[mNewDayNum] / 1000.0 *level );
			stepsSports = ( weekSportsSteps[mNewDayNum] / 1000.0 *level );
			stepsTarget = ( SetValue.AppSet.Goal_Steps  / 1000.0 *level );

            
			word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6*8) / 2 + i * (13 + 8);
			word.x_axis = 190 - 10 - stepsTarget;//Ŀ�경��/���
			if(stepsTarget > areaValue)
			{
				word.x_axis = 190 - 10-areaValue;
				stepsTarget = areaValue;
			}
			LCD_SetRectangle(word.x_axis, stepsTarget, word.y_axis +4 , 5, LCD_LIGHTGRAY, 1, 1, LCD_FILL_ENABLE);


			word.x_axis = 190 - 10 - stepsCommon;//�ճ�����/���
			if(stepsCommon > areaValue)
			{
				word.x_axis = 190 - 10-areaValue;
				stepsCommon = areaValue;
			}
			LCD_SetRectangle(word.x_axis, stepsCommon, word.y_axis + 4, 5, LCD_CYAN, 1, 1, LCD_FILL_ENABLE);
			


			word.x_axis = 190 - 10 - stepsCommon - stepsSports;//�����/���
			if(stepsCommon + stepsSports > areaValue)
			{
				word.x_axis = 190 - 10-areaValue;
				if(stepsSports > areaValue)
				{//������������ֵʱ,����Ϊ�������
					stepsSports = areaValue;
				}
			}
			//LCD_SetRectangle(word.x_axis, stepsSports, word.y_axis, LCD_FONT_16_SIZE - 2, LCD_LIGHTGREEN, 1, 1, LCD_FILL_ENABLE);
			LCD_SetRectangle(word.x_axis, stepsSports, word.y_axis + 4, 5, LCD_CORNFLOWERBLUE, 1, 1, LCD_FILL_ENABLE);
			
			GUI_STEP_PRINTF("weekCommonSteps[%d] = %d,weekSportsSteps[%d] = %d,stepsTarget=%d,word.y_axis=%d,max_steps=%d,level=%d\n",
			                mNewDayNum,weekCommonSteps[mNewDayNum],mNewDayNum,weekSportsSteps[mNewDayNum],stepsTarget,word.y_axis,max_steps,level);
		}
	}
	else
	{//�����״ͼ
		//��ʾ�����������,������������
		multipleValue = gui_sidebar_step_paint(1,max_mile);
		level = areaValue/(3*multipleValue);
		
		for(int i = 0;i < 7;i++)
		{
			mDayNum = day + i;
			if(mDayNum >= 7)
			{
				mNewDayNum = mDayNum - 7;
			}
			else
			{
				mNewDayNum = mDayNum;
			}
			stepsCommon = (int)(weekCommonDistance[mNewDayNum]/100000.0 * level);
			stepsSports = (int)(weekSportsDistance[mNewDayNum]/100000.0 * level);
//			stepsTarget = (int)(SetValue.AppSet.Goal_Distance * level);
			
			word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6*8) / 2 + i * (13 + 8);

//			word.x_axis = 180 - 10 - stepsTarget;//Ŀ�경��/���
//			if(stepsTarget > areaValue)
//			{
//				word.x_axis = 180 - 10-areaValue;
//				stepsTarget = areaValue;
//			}
//			LCD_SetRectangle(word.x_axis, stepsTarget, word.y_axis, LCD_FONT_16_SIZE - 2, LCD_LIGHTGRAY, 1, 1, LCD_FILL_ENABLE);
			word.x_axis = 190 - 10 - stepsCommon;//�ճ�����/���
			if(stepsCommon > areaValue)
			{
				word.x_axis = 190 - 10-areaValue;
				stepsCommon = areaValue;
			}
			//LCD_SetRectangle(word.x_axis, stepsCommon, word.y_axis, LCD_FONT_16_SIZE - 2, LCD_PERU, 1, 1, LCD_FILL_ENABLE);
			
			LCD_SetRectangle(word.x_axis, stepsCommon, word.y_axis + 4, 5, LCD_CYAN, 1, 1, LCD_FILL_ENABLE);
			word.x_axis = 190 - 10 - stepsCommon - stepsSports;//�����/���
			if(stepsCommon + stepsSports > areaValue)
			{
				word.x_axis = 190 - 10-areaValue;
				if(stepsSports > areaValue)
				{//���̴������ֵʱ,����Ϊ�������
					stepsSports = areaValue;
				}
			}
			//LCD_SetRectangle(word.x_axis, stepsSports, word.y_axis, LCD_FONT_16_SIZE - 2, LCD_LIGHTGREEN, 1, 1, LCD_FILL_ENABLE);
			
			LCD_SetRectangle(word.x_axis, stepsSports, word.y_axis + 4, 5, LCD_DARKTURQUOISE, 1, 1, LCD_FILL_ENABLE);
			GUI_STEP_PRINTF("stepsCommon=%d,stepsSports = %d,stepsTarget= %d,\nweekCommonSteps[%d] = %d,weekSportsSteps[%d] = %d,max_mile=%d,level=%d,SetValue.AppSet.Goal_Distance=%d\n",
			                stepsCommon,stepsSports,stepsTarget,mNewDayNum,weekCommonSteps[mNewDayNum],mNewDayNum,weekSportsSteps[mNewDayNum],max_mile,level,SetValue.AppSet.Goal_Distance);
		}
	}
}

/*������ʾ:�Ʋ����� ���������ֵ��ʾ*/
void gui_step_value_paint(void)
{
	SetWord_t word = {0};
	float mile = 0;
	char str[10];
	uint8_t len;
	
	memset(str,0,10);
	sprintf(str,"%d",Get_TotalStep());
	len = strlen(str);
	word.x_axis = LCD_PIXEL_ONELINE_MAX/2 - Font_Number_56.height/2;
    word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	word.forecolor = LCD_WHITE;
    word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word); //����ֵ


	mile = Get_TotalDistance()/100000.f;//����*����=���
	memset(str,0,10);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",Get_TotalDistance()/100000,Get_TotalDistance()/1000%100);
	#else
	sprintf(str,"%0.2f",mile);
	len = strlen(str);
	#endif
	word.x_axis = 188;
	word.y_axis = (LCD_PIXEL_ONELINE_MAX - len*Font_Number_19.width + 0.5*Font_Number_19.width)/2;
	word.size = LCD_FONT_19_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);//���ֵ

	word.x_axis = 188 + Font_Number_19.height - 16;
	word.y_axis  += len*Font_Number_19.width - 0.5*Font_Number_19.width + 6;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][69],&word);//"km"

    
}
/*������ʾ:�Ʋ�����/�����ʾ����*/
void gui_step_paint(void)
{
	SetWord_t word = {0};
	float percent = 0;
	uint32_t step = Get_TotalStep();

	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	//����
	
	word.x_axis = 40;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	LCD_SetPicture(word.x_axis, word.y_axis, LCD_CYAN, LCD_NONE, &img_monitor_step);
	
	gui_step_value_paint();
	
	//������ɶ�
	if(step > SetValue.AppSet.Goal_Steps)
	{
		//�ﵽĿ��ֵ
		percent = 100;
	}
	else
	{
		if(SetValue.AppSet.Goal_Steps > 0)
		{
			percent = (step*100.f/SetValue.AppSet.Goal_Steps);
		}
		else
		{
			percent = 0;
		}
	}
	
	//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_LINE, 115, 8, 225, 135, LCD_GRAY);//�Ʋ���Ȧ
	
	//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_LINE, 115, 8, 145, 215, LCD_CORNFLOWERBLUE);//�»�Ȧ

	//gui_progress_circle(LCD_CENTER_LINE, LCD_CENTER_ROW, 122, 8, 360, 360, percent, LCD_DARKTURQUOISE, LCD_GRAY);    
gui_progress_loop(LCD_CENTER_LINE, LCD_CENTER_ROW, 118, 4, 225,270, percent, LCD_CYAN, LCD_LIGHTGRAY,1); 

}

/*������ʾ:һ�ܲ�����ʾ����*/
void gui_step_week_steps_paint(void)
{
	am_hal_rtc_time_get(&RTC_time);
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	gui_histogram_step_paint(RTC_time.ui32Weekday,0);
}
/*������ʾ:һ�������ʾ����*/
void gui_step_week_mileage_paint(void)
{
	am_hal_rtc_time_get(&RTC_time);
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	gui_histogram_step_paint(RTC_time.ui32Weekday,1);
}
/*������ʾ:�Ʋ�У׼��ʾ����*/
void gui_step_calibration_hint_paint(void)
{
	SetWord_t word = {0};
	//uint16_t u_startx,u_starty,u_endx,u_endy,d_startx,d_starty,d_endx,d_endy;


	//Բ�������յ�����

	//get_circle_point(LCD_CENTER_LINE, LCD_CENTER_LINE, 225, 4, &u_startx,&u_starty);
	//get_circle_point(LCD_CENTER_LINE, LCD_CENTER_LINE, 135, 4, &u_endx,&u_endy);

	//get_circle_point(LCD_CENTER_LINE, LCD_CENTER_LINE, 145, 4, &d_startx,&d_starty);
	//get_circle_point(LCD_CENTER_LINE, LCD_CENTER_LINE, 215, 4, &d_endx,&d_endy);
	am_hal_rtc_time_get(&RTC_time);
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	//����
	//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_LINE, 120, 4, 225, 135, LCD_CYAN);//�ϻ�Ȧ
	
	//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_LINE, 120, 4, 145, 215, LCD_CORNFLOWERBLUE);//�»�Ȧ
	gui_sport_gps_status(SPORT_GPS_LEFT1);//gps ��
	//gui_point_direction(IMG_POINT_UP,LCD_RED);//���ͷ


	//LCD_SetPicture(45,200,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//���ϽǼ�ͷ
	
	LCD_SetPicture(58,205,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//���ϽǼ�ͷ
	word.x_axis = LCD_LINE_CNT_MAX/2 - 80;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][0],&word);//"����У׼"


	
#if  defined WATCH_COM_SHORTCUNT_UP
	word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_24.height - 4;
	word.y_axis = LCD_CENTER_JUSTIFIED;
    word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_CYAN;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][1],&word);//"��ƽֱ·������"
	word.x_axis = LCD_LINE_CNT_MAX/2 + 4;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][2],&word);//"����200��+"

	
	word.x_axis += Font_Number_24.height + 28;
	LCD_SetRectangle(word.x_axis,4+16+4,0,LCD_LINE_CNT_MAX,LCD_GRAY,0,0,1);

	word.x_axis += 4;
	word.size = LCD_FONT_16_SIZE;
    word.forecolor = LCD_BLACK;
	word.bckgrndcolor = LCD_NONE;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][3],&word);//"OK������У׼"
#else
	word.x_axis = LCD_LINE_CNT_MAX/2 - 12;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][4],&word);//"������������200������;"
	word.x_axis += 24;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][5],&word);//"У׼ʱ�����ƽֱ·������;"
	word.x_axis = 200;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][6],&word);//"OK������У׼"
#endif
}
/*������ʾ:�Ʋ�У׼�굯������ʾ����*/
void gui_step_calibration_show_paint(float mile,uint32_t step,float stepLength)
{
	SetWord_t word = {0};
	char str[10];
	
	am_hal_rtc_time_get(&RTC_time);
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	//����
	

	//gui_progess_toothed(360,40);
	
	gui_sport_gps_status(SPORT_GPS_LEFT1);
	
	//LCD_SetPicture(45,200,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//���ϽǼ�ͷ
	
	LCD_SetPicture(58,205,LCD_RED,LCD_NONE,&step_cal_arrow_youshang);//���ϽǼ�ͷ
	memset(str,0,10);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%01d",(uint32_t)(mile/100),(uint32_t)(mile/10)%10);
	#else
	sprintf(str,"%0.1f",mile/100.f);
	#endif
	word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_56.height - 12;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_56_SIZE;
	word.kerning= 1;
	LCD_SetNumber(str,&word);
	

	
	word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_24.height/2 - Font_Number_56.height/2  - 12;
	word.y_axis = LCD_LINE_CNT_MAX/2 +  (strlen(str) * (Font_Number_56.width + word.kerning) - word.kerning - (1*Font_Number_56.width/2))/2 +  8;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("m",&word);
	
	LCD_SetRectangle(LCD_LINE_CNT_MAX/2 - 1,2,LCD_LINE_CNT_MAX/2 - 60,60*2,LCD_WHITE,0,0,LCD_FILL_ENABLE);
	
	memset(str,0,10);
	sprintf(str,"%d",step);
	word.x_axis = LCD_LINE_CNT_MAX/2 + 12;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_56_SIZE;
	word.kerning= 1;
	LCD_SetNumber(str,&word);
	
	
	word.x_axis = LCD_LINE_CNT_MAX/2 + 12 + Font_Number_56.height/2 - Font_Number_16.height;
	word.y_axis =  LCD_LINE_CNT_MAX/2 +  (strlen(str) * (Font_Number_56.width + word.kerning) - word.kerning - (1*Font_Number_56.width/2))/2 +  16;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiHomeLockStrs[SetValue.Language][6],&word);//"��"

	word.x_axis = 200;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_16_SIZE;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][7],&word);//"OK������У׼"
}
/*������ʾ:�Ʋ�У׼��ȷ�ϵ���ʾ����*/
void gui_step_calibration_confirm_save_paint(float mile,uint32_t step,float stepLength)
{
	SetWord_t word = {0};
	char str[10];
	
	am_hal_rtc_time_get(&RTC_time);
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	//����
	/*���»�Ȧ*/
	//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_LINE, 120, 8, 225, 135, LCD_CYAN);//�ϻ�Ȧ
	
	//LCD_SetArc(LCD_CENTER_LINE, LCD_CENTER_LINE, 120, 8, 145, 215, LCD_CORNFLOWERBLUE);//�»�Ȧ
	gui_tick_cross();
	
	memset(str,0,10);
	#if defined (SPRINTF_FLOAT_TO_INT)
	sprintf(str,"%d.%02d",(uint32_t)(stepLength/100),(uint32_t)(stepLength)%100);
	#else
	sprintf(str,"%0.2f",stepLength/100.f);
	#endif
	
	word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_56.height - 12;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_56_SIZE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	
	word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_56.height - 12 + Font_Number_56.height/2 - 24/2;
    word.y_axis = LCD_LINE_CNT_MAX/2 + (strlen(str) * (Font_Number_56.width + word.kerning) - word.kerning - (1*Font_Number_56.width/2))/2 + 8;;
	word.forecolor = LCD_CYAN;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString("m",&word);

	//word.x_axis = 190;
	word.x_axis = LCD_LINE_CNT_MAX/2 + 12;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_BLACK;
	word.size = LCD_FONT_24_SIZE;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][8],&word);//"ѡ���Ƿ���Ϊ����"
	word.x_axis += 32;
	LCD_SetString((char *)GuiStepScreenStrs[SetValue.Language][9],&word);//"��������?"
}
/*��������:�Ʋ����������ʾ����*/
void gui_step_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	home_or_minor_switch = MINOR_SCREEN_SWITCH;

	switch( Key_Value )
	{
		case KEY_OK:
			GUI_STEP_PRINTF("[gui_step]:SetValue.AppGet.CalibrationStride=%d,stepStride=%d\r\n",SetValue.AppGet.CalibrationStride,stepStride);
			get_weekSteps();
			ScreenState = DISPLAY_SCREEN_STEP_WEEK_STEPS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			//��ȡ�� �ճ������ 
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			if(SetValue.IsFirstCalibrationStep == 1)
			{//��δУ׼��ʱ,У׼ͼ����˸
				timer_app_step_calibration_start();
			}
		break;
		case KEY_UP:
			//set_switch_screen_count_evt(Key_Value);
			switch_home_screen = MONITOR_HOME_SCREEN;
			
			back_monitor_screen = DISPLAY_SCREEN_STEP;
			ScreenState = DISPLAY_SCREEN_HOME;
			msg.cmd = MSG_DISPLAY_SCREEN;
			//��ȡ�� �ճ������ 
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
			
		case KEY_DOWN:
           
			if((((SetValue.U_CustomMonitor.val>>1) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>2) & 0x01)== 0)&& (((SetValue.U_CustomMonitor.val>>3) & 0x01)== 0)
				&&(((SetValue.U_CustomMonitor.val>>4) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>5) & 0x01)== 0))
									{
										switch_home_screen = MONITOR_HOME_SCREEN;
										
										back_monitor_screen = DISPLAY_SCREEN_STEP;
										ScreenState = DISPLAY_SCREEN_HOME;
										 //���ش�������
										 msg.cmd = MSG_DISPLAY_SCREEN;
										 xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
									}
								else
									{
									  
									set_switch_screen_count_evt(Key_Value);
									}
		
			break;
		case KEY_BACK:
			
			switch_home_screen = BACK_HOME_SCREEN;
			ScreenState = DISPLAY_SCREEN_HOME;
		back_monitor_screen = DISPLAY_SCREEN_STEP;
			//���ش�������
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		break;
		default:
		break;
	}
}
/*��������:һ�ܲ�����ʾ����*/
void gui_step_week_steps_btn_evt(uint32_t Key_Value)
	{
		DISPLAY_MSG  msg = {0};
		switch( Key_Value )
		{
			case KEY_OK:
				if(IS_SPORT_CONTINUE_LATER==1)
				{//������Ժ����,���������ſ��Խ���
					menuIndex = 1;
					ScreenState = DISPLAY_SCREEN_END_UNFINISH_SPORT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					msg.value = DISTINGUISH_STEP;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else
				{
					menuIndex = 1;
					set_gps_hint_motor_flag(false);
					if(SetValue.IsFirstCalibrationStep == 1)
					{//��δУ׼��ʱ,ֹͣУ׼ͼ����˸
						timer_app_step_calibration_stop(false);
					}
					ScreenState = DISPLAY_SCREEN_STEP_CALIBRATION_HINT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					CreateGPSTask();
				}
				break;
			case KEY_UP:	
			case KEY_DOWN:				
				if(ScreenState == DISPLAY_SCREEN_STEP_WEEK_STEPS)
				{
					ScreenState = DISPLAY_SCREEN_STEP_WEEK_MILEAGE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else
					{
					ScreenState = DISPLAY_SCREEN_STEP_WEEK_STEPS;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	
	
				   }
			break;
			case KEY_BACK:
				if(SetValue.IsFirstCalibrationStep == 1)
				{//��δУ׼��ʱ,ֹͣУ׼ͼ����˸
					timer_app_step_calibration_stop(false);
				}
				ScreenState = DISPLAY_SCREEN_STEP;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
			default:
			break;
		}
	}

/*��������:һ�������ʾ����*/
void gui_step_week_mileage_btn_evt(uint32_t Key_Value)
	{
		DISPLAY_MSG  msg = {0};
		switch( Key_Value )
		{
			case KEY_OK:
				if(IS_SPORT_CONTINUE_LATER==1)
				{//������Ժ����,���������ſ��Խ���
					menuIndex = 1;
					ScreenState = DISPLAY_SCREEN_END_UNFINISH_SPORT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					msg.value = DISTINGUISH_STEP;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else
				{
					menuIndex = 1;
					set_gps_hint_motor_flag(false);
					if(SetValue.IsFirstCalibrationStep == 1)
					{//��δУ׼��ʱ,ֹͣУ׼ͼ����˸
						timer_app_step_calibration_stop(false);
					}
					ScreenState = DISPLAY_SCREEN_STEP_CALIBRATION_HINT;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
					CreateGPSTask();
				}
				break;
				
			case KEY_UP:	
			case KEY_DOWN:				
			if(ScreenState == DISPLAY_SCREEN_STEP_WEEK_STEPS)
				{
					ScreenState = DISPLAY_SCREEN_STEP_WEEK_MILEAGE;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
				}
				else
					{
					ScreenState = DISPLAY_SCREEN_STEP_WEEK_STEPS;
					msg.cmd = MSG_DISPLAY_SCREEN;
					xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
	
	
				   }
			break;
			case KEY_BACK:
				if(SetValue.IsFirstCalibrationStep == 1)
				{//��δУ׼��ʱ,ֹͣУ׼ͼ����˸
					timer_app_step_calibration_stop(false);
				}
				ScreenState = DISPLAY_SCREEN_STEP;
				msg.cmd = MSG_DISPLAY_SCREEN;
				xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
			default:
			break;
		}
	}

/*��������:�Ʋ�У׼��ʾ����*/
void gui_step_calibration_hint_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			SetCalDistanceStatus(true);
			set_gps_hint_motor_flag(false);//�˳��ý��������ͼ���������
			g_VO2max_Step = drv_lsm6dsl_get_stepcount();
			//�㷨��ʼ��
			PhysicalParamCalculateInit();
			ScreenState = DISPLAY_SCREEN_STEP_CALIBRATION_SHOW;
			msg.cmd = MSG_DISPLAY_SCREEN;
		  msg.value = 0;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			CloseGPSTask();
			if(SetValue.IsFirstCalibrationStep == 1)
			{//��δУ׼��ʱ,У׼ͼ����˸
				timer_app_step_calibration_start();
			}
			if(menuIndex == 1)
			{
				ScreenState = DISPLAY_SCREEN_STEP_WEEK_STEPS;
			}
			else if(menuIndex == 2)
			{
				ScreenState = DISPLAY_SCREEN_STEP_WEEK_MILEAGE;
			}
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
		break;
	}
}
/*��������:�Ʋ�У׼�굯������ʾ����*/
void gui_step_calibration_show_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			timer_app_step_calibration_stop(false);
			CloseGPSTask();
			UninitVO2maxMeasing();
			ScreenState = DISPLAY_SCREEN_STEP_CALIBRATION_CONFIRM_SAVE;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
		break;
	}
}
/*��������:�Ʋ�У׼��ȷ�ϵ���ʾ����*/
void gui_step_calibration_confirm_save_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_OK:
			CloseGPSTask();
			UninitVO2maxMeasing();
			SetCalDistanceStatus(false);
			if(stepStride > 255 || stepStride == 0)
			{
				stepStride = 77;//�쳣ֵ���沽��
			}
			SetValue.IsFirstCalibrationStep = 0;//ͼ�겻��˸
			SetValue.AppGet.CalibrationStride  = stepStride;//���沽��
			ScreenState = DISPLAY_SCREEN_STEP;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_BACK:
			CloseGPSTask();
			UninitVO2maxMeasing();
			SetCalDistanceStatus(false);
			ScreenState = DISPLAY_SCREEN_STEP;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
		break;
		default:
		break;
	}
}

