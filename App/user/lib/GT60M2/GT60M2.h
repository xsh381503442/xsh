

#ifndef _FONT_H
#define _FONT_H

extern unsigned char r_dat(unsigned long int address);//客户自己实现，从address地址读取一个字节的数据并返回该数据
//extern void r_dat_bat(unsigned long ADDRESS,unsigned int len,unsigned char *DZ_Data);//客户自己实现，从address地址读取len个字节的数据并存入到DZ_Data数组当中
extern void r_dat_bat( unsigned long ReadAddr, unsigned short NumByteToRead,unsigned char* pBuffer);
extern unsigned char pBits[80*80/8+400];//[192*192/8];///* 192X192*/
extern unsigned char pram[6];
//
void DrawGlyph(unsigned short code1, unsigned short code2,int width, int height, int thick, unsigned short *pBits);
void L2BE(unsigned char *p,unsigned long len);
//
void get_font(unsigned char *pBits,unsigned char sty,unsigned char msb,unsigned char lsb,unsigned char len,unsigned char wth,unsigned char thick);
void Gray_Process(unsigned char *OutPutData ,int width,int High,unsigned char Grade);
void get_gray_data(unsigned char *pBits,unsigned short gb_code,unsigned char w,unsigned char h,unsigned char gray_level,unsigned char thick);
//返回值 : wh_buff =点阵宽度/点阵高度/实际宽度/实际高度/
//sty:字体类型 0x00,0x01
void get_gray_ascii(unsigned char *pBits,unsigned char sty,unsigned short ascii_code,
	unsigned char w,unsigned char h,unsigned char gray_level,unsigned short *wh_buff,unsigned char thick);


#endif
/*************************************************************
函数用法：
     get_font(pBits,0x03,0xb0,0xa1,32,32,32);//读取32X32宋体矢量点阵的” 啊” 字 点阵字节总数128BYTE 
    get_font(pBits,0x05,0xb0,0xa1,32,32,32);//读取32X32黑体矢量点阵的 “啊” 字 点阵字节总数128BYTE 
 
   get_gray_data(pBits,0xboa1,24,24,2);//获取文字"啊"的2bit灰度数据，并存储于数组pBits中
*************************************************************/
