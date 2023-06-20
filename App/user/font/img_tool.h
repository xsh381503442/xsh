#ifndef IMG_TOOL_H
#define	IMG_TOOL_H

#include "drv_lcd.h"
#include "watch_config.h"

enum
{
Img_Location_30X28 = 0U,
Img_Compass_28X28,
Img_Calendar_28X28,
Img_Pressure_21X28,
Img_Stopwatch_25X28,
Img_FindMyPhone_30X28
};



extern const LCD_ImgStruct img_custom_monitor[];
extern const LCD_ImgStruct img_custom_tools[];
extern const LCD_ImgStruct img_custom_tool_hint;
extern const LCD_ImgStruct img_gps_god;

extern const LCD_ImgStruct img_monitor_calendar;


//extern const LCD_ImgStruct Img_tools[];
extern const LCD_ImgStruct img_tools_lists[];
extern const LCD_ImgStruct img_tools_big_lists[];
extern const LCD_ImgStruct img_tool_gps_13x21;

extern const LCD_ImgStruct Img_Pointing_LeftDown_10X10;
extern const LCD_ImgStruct Img_Pointing_RightDown_10X10;
extern const LCD_ImgStruct Img_Pointing_RightUp_10X10;
extern const LCD_ImgStruct Img_Pointing_Up_12X8;
extern const LCD_ImgStruct Img_Pointing_Down_12X8;
extern const LCD_ImgStruct Img_Pointing_Right_12X8;
extern const LCD_ImgStruct Img_Pointing_Left_12X8;
extern const LCD_ImgStruct img_step_arrow_left;
extern const LCD_ImgStruct img_point_middle;
extern const LCD_ImgStruct img_point_leftdown;

//extern const LCD_ImgStruct Img_Altitude_14X19;
//extern const LCD_ImgStruct Img_Pressure_14X19;
//extern const LCD_ImgStruct Img_Temperature_14X19;
extern const LCD_ImgStruct Img_C_22X18;
extern const LCD_ImgStruct Img_hPa_36X18;
extern const LCD_ImgStruct Img_m_18X18;



extern const LCD_ImgStruct img_tool_gps_flag;
extern const LCD_ImgStruct img_tool_bds_flag;

extern const LCD_ImgStruct Img_Degree_6X6;


extern const LCD_ImgStruct img_tool_compass_cal_point;

extern const LCD_ImgStruct img_tool_compass_point;



extern const LCD_ImgStruct img_tool_compass_circle;


extern const LCD_ImgStruct img_tool_compass_cal_track;

extern const LCD_ImgStruct Img_compass_4X4;
extern const LCD_ImgStruct img_tool_compass_flag_degree;

//extern const LCD_ImgStruct Img_Round_54X54;	
//extern const LCD_ImgStruct Img_Round_64X64;

//extern const LCD_ImgStruct Img_findphone_92x84;
extern const LCD_ImgStruct Img_M_30X28;
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION

extern const LCD_ImgStruct Img_Countdown_36X36;
extern const LCD_ImgStruct Img_Countdown_28X28;
#endif
extern const LCD_ImgStruct img_calendar_year;
extern const LCD_ImgStruct img_calendar_month;

extern const LCD_ImgStruct img_tool_vo2max_ahead_quit_warn;


extern const LCD_ImgStruct img_tool_ambient_height;
extern const LCD_ImgStruct img_tool_ambient_pa;
extern const LCD_ImgStruct img_tool_ambient_temp;

//extern const LCD_ImgStruct img_tool_circle;


	
#endif
