#include "lib_quicklz.h"
#include <stdlib.h>


#define MINOFFSET 2
#define UNCONDITIONAL_MATCHLEN 6
#define UNCOMPRESSED_END 4
#define CWORD_LEN 4




static uint32_t fast_read(void const *src, uint32_t bytes)
{

	unsigned char *p = (unsigned char*)src;
	switch (bytes)
	{
		case 4:
			return(*p | *(p + 1) << 8 | *(p + 2) << 16 | *(p + 3) << 24);
		case 3: 
			return(*p | *(p + 1) << 8 | *(p + 2) << 16);
		case 2:
			return(*p | *(p + 1) << 8);
		case 1: 
			return(*p);
	}
	return 0;

}



static uint32_t qlz_size_decompressed(const char *source)
{
	uint32_t n, r;
	n = (((*source) & 2) == 2) ? 4 : 1;
	r = fast_read(source + 1 + n, n);
	r = r & (0xffffffff >> ((4 - n)*8));
	return r;
}
static uint32_t qlz_size_header(const char *source)
{
	size_t n = 2*((((*source) & 2) == 2) ? 4 : 1) + 1;
	return n;
}
static uint32_t qlz_size_compressed(const char *source)
{
	uint32_t n, r;
	n = (((*source) & 2) == 2) ? 4 : 1;
	r = fast_read(source + 1, n);
	r = r & (0xffffffff >> ((4 - n)*8));
	return r;
}
static uint32_t memcpy_up(unsigned char *dst, const unsigned char *src, uint32_t n)
{
	// Caution if modifying memcpy_up! Overlap of dst and src must be special handled.

	uint32_t f = 0;
	do
	{
		*(uint32_t *)(dst + f) = *(uint32_t *)(src + f);


		f += MINOFFSET + 1;
	}
	while (f < n);


	
	return f;
}


static uint32_t qlz_decompress_core(const unsigned char *source, unsigned char *destination, size_t size, qlz_state_decompress *state)
{
	const unsigned char *src = source + qlz_size_header((const char *)source);
	unsigned char *dst = destination;
	const unsigned char *last_destination_byte = destination + size - 1;
	uint32_t cword_val = 1;
	const unsigned char *last_matchstart = last_destination_byte - UNCONDITIONAL_MATCHLEN - UNCOMPRESSED_END;
	unsigned char *last_hashed = destination - 1;
	const unsigned char *last_source_byte = source + qlz_size_compressed((const char *)source) - 1;
	static const uint32_t bitlut[16] = {4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

	
	(void) last_source_byte;
	(void) last_hashed;
	(void) state;
	for(;;) 
	{
		uint32_t fetch;

		if (cword_val == 1)
		{
			cword_val = fast_read(src, CWORD_LEN);
			src += CWORD_LEN;
		}

		fetch = fast_read(src, 4);

		if ((cword_val & 1) == 1)
		{
			uint32_t matchlen;
			const unsigned char *offset2;

			uint32_t offset;
			cword_val = cword_val >> 1;
			if ((fetch & 3) == 0)
			{
				offset = (fetch & 0xff) >> 2;
				matchlen = 3;
				src++;
			}
			else if ((fetch & 2) == 0)
			{
				offset = (fetch & 0xffff) >> 2;
				matchlen = 3;
				src += 2;
			}
			else if ((fetch & 1) == 0)
			{
				offset = (fetch & 0xffff) >> 6;
				matchlen = ((fetch >> 2) & 15) + 3;
				src += 2;
			}
			else if ((fetch & 127) != 3)
			{
				offset = (fetch >> 7) & 0x1ffff;
				matchlen = ((fetch >> 2) & 0x1f) + 2;
				src += 3;
			}
			else
			{
				offset = (fetch >> 15);
				matchlen = ((fetch >> 7) & 255) + 3;
				src += 4;
			}

			offset2 = dst - offset;

			
			memcpy_up(dst, offset2, matchlen);
			dst += matchlen;
		}
		else
		{
			if (dst < last_matchstart)
			{
				unsigned int n = bitlut[cword_val & 0xf];

				
				memcpy_up(dst, src, 4);

				cword_val = cword_val >> n;
				dst += n;
				src += n;

			}
			else
			{			

				while(dst <= last_destination_byte)
				{
					if (cword_val == 1)
					{
						src += CWORD_LEN;
						cword_val = 1U << 31;
					}


					*dst = *src;

					dst++;
					src++;
					cword_val = cword_val >> 1;

				}


				return size;
			}

		}
		

	}
}


static void reset_table_decompress(qlz_state_decompress *state)
{
	int i;
	(void)state;
	(void)i;
}
static uint32_t qlz_decompress(const char *source, void *destination, qlz_state_decompress *state)
{
	size_t dsiz = qlz_size_decompressed(source);


	
	if((*source & 1) == 1)
	{
		reset_table_decompress(state);
		dsiz = qlz_decompress_core((const unsigned char *)source, (unsigned char *)destination, dsiz, state);
	}

	state->stream_counter = 0;
	reset_table_decompress(state);
	

	return dsiz;
}



void qlz_decompress_buf(uint8_t *compress_buf, char *decompress_buf, uint32_t *decompress_size)
{
	uint32_t len = 0;
	qlz_state_decompress state_decompress;
	memset(&state_decompress,0,sizeof(state_decompress));
	
	len = qlz_size_decompressed((char*)compress_buf);

	len = qlz_decompress((char*)compress_buf, decompress_buf, &state_decompress);

	*decompress_size = len;
}






