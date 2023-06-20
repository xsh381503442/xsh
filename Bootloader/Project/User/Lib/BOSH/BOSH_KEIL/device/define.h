#ifdef BOSH_KEIL_API 
/*!
 * @section LICENSE
 *
 * (C) Copyright 2011 - 2015 Bosch Sensortec GmbH All Rights Reserved
 *------------------------------------------------------------------------------
 * Disclaimer
 *
 * Common: Bosch Sensortec products are developed for the consumer goods
 * industry. They may only be used within the parameters of the respective valid
 * product data sheet.  Bosch Sensortec products are provided with the express
 * understanding that there is no warranty of fitness for a particular purpose.
 * They are not fit for use in life-sustaining, safety or security sensitive
 * systems or any system or device that may lead to bodily harm or property
 * damage if the system or device malfunctions. In addition, Bosch Sensortec
 * products are not fit for use in products which interact with motor vehicle
 * systems.  The resale and/or use of products are at the purchaser's own risk
 * and his own responsibility. The examination of fitness for the intended use
 * is the sole responsibility of the Purchaser.
 *
 * The purchaser shall indemnify Bosch Sensortec from all third party claims,
 * including any claims for incidental, or consequential damages, arising from
 * any product use not covered by the parameters of the respective valid product
 * data sheet or not approved by Bosch Sensortec and reimburse Bosch Sensortec
 * for all costs in connection with such claims.
 *
 * The purchaser must monitor the market for the purchased products,
 * particularly with regard to product safety and inform Bosch Sensortec without
 * delay of all security relevant incidents.
 *
 * Engineering Samples are marked with an asterisk (*) or (e). Samples may vary
 * from the valid technical specifications of the product series. They are
 * therefore not intended or fit for resale to third parties or for use in end
 * products. Their sole purpose is internal client testing. The testing of an
 * engineering sample may in no way replace the testing of a product series.
 * Bosch Sensortec assumes no liability for the use of engineering samples. By
 * accepting the engineering samples, the Purchaser agrees to indemnify Bosch
 * Sensortec from all claims arising from the use of engineering samples.
 *
 * Special: This software module (hereinafter called "Software") and any
 * information on application-sheets (hereinafter called "Information") is
 * provided free of charge for the sole purpose to support your application
 * work. The Software and Information is subject to the following terms and
 * conditions:
 *
 * The Software is specifically designed for the exclusive use for Bosch
 * Sensortec products by personnel who have special experience and training. Do
 * not use this Software if you do not have the proper experience or training.
 *
 * This Software package is provided `` as is `` and without any expressed or
 * implied warranties, including without limitation, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 * Bosch Sensortec and their representatives and agents deny any liability for
 * the functional impairment of this Software in terms of fitness, performance
 * and safety. Bosch Sensortec and their representatives and agents shall not be
 * liable for any direct or indirect damages or injury, except as otherwise
 * stipulated in mandatory applicable law.
 *
 * The Information provided is believed to be accurate and reliable. Bosch
 * Sensortec assumes no responsibility for the consequences of use of such
 * Information nor for any infringement of patents or other rights of third
 * parties which may result from its use.
 *
 * @file            define.h
 *
 * @brief
 * This file contains how to define macro, enumerations, structures,
 * functions, etc.
 *
 */

#ifndef DEFINE_H_
#define DEFINE_H_





/*********************************************************************/
/* header includes */
//#include "compiler.h" /**< customer should change this headfile according to their IDE */


/*********************************************************************/
/* macro definitions */

/*! Start of define the use case configuration,Only one should be defined among the following five use case types */
#define INCLUDE_USECASE_NDOF
//#define INCLUDE_USECASE_IMU
//#define INCLUDE_USECASE_COMPASS
//#define INCLUDE_USECASE_M4G
//#define INCLUDE_USECASE_ACCELERATION
/*! End of define the use case configuration */


/*! Start of usecase select shuttle board(shuttle board is simple board which only contain sensors)
note: component library version don't need usecase selection MACRO, just open sensor MACRO is necessary */
#if defined(INCLUDE_USECASE_NDOF)
    #define SHUTTLE_BOARD_BMI160_BMM050_2TH		
	//#define SHUTTLE_BOARD_BMI160_BMM050_DIRECT
#elif defined(INCLUDE_USECASE_IMU)
    //#define SHUTTLE_BOARD_BMI160 
	#define SHUTTLE_BOARD_BMI055
#elif defined(INCLUDE_USECASE_COMPASS)
    #define SHUTTLE_BOARD_BMC156
#elif defined(INCLUDE_USECASE_M4G)
    #define SHUTTLE_BOARD_BMC156
#elif defined(INCLUDE_USECASE_ACCELERATION)
    // 
#endif
/*! End of usecase select shuttle board */


/*! Start sensor MACRO defined, sensors defined start. customer should change with their own sensor selection */
#if defined(SHUTTLE_BOARD_BMI160) 
    #define INCLUDE_BMI160API
#elif defined(SHUTTLE_BOARD_BMI160_BMM050_2TH)
	#define INCLUDE_BMI160API
	#define INCLUDE_BMM050API
	#define INCLUDE_BMI160API_BMM050_2TH_INTERFACE 
#elif defined(SHUTTLE_BOARD_BMI160_BMM050_DIRECT)
    #define INCLUDE_BMI160API
    #define INCLUDE_BMM050API
#elif defined(SHUTTLE_BOARD_BMI055)
	#define INCLUDE_BMA2X2API
	#define INCLUDE_BMG160API
#elif defined(SHUTTLE_BOARD_BMC156)
    #define INCLUDE_BMM050API
    #define INCLUDE_BMA2X2API
    #define INCLUDE_BMM050API_DRICTIVE_INTERFACE
#endif
/*! End of shuttle board defined */


/*! Define Hardware Version, affecting the axis remapping, should be set as the pcb layout */
#define	WSF_SFD20
//#define WSF_SFD21

#define ON			    (1)						
#define OFF			    (0)

#define DEBUG_OPEN

#if defined(DEBUG_OPEN)
#define debug_sensor(format, ...)  printf(format, ##__VA_ARGS__)
#else
#define debug_sensor(format, ...)  {}
#endif


/*********************************************************************/
/* type definitions */

/*!
 * comment: Enumeration for function return values
 */
enum FUNCTION_RETURN_VALUES
{
	OK = 0,									/**< no Error occured */
	ERROR_PARAMETER_NULL_PTR=1,				/**< one or more parameter point is NULL */
	ERROR_PARAMETER_OUT_OF_RANGE=2,			/**< one or more parameter out of range */
	ERROR_RESULT_OUT_OF_RANGE=3,			/**< The return value is out of range */
	ERROR_UNKNOWN_INSTRUCTION=4,			/**< unknown instruction */
	ERROR_UNKNOWN_SENSOR=5,					/**< unknown sensor */
	ERROR_BSC=6,							/**< errors from BSC library */
	ERROR_OTHERS=7,							/**< other error occured */
};


/*!
 * @brief :physical sensor id
 */
typedef enum phys_id_enum_type
{
    ENUM_ACCELEROMETER = 0,
    ENUM_GYROSCOPE,
	ENUM_MAGNETOMETER,
	ENUM_PHYSENSOR_UNKNOWN,
} phys_id_enum_t;

#endif /*  DEFINE_H_ */
#endif //BOSH_KEIL_API
/** @}*/


