//*****************************************************************************
//
//! @file am_multi_boot.c
//!
//! @brief Bootloader implementation accepting multiple host protocols.
//!
//! This is a bootloader implementation that supports flash programming over
//! UART, SPI, and I2C. The correct protocol is selected automatically at boot
//! time. The messaging is expected to follow little-endian format, which is
//! native to Apollo1/2.
//!
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_multi_boot_private.h"
#include "am_multi_boot.h"

//*****************************************************************************
//
// UART pin used for AutoBaud
//
//*****************************************************************************

//*****************************************************************************
//
// Message buffers.
//
// Note: The RX buffer needs to be 32-bit aligned to be compatible with the
// flash helper functions, but we also need an 8-bit pointer to it for copying
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************
uint8_t g_pui8TxBuffer[8];
uint32_t g_pui32RxBuffer[AM_HAL_FLASH_PAGE_SIZE / 4];
uint8_t *g_pui8RxBuffer = (uint8_t *) g_pui32RxBuffer;
uint32_t g_ui32RxBufSize = sizeof(g_pui32RxBuffer);
uint32_t g_ui32BytesInBuffer = 0;
uint32_t g_ui32LastOffset = 0;
bool g_bStoreInSRAM = 0;
uint32_t g_ui32SramBytesUsed;

//*****************************************************************************
//
// Globals to keep track of the image write state.
//
//*****************************************************************************
uint32_t *g_pui32WriteAddress = 0;
uint32_t g_ui32BytesReceived = 0;

volatile bool g_bImageValid = false;

uint32_t g_ui32CRC = 0;

//*****************************************************************************
//
// Global variable to keep track of which interface we're using.
//
//*****************************************************************************
extern volatile bool g_bIOSActive;

//*****************************************************************************
//
// Image structure to hold data about the downloaded boot image.
//
//*****************************************************************************
am_bootloader_image_t g_sImage =
{
    DEFAULT_LINK_ADDRESS,
    0,
    0,
    DEFAULT_OVERRIDE_GPIO,
    DEFAULT_OVERRIDE_POLARITY,
    0,
    0,
    0
};

//*****************************************************************************
//
// Flag page information.
//
//*****************************************************************************
am_bootloader_image_t *g_psBootImage = (am_bootloader_image_t *) FLAG_PAGE_LOCATION;

void setup_ios_interface(void);
void setup_serial(int32_t i32Module, uint32_t ui32BaudRate);
void cleanup_ios_interface(void);

//*****************************************************************************
//
// Setting up a GPIO ISR for the UART autobaud feature.
//
//*****************************************************************************
void
am_gpio_isr(void)
{
    uint32_t ui32Start, ui32End, ui32Counts, ui32BaudRate;

    //
    // Check the time, and record this as the "start" time.
    //
    // We need to hit this register as early as possible, so we are skipping
    // all of the normal logic that checks to make sure we are responding to
    // the right GPIO. In the interest of time, we will just assume that this
    // interrupt is coming from the UART RX pin.
    //
    ui32Start = am_hal_systick_count();

    //
    // Wait for exactly 9 edges on the UART RX pin. This corresponds to the
    // number of edges in the byte 0x55 after the start bit. Using a simple
    // polling approach here gives us the best possible chance to catch every
    // single edge.
    //
    while ( !am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );
    while ( am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );
    while ( !am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );
    while ( am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );
    while ( !am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );
    while ( am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );
    while ( !am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );
    while ( am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );
    while ( !am_hal_gpio_input_bit_read(AM_BSP_GPIO_BOOTLOADER_UART_RX) );

    //
    // Record the "end" time.
    //
    ui32End = am_hal_systick_count();

    //
    // At this point, the timing-critical portion of the interrupt handler is
    // complete, and we are free to clean up our interrupt status. We only
    // intend to perform the automatic baud-rate detection once, so we will go
    // ahead and disable the interrupt on the UART RX pin now.
    //
    am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BOOTLOADER_UART_RX));
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BOOTLOADER_UART_RX));

    //
    // Check to see how long those nine edges took to arrive. This should
    // correspond to exactly nine bit-times of UART traffic from the host. From
    // there, we can use the speed of the processor (which is known) to
    // calculate the host's baud rate.
    //
    ui32Counts = ui32Start - ui32End;
    ui32BaudRate = AM_HAL_CLKGEN_FREQ_MAX_HZ * 9 / ui32Counts;

    //
    // Now that we have a baud rate, we can configure our UART.
    //
    setup_serial(0, ui32BaudRate);

    //
    // Send a '0x55' to give the boot host some indication that we have the
    // correct baud rate and to let it know that our UART is ready for further
    // traffic.
    //
    am_hal_uart_char_transmit_polled(0, 0x55);
}

// Wipe Clean SRAM up to the specified address
// CAUTION!!!
// This will wipe the complete SRAM including stack of the caller
// This should be called as the last thing before calling reset
void wipe_sram(void)
{
    // Use the last SRAM location as temp
    uint32_t lastAddr = SRAM_BASEADDR + g_ui32SramBytesUsed - 4;
    //
    // Wipe SRAM (without using variables).
    //
    *(volatile uint32_t *)(lastAddr) = SRAM_BASEADDR;

    while ( *(volatile uint32_t *)(lastAddr) != lastAddr )
    {
        **(volatile uint32_t **)(lastAddr) = 0x0;
        *(volatile uint32_t *)(lastAddr) += 4;
    }
}

// Programs the flash based on g_pui32WriteAddress, g_pui8RxBuffer & g_ui32BytesInBuffer
void
program_image(uint32_t bEncrypted)
{
    uint32_t ui32WriteAddr = (uint32_t)g_pui32WriteAddress;
    uint32_t *pui32ReadAddr = (uint32_t *)g_pui8RxBuffer;
    uint32_t ui32NumBytes = g_ui32BytesInBuffer;

    if ( g_bStoreInSRAM )
    {
        while ( ui32NumBytes )
        {
            am_bootloader_program_flash_page(ui32WriteAddr, pui32ReadAddr,
                (ui32NumBytes > AM_HAL_FLASH_PAGE_SIZE) ? AM_HAL_FLASH_PAGE_SIZE: ui32NumBytes);
            if ( ui32NumBytes > AM_HAL_FLASH_PAGE_SIZE )
            {
                ui32NumBytes -= AM_HAL_FLASH_PAGE_SIZE;
                ui32WriteAddr += AM_HAL_FLASH_PAGE_SIZE;
                pui32ReadAddr += AM_HAL_FLASH_PAGE_SIZE / 4;
            }
            else
            {
                break;
            }
        }
    }
    // TODO: Apply necessary protections to flash
    // Need to add flash protection - chunk by chunk including potentially
    // the last partial chunk
}

//*****************************************************************************
//
//! @brief Read an image start packet
//!
//! @param psImage is the image structure to read the packet into.
//!
//! This function reads the "new image" packet, and uses that
//! packet to fill in a bootloader image structure. The caller is responsible
//! for verifying the packet type before calling this function.
//! Packet Structure:
//! word0 = Link Address
//! word1 = Number of Bytes (Image Size)
//! word2 = CRC
//! ENCRYPTED?? (#ifdef MULTIBOOT_SECURE)
//! word3 = Security Trailer Length
//!
//! @return true if the image parameters are acceptable.
//
//*****************************************************************************
bool
image_start_packet_read(am_bootloader_image_t *psImage, uint32_t *pui32Packet)
{
    am_hal_mcuctrl_device_t sDevice;

    // Get chip specific info
    am_hal_mcuctrl_device_info_get(&sDevice);

    //
    // Set the image structure parameters based on the information in the
    // packet.
    //
    psImage->pui32LinkAddress = (uint32_t *)(pui32Packet[1]);
    psImage->ui32NumBytes = pui32Packet[2];
    psImage->ui32CRC = pui32Packet[3];
    psImage->ui32OverrideGPIO = DEFAULT_OVERRIDE_GPIO;
    psImage->ui32OverridePolarity = DEFAULT_OVERRIDE_POLARITY;
    psImage->bEncrypted = 0; // This only indicates Copy-Protection in flash

    //
    // We'll need to fill in the stack pointer and reset vector a little later
    // in the process.
    //
    psImage->pui32StackPointer = 0;
    psImage->pui32ResetVector = 0;

    //
    // Check to make sure we're not overwriting the bootloader.
    //
    if ( (uint32_t)psImage->pui32LinkAddress < MAX_BOOTLOADER_SIZE )
    {
        return false;
    }
    if ( USE_FLAG_PAGE )
    {
        //
        // Check to make sure we're not overwriting the flag page.
        //
        uint32_t ui32ImageStart = (uint32_t)psImage->pui32LinkAddress;
        uint32_t ui32ImageEnd = (uint32_t)psImage->pui32LinkAddress + psImage->ui32NumBytes - 1;
        if ( (FLAG_PAGE_LOCATION == ui32ImageStart) ||
             ((FLAG_PAGE_LOCATION < ui32ImageStart) &&
                ((FLAG_PAGE_LOCATION + AM_HAL_FLASH_PAGE_SIZE) > ui32ImageStart)) ||
            ((FLAG_PAGE_LOCATION > ui32ImageStart) &&
                (FLAG_PAGE_LOCATION <= ui32ImageEnd))
           )
        {
            return false;
        }
    }
    // Determine if we can gather image in SRAM completely before flashing all at once
    if ((sDevice.ui32SRAMSize - MAX_SRAM_USED) >= psImage->ui32NumBytes)
    {
        g_bStoreInSRAM = 1;
        g_pui8RxBuffer = (uint8_t *)(SRAM_BASEADDR + MAX_SRAM_USED);
        g_ui32RxBufSize = psImage->ui32NumBytes;
        g_ui32SramBytesUsed = sDevice.ui32SRAMSize;
    }
    else
    {
        g_bStoreInSRAM = 0;
        g_pui8RxBuffer = (uint8_t *) g_pui32RxBuffer;
        g_ui32RxBufSize = sizeof(g_pui32RxBuffer);
        g_ui32SramBytesUsed = MAX_SRAM_USED;
    }

#ifdef MULTIBOOT_SECURE
    // Validate the security trailer & Initialize the security params
    if ( init_multiboot_secure(pui32Packet[4], &pui32Packet[5], g_bStoreInSRAM,
                               &psImage->bEncrypted, psImage) != 0 )
    {
        return false;
    }
#endif
    //
    // Otherwise, the image is presumed to be reasonable. Set our global
    // variables based on the new image structure.
    //
    g_pui32WriteAddress = psImage->pui32LinkAddress;
    g_ui32BytesReceived = 0;
    g_ui32BytesInBuffer = 0;
    g_ui32LastOffset = 0;
    g_ui32CRC = 0;
    return true;
}

//*****************************************************************************
//
//! @brief Read an image start packet
//!
//! @param psImage is the image structure to read the packet into.
//!
//! This function reads the "new image" packet, and uses that
//! packet to fill in a bootloader image structure. The caller is responsible
//! for verifying the packet type before calling this function.
//!
//! @return None.
//
//*****************************************************************************
void
image_data_packet_read(uint8_t *pui8Src, uint32_t ui32Size)
{
    uint32_t i;
    //
    // Loop through the data, copying it into the global buffer.
    //
    for ( i = 0; i < ui32Size; i++ )
    {
        g_pui8RxBuffer[g_ui32BytesInBuffer] = *pui8Src++;

        //
        // Keep track of how much data we've copied into the SRAM buffer.
        //
        g_ui32BytesInBuffer++;
        g_ui32BytesReceived++;

        //
        // Whenever we hit a page boundary or the end of the image, we should
        // write to flash.
        //
        if ( (!g_bStoreInSRAM && (g_ui32BytesInBuffer == AM_HAL_FLASH_PAGE_SIZE)) ||
                 g_ui32BytesReceived == g_sImage.ui32NumBytes )
        {
            //
            // Run a quick CRC on the received bytes, holding on to the result in a
            // global variable, so we can pick up where we left off on the next pass.
            //
            am_bootloader_partial_crc32(g_pui8RxBuffer, g_ui32BytesInBuffer, &g_ui32CRC);

#ifdef MULTIBOOT_SECURE
            // Validate the security trailer & Initialize the security params
            multiboot_secure_decrypt(g_pui8RxBuffer, g_ui32BytesInBuffer);
#endif

            //
            // If this is the first block of our new image, we need to record
            // the reset vector and stack pointer information for inclusion in
            // the flag page.
            //
            if ( g_bStoreInSRAM || (g_ui32BytesReceived <= AM_HAL_FLASH_PAGE_SIZE) )
            {
                g_sImage.pui32StackPointer = (uint32_t *)(((uint32_t *)g_pui8RxBuffer)[0]);
                g_sImage.pui32ResetVector = (uint32_t *)(((uint32_t *)g_pui8RxBuffer)[1]);
            }

            if ( !g_bStoreInSRAM )
            {
                am_bootloader_program_flash_page((uint32_t)g_pui32WriteAddress,
                    (uint32_t *)g_pui8RxBuffer, g_ui32BytesInBuffer);
                //
                // Adjust the global variables.
                //
                g_pui32WriteAddress += (g_ui32BytesInBuffer / 4);
                g_ui32BytesInBuffer = 0;
            }
        }
    }
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
am_multiboot_main(void)
{
    //
    // Set the (active LOW) interrupt pin so the host knows we don't have a
    // message to send yet.
    //
    am_hal_gpio_out_bit_set(INTERRUPT_PIN);
    am_hal_gpio_pin_config(INTERRUPT_PIN, AM_HAL_PIN_OUTPUT);

    //
    // If we're using a flag page, we can run a full CRC check to verify the
    // integrity of our image. If not, we'll just check the override pin.
    //
    if ( USE_FLAG_PAGE )
    {
        //
        // Check the flag page (including the stored CRC) and the override pin
        // to make sure we have a valid image and the host isn't requesting an
        // upgrade.
        //
        if ( am_bootloader_image_check(g_psBootImage) )
        {
            //
            // If everything looks good, disable the interrupt pin and run.
            //
            am_hal_gpio_pin_config(INTERRUPT_PIN, AM_HAL_PIN_DISABLE);
            am_bootloader_image_run(g_psBootImage);
        }
    }
    else
    {
        //
        // Check the override pin to make sure the host isn't requesting an
        // upgrade, and do a quick check to make sure an image actually exists
        // at the default application location.
        //
        if ( am_hal_bootloader_override_check(&g_sImage) == false &&
             *(g_sImage.pui32LinkAddress) != 0xFFFFFFFF)
        {
            //
            // If everything looks good, disable the interrupt pin and run.
            //
            am_hal_gpio_pin_config(INTERRUPT_PIN, AM_HAL_PIN_DISABLE);
            am_bootloader_image_run(&g_sImage);
        }
    }

    //
    // If we get here, we're going to try to download a new image from a host
    // processor. Speed up the clocks and start turning on peripherals.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    //
    // Start systick to measure time for autobaud and for the IOS timeout.
    //
    am_hal_systick_load(0x00FFFFFF);
    am_hal_systick_start();

    //
    // Enable the IOS. Choose the correct protocol based on the state of pin 0.
    //
    setup_ios_interface();

    //
    // Wait for a few milliseconds to see if anyone will send us data.
    //
    while ( g_bIOSActive || am_hal_systick_count() > (0xFFFFFF - WAIT_IOS_BOOT_SYSTICK) )
    {
        //
        // Delay to avoid polling peripheral registers so frequently.
        //
        am_util_delay_ms(1);
    }

    //
    // If we didn't get any IOS packets, we'll move on to the UART option.
    //
    cleanup_ios_interface();

    //
    // Configure our RX pin as a GPIO input with a falling edge interrupt.
    //
    am_hal_gpio_pin_config(AM_BSP_GPIO_BOOTLOADER_UART_RX, AM_HAL_GPIO_INPUT);
    am_hal_gpio_int_polarity_bit_set(AM_BSP_GPIO_BOOTLOADER_UART_RX, AM_HAL_GPIO_FALLING);

    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BOOTLOADER_UART_RX));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BOOTLOADER_UART_RX));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);

    //
    // Enable interrupts so we can receive messages from the boot host.
    //
    am_hal_interrupt_master_enable();

    //
    // Loop forever.
    //
    while (1)
    {
    }
}
