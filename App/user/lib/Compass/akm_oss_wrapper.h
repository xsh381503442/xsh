#ifndef __AKM_OSS_WRAPPER_H
#define __AKM_OSS_WRAPPER_H


#include "AKFS_Device.h"
#include "AKFS_Decomp.h"
#include "AKFS_AOC.h"
#include "AKFS_Math.h"
#include "AKFS_VNorm.h"
#include "string.h"


extern int convert_magnetic(	
	const	float		mag[3],
	const	int16		status,
			AKFLOAT*	vx,
			AKFLOAT*	vy,
			AKFLOAT*	vz,
			int16*		accuracy);
extern int akm_cal_init(void );


#endif

