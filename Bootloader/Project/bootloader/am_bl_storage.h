//*****************************************************************************
//
//! @file am_bl_storage.h
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

#ifndef AM_BL_STORAGE_H
#define AM_BL_STORAGE_H

//*****************************************************************************
//
// Storage image status.
//
//*****************************************************************************
#define BOOT_NEW_IMAGE_INTERNAL_FLASH   0x00000001
#define BOOT_NEW_IMAGE_EXTERNAL_FLASH   0x00000002
#define BOOT_NO_NEW_IMAGE               0xFFFFFFFF

//*****************************************************************************
//
// Extended version of the bootloader flag page.
//
// This structure replaces the standard flag page.
//
//*****************************************************************************
typedef struct
{
    // Starting address where the image was linked to run.
    uint32_t *pui32LinkAddress;

    // Length of the executable image in bytes.
    uint32_t ui32NumBytes;

    // CRC-32 Value for the full image.
    uint32_t ui32CRC;

    // Override GPIO number. (Can be used to force a new image load)
    uint32_t ui32OverrideGPIO;

    // Polarity for the override pin.
    uint32_t ui32OverridePolarity;

    // Stack pointer location.
    uint32_t *pui32StackPointer;

    // Reset vector location.
    uint32_t *pui32ResetVector;

    // Is the image encrypted?
    bool bEncrypted;

    // Boot Options.
    uint32_t ui32Options;

    // Version Informatin of the Current Image
    uint32_t ui32Version;

    // Version Informatin of the New Image
    // Only used to confirm the image being received.
    uint32_t ui32VersionNewImage;

    // Starting address where the new image was stored
    uint32_t *pui32StorageAddressNewImage;

    // Length of the new image being received in bytes
    // Only used to confirm the image being received.
    uint32_t ui32TotalNumBytesNewImage;

    // Bytes already received and stored
    uint32_t ui32StoredNumBytesNewImage;

    // CRC-32 Value for the new image being received
    // Only used to confirm the image being received.
    uint32_t ui32CRCNewImage;
}
am_bl_storage_image_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

extern uint32_t am_bl_storage_flag_page_update(am_bl_storage_image_t *psImage, uint32_t *pui32FlagPage);
extern bool image_get_storage_information_internal(am_bl_storage_image_t *psImage,
                                                   uint32_t ui32NumBytesNewImage,
                                                   uint32_t* pui32StorageAddressNewImage,
                                                   uint32_t* pui32NumBytesSpaceLeft);

#endif // AM_BL_STORAGE_H
