#ifndef APP_SRV_BDSF_H__
#define APP_SRV_BDSF_H__

#include <stdint.h>
#include "ble.h"



extern void bdsf_c_init(void);
extern void on_adv_report(const ble_evt_t * const p_ble_evt);
extern uint8_t find_adv_uuid(const ble_gap_evt_adv_report_t *p_adv_report, const uint16_t uuid_to_find);
extern uint8_t find_adv_name(ble_gap_evt_adv_report_t const * p_adv_report);


#endif				//APP_SRV_BDSF_H__
