#ifndef BSP_IOM_H
#define	BSP_IOM_H

#include <stdint.h>
#include "bsp_config.h"

void bsp_iom_enable(uint32_t ui32Module,am_hal_iom_config_t iomConfig);
void bsp_iom_disable(uint32_t ui32Module);
#endif
