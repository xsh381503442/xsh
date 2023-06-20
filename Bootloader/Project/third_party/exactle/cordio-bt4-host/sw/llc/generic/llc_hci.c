/*************************************************************************************************/
/*!
 *  \file   llc_hci.c
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

#include <stdlib.h>
#include <string.h>

#include "wsf_types.h"
#include "hci_defs.h"

#include "llc_llcc_defs.h"

#include "llc.h"
#include "llc_hci.h"
#include "platform.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief    LLCC state */
enum
{
  LLCC_STATE_IDLE = 0,                /*!< Awaiting reset. */
  LLCC_STATE_WAIT,                    /*!< Waiting for an ack from a command. */
  LLCC_STATE_READY,                   /*!< Ready to transmit or receive. */
};

/*! \brief    Types of LLCC commands. */
enum
{
  LLCC_CMD_SOFT_RESET     = 1,        /*!< Soft reset command. */
  LLCC_CMD_SOFT_RESET_ACK = 2,        /*!< Soft reset acknowledgement command. */
  LLCC_CMD_HCI_XFER_REQ   = 3,        /*!< HCI transfer request command. */
  LLCC_CMD_ERROR          = 4         /*!< Error command. */
};

/*! \brief    LLCC errors. */
enum
{
  LLCC_ERROR_ALLOC_FAIL = 1,          /*!< Allocation failed for receive. */
  LLCC_ERROR_BAD_XFER   = 2           /*!< Transfer request is bad. */
};

/*! \brief    LLCC command. */
typedef union
{
  /*!< Data. */
  struct
  {
    uint32_t              data[2];            /*!< Command data. */
  } data;

  /*!< Basic command. */
  struct
  {
    uint8_t               type;               /*!< Command type. */
    uint8_t               pad[7];             /*!< Padding. */
  } basic;

  /*!< Error command. */
  struct
  {
    uint8_t               type;               /*!< Command type. */
    uint8_t               error[7];           /*!< Error parameters. */
  } error;

  /*!< HCI transfer request. */
  struct
  {
    uint8_t               type;               /*!< Command type. */
    uint8_t               hciType;            /*!< HCI transfer type. */
    uint16_t              len;                /*!< Transfer length, in words. */
    uint8_t               align;              /*!< Transfer alignment. */
    uint8_t               pad[3];             /*!< Padding. */
  } hciXferReq;
} llccCmd_t;

/*! \brief      Transfer data. */
typedef struct
{
  uint8_t                 *pBuf;              /*!< Pointer to buffer. */
  uint8_t                 align;              /*!< Alignment of buffer. */
  uint16_t                len;                /*!< Buffer length, in words. */
  uint16_t                type;               /*!< Packet type. */
} llccXferData_t;

/*! \brief      Driver control block. */
typedef struct
{
  struct
  {
    volatile bool_t       rxBusyFlag;         /*!< Receive busy flag. */
    volatile bool_t       txBusyFlag;         /*!< Transmit busy flag. */
    volatile uint8_t      state;              /*!< Driver state. */

    void                  *pTxContext;        /*/  Transmit Context */
    llccXferData_t        tx;                 /*!< Transmit data. */
    llccXferData_t        rx;                 /*!< Receive data. */
    llccXferData_t        nextRx;             /*!< Next receive data. */

    llccRxEventCback_t    rxEventCback;       /*!< Received event callback. */
  } llcc;

  struct
  {
    bool_t                hciUp;              /*!< Indicates whether HCI is up or down. */
    const LlcHciCbacks_t *pCbacks;           /*!< Pointer to application callbacks. */
    uint8_t               *pRxBuf;            /*!< Pointer to receive buffer. */
  } hci;
} LlcHciCtrlBlk_t;

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief      Control block. */
static LlcHciCtrlBlk_t LlcHciCb;

/**************************************************************************************************
  Functions
**************************************************************************************************/

/*************************************************************************************************/
/*!
 * \brief   Write callback for LLCC driver.
 *
 * \param   type      Packet type.
 * \param   pData     Packet data.
 * \param   pContext  Context pointer.
 * \param   error     Error code.
 *
 * \return  None.
 */
/*************************************************************************************************/
static void LlcHciWrCback(uint8_t type, uint8_t *pData, void *pContext, int32_t error)
{
  LlcHciCb.hci.pCbacks->WriteDone(type, pData, error, pContext);
}

/*************************************************************************************************/
/*!
 * \brief   Allocation callback for LLCC driver.
 *
 * \param   len       Packet length, in bytes
 *
 * \return  Allocated memory.
 */
/*************************************************************************************************/
static void *LlcHciAllocCback(uint16_t len)
{
  return LlcHciCb.hci.pCbacks->BufAlloc(len);
}

/*************************************************************************************************/
/*!
 * \brief   Read callback for LLCC driver.
 *
 * \param   type      Packet type.
 * \param   pData     Packet data.
 * \param   align     Packet alignment, in bytes
 * \param   len       Packet length, in bytes
 *
 * \return  None.
 */
/*************************************************************************************************/
static void LlcHciRdCback(uint8_t type, uint8_t *pData, uint8_t align, uint16_t len)
{
  uint16_t lenAct = 0;
  switch (type)
  {
    case HCI_EVT_TYPE:
      lenAct = pData[align + 1] + HCI_EVT_HDR_LEN;
      break;
    case HCI_ACL_TYPE:
      lenAct = pData[align + 2] + (pData[align + 3] << 8) + HCI_ACL_HDR_LEN;
      break;
  }
  if (len > lenAct)
  {
    len = lenAct;
  }
  if (align > 0)
  {
    memmove(pData, pData + align, len - align);
  }
  LlcHciCb.hci.pCbacks->ReadDone(type, pData, len);
}

/*************************************************************************************************/
/*!
 *  \brief  Transmit a command over LLCC.
 *
 *  \param  pCmd        Pointer to command.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llccTxCmd(const llccCmd_t *pCmd)
{
  while (LLCC_TXD->REQUEST & LLCC_CMD_MASK)
  {
    ;
  }

  LLCC_TXD->CMD_DATA0 = pCmd->data.data[0];
  LLCC_TXD->CMD_DATA1 = pCmd->data.data[1];
}

/*************************************************************************************************/
/*!
 *  \brief  Transmit an event over LLCC.
 *
 *  \param  pEvent      Pointer to event.
 *
 *  \return None.
 */
/*************************************************************************************************/
static inline void llccTxEvent(const void *pEvent)
{
  uint32_t *pEvent32 = (uint32_t *)pEvent;
  while (LLCC_TXD->REQUEST & LLCC_EVT_MASK)
  {
    ;
  }

  LLCC_TXD->EVT_DATA0 = pEvent32[0];
  LLCC_TXD->EVT_DATA1 = pEvent32[1];
}

/*************************************************************************************************/
/*!
 *  \brief  Transmit a soft reset command over LLCC.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llccTxSoftResetCmd(void)
{
  llccCmd_t cmd;
  cmd.basic.type = LLCC_CMD_SOFT_RESET;
  llccTxCmd(&cmd);
}

/*************************************************************************************************/
/*!
 *  \brief  Transmit a soft reset acknowledgment command over LLCC.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llccTxSoftResetAckCmd(void)
{
  llccCmd_t cmd;
  cmd.basic.type = LLCC_CMD_SOFT_RESET_ACK;
  llccTxCmd(&cmd);
}

/*************************************************************************************************/
/*!
 *  \brief  Transmit a transfer request command over LLCC.
 *
 *  \param  type    Type of packet.
 *  \param  align   Alignment of packet.
 *  \param  len     Length of packet.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llccTxXferReqCmd(uint8_t type, uint8_t align, uint16_t len)
{
  llccCmd_t cmd;

  cmd.hciXferReq.type    = LLCC_CMD_HCI_XFER_REQ;
  cmd.hciXferReq.hciType = type;
  cmd.hciXferReq.align   = align;
  cmd.hciXferReq.len     = len;

  llccTxCmd(&cmd);
}

/*************************************************************************************************/
/*!
 *  \brief  Transmit an error command over LLCC.
 *
 *  \param  code0       First byte of error code.
 *  \param  code1       Second byte of error code.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llccTxErrorCmd(uint8_t code0, uint8_t code1)
{
  llccCmd_t cmd;

  cmd.error.type     = LLCC_CMD_ERROR;
  cmd.error.error[0] = code0;
  cmd.error.error[1] = code1;

  llccTxCmd(&cmd);
}

/*************************************************************************************************/
/*!
 *  \brief  Setup a receive over DMA.
 *
 *  \param  type        Type of packet.
 *  \param  align       Alignment of packet.
 *  \param  len         Length of packet.
 *
 *  \return TRUE if DMA set up.
 */
/*************************************************************************************************/
static bool_t llccDmaSetupRx(uint8_t type, uint8_t align, uint32_t len)
{
  LlcHciCb.llcc.rx.len   = len;
  LlcHciCb.llcc.rx.type  = type;
  LlcHciCb.llcc.rx.align = align;
  LlcHciCb.llcc.rx.pBuf  = LlcHciAllocCback(len * 4);
  if (LlcHciCb.llcc.rx.pBuf == NULL)
  {
    return FALSE;
  }

  PlatformWakeLockInc();
  LlcHciCb.llcc.rxBusyFlag = TRUE;

  /* Set up DMA, clearing everything out */
  DMAC_HCIR->BUF0_ADDR = 0;
  DMAC_HCIR->BUF0_CTRL = 0;
  DMAC_HCIR->BUF1_ADDR = 0;
  DMAC_HCIR->BUF1_CTRL = 0;
  DMAC_HCIR->PTR_ADDR  = 0;
  DMAC_HCIR->PTR_CTRL  = 0;
  DMAC_HCIR->BUF_STATE = DMAC_BUFSTATE_MT;    /* force init */
  DMAC_HCIR->NXT_ADDR  = (uint32_t)LlcHciCb.llcc.rx.pBuf;
  DMAC_HCIR->NXT_CTRL  = (len & DMAC_CHAN_COUNT_MASK);

  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Setup a transmit over DMA.
 *
 *  \param  pBuf    Pointer to packet.
 *  \param  len     Length of packet.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llccDmaSetupTx(const uint8_t *pBuf, uint32_t len)
{
  PlatformWakeLockInc();
  LlcHciCb.llcc.txBusyFlag = TRUE;

  /* Set up DMA, clearing everything out. */
  DMAC_HCIW->BUF0_ADDR = 0;
  DMAC_HCIW->BUF0_CTRL = 0;
  DMAC_HCIW->BUF1_ADDR = 0;
  DMAC_HCIW->BUF1_CTRL = 0;
  DMAC_HCIW->PTR_ADDR  = 0;
  DMAC_HCIW->PTR_CTRL  = 0;
  DMAC_HCIW->BUF_STATE = DMAC_BUFSTATE_MT;    /* force init */
  DMAC_HCIW->NXT_ADDR  = (uint32_t)pBuf;
  DMAC_HCIW->NXT_CTRL  = (len & DMAC_CHAN_COUNT_MASK);
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize the LLCC.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void llccInit(void)
{
  /* Disable all interrupts. */
  LlcDisableInterrupts();

  /* Disable all interrupt sources. */
  LLCC_RXD->INTENMASK_CLR = LLCC_CMD_IRQ_MASK | LLCC_EVT_IRQ_MASK; /* command path, RX */
  LLCC_TXD->INTENMASK_CLR = LLCC_CMD_IRQ_MASK | LLCC_EVT_IRQ_MASK; /* command path, TX */
  DMAC_HCIW->INTEN = 0;
  DMAC_HCIR->INTEN = 0;

  /* Flush out RX CMD path. */
  LLCC_RXD->CMD_DATA0;
  LLCC_RXD->CMD_DATA1;
  /* Flush out RX EVT path. */
  LLCC_RXD->EVT_DATA0;
  LLCC_RXD->EVT_DATA1;
  /* Flush out RX DMA path. */
  LLCC_RXD->DMAL_DATA0;
  LLCC_RXD->DMAL_DATA1;

  /* Force-init DMA engines. */
  DMAC_HCIW->BUF_STATE = DMAC_BUFSTATE_MT;
  DMAC_HCIR->BUF_STATE = DMAC_BUFSTATE_MT;

  /* Set up state. */
  memset(&LlcHciCb.llcc, 0, sizeof(LlcHciCb.llcc));

  /* Set up the LLCC interrupts. */
  LLCC_RXD->INTENMASK_SET = LLCC_CMD1_MASK; /* command path, RX */
  DMAC_HCIW->INTEN = 1;
  DMAC_HCIR->INTEN = 1;

  /* Enable all interrupts. */
  LlcEnableInterrupts();
}

/*************************************************************************************************/
/*!
 * \brief   Write packet to LLCC.
 *
 * \param   type      Packet type.
 * \param   pData     Packet data.
 * \param   len       Packet length, in bytes
 * \param   pContext  Context pointer that will be returned in the `WriteDone()` callback
 *
 * \return  None.
 */
/*************************************************************************************************/
static void llccWrite(uint32_t cmd, const uint8_t *pData, uint16_t len, void *pContext)
{
  uint8_t align = (uint32_t)pData & 3;

  /* Wait for channel to become free */
  while ((LlcHciCb.llcc.state != LLCC_STATE_READY) || LlcHciCb.llcc.txBusyFlag)
  {
    ;
  }

  /* Store pointer */
  LlcHciCb.llcc.tx.pBuf = (uint8_t *)pData;

  /* Round up to next multiple of 32 bits */
  len = (len + align + 3) / 4;
  if (align != 0)
  {
    pData -= align;
  }

  LlcHciCb.llcc.tx.type    = cmd;
  LlcHciCb.llcc.tx.align   = align;
  LlcHciCb.llcc.tx.len     = len;
  LlcHciCb.llcc.pTxContext = pContext;

  llccTxXferReqCmd(cmd, align, len);
  llccDmaSetupTx(pData, len);
}

/*************************************************************************************************/
/*!
 *  \brief  Handle LLCC received command valid interrupt.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LLCC_RXCMD_VALID_Handler(void)
{
  llccCmd_t cmd;

  /* Read command. */
  cmd.data.data[0] = LLCC_RXD->CMD_DATA0;
  cmd.data.data[1] = LLCC_RXD->CMD_DATA1;

  switch (cmd.basic.type)
  {
    case LLCC_CMD_SOFT_RESET:
    {
      llccInit();
      LlcHciCb.llcc.state = LLCC_STATE_READY;
      llccTxSoftResetAckCmd();
      break;
    }

    case LLCC_CMD_SOFT_RESET_ACK:
    {
      LlcHciCb.llcc.state = LLCC_STATE_READY;
      break;
    }

    case LLCC_CMD_HCI_XFER_REQ:
    {
      /* Zero-length transfers are impossible. */
      if (cmd.hciXferReq.len == 0)
      {
        llccTxErrorCmd(LLCC_ERROR_BAD_XFER, 0);
        break;
      }

      /* Pend this request if the last receive has not completed. */
      if (LlcHciCb.llcc.rxBusyFlag)
      {
        /* The peer should never send a second overlapped transfer. */
        if (LlcHciCb.llcc.nextRx.len == 0)
        {
          /* The DMA for the overlapped transfer will be setup after the current receive. */
          LlcHciCb.llcc.nextRx.align = cmd.hciXferReq.align;
          LlcHciCb.llcc.nextRx.type  = cmd.hciXferReq.hciType;
          LlcHciCb.llcc.nextRx.len   = cmd.hciXferReq.len;
        }
        else
        {
          llccTxErrorCmd(LLCC_ERROR_BAD_XFER, 0);
        }
      }
      else
      {
        /* Setup the DMA request. */
        if (!llccDmaSetupRx(cmd.hciXferReq.hciType, cmd.hciXferReq.align, cmd.hciXferReq.len))
        {
          /* The peer will retry the transfer later. */
          llccTxErrorCmd(LLCC_ERROR_ALLOC_FAIL, 0);
        }
      }
      break;
    }

    case LLCC_CMD_ERROR:
    {
      switch (cmd.error.error[0])
      {
        case LLCC_ERROR_BAD_XFER:
          break;

        /* The peer failed to allocate the buffer; retry the transfer. */
        case LLCC_ERROR_ALLOC_FAIL:
          /* Retry transfer request; the DMA is still setup. */
          llccTxXferReqCmd(LlcHciCb.llcc.tx.type, LlcHciCb.llcc.tx.align, LlcHciCb.llcc.tx.len);
          break;

        default:
          break;
      }
      break;
    }

    default:
      break;
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Handle LLCC received event valid interrupt.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LLCC_RXEVT_VALID_Handler(void)
{
  uint32_t eventData[2];

  eventData[0] = LLCC_RXD->EVT_DATA0;
  eventData[1] = LLCC_RXD->EVT_DATA1;
  if (LlcHciCb.llcc.rxEventCback != NULL)
  {
    LlcHciCb.llcc.rxEventCback((uint8_t *)&eventData[0]);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Handle LLCC transmit DMA done interrupt.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LLCC_TXDMAL_DONE_Handler(void)
{
  /* Ack the interrupt. */
  DMAC_HCIW->IRQSTATUS = 0;

  if (!LlcHciCb.llcc.txBusyFlag)
  {
    return;
  }

  /* HCI transfer completed. */
  LlcHciCb.llcc.txBusyFlag = FALSE;
  PlatformWakeLockDec();

  /* Notify stack of completed callback  */
  LlcHciWrCback(LlcHciCb.llcc.tx.type, LlcHciCb.llcc.tx.pBuf, LlcHciCb.llcc.pTxContext, LlcHciCb.llcc.tx.len);
}

/*************************************************************************************************/
/*!
 *  \brief  Handle LLCC receive DMA done interrupt.
 *
 *  \return None.
 */
/*************************************************************************************************/
void LLCC_RXDMAL_DONE_Handler(void)
{
  uint32_t xfer = (LlcHciCb.llcc.rx.len - (DMAC_HCIR->PTR_CTRL & DMAC_CHAN_COUNT_MASK)) * 4;

  /* Ack the interrupt. */
  DMAC_HCIR->IRQSTATUS = 0;

  if (!LlcHciCb.llcc.rxBusyFlag)
  {
    return;
  }

  /* HCI transfer completed. */
  LlcHciCb.llcc.rxBusyFlag = FALSE;
  PlatformWakeLockDec();

  LlcHciRdCback(LlcHciCb.llcc.rx.type, LlcHciCb.llcc.rx.pBuf, LlcHciCb.llcc.rx.align, xfer - LlcHciCb.llcc.rx.align);

  /* Set up the next transfer, if there's one pending */
  if (LlcHciCb.llcc.nextRx.len != 0)
  {
    if (llccDmaSetupRx(LlcHciCb.llcc.nextRx.type, LlcHciCb.llcc.nextRx.align, LlcHciCb.llcc.nextRx.len))
    {
      llccTxErrorCmd(LLCC_ERROR_ALLOC_FAIL, 0);
    }

    /* Always clear the overlapped request. */
    LlcHciCb.llcc.nextRx.len = 0;
  }
}

/*************************************************************************************************/
/*!
 * \brief   Initialize the HCI and prepare software for reading and writing.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlcHciInit(void)
{
  /* Clear control block. */
  LlcHciCb.hci.hciUp   = FALSE;
  LlcHciCb.hci.pCbacks = NULL;

  /* Power on/hold ESS in reset. */
  LlcReset(FALSE);
  PlatformDelayUsec(200);

  /* Set up the LLCC. */
  llccInit();

  /* Send a reset. */
  LlcHciCb.llcc.state = LLCC_STATE_WAIT;
  llccTxSoftResetCmd();
}

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
void LlcHciSetCbacks(const LlcHciCbacks_t *pCbacks)
{
  LlcHciCb.hci.pCbacks = pCbacks;
}

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
void LlcHciTakeUp(void)
{
  LlcReset(TRUE);
  LlcHciCb.hci.hciUp = TRUE;
}

/*************************************************************************************************/
/*!
 * \brief   Take down the interface (i.e., stop receiving messages and block transmissions).
 *
 * \return  None.
 *
 * HCI messages will no longer be received and transmissions (i.e., calls to `LlcHci_Write()`) will
 * be blocked.  The HCI callbacks can only safely be cleared after the interface is taken down.
 */
/*************************************************************************************************/
void LlcHciTakeDown(void)
{
  LlcHciCb.hci.hciUp = FALSE;
}

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
void LlcHciDisable(void)
{

}

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
void LlcHciWrite(uint8_t type, const uint8_t *pData, uint16_t len, void *pContext)
{
  if (LlcHciCb.hci.hciUp)
  {
    llccWrite(type, pData, len, pContext);
  }
}

/*************************************************************************************************/
/*!
 * \brief   Set callback for received LLCC event.
 *
 * \param   rxEventCback  Received LLCC event callback.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlccSetRxEventCback(llccRxEventCback_t rxEventCback)
{
  LlcHciCb.llcc.rxEventCback = rxEventCback;
  LLCC_RXD->INTENMASK_SET = LLCC_EVT1_MASK;
}

/*************************************************************************************************/
/*!
 * \brief   Transmit LLCC event.
 *
 * \param   pEvent          Pointer to 8-byte LLCC event.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlccTxEvent(uint8_t *pEvent)
{
  llccTxEvent(pEvent);
}

/*************************************************************************************************/
/*!
 * \brief   Transmit 32-bit LLCC event.
 *
 * \param   event           4-byte LLCC event.
 *
 * \return  None.
 */
/*************************************************************************************************/
void LlccTxEvent32(uint32_t event)
{
  LLCC_TXD->EVT_DATA1 = event;
}

/*************************************************************************************************/
/*!
 * \brief   Receive 32-bit LLCC event.
 *
 * \param   pEvent          Storage for event.
 *
 * \return  TRUE if event received.
 */
/*************************************************************************************************/
bool_t LlccRxEvent32(uint32_t *pEvent)
{
  if ((LLCC_RXD->REQUEST & LLCC_EVT1_MASK) == 0)
  {
    return FALSE;
  }

  *pEvent = LLCC_RXD->EVT_DATA1;
  return TRUE;
}
