//*****************************************************************************
//
//! @file am_bootloader.h
//!
//! @brief
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2017, Ambiq Micro
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
// This is part of revision 1.2.8 of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>

#ifndef AM_BOOTLOADER_H
#define AM_BOOTLOADER_H

//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#define AM_BOOTLOADER_DISABLE_OVERRIDE_PIN      (0xFFFFFFFF)
#define AM_BOOTLOADER_OVERRIDE_HIGH             (0x1)
#define AM_BOOTLOADER_OVERRIDE_LOW              (0x0)


#ifdef __cplusplus
extern "C"
{
#endif


//*****************************************************************************
//
// External function declarations.
//
//*****************************************************************************
extern uint32_t am_bootloader_crc32(const void *pvData, uint32_t ui32Length);
extern uint32_t am_bootloader_fast_crc32(const void *pvData, uint32_t ui32NumBytes);
extern void am_bootloader_partial_crc32(const void *pvData, uint32_t ui32NumBytes, uint32_t *pui32CRC);

bool am_bootloader_check_crc_from_external_flash(uint32_t input_crc,uint32_t external_flash_address ,uint32_t size);
#ifdef COD 
bool cod_check_crc_from_external_flash(uint32_t input_crc,uint32_t external_flash_address ,uint32_t size,uint32_t *output_crc);	
#endif
#ifdef __cplusplus
}
#endif

#endif // AM_BOOTLOADER_H
