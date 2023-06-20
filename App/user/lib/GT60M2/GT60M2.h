

#ifndef _FONT_H
#define _FONT_H

extern unsigned char r_dat(unsigned long int address);//�ͻ��Լ�ʵ�֣���address��ַ��ȡһ���ֽڵ����ݲ����ظ�����
//extern void r_dat_bat(unsigned long ADDRESS,unsigned int len,unsigned char *DZ_Data);//�ͻ��Լ�ʵ�֣���address��ַ��ȡlen���ֽڵ����ݲ����뵽DZ_Data���鵱��
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
//����ֵ : wh_buff =������/����߶�/ʵ�ʿ��/ʵ�ʸ߶�/
//sty:�������� 0x00,0x01
void get_gray_ascii(unsigned char *pBits,unsigned char sty,unsigned short ascii_code,
	unsigned char w,unsigned char h,unsigned char gray_level,unsigned short *wh_buff,unsigned char thick);


#endif
/*************************************************************
�����÷���
     get_font(pBits,0x03,0xb0,0xa1,32,32,32);//��ȡ32X32����ʸ������ġ� ���� �� �����ֽ�����128BYTE 
    get_font(pBits,0x05,0xb0,0xa1,32,32,32);//��ȡ32X32����ʸ������� ������ �� �����ֽ�����128BYTE 
 
   get_gray_data(pBits,0xboa1,24,24,2);//��ȡ����"��"��2bit�Ҷ����ݣ����洢������pBits��
*************************************************************/
