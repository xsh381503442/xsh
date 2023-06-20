//*****************************************************************************
//
//! @file am_bootloader.c
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
#include "am_bootloader.h"



#ifdef BOOTLOADER_DEBUG
#include "am_util_stdio.h"
#include "am_util_delay.h"
// might need va_list someday, but not so far today.
#define DPRINTF(x) am_util_stdio_printf x
#else
#define DPRINTF(x)
#endif



//*****************************************************************************
//
// Forward declarations.
//
//*****************************************************************************
#ifdef __GNUC__
void __attribute__((naked)) am_bootloader_encrypted_image_run(am_bootloader_image_t *psImage);
void __attribute__((naked)) am_bootloader_clear_image_run(am_bootloader_image_t *psImage);
#endif

#ifdef keil
__asm void am_bootloader_encrypted_image_run(am_bootloader_image_t *psImage);
__asm void am_bootloader_clear_image_run(am_bootloader_image_t *psImage);
#endif

#ifdef iar
void am_bootloader_encrypted_image_run(am_bootloader_image_t *psImage);
void am_bootloader_clear_image_run(am_bootloader_image_t *psImage);
#endif

//*****************************************************************************
//
// CRC-32 table
// Polynomial = 0x1EDC6F41 (also listed as CRC-32C or CRC-32/4)
//
// This polynomial should catch all errors up to 4 bits for image sizes under
// about 255MB (which easily covers anything we can actually fit in flash), and
// it has a reasonably high probablility of catching bigger errors.
//
// See http://users.ece.cmu.edu/~koopman/crc for more information.
//
//*****************************************************************************
#define CRC32_POLYNOMIAL                    0x1EDC6F41
const static uint32_t g_pui32CRC32Table[256] =
{
    0x00000000, 0x1EDC6F41, 0x3DB8DE82, 0x2364B1C3,
    0x7B71BD04, 0x65ADD245, 0x46C96386, 0x58150CC7,
    0xF6E37A08, 0xE83F1549, 0xCB5BA48A, 0xD587CBCB,
    0x8D92C70C, 0x934EA84D, 0xB02A198E, 0xAEF676CF,
    0xF31A9B51, 0xEDC6F410, 0xCEA245D3, 0xD07E2A92,
    0x886B2655, 0x96B74914, 0xB5D3F8D7, 0xAB0F9796,
    0x05F9E159, 0x1B258E18, 0x38413FDB, 0x269D509A,
    0x7E885C5D, 0x6054331C, 0x433082DF, 0x5DECED9E,
    0xF8E959E3, 0xE63536A2, 0xC5518761, 0xDB8DE820,
    0x8398E4E7, 0x9D448BA6, 0xBE203A65, 0xA0FC5524,
    0x0E0A23EB, 0x10D64CAA, 0x33B2FD69, 0x2D6E9228,
    0x757B9EEF, 0x6BA7F1AE, 0x48C3406D, 0x561F2F2C,
    0x0BF3C2B2, 0x152FADF3, 0x364B1C30, 0x28977371,
    0x70827FB6, 0x6E5E10F7, 0x4D3AA134, 0x53E6CE75,
    0xFD10B8BA, 0xE3CCD7FB, 0xC0A86638, 0xDE740979,
    0x866105BE, 0x98BD6AFF, 0xBBD9DB3C, 0xA505B47D,
    0xEF0EDC87, 0xF1D2B3C6, 0xD2B60205, 0xCC6A6D44,
    0x947F6183, 0x8AA30EC2, 0xA9C7BF01, 0xB71BD040,
    0x19EDA68F, 0x0731C9CE, 0x2455780D, 0x3A89174C,
    0x629C1B8B, 0x7C4074CA, 0x5F24C509, 0x41F8AA48,
    0x1C1447D6, 0x02C82897, 0x21AC9954, 0x3F70F615,
    0x6765FAD2, 0x79B99593, 0x5ADD2450, 0x44014B11,
    0xEAF73DDE, 0xF42B529F, 0xD74FE35C, 0xC9938C1D,
    0x918680DA, 0x8F5AEF9B, 0xAC3E5E58, 0xB2E23119,
    0x17E78564, 0x093BEA25, 0x2A5F5BE6, 0x348334A7,
    0x6C963860, 0x724A5721, 0x512EE6E2, 0x4FF289A3,
    0xE104FF6C, 0xFFD8902D, 0xDCBC21EE, 0xC2604EAF,
    0x9A754268, 0x84A92D29, 0xA7CD9CEA, 0xB911F3AB,
    0xE4FD1E35, 0xFA217174, 0xD945C0B7, 0xC799AFF6,
    0x9F8CA331, 0x8150CC70, 0xA2347DB3, 0xBCE812F2,
    0x121E643D, 0x0CC20B7C, 0x2FA6BABF, 0x317AD5FE,
    0x696FD939, 0x77B3B678, 0x54D707BB, 0x4A0B68FA,
    0xC0C1D64F, 0xDE1DB90E, 0xFD7908CD, 0xE3A5678C,
    0xBBB06B4B, 0xA56C040A, 0x8608B5C9, 0x98D4DA88,
    0x3622AC47, 0x28FEC306, 0x0B9A72C5, 0x15461D84,
    0x4D531143, 0x538F7E02, 0x70EBCFC1, 0x6E37A080,
    0x33DB4D1E, 0x2D07225F, 0x0E63939C, 0x10BFFCDD,
    0x48AAF01A, 0x56769F5B, 0x75122E98, 0x6BCE41D9,
    0xC5383716, 0xDBE45857, 0xF880E994, 0xE65C86D5,
    0xBE498A12, 0xA095E553, 0x83F15490, 0x9D2D3BD1,
    0x38288FAC, 0x26F4E0ED, 0x0590512E, 0x1B4C3E6F,
    0x435932A8, 0x5D855DE9, 0x7EE1EC2A, 0x603D836B,
    0xCECBF5A4, 0xD0179AE5, 0xF3732B26, 0xEDAF4467,
    0xB5BA48A0, 0xAB6627E1, 0x88029622, 0x96DEF963,
    0xCB3214FD, 0xD5EE7BBC, 0xF68ACA7F, 0xE856A53E,
    0xB043A9F9, 0xAE9FC6B8, 0x8DFB777B, 0x9327183A,
    0x3DD16EF5, 0x230D01B4, 0x0069B077, 0x1EB5DF36,
    0x46A0D3F1, 0x587CBCB0, 0x7B180D73, 0x65C46232,
    0x2FCF0AC8, 0x31136589, 0x1277D44A, 0x0CABBB0B,
    0x54BEB7CC, 0x4A62D88D, 0x6906694E, 0x77DA060F,
    0xD92C70C0, 0xC7F01F81, 0xE494AE42, 0xFA48C103,
    0xA25DCDC4, 0xBC81A285, 0x9FE51346, 0x81397C07,
    0xDCD59199, 0xC209FED8, 0xE16D4F1B, 0xFFB1205A,
    0xA7A42C9D, 0xB97843DC, 0x9A1CF21F, 0x84C09D5E,
    0x2A36EB91, 0x34EA84D0, 0x178E3513, 0x09525A52,
    0x51475695, 0x4F9B39D4, 0x6CFF8817, 0x7223E756,
    0xD726532B, 0xC9FA3C6A, 0xEA9E8DA9, 0xF442E2E8,
    0xAC57EE2F, 0xB28B816E, 0x91EF30AD, 0x8F335FEC,
    0x21C52923, 0x3F194662, 0x1C7DF7A1, 0x02A198E0,
    0x5AB49427, 0x4468FB66, 0x670C4AA5, 0x79D025E4,
    0x243CC87A, 0x3AE0A73B, 0x198416F8, 0x075879B9,
    0x5F4D757E, 0x41911A3F, 0x62F5ABFC, 0x7C29C4BD,
    0xD2DFB272, 0xCC03DD33, 0xEF676CF0, 0xF1BB03B1,
    0xA9AE0F76, 0xB7726037, 0x9416D1F4, 0x8ACABEB5
};

//*****************************************************************************
//
//! @brief CRC-32 implementation for the boot loader.
//!
//! @param pvData - Pointer to the data to be checked.
//! @param ui32NumBytes - Number of bytes to check.
//!
//! This function performs a CRC-32 on the input data and returns the 32-bit
//! result. This version does not use a table, so it has a smaller code
//! footprint.
//!
//! @return 32-bit CRC value.
//
//*****************************************************************************
uint32_t
am_bootloader_crc32(const void *pvData, uint32_t ui32NumBytes)
{
    uint32_t ui32CRC, i, j;
    uint8_t *pui8Data;

    ui32CRC = 0;
    pui8Data = (uint8_t *) pvData;

    for ( i = 0; i < ui32NumBytes; i++ )
    {
        ui32CRC ^= pui8Data[i] << 24;

        for ( j = 0; j < 8; j++ )
        {
            ui32CRC = (ui32CRC & 0x80000000 ?
                       ((ui32CRC << 1) ^ CRC32_POLYNOMIAL):
                       (ui32CRC << 1));
        }
    }

    return ui32CRC;
}

//*****************************************************************************
//
//! @brief Faster CRC-32 implementation for the boot loader.
//!
//! @param pvData - Pointer to the data to be checked.
//! @param ui32NumBytes - Number of bytes to check.
//!
//! This function performs a CRC-32 on the input data and returns the 32-bit
//! result. This version uses a 256-entry lookup table to speed up the
//! computation of the result.
//!
//! @return 32-bit CRC value.
//
//*****************************************************************************
uint32_t
am_bootloader_fast_crc32(const void *pvData, uint32_t ui32NumBytes)
{

    uint32_t ui32CRC, ui32CRCIndex, i;
    uint8_t *pui8Data;

    ui32CRC = 0;
    pui8Data = (uint8_t *) pvData;

    for (i = 0; i < ui32NumBytes; i++ )
    {			
        ui32CRCIndex = pui8Data[i] ^ (ui32CRC >> 24);
        ui32CRC = (ui32CRC << 8) ^ g_pui32CRC32Table[ui32CRCIndex];
    }
    return ui32CRC;
}

//*****************************************************************************
//
//! @brief CRC-32 implementation allowing multiple partial images.
//!
//! @param pvData - Pointer to the data to be checked.
//! @param ui32NumBytes - Number of bytes to check.
//! @param pui32CRC - Location to store the partial CRC32 result.
//!
//! This function performs a CRC-32 on the input data and returns the 32-bit
//! result. This version uses a 256-entry lookup table to speed up the
//! computation of the result. The result of the CRC32 is stored in the
//! location given by the caller. This allows the caller to keep a "running"
//! CRC for individual chunks of an image.
//!
//! @return 32-bit CRC value.
//
//*****************************************************************************
void
am_bootloader_partial_crc32(const void *pvData, uint32_t ui32NumBytes,
                            uint32_t *pui32CRC)
{
    uint32_t ui32CRCIndex, i;
    uint8_t *pui8Data;

    uint32_t ui32TempCRC = *pui32CRC;

    pui8Data = (uint8_t *) pvData;

    for ( i = 0; i < ui32NumBytes; i++ )
    {
        ui32CRCIndex = pui8Data[i] ^ (ui32TempCRC >> 24);
        ui32TempCRC = (ui32TempCRC << 8) ^ g_pui32CRC32Table[ui32CRCIndex];
    }

    *pui32CRC = ui32TempCRC;
}
#include "dev_extFlash.h"
bool am_bootloader_check_crc_from_external_flash(uint32_t input_crc,uint32_t external_flash_address ,uint32_t size)
{
	uint32_t imageSizeWritten = size;
	uint32_t bytesInBuffer;
	uint32_t currentAddr = external_flash_address;
	uint32_t output_crc = 0;
	uint8_t buffer[512];
	while(imageSizeWritten)
	{
		bytesInBuffer = imageSizeWritten < sizeof(buffer) ? imageSizeWritten : sizeof(buffer);
		dev_extFlash_enable();
		dev_extFlash_read(currentAddr, buffer, bytesInBuffer);
		dev_extFlash_disable();	
		am_bootloader_partial_crc32(buffer, bytesInBuffer, &output_crc);
		currentAddr += bytesInBuffer;
		imageSizeWritten -= bytesInBuffer;
	}	
	if(input_crc == output_crc)
	{
		return true;
	}

	return false;
}
#ifdef COD 
#ifdef LAYER_APPLICATION 
#include "cod_crc_utils.h"
static cod_crc32_calculator crc32_calculator = {0,false};
bool cod_check_crc_from_external_flash(uint32_t input_crc,uint32_t external_flash_address ,uint32_t size,uint32_t *output_crc)
{
	uint32_t imageSizeWritten = size;
	uint32_t bytesInBuffer;
	uint32_t currentAddr = external_flash_address;
	uint8_t buffer[512];
	
	cod_crc32_init(&crc32_calculator);
	while(imageSizeWritten)
	{
		bytesInBuffer = imageSizeWritten < sizeof(buffer) ? imageSizeWritten : sizeof(buffer);
		dev_extFlash_enable();
		dev_extFlash_read(currentAddr, buffer, bytesInBuffer);
		dev_extFlash_disable();	
		cod_crc32_update((cod_crc32_calculator*)(&crc32_calculator),buffer, bytesInBuffer);
		currentAddr += bytesInBuffer;
		imageSizeWritten -= bytesInBuffer;
	}	
	cod_crc32_get_final(&crc32_calculator);
    *output_crc = crc32_calculator._crc32;
	if(input_crc == crc32_calculator._crc32)
	{
		return true;
	}

	return false;
}
#endif
#endif
