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
 * @file            sensorcontrol.h
 * @date
 * @commit
 *
 * @brief
 * This file contains sensor device adapter layer APIs
 *
 */


#ifndef __SENSORCONTROL_H__
#define __SENSORCONTROL_H__

/*********************************************************************/
/* header includes */
#include "define.h"
#include "algo_adapter.h"

#if defined(INCLUDE_BMI160API)
#include "bmi160.h"
#elif defined(INCLUDE_BMA2X2API)
#include "bma2x2.h"
#endif

#if defined(INCLUDE_BMG160API)
#include "bmg160.h"
#endif

#if defined(INCLUDE_BMM050API)
#include "bmm050.h"
#endif




/*********************************************************************/
/* macro definitions */
/*! Accelerometer opmode Configuration */
#define ACCOPMODE_SLEEP                   (0)
#define ACCOPMODE_LOWPOWER                (1)
#define ACCOPMODE_REGULAR                 (2)
/*! Accelerometer grange Configuration */
#define ACCGRANGE_2G                      (0)
#define ACCGRANGE_4G                      (1)
#define ACCGRANGE_8G                      (2)
#define ACCGRANGE_16G                     (3)
/*! Accelerometer bandwidth Configuration */
#define ACC_BANDWIDTH_7_81HZ              (0)
#define ACC_BANDWIDTH_15_63HZ             (1)
#define ACC_BANDWIDTH_31_25HZ             (2)
#define ACC_BANDWIDTH_62_50HZ             (3)
#define ACC_BANDWIDTH_125HZ               (4)
/*! Magnetometer operational modes */
#define MAGOPMODE_SLEEP                   (0)
#define MAGOPMODE_FORCE                   (1)
#define MAGOPMODE_REGULAR                 (2)
#define MAGOPMODE_HIGHACCURACY            (3)
#define MAGOPMODE_ALWAYSON                (4)
#define MAGOPMODE_AKM_FUSE                (5)
#define UNKNOWN_MODE                      (6)


/*********************************************************************/
/* function prototype declarations */

/*!
 * @brief:  acc sensor read rawdata
 *             
 * @param[in] : 
 * @param[out] : S32 *x, S32 *y, S32 *z
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_acc_read_xyzdata( S32 *x, S32 *y, S32 *z);

/*!
 * @brief: acc sensor get grange
 *             
 * @param[in] : 
 * @param[out] : U8 *range
 * @param[in,out] : 
 *
 * @return :
 */
U8 dev_acc_get_grange( U8 *range);

/*!
 * @brief: acc sensor set grange
 *             
 * @param[in] : U8 range
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return : 0 when success
 */ 
U8 dev_acc_set_grange( U8 range);



/*!
 * @brief: acc sensor get operation mode
 *             
 * @param[in] : 
 * @param[out] : U8 *opmode, value of operation mode
 * @param[in,out] : 
 *
 * @return :communication routines
 */ 
U8 dev_acc_get_opmode( U8 *opmode);


/*!
 * @brief: acc sensor set operation mode
 *             
 * @param[in] : U8 opmode,location of the opmode
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :communication routines
 */ 
U8 dev_acc_set_opmode( U8 opmode);



/*!
 * @brief: acc sensor get bandwidth
 *             
 * @param[in] : 
 * @param[out] : value of bandwidth
 * @param[in,out] : 
 *
 * @return :
 */ 
U8 dev_acc_get_bandwidth( U8 *bw);


/*!
 * @brief: acc sensor set bandwidth
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return : communication routines
 */
U8 dev_acc_set_bandwidth( U8 bw);


/*!
* @brief: get acc sensor register value
*			  
* @param[in] : U8 address,address of the register. U8 count, Number of bytes of read from register.
* @param[out] : 
* @param[in,out] :	U8 *data, Data read from the register
*
* @return :result of communication routines
*/
U8 dev_acc_get_register(U8 *data,U8 address,U8 count);


/*!
* @brief:set acc sensor register value
*			  
* @param[in] : U8 address,address of the register. U8 count,Number of bytes of written to sensor
* @param[out] : 
* @param[in,out] : U8 *data, Data to be written to the register
*
* @return :result of communication routines
*/
U8 dev_acc_set_register(U8 *data,U8 address,U8 count);



/*!
 * @brief: mag sensor read raw data
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
U8 dev_mag_read_xyzdata( S32 *x, S32 *y, S32 *z);


/*!
 * @brief: mag sensor set operation mode
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
U8 dev_mag_set_opmode( U8 opmode);


/*!
 * @brief: mag sensor get operation mode
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
U8 dev_mag_get_opmode( U8 *opmode);


/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
U8 dev_gyro_read_xyzdata( S32 *x, S32 *y, S32 *z);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
U8 dev_gyro_set_bandwidth(U8 bandwidth);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
U8 dev_gyro_set_opmode(U8 opmode);


/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
uint32_t BST_SENSOR_I2C_READ(unsigned char device_addr, unsigned char register_addr, unsigned char *register_data, unsigned char length);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
uint32_t BST_SENSOR_I2C_WRITE(unsigned char device_addr, unsigned char register_addr, unsigned char *register_data, unsigned char length);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
void delaym_local(U16 mili_seconds);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
int32_t set_shuttle_vdd(uint8_t ucState);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
int32_t set_shuttle_vddio(uint8_t ucState);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
S32 sel_configure(U8 sel_per);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
U8 init_sensorlist(void);

/*!
 * @brief: 
 *             
 * @param[in] : 
 * @param[out] : 
 * @param[in,out] : 
 *
 * @return :
 */
U8 dev_i2c_init(void);

/*!
 * @brief: get the physical sensor odr which should be set after the configuration of virtual sensor 
 *         
 * @param[in] : no
 * @param[out] : no
 * @param[in,out] : no
 *
 * @return : odr which the physical sensor should set. when 0 or negative value, 
 *           should check the input sensor id or the validity of virtual sensor usage 
 */ 
S16 get_vs_configured_phy_odr(phys_id_enum_t phy_sensor_id);

#endif

/** @}*/