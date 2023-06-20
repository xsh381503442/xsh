#ifndef __OCC_H
#define	__OCC_H

#include "main.h"
#include "akm_oss_wrapper.h"
#define		LIS3MDL_ADDR 	0x38
#include <stdbool.h>
extern bool  ComCailFlag;

void Magnet_Init(void);
void Magnet_Start(void);
void Magnet_Close(void);
bool Magnet_Who_AM_I(void);
void Magnet_GetData(float *hw_d);
float Magnet_Cail_Auto(void);
bool Magnet_Cail_Manual(void);

#endif
