/*--------------------------------------------------------------------------
Copyright (c) 2014, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#include "akm_oss_wrapper.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#define SENSOR_CAL_ALGO_VERSION 1
#define AKM_MAG_SENSE                   (1.0)
#define CSPEC_HNAVE_V   8
#define AKFS_GEOMAG_MAX 70


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

//struct sensor_vec {
//	union {
//		struct {
//			float data[4];
//		};
//		struct {
//			float x;
//			float y;
//			float z;
//		};
//	};
//};

typedef struct _AKMPRMS{

	/* Variables for Decomp. */
	AKFVEC                  fva_hdata[AKFS_HDATA_SIZE];
	uint8vec                i8v_asa;

	/* Variables forAOC. */
	AKFS_AOC_VAR    s_aocv;

	/* Variables for Magnetometer buffer. */
	AKFVEC                  fva_hvbuf[AKFS_HDATA_SIZE];
	AKFVEC                  fv_ho;
	AKFVEC                  fv_hs;
	//AKFS_PATNO              e_hpat;

	/* Variables for Accelerometer buffer. */
//	AKFVEC                  fva_avbuf[AKFS_ADATA_SIZE];
//	AKFVEC                  fv_ao;
//	AKFVEC                  fv_as;

	/* Variables for Direction. */
//	AKFLOAT                 f_azimuth;
//	AKFLOAT                 f_pitch;
//	AKFLOAT                 f_roll;

	/* Variables for vector output */
	AKFVEC                  fv_hvec;
	//AKFVEC                  fv_avec;
	int16                   i16_hstatus;

} AKMPRMS;

/*static*/ AKMPRMS g_prms;

 int convert_magnetic(	
	const	float		mag[3],
	const	int16		status,
			AKFLOAT*	vx,
			AKFLOAT*	vy,
			AKFLOAT*	vz,
			int16*		accuracy)
{
	int16 akret;
	int16 aocret;
	AKFLOAT radius;
	AKMPRMS *prms = &g_prms;
	int i;
//	char string[10];

	/* Shift out old data from the buffer for better calibration */
	for (i = AKFS_HDATA_SIZE - 1; i >= 1; i--) {
		prms->fva_hdata[i] = prms->fva_hdata[i - 1];
	}

	prms->fva_hdata[0].u.x = mag[0];
	prms->fva_hdata[0].u.y = mag[1];
	prms->fva_hdata[0].u.z = mag[2];
	
	AOC_PRINTF("AOC input  %6.3f,%6.3f,%6.3f  \n",prms->fva_hdata[0].u.x,prms->fva_hdata[0].u.y,prms->fva_hdata[0].u.z);

	/* Offset calculation is done in this function */
	/* hdata[in] : Android coordinate, sensitivity adjusted. */
	/* ho   [out]: Android coordinate, sensitivity adjusted. */
	aocret = AKFS_AOC(
		&prms->s_aocv,
		prms->fva_hdata,
		&prms->fv_ho
	);

	AOC_PRINTF("AOC cali offset  %6.3f,%6.3f,%6.3f  aocret=%d\n",prms->fv_ho.u.x,prms->fv_ho.u.y,prms->fv_ho.u.z,aocret);

	/* Subtract offset */
	/* hdata[in] : Android coordinate, sensitivity adjusted. */
	/* ho   [in] : Android coordinate, sensitivity adjusted. */
	/* hvbuf[out]: Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	akret = AKFS_VbNorm(
		AKFS_HDATA_SIZE,
		prms->fva_hdata,
		1,
		&prms->fv_ho,
		&prms->fv_hs,
		AKM_MAG_SENSE,
		AKFS_HDATA_SIZE,
		prms->fva_hvbuf
	);
	if (akret == AKFS_ERROR) {
		AOC_WRITESTRING("error here-1!\n");
		return -1;
	}

	/* Averaging */
	/* hvbuf[in] : Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	/* hvec [out]: Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted, averaged. */
	akret = AKFS_VbAve(
		AKFS_HDATA_SIZE,
		prms->fva_hvbuf,
		CSPEC_HNAVE_V,
		&prms->fv_hvec
	);
	if (akret == AKFS_ERROR) {
		AOC_WRITESTRING("error here-2!");
		return -1;
	}

	/* Check the size of magnetic vector */
	radius = AKFS_SQRT(
			(prms->fv_hvec.u.x * prms->fv_hvec.u.x) +
			(prms->fv_hvec.u.y * prms->fv_hvec.u.y) +
			(prms->fv_hvec.u.z * prms->fv_hvec.u.z));

	if (radius > AKFS_GEOMAG_MAX) {
		prms->i16_hstatus = 0;
	} else {
		if (aocret == AKFS_SUCCESS) {
			prms->i16_hstatus = 3;
		}
	}

	*vx = prms->fv_hvec.u.x;
	*vy = prms->fv_hvec.u.y;
	*vz = prms->fv_hvec.u.z;
	*accuracy = prms->i16_hstatus;

	
	AOC_PRINTF("AOC cali output  %6.3f,%6.3f,%6.3f  \n",prms->fv_hvec.u.x,prms->fv_hvec.u.y,prms->fv_hvec.u.z);
	AOC_PRINTF("AOC  radius  %6.3f, Accuracy =%d  \n",radius, prms->i16_hstatus);
	
	return 0;
}


 int akm_cal_init(void )
{
	AKMPRMS *prms = &g_prms;

	/* Clear all data. */
	memset(prms, 0, sizeof(AKMPRMS));

	/* Sensitivity */
	prms->fv_hs.u.x = AKM_MAG_SENSE;
	prms->fv_hs.u.y = AKM_MAG_SENSE;
	prms->fv_hs.u.z = AKM_MAG_SENSE;

	/* Initialize buffer */
	AKFS_InitBuffer(AKFS_HDATA_SIZE, prms->fva_hdata);
	AKFS_InitBuffer(AKFS_HDATA_SIZE, prms->fva_hvbuf);
//	AKFS_InitBuffer(AKFS_ADATA_SIZE, prms->fva_avbuf);

	/* Initialize for AOC */
	AKFS_InitAOC(&prms->s_aocv);
	/* Initialize magnetic status */
	prms->i16_hstatus = 0;

	return 0;
}


