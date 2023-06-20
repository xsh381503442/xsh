#ifndef IMG_HOME_H
#define	IMG_HOME_H


#include <stdint.h>
#include "drv_lcd.h"

extern const LCD_ImgStruct img_monitor_step;

extern const LCD_ImgStruct img_step_cal_ysave;
extern const LCD_ImgStruct img_step_cal_nsave;
extern const LCD_ImgStruct step_cal_arrow_youshang;


extern const LCD_ImgStruct Home_img_unlock;
extern const LCD_ImgStruct Home_img_lock ;


extern const LCD_ImgStruct Img_heartrate_18X16;
extern const LCD_ImgStruct Img_heartrate_32X28;
//extern const LCD_ImgStruct Img_heartrate_56X48;
//extern const LCD_ImgStruct Img_save_64X64;
//extern const LCD_ImgStruct Img_delete_64X64;
//extern const LCD_ImgStruct Img_Lock_14X18;
//extern const LCD_ImgStruct Img_Lock_42X52;
//extern const LCD_ImgStruct Img_Unlock_42X52;
//extern const LCD_ImgStruct Img_calory_36X48;
extern const LCD_ImgStruct img_monitor_calory;
#if !defined COD
extern const LCD_ImgStruct Img_App_MSG[];
#endif
extern const LCD_ImgStruct img_notifi_msg[];
//extern const LCD_ImgStruct img_notifi_msg_30x30[];

//extern const LCD_ImgStruct Img_OK_40X40;
//extern const LCD_ImgStruct Img_Cancel_46X50;

extern const LCD_ImgStruct Img_Step_12X16;
extern const LCD_ImgStruct Img_StepTarget_12X16;
//extern const LCD_ImgStruct Font_Battery_10X10;
#if defined WATCH_HAS_SOS
extern const LCD_ImgStruct Font_SOS_160X48;
extern const LCD_ImgStruct Font_SOS_164X52;
#endif
//extern const LCD_ImgStruct Img_Step_20X24;
extern const LCD_ImgStruct Img_Calorie_20X24;
extern const LCD_ImgStruct Img_Distance_28X20;

#endif
