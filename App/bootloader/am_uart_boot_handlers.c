//*****************************************************************************
//
//! @file am_uart_boot_handlers.c
//!
//! @brief Boot related functions for the UART interface.
//!
//! This file contains the main state machine for handling boot commands via
//! the UART interface.
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

//*****************************************************************************
//
// UART configuration settings.
//
//*****************************************************************************
am_hal_uart_config_t g_sUartConfig =
{
    .ui32BaudRate = 115200,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .bTwoStopBits = false,
    .ui32Parity   = AM_HAL_UART_PARITY_NONE,
    .ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,
};

//
//
#define MAX_CHUNK_SIZE                  512 // Support max 512B chunks
#define MAX_BUFFER                      (MAX_CHUNK_SIZE + 8)
volatile uint32_t ui32_serial_status;
volatile unsigned char cRecChar, RX_FIFO_FULL, RX_FIFO_EMPTY;
volatile unsigned char uart_RX_buffer[MAX_BUFFER];
volatile uint32_t uart_RX_cnt, uart_RX_head, uart_RX_tail;
// ************************************
unsigned char cTxChar, TX_FIFO_FULL, TX_FIFO_EMPTY;
unsigned char uart_TX_buffer[8];
unsigned char uart_TX_cnt, uart_TX_head, uart_TX_tail;
// ************************************

uint32_t command;
uint32_t packet_cnt, pkt_size;

void
setup_serial(int32_t i32Module, uint32_t ui32BaudRate)
{

    command = 0;
    uart_RX_head = uart_RX_tail = uart_RX_cnt = 0;
    RX_FIFO_FULL = 0;  // check serial FIFO operation
    RX_FIFO_EMPTY = 1;
    packet_cnt = 0xff;

    //
    // Make sure the UART RX and TX pins are enabled.
    //
    am_hal_gpio_pin_config(AM_BSP_GPIO_BOOTLOADER_UART_TX, AM_BSP_GPIO_CFG_BOOTLOADER_UART_TX);
    am_hal_gpio_pin_config(AM_BSP_GPIO_BOOTLOADER_UART_RX, AM_BSP_GPIO_CFG_BOOTLOADER_UART_RX);

    //
    // Power on the selected UART
    //
    am_hal_uart_pwrctrl_enable(i32Module);

    //
    // Start the UART interface, apply the desired configuration settings, and
    // enable the FIFOs.
    //
    am_hal_uart_clock_enable(i32Module);

    //
    // Disable the UART before configuring it.
    //
    am_hal_uart_disable(i32Module);

    //
    // Configure the UART.
    //
    g_sUartConfig.ui32BaudRate = ui32BaudRate;
    am_hal_uart_config(i32Module, &g_sUartConfig);

    //
    // Enable the UART FIFO.
    //
   am_hal_uart_fifo_config(i32Module,
        AM_HAL_UART_TX_FIFO_3_4 | AM_HAL_UART_RX_FIFO_3_4);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(i32Module);

    am_hal_uart_int_clear(0, AM_HAL_UART_INT_RX | AM_HAL_UART_INT_RX_TMOUT);
    am_hal_uart_int_enable(0, AM_HAL_UART_INT_RX | AM_HAL_UART_INT_RX_TMOUT);

    am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART);
}

//
void
ser_out(unsigned char data)
{
    AM_REGn(UART, 0, DR) = data;
    while ( AM_BFRn(UART, 0, FR, TXFF) );
}


//
//*****************************************************************************
//
void
hex_out(unsigned char hex_data)
{
    unsigned char temp;
    temp = hex_data >> 4;
    if ( temp > 10 )
    {
        ser_out(temp + 0x37);
    }
    else
    {
        ser_out(temp + 0x30);
    }

    temp = hex_data & 0x0f;
    if ( (hex_data & 0x0f) > 10 )
    {
        ser_out(temp + 0x37);
    }
    else
    {
        ser_out(temp + 0x30);
    }
}

void
hex_out_int(int ihex_data)
{
    hex_out(ihex_data >> 8);
    hex_out(ihex_data & 0x00ff);
}

//*****************************************************************************
//
// am_uart_isr
//
//*****************************************************************************
void
am_uart_isr(void)
{
    uint32_t *rx_pui32Packet;
    unsigned char send_byte_cnt, flush = 0;
    uint32_t cnt = 0;
    uint32_t *tx_pui32Packet;

    uint32_t ui32Status;
    uint8_t rxData;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //
    ui32Status = am_hal_uart_int_status_get(0, false);
    am_hal_uart_int_clear(0, ui32Status);
    // ui32_serial_status = ui32Status;
    //
    // Service the uart FIFO.
    //


    RX_FIFO_EMPTY = 0;
    // Read the UART FIFO till we have data
    while ( !AM_BFRn(UART, 0, FR, RXFE) )
    {
        if ( uart_RX_head == MAX_BUFFER )
        {
            // We should never reach here!
            RX_FIFO_FULL = 1;
            tx_pui32Packet = (uint32_t *) uart_TX_buffer;
            //
            // Bad packet; Send back an error.
            //
            tx_pui32Packet[0] = AM_BOOTLOADER_ERROR;
            flush = 1;
            send_byte_cnt = 4;
            break;
        }
        rxData = AM_REGn(UART, 0, DR);
        uart_RX_buffer[uart_RX_head++] = rxData;

        uart_RX_cnt++;
        cnt++;

        if ( uart_RX_cnt == 4 ) // cmd received, wait for parameters
        {
            command = uart_RX_buffer[0];
            switch (command)
            {
                case AM_BOOTLOADER_NEW_IMAGE:
#ifndef MULTIBOOT_SECURE
                    packet_cnt = 16;
#endif
                    break;
                case AM_BOOTLOADER_SET_OVERRIDE_CMD:
                    packet_cnt = 12;
                    break;
                case AM_BOOTLOADER_NEW_PACKET:
                    //    packet_cnt = pkt_size+8;
                    break;
                case AM_BOOTLOADER_RESET:
                    packet_cnt = 4;
                    break;
                case AM_BOOTLOADER_BL_VERSION_CMD:
                    packet_cnt = 4;
                    break;
                case AM_BOOTLOADER_ACK_CMD:
                    packet_cnt = 4;
                    break;
                case AM_BOOTLOADER_NAK_CMD:
                    packet_cnt = 4;
                    break;
                case AM_BOOTLOADER_RESTART:
                    packet_cnt = 4;
                    break;
                default:
                    // Unknown command
                    packet_cnt = 4;
                    break;
            }
        }

        if ( (command == AM_BOOTLOADER_NEW_PACKET) && (uart_RX_cnt == 8) )
        {
            pkt_size = *(uint32_t *) (&uart_RX_buffer[4]);
            packet_cnt = pkt_size + 8;
        }

#ifdef MULTIBOOT_SECURE
        if ( (command == AM_BOOTLOADER_NEW_IMAGE) && (uart_RX_cnt == 20) )
        {
            pkt_size = *(uint32_t *) (&uart_RX_buffer[16]);
            packet_cnt = pkt_size + 20;
        }
#endif

        if ( (uart_RX_cnt >= 4) && (uart_RX_cnt == packet_cnt) )
        {
            rx_pui32Packet = (uint32_t *) uart_RX_buffer;
            tx_pui32Packet = (uint32_t *) uart_TX_buffer;

            switch (rx_pui32Packet[0])
            {
                case AM_BOOTLOADER_NEW_IMAGE:
                    //
                    // Parse the image packet, and store the result to the global
                    // image structure.
                    //
                    g_bImageValid = image_start_packet_read(&g_sImage,
                                        (uint32_t *) uart_RX_buffer);

                    //
                    // Make sure the image packet had reasonable contents. If it
                    // didn't, we need to let the host know.
                    //
                    if ( g_bImageValid )
                    {
                        //
                        // Good image; Send back a "READY" packet.
                        //
                        tx_pui32Packet[0] = AM_BOOTLOADER_READY;
                    }
                    else
                    {
                        //
                        // Bad image; Send back an error.
                        //
                        tx_pui32Packet[0] = AM_BOOTLOADER_ERROR;
                    }

                    flush = 1;
                    send_byte_cnt = 4;
                    break;

                case AM_BOOTLOADER_SET_OVERRIDE_CMD:
                    //
                    // Set the override GPIO settings based on the packet
                    // information.
                    //
                    g_sImage.ui32OverrideGPIO = rx_pui32Packet[1];
                    g_sImage.ui32OverridePolarity = rx_pui32Packet[2];

                    //
                    // Send back a "READY" packet.
                    //
                    tx_pui32Packet[0] = AM_BOOTLOADER_READY;
                    flush = 1;
                    send_byte_cnt = 4;
                    break;

                case AM_BOOTLOADER_NEW_PACKET:
                    //
                    // Only take new packets if our image structure is valid.
                    //
                    if ( !g_bImageValid )
                    {
                        tx_pui32Packet[0] = AM_BOOTLOADER_ERROR;
                        break;
                    }

                    //
                    // Parse the rest of the packet sitting in the IOS LRAM.
                    //
                    image_data_packet_read((uint8_t *)(uart_RX_buffer + 8),
                        *((uint32_t *)(uart_RX_buffer + 4)));

                    //
                    // If this packet completed the image...
                    //
                    if ( g_ui32BytesReceived == g_sImage.ui32NumBytes )
                    {
#ifdef MULTIBOOT_SECURE
                        if ( (g_ui32CRC != g_sImage.ui32CRC) || multiboot_secure_verify(&g_sImage) )
#else
                        if ( g_ui32CRC != g_sImage.ui32CRC )
#endif
                        {
                            tx_pui32Packet[0] = AM_BOOTLOADER_BAD_CRC;
                        }
                        else
                        {
                            // Protect (and optionally write if stored in SRAM)
                            // image in flash now as it has been validated now
                            program_image(g_sImage.bEncrypted);
                            // Validate the flash contents of a boot image to make
                            // sure it's safe to run
                            if ( am_bootloader_flash_check(&g_sImage) )
                            {
                                tx_pui32Packet[0] = AM_BOOTLOADER_IMAGE_COMPLETE;
                            }
                            else
                            {
                                tx_pui32Packet[0] = AM_BOOTLOADER_ERROR;
                            }
                        }
                    }
                    else
                    {
                        //
                        // If this wasn't the end of the image, just send back a
                        // "READY" packet.
                        //
                        tx_pui32Packet[0] = AM_BOOTLOADER_READY;
                    }

                    flush = 1;
                    send_byte_cnt = 4;

                    break;

                case AM_BOOTLOADER_RESET:
                    if ( USE_FLAG_PAGE )
                    {
                        //
                        // Write the flag page.
                        //
                        am_bootloader_flag_page_update(&g_sImage,
                            (uint32_t *)FLAG_PAGE_LOCATION);
                    }
#ifdef MULTIBOOT_SECURE
                    wipe_sram();
#endif

                case AM_BOOTLOADER_RESTART:
                    //
                    // Perform a software reset.
                    //
                    am_hal_reset_poi();

                    //
                    // Wait for the reset to take effect.
                    //
                    while (1);

                case AM_BOOTLOADER_BL_VERSION_CMD:
                    //
                    // Respond with the version number.
                    //
                    tx_pui32Packet[0] = AM_BOOTLOADER_BL_VERSION;
                    tx_pui32Packet[1] = AM_BOOTLOADER_VERSION_NUM;

                    flush = 1;
                    send_byte_cnt = 8;
                    break;

                case AM_BOOTLOADER_ACK_CMD:
                case AM_BOOTLOADER_NAK_CMD:
                    break;

                default:
                    // Error
                    tx_pui32Packet[0] = AM_BOOTLOADER_ERROR;
                    flush = 1;
                    send_byte_cnt = 4;
                    break;
            }
            break;
        }
    }

    if ( flush )
    {
        uart_TX_tail = 0;

        for ( cnt = 0; cnt < send_byte_cnt; cnt++ )
        {
            ser_out(uart_TX_buffer[uart_TX_tail++]);
        }
        command = 0;
        uart_RX_head = uart_RX_tail = uart_RX_cnt = 0;
        RX_FIFO_FULL = 0;  // check serial FIFO operation
        RX_FIFO_EMPTY = 1;
        flush = 0;
        send_byte_cnt = 0;
        packet_cnt = 0xff;
    }

}
