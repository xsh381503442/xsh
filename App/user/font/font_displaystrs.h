#ifndef FONT_DISPLAYSTRS_H
#define	FONT_DISPLAYSTRS_H

#include "watch_config.h"

extern const char** MonthStrs[];
extern const char** LunarStrs[];
extern const char** ToolStrs[];
extern const char** SportStrs[];
extern const char** WeekStrs[];
extern const char** CompassCalibrationStrs[];
extern const char** CompassStrs[];
extern const char** StopwatchStrs[];
extern const char** NotifiAppStrs[];

extern const char** SportsPromptStrs[];
extern const char** RunPauseStrs[];
extern const char** ClimbPauseStrs[];
extern const char** SwimPauseStrs[];
extern const char** IndoorrunPauseStrs[];
extern const char** CrossPauseStrs[];
extern const char** DotStrs[];
extern const char** CancelStrs[];
extern const char** SportFeelStrs[];
extern const char** SportDetailStrs[];
extern const char** AmbientStrs[];
extern const char** SOSStrs[];
extern const char** FindphoneStrs[];
extern const char** FindwatchStrs[];
extern const char** HeartZoneStrs[];

extern const char** heartzonegodstrs[];

//extern const char** sportsatapreview[];

extern const char** ThemeNameStrs[];

extern const char** sportbarstrs[];


extern const char** SportReadyStrs[];
extern const char** SportDataDisStrs[];
extern const char** sportdetailstrsgod[];


extern const char** SportRemindStrs[];
extern const char** CountdownStrs[];


#define GUI_MENU_INDEX_MAX        7 


extern const char* cMenuStrs[GUI_MENU_INDEX_MAX];
extern const char* eMenuStrs[GUI_MENU_INDEX_MAX];
extern const char** MenuStrs[];


#define GUI_THEME_INDEX_MAX        7

extern const char* cThemeStrs[GUI_THEME_INDEX_MAX];
extern const char* eThemeStrs[GUI_THEME_INDEX_MAX];
extern const char** ThemeStrs[];


#if defined WATCH_TIMEZONE_SET
#define GUI_TIME_INDEX_MAX		  3
#else
#define GUI_TIME_INDEX_MAX        2
#endif


extern const char* cTimeStrs[GUI_TIME_INDEX_MAX];
extern const char* eTimeStrs[GUI_TIME_INDEX_MAX];
extern const char** TimeStrs[];


#if defined WATCH_TIMEZONE_SET
#define GUI_TIME_ZONE_COUNTIES_INDEX_MAX 32
extern const char* cTimeZoneCountiesStrs[GUI_TIME_ZONE_COUNTIES_INDEX_MAX];
extern const char* eTimeZoneCountiesStrs[GUI_TIME_ZONE_COUNTIES_INDEX_MAX];
extern const char** TimeZoneCountiesStrs[];

extern const char* cTimeZoneConfirmStrs[];
extern const char* eTimeZoneConfirmStrs[];
extern const char** TimeZoneConfirmStrs[];

#endif

#define GUI_ALARM_SET_INDEX_MAX        4
extern const char* cAlarmSetStrs[GUI_ALARM_SET_INDEX_MAX];
extern const char* eAlarmSetStrs[GUI_ALARM_SET_INDEX_MAX];
extern const char** AlarmSetStrs[];

#define GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX        3
extern const char* cAlarmSetSoundNViberateStrs[GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX];
extern const char* eAlarmSetSoundNViberateStrs[GUI_ALARM_SET_SOUND_N_VIBERATE_INDEX_MAX];
extern const char** AlarmSetSoundNViberateStrs[];

#define GUI_ALARM_SET_REPITION_INDEX_MAX        4
extern const char* cAlarmSetRepitionStrs[GUI_ALARM_SET_REPITION_INDEX_MAX];
extern const char* eAlarmSetRepitionStrs[GUI_ALARM_SET_REPITION_INDEX_MAX];
extern const char** AlarmSetRepitionStrs[];



#define GUI_CUSTOMIZED_MONITOR_INDEX_MAX       6
extern const char* cCustomizedMonitorStrs[GUI_CUSTOMIZED_MONITOR_INDEX_MAX];
extern const char* eCustomizedMonitorStrs[GUI_CUSTOMIZED_MONITOR_INDEX_MAX];
extern const char** CustomizedMonitorStrs[];

#define GUI_CUSTOMIZED_TOOLS_INDEX_MAX        8
extern const char* cCustomizedToolsStrs[GUI_CUSTOMIZED_TOOLS_INDEX_MAX];
extern const char* eCustomizedToolsStrs[GUI_CUSTOMIZED_TOOLS_INDEX_MAX];
extern const char** CustomizedToolsStrs[];

#define GUI_CUSTOMIZED_SPORTS_INDEX_MAX        6
extern const char* cCustomizedSportsStrs[GUI_CUSTOMIZED_SPORTS_INDEX_MAX];
extern const char* eCustomizedSportsStrs[GUI_CUSTOMIZED_SPORTS_INDEX_MAX];
extern const char** CustomizedSportsStrs[];




#define GUI_Home_INDEX_MAX     97
extern const char** GuiHomeLockStrs[];
#define GUI_VICE_SPORTS_INDEX_MAX        4

extern const char** Guivicescreensport[];



#define GUI_SYSTEM_INDEX_MAX        8
extern const char* cSystemStrs[GUI_SYSTEM_INDEX_MAX];
extern const char* eSystemStrs[GUI_SYSTEM_INDEX_MAX];
extern const char** SystemStrs[];




#define GUI_LANGUAGE_INDEX_MAX        2
extern const char* cLanguageStrs[GUI_LANGUAGE_INDEX_MAX];
extern const char* eLanguageStrs[GUI_LANGUAGE_INDEX_MAX];
extern const char** LanguageStrs[];

extern const char** SysHintStrs[];

#define GUI_BACKLIGHT_TIME_INDEX_MAX        3
extern const char* cBacklightTimeStrs[GUI_BACKLIGHT_TIME_INDEX_MAX];
extern const char* eBacklightTimeStrs[GUI_BACKLIGHT_TIME_INDEX_MAX];
extern const char** BacklightTimeStrs[];

#define GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX        3
extern const char* cSystemSoundNViberateStrs[GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX];
extern const char* eSystemSoundNViberateStrs[GUI_SYSTEM_SOUND_N_VIBERATE_INDEX_MAX];
extern const char** SystemSoundNViberateStrs[];

#define GUI_WEEK_INDEX_MAX        7
extern const char** WeekStrs[];
extern const char** WeekofDayStrs[];


#define WIND_SCALE_INDEX_MAX       13
extern const char* cWindScaleStrs[WIND_SCALE_INDEX_MAX];
extern const char* eWindScaleStrs[WIND_SCALE_INDEX_MAX];
extern const char** WindScaleStrs[];

#define WIND_DIRECT_INDEX_MAX     9
extern const char* cWindDirectStrs[WIND_DIRECT_INDEX_MAX];
extern const char* eWindDirectStrs[WIND_DIRECT_INDEX_MAX];
extern const char** WindDirectStrs[];

#define AIR_QUALITY_INDEX_MAX         6
extern const char* cAirQualityStrs[AIR_QUALITY_INDEX_MAX];
extern const char* eAirQualityStrs[AIR_QUALITY_INDEX_MAX];
extern const char** AirQualityStrs[];



#define GUI_ACCESSORY_INDEX_MAX        4
extern const char* cAccessoryStrs[GUI_ACCESSORY_INDEX_MAX];
extern const char* eAccessoryStrs[GUI_ACCESSORY_INDEX_MAX];
extern const char** AccessoryStrs[];

#define GUI_NOTIFY_MENU_INDEX_MAX        14

extern const char* cNotifyMenuStrs[GUI_NOTIFY_MENU_INDEX_MAX];
extern const char* eNotifyMenuStrs[GUI_NOTIFY_MENU_INDEX_MAX];
extern const char** NotifyMenuStrs[];

#define GUI_NOTIFY_WARNING_INDEX_MAX        3

extern const char* cNotifyWarningStrs[GUI_NOTIFY_WARNING_INDEX_MAX];
extern const char* eNotifyWarningStrs[GUI_NOTIFY_WARNING_INDEX_MAX];
extern const char** NotifyWarningStrs[];


extern const char** GuiHomeScreenNameStrs[];
extern const char** GuiStepScreenStrs[];
extern const char** GuiSportEventStrs[];
extern const char** SportEventPauseStrs[];
extern const char** GuiLeapSecondStrs[];
#ifdef COD 
extern const char** TrainStrs[];
extern const char** trainpauseStrs[];

#endif
#endif



