#include "app_srv_ancs.h"
#include "ble_srv_wechat.h"
#include "nrf_ble_ancs_c.h"
#include "nrf_sdh.h"
#include "nrf_delay.h"
#include "task_ble_conn.h"
#include "drv_apollo2.h"
#include "cmd_apollo2.h"
#include "cmd_nrf52_srv.h"
#include "app_srv_nrf52.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#include "semphr.h"


#include "drv_ble_flash.h"

#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME app_srv_ancs
#define NRF_LOG_LEVEL       LOG_LEVEL_DEBUG
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


#define DELAY_MS(X)                 nrf_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X / ( ( TickType_t ) 1000 / 3 ) );}while(0)



BLE_ANCS_C_DEF(m_ancs_c); 

extern _drv_ble_flash_ancs_switch g_ancs_switch;

#define ATTR_DATA_SIZE                 BLE_ANCS_ATTR_DATA_MAX                       /**< Allocated size for attribute data. */

#if 0
static uint8_t m_attr_appid[ATTR_APPID_SIZE];                                        /**< Buffer to store attribute data. */
static uint8_t m_attr_title[ATTR_TITLE_SIZE];                                        /**< Buffer to store attribute data. */
static uint8_t m_attr_subtitle[ATTR_SUBTITLE_SIZE];                                     /**< Buffer to store attribute data. */
static uint8_t m_attr_message[ATTR_MESSAGE_SIZE];                                      /**< Buffer to store attribute data. */
static uint8_t m_attr_message_size[ATTR_MESSAGE_SIZE_SIZE];                                 /**< Buffer to store attribute data. */
static uint8_t m_attr_date[ATTR_DATE_SIZE];                                         /**< Buffer to store attribute data. */
#endif
//static uint8_t m_attr_posaction[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */
//static uint8_t m_attr_negaction[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */
//static uint8_t m_attr_disp_name[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */

ble_ancs_c_evt_notif_t g_notification_latest;                                /**< Local copy to keep track of the newest arriving notifications. */
ble_ancs_c_attr_t      g_notif_attr_latest;                                  /**< Local copy of the newest notification attribute. */
ble_ancs_c_attr_t      g_notif_attr_app_id_latest;                           /**< Local copy of the newest app attribute. */

_app_srv_ancs_msg g_app_srv_ancs_msg;


//static char const * lit_catid[BLE_ANCS_NB_OF_CATEGORY_ID] =
//{
//    "Other",
//    "Incoming Call",
//    "Missed Call",
//    "Voice Mail",
//    "Social",
//    "Schedule",
//    "Email",
//    "News",
//    "Health And Fitness",
//    "Business And Finance",
//    "Location",
//    "Entertainment"
//};


//static char const * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] =
//{
//    "Added",
////    "Modified",
////    "Removed"
//};

//static char const * lit_attrid[BLE_ANCS_NB_OF_NOTIF_ATTR] =
//{
//    "App Identifier",
//    "Title",
//    "Subtitle",
//    "Message",
//    "Message Size",
//    "Date",
//    "Positive Action Label",
//    "Negative Action Label"
//};

//static char const * lit_appid[BLE_ANCS_NB_OF_APP_ATTR] =
//{
//    "Display Name"
//};


static void ble_ancs_c_evt_notif_atrribute(ble_ancs_c_evt_t * p_evt) 	;	


//static void notif_print(ble_ancs_c_evt_notif_t * p_notif)
//{
//    NRF_LOG_DEBUG("Notification");
//    NRF_LOG_DEBUG("Event:       %s", (uint32_t)lit_eventid[p_notif->evt_id]);
//    NRF_LOG_DEBUG("Category ID: %s", (uint32_t)lit_catid[p_notif->category_id]);
//    NRF_LOG_DEBUG("Category Cnt:%u", (unsigned int) p_notif->category_count);
//    NRF_LOG_DEBUG("UID:         %u", (unsigned int) p_notif->notif_uid);

//    NRF_LOG_INFO("Flags:");
//    if (p_notif->evt_flags.silent == 1)
//    {
//        NRF_LOG_DEBUG(" Silent");
//    }
//    if (p_notif->evt_flags.important == 1)
//    {
//        NRF_LOG_DEBUG(" Important");
//    }
//    if (p_notif->evt_flags.pre_existing == 1)
//    {
//        NRF_LOG_DEBUG(" Pre-existing");
//    }
//    if (p_notif->evt_flags.positive_action == 1)
//    {
//        NRF_LOG_DEBUG(" Positive Action");
//    }
//    if (p_notif->evt_flags.negative_action == 1)
//    {
//        NRF_LOG_DEBUG(" Negative Action");
//    }
//}
//static void notif_attr_print(ble_ancs_c_attr_t * p_attr)
//{
//    if (p_attr->attr_len != 0)
//    {
//        NRF_LOG_DEBUG("%s: %s", (uint32_t)lit_attrid[p_attr->attr_id], nrf_log_push((char *)p_attr->p_attr_data));
//    }
//    else if (p_attr->attr_len == 0)
//    {
//        NRF_LOG_DEBUG("%s: (N/A)", (uint32_t)lit_attrid[p_attr->attr_id]);
//    }
//}
//static void app_attr_print(ble_ancs_c_attr_t * p_attr)
//{
//    if (p_attr->attr_len != 0)
//    {
//        NRF_LOG_DEBUG("%s: %s", (uint32_t)lit_appid[p_attr->attr_id], (uint32_t)p_attr->p_attr_data);
//    }
//    else if (p_attr->attr_len == 0)
//    {
//        NRF_LOG_DEBUG("%s: (N/A)", (uint32_t) lit_appid[p_attr->attr_id]);
//    }
//}
static void err_code_print(uint16_t err_code_np)
{
    switch (err_code_np)
    {
        case BLE_ANCS_NP_UNKNOWN_COMMAND:
            NRF_LOG_DEBUG("Error: Command ID was not recognized by the Notification Provider. ");
            break;

        case BLE_ANCS_NP_INVALID_COMMAND:
            NRF_LOG_DEBUG("Error: Command failed to be parsed on the Notification Provider. ");
            break;

        case BLE_ANCS_NP_INVALID_PARAMETER:
            NRF_LOG_DEBUG("Error: Parameter does not refer to an existing object on the Notification Provider. ");
            break;

        case BLE_ANCS_NP_ACTION_FAILED:
            NRF_LOG_DEBUG("Error: Perform Notification Action Failed on the Notification Provider. ");
            break;

        default:
            break;
    }
}


#define TIMER_INTERVAL            1000 
#define OSTIMER_WAIT_FOR_QUEUE              100
static TimerHandle_t m_timer; 

#define APP_SRV_ANCS_TIMER_EVT_ENABLE           1
#define APP_SRV_ANCS_TIMER_EVT_DISABLE          2
#define APP_SRV_ANCS_TIMER_EVT_NOTIFY           3
static uint8_t m_timer_evt = 0;

static void timeout_handler(TimerHandle_t xTimer)
{
    UNUSED_PARAMETER(xTimer);
    NRF_LOG_DEBUG("timeout_handler");
	
		switch(m_timer_evt)
		{
			case APP_SRV_ANCS_TIMER_EVT_ENABLE:{
				app_srv_ancs_notify_enable();
			}break;
			case APP_SRV_ANCS_TIMER_EVT_DISABLE:{
				app_srv_ancs_notify_disable();
			}break;			
			case APP_SRV_ANCS_TIMER_EVT_NOTIFY:{
				if(g_notification_latest.category_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL  || 
					 g_notification_latest.category_id == BLE_ANCS_CATEGORY_ID_SOCIAL  || //QQ WECHAT
					 #ifdef COD 
					 g_notification_latest.category_id == BLE_ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE  || //COMPANY_WECHAT,DINGDING
					 #endif
					 g_notification_latest.category_id == BLE_ANCS_CATEGORY_ID_OTHER  // SMS
					)
				{
		//			notif_print(&g_notification_latest);
					
					task_ble_conn_handle(TASK_BLE_CONN_ANCS_ATTR_REQUEST, NULL, 0);
				}			
			}break;			
			
		}
	
	

}


static void timer_create(void)
{
	m_timer = xTimerCreate("ANCS",
                         TIMER_INTERVAL,
                         pdFALSE,
                         NULL,
                         timeout_handler);
	if (NULL == m_timer)
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}
static void timer_start(uint32_t timeout_msec)
{
	if( xTimerChangePeriod( m_timer, timeout_msec , OSTIMER_WAIT_FOR_QUEUE ) == pdPASS )
	{
		NRF_LOG_DEBUG("The command was successfully sent.");
	}
	else
	{
		NRF_LOG_DEBUG("The command could not be sent, even after waiting for 100 ticks");
		NRF_LOG_DEBUG("to pass.  Take appropriate action here.");
	}		
	
}
static void timer_stop(void)
{
	if (pdPASS != xTimerStop(m_timer, OSTIMER_WAIT_FOR_QUEUE))
	{
			APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
	}	
}


void app_srv_ancs_notify_enable(void)
{
    ret_code_t ret;

    //nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.
	
    ret = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    ret = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("Notifications Enabled.");
}

void app_srv_ancs_notify_disable(void)
{
    ret_code_t ret;

    //nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    ret = ble_ancs_c_notif_source_notif_disable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    ret = ble_ancs_c_data_source_notif_disable(&m_ancs_c);
    APP_ERROR_CHECK(ret);	
}



static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
    ret_code_t ret = NRF_SUCCESS;
		static bool m_is_notified = false;
    switch (p_evt->evt_type)
    {
        case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:{
            NRF_LOG_INFO("Apple Notification Center Service discovered on the server.");
            ret = nrf_ble_ancs_c_handles_assign(&m_ancs_c, p_evt->conn_handle, &p_evt->service);
						APP_ERROR_CHECK(ret);
//						app_srv_ancs_notify_disable();
//						app_srv_ancs_notify_enable();
					
					
						uint8_t response[2] = {CMD_NRF52_SRV_ANCS_DISC_COMPLETE_RESPONSE,CMD_NRF52_SRV_SUCCUSS};
						app_srv_nrf52_control_send(response, sizeof(response));
						m_timer_evt = APP_SRV_ANCS_TIMER_EVT_ENABLE;
						timer_stop();
						timer_start(10000);						
					  if(!m_is_notified)
						{
							m_is_notified = true;
//							task_ble_conn_handle(TASK_BLE_CONN_EVT_ANCS_DISC_COMPLETE, NULL, 0);													
						}
				}break;

        case BLE_ANCS_C_EVT_NOTIF:{
					NRF_LOG_INFO("BLE_ANCS_C_EVT_NOTIF");
					if (p_evt->notif.category_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL ||
							p_evt->notif.category_id == BLE_ANCS_CATEGORY_ID_SOCIAL ||
							#ifdef COD 
							p_evt->notif.category_id == BLE_ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE ||
							#endif
							p_evt->notif.category_id == BLE_ANCS_CATEGORY_ID_OTHER
							)
					{
						m_timer_evt = APP_SRV_ANCS_TIMER_EVT_NOTIFY;
						timer_stop();
						timer_start(100);
						g_notification_latest = p_evt->notif;					
					}			

				}break;

        case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:{
					NRF_LOG_INFO("BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE");
						ble_ancs_c_evt_notif_atrribute(p_evt);
				

						
//						else
//						{
//							notif_attr_print(&g_notif_attr_latest);
//						}
//            if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
//            {
//                g_notif_attr_app_id_latest = p_evt->attr;
//            }
					}break;
        case BLE_ANCS_C_EVT_DISCOVERY_FAILED:{
            NRF_LOG_INFO("Apple Notification Center Service not discovered on the server.");
//						task_ble_conn_handle(TASK_BLE_CONN_EVT_ANCS_DISC_FAIL, NULL, 0);
//						uint8_t response[2] = {CMD_NRF52_SRV_ANCS_DISC_COMPLETE_RESPONSE,CMD_NRF52_SRV_ERROR_FAIL};
//						app_srv_nrf52_control_send(response, sizeof(response));					
				}break;

        case BLE_ANCS_C_EVT_APP_ATTRIBUTE:{
					NRF_LOG_INFO("BLE_ANCS_C_EVT_APP_ATTRIBUTE");
//            app_attr_print(&p_evt->attr);
				}break;
        case BLE_ANCS_C_EVT_NP_ERROR:{
					NRF_LOG_INFO("BLE_ANCS_C_EVT_NP_ERROR");
            err_code_print(p_evt->err_code_np);
				}break;
				
        case BLE_ANCS_C_EVT_DISCONNECT:{
					NRF_LOG_INFO("BLE_ANCS_C_EVT_DISCONNECT");
          m_is_notified = false;
				}break;				
				
				
        default:
            // No implementation needed.
            break;
    }
}
static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void app_srv_ancs_ble_ancs_c_on_db_disc_evt(ble_db_discovery_evt_t * p_evt)
{
    ble_ancs_c_on_db_disc_evt(&m_ancs_c, p_evt);
}


void app_srv_ancs_init(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    ret_code_t        ret;
	#if 1
	uint8_t m_attr_appid[ATTR_APPID_SIZE];										 /**< Buffer to store attribute data. */
	uint8_t m_attr_title[ATTR_TITLE_SIZE];										 /**< Buffer to store attribute data. */
	uint8_t m_attr_subtitle[ATTR_SUBTITLE_SIZE]; 									/**< Buffer to store attribute data. */
    uint8_t m_attr_message[ATTR_MESSAGE_SIZE];									   /**< Buffer to store attribute data. */
	uint8_t m_attr_message_size[ATTR_MESSAGE_SIZE_SIZE]; 								/**< Buffer to store attribute data. */
	uint8_t m_attr_date[ATTR_DATE_SIZE]; 										/**< Buffer to store attribute data. */
	#endif 

    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER,
                                  m_attr_appid,
                                  sizeof(m_attr_appid));
    APP_ERROR_CHECK(ret);

//    ret = nrf_ble_ancs_c_app_attr_add(&m_ancs_c,
//                                      BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME,
//                                      m_attr_disp_name,
//                                      sizeof(m_attr_disp_name));
//    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_TITLE,
                                  m_attr_title,
                                  sizeof(m_attr_title));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,
                                  m_attr_message,
                                  sizeof(m_attr_message));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
                                  m_attr_subtitle,
                                  sizeof(m_attr_subtitle));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
                                  m_attr_message_size,
                                  sizeof(m_attr_message_size));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_DATE,
                                  m_attr_date,
                                  sizeof(m_attr_date));
    APP_ERROR_CHECK(ret);

//    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                  BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,
//                                  m_attr_posaction,
//                                  ATTR_DATA_SIZE);
//    APP_ERROR_CHECK(ret);

//    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                  BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,
//                                  m_attr_negaction,
//                                  ATTR_DATA_SIZE);
//    APP_ERROR_CHECK(ret);

    ancs_init_obj.evt_handler   = on_ancs_c_evt;
    ancs_init_obj.error_handler = apple_notification_error_handler;

    ret = ble_ancs_c_init(&m_ancs_c, &ancs_init_obj);
    APP_ERROR_CHECK(ret);
		
		
		timer_create();
}


static void ble_ancs_c_evt_notif_atrribute(ble_ancs_c_evt_t * p_evt) 					
{
	static bool is_target_msssage = false;
	g_notif_attr_latest = p_evt->attr;
	if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
	{
		NRF_LOG_INFO("BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER");
		if (g_notif_attr_latest.attr_len != 0)
		{
			NRF_LOG_HEXDUMP_DEBUG(g_notif_attr_latest.p_attr_data, g_notif_attr_latest.attr_len);
		}			
		NRF_LOG_INFO("category_id --> 0x%02X",p_evt->notif.category_id);
		if (p_evt->notif.category_id == BLE_ANCS_CATEGORY_ID_OTHER)
		{
			//wechat or qq
			NRF_LOG_INFO("BLE_ANCS_CATEGORY_ID_OTHER");
			if (g_notif_attr_latest.attr_len != 0)
			{
				uint8_t *addr = NULL;
				if(g_ancs_switch.call == ANCS_MESSAGE_SWITCH_ON)
				{
					addr = (uint8_t*)strstr((const char *)g_notif_attr_latest.p_attr_data,"mobilephone");
					if(addr != NULL)
					{
						NRF_LOG_INFO("APP_MSG_TYPE_CALL_IOS");
						memset(&g_app_srv_ancs_msg,0,sizeof(g_app_srv_ancs_msg));
						g_app_srv_ancs_msg.type = APP_MSG_TYPE_CALL_IOS;
						is_target_msssage = true;								
					}						
				}				
				if(g_ancs_switch.wechat == ANCS_MESSAGE_SWITCH_ON)
				{
					addr = (uint8_t*)strstr((const char *)g_notif_attr_latest.p_attr_data,"tencent.xin");
					if(addr != NULL)
					{
						NRF_LOG_INFO("APP_MSG_TYPE_WECHAT");
						memset(&g_app_srv_ancs_msg,0,sizeof(g_app_srv_ancs_msg));
						g_app_srv_ancs_msg.type = APP_MSG_TYPE_WECHAT;
						is_target_msssage = true;							
					}						
				}					
				if(g_ancs_switch.qq == ANCS_MESSAGE_SWITCH_ON)
				{
					addr = (uint8_t*)strstr((const char *)g_notif_attr_latest.p_attr_data,"tencent.mqq");
					if(addr != NULL)
					{
						NRF_LOG_INFO("APP_MSG_TYPE_QQ");
						memset(&g_app_srv_ancs_msg,0,sizeof(g_app_srv_ancs_msg));
						g_app_srv_ancs_msg.type = APP_MSG_TYPE_QQ;
						is_target_msssage = true;							
					}						
				}					
				if(g_ancs_switch.sms == ANCS_MESSAGE_SWITCH_ON)
				{
					addr = (uint8_t*)strstr((const char *)g_notif_attr_latest.p_attr_data,"MobileSMS");
					if(addr != NULL)
					{
						NRF_LOG_INFO("APP_MSG_TYPE_SMS");
						memset(&g_app_srv_ancs_msg,0,sizeof(g_app_srv_ancs_msg));
						g_app_srv_ancs_msg.type = APP_MSG_TYPE_SMS;
						is_target_msssage = true;							
					}						
				}
				#ifdef COD
				if(g_ancs_switch.company_wechat == ANCS_MESSAGE_SWITCH_ON)
				{
					addr = (uint8_t*)strstr((const char *)g_notif_attr_latest.p_attr_data,"tencent.ww");
					if(addr != NULL)
					{
						NRF_LOG_INFO("APP_MSG_TYPE_COMPANY_WECHAT");
						memset(&g_app_srv_ancs_msg,0,sizeof(g_app_srv_ancs_msg));
						g_app_srv_ancs_msg.type = APP_MSG_TYPE_COMPANY_WECHAT;
						is_target_msssage = true;							
					}						
				}
				if(g_ancs_switch.dingding == ANCS_MESSAGE_SWITCH_ON)
				{
					addr = (uint8_t*)strstr((const char *)g_notif_attr_latest.p_attr_data,"DingTalk");
					if(addr != NULL)
					{
						NRF_LOG_INFO("APP_MSG_TYPE_COMPANY_WECHAT");
						memset(&g_app_srv_ancs_msg,0,sizeof(g_app_srv_ancs_msg));
						g_app_srv_ancs_msg.type = APP_MSG_TYPE_DINGDING;
						is_target_msssage = true;							
					}						
				}	
				#endif
			}					
		}
		else
		{
			is_target_msssage = false;
		}
	}		
	else if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_TITLE)
	{
		NRF_LOG_INFO("BLE_ANCS_NOTIF_ATTR_ID_TITLE");//微信：发消息人名 UTF-8
		
		if (g_notif_attr_latest.attr_len != 0 && is_target_msssage == true)
		{
			NRF_LOG_HEXDUMP_DEBUG(g_notif_attr_latest.p_attr_data, g_notif_attr_latest.attr_len);
			if(g_notif_attr_latest.attr_len > sizeof(g_app_srv_ancs_msg.title))
			{
				g_notif_attr_latest.attr_len = sizeof(g_app_srv_ancs_msg.title);
			}
			memcpy(g_app_srv_ancs_msg.title,g_notif_attr_latest.p_attr_data,g_notif_attr_latest.attr_len);
		}							
	}
	else if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE)
	{
		NRF_LOG_INFO("BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE");
		if (g_notif_attr_latest.attr_len != 0 && is_target_msssage == true)
		{
			NRF_LOG_HEXDUMP_DEBUG(g_notif_attr_latest.p_attr_data, g_notif_attr_latest.attr_len);
			if(g_notif_attr_latest.attr_len > sizeof(g_app_srv_ancs_msg.subtitle))
			{
				g_notif_attr_latest.attr_len = sizeof(g_app_srv_ancs_msg.subtitle);
			}
			memcpy(g_app_srv_ancs_msg.subtitle,g_notif_attr_latest.p_attr_data,g_notif_attr_latest.attr_len);			
		}							
	}		
	else if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE)
	{
		NRF_LOG_INFO("BLE_ANCS_NOTIF_ATTR_ID_MESSAGE");//英文数字unicode 中文UTF-8
		if (g_notif_attr_latest.attr_len != 0 && is_target_msssage == true)
		{
			NRF_LOG_HEXDUMP_DEBUG(g_notif_attr_latest.p_attr_data, g_notif_attr_latest.attr_len);
			if(g_notif_attr_latest.attr_len > sizeof(g_app_srv_ancs_msg.message))
			{
				g_notif_attr_latest.attr_len = sizeof(g_app_srv_ancs_msg.message);
			}
			memcpy(g_app_srv_ancs_msg.message,g_notif_attr_latest.p_attr_data,g_notif_attr_latest.attr_len);				
		}						
	}	
	else if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE)
	{
		NRF_LOG_INFO("BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE"); //unicode
		if (g_notif_attr_latest.attr_len != 0 && is_target_msssage == true)
		{
			NRF_LOG_HEXDUMP_DEBUG(g_notif_attr_latest.p_attr_data, g_notif_attr_latest.attr_len);
			if(g_notif_attr_latest.attr_len > sizeof(g_app_srv_ancs_msg.message_size))
			{
				g_notif_attr_latest.attr_len = sizeof(g_app_srv_ancs_msg.message_size);
			}
			memcpy(g_app_srv_ancs_msg.message_size,g_notif_attr_latest.p_attr_data,g_notif_attr_latest.attr_len);			
		}						
	}	
	else if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_DATE)
	{
		NRF_LOG_INFO("BLE_ANCS_NOTIF_ATTR_ID_DATE"); //unicode
		if (g_notif_attr_latest.attr_len != 0 && is_target_msssage == true)
		{
			NRF_LOG_HEXDUMP_DEBUG(g_notif_attr_latest.p_attr_data, g_notif_attr_latest.attr_len);
			is_target_msssage = false;
			if(g_notif_attr_latest.attr_len > sizeof(g_app_srv_ancs_msg.date_time))
			{
				g_notif_attr_latest.attr_len = sizeof(g_app_srv_ancs_msg.date_time);
			}
			memcpy(g_app_srv_ancs_msg.date_time,g_notif_attr_latest.p_attr_data,g_notif_attr_latest.attr_len);	
			task_ble_conn_handle(TASK_BLE_CONN_ANCS_MESSAGE_SEND, (uint8_t *)&g_app_srv_ancs_msg, sizeof(g_app_srv_ancs_msg));
		}							
	}		
	
}


void app_srv_ancs_msg_action_perform(uint32_t action)
{
	if(action == 0x01)
	{
		ret_code_t error = nrf_ancs_perform_notif_action(&m_ancs_c,
																										 g_notification_latest.notif_uid,
																										 ACTION_ID_NEGATIVE);		
		
	}
	else if(action == 0x02)
	{
		ret_code_t error = nrf_ancs_perform_notif_action(&m_ancs_c,
																										 g_notification_latest.notif_uid,
																										 ACTION_ID_POSITIVE);			
	
	}
}



