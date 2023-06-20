#ifndef __LIB_BOOT_H__
#define __LIB_BOOT_H__



#include "lib_boot_setting.h"
#include "am_bootloader.h"




void lib_boot_goto_image(am_bootloader_image_t *image);
bool lib_boot_jump_to_bootloader(uint32_t reason);
bool lib_boot_jump_to_update_algorithm(void);



#endif //__LIB_BOOT_H__

