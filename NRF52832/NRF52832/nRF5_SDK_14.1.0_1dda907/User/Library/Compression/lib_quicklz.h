#ifndef __LIB_QUICKLZ_H__
#define __LIB_QUICKLZ_H__





#include <string.h>
#include <stdint.h>



#define QLZ_POINTERS 16


typedef struct 
{

	const unsigned char *offset[QLZ_POINTERS];
} qlz_hash_decompress;



typedef struct
{

	size_t stream_counter;
} qlz_state_decompress;







void qlz_decompress_buf(uint8_t *compress_buf, char *decompress_buf, uint32_t *decompress_size);





#endif //__LIB_QUICKLZ_H__
