#ifndef __CMD_GPREGRET_H__
#define __CMD_GPREGRET_H__






#define CMD_GPREGRET_BOOTLOADER_PC_DFU                                  (0x01)
#define CMD_GPREGRET_BOOTLOADER_BLE_DFU                                 (0x02)
#define CMD_GPREGRET_BOOTLOADER_BLE_RESET                               (0x03)
#define CMD_GPREGRET_BOOTLOADER_PIN_RESET                               (0x04)
#define CMD_GPREGRET_APPLICATION_APP_ERROR_RESET                        (0x05)
#define CMD_GPREGRET_BOOTLOADER_EXCEPTION_RESET                         (0x06)//在遇到异常时：BOOTLOADER COPY错误，APPLICATION IMAGE CRC错误，重启后直接进入PC串口升级
#define CMD_GPREGRET_BOOTLOADER_BLE_DFU_FINISH                          (0x07)

#define BOOTLOADER_DFU_START                                            (0xB1) 



#endif //__CMD_GPREGRET_H__
