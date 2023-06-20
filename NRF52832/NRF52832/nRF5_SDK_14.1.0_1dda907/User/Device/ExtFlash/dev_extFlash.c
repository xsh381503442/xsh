#include "dev_extFlash.h"
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "drv_extFlash.h"
#include "nrf_delay.h"



#define LOG_LEVEL_OFF 							0
#define LOG_LEVEL_ERROR 						1
#define LOG_LEVEL_WARNING 						2
#define LOG_LEVEL_INFO 							3
#define LOG_LEVEL_DEBUG 						4
#define NRF_LOG_MODULE_NAME 					dev_extFlash
#define NRF_LOG_LEVEL       					LOG_LEVEL_INFO
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();



#define EXTFLASH_PAGE_SIZE       0x100
#define EXTFLASH_SECTOR_SIZE     0x1000
#define EXTFLASH_32K_SIZE        0x8000
#define EXTFLASH_64K_SIZE        0x10000

#define DEV_EXTFLASH_ERASE_TYPE_SECTOR                 1
#define DEV_EXTFLASH_ERASE_TYPE_32K                 	 2
#define DEV_EXTFLASH_ERASE_TYPE_64K                    3
#define DEV_EXTFLASH_ERASE_TYPE_CHIP                   4



#define DELAY_MS(X)                 nrf_delay_ms(X)
//#define DELAY_MS(X)                 vTaskDelay(X)
//#define DELAY_MS(X)                   do{TickType_t xLastWakeTime;xLastWakeTime = xTaskGetTickCount ();vTaskDelayUntil( &xLastWakeTime, X );}while(0)




static void buf_write( uint32_t addr, uint8_t* p_data, uint16_t len)
{
    uint8_t num_of_page = 0, num_of_single = 0, address = 0, count = 0, temp = 0;

    address = addr % EXTFLASH_PAGE_SIZE;
    count = EXTFLASH_PAGE_SIZE - address;
    num_of_page =  len / EXTFLASH_PAGE_SIZE;
    num_of_single = len % EXTFLASH_PAGE_SIZE;
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
                drv_extFlash_page_write(addr,p_data, EXTFLASH_PAGE_SIZE);
                addr +=  EXTFLASH_PAGE_SIZE;
                p_data += EXTFLASH_PAGE_SIZE;
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
            num_of_page =  len / EXTFLASH_PAGE_SIZE;
            num_of_single = len % EXTFLASH_PAGE_SIZE;

            drv_extFlash_page_write(addr,p_data, count);
            addr +=  count;
            p_data += count;

            while (num_of_page--)
            {
                drv_extFlash_page_write(addr,p_data, EXTFLASH_PAGE_SIZE);
                addr +=  EXTFLASH_PAGE_SIZE;
                p_data += EXTFLASH_PAGE_SIZE;
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
	NRF_LOG_DEBUG("dev_extFlash_write");
	buf_write(  addr,  p_data,  len);
	NRF_LOG_DEBUG("dev_extFlash_write finish");
}

void dev_extFlash_read(uint32_t addr, uint8_t* p_data, uint16_t len)
{
	NRF_LOG_DEBUG("dev_extFlash_read");	
	drv_extFlash_read( addr, p_data,  len);
	NRF_LOG_DEBUG("dev_extFlash_read finish");
}

void dev_extFlash_erase(uint32_t addr, uint32_t size)
{
	NRF_LOG_DEBUG("dev_extFlash_erase");
	NRF_LOG_DEBUG("addr : 0x%08x , size : 0x%08x",addr,size);
	
	uint8_t erase_type = 0;
	uint32_t remain_size = size;
	uint32_t address = addr;
	
	if((remain_size%EXTFLASH_SECTOR_SIZE) != 0)
	{
		remain_size += EXTFLASH_SECTOR_SIZE - remain_size%EXTFLASH_SECTOR_SIZE;
	}
	
	NRF_LOG_DEBUG("remain_size --> 0x%08x",remain_size);
	

	while(true)
	{
		if (remain_size >= (64*1024) && ((address%EXTFLASH_64K_SIZE) == 0))
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_64K;
		}
		else if(remain_size >= (32*1024) && ((address%EXTFLASH_32K_SIZE) == 0))
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_32K;
		}
		else if(remain_size >= EXTFLASH_SECTOR_SIZE)
		{
			erase_type = DEV_EXTFLASH_ERASE_TYPE_SECTOR;
		}		
	
		switch(erase_type)
		{
			case DEV_EXTFLASH_ERASE_TYPE_64K:{
				NRF_LOG_DEBUG("DEV_EXTFLASH_ERASE_TYPE_64K");
				drv_extFlash_64K_erase(address);
				address += (64*1024);
				remain_size -= (64*1024);
			}break;
			case DEV_EXTFLASH_ERASE_TYPE_32K:{
				NRF_LOG_DEBUG("DEV_EXTFLASH_ERASE_TYPE_32K");
				drv_extFlash_32K_erase(address);
				address += (32*1024);	
				remain_size -= (32*1024);
			}break;
			case DEV_EXTFLASH_ERASE_TYPE_SECTOR:{
				NRF_LOG_DEBUG("DEV_EXTFLASH_ERASE_TYPE_SECTOR");
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

	NRF_LOG_DEBUG("dev_extFlash_erase finish");
}


void dev_extFlash_IDverify(void)
{
	NRF_LOG_DEBUG("dev_extFlash_IDverify");
	drv_extFlash_enable();
	drv_extFlash_readID();
	drv_extFlash_disable();
	
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


