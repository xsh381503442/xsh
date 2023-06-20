/*************************************************************************************************/
/*!
 *  \file   llc_hci.h
 *
 *  \brief  Main Link Layer Controller (LLC) HCI services.
 *
 *          $Date: 2016-03-29 13:29:10 -0700 (Tue, 29 Mar 2016) $
 *          $Revision: 6514 $
 *
 *  Copyright (c) 2015 ARM, Ltd., all rights reserved.
 *  ARM confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact ARM, Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#ifndef LLC_HCI_H
#define LLC_HCI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wsf_types.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \struct LlcHciCbacks_t
 *
 *  \brief  Callbacks for allocating buffers and handling read and write completion.
 */
/*************************************************************************************************/
typedef struct
{
    /*********************************************************************************************/
    /*!
     * \brief   Allocate a buffer for a received message.
     *
     * This function is called from an interrupt context.
     *
     * \param   len  Length of buffer
     *
     * \return  Pointer to buffer or NULL if buffer could not be allocated
     */
    /*********************************************************************************************/
    uint8_t *(*BufAlloc) (uint16_t len);

    /*********************************************************************************************/
    /*!
     * \brief   Free a buffer previously allocated with BufAlloc().
     *
     * \param   buf  Pointer to buffer
     */
    /*********************************************************************************************/
    void (*BufFree)  (uint8_t *buf);

    /*********************************************************************************************/
    /*!
     * \brief   Handle read completion.
     *
     * This function is called from an interrupt context.
     *
     * \param   type    Packet type
     * \param   pData   Packet data, which was allocated with BufAlloc().  The caller must free
     *                  this buffer
     * \param   len     Length of packet data, in bytes
     */
    /*********************************************************************************************/
    void (*ReadDone) (uint8_t type, uint8_t *pData, uint16_t len);

    /*********************************************************************************************/
    /*!
     * \brief   Handle write completion.
     *
     * This function is called from an interrupt context.
     *
     * \parma   type      Packet type.
     * \param   pData     Pointer to buffer that held written data, which was passed to
     *                    LlcHciWrite()
     * \param   err       Indicates success (0) or error (one of the LLC_HCI_ERROR_#### codes)
     * \param   pContext  Context pointer passed to `LlcHciWrite()`
     *
     * \return  None.
     */
    /*********************************************************************************************/
    void (*WriteDone)(uint8_t type, uint8_t *pData, int32_t err, void *pContext);
} LlcHciCbacks_t;

/*! \brief    Error codes. */
enum
{
    LLC_HCI_ERROR_OK      =  0,  /*!< No error; the operation succeeded. */
    LLC_HCI_ERROR_BAD_ACK = -1,  /*!< The write failed because a bad ACK was received. */
    LLC_HCI_ERROR_ABORTED = -2,  /*!< The write was aborted. */
    LLC_HCI_ERROR_ALLOC   = -3,  /*!< Allocation failed. */
};

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 * \brief   Initialize the HCI and prepare software for reading and writing.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlcHciInit(void);

/*************************************************************************************************/
/*!
 * \brief   Set callbacks for HCI notifications and buffer allocations.
 *
 * \param   pCbacks   Pointer to callbacks.  If NULL, the reference to the existing callbacks will be
 *                    cleared.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlcHciSetCbacks(const LlcHciCbacks_t *pCbacks);

/*************************************************************************************************/
/*!
 * \brief   Take up the interface (i.e., start receiving messages and be able to transmit).
 *
 * \return  None.
 *
 * HCI messages will be received and transmissions will be allowed.  The HCI callbacks should have
 * been set before the interface is taken up.
 */
/*************************************************************************************************/
void LlcHciTakeUp(void);

/*************************************************************************************************/
/*!
 * \brief   Take down the interface (i.e., stop receiving messages and block transmissions).
 *
 * \return  None.
 *
 * HCI messages will no longer be received and transmissions (i.e., calls to `LlcHciWrite()`) will
 * be blocked.  The HCI callbacks can only safely be cleared after the interface is taken down.
 */
/*************************************************************************************************/
void LlcHciTakeDown(void);

/*************************************************************************************************/
/*!
 * \brief   Fully disable the interface.
 *
 * \return  None.
 *
 * Disabling the interface may be necessary upon an unexpected event, such as a BLEP-specific
 * command received after startup is complete.
 */
/*************************************************************************************************/
void LlcHciDisable(void);

/*************************************************************************************************/
/*!
 * \brief   Write packet to HCI.
 *
 * \param   type      Packet type.
 * \param   pData     Packet data.
 * \param   len       Packet length, in bytes
 * \param   pContext  Context pointer that will be returned in the `WriteDone()` callback
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlcHciWrite(uint8_t type, const uint8_t *pData, uint16_t len, void *pContext);

/**************************************************************************************************
  Interrupt Handlers
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Handle LLCC received command valid interrupt.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LLCC_RXCMD_VALID_Handler(void);

/*************************************************************************************************/
/*!
 *  \brief  Handle LLCC received event valid interrupt.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LLCC_RXEVT_VALID_Handler(void);

/*************************************************************************************************/
/*!
 *  \brief  Handle LLCC transmit DMA done interrupt.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LLCC_TXDMAL_DONE_Handler(void);

/*************************************************************************************************/
/*!
 *  \brief  Handle LLCC receive DMA done interrupt.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LLCC_RXDMAL_DONE_Handler(void);

/**************************************************************************************************
  Direct LLCC
**************************************************************************************************/

/*! \brief    Length of LLCC event. */
#define LLCC_EVENT_LEN        8

/*! \brief    Received LLCC event callback. */
typedef void (*llccRxEventCback_t)(const uint8_t *pEvt);

/*************************************************************************************************/
/*!
 * \brief   Set callback for received LLCC event.
 *
 * \param   rxEventCback    Received LLCC event callback.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlccSetRxEventCback(llccRxEventCback_t rxEventCback);

/*************************************************************************************************/
/*!
 * \brief   Transmit LLCC event.
 *
 * \param   pEvent          Pointer to 8-byte LLCC event.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlccTxEvent(uint8_t *pEvent);

/*************************************************************************************************/
/*!
 * \brief   Transmit 32-bit LLCC event.
 *
 * \param   event           4-byte LLCC event.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlccTxEvent32(uint32_t event);

/*************************************************************************************************/
/*!
 * \brief   Receive 32-bit LLCC event.
 *
 * \param   pEvent          Storage for event.
 *
 * \return  TRUE if event received.
 */
/*************************************************************************************************/
bool_t LlccRxEvent32(uint32_t *pEvent);

#ifdef __cplusplus
};
#endif

#endif /* LLC_HCI_H */
