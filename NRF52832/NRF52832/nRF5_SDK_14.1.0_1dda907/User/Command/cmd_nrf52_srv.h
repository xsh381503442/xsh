#ifndef __CMD_NRF52_SRV_H__
#define __CMD_NRF52_SRV_H__
#include "watch_config.h"

//config char command
#define CMD_NRF52_SRV_DEVICE_NAME_READ                              (0x01)
#define CMD_NRF52_SRV_DEVICE_NAME_UPDATE                            (0x02)
#define CMD_NRF52_SRV_ADV_PARAM_READ                                (0x03)
#define CMD_NRF52_SRV_ADV_PARAM_UPDATE                              (0x04)
#define CMD_NRF52_SRV_CONN_PARAM_READ                               (0x05)
#define CMD_NRF52_SRV_CONN_PARAM_UPDATE                             (0x06)
#define CMD_NRF52_SRV_VERSION_CODE_READ                             (0x07)
#define CMD_NRF52_SRV_VERSION_CODE_UPDATE                           (0x08)
#define CMD_NRF52_SRV_MTU_REQUEST                                   (0x09)


//config char response
#define CMD_NRF52_SRV_DEVICE_NAME_READ_RESPONSE                     (0x81)
#define CMD_NRF52_SRV_DEVICE_NAME_UPDATE_RESPONSE                   (0x82)
#define CMD_NRF52_SRV_ADV_PARAM_READ_RESPONSE                       (0x83)
#define CMD_NRF52_SRV_ADV_PARAM_UPDATE_RESPONSE                     (0x84)
#define CMD_NRF52_SRV_CONN_PARAM_READ_RESPONSE                      (0x85)
#define CMD_NRF52_SRV_CONN_PARAM_UPDATE_RESPONSE                    (0x86)
#define CMD_NRF52_SRV_VERSION_CODE_READ_RESPONSE                    (0x87)
#define CMD_NRF52_SRV_VERSION_CODE_UPDATE_RESPONSE                  (0x88)
#define CMD_NRF52_SRV_MTU_REQUEST_RESPONSE                          (0x89)


//control char command
#define CMD_NRF52_SRV_JUMP_BOOTLOADER                               (0x01)
#define CMD_NRF52_SRV_RESET                                         (0x02)
#define CMD_NRF52_SRV_BOND_REQUEST                                  (0x03)
#define CMD_NRF52_SRV_BOND_REQUEST_ANDROID                          (0x04)
#define CMD_NRF52_SRV_BOND_DELETE_REQUEST_ANDROID                   (0x05)
#define CMD_NRF52_SRV_BOND_DELETE_ALL_REQUEST_ANDROID               (0x06)
#define CMD_NRF52_SRV_PASSKEY_REQUEST_ANDROID                       (0x07)
#define CMD_NRF52_SRV_PASSKEY_CONFIRM_ANDROID                       (0x08)
#define CMD_NRF52_SRV_DISCONNECT                                    (0x09)
#define CMD_NRF52_SRV_ANCS_DISC_COMPLETE                            (0x0A)
#define CMD_NRF52_SRV_ANCS_SWITCH_READ                              (0x0B)
#define CMD_NRF52_SRV_ANCS_SWITCH_WRITE                             (0x0C)
#define CMD_NRF52_SRV_ANCS_NOTIFY_ENABLE                            (0x0D)
#define CMD_NRF52_SRV_ANCS_NOTIFY_DISABLE                           (0x0E)
#define CMD_NRF52_SRV_DEBOND_IOS                                    (0x11)
#define CMD_NRF52_SRV_DEBOND_ANDROID                                (0x12)
#define CMD_NRF52_SRV_FIND_PHONE                                    (0x13)
#define CMD_NRF52_SRV_BOND_WRITE_DIRECT                             (0x14)
#define CMD_NRF52_SRV_SOS_GPS                                       (0x15)
#define CMD_NRF52_SRV_FIND_WATCH                                    (0x16)
#define CMD_NRF52_SRV_GET_WATCH_DEFAULT_SPORT_MODE                  (0x17)

#ifdef WATCH_HAS_HYTERA_FUNC
#define CMD_NRF52_SRV_HYTERA_HEARTRATE_COLLECTION_START				(0x20)
#define CMD_NRF52_SRV_HYTERA_HEARTRATE_COLLECTION_STOP				(0x21)
#define CMD_NRF52_SRV_HYTERA_SPORT_DATA_COLLECTION_START			(0x22)
#define CMD_NRF52_SRV_HYTERA_SPORT_DATA_COLLECTION_STOP				(0x23)
#define CMD_NRF52_SRV_HYTERA_SPORT_DATA_READ						(0x24)
#define CMD_NRF52_SRV_HYTERA_HEARTRATE_DATA_READ					(0x25)

#endif


//control char response
#define CMD_NRF52_SRV_JUMP_BOOTLOADER_RESPONSE                      (0x81)
#define CMD_NRF52_SRV_RESET_RESPONSE                                (0x82)
#define CMD_NRF52_SRV_BOND_REQUEST_RESPONSE                         (0x83)
#define CMD_NRF52_SRV_BOND_REQUEST_ANDROID_RESPONSE                 (0x84)
#define CMD_NRF52_SRV_BOND_DELETE_REQUEST_ANDROID_RESPONSE          (0x85)
#define CMD_NRF52_SRV_BOND_DELETE_ALL_REQUEST_ANDROID_RESPONSE      (0x86)
#define CMD_NRF52_SRV_PASSKEY_REQUEST_ANDROID_RESPONSE              (0x87)
#define CMD_NRF52_SRV_PASSKEY_CONFIRM_ANDROID_RESPONSE              (0x88)
#define CMD_NRF52_SRV_DISCONNECT_RESPONSE                           (0x89)
#define CMD_NRF52_SRV_ANCS_DISC_COMPLETE_RESPONSE                   (0x8A)
#define CMD_NRF52_SRV_ANCS_SWITCH_READ_RESPONSE                     (0x8B)
#define CMD_NRF52_SRV_ANCS_SWITCH_WRITE_RESPONSE                    (0x8C)
#define CMD_NRF52_SRV_ANCS_NOTIFY_ENABLE_RESPONSE                   (0x8D)
#define CMD_NRF52_SRV_ANCS_NOTIFY_DISABLE_RESPONSE                  (0x8E)
#define CMD_NRF52_SRV_DEBOND_IOS_RESPONSE                           (0x91)
#define CMD_NRF52_SRV_DEBOND_ANDROID_RESPONSE                       (0x92)
#define CMD_NRF52_SRV_FIND_PHONE_RESPONSE                           (0x93)
#define CMD_NRF52_SRV_BOND_WRITE_DIRECT_RESPONSE                    (0x94)
#define CMD_NRF52_SRV_SOS_GPS_RESPONSE                              (0x95)
#define CMD_NRF52_SRV_FIND_WATCH_RESPONSE                           (0x96)
#define CMD_NRF52_SRV_GET_WATCH_DEFAULT_SPORT_MODE_RESPONSE         (0x97)

#ifdef WATCH_HAS_HYTERA_FUNC
#define CMD_NRF52_SRV_HYTERA_HEARTRATE_COLLECTION_START_RESPONSE	(0xA0)
#define CMD_NRF52_SRV_HYTERA_HEARTRATE_COLLECTION_STOP_RESPONSE		(0xA1)
#define CMD_NRF52_SRV_HYTERA_SPORT_DATA_COLLECTION_START_RESPONSE	(0xA2)
#define CMD_NRF52_SRV_HYTERA_SPORT_DATA_COLLECTION_STOP_RESPONSE	(0xA3)
#define CMD_NRF52_SRV_HYTERA_SPORT_DATA_READ_RESPONSE				(0xA4)
#define CMD_NRF52_SRV_HYTERA_HEARTRATE_DATA_READ_RESPONSE			(0xA5)

#endif




//error code
#define CMD_NRF52_SRV_SUCCUSS                                   0x00//USE MASK
#define CMD_NRF52_SRV_ERROR_OUT_OF_BOUND                        0x01
#define CMD_NRF52_SRV_ERROR_INVALID_PARAMS                      0x02
#define CMD_NRF52_SRV_ERROR_CONN_PARAM_NOT_ACCEPT               0x04
#define CMD_NRF52_SRV_ERROR_FAIL                                0x08
#define CMD_NRF52_SRV_ERROR_ANDROID_MAC_TABLE_FULL              0x10
#define CMD_NRF52_SRV_ERROR_ANDROID_MAC_NOT_EXIT                0x20
#define CMD_NRF52_SRV_ERROR_INTERVAL_MASK                       0x80//MASK FOR NRF INTERNEL ERROR


#endif //__CMD_NRF52_SRV_H__
