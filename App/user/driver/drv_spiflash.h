//*****************************************************************************
//
//! @file SPIFLASH.h
//!
//! @brief Generic spiflash driver.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2015, Ambiq Micro
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision 1.0.1 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef _DRV_SPIFLASH_H__
#define _DRV_SPIFLASH_H__

#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif


//*****************************************************************************
//
// Global definitions for flash commands
//
//*****************************************************************************
#define SPIFLASH_WREN		0x06        // Write enable
#define SPIFLASH_WRDI		0x04        // Write disable
#define SPIFLASH_RDID		0x9F        // Read Identification
#define SPIFLASH_RDRSR		0x05        // Read status register
#define SPIFLASH_WRSR		0x01        // Write status register
#define SPIFLASH_READ		0x03        // Read data bytes
#define SPIFLASH_PP   		0x02        // Page program
#define SPIFLASH_SE 		0x20        // Sector Erase
#define SPIFLASH_CE			0xC7        // Chip Erase
#define SPIFLASH_BE64K		0xD8		// 64KB Block Erase 
#define SPIFLASH_BE32K		0x52		// 32KB Block Erase
#define SPIFLASH_DP			0xB9		// Deep Power-Down
#define SPIFLASH_RDP		0xAB		// Release from Deep Power-Down

//*****************************************************************************
//
// Global definitions for the flash status register
//
//*****************************************************************************
#define SPIFLASH_WEL		0x02        // Write enable latch
#define SPIFLASH_WIP		0x01        // Write in progress

#define	SPIFLASH_ID			0x176001

#define	SPIFLASH_PageSize	0x100

//*****************************************************************************
//
// Function pointers for SPI write and read.
//
//*****************************************************************************
typedef bool (*spiflash_write_t)(uint32_t ui32Module,
                                            uint32_t ui32ChipSelect,
                                            uint32_t *pui32Data,
                                            uint32_t ui32NumBytes,
                                            uint32_t ui32Options);

typedef bool (*spiflash_read_t)(uint32_t ui32Module,
                                           uint32_t ui32ChipSelect,
                                           uint32_t *pui32Data,
                                           uint32_t ui32NumBytes,
                                           uint32_t ui32Options);

//*****************************************************************************
//
// Device structure used for communication.
//
//*****************************************************************************
typedef struct
{
    //
    // Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    // Chip Select number to use for IOM access.
    //
    uint32_t ui32ChipSelect;
}
spiflash_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void drv_spiflash_enable(void);

extern uint8_t spiflash_status(void);

extern uint32_t drv_spiflash_id_get(void);

extern void drv_spiflash_read(uint8_t *pui8RxBuffer,
                                     uint32_t ui32ReadAddress,
                                     uint32_t ui32NumBytes);

extern void drv_spiflash_pagewrite(uint8_t *ui8TxBuffer,
                                      uint32_t ui32WriteAddress,
                                      uint32_t ui32NumBytes);

extern void drv_spiflash_bufferwrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);

extern void drv_spiflash_chip_erase(void);

extern void drv_spiflash_sector_erase(uint32_t ui32SectorAddress);

extern void drv_spiflash_32k_erase(uint32_t ui32SectorAddress);

extern void drv_spiflash_64k_erase(uint32_t ui32SectorAddress);

extern void drv_spiflash_powerdown(void);

extern void spiflash_release_powerdown(void);

extern void drv_spiflash_disable(void);



#ifdef __cplusplus
}
#endif

#endif // SPIFLASH_H

