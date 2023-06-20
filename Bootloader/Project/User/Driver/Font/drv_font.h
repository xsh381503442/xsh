#ifndef __DRV_FONT_H__
#define __DRV_FONT_H__




#include <stdint.h>

#define DRV_FONT_MOULDE                  0
#define DRV_FONT_SPEED                   4000000
#define DRV_FONT_PIN_MOSI                7
#define DRV_FONT_PIN_MISO                6
#define DRV_FONT_PIN_CLK                 5
#define DRV_FONT_PIN_CS                  48
#define DRV_FONT_CHIP_SELECT             6

  
	
#define	DRV_FONT_REG_Read									0x03	//read data bytes
#define	DRV_FONT_REG_FastRead							0x0B	//read data bytes at higher speed
#define	DRV_FONT_REG_WREN									0x06	//write enable
#define	DRV_FONT_REG_WRDI									0x04	//write disable
#define	DRV_FONT_REG_PP										0x02	//page program
#define	DRV_FONT_REG_SE										0x20	//sector erase
#define	DRV_FONT_REG_BE										0xD8	//block erase(64k)
#define	DRV_FONT_REG_CE										0x60	//chip erase
#define DRV_FONT_REG_POWER_DOWN       	  (0xB9)
#define DRV_FONT_REG_RELEASE_POWER_DOWN   (0xAB)

#define HZ_MODE00 				0		//ÊúÖÃºáÅÅ(Y)
#define HZ_MODE01 				1		//ÊúÖÃÊúÅÅ(Z)
#define HZ_MODE10 				2		//ºáÖÃºáÅÅ(W)
#define HZ_MODE11 				3		//ºáÖÃÊúÅÅ(X)

#define HZ_MODE4  				4		//Y-->W 
#define HZ_MODE5  				5		//Y-->X
#define HZ_MODE6  				6		//Y-->Z
#define HZ_MODE7  				7		//Y-->Y
	
	
	




void drv_font_disable(void);
void drv_font_enable(void);
void drv_font_pwr_up(void);
void drv_font_pwr_down(void);


//For HFMA2Ylib
void r_dat_bat(unsigned long TAB_addr,unsigned int Num,unsigned char *p_arr);





#endif //__DRV_FONT_H__
