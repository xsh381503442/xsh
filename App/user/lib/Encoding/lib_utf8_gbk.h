#ifndef __LIB_UTF8_GBK_H__
#define __LIB_UTF8_GBK_H__



#include <stdint.h>





void lib_utf8_gbk(uint8_t *utf8_code,uint8_t *gbk_code,uint16_t *gbk_length);

void lib_utf16_gbk(uint8_t *utf16_code,uint16_t code_len,uint8_t *gbk_code,uint16_t *gbk_length);




#endif //__LIB_UTF8_GBK_H__
