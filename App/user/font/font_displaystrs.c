#include "drv_lcd.h"
#include "font_config.h"
#include "font_displaystrs.h"
#include "watch_config.h"

const char* cMonthStrs[] = 
{
	"一月",
	"二月",
	"三月",
	"四月",
	"五月",
	"六月",
	"七月",
	"八月",
	"九月",
	"十月",
	"十一",
	"十二"
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
	"初一",
	"初二",
	"初三",
	"初四",
	"初五",
	"初六",
	"初七",
	"初八",
	"初九",
	"初十",
	"十一",
	"十二",
	"十三",
	"十四",
	"十五",
	"十六",
	"十七",
	"十八",
	"十九",
	"廿十",
	"廿一",
	"廿二",
	"廿三",
	"廿四",
	"廿五",
	"廿六",
	"廿七",
	"廿八",
	"廿九",
	"三十",
	"三一"
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
    "恢复时间",
	"最大摄氧量",
	"乳酸阈",
	"倒计时",
	"秒表",
	"经纬度",
	"指南针",
    "海拔气压"
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
	"跑步",
	"马拉松",
	"越野跑",
	"室内跑",
	"健走",
	"登山",
	"骑行",
	"徒步越野",
	"泳池游泳",
	"铁人三项"
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
	"跑步",
	"骑行",
	"登山",
	"泳池游泳",
	"徒步越野",
	"越野跑"
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
	"跑步",
	"骑行",
	"登山",
	"泳池游泳",
	"徒步越野",
	"越野跑"
};


const char** SportStrs[] = {cSportStrs,eSportStrs};


const char* cWeekStrs[GUI_WEEK_INDEX_MAX] =
{
	"周日",
	"周一",
	"周二",
	"周三",
	"周四",
	"周五",
	"周六",
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
	"日",
	"一",
	"二",
	"三",
	"四",
	"五",
	"六",
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
	"佩戴手表",
	"请按照轨迹校准",
	"指北针校准"
};

const char* eCompassCalibrationStrs[] = 
{
	"Wear watch",
	"Plase calibration",
	"指北针校准"
};

const char** CompassCalibrationStrs[] = {cCompassCalibrationStrs,eCompassCalibrationStrs};

const char* cCompassStrs[] = 
{
	"北",
	"西",
	"南",
	"东",
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
	"按OK键开始运动",
    "已暂停",
    "保存成功",
    "正在保存",
    "请做好准备,按OK键开始游泳",
	"铁人三项已完成",
 
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

const char	jixu[] =            {"继续"};
const char	baocun[] =          {"结束运动"};
const char	shaohpujixu[] =     {"稍后继续"};
const char	xunjifanhang[] =    {"循迹返航"};
const char	fangqi[] =          {"放弃"};
const char	jieshusaiduan[] =   {"结束赛段"};

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
	"标注点达到上限",
	"GPS定位不成功",
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
	"否",
	"是",
};

const char* eCancelStrs[] = 
{
	"NO",
	"YES",


};

const char** CancelStrs[] = {cCancelStrs,eCancelStrs};

const char* cSportFeelStrs[] = 
{
    "安静,不费力",
    "极其轻松",
    "很轻松",
	"轻松",
	"有点吃力",
	"吃力",
	"非常吃力",
	"极其吃力",
	"精疲力竭",
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
	"距离",         //0
    "时长",
    "平均配速",
    "最佳配速",
    "卡路里",
    "平均心率",		//5
    "最大心率",
    "有氧效果",
    "无氧效果",
    "恢复时间",
    "平均速度",     //10
    "最佳速度",
    "轨迹",
    "心率区间图",
    "区间",
	"平均踏频",		//15
	"上升高度",
	"下降高度",
	"平均划频",		//18			//Added in by Jason for Swimming Purpose
	"游泳效率",								
	"泳池长度",		//20						
	"游泳划数",								
	"平均Swolf",
	"划数",			//23			//Added in by Jason for Swimming Purpose
	"平均步频",
	"划频",			//25
	"次/min",
	"总时长",
	"转换时长",
	"第一转换",
	"第二转换",		//30
	"累计上升",
	"累计下降",
	"垂直速度",
	"剩余距离",
	"游泳",			//35
	"距离轨迹",
	"已在轨迹上",
	"未在轨迹上",
	"配速",
	"步频",			//40
	"上一圈配速",
	"心率恢复率",
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
	"距离",         //0
    "时长",
    "平均配速",
    "平均心率",		
    "卡路里",       //4
    "总步数",   //5
    "平均步频",
    "平均步幅",
    "累积爬升",
    "最佳配速",
    "最大心率",   //10
    "有氧效果",
    "无氧效果",
    "恢复时间",
    "平均速度",   
    "最大速度",//15
    "轨迹",
    "心率区间图",
    "区间",
	"平均踏频",		//19
	"海拔上升",//上升高度
	"下降高度",
	"平均划频",					//Added in by Jason for Swimming Purpose
	"游泳效率",								
	"泳池长度",							
	"游泳划频",								
	"平均Swolf",
	"划水次数",			//27		//Added in by Jason for Swimming Purpose
	"平均步频",
	"划频",			
	"次/min",   //30
	"总时长",
	"转换时长",
	"第一转换",
	"第二转换",		
	"累计上升",
	"累计下降",
	"垂直速度",
	"剩余距离",
	"游泳",			
	"距离轨迹",//40
	"已在轨迹上",
	"未在轨迹上",
	"配速",
	"步频",			
	"上一圈配速",
	"心率恢复率",//46
	"最高海拔",//47
	"上坡里程"
};

const char* esportdetailstrsgod[] = 
{
	"距离",         //0
    "时长",
    "平均配速",
    "平均心率",		//5
    "卡路里",
    "总步数",
    "平均步幅",
    "平均步频",
    "累积爬升",
    "最佳配速",
    "最大心率",
    "有氧效果",
    "无氧效果",
    "恢复时间",
    "平均速度",     
    "最大速度",
    "轨迹",
    "心率区间图",
    "区间",
	"平均踏频",		
	"上升高度",
	"下降高度",
	"平均划频",				//Added in by Jason for Swimming Purpose
	"游泳效率",								
	"泳池长度",							
	"游泳划频",								
	"平均Swolf",
	"划数",					//Added in by Jason for Swimming Purpose
	"平均步频",
	"划频",			
	"次/min",
	"总时长",
	"转换时长",
	"第一转换",
	"第二转换",		
	"累计上升",
	"累计下降",
	"垂直速度",
	"剩余距离",
	"游泳",			
	"距离轨迹",
	"已在轨迹上",
	"未在轨迹上",
	"配速",
	"步频",			
	"上一圈配速",
	"心率恢复率",
	"最高海拔",//47
	"上坡里程"

};

const char** sportdetailstrsgod[] = {csportdetailstrsgod,esportdetailstrsgod};



const char* cMenuStrs[GUI_MENU_INDEX_MAX] = 
{
	"主题",
	"时间",
	"系统",
	"自定义运动",
	"自定义功能",
	"配件连接",
	"关于手表",	


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
	"默认主题",
	"是否应用?",
	"主题已更新",
	"自定义",
	"添加",
	"更多表盘主题",
	"请连接APP下载"
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
	"默认主题",
	"默认主题2"
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
	"时区设置",
#endif
	"卫星授时",
	
	"闹钟",
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
	"中途岛",
	"檀香山",
	"奥克拉",
	"洛杉矶",
	"丹佛",
	"芝加哥",
	"纽约",
	"加拉加斯",
	"圣地亚哥",
	"圣约翰",
	"里约热内卢",
	"费尔南多迪诺罗尼亚",
	"亚速尔群岛",
	"伦敦",
	"巴黎",
	"开罗",
	"莫斯科",
	"德黑兰",
	"迪拜",
	"喀布尔",
	"卡拉奇",
	"新德里",
	"加德满都",
	"达卡",
	"仰光",
	"曼谷",
	"北京",
	"东京",
	"阿德莱德",
	"悉尼",
	"努美阿",
	"惠灵顿",
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
	"确认使用此时区?",
};

const char* eTimeZoneConfirmStrs[] = 
{
	"Confirm Timezone?",
};



const char** TimeZoneConfirmStrs[] = {cTimeZoneConfirmStrs,eTimeZoneConfirmStrs};


#endif
const char* cAlarmSetStrs[GUI_ALARM_SET_INDEX_MAX] = 
{
	"设置时间",
	"声音",
	"重复",
	"状态",
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
	"声音加震动",
	"声音",
	"震动",
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
	"单次",
	"每日",
	"工作日",
	"周末",
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
   
    "恢复时间",
	"最大摄氧量",
	"乳酸阈",
	"倒计时",
	"秒表",
	"经纬度",
	"指南针",
	"海拔气压"	

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
    "步数",
	"卡路里",
	"心率",
	"运动记录",
	"消息通知",
	"日历天气"

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
	
	"跑步",
	"骑行",
	"登山",
	"泳池游泳",
	"徒步越野",
	"越野跑"
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
	
	"开始跑步",
	"开始健走",
	"开始骑行",
	"开始登山"
};

const char* eGuivicescreensport[GUI_VICE_SPORTS_INDEX_MAX] = 
{
	"开始跑步",
	"开始健走",
	"开始骑行",
	"开始登山"
};



const char** Guivicescreensport[] = {cGuivicescreensport,eGuivicescreensport};



const char* cSystemStrs[GUI_SYSTEM_INDEX_MAX] = 
{
	"蓝牙",
	"基础心率",
	"自动锁屏",
	"语言设置",
	"背光",
	"声音",
	"勿扰状态",
	"抬手亮屏",//7
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
	"中文",
	"英文",
};

const char* eLanguageStrs[GUI_LANGUAGE_INDEX_MAX] = 
{
	"CHINESE",
	"ENGLISH",
};

const char** LanguageStrs[] = {cLanguageStrs,eLanguageStrs};

//系统提示语
const char* cSysHintStrs[] = 
{
	"已连接",
	"设备未连接",
	"系统",
	"开启",
	"关闭",
	"时间段设置",
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
	" 7秒",
	"15秒",
	"30秒",
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
	"声音加震动",
	"声音",
	"震动",
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
	"计次",
	"计圈",
	"按OK键开始/暂停",
	"UP键计次,DOWN键计圈",
	"继续",
	"完成",
	"查看",
	"重置"
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
	"来电",
	"微信",
	"QQ",
	"短信",
	"企业微信",
	"钉钉"
	
};

const char* eNotifiAppStrs[] = 
{
	"来电",
	"微信",
	"QQ",
	"短信",
	"企业微信",
	"钉钉"
};

const char** NotifiAppStrs[] = {cNotifiAppStrs,eNotifiAppStrs};


const char* cWindScaleStrs[WIND_SCALE_INDEX_MAX] = 
{
	"无风",
	"软风",
	"轻风",
	"微风",
	"和风",
	"劲风",
	"强风",
	"疾风",
	"大风",
	"烈风",
	"狂风",
	"暴风",
	"飓风",
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
	"北风",
	"东北风",
	"东风",
	"东南风",
	"南风",
	"西南风",
	"西风",
	"西北风",
	"无持续风向",

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
	"优",
	"良",
	"轻污染",
	"中污染",
	"强污染",
	"重污染",
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
	"开启",
	"心率带",
	"步频传感器",
	"踏频传感器",
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
	"心率告警",							//0
	"恭喜您今日步数",
	"恭喜您今日热量",
	"目标已完成",
	"目标圈数",
	"目标时间",					//5
	"目标距离",
	"目标配速",
	"当前高度",
	"已完成!",
	"目标卡路里",						//10
	"目标速度",
	"耗时",
	"速度",
	
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
	"气压",
	"海拔",
	"过去12小时",
	"过去5小时",
	"海拔校准"
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
	"正在发出求救信号...",
	"退出求助"
};

const char* eSOSStrs[] = 
{
	"SOS",
	"Exit SOS",
};

const char** SOSStrs[] = {cSOSStrs,eSOSStrs};

const char* cNotifyWarningStrs[GUI_NOTIFY_WARNING_INDEX_MAX] = 
{
	"提示",
	"请先开启蓝牙",
	"系统>蓝牙",
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
	"进入寻找手机",
	"未连接手机蓝牙",
};

const char* eFindphoneStrs[] = 
{
	"Find Phone",
	"Ble Disconnect",
};
const char** FindphoneStrs[] = {cFindphoneStrs,eFindphoneStrs};

const char* cFindwatchStrs[] = 
{
	"寻找腕表中...",
	"按OK键确认",
};

const char* eFindwachStrs[] = 
{
	"Finding watch...",
	"Press OK to confirm",
};
const char** FindwatchStrs[] = {cFindwatchStrs,eFindwachStrs};
//心率区间
const char* cHeartZoneStrsStrs[] = 
{
	"极限",
	"无氧耐力",
	"有氧耐力",
	"燃脂",
	"热身",	
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

/*咕咚项目定义*/
const char*cheartzonegodstrs[] = 
{
   "热身放松",
   "有氧燃脂",
   "有氧耐力",
   "无氧耐力",
   "极限冲刺"


};
const char*eheartzonegodstrs[] = 
{
	"热身放松",
	"有氧燃脂",
	"有氧耐力",
	"无氧耐力",
	"极限冲刺"
};



const char** heartzonegodstrs[] = {cheartzonegodstrs,eheartzonegodstrs};


//运动准备
const char* cSportReadyStrs[] = 
{
	"数据显示",
	"开始",
	"运动设置",
	"平均",
	"实时",
	"最佳",
	"关闭",
	"开启",
	"推荐值",
	"圈",
	"需连接第三方配件"
	
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


//运动数据显示
const char* cSportDataDisStrs[] = 
{
	"距离",
	"时长",
	"平均配速",
	"实时配速",
	"总步数",
	"累积爬升",
	"心率",
	"平均速度",
	"实时速度",
    "海拔高度",
   	"趟数",
   	"上一趟SWOLF",
	"划水速率",
	"百米平均配速",//游泳的平均配速
	"垂直速度",
	"最佳配速",
	"平均心率",
	"实时步频",
	"平均步频",
	"卡路里",
	"上一圈配速",
	"上一圈时间",
	"最佳速度",
	"实时踏频",
	"平均踏频",
	"总划次",
	"平均划次",
	"平均swolf"
	
	
};

const char* eSportDataDisStrs[] = 
{
	"距离",
	"时长",
	"平均配速",
	"实时配速",
	"总步数",
	"累积爬升",
	"心率",
	"平均速度",
	"实时速度",
    "海拔高度",
   	"趟数",
   	"上一趟SWOLF",
	"划水速率",
	"百米平均配速",//游泳的平均配速
	"垂直速度",
	"最佳配速",
	"平均心率",
	"实时步频",
	"平均步频",
	"卡路里",
	"上一圈配速",
	"上一圈时间",
	"最佳速度",
	"实时踏频",
	"平均踏频",
	"总划次",
	"平均划次",
	"平均swolf"
	
	
};

const char** SportDataDisStrs[] = {cSportDataDisStrs,eSportDataDisStrs};


/*预览界面运动数据初始值*/
/*const char*csportsatapreview[] = 
{
  "00.00",
  "0:00:00",
  "0:00",
  "0:00",
   "0",//总步数
   "0",
   "0",//心率
   "0.0",
   "0.0",
   "0",
   "0",//趟数
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
   "0",//总步数
   "0 m",
   "0",//心率
   "0.0 km/h",
   "0.0 km/h",
   "0 m",
   "0",//趟数
   "0",
   "0 /min",
   "0 /100m",
   "00.0 km/h"




};


const char** sportsatapreview[] = {csportsatapreview,esportsatapreview};*/


//运动提醒设置
const char* cSportRemindStrs[] = 
{
	"心率告警",
	"心率恢复率",
	"配速提醒",
	"距离提醒",
	"自动计圈",
	"速度提醒",
    "高度提醒",//新增
	"泳池长度",
	"时间提醒",
	"计圈提醒"
	//"燃脂目标",
	
	//"室内跑步幅",
	
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
	"心率告警",
	"心率恢复率",
	"配速提醒",
	"距离提醒",
	"自动计圈",
	"速度提醒",
    "高度提醒",//新增
	"泳池长度",
	"时间提醒",
	"计圈提醒"
	//"燃脂目标",
	//"计圈提醒",
	//"室内跑步幅",
};

const char** SportRemindStrs[] = {cSportRemindStrs,eSportRemindStrs};



const char* cCountdownStrs[] = 
{
	"进入倒计时",
	"时间设置",
	"开始",
	"是否退出？",
	"否",
	"是",
	"倒计时结束"
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


/*待机界面名称/SOS相关搜救信息*/
const char* cGuiHomeScreenNameStrs[] = 
{
	"最大摄氧量",//0
	"运动记录",
	"通知",
	"卡路里",
	"乳酸阈",
	"训练计划",//5
	"恢复时间",
	"无",
	"移除",
	"添加",
	"位置已发出",//10
	"求助已发出",
	"退出求助",
	"不可移除项",
};

const char* eGuiHomeScreenNameStrs[] = 
{
	"VO2max",
	"运动记录",
	"通知",
	"卡路里",
	"乳酸阈",
	"训练计划",
	"恢复时间",
	"无",
	"移除",
	"添加",
	"位置已发出",//10
	"求助已发出",
	"退出求助",
	"不可移除项",
};

const char** GuiHomeScreenNameStrs[] = {cGuiHomeScreenNameStrs,eGuiHomeScreenNameStrs};

/*待机计步界面:校准补偿提示信息*/
const char* cGuiStepScreenStrs[] = 
{
	"步长校准",//0
	"在平直路径正常",
	"步行200米+",
	"OK键进入校准",
	"建议正常步行200米以上;",
	"校准时最好在平直路径进行;",//5
	"OK键进入校准",
	"OK键结束校准",
	"选择是否作为个人",
	"步长保存?",
};

const char* eGuiStepScreenStrs[] = 
{
	"步长校准",//0
	"在平直路径正常",
	"步行200米+",
	"OK键进入校准",
	"建议正常步行200米以上;",
	"校准时最好在平直路径进行;",//5
	"OK键进入校准",
	"OK键结束校准",
	"选择是否作为个人",
	"步长保存?",
};

const char** GuiStepScreenStrs[] = {cGuiStepScreenStrs,eGuiStepScreenStrs};

/*徒步越野、越野跑等赛事赛程字符*/
const char* cGuiSportEventStrs[] = 
{
	"继续完成比赛?",//0
	"继续",
	"重新开始运动?",
	"重新开始",
	"赛段结束",
	"本次赛事(第",//5
	"赛段)",
	"赛段累计",
	"总里程",
	"总时间",
	"总平均配速",//10
	"时间",
	"里程",
	"总里程",
	"总时间",
	"实时配速",//15
	"累计上升",
	"累计下降",
	"实时心率",
	"北京时间",
	"距离CP",//20
	"关门时间还剩",
	"偏离轨迹",
};

const char* eGuiSportEventStrs[] = 
{
	"继续完成比赛?",//0
	"继续",
	"重新开始运动?",
	"重新开始",
	"赛段结束",
	"本次赛事(第",//5
	"赛段)",
	"赛段累计",
	"总里程",
	"总时间",
	"总平均配速",//10
	"时间",
	"里程",
	"总里程",
	"总时间",
	"实时配速",//15
	"累计上升",
	"累计下降",
	"实时心率",
	"北京时间",
	"距离CP",//20
	"关门时间还剩",
	"偏离轨迹",
};

const char** GuiSportEventStrs[] = {cGuiSportEventStrs,eGuiSportEventStrs};

const char* cGuiLeapSecondStrs[] = 
{
	"北斗授时",//0
	"闰秒模式",
	"闰秒接收中",
	"请您耐心等待",
	"闰秒接收成功",
	"闰秒接收失败",//5
	"闰秒再次接受中",
	"距离下个窗口时间",
	"卫星授时",
};

const char* eGuiLeapSecondStrs[] = 
{
	"北斗授时",//0
	"闰秒模式",
	"闰秒接收中",
	"请您耐心等待",
	"闰秒接收成功",
	"闰秒接收失败",//5
	"闰秒再次接受中",
	"距离下个窗口时间",
	"卫星授时",
};


const char** GuiLeapSecondStrs[] = {cGuiLeapSecondStrs,eGuiLeapSecondStrs};
#ifdef COD 
const char* cTrainStrs[] = 
{
	"即将开始训练",//0
	"准备",
	"休息",
	"个",
	"分钟",
	"长按OK键暂停训练",//5
	"直播训练中",
	"训练中",
	"运动暂停",
	"下键结束",
	"查看数据",//10
	"训练完成",
	"未知运动",
	"讲解中",
	"上键继续",
};

const char* eTrainStrs[] = 
{
	"t",
};

const char** TrainStrs[] = {cTrainStrs,eTrainStrs};

const char* ctrainpauseStrs[] = 
{
	"继续",
	"结束" 
};

const char* etrainpauseStrs[] = 
{
	"继续",
	"结束" 

};

const char** trainpauseStrs[] = {ctrainpauseStrs,etrainpauseStrs};

#endif

const char* csportbarstrs[] = 
{
	"第一栏",
	"第二栏",
	"第三栏",
	"第四栏"

};

const char* esportbarstrs[] = 
{
	"第一栏",
	"第二栏",
	"第三栏",
	"第四栏"
};

const char** sportbarstrs[] = {csportbarstrs,esportbarstrs};





