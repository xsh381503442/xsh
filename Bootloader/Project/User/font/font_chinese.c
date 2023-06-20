#include "drv_lcd.h"
#include "font_chinese.h"

const uint16_t Font_CN_16_Index[] = {
//升   //级    //中    //请    //勿    //关    //机
0xC9FD,0xBCB6, 0xD6D0, 0xC7EB, 0xCEF0, 0xB9D8, 0xBBFA,	
	
//正   //在	
0xD5FD,0xD4DA,	
	
};

const uint8_t Font_CN_16_pxdata[] = {


	
0x01,0x20,0x07,0xA0,0x3C,0x20,0x04,0x20,0x04,0x20,0x04,0x20,0x04,0x20,0xFF,0xFE,
0x04,0x20,0x04,0x20,0x04,0x20,0x08,0x20,0x08,0x20,0x10,0x20,0x20,0x20,0x40,0x20,//升 //0
0x10,0x00,0x13,0xFC,0x20,0x84,0x20,0x88,0x48,0x88,0xF8,0x90,0x10,0x9C,0x20,0x84,
0x41,0x44,0xF9,0x44,0x41,0x28,0x01,0x28,0x1A,0x10,0xE2,0x28,0x44,0x44,0x01,0x82,//级 //1
0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x3F,0xF8,0x21,0x08,0x21,0x08,0x21,0x08,
0x21,0x08,0x21,0x08,0x3F,0xF8,0x21,0x08,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,//中 //2
0x00,0x40,0x40,0x40,0x27,0xFC,0x20,0x40,0x03,0xF8,0x00,0x40,0xE7,0xFE,0x20,0x00,
0x23,0xF8,0x22,0x08,0x23,0xF8,0x22,0x08,0x2B,0xF8,0x32,0x08,0x22,0x28,0x02,0x10,//请 //3
0x08,0x00,0x08,0x00,0x08,0x00,0x1F,0xFC,0x12,0x44,0x22,0x44,0x42,0x44,0x84,0x44,
0x04,0x44,0x08,0x84,0x10,0x84,0x21,0x04,0x42,0x04,0x04,0x04,0x08,0x28,0x10,0x10,//勿 //4
0x10,0x10,0x08,0x10,0x08,0x20,0x00,0x00,0x3F,0xF8,0x01,0x00,0x01,0x00,0x01,0x00,
0xFF,0xFE,0x01,0x00,0x02,0x80,0x02,0x80,0x04,0x40,0x08,0x20,0x30,0x18,0xC0,0x06,//关 //5
0x10,0x00,0x11,0xF0,0x11,0x10,0x11,0x10,0xFD,0x10,0x11,0x10,0x31,0x10,0x39,0x10,
0x55,0x10,0x55,0x10,0x91,0x10,0x11,0x12,0x11,0x12,0x12,0x12,0x12,0x0E,0x14,0x00,//机 //6
0x00,0x00,0x7F,0xFC,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x11,0x00,0x11,0xF8,
0x11,0x00,0x11,0x00,0x11,0x00,0x11,0x00,0x11,0x00,0x11,0x00,0xFF,0xFE,0x00,0x00,//正 //7
0x02,0x00,0x02,0x00,0x04,0x00,0xFF,0xFE,0x08,0x00,0x08,0x40,0x10,0x40,0x30,0x40,
0x57,0xFC,0x90,0x40,0x10,0x40,0x10,0x40,0x10,0x40,0x10,0x40,0x1F,0xFE,0x10,0x00,//在 //8
	
	
};

const LCD_ChineseStruct Font_CN_16 = {
16, 16, .num = 9, 
LCD_1BIT_BITMAP, 
Font_CN_16_Index, Font_CN_16_pxdata
};


