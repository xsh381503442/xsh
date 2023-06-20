#ifndef __CMD_PC_H__
#define __CMD_PC_H__


#define UART_PACKET_HEADER				(0xA5)


#define PC_UART_CCOMMAND_MASK_BOOTLOADER      ((1<<8)<<8|(UART_PACKET_HEADER))


#define PC_UART_CCOMMAND_BOOTLOADER_UNSPPORT       					(0xFF<<8)

#define PC_UART_CCOMMAND_BOOTLOADER_BOOT_SETTING      					(0x01<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_PREVALIDATE     						(0x02<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_IMAGE_DATA      						(0x03<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_IMAGE_WRITE     						(0x04<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_POSTVALIDATE      					(0x05<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_RESET_N_ACTIVATE  					(0x06<<8)

#define PC_UART_RESPONSE_BOOTLOADER_BOOT_SETTING      					(0x81<<8)
#define PC_UART_RESPONSE_BOOTLOADER_PREVALIDATE     						(0x82<<8)
#define PC_UART_RESPONSE_BOOTLOADER_IMAGE_DATA      						(0x83<<8)
#define PC_UART_RESPONSE_BOOTLOADER_IMAGE_WRITE     						(0x84<<8)
#define PC_UART_RESPONSE_BOOTLOADER_POSTVALIDATE      					(0x85<<8)
#define PC_UART_RESPONSE_BOOTLOADER_RESET_N_ACTIVATE  					(0x86<<8)


#define PC_UART_CCOMMAND_BOOTLOADER_INFO_APOLLO2               (0x11<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_JUMP_BOOTLOADER            (0x12<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_JUMP_APPLICATION           (0x13<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_JUMP_UPDATE_ALGORITHM      (0x14<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_RESET                      (0x15<<8)


#define PC_UART_RESPONSE_BOOTLOADER_INFO_APOLLO2               (0x91<<8)
#define PC_UART_RESPONSE_BOOTLOADER_JUMP_BOOTLOADER            (0x92<<8)
#define PC_UART_RESPONSE_BOOTLOADER_JUMP_APPLICATION           (0x93<<8)
#define PC_UART_RESPONSE_BOOTLOADER_JUMP_UPDATE_ALGORITHM      (0x94<<8)
#define PC_UART_RESPONSE_BOOTLOADER_RESET                      (0x95<<8)





#define PC_UART_CCOMMAND_BOOTLOADER_BLE_INFO                             (0x21<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_BOOTLOADER                  (0x22<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_JUMP_APPLICATION                 (0x23<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET                            (0x24<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_BOOTSETTING                      (0x25<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_PREVALIDATE                      (0x26<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_IMAGE_DATA                       (0x27<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_IMAGE_WRITE                      (0x28<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_POSTVALIDATE                     (0x29<<8)
#define PC_UART_CCOMMAND_BOOTLOADER_BLE_RESET_N_ACTIVATE                 (0x2A<<8)


#define PC_UART_RESPONSE_BOOTLOADER_BLE_INFO                             (0xA1<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_JUMP_BOOTLOADER                  (0xA2<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_JUMP_APPLICATION                 (0xA3<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_RESET                            (0xA4<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_BOOTSETTING                      (0xA5<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_PREVALIDATE                      (0xA6<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_IMAGE_DATA                       (0xA7<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_IMAGE_WRITE                      (0xA8<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_POSTVALIDATE                     (0xA9<<8)
#define PC_UART_RESPONSE_BOOTLOADER_BLE_RESET_N_ACTIVATE                 (0xAA<<8)








#define PC_UART_COMMAND_MASK_APPLICATION               ((1<<9)<<8|(UART_PACKET_HEADER))

#define PC_UART_CCOMMAND_APPLICATION_UNSPPORT       					(0xFF<<8)


#define PC_UART_CCOMMAND_APPLICATION_INFO_APOLLO2               (0x11<<8)
#define PC_UART_CCOMMAND_APPLICATION_JUMP_BOOTLOADER            (0x12<<8)
#define PC_UART_CCOMMAND_APPLICATION_JUMP_APPLICATION           (0x13<<8)
#define PC_UART_CCOMMAND_APPLICATION_JUMP_UPDATE_ALGORITHM      (0x14<<8)
#define PC_UART_CCOMMAND_APPLICATION_RESET                      (0x15<<8)





#define PC_UART_RESPONSE_APPLICATION_INFO_APOLLO2               (0x91<<8)
#define PC_UART_RESPONSE_APPLICATION_JUMP_BOOTLOADER            (0x92<<8)
#define PC_UART_RESPONSE_APPLICATION_JUMP_APPLICATION           (0x93<<8)
#define PC_UART_RESPONSE_APPLICATION_JUMP_UPDATE_ALGORITHM      (0x94<<8)
#define PC_UART_RESPONSE_APPLICATION_RESET                      (0x95<<8)




#define PC_UART_CCOMMAND_APPLICATION_BLE_INFO                             (0x21<<8)
#define PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_BOOTLOADER                  (0x22<<8)
#define PC_UART_CCOMMAND_APPLICATION_BLE_JUMP_APPLICATION                 (0x23<<8)
#define PC_UART_CCOMMAND_APPLICATION_BLE_RESET                            (0x24<<8)


#define PC_UART_RESPONSE_APPLICATION_BLE_INFO                             (0xA1<<8)
#define PC_UART_RESPONSE_APPLICATION_BLE_JUMP_BOOTLOADER                  (0xA2<<8)
#define PC_UART_RESPONSE_APPLICATION_BLE_JUMP_APPLICATION                 (0xA3<<8)
#define PC_UART_RESPONSE_APPLICATION_BLE_RESET                            (0xA4<<8)




#define PC_UART_COMMAND_MASK_UPDATE_ALGORITHM           ((1<<10)<<8|(UART_PACKET_HEADER))

#define PC_UART_CCOMMAND_UPDATE_ALGORITHM_UNSPPORT       					(0xFF<<8)

#define PC_UART_RESPONSE_UPDATE_ALGORITHM_BOOTLOADER_CRC_ERROR        (0x81<<8)
#define PC_UART_RESPONSE_UPDATE_ALGORITHM_APPLICATION_CRC_ERROR       (0x82<<8)
#define PC_UART_RESPONSE_UPDATE_ALGORITHM_BOOTLOADER_SUCCESS          (0x83<<8)
#define PC_UART_RESPONSE_UPDATE_ALGORITHM_APPLICATION_SUCCESS         (0x84<<8)




#define PC_UART_CCOMMAND_UPDATE_ALGORITHM_INFO_APOLLO2               (0x11<<8)
#define PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_BOOTLOADER            (0x12<<8)
#define PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_APPLICATION           (0x13<<8)
#define PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_UPDATE_ALGORITHM      (0x14<<8)
#define PC_UART_CCOMMAND_UPDATE_ALGORITHM_JUMP_RESET                 (0x15<<8)

#define PC_UART_RESPONSE_UPDATE_ALGORITHM_INFO_APOLLO2               (0x91<<8)
#define PC_UART_RESPONSE_UPDATE_ALGORITHM_JUMP_BOOTLOADER            (0x92<<8)
#define PC_UART_RESPONSE_UPDATE_ALGORITHM_JUMP_APPLICATION           (0x93<<8)
#define PC_UART_RESPONSE_UPDATE_ALGORITHM_JUMP_UPDATE_ALGORITHM      (0x94<<8)
#define PC_UART_RESPONSE_UPDATE_ALGORITHM_JUMP_RESET                 (0x95<<8)

















#endif //__CMD_PC_H__