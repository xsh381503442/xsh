#ifndef __LIB_DISPLAY_H__
#define __LIB_DISPLAY_H__





#include <stdint.h>
#include <stdbool.h>


				
#define LIB_DISPLAY_COLOR_00            0x00
#define LIB_DISPLAY_COLOR_01            0x01
#define LIB_DISPLAY_COLOR_02            0x02
#define LIB_DISPLAY_COLOR_03            0x03
#define LIB_DISPLAY_COLOR_04            0x04
#define LIB_DISPLAY_COLOR_05            0x05
#define LIB_DISPLAY_COLOR_06            0x06
#define LIB_DISPLAY_COLOR_07            0x07	
#define LIB_DISPLAY_COLOR_10            0x10
#define LIB_DISPLAY_COLOR_11            0x11
#define LIB_DISPLAY_COLOR_12            0x12
#define LIB_DISPLAY_COLOR_13            0x13
#define LIB_DISPLAY_COLOR_14            0x14
#define LIB_DISPLAY_COLOR_15            0x15
#define LIB_DISPLAY_COLOR_16            0x16
#define LIB_DISPLAY_COLOR_17            0x17		
#define LIB_DISPLAY_COLOR_20            0x20
#define LIB_DISPLAY_COLOR_21            0x21
#define LIB_DISPLAY_COLOR_22            0x22
#define LIB_DISPLAY_COLOR_23            0x23
#define LIB_DISPLAY_COLOR_24            0x24
#define LIB_DISPLAY_COLOR_25            0x25
#define LIB_DISPLAY_COLOR_26            0x26
#define LIB_DISPLAY_COLOR_27            0x27	
#define LIB_DISPLAY_COLOR_30            0x30
#define LIB_DISPLAY_COLOR_31            0x31
#define LIB_DISPLAY_COLOR_32            0x32
#define LIB_DISPLAY_COLOR_33            0x33
#define LIB_DISPLAY_COLOR_34            0x34
#define LIB_DISPLAY_COLOR_35            0x35
#define LIB_DISPLAY_COLOR_36            0x36
#define LIB_DISPLAY_COLOR_37            0x37	
#define LIB_DISPLAY_COLOR_40            0x40
#define LIB_DISPLAY_COLOR_41            0x41
#define LIB_DISPLAY_COLOR_42            0x42
#define LIB_DISPLAY_COLOR_43            0x43
#define LIB_DISPLAY_COLOR_44            0x44
#define LIB_DISPLAY_COLOR_45            0x45
#define LIB_DISPLAY_COLOR_46            0x46
#define LIB_DISPLAY_COLOR_47            0x47
#define LIB_DISPLAY_COLOR_50            0x50
#define LIB_DISPLAY_COLOR_51            0x51
#define LIB_DISPLAY_COLOR_52            0x52
#define LIB_DISPLAY_COLOR_53            0x53
#define LIB_DISPLAY_COLOR_54            0x54
#define LIB_DISPLAY_COLOR_55            0x55
#define LIB_DISPLAY_COLOR_56            0x56
#define LIB_DISPLAY_COLOR_57            0x57		
#define LIB_DISPLAY_COLOR_60            0x60
#define LIB_DISPLAY_COLOR_61            0x61
#define LIB_DISPLAY_COLOR_62            0x62
#define LIB_DISPLAY_COLOR_63            0x63
#define LIB_DISPLAY_COLOR_64            0x64
#define LIB_DISPLAY_COLOR_65            0x65
#define LIB_DISPLAY_COLOR_66            0x66
#define LIB_DISPLAY_COLOR_67            0x67	
#define LIB_DISPLAY_COLOR_70            0x70
#define LIB_DISPLAY_COLOR_71            0x71
#define LIB_DISPLAY_COLOR_72            0x72
#define LIB_DISPLAY_COLOR_73            0x73
#define LIB_DISPLAY_COLOR_74            0x74
#define LIB_DISPLAY_COLOR_75            0x75
#define LIB_DISPLAY_COLOR_76            0x76
#define LIB_DISPLAY_COLOR_77            0x77	


				



typedef struct {

	uint16_t x;
	uint16_t y;
	uint16_t height;
	uint8_t gap;
	uint8_t bg_color;
	uint8_t fg_color;
}_lib_display_param;


void lib_display_ascii(char *p_char, uint8_t char_len, _lib_display_param *param);







void lib_display_example1(uint16_t x,uint16_t y);
void lib_display_example2(void);
void lib_display_example3(void);
void lib_display_example4(uint8_t color);
void lib_display_example5_fill_pic(void);
void lib_display_example6_num(void);


void lib_display_point_example(void);
void lib_display_line_example(void);
void lib_display_rect_example(bool fill);
void lib_display_circle_example(bool fill);
void lib_display_watch_face_example(void);
void lib_display_watch_hand_example(uint8_t second, uint8_t minute, uint8_t hour, uint8_t radius);









#endif //__LIB_DISPLAY_H__
