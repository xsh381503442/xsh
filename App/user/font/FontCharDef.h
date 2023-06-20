#ifndef FONTCHARDEF_H
#define	FONTCHARDEF_H

#include <stdint.h>
#include "drv_lcd.h"

#define		FONT_CHAR_16_ADDR		(0x80000)
#define		FONT_CHAR_24_ADDR		(FONT_CHAR_16_ADDR + 1*16*95)
#define		FONT_CHAR_32_ADDR		(FONT_CHAR_24_ADDR + 2*24*95)
#define		FONT_CHAR_46_ADDR		(FONT_CHAR_32_ADDR + 2*32*95)
#define		FONT_ARIAL_12_ADDR		(FONT_CHAR_46_ADDR + 3*46*10)
#define		FONT_ARIAL_16_ADDR		(FONT_ARIAL_12_ADDR + 2*12*95)
#define		FONT_ARIAL_24_ADDR		(FONT_ARIAL_16_ADDR + 2*16*95)
#define		FONT_ARIAL_32_ADDR		(FONT_ARIAL_24_ADDR + 3*24*95)

#define		FONT_NUM_16_ADDR		(FONT_ARIAL_32_ADDR + 4*32*95)
#define		FONT_NUM_24_ADDR		(FONT_NUM_16_ADDR + (LCD_NUM_16_WIDTH*LCD_NUM_16_HEIGHT*10))
#define		FONT_NUM_36_ADDR		(FONT_NUM_24_ADDR + (LCD_NUM_24_WIDTH*LCD_NUM_24_HEIGHT*10))
#define		FONT_NUM_46_ADDR		(FONT_NUM_36_ADDR + (LCD_NUM_36_WIDTH*LCD_NUM_36_HEIGHT*10))
#define		FONT_NUM_66_ADDR		(FONT_NUM_46_ADDR + (LCD_NUM_46_WIDTH*LCD_NUM_46_HEIGHT*10))
                          
#define		LCD_CN_16_SUM			(254)
#define		LCD_CN_24_SUM			(185)	
#define		LCD_CN_32_SUM			(34)
					
#define		INDEX_CN_16_ADDR			(FONT_NUM_66_ADDR + (LCD_NUM_66_WIDTH*LCD_NUM_66_HEIGHT*11))
#define		FONT_CN_16_ADDR				(INDEX_CN_16_ADDR + (LCD_CN_16_SUM*2))
#define		INDEX_CN_24_ADDR			(FONT_CN_16_ADDR + (LCD_CN_16_SUM*32))
#define		FONT_CN_24_ADDR				(INDEX_CN_24_ADDR + (LCD_CN_24_SUM*2))
#define		INDEX_CN_32_ADDR			(FONT_CN_24_ADDR + (LCD_CN_24_SUM*72))
#define		FONT_CN_32_ADDR				(INDEX_CN_32_ADDR + (LCD_CN_32_SUM*2))

#define		FONT_PIC_6X6_ADDR			(FONT_CN_32_ADDR + (LCD_CN_32_SUM*128))
#define		FONT_PIC_17X15_ADDR			(FONT_PIC_6X6_ADDR + (LCD_PIC_6X6_WIDTH*LCD_PIC_6X6_HEIGHT*PIC_6X6_SUM))		
#define		FONT_PIC_12X18_ADDR			(FONT_PIC_17X15_ADDR + (LCD_PIC_17X15_WIDTH*LCD_PIC_17X15_HEIGHT*PIC_17X15_SUM))
#define		FONT_PIC_15X16_ADDR			(FONT_PIC_12X18_ADDR + (LCD_PIC_12X18_WIDTH*LCD_PIC_12X18_HEIGHT*PIC_12X18_SUM))
#define		FONT_PIC_31X14_ADDR			(FONT_PIC_15X16_ADDR + (LCD_PIC_15X16_WIDTH*LCD_PIC_15X16_HEIGHT*PIC_15X16_SUM))
#define		FONT_PIC_23X24_ADDR			(FONT_PIC_31X14_ADDR + (LCD_PIC_31X14_WIDTH*LCD_PIC_31X14_HEIGHT*PIC_31X14_SUM))
#define		FONT_PIC_28X28_ADDR			(FONT_PIC_23X24_ADDR + (LCD_PIC_23X24_WIDTH*LCD_PIC_23X24_HEIGHT*PIC_23X24_SUM))
#define		FONT_PIC_46X38_ADDR			(FONT_PIC_28X28_ADDR + (LCD_PIC_28X28_WIDTH*LCD_PIC_28X28_HEIGHT*PIC_28X28_SUM))
#define		FONT_PIC_46X46_ADDR			(FONT_PIC_46X38_ADDR + (LCD_PIC_46X38_WIDTH*LCD_PIC_46X38_HEIGHT*PIC_46X38_SUM))
#define		FONT_PIC_17X22_ADDR			(FONT_PIC_46X46_ADDR + (LCD_PIC_46X46_WIDTH*LCD_PIC_46X46_HEIGHT*PIC_46X46_SUM))
#define		FONT_PIC_32X32_ADDR			(FONT_PIC_17X22_ADDR + (LCD_PIC_17X22_WIDTH*LCD_PIC_17X22_HEIGHT*PIC_17X22_SUM))
#define		FONT_PIC_50X54_ADDR			(FONT_PIC_32X32_ADDR + (LCD_PIC_32X32_WIDTH*LCD_PIC_32X32_HEIGHT*PIC_32X32_SUM))
#define		FONT_PIC_47X47_ADDR			(FONT_PIC_50X54_ADDR + (LCD_PIC_50X54_WIDTH*LCD_PIC_50X54_HEIGHT*PIC_50X54_SUM))
#define		FONT_PIC_40X40_ADDR			(FONT_PIC_47X47_ADDR + (LCD_PIC_47X47_WIDTH*LCD_PIC_47X47_HEIGHT*PIC_47X47_SUM))
#define		FONT_PIC_140X58_ADDR		(FONT_PIC_40X40_ADDR + (LCD_PIC_40X40_WIDTH*LCD_PIC_40X40_HEIGHT*PIC_40X40_SUM))
#define		FONT_PIC_190X190_ADDR		(FONT_PIC_140X58_ADDR + (LCD_PIC_140X58_WIDTH*LCD_PIC_140X58_HEIGHT*PIC_140X58_SUM))
#define		FONT_PIC_122X122_ADDR		(FONT_PIC_190X190_ADDR + (LCD_PIC_190X190_WIDTH*LCD_PIC_190X190_HEIGHT*PIC_190X190_SUM))
#define		FONT_PIC_19X38_ADDR			(FONT_PIC_122X122_ADDR + (LCD_PIC_122X122_WIDTH*LCD_PIC_122X122_HEIGHT*PIC_122X122_SUM))
#define		FONT_PIC_26X12_ADDR			(FONT_PIC_19X38_ADDR + (LCD_PIC_19X38_WIDTH*LCD_PIC_19X38_HEIGHT*PIC_19X38_SUM))
#define		FONT_PIC_12X12_ADDR			(FONT_PIC_26X12_ADDR + (LCD_PIC_26X12_WIDTH*LCD_PIC_26X12_HEIGHT*PIC_26X12_SUM))
#define		FONT_PIC_13X13_ADDR			(FONT_PIC_12X12_ADDR + (LCD_PIC_12X12_WIDTH*LCD_PIC_12X12_HEIGHT*PIC_12X12_SUM))
#define		FONT_PIC_7X7_ADDR			(FONT_PIC_13X13_ADDR + (LCD_PIC_13X13_WIDTH*LCD_PIC_13X13_HEIGHT*PIC_13X13_SUM))
#define		FONT_PIC_24X20_ADDR			(FONT_PIC_7X7_ADDR + (LCD_PIC_7X7_WIDTH*LCD_PIC_7X7_HEIGHT*PIC_7X7_SUM))
#define		FONT_PIC_36X28_ADDR			(FONT_PIC_24X20_ADDR + (LCD_PIC_24X20_WIDTH*LCD_PIC_24X20_HEIGHT*PIC_24X20_SUM))
#define		FONT_PIC_14X18_ADDR			(FONT_PIC_36X28_ADDR + (LCD_PIC_36X28_WIDTH*LCD_PIC_36X28_HEIGHT*PIC_36X28_SUM))
#define		FONT_PIC_10X13_ADDR			(FONT_PIC_14X18_ADDR + (LCD_PIC_14X18_WIDTH*LCD_PIC_14X18_HEIGHT*PIC_14X18_SUM))
#define		FONT_PIC_50X44_ADDR			(FONT_PIC_10X13_ADDR + (LCD_PIC_10X13_WIDTH*LCD_PIC_10X13_HEIGHT*PIC_10X13_SUM))
#define		FONT_PIC_11X16_ADDR			(FONT_PIC_50X44_ADDR + (LCD_PIC_50X44_WIDTH*LCD_PIC_50X44_HEIGHT*PIC_50X44_SUM))
#define		FONT_PIC_9X9_ADDR			(FONT_PIC_11X16_ADDR + (LCD_PIC_11X16_WIDTH*LCD_PIC_11X16_HEIGHT*PIC_11X16_SUM))
#define		FONT_PIC_18X14_ADDR			(FONT_PIC_9X9_ADDR + (LCD_PIC_9X9_WIDTH*LCD_PIC_9X9_HEIGHT*PIC_9X9_SUM))
#define		FONT_PIC_16X18_ADDR			(FONT_PIC_18X14_ADDR + (LCD_PIC_18X14_WIDTH*LCD_PIC_18X14_HEIGHT*PIC_18X14_SUM))

#define		FONT_BITMAP_180X192_ADDR	(FONT_PIC_16X18_ADDR + (LCD_PIC_16X18_WIDTH*LCD_PIC_16X18_HEIGHT*PIC_16X18_SUM))
#define		FONT_PIC_62X62_ADDR			(FONT_BITMAP_180X192_ADDR + (23*LCD_BITMAP_180X192_HEIGHT*BITMAP_180X192_SUM))
#define   FONT_BMP_32X30_ADDR     (FONT_PIC_62X62_ADDR + (LCD_PIC_62X62_WIDTH*LCD_PIC_62X62_HEIGHT*PIC_62x62_SUM))
#define   FONT_BMP_36X30_ADDR     (FONT_BMP_32X30_ADDR + (4*LCD_BMP_32X30_HEIGHT*BMP_32X30_SUM))
#define   FONT_BMP_10X12_ADDR     (FONT_BMP_36X30_ADDR + (5*LCD_BMP_36X30_HEIGHT*BMP_36X30_SUM))
#define   FONT_BMP_16X16_ADDR     (FONT_BMP_10X12_ADDR + (2*LCD_BMP_10X12_HEIGHT*BMP_10X12_SUM))
#define   FONT_BMP_24X18_ADDR     (FONT_BMP_16X16_ADDR + (2*LCD_BMP_16X16_HEIGHT*BMP_16X16_SUM))
#define   FONT_BMP_30X12_ADDR     (FONT_BMP_24X18_ADDR + (3*LCD_BMP_24X18_HEIGHT*BMP_24X18_SUM))
#define   FONT_BMP_48X48_ADDR     (FONT_BMP_30X12_ADDR + (4*LCD_BMP_30X12_HEIGHT*BMP_30X12_SUM))
#define   FONT_BMP_40X48_ADDR     (FONT_BMP_48X48_ADDR + (6*LCD_BMP_48X48_HEIGHT*BMP_48X48_SUM))



#endif