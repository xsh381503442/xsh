#ifndef _IMG_NOTIFY_H_
#define	_IMG_NOTIFY_H_


#include <stdint.h>
#include "drv_lcd.h"

//enum
//{
//	
//Img_Low_Battery_32X15 = 0U,
//Img_Goal_Calory_58X78,
//Img_Goal_Step_58X78,
//	
//};

extern const LCD_ImgStruct Img_Low_Battery_32X15;
extern const LCD_ImgStruct Img_Goal_Calory_58X78;
extern const LCD_ImgStruct Img_Goal_Step_58X78;
extern const LCD_ImgStruct Img_Charging_Battery_32X20;
extern const LCD_ImgStruct bluetooth_connect_30X60;
extern const LCD_ImgStruct bluetooth_disconnect_30X60;
#ifdef COD 
extern const LCD_ImgStruct img_train;

#endif
#endif

