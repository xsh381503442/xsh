/*
*�������ܽ���:��·�� �����ܺģ��˶��ܺģ�һ�ܲ���ֱ��ͼ��
*/
#include "gui_calories.h"
#include "gui_home_config.h"
#include "gui_home.h"
#include "gui.h"

#include "algo_sport.h"

#if DEBUG_ENABLED == 1 && GUI_CALORIES_LOG_ENABLED == 1
	#define GUI_CALORIES_WRITESTRING(...)			SEGGER_RTT_WriteString(0,__VA_ARGS__)
	#define GUI_CALORIES_PRINTF(...) 				SEGGER_RTT_printf(0,##__VA_ARGS__)
#else
	#define GUI_CALORIES_WRITESTRING(...)
	#define GUI_CALORIES_PRINTF(...)		        
#endif

#define CALORIES_MAX  (100000000)//100kǧ��
static uint32_t weekDayCalories[7] = {0},weekDayMaxValue = 0;
uint32_t weekValue_sport[7] = {0},weekValue_common[7] = {0};

extern SWITCH_HOME_SCREEN_T switch_home_screen;
extern HOME_OR_MINOR_SCREEN_T home_or_minor_switch;
extern uint16_t s_switch_screen_count;
extern ScreenState_t back_monitor_screen;
static void get_weekCalories(uint8_t day)
{
	GetDayMainData_SportsCaloriesWeekData(weekDayCalories);

	for(uint8_t i = 0;i < 7;i++)
	{
		if(weekDayCalories[i] >= CALORIES_MAX)
		{
			weekDayCalories[i] = 0; 
		}
		if(weekDayCalories[i] > weekDayMaxValue)
		{
			weekDayMaxValue = weekDayCalories[i];
		}
	}
	#ifdef COD 
	SetValue.AppSet.Goal_Energy = 300;
	#else
	if(SetValue.AppSet.Goal_Energy == 0)
	{
		SetValue.AppSet.Goal_Energy = 3000;
	}
	#endif
	if(weekDayMaxValue < SetValue.AppSet.Goal_Energy*1000)//��Ŀ�꿨·���
	{
		weekDayMaxValue = SetValue.AppSet.Goal_Energy*1000;
	}
//	for(uint8_t i = 0;i < 7;i++)
//	{
//		weekDayCalories[i] = 4000000;
//	}
//	weekDayMaxValue = 40000000;
//	SetValue.AppSet.Goal_Energy = 400;
	GUI_CALORIES_PRINTF("[gui_calories]:get_weekCalories weekDayMaxValue=%d\r\n",weekDayMaxValue);
}
/*������ʾ:��·���˶���������*/
static void gui_calories_sports_foundation_title_paint(void)
{
	SetWord_t word = {0};
	uint32_t total_calories = 0;
	float percent_calories = 0;
	char str[10];
	DayMainData.CommonEnergy = Get_CommonEnergy();//ÿ���Ӽ���һ�λ�����·��
	total_calories = DayMainData.SportEnergy + DayMainData.CommonEnergy;
	if(total_calories > (SetValue.AppSet.Goal_Energy*1000))
	{
		//�ﵽĿ��ֵ
		percent_calories = 100;
	}
	else
	{
		if((SetValue.AppSet.Goal_Energy*1000) > 0)
		{
			percent_calories = (total_calories * 100.f/(SetValue.AppSet.Goal_Energy*1000.f));
		}
		else
		{
			percent_calories = 0;
		}
	}
	
	word.x_axis = 15;
	word.y_axis = LCD_CENTER_JUSTIFIED;
  
  LCD_SetPicture(word.x_axis, word.y_axis, LCD_RED, LCD_NONE, &img_monitor_calory);
  
	memset(str,0,5);
	sprintf(str,"%d",total_calories/1000);//�ܵĿ�·������
	word.x_axis = LCD_LINE_CNT_MAX/2 - Font_Number_56.height/2;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_56_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);

  
    word.x_axis = LCD_LINE_CNT_MAX/2 + Font_Number_56.height/2 - 16;
    word.y_axis =  LCD_LINE_CNT_MAX/2 + strlen(str)*Font_Number_56.width/2 + (strlen(str)-1)*word.kerning/2 + 2;
    word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
    LCD_SetString("kcal",&word);
	
	word.x_axis = LCD_LINE_CNT_MAX/2 + Font_Number_56.height/2 + 16 + 10;
	word.y_axis = LCD_LINE_CNT_MAX/2 - strlen("�˶�")*Font_Number_16.width - word.kerning - 2;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("�˶�",&word);
	
	memset(str,0,5);
	sprintf(str,"%d",DayMainData.SportEnergy/1000);//�˶���·��
	word.x_axis = LCD_LINE_CNT_MAX/2 + Font_Number_56.height/2 + 16 + 10 + Font_Number_16.height - Font_Number_19.height;
	word.y_axis = LCD_LINE_CNT_MAX/2+2;
	word.size = LCD_FONT_19_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	
	
	word.x_axis = LCD_LINE_CNT_MAX/2 + Font_Number_56.height/2 + 16 + 10 + Font_Number_16.height + 10;
	word.y_axis = LCD_LINE_CNT_MAX/2 - strlen("����")*Font_Number_16.width - word.kerning -2;
	word.size = LCD_FONT_16_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetString("����",&word);
	
	memset(str,0,5);
	sprintf(str,"%d",DayMainData.CommonEnergy/1000);//������·��
	word.x_axis = LCD_LINE_CNT_MAX/2 + Font_Number_56.height/2 + 16 + 10 + Font_Number_16.height + 10 + Font_Number_16.height - Font_Number_19.height;
	word.y_axis = LCD_LINE_CNT_MAX/2+2;
	word.size = LCD_FONT_19_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	
	

}
/* ���⼰�������ʾ �˶���·��
*����ֵ �������ص���������
*/
static uint32_t gui_sidebar_calories_paint(void)
{
	SetWord_t word = {0};
	uint32_t lowValue = 0,multipleValue = 0;//����һ�̶ȵ�ֵ���
	uint8_t level = 20;//levelĬ��36
	char str[10];
	
	if(weekDayMaxValue < CALORIES_MAX && weekDayMaxValue > 0)
	{
		lowValue = weekDayMaxValue / 1000.f;
	}
	else if(weekDayMaxValue == 0)
	{
		lowValue = 1;
	}
	else
	{
		lowValue = 99000;//99kǧ��
	}

	
	if(lowValue <= 3 )//<=3ǧ��
	{
		multipleValue = 1;
	}
	else if(lowValue > 3 && lowValue < CALORIES_MAX/1000)//<100kǧ��
	{
		multipleValue = lowValue/3;
		if((int)(lowValue)%3 != 0)
		{
			multipleValue += 1;
		}
	}
	//��������ֶ��뼰��ʾ
	memset(str,0,6);
	if((3 * multipleValue) < 10)
	{//��ʾ0ǧ����10ǧ��
		if((3 * multipleValue) <= 3)
		{//�̶����ֵ��ʾΪ3�ı����������� ��ֹ�̶�ֵ��С
			multipleValue = 1;
		}
		sprintf(str,"%d",3 * multipleValue);
		word.x_axis = 190 - 6  - 4*LCD_FONT_13_SIZE - (3 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",2 * multipleValue);
		word.x_axis = 190 - 6  - 3*LCD_FONT_13_SIZE - (2 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",1 * multipleValue);
		word.x_axis = 190 - 6  - 2*LCD_FONT_13_SIZE - (1 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	}
	else if((3 * multipleValue) >= 10 && (3 * multipleValue) < 100)
	{//��ʾ10ǧ����100ǧ��
		if((3 * multipleValue) <= 30)
		{//�̶����ֵ��ʾΪ3�ı����������� ��ֹ�̶�ֵ��С
			multipleValue = 10;
		}
		sprintf(str,"%d",3 * multipleValue);
		word.x_axis = 190 - 6  - 4*LCD_FONT_13_SIZE - (3 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",2 * multipleValue);
		word.x_axis = 190 - 6  - 3*LCD_FONT_13_SIZE - (2 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",1 * multipleValue);
		word.x_axis = 190 - 6  - 2*LCD_FONT_13_SIZE - (1 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	}
	else if((3 * multipleValue) >= 100 && (3 * multipleValue) < 1000)
	{//��ʾ0.1kǧ����1kǧ��
		if((3 * multipleValue) <= 300)
		{//�̶����ֵ��ʾΪ3�ı����������� ��ֹ�̶�ֵ��С
			multipleValue = 100;
		}
		sprintf(str,"%d",3 * multipleValue);
		word.x_axis = 190 - 6  - 4*LCD_FONT_13_SIZE - (3 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",2 * multipleValue);
		word.x_axis = 190 - 6  - 3*LCD_FONT_13_SIZE - (2 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",1 * multipleValue);
		word.x_axis = 190 - 6  - 2*LCD_FONT_13_SIZE - (1 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	}
	else if((3 * multipleValue) >= 1000 && (3 * multipleValue) < 10000)
	{//��ʾ1kǧ����10kǧ��
		if((3 * multipleValue) <= 3000)
		{//�̶����ֵ��ʾΪ3�ı����������� ��ֹ�̶�ֵ��С
			multipleValue = 1000;
		}
		sprintf(str,"%d",3 * multipleValue);
	word.x_axis = 190 - 6  - 4*LCD_FONT_13_SIZE - (3 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",2 * multipleValue);
		word.x_axis = 190 - 6  - 3*LCD_FONT_13_SIZE - (2 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",1 * multipleValue);
		word.x_axis = 190 - 6  - 2*LCD_FONT_13_SIZE - (1 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	}
	else if((3 * multipleValue) >= 10000 && (3 * multipleValue) < CALORIES_MAX/1000)
	{//��ʾ10kǧ����100kǧ��
		sprintf(str,"%d",3 * multipleValue);
		word.x_axis = 190 - 6  - 4*LCD_FONT_13_SIZE - (3 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%d",2 * multipleValue);
	word.x_axis = 190 - 6  - 3*LCD_FONT_13_SIZE - (2 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
		memset(str,0,6);
		sprintf(str,"%dk",1 * multipleValue);
		word.x_axis = 190 - 6  - 2*LCD_FONT_13_SIZE - (1 * level);
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
		word.size = LCD_FONT_13_SIZE;
		word.forecolor = LCD_WHITE;
		word.bckgrndcolor = LCD_NONE;
		word.kerning = 1;
		LCD_SetNumber(str,&word);
	}
	
	word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 16;
	memset(str,0,6);
	sprintf(str,"%d",0);
	word.x_axis = 190 - 6  - 1*LCD_FONT_13_SIZE - (0 * level);
	word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 - 5 - strlen(str)*7;
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	LCD_SetNumber(str,&word);
	return multipleValue;
}
/*�ײ�����������ʾ 
*���:�����еڼ��죬��η�Χ0~6
*������0�����������������
��·����������װ
*/
static void gui_daysofweek_calories_paint(uint8_t day)
{
	SetWord_t word = {0};
	uint8_t mDayNum = 0,mNewDayNum = 0;
	
	word.x_axis = 190;
	word.size = LCD_FONT_13_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	//day:0~6��������һ~������
	if(day == Sat)
	{
		day = Sun;
	}
	else if( day >= 7)
	{//�����δ�����day=1
		mDayNum = 1;
	}
	else
	{
		day += 1;
	}
	for(uint8_t i = 0;i < 7;i++)
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
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8 ) / 2 + i * (13 + 8) ;
		//LCD_SetString((char *)WeekofDayStrs[SetValue.Language][mNewDayNum],&word);
		
		
		LCD_SetPicture(word.x_axis,word.y_axis,word.forecolor,word.bckgrndcolor,&img_week_13x13[mNewDayNum]);
		
	}
}

static void gui_calories_week_sports_title_paint(uint8_t day)
{
	SetWord_t word = {0};
	//uint32_t caloriesValue = 0,caloriesTargetValue = 0;
	
	uint32_t caloriesValue_sport = 0,caloriesValue_common = 0,caloriesTargetValue = 0;
	uint32_t multipleValue = 0,level = 0;
	uint8_t mDayNum = 0,mNewDayNum = 0;
	uint8_t areaValue = 108;//��״ͼ�������ű���
	
	word.x_axis = 25;
	word.y_axis = LCD_CENTER_JUSTIFIED;
	word.size = LCD_FONT_24_SIZE;
	word.forecolor = LCD_WHITE;
	word.bckgrndcolor = LCD_NONE;
	word.kerning = 1;
	//LCD_SetString("�˶���·��",&word);
	LCD_SetString("��·��",&word);
	
	gui_daysofweek_calories_paint(RTC_time.ui32Weekday);//�ײ�����
	get_weekCalories(day);//��ȡһ�ܿ�·�Ｐ���ֵ
	
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
	
	multipleValue = gui_sidebar_calories_paint();

	//��ÿ�����״��
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
		if((3 * multipleValue) < 100)
		{
			GUI_CALORIES_PRINTF("[gui_calories]:1--3*multipleValue=%d,weekDaySportCalories[mNewDayNum]=%d\n",3 * multipleValue,weekDayCalories[mNewDayNum]);
			level = areaValue / (3 * multipleValue);//ÿǧ��ռ�ݵ�����ֵ
			caloriesValue_sport = ( weekValue_sport[mNewDayNum] / 1000 *level);
			caloriesValue_common = ( weekValue_common[mNewDayNum] / 1000 *level);
			caloriesTargetValue = ( SetValue.AppSet.Goal_Energy  * level);//kcal
		}
		else if((3 * multipleValue) >= 100 && (3 * multipleValue) < 1000)
		{
			GUI_CALORIES_PRINTF("[gui_calories]:2--3*multipleValue=%d,weekDaySportCalories[mNewDayNum]=%d\n",3 * multipleValue,weekDayCalories[mNewDayNum]);
			level = areaValue / (3 * multipleValue/36);//���ű�������Ϊ108/3px
			caloriesValue_sport = ( weekValue_sport[mNewDayNum] / 36000 *level);
			caloriesValue_common = ( weekValue_common[mNewDayNum] / 36000 *level);
			caloriesTargetValue = ( SetValue.AppSet.Goal_Energy /36 * level);//kcal
		}
		else if((3 * multipleValue) >= 1000 && (3 * multipleValue) < 10000)
		{
			GUI_CALORIES_PRINTF("[gui_calories]:3--3*multipleValue=%d,weekDaySportCalories[mNewDayNum]=%d\n",3 * multipleValue,weekDayCalories[mNewDayNum]);
			level = areaValue / (3 * multipleValue/100);
            caloriesValue_sport = ( weekValue_sport[mNewDayNum] / 100000 *level);
			caloriesValue_common = ( weekValue_common[mNewDayNum] / 100000 *level);
			caloriesTargetValue =  SetValue.AppSet.Goal_Energy /100 * level;//kcal
		}
		else if((3 * multipleValue) >= 10000 && (3 * multipleValue) < CALORIES_MAX/1000)
		{
			GUI_CALORIES_PRINTF("[gui_calories]:4--3*multipleValue=%d,weekDaySportCalories[mNewDayNum]=%d\n",3 * multipleValue,weekDayCalories[mNewDayNum]);
			level = areaValue / (3 * multipleValue/1000);
            caloriesValue_sport = ( weekValue_sport[mNewDayNum] / 1000000 *level);
			caloriesValue_common = ( weekValue_common[mNewDayNum] / 1000000 *level);
			caloriesTargetValue =  SetValue.AppSet.Goal_Energy /1000 * level;//kcal
		}
		word.y_axis = (LCD_LINE_CNT_MAX - 7 * 13 - 6 * 8) / 2 + i * (13 + 8);//ƽ��13px

       

		word.x_axis = 190 - 6 - (int)caloriesValue_common;
			if(caloriesValue_common > areaValue)
		{
			word.x_axis = 190 - 6 - areaValue;
			caloriesValue_common = areaValue;
		}
		LCD_SetRectangle(word.x_axis, caloriesValue_common, word.y_axis + 4, 5, LCD_CYAN, 1, 1, LCD_FILL_ENABLE);
		word.x_axis = 190 - 6 - (int)caloriesValue_common-(int)caloriesValue_sport;//�ճ�����/���
		if(caloriesValue_common+caloriesValue_sport > areaValue)
		{
			word.x_axis = 190 - 6 - areaValue;
		if(caloriesValue_sport > areaValue)
			{//���̴������ֵʱ,����Ϊ�������
					caloriesValue_sport = areaValue;
			}
		}
		LCD_SetRectangle(word.x_axis, caloriesValue_sport, word.y_axis + 4, 5, LCD_CORNFLOWERBLUE, 1, 1, LCD_FILL_ENABLE);
	
	}
}
/*������ʾ:��·���˶���������*/
void gui_calories_sports_foundation_paint(void)
{
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	
	gui_calories_sports_foundation_title_paint();//����/�ٷֱ�
}
/*������ʾ:��·�����˶�*/
void gui_calories_week_sports_paint(void)
{
	am_hal_rtc_time_get(&RTC_time);
	//���ñ�����ɫ
	LCD_SetBackgroundColor(LCD_BLACK);
	gui_calories_week_sports_title_paint(RTC_time.ui32Weekday);//����/��״ͼ
}

/*��������:��·���˶���������*/
void gui_calories_sports_foundation_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	
	home_or_minor_switch = MINOR_SCREEN_SWITCH;
	switch( Key_Value )
	{
		case KEY_OK:
			ScreenState = DISPLAY_SCREEN_CALORIES_WEEK_SPORTS;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		case KEY_UP:
			if((SetValue.U_CustomMonitor.val & 0x01)== 0) //�Ʋ�ɾ��
				{
				    
					s_switch_screen_count = 0;
					switch_home_screen = MONITOR_HOME_SCREEN;
					
					back_monitor_screen = DISPLAY_SCREEN_CALORIES;
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
		case KEY_DOWN:
               
			if((((SetValue.U_CustomMonitor.val>>2) & 0x01)== 0)&& (((SetValue.U_CustomMonitor.val>>3) & 0x01)== 0)
				&&(((SetValue.U_CustomMonitor.val>>4) & 0x01)== 0)&&(((SetValue.U_CustomMonitor.val>>5) & 0x01)== 0))
						{
							switch_home_screen = MONITOR_HOME_SCREEN;
							
							back_monitor_screen = DISPLAY_SCREEN_CALORIES;
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
		
			back_monitor_screen = DISPLAY_SCREEN_CALORIES;
			//���ش�������
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}

/*��������:��·�����˶�*/
void gui_calories_week_sports_btn_evt(uint32_t Key_Value)
{
	DISPLAY_MSG  msg = {0};
	switch( Key_Value )
	{
		case KEY_BACK:
			ScreenState = DISPLAY_SCREEN_CALORIES;
			msg.cmd = MSG_DISPLAY_SCREEN;
			xQueueSend(DisplayQueue, &msg, portMAX_DELAY);
			break;
		default:
			break;
	}
}


