#ifndef __CMD_BLE_H__
#define __CMD_BLE_H__
#include "watch_config.h"

#define UART_PACKET_HEADER				(0xA5)



#define BLE_UART_CCOMMAND_MASK_BOOTLOADER_POSITION             (8)
#define BLE_UART_CCOMMAND_MASK_APPLICATION_POSITION            (9)
#define PC_UART_CCOMMAND_MASK_UPDATE_ALGORITHM_POSITION       (10)
#define PC_UART_CCOMMAND_MASK_HARDWARE_TEST_POSITION          (11)
#define BLE_UART_CCOMMAND_MASK_APP_POSITION                    (12)




#define BLE_UART_CCOMMAND_MASK_BOOTLOADER                                  ((1<<BLE_UART_CCOMMAND_MASK_BOOTLOADER_POSITION)<<8|(UART_PACKET_HEADER))

#define BLE_UART_CCOMMAND_BOOTLOADER_UNSPPORT       		            			 (0xFF<<8)

#define BLE_UART_CCOMMAND_BOOTLOADER_BLE_INFO                              (0x21<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_JUMP_BOOTLOADER                       (0x22<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_JUMP_APPLICATION                      (0x23<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_RESET                                 (0x24<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_BLE_BOOTSETTING                       (0x25<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_BLE_PREVALIDATE                       (0x26<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_BLE_IMAGE_DATA                        (0x27<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_BLE_IMAGE_WRITE                       (0x28<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_BLE_POSTVALIDATE                      (0x29<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_BLE_RESET_N_ACTIVATE                  (0x2A<<8)



#define BLE_UART_RESPONSE_BOOTLOADER_BLE_INFO                              (0xA1<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_JUMP_BOOTLOADER                       (0xA2<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_JUMP_APPLICATION                      (0xA3<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_RESET                                 (0xA4<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_BLE_BOOTSETTING                      (0xA5<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_BLE_PREVALIDATE                      (0xA6<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_BLE_IMAGE_DATA                       (0xA7<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_BLE_IMAGE_WRITE                      (0xA8<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_BLE_POSTVALIDATE                     (0xA9<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_BLE_RESET_N_ACTIVATE                 (0xAA<<8)




#define BLE_UART_CCOMMAND_MASK_APPLICATION                                 ((1<<BLE_UART_CCOMMAND_MASK_APPLICATION_POSITION)<<8|(UART_PACKET_HEADER))

#define BLE_UART_CCOMMAND_APPLICATION_UNSPPORT       		            			 ((0xFF)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APP_ERROR      		            			 ((0xFE)<<8)


/*
����
*/

#define BLE_UART_CCOMMAND_APPLICATION_INFO_APOLLO2                        ((0x11)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APOLLO2_JUMP_BOOTLOADER             ((0x12)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APOLLO2_JUMP_APPLICATION            ((0x13)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APOLLO2_JUMP_UPDATE_ALGORITHM       ((0x14)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APOLLO2_RESET                       ((0x15)<<8)

#define BLE_UART_CCOMMAND_APPLICATION_SOS_GPS                             ((0x16)<<8)

#ifdef COD
#define BLE_UART_CCOMMAND_APPLICATION_INFO_BAT                       	  ((0x17)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_CURRENT_TIME_READ                   ((0x18)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_LOCATION_INFO						  ((0x19)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_STATUS                   ((0x1A)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_PHONE_CALL_DELAY                    ((0x1B)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ON_BIND_DEVICE                      ((0x1C)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_SEND_BIND_STATUS                    ((0x1D)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ON_SPORT_CAST                    	  ((0x1E)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ON_TRAINING_CAST                    ((0x1F)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_RT_DATA_TRANS                    	  ((0x20)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_SENSOR_DATA_TRANS                   ((0x2B)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_REST_HEARTRATE                  	  ((0x2C)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_DATA_FRAME_NUM                  	  ((0x2D)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_DELETE_DATA                  	      ((0x2E)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_MAC_READ                  	      ((0x2F)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_MAC_WRITE                  	      ((0x3F)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_UPDATA_AGPS_TIME                 	  ((0x4A)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_UPDATA_BP_START                 	  ((0x4B)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_UPDATA_BP_FINISH                 	  ((0x4C)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_REAL_TIME_LOG_TRANS                 ((0x4D)<<8)

#endif

#define BLE_UART_CCOMMAND_APPLICATION_BLE_INFO                              ((0x21)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER                       ((0x22)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_JUMP_APPLICATION                      ((0x23)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_RESET                                 ((0x24)<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_BOOTSETTING                            (0x25<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_PREVALIDATE                       			(0x26<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_IMAGE_DATA                        			(0x27<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_IMAGE_WRITE                       			(0x28<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_POSTVALIDATE                      			(0x29<<8)
#define BLE_UART_CCOMMAND_BOOTLOADER_RESET_N_ACTIVATE                  			(0x2A<<8)

#define BLE_UART_CCOMMAND_APPLICATION_BLE_CONNECT                         ((0x31)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_BLE_DISCONNECT                      ((0x32)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_BLE_BOND                            ((0x33)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_BLE_PASSKEY                         ((0x34)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_DEV_NAME_UPDATE                     ((0x35)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_CURRENT_TIME_WRITE                  ((0x36)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_WEATHER_INFO_REQUEST                ((0x37)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_PERSONAL_INFO_READ                  ((0x38)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_PERSONAL_INFO_WRITE                 ((0x39)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_SPORTS_PARAM_READ                   ((0x3A)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_SPORTS_PARAM_WRITE                  ((0x3B)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APP_MESSAGE_IOS                     ((0x3C)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APP_MESSAGE_ANDROID                 ((0x3D)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_WECHAT_SPORT_DATA_REQUEST           ((0x3E)<<8)

#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST                      ((0x41)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_MAIN_DAY_DATA_REQUEST                   ((0x42)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST                ((0x43)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_REQUEST       ((0x44)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_ADDRESS_REQUEST             ((0x48)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_HEADER_WRITE                ((0x49)<<8)


#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID                			((0x51)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST        			((0x52)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST       			((0x53)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID         			((0x54)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST      ((0x55)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST     ((0x56)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_NOTIFY_SWITCH                    			((0x57)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST       			((0x58)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST     ((0x59)<<8)



#define BLE_UART_CCOMMAND_APPLICATION_SCAN_ENABLE                         ((0x61)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_SCAN_DISABLE                        ((0x62)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_HEARTRATE_CONNECT         ((0x63)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_HEARTRATE_DISCONNECT      ((0x64)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_RUNNING_CONNECT           ((0x65)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_RUNNING_DISCONNECT        ((0x66)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_CYCLING_CONNECT           ((0x67)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_CYCLING_DISCONNECT        ((0x68)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_ACCESSORY_DATA_UPLOAD               ((0x69)<<8)


#define BLE_UART_CCOMMAND_APPLICATION_FIND_PHONE                          ((0x71)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_CALCULATE_PARAM_WRITE     ((0x72)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_CALCULATE_PARAM_READ      ((0x73)<<8)


#define BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO                      ((0x74)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_INFO_2                    ((0x75)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_FINISH                    ((0x76)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_WATCH_FACE_ACTIVATE                  ((0x77)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_FIND_WATCH                           ((0x79)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_GET_WATCH_DEFAULT_SPORT_MODE         ((0x7A)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_SEND_GPS_TEST_DATA         		   ((0x7B)<<8)



#define BLE_UART_CCOMMAND_APPLICATION_NOTIFY_MSG_ACTION_PERFORM           ((0x01)<<8)//����֪ͨ�������߹Ҷ�



/*
��Ӧ
*/

#define BLE_UART_RESPONSE_APPLICATION_INFO_APOLLO2                        ((0x91)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APOLLO2_JUMP_BOOTLOADER             ((0x92)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APOLLO2_JUMP_APPLICATION            ((0x93)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APOLLO2_JUMP_UPDATE_ALGORITHM       ((0x94)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APOLLO2_RESET                       ((0x95)<<8)
#ifdef COD 
#define BLE_UART_RESPONSE_APPLICATION_INFO_BAT                        ((0x96)<<8)
#define BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_READ               ((0x97)<<8)
#define BLE_UART_RESPONSE_APPLICATION_LOCATION_INFO              	  ((0x98)<<8)
#define BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_STATUS              	  ((0x99)<<8)
#define BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_DELAY              	  ((0x9A)<<8)
#define BLE_UART_RESPONSE_APPLICATION_PHONE_CALL_CTRL             	      ((0x9B)<<8)
#define BLE_UART_RESPONSE_APPLICATION_ON_BIND_DEVICE             	      ((0x9C)<<8)
#define BLE_UART_RESPONSE_APPLICATION_SEND_BIND_STATUS              	  ((0x9D)<<8)
#define BLE_UART_RESPONSE_APPLICATION_CMD_CAST                    		  ((0x9E)<<8)
#define BLE_UART_RESPONSE_APPLICATION_RT_TODAY                    		  ((0x9F)<<8)
#define BLE_UART_RESPONSE_APPLICATION_RT_HEART                    		  ((0xA0)<<8)
#define BLE_UART_RESPONSE_APPLICATION_SPORT_CTRL                    	  ((0xAB)<<8)
#define BLE_UART_RESPONSE_APPLICATION_SENSOR_DATA_TRAINS                  ((0xAC)<<8)
#define BLE_UART_RESPONSE_APPLICATION_REST_HEARTRATE                 	  ((0xAD)<<8)
#define BLE_UART_RESPONSE_APPLICATION_DATA_FRAME_NUM                 	  ((0xAE)<<8)
#define BLE_UART_RESPONSE_APPLICATION_DELETE_DATA                 	  	  ((0xAF)<<8)
#define BLE_UART_RESPONSE_APPLICATION_MAC_READ                	  		  ((0xCA)<<8)
#define BLE_UART_RESPONSE_APPLICATION_MAC_WRITE                	  	  	  ((0xCB)<<8)
#define BLE_UART_RESPONSE_APPLICATION_UPDATA_BP_START                 	  ((0xCC)<<8)
#define BLE_UART_RESPONSE_APPLICATION_REAL_TIME_LOG_TRAINS                ((0xCD)<<8)
#endif
#define BLE_UART_RESPONSE_APPLICATION_BLE_INFO                              ((0xA1)<<8)
#define BLE_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER                       ((0xA2)<<8)
#define BLE_UART_RESPONSE_APPLICATION_JUMP_APPLICATION                      ((0xA3)<<8)
#define BLE_UART_RESPONSE_APPLICATION_RESET                                 ((0xA4)<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_BOOTSETTING                       			(0xA5<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_PREVALIDATE                       			(0xA6<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_IMAGE_DATA                        			(0xA7<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_IMAGE_WRITE                       			(0xA8<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_POSTVALIDATE                      			(0xA9<<8)
#define BLE_UART_RESPONSE_BOOTLOADER_RESET_N_ACTIVATE                  			(0xAA<<8)
#define BLE_UART_RESPONSE_APPLICATION_BLE_CONNECT                         ((0xB1)<<8)
#define BLE_UART_RESPONSE_APPLICATION_BLE_DISCONNECT                      ((0xB2)<<8)
#define BLE_UART_RESPONSE_APPLICATION_BLE_BOND                            ((0xB3)<<8)
#define BLE_UART_RESPONSE_APPLICATION_BLE_PASSKEY                         ((0xB4)<<8)
#define BLE_UART_RESPONSE_APPLICATION_BLE_DEV_NAME_UPDATE                 ((0xB5)<<8)
#define BLE_UART_RESPONSE_APPLICATION_CURRENT_TIME_WRITE                  ((0xB6)<<8)
#define BLE_UART_RESPONSE_APPLICATION_WEATHER_INFO_REQUEST                ((0xB7)<<8)
#define BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_READ                  ((0xB8)<<8)
#define BLE_UART_RESPONSE_APPLICATION_PERSONAL_INFO_WRITE                 ((0xB9)<<8)
#define BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_READ                   ((0xBA)<<8)
#define BLE_UART_RESPONSE_APPLICATION_SPORTS_PARAM_WRITE                  ((0xBB)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APP_MESSAGE_IOS                     ((0xBC)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APP_MESSAGE_ANDROID                 ((0xBD)<<8)
#define BLE_UART_RESPONSE_APPLICATION_WECHAT_SPORT_DATA_REQUEST           ((0xBE)<<8)

#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ASTRO_SYNC_REQUEST             ((0xC1)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_MAIN_DAY_DATA_REQUEST          ((0xC2)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_REQUEST          ((0xC3)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_ACTIVITY_ADDRESS_BOUNDARY_REQUEST       ((0xC4)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_ADDRESS_REQUEST             ((0xC8)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_SHARD_TRACK_HEADER_WRITE                ((0xC9)<<8)

#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_REQUEST_UUID                			((0xD1)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_READ_ADDRESS_REQUEST        			((0xD2)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_WRITE_ADDRESS_REQUEST       			((0xD3)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_REQUEST_UUID         			((0xD4)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_READ_ADDRESS_REQUEST      ((0xD5)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_WRITE_ADDRESS_REQUEST     ((0xD6)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_NOTIFY_SWITCH                    			((0xD7)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_ERASE_ADDRESS_REQUEST       			((0xD8)<<8)
#define BLE_UART_RESPONSE_APPLICATION_EXTFLASH_CONFIG_TRAIN_PLAN_RESULT_ERASE_ADDRESS_REQUEST     ((0xD9)<<8)

#define BLE_UART_RESPONSE_APPLICATION_SCAN_ENABLE                         ((0xE1)<<8)
#define BLE_UART_RESPONSE_APPLICATION_SCAN_DISABLE                        ((0xE2)<<8)
#define BLE_UART_RESPONSE_APPLICATION_ACCESSORY_HEARTRATE_CONNECT         ((0xE3)<<8)
#define BLE_UART_RESPONSE_APPLICATION_ACCESSORY_HEARTRATE_DISCONNECT      ((0xE4)<<8)
#define BLE_UART_RESPONSE_APPLICATION_ACCESSORY_RUNNING_CONNECT           ((0xE5)<<8)
#define BLE_UART_RESPONSE_APPLICATION_ACCESSORY_RUNNING_DISCONNECT        ((0xE6)<<8)
#define BLE_UART_RESPONSE_APPLICATION_ACCESSORY_CYCLING_CONNECT           ((0xE7)<<8)
#define BLE_UART_RESPONSE_APPLICATION_ACCESSORY_CYCLING_DISCONNECT        ((0xE8)<<8)
#define BLE_UART_RESPONSE_APPLICATION_ACCESSORY_DATA_UPLOAD               ((0xE9)<<8)

#define BLE_UART_RESPONSE_APPLICATION_FIND_PHONE                          ((0xF1)<<8)
#define BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_WRITE     ((0xF2)<<8)
#define BLE_UART_RESPONSE_APPLICATION_GET_WATCH_CALCULATE_PARAM_READ      ((0xF3)<<8)


#define BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO                      ((0xF4)<<8)
#define BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_INFO_2                    ((0xF5)<<8)
#define BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_FINISH                    ((0xF6)<<8)
#define BLE_UART_RESPONSE_APPLICATION_WATCH_FACE_ACTIVATE                  ((0xF7)<<8)

#define BLE_UART_RESPONSE_APPLICATION_FIND_WATCH                           ((0xF9)<<8)
#define BLE_UART_RESPONSE_APPLICATION_GET_WATCH_DEFAULT_SPORT_MODE         ((0xFA)<<8)
#define BLE_UART_RESPONSE_APPLICATION_SEND_GPS_TES_DATA                    ((0xFB)<<8)


#define BLE_UART_RESPONSE_APPLICATION_NOTIFY_MSG_ACTION_PERFORM           ((0x81)<<8)



#define BLE_UART_CCOMMAND_MASK_APP                   ((1<<BLE_UART_CCOMMAND_MASK_APP_POSITION)<<8|(UART_PACKET_HEADER))



#define BLE_UART_CCOMMAND_APP_BLE_DFU_START                          ((0x01)<<8)
#define BLE_UART_CCOMMAND_APP_BLE_DFU_FINISH                         ((0x02)<<8)
#define BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_BOOTSETTING             ((0x03)<<8)
#define BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_PREVALIDATE             ((0x04)<<8)
#define BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_POSTVALIDATE            ((0x05)<<8)
#define BLE_UART_CCOMMAND_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE        ((0x06)<<8)

#define BLE_UART_RESPONSE_APP_BLE_DFU_START                          ((0x81)<<8)
#define BLE_UART_RESPONSE_APP_BLE_DFU_FINISH                         ((0x82)<<8)
#define BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_BOOTSETTING             ((0x83)<<8)
#define BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_PREVALIDATE             ((0x84)<<8)
#define BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_POSTVALIDATE            ((0x85)<<8)
#define BLE_UART_RESPONSE_APP_BLE_DFU_APOLLO_RESET_N_ACTIVATE        ((0x86)<<8)



#define FIND_PHONE_PARAM_START       0x00000001
#define FIND_PHONE_PARAM_END         0x00000002
#define FIND_PHONE_PARAM_FOUND       0x00000003

#define FIND_WATCH_PARAM_START       0x00000001
#define FIND_WATCH_PARAM_END         0x00000002
#define FIND_WATCH_PARAM_FOUND       0x00000003



#endif //__CMD_BLE_H__