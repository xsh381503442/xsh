#include "drv_lcd.h"
#include "font_config.h"
#include "font_displaystrs.h"
#include "watch_config.h"

const char* cMonthStrs[] = 
{
	"һ��",
	"����",
	"����",
	"����",
	"����",
	"����",
	"����",
	"����",
	"����",
	"ʮ��",
	"ʮһ",
	"ʮ��"
};

const char* eMonthStrs[] = 
{
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

const char** MonthStrs[] = {cMonthStrs,eMonthStrs};


const char* cLunarStrs[] = 
{
	"��һ",
	"����",
	"����",
	"����",
	"����",
	"����",
	"����",
	"����",
	"����",
	"��ʮ",
	"ʮһ",
	"ʮ��",
	"ʮ��",
	"ʮ��",
	"ʮ��",
	"ʮ��",
	"ʮ��",
	"ʮ��",
	"ʮ��",
	"إʮ",
	"إһ",
	"إ��",
	"إ��",
	"إ��",
	"إ��",
	"إ��",
	"إ��",
	"إ��",
	"إ��",
	"��ʮ",
	"��һ"
};

const char* eLunarStrs[] = 
{
	"1st",
	"2nd",
	"3rd",
	"4th",
	"5th",
	"6th",
	"7th",
	"8th",
	"9th",
	"10th",
	"11th",
	"12th",
	"13th",
	"14th",
	"15th",
	"16th",
	"17th",
	"18th",
	"19th",
	"20th",
	"21th",
	"22th",
	"23th",
	"24th",
	"25th",
	"26th",
	"27th",
	"28th",
	"29th",
	"30th",
	"31th"
};

const char** LunarStrs[] = {cLunarStrs,eLunarStrs};

const char* cToolStrs[] = 
{
    "�ָ�ʱ��",
	"���������",
	"������",
	"����ʱ",
	"���",
	"��γ��",
	"ָ����",
    "������ѹ"
};

const char* eToolStrs[] = 
{
    " recovery time",
	"VOZmax",
	"Lactate threshold",
	"Countdown",
	"StopWatch",
	"Location",
	"Compass",
	"Ambient"
};

const char** ToolStrs[] = {cToolStrs,eToolStrs};


/*const char* cSportStrs[] = 
{
	"�ܲ�",
	"������",
	"ԽҰ��",
	"������",
	"����",
	"��ɽ",
	"����",
	"ͽ��ԽҰ",
	"Ӿ����Ӿ",
	"��������"
};

const char* eSportStrs[] = 
{
	"Run",
	"Marathon",
	"CrosscountryRun",
	"IndoorRun",
	"FitnessWalk",
	"Climbing",
	"Cycling",
	"Hiking",
	"Swimming",
	"Triathlon"
};*/



const char* cSportStrs[] = 
{
	"�ܲ�",
	"����",
	"��ɽ",
	"Ӿ����Ӿ",
	"ͽ��ԽҰ",
	"ԽҰ��"
};

const char* eSportStrs[] = 
{
	/*"Run",
	"Marathon",
	"CrosscountryRun",
	"IndoorRun",
	"FitnessWalk",
	"Climbing",
	"Cycling",
	"Hiking",
	"Swimming",
	"Triathlon"*/
	"�ܲ�",
	"����",
	"��ɽ",
	"Ӿ����Ӿ",
	"ͽ��ԽҰ",
	"ԽҰ��"
};


const char** SportStrs[] = {cSportStrs,eSportStrs};


const char* cWeekStrs[GUI_WEEK_INDEX_MAX] =
{
	"����",
	"��һ",
	"�ܶ�",
	"����",
	"����",
	"����",
	"����",
};

const char* eWeekStrs[GUI_WEEK_INDEX_MAX] =
{
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
};

const char** WeekStrs[] = {cWeekStrs,eWeekStrs};

const char* cWeekofDayStrs[GUI_WEEK_INDEX_MAX] =
{
	"��",
	"һ",
	"��",
	"��",
	"��",
	"��",
	"��",
};

const char* eWeekofDayStrs[GUI_WEEK_INDEX_MAX] =
{
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
};

const char** WeekofDayStrs[] = {cWeekofDayStrs,eWeekofDayStrs};
const char* cCompassCalibrationStrs[] = 
{
	"����ֱ�",
	"�밴�չ켣У׼",
	"ָ����У׼"
};

const char* eCompassCalibrationStrs[] = 
{
	"Wear watch",
	"Plase calibration",
	"ָ����У׼"
};

const char** CompassCalibrationStrs[] = {cCompassCalibrationStrs,eCompassCalibrationStrs};

const char* cCompassStrs[] = 
{
	"��",
	"��",
	"��",
	"��",
};

const char* eCompassStrs[] = 
{
	"N",
	"W",
	"S",
	"E"
};

const char** CompassStrs[] = {cCompassStrs,eCompassStrs};

const char* cSportsPromptStrs[] = 
{
	"��OK����ʼ�˶�",
    "����ͣ",
    "����ɹ�",
    "���ڱ���",
    "������׼��,��OK����ʼ��Ӿ",
	"�������������",
 
};

const char* eSportsPromptStrs[] = 
{
	"Press OK to start Sport",
    "Paused",
    "Saved",
    "Saveing",
	"Please prepare and Press OK to start swimming",
	"Triathlon Finish",
	
};


const char** SportsPromptStrs[] = {cSportsPromptStrs,eSportsPromptStrs};

const char	jixu[] =            {"����"};
const char	baocun[] =          {"�����˶�"};
const char	shaohpujixu[] =     {"�Ժ����"};
const char	xunjifanhang[] =    {"ѭ������"};
const char	fangqi[] =          {"����"};
const char	jieshusaiduan[] =   {"��������"};

const char	ejixu[] =            {"continue"};
const char	ebaocun[] =          {"end sport"};
const char	eshaohpujixu[] =     {"continue later"};
const char	exunjifanhang[] =    {"tracking back"};
const char	efangqi[] =          {"give up"};
const char	ejieshusaiduan[] =   {"end schedule"};

const char* cRunPauseStrs[] = 
{
	jixu,
	baocun,
	xunjifanhang,
	fangqi,
};

const char* eRunPauseStrs[] = 
{
	ejixu,
	ebaocun,
	exunjifanhang,
	efangqi,

};

const char** RunPauseStrs[] = {cRunPauseStrs,eRunPauseStrs};

const char* cSportEventPauseStrs[] = 
{
	jixu,
	jieshusaiduan,
	baocun,
	shaohpujixu,
	xunjifanhang,
	fangqi,
};

const char* eSportEventPauseStrs[] = 
{
	ejixu,
	ejieshusaiduan,
	ebaocun,
	eshaohpujixu,
	exunjifanhang,
	efangqi,
};

const char** SportEventPauseStrs[] = {cSportEventPauseStrs,eSportEventPauseStrs};

const char* cClimbPauseStrs[] = 
{
	jixu,
	baocun,
	shaohpujixu,
	xunjifanhang,
	fangqi,
};

const char* eClimbPauseStrs[] = 
{
	ejixu,
	ebaocun,
	eshaohpujixu,
	exunjifanhang,
	efangqi,

};

const char** ClimbPauseStrs[] = {cClimbPauseStrs,eClimbPauseStrs};

// Addin Pause Strings for Swimming and CrossCountryRunning, Jason V1.10

const char* cSwimPauseStrs[] = 
{
	jixu,
	baocun,
	fangqi,
};

const char* eSwimPauseStrs[] = 
{
	ejixu,
	ebaocun,
	efangqi,

};

const char** SwimPauseStrs[] = {cSwimPauseStrs,eSwimPauseStrs};


const char* cIndoorrunPauseStrs[] = 
{
	jixu,
	baocun,
	shaohpujixu,
	fangqi,
};

const char* eIndoorrunPauseStrs[] = 
{
	ejixu,
	ebaocun,
	eshaohpujixu,
	efangqi,
};
const char** IndoorrunPauseStrs[] = {cIndoorrunPauseStrs,eIndoorrunPauseStrs};


const char* cCrossPauseStrs[] = 
{
	jixu,
	baocun,
	shaohpujixu,
	xunjifanhang,
	fangqi,
};

const char* eCrossPauseStrs[] = 
{
	ejixu,
	ebaocun,
	eshaohpujixu,
	exunjifanhang,
	efangqi,
};

const char** CrossPauseStrs[] = {cCrossPauseStrs,eCrossPauseStrs};

const char* cDotStrs[] = 
{
	"��ע��ﵽ����",
	"GPS��λ���ɹ�",
};

const char* eDotStrs[] = 
{
	"dot numbers out",
	"unvalid gps",


};

const char** DotStrs[] = {cDotStrs,eDotStrs};

//Jason's code ends in here

const char* cCancelStrs[] = 
{
	"��",
	"��",
};

const char* eCancelStrs[] = 
{
	"NO",
	"YES",


};

const char** CancelStrs[] = {cCancelStrs,eCancelStrs};

const char* cSportFeelStrs[] = 
{
    "����,������",
    "��������",
    "������",
	"����",
	"�е����",
	"����",
	"�ǳ�����",
	"�������",
	"��ƣ����",
};

const char* eSportFeelStrs[] = 
{
    "Ease",
    "Extremely Easy",
    "Very Easy",
	"Easy",
	"A bit tired",
	"Tired",
	"Very Tired",
	"Extremely Tired",
	"Exhausted",
	

};

const char** SportFeelStrs[] = {cSportFeelStrs,eSportFeelStrs};

const char* cSportDetailStrs[] = 
{
	"����",         //0
    "ʱ��",
    "ƽ������",
    "�������",
    "��·��",
    "ƽ������",		//5
    "�������",
    "����Ч��",
    "����Ч��",
    "�ָ�ʱ��",
    "ƽ���ٶ�",     //10
    "����ٶ�",
    "�켣",
    "��������ͼ",
    "����",
	"ƽ��̤Ƶ",		//15
	"�����߶�",
	"�½��߶�",
	"ƽ����Ƶ",		//18			//Added in by Jason for Swimming Purpose
	"��ӾЧ��",								
	"Ӿ�س���",		//20						
	"��Ӿ����",								
	"ƽ��Swolf",
	"����",			//23			//Added in by Jason for Swimming Purpose
	"ƽ����Ƶ",
	"��Ƶ",			//25
	"��/min",
	"��ʱ��",
	"ת��ʱ��",
	"��һת��",
	"�ڶ�ת��",		//30
	"�ۼ�����",
	"�ۼ��½�",
	"��ֱ�ٶ�",
	"ʣ�����",
	"��Ӿ",			//35
	"����켣",
	"���ڹ켣��",
	"δ�ڹ켣��",
	"����",
	"��Ƶ",			//40
	"��һȦ����",
	"���ʻָ���",
};

const char* eSportDetailStrs[] = 
{
	"Mileage",		//0
    "Time",
    "Avg Pace",
    "Max Pace",
    "Calorie",
    "Avg BMP",		//5
    "Max BMP",
    "Aerobic",
    "Anaerobic",
    "Recovery",
    "Avg Speed",	//10
    "Max Speed",
    "Track",
    "Heartrate Zone",
    "Zone",
	"A-RPM",		//15
	"Rise Height",
	"Drop Height",
	"Avg Stroke",				//Added in by Jason for Swimming Purpose
	"Swolf", 						
	"PoolLength",	//20				
	"Stroke",						//Added in by Jason for Swimming Purpose
	"Cadence",
	"Average pace",
	"Stroke",
	"Frequency",	//25
	"/min",
	"Total Time",
	"Change Time",
	"First",
	"Second",		//30
	"Total Ascending Distance",
	"Total Descending Dist",
	"Vertical Speed",
	"Distance",
	"Swimming",		//35
	"Distance",
	"tracked",
	"untracked",
	"Speed",
	"Stride",		//40
	"Last",
    "HeartRate Recovery",
};

const char** SportDetailStrs[] = {cSportDetailStrs,eSportDetailStrs};




const char* csportdetailstrsgod[] = 
{
	"����",         //0
    "ʱ��",
    "ƽ������",
    "ƽ������",		
    "��·��",       //4
    "�ܲ���",   //5
    "ƽ����Ƶ",
    "ƽ������",
    "�ۻ�����",
    "�������",
    "�������",   //10
    "����Ч��",
    "����Ч��",
    "�ָ�ʱ��",
    "ƽ���ٶ�",   
    "����ٶ�",//15
    "�켣",
    "��������ͼ",
    "����",
	"ƽ��̤Ƶ",		//19
	"��������",//�����߶�
	"�½��߶�",
	"ƽ����Ƶ",					//Added in by Jason for Swimming Purpose
	"��ӾЧ��",								
	"Ӿ�س���",							
	"��Ӿ��Ƶ",								
	"ƽ��Swolf",
	"��ˮ����",			//27		//Added in by Jason for Swimming Purpose
	"ƽ����Ƶ",
	"��Ƶ",			
	"��/min",   //30
	"��ʱ��",
	"ת��ʱ��",
	"��һת��",
	"�ڶ�ת��",		
	"�ۼ�����",
	"�ۼ��½�",
	"��ֱ�ٶ�",
	"ʣ�����",
	"��Ӿ",			
	"����켣",//40
	"���ڹ켣��",
	"δ�ڹ켣��",
	"����",
	"��Ƶ",			
	"��һȦ����",
	"���ʻָ���",//46
	"��ߺ���",//47
	"�������"
};

const char* esportdetailstrsgod[] = 
{
	"����",         //0
    "ʱ��",
    "ƽ������",
    "ƽ������",		//5
    "��·��",
    "�ܲ���",
    "ƽ������",
    "ƽ����Ƶ",
    "�ۻ�����",
    "�������",
    "�������",
    "����Ч��",
    "����Ч��",
    "�ָ�ʱ��",
    "ƽ���ٶ�",     
    "����ٶ�",
    "�켣",
    "��������ͼ",
    "����",
	"ƽ��̤Ƶ",		
	"�����߶�",
	"�½��߶�",
	"ƽ����Ƶ",				//Added in by Jason for Swimming Purpose
	"��ӾЧ��",								
	"Ӿ�س���",							
	"��Ӿ��Ƶ",								
	"ƽ��Swolf",
	"����",					//Added in by Jason for Swimming Purpose
	"ƽ����Ƶ",
	"��Ƶ",			
	"��/min",
	"��ʱ��",
	"ת��ʱ��",
	"��һת��",
	"�ڶ�ת��",		
	"�ۼ�����",
	"�ۼ��½�",
	"��ֱ�ٶ�",
	"ʣ�����",
	"��Ӿ",			
	"����켣",
	"���ڹ켣��",
	"δ�ڹ켣��",
	"����",
	"��Ƶ",			
	"��һȦ����",
	"���ʻָ���",
	"��ߺ���",//47
	"�������"

};

const char** sportdetailstrsgod[] = {csportdetailstrsgod,esportdetailstrsgod};



const char* cMenuStrs[GUI_MENU_INDEX_MAX] = 
{
	"����",
	"ʱ��",
	"ϵͳ",
	"�Զ����˶�",
	"�Զ��幦��",
	"�������",
	"�����ֱ�",	


};

const char* eMenuStrs[GUI_MENU_INDEX_MAX] = 
{
	"THEME",
	"TIME",
	"SYSTEM",
	"CUSTOMIZED_SPORTS"
	"CUSTOMIZED_TOOLS"
	"ACCESSORY",
	"WATCH INFO",

};

const char** MenuStrs[] = {cMenuStrs,eMenuStrs};



const char* cThemeStrs[GUI_THEME_INDEX_MAX] = 
{
	"Ĭ������",
	"�Ƿ�Ӧ��?",
	"�����Ѹ���",
	"�Զ���",
	"���",
	"�����������",
	"������APP����"
};

const char* eThemeStrs[GUI_THEME_INDEX_MAX] = 
{
	"DIGITAL",
	"APPLICATION",
	"Theme Update",
	"Setting",
	"Add",
	"For More Theme",
	"Please Connect APP"
};

const char** ThemeStrs[] = {cThemeStrs,eThemeStrs};


const char* cThemeNameStrs[] = 
{
	"Ĭ������",
	"Ĭ������2"
};

const char* eThemeNameStrs[] = 
{
	"Theme1",
	"Theme2"
};


const char** ThemeNameStrs[] = {cThemeNameStrs,eThemeNameStrs};

const char* cTimeStrs[GUI_TIME_INDEX_MAX] = 
{
#if defined WATCH_TIMEZONE_SET
	"ʱ������",
#endif
	"������ʱ",
	
	"����",
};

const char* eTimeStrs[GUI_TIME_INDEX_MAX] = 
{
#if defined WATCH_TIMEZONE_SET
	"TIMEZONE SET",
#endif
	"UTC GET",
	"ALARM",
};

const char** TimeStrs[] = {cTimeStrs,eTimeStrs};

#if defined WATCH_TIMEZONE_SET
const char* cTimeZoneCountiesStrs[GUI_TIME_ZONE_COUNTIES_INDEX_MAX] = 
{
	"��;��",
	"̴��ɽ",
	"�¿���",
	"��ɼ�",
	"����",
	"֥�Ӹ�",
	"ŦԼ",
	"������˹",
	"ʥ���Ǹ�",
	"ʥԼ��",
	"��Լ����¬",
	"�Ѷ��϶��ŵ������",
	"���ٶ�Ⱥ��",
	"�׶�",
	"����",
	"����",
	"Ī˹��",
	"�º���",
	"�ϰ�",
	"������",
	"������",
	"�µ���",
	"�ӵ�����",
	"�￨",
	"����",
	"����",
	"����",
	"����",
	"��������",
	"Ϥ��",
	"Ŭ����",
	"�����",
};
const char* eTimeZoneCountiesStrs[GUI_TIME_ZONE_COUNTIES_INDEX_MAX] = 
{
	"MDY",
	"HNL",
	"ANC",
	"LAX",
	"DEN",
	"CHI",
	"NYC",
	"CCS",
	"SCL",
	"YYT",
	"RIO",
	"FEN",
	"PDL",
	"LON",
	"PAR",
	"CAI",
	"MOW",
	"THR",
	"DXB",
	"KBL",
	"KHI",
	"DEL",
	"KTM",
	"DAC",
	"RGN",
	"BKK",
	"BJS",
	"TYO",
	"ADL",
	"SYD",
	"NOU",
	"WLG",
};

const char** TimeZoneCountiesStrs[] = {cTimeZoneCountiesStrs,eTimeZoneCountiesStrs};

const char* cTimeZoneConfirmStrs[] = 
{
	"ȷ��ʹ�ô�ʱ��?",
};

const char* eTimeZoneConfirmStrs[] = 
{
	"Confirm Timezone?",
};



const char** TimeZoneConfirmStrs[] = {cTimeZoneConfirmStrs,eTimeZoneConfirmStrs};


#endif
const char* cAlarmSetStrs[GUI_ALARM_SET_INDEX_MAX] = 
{
	"����ʱ��",
	"����",
	"�ظ�",
	"״̬",
};

const char* eAlarmSetStrs[GUI_ALARM_SET_INDEX_MAX] = 
{
	"TIME SET",
	"SOUND",
	"REPITION",
	"STATUS",	
};

const char** AlarmSetStrs[] = {cAlarmSetStrs,eAlarmSetStrs};



const char* cAlarmSetSoundNViberateStrs[GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX] = 
{
	"��������",
	"����",
	"��",
};

const char* eAlarmSetSoundNViberateStrs[GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX] = 
{
	"SOUND AND VIBERATE",
	"VIBERATE",	
	"SOUND",
};

const char** AlarmSetSoundNViberateStrs[] = {cAlarmSetSoundNViberateStrs,eAlarmSetSoundNViberateStrs};


const char* cAlarmSetRepitionStrs[GUI_ALARM_SET_REPITION_INDEX_MAX] = 
{
	"����",
	"ÿ��",
	"������",
	"��ĩ",
};

const char* eAlarmSetRepitionStrs[GUI_ALARM_SET_REPITION_INDEX_MAX] = 
{
	"SINGLE",
	"DAILY",
	"WEEK DAY",
	"WEEKEND",
};

const char** AlarmSetRepitionStrs[] = {cAlarmSetRepitionStrs,eAlarmSetRepitionStrs};






const char* cCustomizedToolsStrs[GUI_CUSTOMIZED_TOOLS_INDEX_MAX] = 
{  
   
    "�ָ�ʱ��",
	"���������",
	"������",
	"����ʱ",
	"���",
	"��γ��",
	"ָ����",
	"������ѹ"	

};

const char* eCustomizedToolsStrs[GUI_CUSTOMIZED_TOOLS_INDEX_MAX] = 
{
    
	 "recover",
	 "vo2max",
	 "threshold",
	 "Countdown",
	 "StopWatch",
	 "GPS",
	"COMPASS",
	"ambient"
};

const char** CustomizedToolsStrs[] = {cCustomizedToolsStrs,eCustomizedToolsStrs};



const char* cCustomizedMonitorStrs[GUI_CUSTOMIZED_MONITOR_INDEX_MAX] = 
{  
    "����",
	"��·��",
	"����",
	"�˶���¼",
	"��Ϣ֪ͨ",
	"��������"

};

const char* eCustomizedMonitorStrs[GUI_CUSTOMIZED_MONITOR_INDEX_MAX] = 
{
     "step",
	 "calories",
	 "heartrate",
	 "motions_record",
	 "notification",
	 "Calendar"
};

const char** CustomizedMonitorStrs[] = {cCustomizedMonitorStrs,eCustomizedMonitorStrs};




const char* cCustomizedSportsStrs[GUI_CUSTOMIZED_SPORTS_INDEX_MAX] = 
{
	
	"�ܲ�",
	"����",
	"��ɽ",
	"Ӿ����Ӿ",
	"ͽ��ԽҰ",
	"ԽҰ��"
};

const char* eCustomizedSportsStrs[GUI_CUSTOMIZED_SPORTS_INDEX_MAX] = 
{
	
	"RUN",
	"BIKE",
	"MOUNTAIN",
	"SWIM",	
	"HIKING",
	"CROSS COUNTRY RUN"
};

const char** CustomizedSportsStrs[] = {cCustomizedSportsStrs,eCustomizedSportsStrs};

const char** GuiHomeLockStrs[] = {cGuiHomeStrs,eGuiHomeStrs};


const char* cGuivicescreensport[GUI_VICE_SPORTS_INDEX_MAX] = 
{
	
	"��ʼ�ܲ�",
	"��ʼ����",
	"��ʼ����",
	"��ʼ��ɽ"
};

const char* eGuivicescreensport[GUI_VICE_SPORTS_INDEX_MAX] = 
{
	"��ʼ�ܲ�",
	"��ʼ����",
	"��ʼ����",
	"��ʼ��ɽ"
};



const char** Guivicescreensport[] = {cGuivicescreensport,eGuivicescreensport};



const char* cSystemStrs[GUI_SYSTEM_INDEX_MAX] = 
{
	"����",
	"��������",
	"�Զ�����",
	"��������",
	"����",
	"����",
	"����״̬",
	"̧������",//7
};

const char* eSystemStrs[GUI_SYSTEM_INDEX_MAX] = 
{
	"BLUETOOTH",
	"BASE HEARTRATE",
	"AUTO SCREEN LOCK",
	"LANGUGAGE",
	"BACKLIGHT",
	"SOUND",
	"SILENT",
	"AUTO BACKLIGHT",

};

const char** SystemStrs[] = {cSystemStrs,eSystemStrs};








const char* cLanguageStrs[GUI_LANGUAGE_INDEX_MAX] = 
{
	"����",
	"Ӣ��",
};

const char* eLanguageStrs[GUI_LANGUAGE_INDEX_MAX] = 
{
	"CHINESE",
	"ENGLISH",
};

const char** LanguageStrs[] = {cLanguageStrs,eLanguageStrs};

//ϵͳ��ʾ��
const char* cSysHintStrs[] = 
{
	"������",
	"�豸δ����",
	"ϵͳ",
	"����",
	"�ر�",
	"ʱ�������",
};

const char* eSysHintStrs[] = 
{
	"Connect",
	"Disconnect",
	"SYSTEM",
	"ON",
	"OFF",
	"Time Set",
};

const char** SysHintStrs[] = {cSysHintStrs,eSysHintStrs};



const char* cBacklightTimeStrs[GUI_BACKLIGHT_TIME_INDEX_MAX] = 
{
	" 7��",
	"15��",
	"30��",
};

const char* eBacklightTimeStrs[GUI_BACKLIGHT_TIME_INDEX_MAX] = 
{
	" 7SEC",
	"15SEC",
	"30SEC",
};

const char** BacklightTimeStrs[] = {cBacklightTimeStrs,eBacklightTimeStrs};





const char* cSystemSoundNViberateStrs[GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX] = 
{
	"��������",
	"����",
	"��",
};

const char* eSystemSoundNViberateStrs[GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX] = 
{
	"SOUND AND VIBERATE",
	"VIBERATE",	
	"SOUND",
};

const char** SystemSoundNViberateStrs[] = {cSystemSoundNViberateStrs,eSystemSoundNViberateStrs};

const char* cStopwatchStrs[] = 
{
	"�ƴ�",
	"��Ȧ",
	"��OK����ʼ/��ͣ",
	"UP���ƴ�,DOWN����Ȧ",
	"����",
	"���",
	"�鿴",
	"����"
};

const char* eStopwatchStrs[] = 
{
	"Count",
	"Lap",
	"Press OK to start/pause",
	"UP count,DOWN lap",
	"Continue",
	"complete",
	"View",
	"Reset"
};

const char** StopwatchStrs[] = {cStopwatchStrs,eStopwatchStrs};



const char* cNotifiAppStrs[] = 
{
	"����",
	"΢��",
	"QQ",
	"����",
	"��ҵ΢��",
	"����"
	
};

const char* eNotifiAppStrs[] = 
{
	"����",
	"΢��",
	"QQ",
	"����",
	"��ҵ΢��",
	"����"
};

const char** NotifiAppStrs[] = {cNotifiAppStrs,eNotifiAppStrs};


const char* cWindScaleStrs[WIND_SCALE_INDEX_MAX] = 
{
	"�޷�",
	"���",
	"���",
	"΢��",
	"�ͷ�",
	"����",
	"ǿ��",
	"����",
	"���",
	"�ҷ�",
	"���",
	"����",
	"쫷�",
};

const char* eWindScaleStrs[WIND_SCALE_INDEX_MAX] = 
{
	"NO_WIND",
	"LIGHT_AIR",
	"MILD_AIR",
	"LIGHT_WIND",
	"WIND",
	"STRONG_WIND",
	"INTENSE_WIND",
	"SPEED_WIND",
	"HEAVY_WIND",
	"HUG_WIND",
	"TORNADO",
	"STORM",
	"HARRICANE",

};

const char** WindScaleStrs[] = {cWindScaleStrs,eWindScaleStrs};


const char* cWindDirectStrs[WIND_DIRECT_INDEX_MAX] = 
{
	"����",
	"������",
	"����",
	"���Ϸ�",
	"�Ϸ�",
	"���Ϸ�",
	"����",
	"������",
	"�޳�������",

};

const char* eWindDirectStrs[WIND_DIRECT_INDEX_MAX] = 
{
	"NORTH",
	"NORTHEAST",
	"EAST",
	"SOUTHEAST",
	"SOUTH",
	"SOUTHWEST",
	"WEST",
	"NORTHWEST",
	"UNPREDICATE",

};

const char** WindDirectStrs[] = {cWindDirectStrs,eWindDirectStrs};


const char* cAirQualityStrs[AIR_QUALITY_INDEX_MAX] = 
{
	"��",
	"��",
	"����Ⱦ",
	"����Ⱦ",
	"ǿ��Ⱦ",
	"����Ⱦ",
};

const char* eAirQualityStrs[AIR_QUALITY_INDEX_MAX] = 
{
	"GOOD",
	"WELL",
	"LIGHT",
	"MILD",
	"HEAVY",
	"INTENSE",


};

const char** AirQualityStrs[] = {cAirQualityStrs,eAirQualityStrs};



const char* cAccessoryStrs[GUI_ACCESSORY_INDEX_MAX] = 
{
	"����",
	"���ʴ�",
	"��Ƶ������",
	"̤Ƶ������",
};

const char* eAccessoryStrs[GUI_ACCESSORY_INDEX_MAX] = 
{
	"SWTICH",
	"HEARTRATE",
	"RUNNING",
	"CYCLING",
};

const char** AccessoryStrs[] = {cAccessoryStrs,eAccessoryStrs};

const char* cNotifyMenuStrs[GUI_NOTIFY_MENU_INDEX_MAX] = 
{
	"���ʸ澯",							//0
	"��ϲ�����ղ���",
	"��ϲ����������",
	"Ŀ�������",
	"Ŀ��Ȧ��",
	"Ŀ��ʱ��",					//5
	"Ŀ�����",
	"Ŀ������",
	"��ǰ�߶�",
	"�����!",
	"Ŀ�꿨·��",						//10
	"Ŀ���ٶ�",
	"��ʱ",
	"�ٶ�",
	
};

const char* eNotifyMenuStrs[GUI_NOTIFY_MENU_INDEX_MAX] = 
{
	"HEARTRATE WARNING",
	"Congratulation! Your step goal",
	"Congratulation! Your calory goal",
	"today has been completed",
	"Target Laps",
	"Target Exercise Time",
	"Target Exercise Distance",
	"Current Pace",
	"Current Altitude",
	"COMPLETED !",
	"Target Calory",
	"Target Speed",
	"Time",
	"Speed",
};

const char** NotifyMenuStrs[] = {cNotifyMenuStrs,eNotifyMenuStrs};

const char* cAmbientStrs[] = 
{
	"��ѹ",
	"����",
	"��ȥ12Сʱ",
	"��ȥ5Сʱ",
	"����У׼"
};

const char* eAmbientStrs[] = 
{
	"Pressure",
	"Altitude",
	"After 12h",
	"After 5h",
	"Alt Cali"
};

const char** AmbientStrs[] = {cAmbientStrs,eAmbientStrs};

const char* cSOSStrs[] = 
{
	"���ڷ�������ź�...",
	"�˳�����"
};

const char* eSOSStrs[] = 
{
	"SOS",
	"Exit SOS",
};

const char** SOSStrs[] = {cSOSStrs,eSOSStrs};

const char* cNotifyWarningStrs[GUI_NOTIFY_WARNING_INDEX_MAX] = 
{
	"��ʾ",
	"���ȿ�������",
	"ϵͳ>����",
};

const char* eNotifyWarningStrs[GUI_NOTIFY_WARNING_INDEX_MAX] = 
{
	"HINT",
	"Please turn on BLE first",
	"SYSTEM>BLE",
};
const char** NotifyWarningStrs[] = {cNotifyWarningStrs,eNotifyWarningStrs};

const char* cFindphoneStrs[] = 
{
	"����Ѱ���ֻ�",
	"δ�����ֻ�����",
};

const char* eFindphoneStrs[] = 
{
	"Find Phone",
	"Ble Disconnect",
};
const char** FindphoneStrs[] = {cFindphoneStrs,eFindphoneStrs};

const char* cFindwatchStrs[] = 
{
	"Ѱ�������...",
	"��OK��ȷ��",
};

const char* eFindwachStrs[] = 
{
	"Finding watch...",
	"Press OK to confirm",
};
const char** FindwatchStrs[] = {cFindwatchStrs,eFindwachStrs};
//��������
const char* cHeartZoneStrsStrs[] = 
{
	"����",
	"��������",
	"��������",
	"ȼ֬",
	"����",	
};

const char* eHeartZoneStrsStrs[] = 
{
	"Extremity",
	"Anaerobic ",
	"Aerobic",
	"FatBurn",
	"Warm-up",
};

const char** HeartZoneStrs[] = {cHeartZoneStrsStrs,eHeartZoneStrsStrs};

/*������Ŀ����*/
const char*cheartzonegodstrs[] = 
{
   "�������",
   "����ȼ֬",
   "��������",
   "��������",
   "���޳��"


};
const char*eheartzonegodstrs[] = 
{
	"�������",
	"����ȼ֬",
	"��������",
	"��������",
	"���޳��"
};



const char** heartzonegodstrs[] = {cheartzonegodstrs,eheartzonegodstrs};


//�˶�׼��
const char* cSportReadyStrs[] = 
{
	"������ʾ",
	"��ʼ",
	"�˶�����",
	"ƽ��",
	"ʵʱ",
	"���",
	"�ر�",
	"����",
	"�Ƽ�ֵ",
	"Ȧ",
	"�����ӵ��������"
	
};

const char* eSportReadyStrs[] = 
{
	"Data Display",
	"Start",
	"Sports Set",
	"AVE",
	"RealTime",
	"Optimum",
	"OFF",
	"ON",
	"Recommend",
	"Laps",
	"Need connect accessory"

};

const char** SportReadyStrs[] = {cSportReadyStrs,eSportReadyStrs};


//�˶�������ʾ
const char* cSportDataDisStrs[] = 
{
	"����",
	"ʱ��",
	"ƽ������",
	"ʵʱ����",
	"�ܲ���",
	"�ۻ�����",
	"����",
	"ƽ���ٶ�",
	"ʵʱ�ٶ�",
    "���θ߶�",
   	"����",
   	"��һ��SWOLF",
	"��ˮ����",
	"����ƽ������",//��Ӿ��ƽ������
	"��ֱ�ٶ�",
	"�������",
	"ƽ������",
	"ʵʱ��Ƶ",
	"ƽ����Ƶ",
	"��·��",
	"��һȦ����",
	"��һȦʱ��",
	"����ٶ�",
	"ʵʱ̤Ƶ",
	"ƽ��̤Ƶ",
	"�ܻ���",
	"ƽ������",
	"ƽ��swolf"
	
	
};

const char* eSportDataDisStrs[] = 
{
	"����",
	"ʱ��",
	"ƽ������",
	"ʵʱ����",
	"�ܲ���",
	"�ۻ�����",
	"����",
	"ƽ���ٶ�",
	"ʵʱ�ٶ�",
    "���θ߶�",
   	"����",
   	"��һ��SWOLF",
	"��ˮ����",
	"����ƽ������",//��Ӿ��ƽ������
	"��ֱ�ٶ�",
	"�������",
	"ƽ������",
	"ʵʱ��Ƶ",
	"ƽ����Ƶ",
	"��·��",
	"��һȦ����",
	"��һȦʱ��",
	"����ٶ�",
	"ʵʱ̤Ƶ",
	"ƽ��̤Ƶ",
	"�ܻ���",
	"ƽ������",
	"ƽ��swolf"
	
	
};

const char** SportDataDisStrs[] = {cSportDataDisStrs,eSportDataDisStrs};


/*Ԥ�������˶����ݳ�ʼֵ*/
/*const char*csportsatapreview[] = 
{
  "00.00",
  "0:00:00",
  "0:00",
  "0:00",
   "0",//�ܲ���
   "0",
   "0",//����
   "0.0",
   "0.0",
   "0",
   "0",//����
   "0",
   "0",
   "0",
   "00.0"


};

const char*esportsatapreview[] = 
{
	 "00.00 km",
  "0:00:00",
  "0:00 /km",
  "0:00 /km",
   "0",//�ܲ���
   "0 m",
   "0",//����
   "0.0 km/h",
   "0.0 km/h",
   "0 m",
   "0",//����
   "0",
   "0 /min",
   "0 /100m",
   "00.0 km/h"




};


const char** sportsatapreview[] = {csportsatapreview,esportsatapreview};*/


//�˶���������
const char* cSportRemindStrs[] = 
{
	"���ʸ澯",
	"���ʻָ���",
	"��������",
	"��������",
	"�Զ���Ȧ",
	"�ٶ�����",
    "�߶�����",//����
	"Ӿ�س���",
	"ʱ������",
	"��Ȧ����"
	//"ȼ֬Ŀ��",
	
	//"�����ܲ���",
	
};

const char* eSportRemindStrs[] = 
{
	/*"Herat Remind",
	"Pace Remind",
	"Distance Remind",
	"Auto Circle",
	"Goal Kcal",
	"Time Remind",
	"Speed Remind",
	"swim pool",
	"Circle Remind",
	"IndoorRunLength ",
	"HeartRate Recovery",*/
	"���ʸ澯",
	"���ʻָ���",
	"��������",
	"��������",
	"�Զ���Ȧ",
	"�ٶ�����",
    "�߶�����",//����
	"Ӿ�س���",
	"ʱ������",
	"��Ȧ����"
	//"ȼ֬Ŀ��",
	//"��Ȧ����",
	//"�����ܲ���",
};

const char** SportRemindStrs[] = {cSportRemindStrs,eSportRemindStrs};



const char* cCountdownStrs[] = 
{
	"���뵹��ʱ",
	"ʱ������",
	"��ʼ",
	"�Ƿ��˳���",
	"��",
	"��",
	"����ʱ����"
};

const char* eCountdownStrs[] = 
{
	"Countdown",
	"Time",
	"Start",
	"Exit?",
	"No",
	"Yes",
	"Countdown Finish"
};
const char** CountdownStrs[] = {cCountdownStrs,eCountdownStrs};


/*������������/SOS����Ѿ���Ϣ*/
const char* cGuiHomeScreenNameStrs[] = 
{
	"���������",//0
	"�˶���¼",
	"֪ͨ",
	"��·��",
	"������",
	"ѵ���ƻ�",//5
	"�ָ�ʱ��",
	"��",
	"�Ƴ�",
	"���",
	"λ���ѷ���",//10
	"�����ѷ���",
	"�˳�����",
	"�����Ƴ���",
};

const char* eGuiHomeScreenNameStrs[] = 
{
	"VO2max",
	"�˶���¼",
	"֪ͨ",
	"��·��",
	"������",
	"ѵ���ƻ�",
	"�ָ�ʱ��",
	"��",
	"�Ƴ�",
	"���",
	"λ���ѷ���",//10
	"�����ѷ���",
	"�˳�����",
	"�����Ƴ���",
};

const char** GuiHomeScreenNameStrs[] = {cGuiHomeScreenNameStrs,eGuiHomeScreenNameStrs};

/*�����Ʋ�����:У׼������ʾ��Ϣ*/
const char* cGuiStepScreenStrs[] = 
{
	"����У׼",//0
	"��ƽֱ·������",
	"����200��+",
	"OK������У׼",
	"������������200������;",
	"У׼ʱ�����ƽֱ·������;",//5
	"OK������У׼",
	"OK������У׼",
	"ѡ���Ƿ���Ϊ����",
	"��������?",
};

const char* eGuiStepScreenStrs[] = 
{
	"����У׼",//0
	"��ƽֱ·������",
	"����200��+",
	"OK������У׼",
	"������������200������;",
	"У׼ʱ�����ƽֱ·������;",//5
	"OK������У׼",
	"OK������У׼",
	"ѡ���Ƿ���Ϊ����",
	"��������?",
};

const char** GuiStepScreenStrs[] = {cGuiStepScreenStrs,eGuiStepScreenStrs};

/*ͽ��ԽҰ��ԽҰ�ܵ����������ַ�*/
const char* cGuiSportEventStrs[] = 
{
	"������ɱ���?",//0
	"����",
	"���¿�ʼ�˶�?",
	"���¿�ʼ",
	"���ν���",
	"��������(��",//5
	"����)",
	"�����ۼ�",
	"�����",
	"��ʱ��",
	"��ƽ������",//10
	"ʱ��",
	"���",
	"�����",
	"��ʱ��",
	"ʵʱ����",//15
	"�ۼ�����",
	"�ۼ��½�",
	"ʵʱ����",
	"����ʱ��",
	"����CP",//20
	"����ʱ�仹ʣ",
	"ƫ��켣",
};

const char* eGuiSportEventStrs[] = 
{
	"������ɱ���?",//0
	"����",
	"���¿�ʼ�˶�?",
	"���¿�ʼ",
	"���ν���",
	"��������(��",//5
	"����)",
	"�����ۼ�",
	"�����",
	"��ʱ��",
	"��ƽ������",//10
	"ʱ��",
	"���",
	"�����",
	"��ʱ��",
	"ʵʱ����",//15
	"�ۼ�����",
	"�ۼ��½�",
	"ʵʱ����",
	"����ʱ��",
	"����CP",//20
	"����ʱ�仹ʣ",
	"ƫ��켣",
};

const char** GuiSportEventStrs[] = {cGuiSportEventStrs,eGuiSportEventStrs};

const char* cGuiLeapSecondStrs[] = 
{
	"������ʱ",//0
	"����ģʽ",
	"���������",
	"�������ĵȴ�",
	"������ճɹ�",
	"�������ʧ��",//5
	"�����ٴν�����",
	"�����¸�����ʱ��",
	"������ʱ",
};

const char* eGuiLeapSecondStrs[] = 
{
	"������ʱ",//0
	"����ģʽ",
	"���������",
	"�������ĵȴ�",
	"������ճɹ�",
	"�������ʧ��",//5
	"�����ٴν�����",
	"�����¸�����ʱ��",
	"������ʱ",
};


const char** GuiLeapSecondStrs[] = {cGuiLeapSecondStrs,eGuiLeapSecondStrs};
#ifdef COD 
const char* cTrainStrs[] = 
{
	"������ʼѵ��",//0
	"׼��",
	"��Ϣ",
	"��",
	"����",
	"����OK����ͣѵ��",//5
	"ֱ��ѵ����",
	"ѵ����",
	"�˶���ͣ",
	"�¼�����",
	"�鿴����",//10
	"ѵ�����",
	"δ֪�˶�",
	"������",
	"�ϼ�����",
};

const char* eTrainStrs[] = 
{
	"t",
};

const char** TrainStrs[] = {cTrainStrs,eTrainStrs};

const char* ctrainpauseStrs[] = 
{
	"����",
	"����" 
};

const char* etrainpauseStrs[] = 
{
	"����",
	"����" 

};

const char** trainpauseStrs[] = {ctrainpauseStrs,etrainpauseStrs};

#endif

const char* csportbarstrs[] = 
{
	"��һ��",
	"�ڶ���",
	"������",
	"������"

};

const char* esportbarstrs[] = 
{
	"��һ��",
	"�ڶ���",
	"������",
	"������"
};

const char** sportbarstrs[] = {csportbarstrs,esportbarstrs};





