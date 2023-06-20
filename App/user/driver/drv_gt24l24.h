#ifndef DRV_GT24L24_H
#define	DRV_GT24L24_H

#include <stdint.h>
#include <stdbool.h>

#define	DRV_GT24L24_Read		0x03	//read data bytes
#define	DRV_GT24L24_FastRead	0x0B	//read data bytes at higher speed
#define	DRV_GT24L24_WREN		0x06	//write enable
#define	DRV_GT24L24_WRDI		0x04	//write disable
#define	DRV_GT24L24_PP			0x02	//page program
#define	DRV_GT24L24_SE			0x20	//sector erase
#define	DRV_GT24L24_BE			0xD8	//block erase(64k)
#define	DRV_GT24L24_CE			0x60	//chip erase
#define	DRV_GT24L24_SLEEP		0xB9	//Deep power down mode
#define	DRV_GT24L24_WAKEUP		0xAB	//Wake up

#define HZ_MODE00 				0		//ÊúÖÃºáÅÅ(Y)
#define HZ_MODE01 				1		//ÊúÖÃÊúÅÅ(Z)
#define HZ_MODE10 				2		//ºáÖÃºáÅÅ(W)
#define HZ_MODE11 				3		//ºáÖÃÊúÅÅ(X)

#define HZ_MODE4  				4		//Y-->W 
#define HZ_MODE5  				5		//Y-->X
#define HZ_MODE6  				6		//Y-->Z
#define HZ_MODE7  				7		//Y-->Y

void drv_gt24l24_enable(void);
void drv_gt24l24_disable(void);

//gt24l24¿âº¯Êý
extern void r_dat_bat(unsigned long TAB_addr,unsigned int Num,unsigned char *p_arr);

extern void drv_gt24l24_sleep(void);
extern void drv_gt24l24_wakeup(void);
extern void dev_gt24l24_sleep(void);
extern void dev_gt24l24_wakeup(void);
extern bool drv_gt24l24_self_test(void);
extern void drv_lcdram_map( uint8_t *getdate,uint8_t *putdata,uint8_t high,uint8_t width,uint8_t style);
extern void drv_gt24l24_gb_getdata(unsigned char charset, unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);
extern void drv_gt24l24_ascii_getdata(unsigned char ASCIICode,unsigned long ascii_kind,unsigned char *DZ_Data);



#endif
