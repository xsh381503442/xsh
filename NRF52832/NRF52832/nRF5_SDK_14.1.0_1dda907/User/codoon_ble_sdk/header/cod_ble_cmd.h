//
// Created by Paint on 2019/7/19.
//

#ifndef COD_SDK_COD_BLE_CMD_H
#define COD_SDK_COD_BLE_CMD_H

#include <stdint.h>
#include <stdio.h>

// CBC - Codoon Ble CMD
/**
 * 头指令
 * */
#define CBC_HEAD                        0xAA
/**
 * 文件传输的头指令
 * */
#define CBC_OTA_HEAD                    0xAB

/**
 * 主指令码
 */
#define CBC_BASIC_CMD                   0x01
#define CBC_SETTING_CMD                 0x02
#define CBC_DEV_CTRL_CMD                0x03
#define CBC_DATA_SYNC_CMD               0x04
#define CBC_FIND_NOTIFY_CMD             0x06
#define CBC_FACTORY_CMD                 0x07
#define CBC_OTA_CMD                     0x0A

/**
 * 子指令码
 */

/*--------------------------------基础命令（CBC_BASIC_CMD）----------------------------------------*/
#define CBC_BASIC_CONN_TEST                   0x01
#define CBC_BASIC_CONN_TEST_RES               0x81
#define CBC_BASIC_DEVICE_VERSION              0x02
#define CBC_BASIC_DEVICE_VERSION_RES          0x82
#define CBC_BASIC_DEVICE_ELE                  0x03
#define CBC_BASIC_DEVICE_ELE_RES              0x83
#define CBC_BASIC_UNBIND_DEVICE               0x05
#define CBC_BASIC_UNBIND_DEVICE_RES           0x85
#define CBC_BASIC_TX_MTU_TEST                 0x06
#define CBC_BASIC_TX_MTU_TEST_RES             0x86
#define CBC_BASIC_RX_MTU_TEST                 0x46
#define CBC_BASIC_RX_MTU_TEST_RES             0xC6
#define CBC_BASIC_GET_MAC                     0x07
#define CBC_BASIC_GET_MAC_RES                 0x87
#define CBC_BASIC_AGPS_STATUS                 0x08
#define CBC_BASIC_AGPS_STATUS_RES             0x88
#define CBC_BASIC_SYNC_IMEI                   0x09
#define CBC_BASIC_SYNC_IMEI_RES               0x89
#define CBC_BASIC_IOS_ANCS                    0x10
#define CBC_BASIC_IOS_ANCS_RES                0x90
#define CBC_BASIC_SET_MAC                     0x11
#define CBC_BASIC_SET_MAC_RES                 0x91
#define CBC_BASIC_BIND_DEVICE                 0x12
#define CBC_BASIC_BIND_DEVICE_RES             0x92
#define CBC_BASIC_CHECK_BIND                  0x13
#define CBC_BASIC_CHECK_BIND_RES              0x93
#define CBC_BASIC_COMMON_HEART                0x14
#define CBC_BASIC_COMMON_HEART_RES            0x94
#define CBC_BASIC_DEVICE_SYNC_IMEI            0x15
#define CBC_BASIC_DEVICE_SYNC_IMEI_RES        0x95
#define CBC_BASIC_SEND_BIND_STATUS            0x16
#define CBC_BASIC_SEND_BIND_STATUS_RES        0x96

/*--------------------------------设备设置（CBC_SETTING_CMD）---------------------------------------*/
#define CBC_SETTING_SET_TIME                                0x02
#define CBC_SETTING_SET_TIME_RES                            0x82
#define CBC_SETTING_GET_TIME                                0x42
#define CBC_SETTING_GET_TIME_RES                            0xC2
#define CBC_SETTING_SET_ALARM                               0x03
#define CBC_SETTING_SET_ALARM_RES                           0x83
#define CBC_SETTING_GET_ALARM                               0x43
#define CBC_SETTING_GET_ALARM_RES                           0xC3
#define CBC_SETTING_SET_USER_INFO                           0x05
#define CBC_SETTING_SET_USER_INFO_RES                       0x85
#define CBC_SETTING_GET_USER_INFO                           0x45
#define CBC_SETTING_GET_USER_INFO_RES                       0xC5
#define CBC_SETTING_SET_SIT_REMIND                          0x07
#define CBC_SETTING_SET_SIT_REMIND_RES                      0x87
#define CBC_SETTING_GET_SIT_REMIND                          0x47
#define CBC_SETTING_GET_SIT_REMIND_RES                      0xC7
#define CBC_SETTING_SET_SLEEP_SETTING                       0x08
#define CBC_SETTING_SET_SLEEP_SETTING_RES                   0x88
#define CBC_SETTING_GET_SLEEP_SETTING                       0x48
#define CBC_SETTING_GET_SLEEP_SETTING_RES                   0xC8
#define CBC_SETTING_SET_DRINK_REMIND                        0x0A
#define CBC_SETTING_SET_DRINK_REMIND_RES                    0x8A
#define CBC_SETTING_GET_DRINK_REMIND                        0x4A
#define CBC_SETTING_GET_DRINK_REMIND_RES                    0xCA
#define CBC_SETTING_SET_MSG_PUSH_SETTING                    0x0B
#define CBC_SETTING_SET_MSG_PUSH_SETTING_RES                0x8B
#define CBC_SETTING_GET_MSG_PUSH_SETTING                    0x4B
#define CBC_SETTING_GET_MSG_PUSH_SETTING_RES                0xCB
#define CBC_SETTING_SET_DIAL_PLATE                          0x0D
#define CBC_SETTING_SET_DIAL_PLATE_RES                      0x8D
#define CBC_SETTING_GET_DIAL_PLATE                          0x4D
#define CBC_SETTING_GET_DIAL_PLATE_RES                      0xCD
#define CBC_SETTING_SET_AUTO_PAUSE                          0x0E
#define CBC_SETTING_SET_AUTO_PAUSE_RES                      0x8E
#define CBC_SETTING_GET_AUTO_PAUSE                          0x4E
#define CBC_SETTING_GET_AUTO_PAUSE_RES                      0xCE
#define CBC_SETTING_SET_SPORT_NOTIFY_SETTING                0x0F
#define CBC_SETTING_SET_SPORT_NOTIFY_SETTING_RES            0x8F
#define CBC_SETTING_GET_SPORT_NOTIFY_SETTING                0x4F
#define CBC_SETTING_GET_SPORT_NOTIFY_SETTING_RES            0xCF
#define CBC_SETTING_SET_SPORT_BTN_FUNC                      0x10
#define CBC_SETTING_SET_SPORT_BTN_FUNC_RES                  0x90
#define CBC_SETTING_GET_SPORT_BTN_FUNC                      0x50
#define CBC_SETTING_GET_SPORT_BTN_FUNC_RES                  0xD0
#define CBC_SETTING_SET_STEP_TARGET                         0x11
#define CBC_SETTING_SET_STEP_TARGET_RES                     0x91
#define CBC_SETTING_SET_RAISE_TO_WAKE                       0x12
#define CBC_SETTING_SET_RAISE_TO_WAKE_RES                   0x92
#define CBC_SETTING_SET_ALL_DAY_HEART_CHECK                 0x13
#define CBC_SETTING_SET_ALL_DAY_HEART_CHECK_RES             0x93
#define CBC_SETTING_SET_AUTO_RECORD_SPORT                   0x14
#define CBC_SETTING_SET_AUTO_RECORD_SPORT_RES               0x94
#define CBC_SETTING_SET_DISTANCE_UNIT                       0x15
#define CBC_SETTING_SET_DISTANCE_UNIT_RES                   0x95
#define CBC_SETTING_SET_HEART_CHECK_IN_STANDALONE_SPORT     0x16
#define CBC_SETTING_SET_HEART_CHECK_IN_STANDALONE_SPORT_RES 0x96
#define CBC_SETTING_SET_LANGUAGE                            0x17
#define CBC_SETTING_SET_LANGUAGE_RES                        0x97
#define CBC_SETTING_GET_CUR_LANGUAGE                        0x57
#define CBC_SETTING_GET_CUR_LANGUAGE_RES                    0xD7
#define CBC_SETTING_GET_SUPPORT_LANGUAGE                    0x18
#define CBC_SETTING_GET_SUPPORT_LANGUAGE_RES                0x98
#define CBC_SETTING_SET_LOCATION_INFO                       0x19
#define CBC_SETTING_SET_LOCATION_INFO_RES                   0x99
#define CBC_SETTING_SET_EMERGENCY_CONTACTS                  0x1A
#define CBC_SETTING_SET_EMERGENCY_CONTACTS_RES              0x9A
#define CBC_SETTING_SET_REST_HEART_RATE                     0x1B
#define CBC_SETTING_SET_REST_HEART_RATE_RES                 0x9B
#define CBC_SETTING_SET_APP_INFO                            0x1C
#define CBC_SETTING_SET_APP_INFO_RES                        0x9C
#define CBC_SETTING_SET_LOG_SETTING                         0x1D
#define CBC_SETTING_SET_LOG_SETTING_RES                     0x9D

/*--------------------------------设备控制（CBC_DEV_CTRL_CMD）---------------------------------------*/
#define CBC_DC_SPORT_CTRL                             0x06
#define CBC_DC_SPORT_CTRL_RES                         0x86
#define CBC_DC_SET_LIMIT_HEART                        0x07
#define CBC_DC_SET_LIMIT_HEART_RES                    0x87
#define CBC_DC_SYNC_PHONE_CALL_STATUS                 0x08
#define CBC_DC_SYNC_PHONE_CALL_STATUS_RES             0x88
#define CBC_DC_PHONE_CALL_CTRL                        0x09
#define CBC_DC_PHONE_CALL_CTRL_RES                    0x89
#define CBC_DC_SET_PHONE_CALL_VIRB_DELAY              0x0A
#define CBC_DC_SET_PHONE_CALL_VIRB_DELAY_RES          0x8A
#define CBC_DC_SOS                                    0x0B
#define CBC_DC_SOS_RES                                0x8B

/*--------------------------------数据同步（CBC_DATA_SYNC_CMD）---------------------------------------*/
#define CBC_DS_GET_DATA_FRAME_NUM                     0x01
#define CBC_DS_GET_DATA_FRAME_NUM_RES                 0x81
#define CBC_DS_DELETE_DATA                            0x41
#define CBC_DS_DELETE_DATA_RES                        0xC1
#define CBC_DS_GET_STEP_DATA                          0x02
#define CBC_DS_GET_STEP_DATA_RES                      0x82
#define CBC_DS_GET_SLEEP_DATA                         0x03
#define CBC_DS_GET_SLEEP_DATA_RES                     0x83
#define CBC_DS_GET_HEART_DATA                         0x04
#define CBC_DS_GET_HEART_DATA_RES                     0x84
#define CBC_DS_GET_SPORT_DATA                         0x06
#define CBC_DS_GET_SPORT_DATA_RES                     0x86
#define CBC_DS_REAL_TIME_DATA                         0x07
#define CBC_DS_REAL_TIME_DATA_RES                     0x87
#define CBC_DS_CAST                                   0x08
#define CBC_DS_CAST_RES                               0x88
#define CBC_DS_GET_SENSOR_CAP                         0x12
#define CBC_DS_GET_SENSOR_CAP_RES                     0x92
#define CBC_DS_SENSOR_DATA_TRANS                      0x13
#define CBC_DS_SENSOR_DATA_TRANS_RES                  0x93
#define CBC_DS_WEATHER                                0x14
#define CBC_DS_WEATHER_RES                            0x94
#define CBC_DS_RT_LOG                                 0x15
#define CBC_DS_RT_LOG_RES                             0x95


/*--------------------------------查找/提醒（CBC_FIND_NOTIFY_CMD）---------------------------------------*/
#define CBC_FN_NOTIFY_MSG                                  0x03
#define CBC_FN_NOTIFY_MSG_RES                              0x83
#define CBC_FN_SET_NO_DISTURB                              0x04
#define CBC_FN_SET_NO_DISTURB_RES                          0x84

/*--------------------------------工厂命令（CBC_FACTORY_CMD）----------------------------------------------*/
#define CBC_FACTORY_POWER_OFF                                  0x01
#define CBC_FACTORY_POWER_OFF_RES                              0x81

/*--------------------------------OTA（CBC_OTA_CMD）----------------------------------------------*/

#define CBC_OTA_START                           0x01
#define CBC_OTA_START_RES                       0x81
#define CBC_OTA_GROUP_CRC                       0x02
#define CBC_OTA_GROUP_CRC_RES                   0x82
#define CBC_OTA_FILE_CRC                        0x03
#define CBC_OTA_FILE_CRC_RES                    0x83



#endif //COD_SDK_COD_BLE_CMD_H
