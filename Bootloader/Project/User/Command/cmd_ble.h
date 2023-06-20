#ifndef __CMD_BLE_H__
#define __CMD_BLE_H__


#define UART_PACKET_HEADER				(0xA5)


#define BLE_UART_CCOMMAND_MASK_BOOTLOADER                                  ((1<<8)<<8|(UART_PACKET_HEADER))

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




#define BLE_UART_CCOMMAND_MASK_APPLICATION                                 ((1<<9)<<8|(UART_PACKET_HEADER))


#define BLE_UART_CCOMMAND_APPLICATION_UNSPPORT       		            			 ((0xFF)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APP_ERROR      		            			 ((0xFE)<<8)

#define BLE_UART_CCOMMAND_APPLICATION_INFO_APOLLO2                        ((0x11)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APOLLO2_JUMP_BOOTLOADER             ((0x12)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APOLLO2_JUMP_APPLICATION            ((0x13)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APOLLO2_JUMP_UPDATE_ALGORITHM       ((0x14)<<8)
#define BLE_UART_CCOMMAND_APPLICATION_APOLLO2_RESET                       ((0x15)<<8)
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



#define BLE_UART_RESPONSE_APPLICATION_INFO_APOLLO2                        ((0x91)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APOLLO2_JUMP_BOOTLOADER             ((0x92)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APOLLO2_JUMP_APPLICATION            ((0x93)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APOLLO2_JUMP_UPDATE_ALGORITHM       ((0x94)<<8)
#define BLE_UART_RESPONSE_APPLICATION_APOLLO2_RESET                       ((0x95)<<8)
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


#endif //__CMD_BLE_H__
