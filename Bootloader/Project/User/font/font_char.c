#include "drv_lcd.h"
#include "font_char.h"

const uint8_t Font_Char_16_pxdata[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// //0
0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x18,0x18,0x00,//!//1
0x00,0x6c,0x6c,0x6c,0x48,0x48,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"//2
0x00,0x24,0x24,0x24,0x24,0xfe,0x24,0x24,0x48,0x48,0xfe,0x48,0x48,0x48,0x48,0x00,//#//3
0x00,0x10,0x3c,0x52,0x90,0x90,0x50,0x38,0x14,0x12,0x12,0x94,0x78,0x10,0x10,0x00,//$//4
0x00,0x62,0x92,0x94,0x94,0x98,0x68,0x10,0x10,0x2c,0x32,0x52,0x52,0x92,0x8c,0x00,//%//5
0x00,0x30,0x48,0x48,0x48,0x50,0x20,0x20,0x50,0x52,0x92,0x8a,0x84,0x4c,0x32,0x00,//&//6
0x00,0x10,0x38,0x38,0x38,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//'//7
0x00,0x04,0x08,0x10,0x10,0x20,0x20,0x20,0x20,0x20,0x20,0x10,0x10,0x08,0x04,0x00,//(//8
0x00,0x40,0x20,0x10,0x10,0x08,0x08,0x08,0x08,0x08,0x08,0x10,0x10,0x20,0x40,0x00,//)//9
0x00,0x00,0x00,0x00,0x00,0x10,0x92,0x54,0x38,0x38,0x54,0x92,0x10,0x00,0x00,0x00,//*//10
0x00,0x00,0x00,0x10,0x10,0x10,0x10,0xfe,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x00,//+//11
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x10,0x20,0x00,//,//12
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//-//13
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,//.//14
0x00,0x02,0x02,0x04,0x04,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x40,0x80,0x80,0x00,/////15
0x00,0x00,0x18,0x24,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18,0x00,0x00,//0//16
0x00,0x00,0x08,0x18,0x28,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x00,0x00,//1//17
0x00,0x00,0x38,0x44,0x82,0x02,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0xfe,0x00,0x00,//2//18
0x00,0x00,0x78,0x84,0x02,0x02,0x04,0x18,0x04,0x02,0x02,0x02,0x84,0x78,0x00,0x00,//3//19
0x00,0x00,0x08,0x10,0x10,0x20,0x28,0x48,0x48,0x88,0xfe,0x08,0x08,0x08,0x00,0x00,//4//20
0x00,0x00,0x7e,0x40,0x40,0x40,0x78,0x44,0x02,0x02,0x02,0x02,0x44,0x38,0x00,0x00,//5//21
0x00,0x00,0x08,0x10,0x20,0x40,0x40,0xb8,0xc4,0x82,0x82,0x82,0x44,0x38,0x00,0x00,//6//22
0x00,0x00,0xfe,0x02,0x04,0x04,0x08,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x00,0x00,//7//23
0x00,0x00,0x38,0x44,0x82,0x82,0x44,0x38,0x44,0x82,0x82,0x82,0x44,0x38,0x00,0x00,//8//24
0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x46,0x3a,0x04,0x04,0x08,0x10,0x20,0x00,0x00,//9//25
0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,//://26
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x10,0x20,//;//27
0x00,0x00,0x00,0x00,0x02,0x0c,0x30,0xc0,0x30,0x0c,0x02,0x00,0x00,0x00,0x00,0x00,//<//28
0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,0x00,0x00,0x00,0x00,//=//29
0x00,0x00,0x00,0x00,0x80,0x60,0x18,0x06,0x18,0x60,0x80,0x00,0x00,0x00,0x00,0x00,//>//30
0x00,0x38,0x44,0x82,0x82,0x02,0x04,0x18,0x30,0x30,0x30,0x00,0x00,0x30,0x30,0x00,//?//31
0x00,0x00,0x3c,0x42,0x99,0xa5,0xa5,0xa5,0xa5,0xa5,0xa7,0x98,0x42,0x3c,0x00,0x00,//@//32
0x00,0x00,0x00,0x10,0x28,0x28,0x28,0x44,0x44,0x7c,0x82,0x82,0x82,0x82,0x00,0x00,//A//33
0x00,0x00,0x00,0x7c,0x42,0x42,0x42,0x44,0x78,0x44,0x42,0x42,0x42,0x7c,0x00,0x00,//B//34
0x00,0x00,0x00,0x38,0x44,0x82,0x80,0x80,0x80,0x80,0x80,0x82,0x44,0x38,0x00,0x00,//C//35
0x00,0x00,0x00,0x78,0x44,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x44,0x78,0x00,0x00,//D//36
0x00,0x00,0x00,0x7e,0x40,0x40,0x40,0x40,0x7c,0x40,0x40,0x40,0x40,0x7e,0x00,0x00,//E//37
0x00,0x00,0x00,0x7e,0x40,0x40,0x40,0x7c,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x00,//F//38
0x00,0x00,0x00,0x38,0x44,0x82,0x80,0x80,0x8e,0x82,0x82,0x82,0x46,0x3a,0x00,0x00,//G//39
0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x7e,0x42,0x42,0x42,0x42,0x42,0x00,0x00,//H//40
0x00,0x00,0x00,0x7c,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7c,0x00,0x00,//I//41
0x00,0x00,0x00,0x3e,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x88,0x88,0x70,0x00,0x00,//J//42
0x00,0x00,0x00,0x44,0x48,0x48,0x50,0x50,0x60,0x50,0x50,0x48,0x48,0x44,0x00,0x00,//K//43
0x00,0x00,0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x42,0x42,0xfe,0x00,0x00,//L//44
0x00,0x00,0x00,0x82,0xc6,0xc6,0xaa,0xaa,0xaa,0x92,0x92,0x82,0x82,0x82,0x00,0x00,//M//45
0x00,0x00,0x00,0x42,0x62,0x62,0x52,0x52,0x4a,0x4a,0x4a,0x46,0x46,0x42,0x00,0x00,//N//46
0x00,0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x44,0x38,0x00,0x00,//O//47
0x00,0x00,0x00,0x78,0x44,0x42,0x42,0x44,0x78,0x40,0x40,0x40,0x40,0x40,0x00,0x00,//P//48
0x00,0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0x92,0x8a,0x44,0x3a,0x00,0x00,//Q//49
0x00,0x00,0x00,0x78,0x44,0x42,0x42,0x44,0x78,0x50,0x48,0x48,0x44,0x42,0x00,0x00,//R//50
0x00,0x00,0x00,0x38,0x44,0x82,0x80,0x40,0x38,0x04,0x02,0x82,0x44,0x38,0x00,0x00,//S//51
0x00,0x00,0x00,0xfe,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,//T//52
0x00,0x00,0x00,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x44,0x38,0x00,0x00,//U//53
0x00,0x00,0x00,0x82,0x82,0x82,0x44,0x44,0x44,0x28,0x28,0x28,0x10,0x10,0x00,0x00,//V//54
0x00,0x00,0x00,0x82,0x92,0x92,0x92,0x92,0xaa,0xaa,0xaa,0xc6,0xc6,0x82,0x00,0x00,//W//55
0x00,0x00,0x00,0x82,0x44,0x44,0x28,0x28,0x10,0x28,0x28,0x44,0x44,0x82,0x00,0x00,//X//56
0x00,0x00,0x00,0x82,0x82,0x44,0x44,0x28,0x28,0x10,0x10,0x10,0x10,0x10,0x00,0x00,//Y//57
0x00,0x00,0x00,0xfe,0x02,0x04,0x04,0x08,0x10,0x10,0x20,0x20,0x40,0xfe,0x00,0x00,//Z//58
0x00,0x1c,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1c,0x00,//[//59
0x00,0x80,0x80,0x40,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x04,0x04,0x02,0x02,0x00,//\//60
0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x70,0x00,//]//61
0x00,0x10,0x28,0x44,0x82,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//^//62
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,//_//63
0x00,0x10,0x10,0x10,0x08,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//`//64
0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0x82,0x02,0x7a,0x86,0x82,0x86,0x7a,0x00,0x00,//a//65
0x00,0x00,0x40,0x40,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x42,0x64,0x58,0x00,0x00,//b//66
0x00,0x00,0x00,0x00,0x00,0x00,0x3c,0x42,0x80,0x80,0x80,0x80,0x42,0x3c,0x00,0x00,//c//67
0x00,0x00,0x04,0x04,0x04,0x04,0x34,0x4c,0x84,0x84,0x84,0x84,0x4c,0x34,0x00,0x00,//d//68
0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x44,0x82,0xfe,0x80,0x80,0x42,0x3c,0x00,0x00,//e//69
0x00,0x00,0x0c,0x10,0x10,0x10,0x7c,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,//f//70
0x00,0x00,0x00,0x00,0x00,0x00,0x34,0x4c,0x44,0x44,0x44,0x4c,0x34,0x04,0x44,0x38,//g//71
0x00,0x00,0x40,0x40,0x40,0x40,0x5c,0x62,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,//h//72
0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,//i//73
0x00,0x00,0x00,0x08,0x08,0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x48,0x30,//j//74
0x00,0x00,0x00,0x40,0x40,0x40,0x40,0x44,0x48,0x50,0x60,0x50,0x48,0x44,0x00,0x00,//k//75
0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x24,0x18,0x00,0x00,//l//76
0x00,0x00,0x00,0x00,0x00,0x00,0xac,0xd2,0x92,0x92,0x92,0x92,0x92,0x92,0x00,0x00,//m//77
0x00,0x00,0x00,0x00,0x00,0x00,0x5c,0x62,0x42,0x42,0x42,0x42,0x42,0x42,0x00,0x00,//n//78
0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x44,0x38,0x00,0x00,//o//79
0x00,0x00,0x00,0x00,0x00,0x00,0x58,0x64,0x42,0x42,0x42,0x42,0x64,0x58,0x40,0x40,//p//80
0x00,0x00,0x00,0x00,0x00,0x00,0x34,0x4c,0x84,0x84,0x84,0x84,0x4c,0x34,0x04,0x04,//q//81
0x00,0x00,0x00,0x00,0x00,0x00,0x4c,0x32,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,//r//82
0x00,0x00,0x00,0x00,0x00,0x00,0x3c,0x42,0x40,0x30,0x0c,0x02,0x42,0x3c,0x00,0x00,//s//83
0x00,0x00,0x00,0x10,0x10,0x10,0x7c,0x10,0x10,0x10,0x10,0x10,0x12,0x0c,0x00,0x00,//t//84
0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x46,0x3a,0x00,0x00,//u//85
0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x82,0x44,0x44,0x28,0x28,0x10,0x10,0x00,0x00,//v//86
0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x92,0x92,0x92,0xaa,0xaa,0x44,0x44,0x00,0x00,//w//87
0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,0x00,0x00,//x//88
0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x82,0x44,0x44,0x28,0x28,0x10,0x20,0x40,0x80,//y//89
0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x02,0x04,0x08,0x10,0x20,0x40,0xfe,0x00,0x00,//z//90
0x04,0x08,0x08,0x08,0x08,0x08,0x10,0x20,0x10,0x08,0x08,0x08,0x08,0x08,0x04,0x00,//{//91
0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,//|//92
0x40,0x20,0x20,0x20,0x20,0x20,0x10,0x08,0x10,0x20,0x20,0x20,0x20,0x20,0x40,0x00,//}//93
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x61,0x99,0x86,0x00,0x00,0x00,0x00,0x00,0x00,//~//94
};

const LCD_CharStruct Font_Char_16 = {
	8, 16, 95, 0x20,
	LCD_1BIT_BITMAP, 
	Font_Char_16_pxdata
};

