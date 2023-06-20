#include "dev_extFlash.h"
#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"

#include "am_mcu_apollo.h"
#ifdef AM_FREERTOS 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "event_groups.h"
#endif

#include "com_apollo2.h"
#include "drv_extFlash.h"



#define MOUDLE_LOG_ENABLED 							0
#define MOUDLE_DEBUG_ENABLED   					0
#define MOUDLE_NAME                     "[DEV_EXTFLASH]:"

#if MOUDLE_LOG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define LOG_PRINTF(...)                      SEGGER_RTT_printf(0, ##__VA_ARGS__) 
#define LOG_HEXDUMP(p_data, len)             rtt_log_hexdump(p_data, len) 
#else
#define LOG_PRINTF(...) 
#define LOG_HEXDUMP(p_data, len)             
#endif

#if MOUDLE_DEBUG_ENABLED == 1 && RTT_LOG_ENABLED == 1
#define DEBUG_PRINTF(...)                    SEGGER_RTT_printf(0, ##__VA_ARGS__)
#define DEBUG_HEXDUMP(p_data, len)           rtt_log_hexdump(p_data, len)
#else
#define DEBUG_PRINTF(...)   
#define DEBUG_HEXDUMP(p_data, len) 
#endif


#define FLASH_PAGE_SIZE W25Q128_PAGE_SIZE


static void buf_write( uint32_t addr, uint8_t* p_data, uint16_t len)
{
    uint8_t num_of_page = 0, num_of_single = 0, address = 0, count = 0, temp = 0;

    address = addr % FLASH_PAGE_SIZE;
    count = FLASH_PAGE_SIZE - address;
    num_of_page =  len / FLASH_PAGE_SIZE;
    num_of_single = len % FLASH_PAGE_SIZE;
    if (address == 0)
    {   
        if (num_of_page == 0) 
        {
            drv_extFlash_page_write(addr,p_data, len);
        }
        else 
        {
            while (num_of_page--)
            {
                drv_extFlash_page_write(addr,p_data, FLASH_PAGE_SIZE);
                addr +=  FLASH_PAGE_SIZE;
                p_data += FLASH_PAGE_SIZE;
            }
            drv_extFlash_page_write(addr,p_data, num_of_single);
        }
    }
    else 
    {
        if (num_of_page == 0)
        {
            if (num_of_single > count) 
            {
                temp = num_of_single - count;
                drv_extFlash_page_write(addr,p_data, count);
                addr +=  count;
                p_data += count;
                drv_extFlash_page_write(addr,p_data, temp);
            }
            else
            {
                drv_extFlash_page_write(addr,p_data, len);
            }
        }
        else 
        {
            len -= count;
            num_of_page =  len / FLASH_PAGE_SIZE;
            num_of_single = len % FLASH_PAGE_SIZE;

            drv_extFlash_page_write(addr,p_data, count);
            addr +=  count;
            p_data += count;

            while (num_of_page--)
            {
                drv_extFlash_page_write(addr,p_data, FLASH_PAGE_SIZE);
                addr +=  FLASH_PAGE_SIZE;
                p_data += FLASH_PAGE_SIZE;
            }

            if (num_of_single != 0)
            {
                drv_extFlash_page_write(addr,p_data, num_of_single);
            }
        }
    }
}


void dev_extFlash_write(uint32_t addr, uint8_t* p_data, uint16_t len)
{
	LOG_PRINTF(MOUDLE_NAME"dev_extFlash_write\n");
	buf_write(  addr,  p_data,  len);
	LOG_PRINTF(MOUDLE_NAME"dev_extFlash_write finish\n");
}



void dev_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len)
{
	LOG_PRINTF(MOUDLE_NAME"dev_extFlash_read\n");
	drv_extFlash_read( addr,  p_data,  len);
	LOG_PRINTF(MOUDLE_NAME"dev_extFlash_read finish\n");
}



void dev_extFlash_erase(uint32_t addr, uint32_t size)
{
	LOG_PRINTF(MOUDLE_NAME"dev_extFlash_erase\n");
	LOG_PRINTF(MOUDLE_NAME"addr : 0x%08x , size : 0x%08x\n",addr,size);
	
	uint8_t erase_type = 0;
	uint32_t remain_size = size;
	uint32_t address = addr;
	
	if((remain_size%W25Q128_SECTOR_SIZE) != 0)
	{
		remain_size += W25Q128_SECTOR_SIZE - remain_size%W25Q128_SECTOR_SIZE;
	}
	
	LOG_PRINTF(MOUDLE_NAME"remain_size --> 0x%08x\n",remain_size);

	while(true)
	{
		if (remain_size >= (64*1024) && ((address%W25Q128_64K_SIZE) == 0))
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_64K;
		}
		else if(remain_size >= (32*1024) && ((address%W25Q128_32K_SIZE) == 0))
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_32K;
		}
		else
		if(remain_size >= W25Q128_SECTOR_SIZE)
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_SECTOR;
		}		
	
		switch(erase_type)
		{
			case DEV_EXTFLASH_ERASE_TYPE_64K:{
				LOG_PRINTF(MOUDLE_NAME"DEV_EXTFLASH_ERASE_TYPE_64K\n");
				drv_extFlash_64K_erase(address);
				address += (64*1024);
				remain_size -= (64*1024);
			}break;
			case DEV_EXTFLASH_ERASE_TYPE_32K:{
				LOG_PRINTF(MOUDLE_NAME"DEV_EXTFLASH_ERASE_TYPE_32K\n");
				drv_extFlash_32K_erase(address);
				address += (32*1024);	
				remain_size -= (32*1024);
			}break;
			case DEV_EXTFLASH_ERASE_TYPE_SECTOR:{
				LOG_PRINTF(MOUDLE_NAME"DEV_EXTFLASH_ERASE_TYPE_SECTOR\n");
				drv_extFlash_sector_erase(address);
				address += (4*1024);
				remain_size -= (4*1024);
			}break;			
		}
		if(remain_size == 0)
		{
			break;
		}
	}

	

	LOG_PRINTF(MOUDLE_NAME"dev_extFlash_erase finish\n");
}


bool dev_extFlash_IDverify(uint8_t times)
{
	LOG_PRINTF(MOUDLE_NAME"dev_extFlash_IDverify\n");
	uint32_t id , i;
	drv_extFlash_enable();
	for(i=0;i<times;i++)
	{
		id = drv_extFlash_ID_read();
		
		if(id == W25Q128_REG_ID_VAL)
		{
			LOG_PRINTF(MOUDLE_NAME"SUCCESS\n");
			break;
		}
	}
	drv_extFlash_disable();
	if(i == times)
	{
		LOG_PRINTF(MOUDLE_NAME"FAILED\n");
//		ERR_HANDLER(ERR_NOT_FOUND);
		return false;
	}
	return true;
}


void dev_extFlash_enable(void)
{
	drv_extFlash_enable();
	#ifdef LAYER_APPLICATION 
	drv_extFlash_pwr_up();
	#endif	
}

void dev_extFlash_disable(void)
{
	#ifdef LAYER_APPLICATION 
	drv_extFlash_pwr_down();
	#endif
	drv_extFlash_disable();
}


void dev_extFlash_enter_deep_sleep(void)
{
	drv_extFlash_enable();
	drv_extFlash_pwr_down();
	drv_extFlash_disable();

}

void dev_extFlash_exit_deep_sleep(void)
{

	drv_extFlash_enable();
	drv_extFlash_pwr_up();
	drv_extFlash_disable();


}




