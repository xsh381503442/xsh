/**
 * Copyright (c) 2017 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/** @file
 *
 * @ingroup nfc_writable_ndef_msg_example_ndef_file_m ndef_file_m.c
 * @{
 * @ingroup nfc_writable_ndef_msg_example
 * @brief The FDS file handler for the NFC writable NDEF message example.
 *
 */
 #include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_dfu_svci.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_freertos.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_state.h"
#include "ble_dfu.h"
#include "nrf_ble_gatt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "fds.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"

#include "task_apollo2.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_power.h"

#ifdef BOARD_PCA10040
#include "bsp.h"
#include "boards.h"
#include "bsp_btn_ble.h"
#endif


#include "app_srv_apollo.h"
#include "drv_ble_flash.h"
#include "ble_srv_nrf52.h"
#include "app_srv_nrf52.h"
#include "drv_extFlash.h"
#include "dev_extFlash.h"


#include "drv_apollo2.h"
#include "dev_apollo2.h"
#include "nrf_delay.h"

#include "cmd_apollo2.h"
#include "cmd_gpregret.h"
#include "cmd_nrf52_srv.h"
#include "nrf_drv_gpiote.h"
#include "nrf_sdm.h"
#include "ble_srv_wechat.h"
#include "app_srv_wechat.h"
#include "task_ble_conn.h"


#include "ble_db_discovery.h"
#include "app_srv_ancs.h"
#include "nrf_ble_ancs_c.h"
#include "app_srv_acces.h"

#include "task_extFlash.h"
#include "task_ble_accessary.h"

#include "ble_srv_apollo.h"
#include "ble_srv_nrf52.h"
#include "ble_srv_wechat.h"


#include "nrf_ble_ancs_c.h"
#include "ble_hrs_c.h"
#include "ble_rscs_c.h"
#include "ble_cscs_c.h"

#include "drv_nfc_tag4.h"
#include "fds.h"
#include "nfc_uri_msg.h"


#define BYTES_TO_WORDS(n_bytes) (((n_bytes) + 3) >> 2)

#define FILE_ID 0x1111                              /**< NDEF message file ID. */
#define REC_KEY 0x2222                              /**< NDEF message record KEY. */

static const uint8_t m_url[] =                      /**< Default NDEF message: URL "nordicsemi.com". */
	{'i', 'i', 'p', 'i', 'i', '.', 'n', 'e', 't'}; //URL "iipii.net"


static volatile bool m_fds_ready      = false;      /**< Flag used to indicate that FDS initialization is finished. */
static volatile bool m_pending_write  = false;      /**< Flag used to preserve write request during Garbage Collector activity. */
static volatile bool m_pending_update = false;      /**< Flag used to preserve update request during Garbage Collector activity. */

static uint32_t        m_pending_msg_size   = 0;    /**< Pending write/update request data size. */
static uint8_t const * m_p_pending_msg_buff = NULL; /**< Pending write/update request data pointer. */

static fds_record_desc_t  m_record_desc;            /**< Record descriptor. */
static fds_record_t       m_record;                 /**< Record description used for writes. */


/**
 * @brief   Prepare record structure for write or update request.
 *
 * @details Configures file ID, record KEY, data to be written and message length.
 *
 * @param[in] buff  Pointer to the NDEF message to be stored in FLASH.
 * @param[in] size  Size of NDEF message.
 */
static void ndef_file_prepare_record(uint8_t const * p_buff, uint32_t size)
{
    // Set up record.
    m_record.file_id           = FILE_ID;
    m_record.key               = REC_KEY;
    m_record.data.p_data       = p_buff;
    m_record.data.length_words = BYTES_TO_WORDS(size); // Align data length to 4 bytes.
}


/**
 * @brief   Function for creating NDEF message in FLASH file.
 *
 * @details FDS write operation is performed asynchronously,
 *          operation status is reported through events.
 *
 * @param[in] p_buff Pointer to the NDEF message to be stored in FLASH.
 * @param[in] size   Size of NDEF message.
 *
 * @return  NRF_SUCCESS when update request has been added to queue,
 *          otherwise it returns FDS error code.
 */
static ret_code_t ndef_file_create(uint8_t const * p_buff, uint32_t size)
{
    ret_code_t err_code;

    // Prepare record structure.
    ndef_file_prepare_record(p_buff, size);

    // Create FLASH file with NDEF message.
    err_code = fds_record_write(&m_record_desc, &m_record);
    if (err_code == FDS_ERR_NO_SPACE_IN_FLASH)
    {
        // If there is no space, preserve write request and call Garbage Collector.
        m_pending_write      = true;
        m_pending_msg_size   = size;
        m_p_pending_msg_buff = p_buff;

        err_code = fds_gc();
    }

    return err_code;
}


/**
 * @brief   Flash Data Storage(FDS) event handler.
 *
 * @details This function is used to handle various FDS events like end of initialization,
 *          write, update and Garbage Collection activity. It is used to track FDS actions
 *          and perform pending writes after the Garbage Collecion activity.
 *
 * @param[in] p_fds_evt Pointer to the FDS event.
 */
static void fds_evt_handler(fds_evt_t const * const p_fds_evt)
{
    ret_code_t err_code;



    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            APP_ERROR_CHECK(p_fds_evt->result);
            m_fds_ready = true;
            break;

        case FDS_EVT_UPDATE:
            APP_ERROR_CHECK(p_fds_evt->result);
//            NRF_LOG_INFO("FDS update success.");
            break;

        case FDS_EVT_WRITE:
            APP_ERROR_CHECK(p_fds_evt->result);
//            NRF_LOG_INFO("FDS write success.");
            break;

        case FDS_EVT_GC:
            APP_ERROR_CHECK(p_fds_evt->result);
//            NRF_LOG_INFO("Garbage Collector activity finished.");

            //Perform pending write/update.
            if (m_pending_write)
            {
//                NRF_LOG_DEBUG("Write pending msg.", p_fds_evt->id, p_fds_evt->result);
                m_pending_write = false;
                err_code        = ndef_file_create(m_p_pending_msg_buff, m_pending_msg_size);
                APP_ERROR_CHECK(err_code);
            }
            else if (m_pending_update)
            {
//                NRF_LOG_DEBUG("Update pending msg.", p_fds_evt->id, p_fds_evt->result);
                m_pending_update = false;
                err_code         = ndef_file_update(m_p_pending_msg_buff, m_pending_msg_size);
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}


ret_code_t ndef_file_setup(void)
{
    ret_code_t err_code;

    // Register FDS event handler to the FDS module.
    err_code = fds_register(fds_evt_handler);
    VERIFY_SUCCESS(err_code);

    // Initialize FDS.
    err_code = fds_init();
    VERIFY_SUCCESS(err_code);

    // Wait until FDS is initialized.
    while (!m_fds_ready);

    return err_code;
}


ret_code_t ndef_file_update(uint8_t const * p_buff, uint32_t size)
{
    ret_code_t err_code;

    // Prepare record structure.
    ndef_file_prepare_record(p_buff, size);

    // Update FLASH file with new NDEF message.
    err_code = fds_record_update(&m_record_desc, &m_record);
    if (err_code == FDS_ERR_NO_SPACE_IN_FLASH)
    {
        // If there is no space, preserve update request and call Garbage Collector.
        m_pending_update     = true;
        m_pending_msg_size   = size;
        m_p_pending_msg_buff = p_buff;
//        NRF_LOG_INFO("FDS has no space left, Garbage Collector triggered!");
        err_code = fds_gc();
    }

    return err_code;
}


ret_code_t ndef_file_default_message(uint8_t * p_buff, uint32_t * p_size)
{
    /* Encode URI message into buffer. */
    ret_code_t err_code = nfc_uri_msg_encode(NFC_URI_HTTP_WWW,
                                             m_url,
                                             sizeof(m_url),
                                             p_buff,
                                             p_size);
    return err_code;
}


ret_code_t ndef_file_load(uint8_t * p_buff, uint32_t size)
{
    ret_code_t         err_code;
    fds_find_token_t   ftok;
    fds_flash_record_t flash_record;

    // Always clear token before running new file/record search.
    memset(&ftok, 0x00, sizeof(fds_find_token_t));

    // Search for NDEF message in FLASH.
    err_code = fds_record_find(FILE_ID, REC_KEY, &m_record_desc, &ftok);

    // If there is no record with given key and file ID,
    // create default message and store in FLASH.
    if (err_code == FDS_SUCCESS)
    {
//        NRF_LOG_INFO("Found NDEF file record.");

        // Open record for read.
        err_code = fds_record_open(&m_record_desc, &flash_record);
        VERIFY_SUCCESS(err_code);

        // Access the record through the flash_record structure.
        memcpy(p_buff,
               flash_record.p_data,
               flash_record.p_header->length_words * sizeof(uint32_t));

        // Print file length and raw message data.
//        NRF_LOG_DEBUG("NDEF file data length: %u bytes.",
//                      flash_record.p_header->length_words * sizeof(uint32_t));

//        NRF_LOG_HEXDUMP_DEBUG(p_buff, flash_record.p_header->length_words * sizeof(uint32_t));

        // Close the record when done.
        err_code = fds_record_close(&m_record_desc);
    }
    else if (err_code == FDS_ERR_NOT_FOUND)
    {
//        NRF_LOG_INFO("NDEF file record not found, default NDEF file created.", err_code);

        // Create default NDEF message.
        err_code = ndef_file_default_message(p_buff, &size);
        VERIFY_SUCCESS(err_code);

        // Create record with default NDEF message.
        err_code = ndef_file_create(p_buff, size);
    }

    return err_code;
}

/** @} */
