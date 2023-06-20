#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "cmd_nrf52_srv.h"
#include "dev_apollo2.h"
#include "cmd_gpregret.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "drv_apollo2.h"
#include "cmd_apollo2.h"
#include "task_ble_conn.h"
#include "drv_ble_flash.h"
#include "nrf_ble_gatt.h"
#include "ble_conn_state.h"
#include "ble_db_discovery.h"
#include "ble_hrs_c.h"
#include "ble_rscs_c.h"
#include "app_srv_acces.h"
#include "ble_cscs_c.h"
#include "task_ble_accessary.h"
#include "peer_manager.h"

#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME app_srv_acces
#define NRF_LOG_LEVEL       LOG_LEVEL_OFF
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

//nrf defined device management struct
BLE_HRS_C_DEF(m_hrs_c);
BLE_RSCS_C_DEF(m_rscs_c);
BLE_CSCS_C_DEF(m_cscs_c);  

extern _accessory_heartrate g_accessory_heartrate;
extern _accessory_running g_accessory_running;
extern _accessory_cycling g_accessory_cycling;


static void app_hrs_c_evt_handler(ble_hrs_c_t * p_hrs_c, ble_hrs_c_evt_t * p_hrs_c_evt)
{

    switch (p_hrs_c_evt->evt_type)
    {
        case BLE_HRS_C_EVT_DISCOVERY_COMPLETE:
        {
            NRF_LOG_DEBUG("Heart rate service discovered.");
            if (g_accessory_heartrate.info.conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                ret_code_t err_code;
                NRF_LOG_INFO("HRS discovered on conn_handle 0x%x", g_accessory_heartrate.info.conn_handle);

                err_code = ble_hrs_c_handles_assign(p_hrs_c,
                                                    g_accessory_heartrate.info.conn_handle,
                                                    &p_hrs_c_evt->params.peer_db);
                APP_ERROR_CHECK(err_code);
                // Initiate bonding.
//                err_code = pm_conn_secure(g_accessory_heartrate.info.conn_handle, false);
//                if (err_code != NRF_ERROR_INVALID_STATE)
//                {
//                    APP_ERROR_CHECK(err_code);
//                }

                // Heart rate service discovered. Enable notification of Heart Rate Measurement.
                err_code = ble_hrs_c_hrm_notif_enable(p_hrs_c);
                APP_ERROR_CHECK(err_code);
							
								task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_DATA_TIMER_START, NULL, 0);	
							
            }
        } break;

        case BLE_HRS_C_EVT_HRM_NOTIFICATION:
        {
            NRF_LOG_INFO("Heart Rate = %d.", p_hrs_c_evt->params.hrm.hr_value);

						g_accessory_heartrate.data_valid = true;
						g_accessory_heartrate.hr_value = p_hrs_c_evt->params.hrm.hr_value;
					
					
//            if (p_hrs_c_evt->params.hrm.rr_intervals_cnt != 0)
//            {
//                uint32_t rr_avg = 0;
//                for (uint32_t i = 0; i < p_hrs_c_evt->params.hrm.rr_intervals_cnt; i++)
//                {
//                    rr_avg += p_hrs_c_evt->params.hrm.rr_intervals[i];
//                }
//                rr_avg = rr_avg / p_hrs_c_evt->params.hrm.rr_intervals_cnt;
//                NRF_LOG_DEBUG("rr_interval (avg) = %d.", rr_avg);
//            }
        } break;

        default:
            break;
    }
}

static void app_rscs_c_evt_handler(ble_rscs_c_t * p_rscs_c, ble_rscs_c_evt_t * p_rscs_c_evt)
{

	switch (p_rscs_c_evt->evt_type)
	{
			case BLE_RSCS_C_EVT_DISCOVERY_COMPLETE:{
            if (g_accessory_running.info.conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                ret_code_t err_code;
                NRF_LOG_INFO("Running Speed and Cadence service discovered on conn_handle 0x%x",
                             g_accessory_running.info.conn_handle);

                err_code = ble_rscs_c_handles_assign(p_rscs_c,
                                                    g_accessory_running.info.conn_handle,
                                                    &p_rscs_c_evt->params.rscs_db);
                APP_ERROR_CHECK(err_code);

                // Initiate bonding.
//                err_code = pm_conn_secure(g_accessory_running.info.conn_handle, false);
//                if (err_code != NRF_ERROR_INVALID_STATE)
//                {
//                    APP_ERROR_CHECK(err_code);
//                }

                // Running speed cadence service discovered. Enable notifications.
                err_code = ble_rscs_c_rsc_notif_enable(p_rscs_c);
                APP_ERROR_CHECK(err_code);
							
							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_DATA_TIMER_START, NULL, 0);	
							
            }
					}break;

			case BLE_RSCS_C_EVT_RSC_NOTIFICATION:
			{
				NRF_LOG_INFO("BLE_RSCS_C_EVT_RSC_NOTIFICATION");				
				g_accessory_running.data_valid = true;
				g_accessory_running.is_inst_stride_len_present = p_rscs_c_evt->params.rsc.is_inst_stride_len_present;
				g_accessory_running.is_total_distance_present = p_rscs_c_evt->params.rsc.is_total_distance_present;
				g_accessory_running.is_running = p_rscs_c_evt->params.rsc.is_running;
				g_accessory_running.inst_speed = p_rscs_c_evt->params.rsc.inst_speed;
				g_accessory_running.inst_cadence = p_rscs_c_evt->params.rsc.inst_cadence;
				g_accessory_running.inst_stride_length = p_rscs_c_evt->params.rsc.inst_stride_length;
				g_accessory_running.total_distance = p_rscs_c_evt->params.rsc.total_distance;
				
					NRF_LOG_DEBUG("");
					NRF_LOG_DEBUG("RSC Measurement received %d ",
									 p_rscs_c_evt->params.rsc.inst_speed);

					NRF_LOG_DEBUG("Instantanious Speed   = %d", p_rscs_c_evt->params.rsc.inst_speed);
					if (p_rscs_c_evt->params.rsc.is_inst_stride_len_present)
					{
							NRF_LOG_DEBUG("Stride Length         = %d",
										 p_rscs_c_evt->params.rsc.inst_stride_length);
					}
					if (p_rscs_c_evt->params.rsc.is_total_distance_present)
					{
							NRF_LOG_DEBUG("Total Distance = %u",
										 (unsigned int)p_rscs_c_evt->params.rsc.total_distance);
					}
					NRF_LOG_DEBUG("Instantanious Cadence = %d", p_rscs_c_evt->params.rsc.inst_cadence);
					NRF_LOG_DEBUG("Flags");
					NRF_LOG_DEBUG("  Stride Length Present = %d",
								 p_rscs_c_evt->params.rsc.is_inst_stride_len_present);
					NRF_LOG_DEBUG("  Total Distance Present= %d",
								 p_rscs_c_evt->params.rsc.is_total_distance_present);
					NRF_LOG_DEBUG("  Is Running            = %d", p_rscs_c_evt->params.rsc.is_running);
					
			}break;

			default:
					break;
	}
    
}



static void app_cscs_c_evt_handler(ble_cscs_c_t * p_csc_c, ble_cscs_c_evt_t * p_csc_c_evt)
{

    switch (p_csc_c_evt->evt_type)
    {
        case BLE_CSCS_C_EVT_DISCOVERY_COMPLETE:{
            if (g_accessory_cycling.info.conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                ret_code_t err_code;
                NRF_LOG_INFO("HRS discovered on conn_handle 0x%x", g_accessory_cycling.info.conn_handle);

                err_code = ble_cscs_c_handles_assign(p_csc_c,
                                                    g_accessory_cycling.info.conn_handle,
                                                    &p_csc_c_evt->params.cscs_db);
                APP_ERROR_CHECK(err_code);
                // Initiate bonding.
//                err_code = pm_conn_secure(g_accessory_cycling.info.conn_handle, false);
//                if (err_code != NRF_ERROR_INVALID_STATE)
//                {
//                    APP_ERROR_CHECK(err_code);
//                }

                // Heart rate service discovered. Enable notification of Heart Rate Measurement.
                err_code = ble_cscs_c_csc_notif_enable(p_csc_c);
                APP_ERROR_CHECK(err_code);
							
							task_ble_accessary_handle(TASK_BLE_ACCESSORY_EVT_DATA_TIMER_START, NULL, 0);	
							
            }
				}break;

        case BLE_CSCS_C_EVT_CSC_NOTIFICATION:{
					NRF_LOG_DEBUG("BLE_CSCS_C_EVT_CSC_NOTIFICATION");					
					g_accessory_cycling.data_valid = true;
					g_accessory_cycling.is_wheel_revolution_data_present = p_csc_c_evt->params.csc.is_wheel_revolution_data_present;
					g_accessory_cycling.is_crank_revolution_data_present = p_csc_c_evt->params.csc.is_crank_revolution_data_present;
					g_accessory_cycling.cumulative_wheel_revolutions = p_csc_c_evt->params.csc.cumulative_wheel_revolutions;
					g_accessory_cycling.last_wheel_event_time = p_csc_c_evt->params.csc.last_wheel_event_time;
					g_accessory_cycling.cumulative_crank_revolutions = p_csc_c_evt->params.csc.cumulative_crank_revolutions;
					g_accessory_cycling.last_crank_event_time = p_csc_c_evt->params.csc.last_crank_event_time;
					
					if (p_csc_c_evt->params.csc.is_wheel_revolution_data_present)
					{
						uint32_t wheel_rev = p_csc_c_evt->params.csc.cumulative_wheel_revolutions;
						uint16_t wheel_evt_time = p_csc_c_evt->params.csc.last_wheel_event_time;
						NRF_LOG_DEBUG("wheel_rev --> %d",wheel_rev);
						NRF_LOG_DEBUG("wheel_evt_time --> %d",wheel_evt_time);
					}
					if (p_csc_c_evt->params.csc.is_crank_revolution_data_present)
					{
						uint32_t crank_rev = p_csc_c_evt->params.csc.cumulative_crank_revolutions;
						uint16_t crank_evt_time = p_csc_c_evt->params.csc.last_crank_event_time;
						NRF_LOG_DEBUG("crank_rev --> %d",crank_rev);
						NRF_LOG_DEBUG("crank_evt_time --> %d",crank_evt_time);							
					}            
        }break;

        default:
            break;
    }
}




void app_srv_acces_c_init(void)
{
	ret_code_t err_code;
    //init hrs_c module
    memset(&m_hrs_c,0,sizeof(m_hrs_c));
    ble_hrs_c_init_t hrs_c_init;
    hrs_c_init.evt_handler = app_hrs_c_evt_handler;
    err_code = ble_hrs_c_init(&m_hrs_c, &hrs_c_init);
    APP_ERROR_CHECK(err_code);
	
	
	
    //init rscs_c module
    memset(&m_rscs_c,0,sizeof(m_rscs_c));
    ble_rscs_c_init_t rscs_c_init;
    rscs_c_init.evt_handler = app_rscs_c_evt_handler;
    err_code = ble_rscs_c_init(&m_rscs_c, &rscs_c_init);
	APP_ERROR_CHECK(err_code);
	
	
    
	memset(&m_cscs_c,0,sizeof(m_cscs_c));
	ble_cscs_c_init_t cscs_c_init;
    cscs_c_init.evt_handler = app_cscs_c_evt_handler;
    err_code = ble_cscs_c_init(&m_cscs_c, &cscs_c_init);
    APP_ERROR_CHECK(err_code);	
	
}


