#ifndef __APP_SRV_ANCS_H__
#define __APP_SRV_ANCS_H__



#include "ble_db_discovery.h"

#define ATTR_APPID_SIZE             32
#define ATTR_TITLE_SIZE             32*2
#define ATTR_SUBTITLE_SIZE          32
#define ATTR_MESSAGE_SIZE           32*3
#define ATTR_MESSAGE_SIZE_SIZE      4
#define ATTR_DATE_SIZE              16

typedef enum __attribute__ ((packed))
{
	APP_MSG_TYPE_CALL = 1,
	APP_MSG_TYPE_WECHAT,
	APP_MSG_TYPE_QQ,
	APP_MSG_TYPE_SMS,
	
	APP_MSG_TYPE_CALL_IOS,
#ifdef COD
	APP_MSG_TYPE_COMPANY_WECHAT, //∆Û“µŒ¢–≈
	APP_MSG_TYPE_DINGDING,		 //∂§∂§
#endif	
}_app_msg_type;

typedef struct __attribute__ ((packed))
{
	uint32_t command;
	uint32_t crc;
	_app_msg_type type;
	uint8_t date_time[ATTR_DATE_SIZE];
	uint8_t title[ATTR_TITLE_SIZE];
	uint8_t subtitle[ATTR_SUBTITLE_SIZE];
	uint8_t message[ATTR_MESSAGE_SIZE];
	uint8_t message_size[ATTR_MESSAGE_SIZE_SIZE];
}_app_srv_ancs_msg;



void app_srv_ancs_ble_ancs_c_on_db_disc_evt(ble_db_discovery_evt_t * p_evt);
void app_srv_ancs_init(void);
void app_srv_ancs_notify_enable(void);
void app_srv_ancs_notify_disable(void);
void app_srv_ancs_msg_action_perform(uint32_t action);


#endif //__APP_SRV_ANCS_H__
