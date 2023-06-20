/******************************************************************************
 *
 * Copyright (C) 2012 Asahi Kasei Microdevices Corporation, Japan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/
#ifndef AKFS_INC_AOC_H
#define AKFS_INC_AOC_H

#include "AKFS_Device.h"
#include "main.h"

/***** Constant definition ****************************************************/
#if 0 // ak09911
#define AKFS_HBUF_SIZE	20
//#define AKFS_HOBUF_SIZE	4
#define AKFS_HOBUF_SIZE	20
#define AKFS_HOBUF_SIZE_MIN	20 
#define AKFS_HR_TH		10
#define AKFS_HO_TH		0.15
#else
#define AKFS_HBUF_SIZE	20
#define AKFS_HOBUF_SIZE	20
#define AKFS_HOBUF_SIZE_MIN	20 //30
#define AKFS_HR_TH		10
#define AKFS_HO_TH		0.8f
#endif

#if 0
	#define	AOC_WRITESTRING(...)			AM_LOG_WRITESTRING(__VA_ARGS__)
	#define	AOC_PRINTF(...) 				AM_LOG_PRINTF(__VA_ARGS__)
#else
	#define AOC_WRITESTRING(...)
	#define	AOC_PRINTF(...)	
#endif

extern int16 AKFS_HOBUF_SIZE_ADD2;

/***** Macro definition *******************************************************/

/***** Type declaration *******************************************************/
typedef struct _AKFS_AOC_VAR{
	AKFVEC		hbuf[AKFS_HBUF_SIZE];
	AKFVEC		hobuf[AKFS_HOBUF_SIZE];
	AKFLOAT		hraoc;
} AKFS_AOC_VAR;

/***** Prototype of function **************************************************/
AKLIB_C_API_START
int16 AKFS_AOC(
			AKFS_AOC_VAR	*haocv,
	const	AKFVEC			*hdata,
			AKFVEC			*ho
);

void AKFS_InitAOC(
			AKFS_AOC_VAR	*haocv
);

AKLIB_C_API_END

#endif

