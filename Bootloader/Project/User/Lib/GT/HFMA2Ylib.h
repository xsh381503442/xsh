//#include <A2Ylib.h>
//┌────────────────────┐
//│        高通科技版权所有  │
//│      GENITOP RESEARCH CO.,LTD.         │
//│        created on 2015.7.27            │
//└────────────────────┘

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "drv_font.h"

typedef  unsigned char  u8;
//typedef  unsigned long  u32;
typedef  unsigned short  u16;
typedef  unsigned long   ulong;
typedef  unsigned char   uchar;
typedef  unsigned char   BYTE;
typedef  unsigned short  WORD;
typedef  unsigned long   DWORD;

static DWORD hfmbdd;

void decompressbmp16(BYTE *dst,int h);

#define ASCII_5X7              1	//ASCII编码5*7点阵
#define ASCII_7X8              2	//ASCII编码7*8点阵
#define ASCII_6X12             3	//ASCII编码6*12点阵
#define ASCII_12_B_A           4	//ASCII编码12*12点阵不等宽Arial风格字符
#define ASCII_12_B_T           5	//ASCII编码12*12点阵不等宽Times New Roman风格字符
#define ASCII_8X16             6	//ASCII编码8*16点阵
#define ASCII_16_A             7	//ASCII编码16*16点阵不等宽Arial风格字符
#define ASCII_16_T             8	//ASCII编码16*16点阵不等宽Time New Roman风格字符
#define ASCII_12X24            9	//ASCII编码12*24点阵打印体风格字符
#define ASCII_24_B             10	//ASCII编码24*24点阵不等宽字符
#define ASCII_16X32            11	//ASCII编码16*32点阵Arial风格字符
#define ASCII_32_B             12	//ASCII编码32*32点阵Arial风格字符

#define B_11X16_A              13	//自制字符11*16 Arial风格字体
#define B_18X24_A              14	//自制字符18*24 Arial风格字体
#define B_22X32_A              15	//自制字符22*32 Arial风格字体
#define B_34X48_A              16	//自制字符34*48 Arial风格字体
#define B_40X64_A              17	//自制字符40*64 Arial风格字体
#define B_11X16_T              18	//自制字符11*16 Time New Roman风格字体
#define B_18X24_T              19	//自制字符18*24 Time New Roman风格字体
#define B_22X32_T              20	//自制字符22*32 Time New Roman风格字体
#define B_34X48_T              21	//自制字符34*48 Time New Roman风格字体
#define B_40X64_T              22	//自制字符40*64 Time New Roman风格字体
#define T_FONT_20X24           23	//自制字符20*24 时钟体风格字体
#define T_FONT_24X32           24	//自制字符24*32 时钟体风格字体
#define T_FONT_34X48           25	//自制字符34*48 时钟体风格字体
#define T_FONT_48X64           26	//自制字符48*64 时钟体风格字体
#define F_FONT_816             27	//自制字符8*16 方块体风格字体
#define F_FONT_1624            28	//自制字符16*24 方块体风格字体
#define F_FONT_1632            29	//自制字符16*32 方块体风格字体
#define F_FONT_2448            30	//自制字符24*48 方块体风格字体
#define F_FONT_3264            31	//自制字符32*64 方块体风格字体
#define KCD_UI_32              32	//自制UI图片 32*32

#define SEL_GB                 33	//在hzbmp16函数中用于charset参数,SEL_GB表示简体中文
#define SEL_JIS                34	//在hzbmp16函数中用于charset参数,SEL_JIS表示日文
#define SEL_KSC                35	//在hzbmp16函数中用于charset参数,SEL_KSC表示韩文

//读取ASCII编码的点阵数据
unsigned char  ASCII_GetData(unsigned char ASCIICode,unsigned long ascii_kind,unsigned char *DZ_Data);
unsigned long gt (unsigned int gb,unsigned int gbex);
unsigned long GB2312_24_GetData(unsigned int gb);
unsigned long JIS_16_GetData(unsigned int gb);
//unsigned long shift_h_GetData(unsigned char CODE);
unsigned long shift_h_GetData(unsigned char CODE,unsigned char *DZ_Data);
unsigned long KSC_16_GetData(unsigned int gb);
//读取8*16拉丁文编码数据
unsigned long LATIN_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//读取8*16西里尔文编码数据
unsigned long  CYRILLIC_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//读取8*16希腊文编码数据
unsigned long GREECE_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//读取8*16希伯来文编码数据
unsigned long HEBREW_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//读取16*16不等宽拉丁文编码数据
unsigned long LATIN_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//读取16*16不等宽西里尔文编码数据
unsigned long CYRILLIC_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//读取16*16不等宽希腊文编码数据
unsigned long GREECE_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//读取16*16不等宽西里尔文编码数据
unsigned long ALB_B_GetData(unsigned int unicode_alb,unsigned char *DZ_Data);
unsigned long THAILAND_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//Unicode转GB
unsigned int U2G(unsigned int UN_CODE);
//BIG5转GB
unsigned int BIG52GBK( unsigned char h,unsigned char l);
//Unicode转JIS0208
unsigned int U2J(WORD Unicode);
//Unicode转BIG5
unsigned int U2K(WORD Unicode);
//SHIFT-JIS转JIS0208
unsigned int SJIS2JIS(WORD code);
//读取自制字符数据
void zz_zf(unsigned char Sequence,unsigned char kind,unsigned char *DZ_Data);

#define ntohs(s)
              
void hzbmp16(unsigned char charset, unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);
void hzbmp24(unsigned char charset, unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);



