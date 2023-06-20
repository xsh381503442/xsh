#ifndef DRV_LCD_H
#define	DRV_LCD_H

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

//LCD
#define		BSP_GPIO_LCD_VDD1					BSP_APOLLO2_GPIO_LCD_VDD1
#define		BSP_GPIO_LCD_VDD2					BSP_APOLLO2_GPIO_LCD_VDD2

#define		BSP_GPIO_LCD_XFRP					      BSP_APOLLO2_GPIO_LCD_XFRP
#define		BSP_GPIO_CFG_LCD_XFRP				    BSP_APOLLO2_GPIO_CFG_LCD_XFRP
#define		BSP_LCD_XFRF_CTIMER_SEGMENT			BSP_APOLLO2_LCD_XFRF_CTIMER_SEGMENT
#define		BSP_LCD_XFRP_CTIMER_NUM				  BSP_APOLLO2_LCD_XFRP_CTIMER_NUM
#define		BSP_GPIO_LCD_VCOM					      BSP_APOLLO2_GPIO_LCD_VCOM
#define		BSP_GPIO_CFG_LCD_VCOM				    BSP_APOLLO2_GPIO_CFG_LCD_VCOM
#define		BSP_LCD_VCOM_CTIMER_SEGMENT			BSP_APOLLO2_LCD_VCOM_CTIMER_SEGMENT
#define		BSP_LCD_VCOM_CTIMER_NUM				  BSP_APOLLO2_LCD_VCOM_CTIMER_NUM

#define		BSP_GPIO_LCD_HST					BSP_APOLLO2_GPIO_LCD_HST
#define		BSP_GPIO_LCD_VST					BSP_APOLLO2_GPIO_LCD_VST
#define		BSP_GPIO_LCD_HCK					BSP_APOLLO2_GPIO_LCD_HCK
#define		BSP_GPIO_LCD_VCK					BSP_APOLLO2_GPIO_LCD_VCK
#define		BSP_GPIO_LCD_ENB					BSP_APOLLO2_GPIO_LCD_ENB
#define		BSP_GPIO_LCD_R0						BSP_APOLLO2_GPIO_LCD_R0
#define		BSP_GPIO_LCD_G0						BSP_APOLLO2_GPIO_LCD_G0
#define		BSP_GPIO_LCD_B0						BSP_APOLLO2_GPIO_LCD_B0
#define		BSP_GPIO_LCD_R1						BSP_APOLLO2_GPIO_LCD_R1
#define		BSP_GPIO_LCD_G1						BSP_APOLLO2_GPIO_LCD_G1
#define		BSP_GPIO_LCD_B1						BSP_APOLLO2_GPIO_LCD_B1
#define		BSP_GPIO_LCD_XRST					BSP_APOLLO2_GPIO_LCD_XRST	


#define	LCD_VCOM_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_VCOM))
#define	LCD_VCOM_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_VCOM))

#define	LCD_VDD1_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_VDD1))
#define	LCD_VDD1_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_VDD1))

#define LCD_VDD2_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_VDD2))
#define	LCD_VDD2_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_VDD2))

#define LCD_HCK_HIGH()         	(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_HCK))	
#define LCD_HCK_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_HCK))

#define LCD_HST_HIGH()         	(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_HST))
#define LCD_HST_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_HST))

#define LCD_ENB_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_ENB))	
#define LCD_ENB_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_ENB))

#define LCD_VCK_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_VCK))	
#define LCD_VCK_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_VCK))

#define LCD_VST_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_VST))	
#define LCD_VST_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_VST))

#define LCD_XRST_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_XRST))	
#define LCD_XRST_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_XRST))

#define LCD_RED0_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_R0))	
#define LCD_GREEN0_HIGH()		(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_G0))
#define LCD_BLUE0_HIGH()		(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_B0))	
#define LCD_RED0_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_R0))
#define LCD_GREEN0_LOW()		(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_G0))	
#define LCD_BLUE0_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_B0))	

#define LCD_RED1_HIGH()			(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_R1))	
#define LCD_GREEN1_HIGH()		(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_G1))	
#define LCD_BLUE1_HIGH()		(am_hal_gpio_out_bit_set(BSP_GPIO_LCD_B1))	
#define LCD_RED1_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_R1))
#define LCD_GREEN1_LOW()		(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_G1))	
#define LCD_BLUE1_LOW()			(am_hal_gpio_out_bit_clear(BSP_GPIO_LCD_B1))

#define	LCD_LINE_CNT_MAX		240
#define	LCD_PIXEL_ONELINE_MAX	240

#define LCD_CENTER_LINE			120
#define	LCD_CENTER_ROW			120

/******--------第一方阵色区-------************/
#define LCD_RED          		0x44 	//RGB(192,0,0) 红色
#define LCD_CRIMSON      		0x45 	//RGB(192,0,64)猩红
#define LCD_DEEPPINK     		0x54 	//RGB(192,0,128)深粉色
#define LCD_HOTPINK      		0x55 	//RGB(192,0,192)热情的粉红
	
#define LCD_CHOCOLATE          	0x46 	//RGB(192,64,0)巧克力色
#define LCD_PALEVIOLETRED      	0x47 	//RGB(192,64,64)苍白的紫罗兰红
#define LCD_PLUM               	0x56 	//RGB(192,64,128)李子色
#define LCD_ORCHID             	0x57 	//RGB(192,64,192)兰花色
	
#define	LCD_PERU                0x64 	//RGB(192,128,0)秘鲁色
#define	LCD_SANDYBROWN          0x65 	//RGB(192,128,64)沙棕色
#define LCD_PINK                0x74 	//RGB(192,128,128)粉红
#define LCD_THISTLE             0x75 	//RGB(192,128,192)蓟
	
#define LCD_YELLOW              0x66 	//RGB(192,192,0)金黄色
#define	LCD_ORANGE              0x67 	//RGB(192,192,64)橙色
#define LCD_WHEAT               0x76 	//RGB(192,192,128)小麦色
#define	LCD_WHITE               0x77 	//RGB(192,192,192)白色
/******--------第二方阵色区-------************/
#define	LCD_BLACK               0x00 	//RGB(0,0,0)黑色
#define	LCD_DARKBLUE            0x01 	//RGB(0,0,64)深蓝色
#define LCD_MEDIUMBLUE          0x10 	//RGB(0,0,128)适中的蓝色
#define	LCD_BLUE                0x11 	//RGB(0,0,192)纯蓝

#define	LCD_LIMEGREEN           0x02 	//RGB(0,64,0)酸橙绿
#define	LCD_SEAGREEN            0x03 	//RGB(0,64,64)海洋绿
#define	LCD_NAVY                0x12 	//RGB(0,64,128)海军蓝
#define	LCD_CORNFLOWERBLUE      0x13 	//RGB(0,64,192)矢车菊蓝

#define LCD_FORESTGREEN         0x20 	//RGB(0,128,0)森林绿
#define LCD_DARKGREEN           0x21 	//RGB(0,128,64)深绿
#define LCD_SPRINGGREEN         0x30 	//RGB(0,128,128)春天绿
#define	LCD_DARKTURQUOISE       0x31 	//RGB(0,128,192)深绿宝石

#define LCD_GREEN               0x22 	//RGB(0,192,0)纯绿
#define LCD_AUQAMARIN           0x23 	//RGB(0,192,64)碧绿
#define	LCD_LIME                0x32 	//RGB(0,192,128)酸橙色
#define LCD_CYAN                0x33 	//RGB(0,192,192)青色
/******--------第三方阵色区-------************/
#define LCD_SADDLEBROWN        	0x04	//RGB(64,0,0)马鞍棕色
#define LCD_INDIGO             	0x05	//RGB(64,0,64)靛青
#define LCD_DARKORCHID         	0x14	//RGB(64,0,128)深蓝花紫
#define LCD_DODERBLUE          	0x15	//RGB(64,0,192)道奇蓝
		
#define LCD_DARKKHAKI          	0x06	//RGB(64,64,0)深卡其布
#define	LCD_GRAY               	0x07	//RGB(64,64,64)灰色
#define LCD_POWDERBLUE         	0x16	//RGB(64,64,128)火药蓝
#define LCD_LIGHTBLUE          	0x17	//RGB(64,64,192)淡蓝
		
#define LCD_CHARTREUSE         	0x24	//RGB(64,128,0)查特酒绿
#define LCD_LAWNGREEN          	0x25	//RGB(64,128,64)草坪绿
#define LCD_GREENBLUE          	0x34	//RGB(64,128,128)绿蓝色
#define LCD_TEAL               	0x35	//RGB(64,128,192)水鸭色
		
#define LCD_YELLOWGREEN        	0x26	//RGB(64,192,0)黄绿
#define LCD_MEDIUMAQUAMARINE   	0x27	//RGB(64,192,64)适中的碧绿色
#define LCD_LIGHTGREEN         	0x36	//RGB(64,192,128)浅绿
#define LCD_AQUA               	0x37	//RGB(64,192,192)水绿色
/******--------第四方阵色区-------************/
#define	LCD_MAROON             	0x40	//RGB(128,0,0)栗色
#define	LCD_MAGENTA            	0x41	//RGB(128,0,64)洋红
#define	LCD_FUCHSIA            	0x50	//RGB(128,0,128)紫红色
#define	LCD_VIOLET             	0x51	//RGB(128,0,192)紫罗兰
				
#define	LCD_BROWN              	0x42	//RGB(128,64,0)棕色
#define	LCD_TAN                	0x43	//RGB(128,64,64)晒黑
#define	LCD_BISQUE             	0x52	//RGB(128,64,128)番茄
#define	LCD_MEDIUMPURPLE       	0x53	//RGB(128,64,192)适中的紫色
				
#define	LCD_SIENNA             	0x60	//RGB(128,128,0)黄土赭色
#define	LCD_LIGHTSALMON        	0x61	//RGB(128,128,64)鲑鱼色
#define	LCD_LIGHTGRAY          	0x70	//RGB(128,128,128)浅灰色
#define	LCD_SLATEBLUE          	0x71	//RGB(128,128,192)板岩暗蓝灰
				
#define	LCD_OLIVERDRAB         	0x62	//RGB(128,192,0)橄榄土褐色
#define	LCD_PALEGREEN          	0x63	//RGB(128,192,64)苍白绿
#define	LCD_LIGHTAQUA          	0x72	//RGB(128,192,128)浅水绿色
#define	LCD_PALETURQUOISE      	0x73	//RGB(128,192,192)苍白的绿宝石

#define	LCD_NONE				0xFF	//不设置颜色，保持原色

#define LCD_FILL_ENABLE			1
#define LCD_FILL_DISABLE		0

#define	LCD_CENTER_JUSTIFIED	0xF1	//中心对齐
#define	LCD_LEFT_JUSTIFIED		0xF2	//左对齐
#define	LCD_RIGHT_JUSTIFIED		0xF3	//右对齐

//图片类型
typedef enum {
	LCD_1BIT_BITMAP				= 0U,	//1-bit bitmap
	LCD_2BIT_BITMAP				= 1U,	//2-bit bitmap
	LCD_6BIT_COLOR             	= 2U,	//6-bit color
	LCD_1BPP_GRSCL             	= 3U,	//1 bit per pixel
	LCD_2BPP_GRSCL             	= 4U,	//2 bits per pixel
	LCD_4BPP_GRSCL				= 5U,	//4 bits per pixel
	LCD_8BPP_GRSCL             	= 6U,	//8 bits per pixel
} LCD_IMGTYPE;

//字体大小
typedef enum {							//		中文		英文		数字
	LCD_FONT_8_SIZE				= 8U,	//8#	
	LCD_FONT_16_SIZE			= 16U,	//16#	16X16(宋体)	8X16(宋体)	6X17(自制)
	LCD_FONT_18_SIZE			= 18U,	//18#	12X18(自制)				6X17(自制)
	LCD_FONT_24_SIZE			= 24U,	//24#	24X24(宋体)	12X24(宋体)	10X25(自制)
	LCD_FONT_32_SIZE			= 32U,	//32#	32X32(宋体)	16X32(宋体)	14X38(自制)
	LCD_FONT_48_SIZE			= 48U,	//48#							17X45(自制)
	LCD_FONT_88_SIZE			= 88U,	//88#							29X90(自制)
	LCD_FONT_108_SIZE			= 108U,	//108#							30X108(自制)
} LCD_FONTSIZE;

/** 
  * @brief  Image Structure
  */
typedef struct {
	uint8_t width;       				//Image Width
	uint8_t height;      				//Image Height
	uint8_t stride;      				//Image Stride (typically same as width)
	LCD_IMGTYPE imgtype;  				//Image type (LCD_IMGTYPE)
	const uint8_t *pxdata;				//Pixel data
} LCD_ImgStruct;

/** 
  * @brief  font structure.
  */
typedef struct {
	uint8_t width;       				//Width of font characters
	uint8_t height;                     //Height of font characters
	uint8_t numchars;                   //Number of characters continguous in font
	uint8_t ascii_offset;               //ASCII offset of first font character
	LCD_IMGTYPE imgtype;  				//Image type (LCD_IMGTYPE)
	const uint8_t *pxdata;              //Font set pixel data array
} LCD_CharStruct;

/** 
  * @brief  chinese structure.
  */
typedef struct {
	uint8_t width;       				//Width of font characters
	uint8_t height;                     //Height of font characters
	uint16_t num;						//Chinese number
	LCD_IMGTYPE imgtype;  				//Image type (LCD_IMGTYPE)
	const uint16_t *pxindex;			//Chinese ASCII index
	const uint8_t *pxdata;              //Font set pixel data array
} LCD_ChineseStruct;

/** 
  * @brief  string display parameters structure
  */
typedef struct {
	uint8_t x_axis;						//Destination X coordinate
	uint8_t y_axis;						//Destination Y coordinate
	uint8_t forecolor;                  //Foreground Color of text
	uint8_t bckgrndcolor;				//Background Color of text
	LCD_FONTSIZE size;          		//Font Size
	int8_t kerning;						//Font Kerning
} SetWord_t;



static void LCD_PWM_Init(void);
static void LCD_InvertVck(void);
static void LCD_InvertHck(void);
static void LCD_InvertEnb(void);
static void LCD_SetRGB(uint8_t data);
static void LCD_SetDataInHorizontal(uint16_t line,uint8_t *pixelData);

static void LCD_SetChar(uint8_t fontCode, uint8_t destx, uint8_t desty, uint8_t forecolor, 
						uint8_t bckgrndcolor, LCD_CharStruct *charParams);
static void LCD_SetChinese(uint16_t fontCode, uint8_t destx, uint8_t desty, uint8_t forecolor, 
						   uint8_t bckgrndcolor, LCD_ChineseStruct *chineseParams);

extern void LCD_Poweron(void);
extern void LCD_Poweroff(void);
extern void LCD_DisplayOn(void);
extern void LCD_DisplayOff(void);
extern void LCD_Init(void);
extern void LCD_DisplaySomeLine(uint16_t starline,uint16_t len);

extern void get_circle_point(uint16_t centery, uint16_t centerx, uint16_t angle,
						uint16_t radius,uint16_t *p_x,uint16_t *p_y);

extern void LCD_SetBackgroundColor(uint8_t color);
extern void LCD_SetLine( uint16_t point1y, uint16_t point1x, uint16_t point2y, uint16_t point2x,
							uint8_t pencolor, uint8_t thickness );
extern void LCD_SetRectangle( uint8_t startline, uint8_t height, uint8_t startcolumn, uint8_t width,
								uint8_t pencolor, uint8_t hlinethick, uint8_t vlinethick, uint8_t fillenable);
extern void LCD_SetTriangle(uint8_t y1, uint8_t x1, uint8_t y2, uint8_t x2, uint8_t y3, uint8_t x3,
							uint8_t pencolor, uint8_t linethick, uint8_t fillenable);
extern void LCD_SetPolygon(uint8_t *p,	uint8_t pencolor, uint8_t linethick, uint8_t fillenable);
extern void LCD_SetCircle( uint16_t centery, uint16_t centerx, uint16_t radius,
							uint16_t pencolor, uint16_t thickness, uint8_t fillenable);
extern void LCD_SetArc(uint8_t centery, uint8_t centerx, uint8_t radius, uint8_t thickness, 
						uint16_t startangle, uint16_t endangle, uint8_t pencolor);
extern void LCD_SetPicture( uint8_t destx, uint8_t desty, uint8_t forecolor, 
							uint8_t bckgrndcolor, const LCD_ImgStruct *imgParams);
extern void LCD_SetString(char *code,SetWord_t *word);


#endif
