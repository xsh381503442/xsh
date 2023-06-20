#ifndef __TASK_BLE_H__
#define __TASK_BLE_H__




#include <stdint.h>
#include <stdbool.h>
#include "watch_config.h"
#include "com_data.h"

#define BLE_DISCONNECT     1
#define BLE_CONNECT        2
#define BLE_BONDED         3

#ifdef WATCH_COD_BSP 
#define DEV_BLE_SYS_OFF_PIN            1
#else
#define DEV_BLE_SYS_OFF_PIN            37
#endif



#define TASK_BLE_CONN_DISCONNECT_REASON_WRONG_PASSKEY             1


/**@brief GAP Passkey Length. */
#define BLE_GAP_PASSKEY_LEN 6
typedef struct
{
	uint32_t command;
	uint8_t passkey[BLE_GAP_PASSKEY_LEN];
}_ble_passkey;


#define BLE_CONFIG_DEVICE_NAME_LEN_MAX 10
typedef struct __attribute__ ((packed))
{
    uint8_t name[BLE_CONFIG_DEVICE_NAME_LEN_MAX+1];
    uint8_t len;
}ble_config_dev_name_t;

typedef struct __attribute__ ((packed))
{

	uint32_t command;
	uint32_t setting_version;
	uint32_t bootloader_version;
	uint32_t application_version;
	uint32_t softdevice_version;
	uint32_t pcb_version;
	uint32_t mac_address_f4b;
	uint32_t mac_address_l2b;
	ble_config_dev_name_t      dev_name;
	uint32_t build_date;
	uint32_t build_time;
	
} _task_ble_info;



#define SCAN_CONTROLLER_HWT_TARGET_DEV_RSSI_SCAN           1

#define SCAN_DEV_NAME_LEN_MAX             10
#define SCAN_DEV_MAX                      3


typedef struct
{
  uint32_t interval;            /**< Scan interval between 0x0004 and 0x4000 in 0.625 ms units (2.5 ms to 10.24 s). */
  uint32_t window;              /**< Scan window between 0x0004 and 0x4000 in 0.625 ms units (2.5 ms to 10.24 s). */
  uint32_t timeout;             /**< Scan timeout between 0x0001 and 0xFFFF in seconds, 0x0000 disables timeout. */
} _scan_param;
#define BLE_GAP_ADDR_LEN (6)
typedef struct
{
  uint8_t addr_id_peer : 1;       /**< Only valid for peer addresses.
                                       Reference to peer in device identities list (as set with @ref sd_ble_gap_device_identities_set) when peer is using privacy. */
  uint8_t addr_type    : 7;       /**< See @ref BLE_GAP_ADDR_TYPES. */
  uint8_t addr[BLE_GAP_ADDR_LEN]; /**< 48-bit address, LSB format. */
} ble_gap_addr_t;

typedef struct{
	uint8_t name[SCAN_DEV_NAME_LEN_MAX + 1];
	uint8_t length;
} _scan_dev_name;


typedef struct{
	bool found;
	uint16_t	uuid;
	_scan_dev_name name;
	ble_gap_addr_t addr;	
	int8_t rssi;
} _scan_dev;


typedef struct{
	uint8_t	evt;
	_scan_param param;
	_scan_dev scan_dev[SCAN_DEV_MAX];
	
	
} _central_controller;



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
	APP_MSG_TYPE_COMPANY_WECHAT, //��ҵ΢��
	APP_MSG_TYPE_DINGDING,		 //����
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
}_app_srv_msg;

#define APP_NOTIFY_MSG_STATUS_ERASED           0xFF
#define APP_NOTIFY_MSG_STATUS_UNREAD           0xFE
#define APP_NOTIFY_MSG_STATUS_READ             0xFC

/*
�ⲿFLASH���Խ�BIT 1д�� 0�������ڲ���ȫ1���0λд0�󣬻������ڵ�һλд0��
*/

typedef struct __attribute__ ((packed))
{
	uint8_t status;									//��Ϣ״̬��������д��δ����Ϣ���Ѷ���Ϣ
	_app_msg_type type;													//��Ϣ���ͣ�1����	2΢��	3 QQ	4	����  6��ҵ΢�� 7����
	uint8_t date_time[ATTR_DATE_SIZE];								//ASCII�룬�ַ�������0����
	uint8_t title[ATTR_TITLE_SIZE];								//�����û������֣����룬���˺�����
	uint8_t subtitle[ATTR_SUBTITLE_SIZE];							//�����⣬ֻ��Call�У�������ϢΪ��
	uint8_t message[ATTR_MESSAGE_SIZE];								//GBK���룬�ַ�������0����
	uint8_t message_size[ATTR_MESSAGE_SIZE_SIZE];					//ASCII���룬�ַ�������0����
}_app_notification_msg;





#define APP_NOTIFY_MSG_NUM_MAX         50
#define APP_NOTIFY_MSG_SIZE_MAX        256



void task_ble_init(void);
void task_ble_uninit(void);



void dev_ble_init(uint8_t ble_switch);
void dev_ble_system_on(void);
void dev_ble_system_off(void);

#ifdef COD 
/**
 * �������
 */
typedef enum{
    ANSWER_PHONE_CALL = 1,      //������������Щ�ֻ�ʵ�ֲ��ˡ�
    REJECT_PHONE_CALL,          //�ܽ�
    IGNORE_PHONE_CALL,          //����
} cod_ble_phone_call_ctrl;
/**
 * ����������
 */
typedef enum{
    SENSOR_TYPE_ACC = 1,    //���ٶȻ�
    SENSOR_TYPE_GYRO,       //������
    SENSOR_TYPE_HEART,      //����
    SENSOR_TYPE_MAG,        //�ش�
    SENSOR_TYPE_PRESSURE,   //��ѹ��
} cod_ble_sensor_type;

/**
 * Sensor��������
 */
typedef struct {
    cod_ble_sensor_type type;  //����������
    uint8_t freq;              //���������ݵ�����Ƶ�ʣ��ɼ�Ƶ��һ��Ϊ 1hz, 50hz 100hz������һ��1Hz��6������һ��50Hz��
} cod_ble_sensor_req;

typedef struct {
   uint8_t sensor_start;
   uint8_t sensor_open;
   uint8_t acc_flag;
   uint8_t gyro_flag;
   uint8_t six_freq;
   uint8_t heart_flag;
   uint8_t heart_freq;
   uint16_t sequ_id;
} _cod_user_sensor_req;
/**
 * �˶�����ѵ���Ŀ���
 */
typedef enum{
    SPORT_CTRL_PAUSE = 1,      //��ͣ�˶�
    SPORT_CTRL_RESUME,         //�����˶�
    SPORT_CTRL_STOP,           //�����˶�
} cod_ble_sport_ctrl;



typedef struct __attribute__((packed))
{
	uint32_t cmd;
	uint8_t mac[6];	
}_ble_mac;
#define VICE_SPORT_PAUSE 0
#define VICE_SPORT_ING 1

#define VICE_TRAIN_PAUSE 0
#define VICE_TRAIN_ING 1


extern _cod_user_sensor_req cod_user_sensor_req;
extern _cod_user_training_cast cod_user_training_cast;
void ble_uart_ccommand_application_phone_call_ctrl(cod_ble_phone_call_ctrl ctrl);
void ble_uart_response_application_on_bind_device(bool is_success);
void ble_uart_response_application_sport_ctrl(cod_ble_sport_ctrl ctrl);
void ble_cod_sensor_data_trains(uint8_t *acc_data,uint8_t acc_len ,uint8_t *gyro_data,uint8_t gyro_len,uint8_t *heart_data,uint8_t heart_len);
void cod_user_sensor_close(void);
void ble_uart_response_application_real_time_log_trains(uint8_t *data,uint16_t len);


#endif

#ifdef WATCH_GPS_SNR_TEST
#include "drv_ublox.h"
extern void ble_uart_ccommand_application_send_gps_test_data(GPSMode *gps_data);
#endif




#endif //__TASK_BLE_H__
