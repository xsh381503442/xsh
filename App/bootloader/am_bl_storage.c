//*****************************************************************************
//
//! @file am_bl_storage.c
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
//
#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bl_storage.h"

//*****************************************************************************
//
// Update the extended flag page used in storage-based bootloaders.
//
//*****************************************************************************
uint32_t
am_bl_storage_flag_page_update(am_bl_storage_image_t *psImage,
                               uint32_t *pui32FlagPage)
{
    uint32_t ui32Block, ui32Page;
    uint32_t ui32Critical;

    //
    // Calculate the correct flag page number.
    //
    ui32Page = AM_HAL_FLASH_ADDR2PAGE((uintptr_t)pui32FlagPage);
    ui32Block = AM_HAL_FLASH_ADDR2INST((uint32_t)pui32FlagPage);

    //
    // Start a critical section.
    //
    ui32Critical = am_hal_interrupt_master_disable();
    //
    // Erase the page.
    //
    int rc = am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY, ui32Block, ui32Page);

    //
    // Write the psImage structure directly to the flag page.
    //
    rc = am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
                              (uint32_t *) psImage,
                              pui32FlagPage,
                              sizeof(am_bl_storage_image_t) / 4);
    //
    // Exit the critical section.
    //
    am_hal_interrupt_master_set(ui32Critical);
    return rc;
}


//*****************************************************************************
//
//! @brief Checks where to store the received new image in internal flash
//!
//! @param[in] psImage is the pointer that points to the flash flag page.
//! @param[in] ui32NumBytesNewImage is the length of the new image in bytes.
//! @param[out] pui32StorageAddressNewImage the available storage start address.
//! @param[out] pui32NumBytesSpaceLeft is space left in internal flash (in bytes).
//!
//! This function reads the flash flag page informatin to determine the available
//! storage start address and space left in the internal flash. It compares the 
//! space left in the internal flash with the new image length and returns whether
//! there is enough space left to store the image.
//! The last page of the internal flash is reserved for future usage, therefore
//! not counted in the space left.
//!
//! @return true if there is enough space to store the new image.
//!         false if there is not.
//
//*****************************************************************************
bool
image_get_storage_information_internal(am_bl_storage_image_t *psImage, 
                                        uint32_t ui32NumBytesNewImage,
                                        uint32_t* pui32StorageAddressNewImage,
                                        uint32_t* pui32NumBytesSpaceLeft)
{

    uint32_t ui32SpaceLeft;
    //
    // Read device information to determine total flash available
    //
    am_hal_mcuctrl_device_t device;
    am_hal_mcuctrl_device_info_get(&device);

    //
    // Calculate space left in internal flash according to flag page inforamtion.
    // Last page of the internal flash is reserved.
    // (effective LinkAddress shall be equal to or larger than 0x4000)
    //
    if((device.ui32FlashSize < ((uint32_t)(psImage->pui32LinkAddress) 
        + (uint32_t)(psImage->ui32NumBytes) + AM_HAL_FLASH_PAGE_SIZE)) ||
    		((uint32_t)(psImage->pui32LinkAddress) > device.ui32FlashSize) ||
			((uint32_t)(psImage->ui32NumBytes) > device.ui32FlashSize))
    {
        //image size error, or flash flag page info error
        *pui32StorageAddressNewImage = 0xFFFFFFFF; 
        *pui32NumBytesSpaceLeft = 0xFFFFFFFF;
        return false;
    }
    ui32SpaceLeft = device.ui32FlashSize - (uint32_t)(psImage->pui32LinkAddress) 
                    - (uint32_t)(psImage->ui32NumBytes) - AM_HAL_FLASH_PAGE_SIZE;
#if defined (keil)
    ui32SpaceLeft &= 0xFFFFFFFF << (31 - __clz(AM_HAL_FLASH_PAGE_SIZE));
#else
    ui32SpaceLeft &= (0xFFFFFFFF - AM_HAL_FLASH_PAGE_SIZE + 1);   //storage starts from page boundaries
#endif //__arm__
#if USE_LAST_PAGE_FOR_FLAG
    if(ui32SpaceLeft > AM_HAL_FLASH_PAGE_SIZE)
    {
        ui32SpaceLeft -= AM_HAL_FLASH_PAGE_SIZE;
    }
    else
    {
        //
        // Not enought space left in the internal flash to store the new image
        //
        *pui32NumBytesSpaceLeft = ui32SpaceLeft;
        *pui32StorageAddressNewImage = 0xFFFFFFFF;  
        return false;
    }
#endif 

    if(ui32NumBytesNewImage > ui32SpaceLeft) 
    {
        //
        // Not enought space left in the internal flash to store the new image
        //
        *pui32NumBytesSpaceLeft = ui32SpaceLeft;
        *pui32StorageAddressNewImage = 0xFFFFFFFF;  
        return false;
    }
    else
    {
        //
        // Find the right address in internal flash to store the new image.
        // The starting address of the first available flash page
        //
        *pui32NumBytesSpaceLeft = ui32SpaceLeft;
        *pui32StorageAddressNewImage = (((uint32_t)(psImage->pui32LinkAddress) + (uint32_t)(psImage->ui32NumBytes)) |
                                        (AM_HAL_FLASH_PAGE_SIZE - 1)) + 1;
        return true;
    }
}
