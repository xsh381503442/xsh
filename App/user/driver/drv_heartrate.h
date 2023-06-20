#ifndef _DRV_HEARTRATE_H__
#define _DRV_HEARTRATE_H__


#include <stdint.h>
#include <stdbool.h>
#include "drv_config.h"
#define PACKET_HDR_PREFIX       (0x00)
#define PACKET_HDR              (0x81)

/*COMMAND DEFINITION*/
#define CMD_STATE               (0x50)
#define CMD_CONFIG              (0x56)
#define CMD_CONFIG_VERSION      (0x01)
#define CMD_CONFIG_SERIAL       (0x03)

/*RESPONSE DEFINITION*/
#define RESP_PACKET_ACK         (0x50)
#define RESP_HEADER_INCORRECT   (0x51)
#define RESP_CRC_INCORRECT      (0x52)
#define RESP_PACKET_ZERO        (0x53)
#define RESP_PACKET_TOO_BIG     (0x54)

#define HDR_BUFFER_SIZE         256
#define DRV_HDR_UART_TX_BUF_SIZE                  (256)
#define DRV_HDR_UART_RX_BUF_SIZE                  (256)
#define DRV_HRD_EVT_UART_DATA   0

typedef void (* hdr_callback)(uint32_t evt, uint8_t *p_data, uint32_t length);
extern void drv_hdr_init(hdr_callback hdr_cb);
extern void drv_hdr_uninit(void);
extern void drv_receive_handler(uint8_t *p_data, uint8_t length);

extern bool drv_receivePacket(void);
extern void drv_heartrate_init(void);
extern void drv_heartrate_uninit(void);
extern void drv_getHeartrateVersion(char strVersion[],uint8_t length);
extern uint8_t drv_getHdrValue(void);
extern void drv_clearHdrValue(void);
extern void drv_sendHeartrateMetricData(void);
extern void drv_sendHdrHandler(void);
#endif


