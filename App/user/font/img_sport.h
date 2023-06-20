#ifndef IMG_SPORT_H
#define	IMG_SPORT_H

#include "drv_lcd.h"


enum
{
Img_Running_38X38 = 0U,
Img_Marathon_38X38,
Img_CrossCountryRunning_38X38,
Img_IndoorRunning_38X38,
Img_Walking_38X38,
Img_Climbing_38X38,
Img_Cycling_38X38,
Img_Hiking_38X38,
Img_IndoorSwimming_38X38,
Img_Triathlon_38X38,


};

extern const LCD_ImgStruct Img_Sport[];
extern const LCD_ImgStruct img_custom_sports[];
extern const LCD_ImgStruct img_vice_sports[];//¸±ÆÁÄ£Ê½
extern const LCD_ImgStruct img_sports_record[];
extern const LCD_ImgStruct img_sportready_start;

//extern const LCD_ImgStruct Img_gpsgreen_16X22;
//extern const LCD_ImgStruct Img_gpsred_16X22;
extern const LCD_ImgStruct Img_selected_8X8;
extern const LCD_ImgStruct Img_unselected_8X8;
extern const LCD_ImgStruct Img_start_18X24;
extern const LCD_ImgStruct Img_setting_16X16;
extern const LCD_ImgStruct Img_Distance_16X16;
extern const LCD_ImgStruct Img_RiseHeight_22X22;
extern const LCD_ImgStruct Img_DropHeight_22X22;
extern const LCD_ImgStruct Img_Pentastar_24X24;
extern const LCD_ImgStruct Img_Dot_24X30;
//extern const LCD_ImgStruct Img_VerticalHeight_10X28;


//extern const LCD_ImgStruct Img_start_30X42;
extern const LCD_ImgStruct img_sport_pause;
extern const LCD_ImgStruct img_sport_start;
extern const LCD_ImgStruct img_sport_pause_small;
#ifdef COD 
extern const LCD_ImgStruct img_sport_pause_cod;
extern const LCD_ImgStruct img_compass_nwse[];

#endif
#if defined WATCH_SPORT_NEW_CLOUD_NAVIGATION

extern const LCD_ImgStruct Img_new_cloud_navigation_54X54;
#endif

extern const LCD_ImgStruct img_custom_sport_level_open;
extern const LCD_ImgStruct img_custom_sport_level_close;

extern const LCD_ImgStruct img_sport_bar[];

extern const LCD_ImgStruct img_sport_save_ok;


#endif
