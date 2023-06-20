//#include <A2Ylib.h>
//��������������������������������������������
//��        ��ͨ�Ƽ���Ȩ����  ��
//��      GENITOP RESEARCH CO.,LTD.         ��
//��        created on 2015.7.27            ��
//��������������������������������������������

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

#define ASCII_5X7              1	//ASCII����5*7����
#define ASCII_7X8              2	//ASCII����7*8����
#define ASCII_6X12             3	//ASCII����6*12����
#define ASCII_12_B_A           4	//ASCII����12*12���󲻵ȿ�Arial����ַ�
#define ASCII_12_B_T           5	//ASCII����12*12���󲻵ȿ�Times New Roman����ַ�
#define ASCII_8X16             6	//ASCII����8*16����
#define ASCII_16_A             7	//ASCII����16*16���󲻵ȿ�Arial����ַ�
#define ASCII_16_T             8	//ASCII����16*16���󲻵ȿ�Time New Roman����ַ�
#define ASCII_12X24            9	//ASCII����12*24�����ӡ�����ַ�
#define ASCII_24_B             10	//ASCII����24*24���󲻵ȿ��ַ�
#define ASCII_16X32            11	//ASCII����16*32����Arial����ַ�
#define ASCII_32_B             12	//ASCII����32*32����Arial����ַ�

#define B_11X16_A              13	//�����ַ�11*16 Arial�������
#define B_18X24_A              14	//�����ַ�18*24 Arial�������
#define B_22X32_A              15	//�����ַ�22*32 Arial�������
#define B_34X48_A              16	//�����ַ�34*48 Arial�������
#define B_40X64_A              17	//�����ַ�40*64 Arial�������
#define B_11X16_T              18	//�����ַ�11*16 Time New Roman�������
#define B_18X24_T              19	//�����ַ�18*24 Time New Roman�������
#define B_22X32_T              20	//�����ַ�22*32 Time New Roman�������
#define B_34X48_T              21	//�����ַ�34*48 Time New Roman�������
#define B_40X64_T              22	//�����ַ�40*64 Time New Roman�������
#define T_FONT_20X24           23	//�����ַ�20*24 ʱ����������
#define T_FONT_24X32           24	//�����ַ�24*32 ʱ����������
#define T_FONT_34X48           25	//�����ַ�34*48 ʱ����������
#define T_FONT_48X64           26	//�����ַ�48*64 ʱ����������
#define F_FONT_816             27	//�����ַ�8*16 ������������
#define F_FONT_1624            28	//�����ַ�16*24 ������������
#define F_FONT_1632            29	//�����ַ�16*32 ������������
#define F_FONT_2448            30	//�����ַ�24*48 ������������
#define F_FONT_3264            31	//�����ַ�32*64 ������������
#define KCD_UI_32              32	//����UIͼƬ 32*32

#define SEL_GB                 33	//��hzbmp16����������charset����,SEL_GB��ʾ��������
#define SEL_JIS                34	//��hzbmp16����������charset����,SEL_JIS��ʾ����
#define SEL_KSC                35	//��hzbmp16����������charset����,SEL_KSC��ʾ����

//��ȡASCII����ĵ�������
unsigned char  ASCII_GetData(unsigned char ASCIICode,unsigned long ascii_kind,unsigned char *DZ_Data);
unsigned long gt (unsigned int gb,unsigned int gbex);
unsigned long GB2312_24_GetData(unsigned int gb);
unsigned long JIS_16_GetData(unsigned int gb);
//unsigned long shift_h_GetData(unsigned char CODE);
unsigned long shift_h_GetData(unsigned char CODE,unsigned char *DZ_Data);
unsigned long KSC_16_GetData(unsigned int gb);
//��ȡ8*16�����ı�������
unsigned long LATIN_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//��ȡ8*16������ı�������
unsigned long  CYRILLIC_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//��ȡ8*16ϣ���ı�������
unsigned long GREECE_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//��ȡ8*16ϣ�����ı�������
unsigned long HEBREW_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//��ȡ16*16���ȿ������ı�������
unsigned long LATIN_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//��ȡ16*16���ȿ�������ı�������
unsigned long CYRILLIC_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//��ȡ16*16���ȿ�ϣ���ı�������
unsigned long GREECE_B_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//��ȡ16*16���ȿ�������ı�������
unsigned long ALB_B_GetData(unsigned int unicode_alb,unsigned char *DZ_Data);
unsigned long THAILAND_GetData(unsigned int FontCode,unsigned char *DZ_Data);
//UnicodeתGB
unsigned int U2G(unsigned int UN_CODE);
//BIG5תGB
unsigned int BIG52GBK( unsigned char h,unsigned char l);
//UnicodeתJIS0208
unsigned int U2J(WORD Unicode);
//UnicodeתBIG5
unsigned int U2K(WORD Unicode);
//SHIFT-JISתJIS0208
unsigned int SJIS2JIS(WORD code);
//��ȡ�����ַ�����
void zz_zf(unsigned char Sequence,unsigned char kind,unsigned char *DZ_Data);

#define ntohs(s)
              
void hzbmp16(unsigned char charset, unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);
void hzbmp24(unsigned char charset, unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);



