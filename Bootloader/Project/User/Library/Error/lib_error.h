#ifndef __LIB_ERROR_H__
#define __LIB_ERROR_H__


#include <stdint.h>




#define ERR_BASE_NUM      				(0x0)       
#define ERR_BOOT_BASE_NUM  				(0x1000)    
#define ERR_APP_BASE_NUM  				(0x2000)    
#define ERR_UPDATE_BASE_NUM  			(0x3000)    


#define ERR_SUCCESS                     (ERR_BASE_NUM + 0)   
#define ERR_INTERNAL                    (ERR_BASE_NUM + 1)  
#define ERR_NO_MEM                      (ERR_BASE_NUM + 2)  
#define ERR_NOT_FOUND                   (ERR_BASE_NUM + 3)  
#define ERR_NOT_SUPPORTED               (ERR_BASE_NUM + 4)  
#define ERR_INVALID_PARAM               (ERR_BASE_NUM + 5)  
#define ERR_INVALID_STATE               (ERR_BASE_NUM + 6)  
#define ERR_INVALID_LENGTH              (ERR_BASE_NUM + 7)  
#define ERR_INVALID_FLAGS               (ERR_BASE_NUM + 8) 
#define ERR_INVALID_DATA                (ERR_BASE_NUM + 9) 
#define ERR_DATA_SIZE                   (ERR_BASE_NUM + 10) 
#define ERR_TIMEOUT                     (ERR_BASE_NUM + 11) 
#define ERR_NULL                        (ERR_BASE_NUM + 12) 
#define ERR_FORBIDDEN                   (ERR_BASE_NUM + 13) 
#define ERR_INVALID_ADDR                (ERR_BASE_NUM + 14) 
#define ERR_BUSY                        (ERR_BASE_NUM + 15) 
#define ERR_CRC                         (ERR_BASE_NUM + 16) 


typedef struct
{
    uint16_t        line_num;    
    uint8_t const * p_file_name; 
    uint32_t        err_code;    
} error_info_t;


typedef uint32_t ret_code_t;

void error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);


#define ERR_HANDLER(ERR_CODE)                                    \
    do                                                                 \
    {                                                                  \
        error_handler((ERR_CODE), __LINE__, (uint8_t*) __FILE__);  \
    } while (0)



#define ERR_CHECK(ERR_CODE)                           \
    do                                                      \
    {                                                       \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);         \
        if (LOCAL_ERR_CODE != ERR_SUCCESS)                  \
        {                                                   \
            ERR_HANDLER(LOCAL_ERR_CODE);              \
        }                                                   \
    } while (0)
	












#endif //__LIB_ERROR_H__
