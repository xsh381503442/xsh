
/**************************************************************************//**
 * @brief Implementation ohrl driver
 * @version 0.1.24  for 25hz acc
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2016 OhrAccurate Labs, http://www.OhrAccurate.com</b>
 *******************************************************************************
 *
 * This file is licensed under the OhrAccurate License Agreement. See the file
 * "OhrAccurate_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
 
#ifndef _OHRL_H_ 
#define _OHRL_H_ 

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stdint.h"
#include "types.h"

#define SAMPLERATE_PEDO   26

#define OHRL_ON		1
#define OHRL_OFF 	0

#define OHRL_CURRENT_CHANGED		1
#define OHRL_CURRENT_NOTCHANGE	0

#define OHRL_ENCRYPT_DEVICEID_ERROR	2
#define OHRL_ENCRYPT_CHECK_FAIL	1
#define OHRL_ENCRYPT_CHECK_PASS	0
//#define OHRL_ENCRYPT_LENGTH	    30

//#define OHRLRUN_NODATA				  0
//#define OHRLRUN_HANDLED					1
//#define OHRLRUN_NOENCRYPT			  0xFF

/* ohr output:hr data structure. */
typedef struct {
	uint8_t heartrate;		// heart rate,  40 to 220
	int8_t quality;				// -30 to 30, the bigger the beter 
	uint8_t onwrist;      //0 - on wrist, >0  - off wrist	
	int16_t ppg;          //ppg value  
	int16_t amb;          //amb value
	uint16_t ppg_maxmin;  //the change value of ppg within 1.5sec    
} type_OHRL_output;

extern type_OHRL_output  OHRL_data;
	
/**@brief OHLR_start_configure 
 *
 * @details    every time call this routine before restart ohrl.
 * @note       
 * @param[in]     
 * @retval                               
 */ 
void OHLR_start_configure(void);

/**@brief data process at ohr data ready 
 *
 * @details    this function put data into FIFO of ohr raw data, and run led gain control.
 * @note       call this function at si1171 data ready event !
 * @param[in]  OHRfifo     fifo of 16hz ohr raw data,
               ACCfifo     fifo of 25hz acc raw data,   
 * @retval     OHRL_CURRENT_CHANGED		ohr current need to change, call OHRL_GetLEDcurrent() to get new current 
 * 						 OHRL_CURRENT_NOTCHANGE	ohr current not need to change                               
 */ 

uint8_t OHRL_IrqDatatProcess(typ_fifodata * OHRfifo,typ_fifodata * ACCfifo);

//uint8_t OHRL_SLIrqDatatProcess(type_OHRL_input  *raw_instack);

/**@brief get ohr current value.
 *
 * @details  
 * @note       
 * @retval     current value for OHR LED    
 */ 
uint8_t OHRL_GetLEDcurrent(void);

/**@brief     run ohr algorithm.
 *
 * @details   this function gets data from FIFO of ohr raw data, and run ohr algorithm, including pedometer function if it 
 *            is enabled, content of OHRL_data1 and PedoSummary is updated after ohr algorithm
 *            if no data in the FIFO, it returns immediately.
 * @note      call this function at main loop.  
 * @retval    
 */
void  OHRL_algorithm(void);

/**@brief get string of ohrl version  
 *
 * @details   
 * @note                    
 * @retval    pointer of ohrl version  
							 the verion is a string AA.BB.CC.DD, where
							AA -  80~  means special verion for interface test
										00 to 10  normal release
							BB -  AFE version
										05 for si1181                        										
							CC -  verion of ohrl library
							DD -  verion of ohrl library	
 */
char *OHRL_GetOHRLVERSION(void);

/**@brief get string of ohr build version  
 *
 * @details   
 * @note                    
 * @retval    pointer of build version 
 							 the verion is a string YYYYMMDD.XX where
							YYYYMMDD -  year/month/date
							XX -  serial number at same day 
 */
char *OHRL_GetBUILDVERSION(void);


/**@brief ohr init 
 *
 * @details   do ohr initialize
 *
 * @note      call at ohr start 
 *             
 * @param[in]  block_id_encrypt     pointeer where to store encryption key
				       current               0, startup current with auto gain control enabled 
							                       1~0x3F  startup current with auto gain control disabled    				 
							 ohrlogbuff            pointer to a buff to logdata, the buff length should must be 16x20+1
																		 
 * @retval    OHRL_ENCRYPT_CHECK_FAIL   encryption verify fail    
 *            OHRL_ENCRYPT_CHECK_PASS   encryption verify pass 
 */
uint8_t OHRL_init(uint32_t* block_id_encrypt,uint8_t current, uint8_t *ohrlogbuff);


/**@brief push gsensor data into buff for step algorithm at ppg sensor stopped
 *
 * @details   push gsensor data into buff for step algorithm at ppg sensor stopped
 *
 * @note      gsensor configure:  +-8G, 16bit, 25ODR
 *             
 * @param[in]  fifo     pointeer of fifo data of gsensor
 */
void  OHRL_pushXYZ(typ_fifodata * fifo);

/*********************************************************************************************************************************
   OUTPUT of pedometer algorithm
*********************************************************************************************************************************/
typedef struct
{

	uint32_t TotalSteps_ul;  //total steps
	uint8_t  instantsteps;
	uint8_t  PedoSleep_flag;	// 0: active  1:Sleep
								  //if sleep, the application should set sample rate of g-sensor to 1hz to save power consumption.
								  //and it should set back to 16hz sample rate when active.

	uint8_t  PedoStatuschanged_flag;// 0: PedoSleep_flag not changed	1:PedoSleep_flag changed
																				// it is for monitor change of PedoSleep_flag.
  uint8_t   pace;                // pace in minute
}type_PedoSummary; 
extern type_PedoSummary  PedoSummary;

/**@brief initialize step counter
 *
 * @details   initialize step counter, call at system startup
 *
 * @note      gsensor configure:  +-8G, 16bit, 25ODR
 *             
 * @param[in]  
							 dir_gsensor           set direction of x,y,z
							                       0:  z - point to watch face, x - point to finger
							                       1:  z - point to watch face, x - point to arm
																		 2:  z - point to watch face, y - point to finger
																		 3:  z - point to watch face, y - point to arm
																		 4:  z - point to watch bottom, x - point to finger
																		 5:  z - point to watch bottom, x - point to arm
																		 6:  z - point to watch bottom, y - point to finger
																		 7:  z - point to watch bottom, y - point to arm
 */
void  Initial_Pedometer(uint8_t dir_gsensor);

void  OHRL_reset(void);

bool  OHRL_logtype(void);

#endif//_OHRL_H_
