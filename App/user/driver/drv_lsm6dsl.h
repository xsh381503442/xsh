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
//#include "define.h"
//#include "algo_adapter.h"


#include "stdint.h"
#include <stdbool.h>
#include "lsm6dsl/LSM6DSL_ACC_GYRO_driver.h"
#include "watch_config.h"
typedef struct
{

  /* Identity */
  uint8_t who_am_i;

  /* Configuration */
  uint8_t ifType;        /* 0 means I2C, 1 means SPI, etc. */
  uint8_t address;       /* Sensor I2C address (NOTE: Not a unique sensor ID). */
  uint8_t spiDevice;     /* Sensor Chip Select for SPI Bus */
  uint8_t instance;      /* Sensor instance (NOTE: Sensor ID unique only within its class). */
  uint8_t isInitialized; /* Sensor setup done. */
  uint8_t isEnabled;     /* Sensor ON. */
  uint8_t isCombo;       /* Combo sensor (component consists of more sensors). */

  /* Pointer to the Data */
  void *pData;

  /* Pointer to the Virtual Table */
  void *pVTable;
  /* Pointer to the Extended Virtual Table */
  void *pExtVTable;
} DrvContextTypeDef;

//for wrist tilt recognition
typedef struct{
    uint8_t is_verify_timer_started;
    uint8_t up_state_count;
//    uint8_t negative_count;
    uint8_t turning_state;
    uint8_t lExpireCounters;
}wrist_tilt_cb_t;


typedef enum
{
  COMPONENT_OK = 0,
  COMPONENT_ERROR,
  COMPONENT_TIMEOUT,
  COMPONENT_NOT_IMPLEMENTED
} DrvStatusTypeDef;

#define LSM6DSL_PEDOMETER_THRESHOLD_LOW       0x00  /**< Lowest  value of pedometer threshold */
#define LSM6DSL_PEDOMETER_THRESHOLD_MID_LOW   0x07
#define LSM6DSL_PEDOMETER_THRESHOLD_MID       0x0F
#define LSM6DSL_PEDOMETER_THRESHOLD_MID_HIGH  0x12
#define LSM6DSL_PEDOMETER_THRESHOLD_HIGH      0x1F  /**< Highest value of pedometer threshold */

extern uint8_t Sensor_IO_Write( void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite );
extern uint8_t Sensor_IO_Read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead );

uint8_t		Write_SIM( void );

//ÍÓÂÝÒÇ GYRO,¼ÓËÙ¶ÈACC
extern DrvStatusTypeDef drv_lsm6dsl_enable_pedometer( void);
extern DrvStatusTypeDef drv_lsm6dsl_disable_pedometer( void);
extern DrvStatusTypeDef drv_lsm6dsl_wrist_tilt_enable(void);
extern DrvStatusTypeDef drv_lsm6dsl_wrist_tilt_disable(void);
extern void drv_lsm6dsl_clear_stepcount(void);
extern uint32_t drv_lsm6dsl_get_stepcount( void );
extern DrvStatusTypeDef  drv_lsm6dsl_reset( void);
extern bool drv_lsm6dsl_id(void);
extern bool drv_lsm6dsl_sleep_status(void);
extern DrvStatusTypeDef  drv_lsm6dsl_acc_init(void);
extern void  drv_lsm6dsl_acc_powerdown(void);
extern void  drv_lsm6dsl_acc_lowpower(void);
extern void  drv_lsm6dsl_gyro_lowpower(void);
extern void  drv_lsm6dsl_gyro_init( void);
extern void  drv_lsm6dsl_gyro_powerdown( void);
extern DrvStatusTypeDef drv_lsm6dsl_acc_axes_raw(int16_t *pData);
extern DrvStatusTypeDef drv_lsm6dsl_gyro_axes_raw(int16_t *pData);
extern void  drv_lsm6dsl_fifo_init( void);
extern void  drv_lsm6dsl_fifo_uninit(void);
extern void drv_lsm6dsl_fifo_data(int16_t *dataRaw);
extern void timer_wrist_tilt_init(void);
extern void drv_lsm6dsl_acc_fifo_init( void);
#ifdef COD 
extern void  drv_lsm6dsl_cod_open(uint8_t acc_flag,uint8_t gyro_flag, uint8_t freq);
#endif
#ifdef WATCH_COD_BSP
extern void  drv_lsm6dsl_cod_close(void);
extern int dev_hrt_acc_init(void);
extern int dev_hrt_acc_pwron(void);
extern int dev_hrt_acc_fifo_data(int16_t *accbuf);
#endif

#if defined WATCH_IMU_CALIBRATION

extern DrvStatusTypeDef  drv_lsm6dsl_acc_test_init(void);
extern void  drv_lsm6dsl_gyro_test_init(LSM6DSL_ACC_GYRO_ODR_G_t nHz,LSM6DSL_ACC_GYRO_FS_G_t nDps);
extern void drv_lsm6dsl_fifo_test_init( void);
#endif
#endif

/** @}*/
