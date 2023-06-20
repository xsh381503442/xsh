#include "drv_ble_flash.h"
#include "nrf_delay.h"




#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 					2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME drv_ble_flash
#define NRF_LOG_LEVEL       LOG_LEVEL_INFO
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();



/* Array to map FDS return values to strings. */
char const * fds_err_str[] =
{
    "FDS_SUCCESS",
    "FDS_ERR_OPERATION_TIMEOUT",
    "FDS_ERR_NOT_INITIALIZED",
    "FDS_ERR_UNALIGNED_ADDR",
    "FDS_ERR_INVALID_ARG",
    "FDS_ERR_NULL_ARG",
    "FDS_ERR_NO_OPEN_RECORDS",
    "FDS_ERR_NO_SPACE_IN_FLASH",
    "FDS_ERR_NO_SPACE_IN_QUEUES",
    "FDS_ERR_RECORD_TOO_LARGE",
    "FDS_ERR_NOT_FOUND",
    "FDS_ERR_NO_PAGES",
    "FDS_ERR_USER_LIMIT_REACHED",
    "FDS_ERR_CRC_CHECK_FAILED",
    "FDS_ERR_BUSY",
    "FDS_ERR_INTERNAL",
};

/* Array to map FDS events to strings. */
static char const * fds_evt_str[] =
{
    "FDS_EVT_INIT",
    "FDS_EVT_WRITE",
    "FDS_EVT_UPDATE",
    "FDS_EVT_DEL_RECORD",
    "FDS_EVT_DEL_FILE",
    "FDS_EVT_GC",
};




#define BLE_CONFIG_FILE_ID 0xF010
#define BLE_CONFIG_REC_KEY 0x7010

#define BLE_IOS_ANCS_SWITCH_FILE_ID           0xE010
#define BLE_IOS_ANCS_SWITCH_REC_KEY           0x7010


#define BLE_ANDROID_MAC_FILE_ID           0xD010
#define BLE_ANDROID_MAC_START_REC_KEY     0x1010

#ifdef COD 
#define BLE_COD_MAC_FILE_ID           0xC010
#define BLE_COD_MAC_START_REC_KEY     0x1010
static fds_record_desc_t    m_cod_mac_record_desc = {0};
#endif

_android_bond g_android_bond_table[ANDROID_BOND_MAX];




static fds_record_desc_t    m_record_desc = {0};
static ble_config_params_t    m_config;

static fds_record_desc_t    m_android_mac_record_desc = {0};

static fds_record_desc_t    m_ancs_swtich_record_desc = {0};




_drv_ble_flash_ancs_switch g_ancs_switch;

static bool volatile m_fds_initialized = 0, m_fds_gc_done = 0;

static void fds_evt_handler(fds_evt_t const * const p_fds_evt)
{
    NRF_LOG_DEBUG("Event: %s received (%s)",
                  fds_evt_str[p_fds_evt->id],
                  fds_err_str[p_fds_evt->result]);

    switch (p_fds_evt->id)
    {
			
        case FDS_EVT_INIT:{
					NRF_LOG_DEBUG("FDS_EVT_INIT");
            if (p_fds_evt->result == FDS_SUCCESS)
            {
								NRF_LOG_DEBUG("FDS init success!");
                m_fds_initialized = true;

            }
            else
            {
                // Initialization failed.
                NRF_LOG_DEBUG("FDS init failed!");
                APP_ERROR_CHECK_BOOL(false);
            }
					}break;

        case FDS_EVT_WRITE:{
					NRF_LOG_DEBUG("FDS_EVT_WRITE");
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                if (p_fds_evt->write.file_id == BLE_CONFIG_FILE_ID)
                {
                    NRF_LOG_DEBUG("FDS write success! %d FileId: 0x%x RecKey:0x%x", p_fds_evt->write.is_record_updated,
																																										p_fds_evt->write.file_id,
																																										p_fds_evt->write.record_key);
                    
                }
            }
            else
            {
                // Initialization failed.
                NRF_LOG_DEBUG("FDS write failed!");
                APP_ERROR_CHECK_BOOL(false);
            }
        } break;

        case FDS_EVT_DEL_RECORD:{
					NRF_LOG_DEBUG("FDS_EVT_DEL_RECORD");
        } break;			
			
			

        case FDS_EVT_UPDATE:
					NRF_LOG_DEBUG("FDS_EVT_UPDATE");

            break;

        case FDS_EVT_DEL_FILE:
					NRF_LOG_DEBUG("FDS_EVT_DEL_FILE");
            break;

        case FDS_EVT_GC:{
					NRF_LOG_DEBUG("FDS_EVT_GC");
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_DEBUG("garbage collect success");
								m_fds_gc_done = true;
            }
				}break;

        default:
            NRF_LOG_DEBUG("FDS handler - %d - %d", p_fds_evt->id, p_fds_evt->result);
            APP_ERROR_CHECK(p_fds_evt->result);
            break;
    }


}

uint32_t drv_ble_flash_config_store(ble_config_params_t * p_config)
{
    uint32_t            err_code;
    fds_record_t        record;

    NRF_LOG_DEBUG("Storing configuration");


    memcpy(&m_config, p_config, sizeof(ble_config_params_t));

    // Set up record.
    record.file_id              = BLE_CONFIG_FILE_ID;
    record.key                  = BLE_CONFIG_REC_KEY;
		record.data.p_data          = &m_config;
		record.data.length_words    = sizeof(ble_config_params_t)/4;

    err_code = fds_record_update(&m_record_desc, &record);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

#include "nrf_fstorage.h"
ret_code_t drv_ble_flash_init(ble_config_params_t * p_default_config,
									 					  ble_config_params_t       ** p_config)
{
    ret_code_t err_code;
    fds_find_token_t     ftok = {0};
    fds_flash_record_t   flash_record = {0};

		err_code = fds_register(fds_evt_handler);
		VERIFY_SUCCESS(err_code);

		err_code = fds_init();
		VERIFY_SUCCESS(err_code);

    while (!m_fds_initialized)
    {
			__WFE();
    }
		err_code = fds_record_find(BLE_CONFIG_FILE_ID, BLE_CONFIG_REC_KEY, &m_record_desc, &ftok);
		
		if (err_code == FDS_SUCCESS)
    {
			NRF_LOG_INFO("Reading config file from flash storage");

        /* Open the record and read its contents. */
        err_code = fds_record_open(&m_record_desc, &flash_record);
        VERIFY_SUCCESS(err_code);

        /* Copy the configuration from flash into m_dummy_cfg. */
        memcpy(&m_config, flash_record.p_data, sizeof(ble_config_params_t));

        /* Close the record when done reading. */
        err_code = fds_record_close(&m_record_desc);
        VERIFY_SUCCESS(err_code);
				
			 *p_config = &m_config;
    }
    else
    {
        /* System config not found; write a new one. */
        NRF_LOG_INFO("Writing config file to flash storage");
        fds_record_t        record;
			
        memcpy(&m_config, p_default_config, sizeof(ble_config_params_t));

        // Set up record.
        record.file_id              = BLE_CONFIG_FILE_ID;
        record.key                  = BLE_CONFIG_REC_KEY;
        record.data.p_data          = &m_config;
        record.data.length_words    = sizeof(ble_config_params_t)/4;
			
        err_code = fds_record_write(&m_record_desc, &record);
			
				if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
				{
					err_code = fds_file_delete(BLE_CONFIG_FILE_ID);
					VERIFY_SUCCESS(err_code);	
					
					err_code = fds_gc();
					VERIFY_SUCCESS(err_code);		

					while (!m_fds_gc_done)
					{
						__WFE();
					}
					NVIC_SystemReset();
				}
				else{
					VERIFY_SUCCESS(err_code);
				}
			
        *p_config = &m_config;
    }	
		
//		if((*p_config)->dev_name.len>10 || (*p_config)->dev_name.len==0)
//		{
//			return NRF_ERROR_NO_MEM;
//		}
//		if((*p_config)->dev_name.name[10]!=0)
//		{
//			return NRF_ERROR_NO_MEM;
//		}		
		
    return NRF_SUCCESS;
}


ret_code_t drv_ble_flash_delete(void)
{
	fds_file_delete(BLE_CONFIG_FILE_ID);
	fds_gc();


	nrf_delay_ms(1000);	
	nrf_delay_ms(1000);	//give sometime for garbage collection

	return NRF_SUCCESS;
}



uint32_t drv_ble_flash_android_mac_init(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	fds_find_token_t     ftok = {0};
	fds_flash_record_t   flash_record = {0};	
		
	for(uint8_t i=0;i<ANDROID_BOND_MAX;i++)
	{
		err_code = fds_record_find(BLE_ANDROID_MAC_FILE_ID, BLE_ANDROID_MAC_START_REC_KEY+i, &m_android_mac_record_desc, &ftok);
		if (err_code == FDS_SUCCESS)
		{
//			NRF_LOG_INFO("FDS_SUCCESS");
			err_code = fds_record_open(&m_android_mac_record_desc, &flash_record);
			APP_ERROR_CHECK(err_code);
			memcpy(g_android_bond_table[i].mac, flash_record.p_data, BLE_GAP_ADDR_LEN);//6 bytes mac address + 2 word align
			err_code = fds_record_close(&m_android_mac_record_desc);
			APP_ERROR_CHECK(err_code);			
		}
		else
		{
//			NRF_LOG_INFO("FDS_ERR_NOT_FOUND");
			uint8_t empty_space[BLE_GAP_ADDR_LEN+2] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
			fds_record_t        record;
			record.file_id              = BLE_ANDROID_MAC_FILE_ID;
			record.key                  = BLE_ANDROID_MAC_START_REC_KEY + i;
			record.data.p_data          = empty_space;
			record.data.length_words    = 2;//6 bytes mac address + 2 word align
			err_code = fds_record_write(&m_android_mac_record_desc, &record);
			if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
			{
				NRF_LOG_ERROR("MAC INDEX --> %d",i);
				
				err_code = fds_file_delete(BLE_ANDROID_MAC_FILE_ID);
				VERIFY_SUCCESS(err_code);	
				
				err_code = fds_gc();
				VERIFY_SUCCESS(err_code);					
						
				while (!m_fds_gc_done)
				{
					__WFE();
				}
				NVIC_SystemReset();
			}	
			else{
				VERIFY_SUCCESS(err_code);
			}
			
			memset(g_android_bond_table[i].mac,0xFF,BLE_GAP_ADDR_LEN);	
		}
		
//		NRF_LOG_HEXDUMP_INFO(g_android_bond_table[i].mac,BLE_GAP_ADDR_LEN);	
		
	}

	
	
	
	return NRF_SUCCESS;
}




uint32_t drv_ble_flash_android_mac_write(uint8_t index, uint8_t *mac)
{
	ret_code_t err_code = NRF_SUCCESS;
	fds_record_t        record;
	fds_find_token_t     ftok = {0};
//	fds_flash_record_t   flash_record = {0};	
	// Set up record.
	record.file_id              = BLE_ANDROID_MAC_FILE_ID;
	record.key                  = BLE_ANDROID_MAC_START_REC_KEY + index;
	record.data.p_data          = mac;
	record.data.length_words    = 2;//6 bytes mac address + 2 word align

	NRF_LOG_HEXDUMP_DEBUG(mac,BLE_GAP_ADDR_LEN);

	
	err_code = fds_record_find(BLE_ANDROID_MAC_FILE_ID, BLE_ANDROID_MAC_START_REC_KEY+index, &m_android_mac_record_desc, &ftok);
	if (err_code == FDS_SUCCESS)
	{
		NRF_LOG_INFO("fds_record_update");
		err_code = fds_record_update(&m_android_mac_record_desc, &record);
//		APP_ERROR_CHECK(err_code);
	}else{
		NRF_LOG_INFO("fds_record_write");
		err_code = fds_record_write(&m_android_mac_record_desc, &record);
//		APP_ERROR_CHECK(err_code);		
	}
	return err_code;
}




uint32_t drv_ble_flash_android_mac_delete_all(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = fds_file_delete(BLE_ANDROID_MAC_FILE_ID);
	VERIFY_SUCCESS(err_code);	
	
	err_code = fds_gc();
	VERIFY_SUCCESS(err_code);	
	
	return err_code;
}


uint32_t drv_ble_flash_android_mac_delete(uint8_t index)
{
	ret_code_t err_code = NRF_SUCCESS;
//	fds_record_t        record;
	fds_find_token_t     ftok = {0};	
	
	
	err_code = fds_record_find(BLE_ANDROID_MAC_FILE_ID, BLE_ANDROID_MAC_START_REC_KEY+index, &m_android_mac_record_desc, &ftok);
	if (err_code == FDS_SUCCESS)
	{
		err_code = fds_record_delete(&m_android_mac_record_desc);
		VERIFY_SUCCESS(err_code);		
	}
	
	return err_code;
}



uint32_t drv_ble_flash_ancs_switch_read(_drv_ble_flash_ancs_switch *ancs_switch)
{
	uint32_t err_code = NRF_SUCCESS;
	fds_find_token_t     ftok = {0};
	fds_flash_record_t   flash_record = {0};	
	
	err_code = fds_record_find(BLE_IOS_ANCS_SWITCH_FILE_ID, BLE_IOS_ANCS_SWITCH_REC_KEY, &m_ancs_swtich_record_desc, &ftok);
	if (err_code == FDS_SUCCESS)
	{
		err_code = fds_record_open(&m_ancs_swtich_record_desc, &flash_record);
		VERIFY_SUCCESS(err_code);		
		memcpy((uint8_t *)ancs_switch, flash_record.p_data, 8);//6 bytes mac address + 2 word align
		err_code = fds_record_close(&m_ancs_swtich_record_desc);
		VERIFY_SUCCESS(err_code);
	}
	return err_code;
}


uint32_t drv_ble_flash_ancs_switch_write(_drv_ble_flash_ancs_switch *ancs_switch)
{
	ret_code_t err_code = NRF_SUCCESS;
	fds_record_t        record;
	fds_find_token_t     ftok = {0};

	record.file_id              = BLE_IOS_ANCS_SWITCH_FILE_ID;
	record.key                  = BLE_IOS_ANCS_SWITCH_REC_KEY;
	record.data.p_data          = (uint8_t *)ancs_switch;
	record.data.length_words    = 2;//6 bytes mac address + 2 word align

	err_code = fds_record_find(BLE_IOS_ANCS_SWITCH_FILE_ID, BLE_IOS_ANCS_SWITCH_REC_KEY, &m_ancs_swtich_record_desc, &ftok);
	if (err_code == FDS_SUCCESS)
	{
		err_code = fds_record_update(&m_ancs_swtich_record_desc, &record);
		VERIFY_SUCCESS(err_code);		
	}
	else
	{	
		err_code = fds_record_write(&m_ancs_swtich_record_desc, &record);
		if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
		{
			err_code = fds_record_delete(&m_ancs_swtich_record_desc);
			VERIFY_SUCCESS(err_code);		
			err_code = fds_gc();
			VERIFY_SUCCESS(err_code);
			err_code = fds_record_write(&m_ancs_swtich_record_desc, &record);	
			VERIFY_SUCCESS(err_code);

		}
		else
		{
			VERIFY_SUCCESS(err_code);
		}
	}
	return err_code;
}


uint32_t drv_ble_flash_ancs_switch_init(_drv_ble_flash_ancs_switch *ancs_switch)
{
	ret_code_t err_code = NRF_SUCCESS;
	fds_record_t        record;
	fds_find_token_t     ftok = {0};
	fds_flash_record_t   flash_record = {0};	

	record.file_id              = BLE_IOS_ANCS_SWITCH_FILE_ID;
	record.key                  = BLE_IOS_ANCS_SWITCH_REC_KEY;
	record.data.p_data          = (uint8_t *)ancs_switch;
	record.data.length_words    = 2;//6 bytes mac address + 2 word align

	err_code = fds_record_find(BLE_IOS_ANCS_SWITCH_FILE_ID, BLE_IOS_ANCS_SWITCH_REC_KEY, &m_ancs_swtich_record_desc, &ftok);
	if (err_code == FDS_ERR_NOT_FOUND)
	{
		NRF_LOG_INFO("FDS_ERR_NOT_FOUND");
		err_code = fds_record_write(&m_ancs_swtich_record_desc, &record);
		if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
		{
			err_code = fds_file_delete(BLE_IOS_ANCS_SWITCH_FILE_ID);
			VERIFY_SUCCESS(err_code);	
			
			err_code = fds_gc();
			VERIFY_SUCCESS(err_code);		
			
			while (!m_fds_gc_done)
			{
				__WFE();
			}
			NVIC_SystemReset();	
		}
		else{
			VERIFY_SUCCESS(err_code);
		}
	}
	else if(err_code == FDS_SUCCESS)
	{
		NRF_LOG_INFO("FDS_SUCCESS");
		err_code = fds_record_open(&m_ancs_swtich_record_desc, &flash_record);
		VERIFY_SUCCESS(err_code);		
		memcpy((uint8_t *)ancs_switch, flash_record.p_data, 8);//6 bytes mac address + 2 word align
		err_code = fds_record_close(&m_ancs_swtich_record_desc);
		VERIFY_SUCCESS(err_code);		
	}
	return NRF_SUCCESS;
}

#ifdef COD 

uint32_t drv_ble_flash_cod_mac_read(ble_gap_addr_t * device_addr)
{
	uint32_t err_code = NRF_SUCCESS;
	fds_find_token_t     ftok = {0};
	fds_flash_record_t   flash_record = {0};	
	
	err_code = fds_record_find(BLE_COD_MAC_FILE_ID, BLE_COD_MAC_START_REC_KEY, &m_cod_mac_record_desc, &ftok);
	if (err_code == FDS_SUCCESS)
	{
		err_code = fds_record_open(&m_cod_mac_record_desc, &flash_record);
		VERIFY_SUCCESS(err_code);		
		memcpy((uint8_t *)device_addr, flash_record.p_data, 7);//6 bytes mac address + 2 word align
		err_code = sd_ble_gap_addr_set(device_addr);
		NRF_LOG_INFO("drv_ble_flash_cod_mac_read err %x,id %x,type%x\r\n",err_code,device_addr->addr_id_peer,device_addr->addr_type);
		NRF_LOG_INFO("drv_ble_flash_cod_mac_read addr %x,%x,%x,%x,%x,%x\r\n",device_addr->addr[0],device_addr->addr[1],device_addr->addr[2],device_addr->addr[3],device_addr->addr[4],device_addr->addr[5]);
		err_code = fds_record_close(&m_cod_mac_record_desc);
		VERIFY_SUCCESS(err_code);
	}
	return err_code;
}


uint32_t drv_ble_flash_cod_mac_write(ble_gap_addr_t * device_addr)
{
	ret_code_t err_code = NRF_SUCCESS;
	fds_record_t        record;
	fds_find_token_t     ftok = {0};
	uint8_t temp[8];

	record.file_id              = BLE_COD_MAC_FILE_ID;
	record.key                  = BLE_COD_MAC_START_REC_KEY;
	record.data.p_data          = (uint8_t *)device_addr;
	record.data.length_words    = 2;//6 bytes mac address + 2 word align

	err_code = fds_record_find(BLE_COD_MAC_FILE_ID, BLE_COD_MAC_START_REC_KEY, &m_cod_mac_record_desc, &ftok);
	if (err_code == FDS_SUCCESS)
	{
		err_code = fds_record_update(&m_cod_mac_record_desc, &record);
		memcpy((uint8_t *)temp, (uint8_t *)record.data.p_data, 7);//6 bytes mac address + 2 word align
		NRF_LOG_INFO("fds_record_update %x,%x\r\n",temp[0],temp[1]);
		NRF_LOG_INFO("mac %x,%x,%x,%x,%x,%x\r\n",temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]);
		VERIFY_SUCCESS(err_code);		
	}
	else
	{	
		err_code = fds_record_write(&m_cod_mac_record_desc, &record);
		if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
		{
			
			err_code = fds_record_delete(&m_cod_mac_record_desc);
			VERIFY_SUCCESS(err_code);		
			err_code = fds_gc();
			VERIFY_SUCCESS(err_code);
			NRF_LOG_INFO("fds_record_write");
			err_code = fds_record_write(&m_cod_mac_record_desc, &record);	
			VERIFY_SUCCESS(err_code);

		}
		else
		{
			VERIFY_SUCCESS(err_code);
		}
	}
	return err_code;
}


uint32_t drv_ble_flash_cod_mac_init(ble_gap_addr_t * device_addr)
{
	ret_code_t err_code = NRF_SUCCESS;
	fds_record_t        record;
	fds_find_token_t     ftok = {0};
//	fds_flash_record_t   flash_record = {0};	

	record.file_id              = BLE_COD_MAC_FILE_ID;
	record.key                  = BLE_COD_MAC_START_REC_KEY;
	record.data.p_data          = (uint8_t *)device_addr;
	record.data.length_words    = 2;//6 bytes mac address + 2 word align

	err_code = fds_record_find(BLE_COD_MAC_FILE_ID, BLE_COD_MAC_START_REC_KEY, &m_cod_mac_record_desc, &ftok);
	if (err_code == FDS_ERR_NOT_FOUND)
	{
		NRF_LOG_INFO("FDS_ERR_NOT_FOUND");
		err_code = fds_record_write(&m_cod_mac_record_desc, &record);
		if(err_code == FDS_ERR_NO_SPACE_IN_FLASH)
		{
			err_code = fds_file_delete(BLE_COD_MAC_FILE_ID);
			VERIFY_SUCCESS(err_code);	
			
			err_code = fds_gc();
			VERIFY_SUCCESS(err_code);		
			
			while (!m_fds_gc_done)
			{
				__WFE();
			}
			NVIC_SystemReset();	
		}
		else{
			VERIFY_SUCCESS(err_code);
		}
	}
	#if 0 //
	else if(err_code == FDS_SUCCESS)
	{
		
		err_code = fds_record_open(&m_cod_mac_record_desc, &flash_record);
		VERIFY_SUCCESS(err_code);		
		memcpy((uint8_t *)device_addr, flash_record.p_data, 7);//6 bytes mac address + 2 word align
		//device_addr->addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
		err_code = sd_ble_gap_addr_set(device_addr);
		NRF_LOG_INFO("sd_ble_gap_addr_set err %x\r\n",err_code);
		err_code = fds_record_close(&m_cod_mac_record_desc);
		VERIFY_SUCCESS(err_code);		
	}
	#endif
	return NRF_SUCCESS;
}


#endif








