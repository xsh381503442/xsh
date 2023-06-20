#include "rtt_log.h"
#include "lib_error.h"
#include "SEGGER_RTT.h"   




#define RTT_RETRY_COUNTER                    10
#define HEXDUMP_BYTES_PER_LINE               16
#define HEXDUMP_HEXBYTE_AREA                 3 // Two bytes for hexbyte and space to separate
#define HEX_BUFFER_SIZE                      512


static void byte2hex(const uint8_t c, char * p_out)
{
	uint8_t  nibble;
	uint32_t i = 2;
	while (i-- != 0)
	{
			nibble       = (c >> (4 * i)) & 0x0F;
			p_out[1 - i] = (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble);
	}
}


static void serial_tx(uint8_t * p_buf, uint32_t len)
{
	uint32_t idx    = 0;
	uint32_t length = len;
	uint32_t processed;	
	uint32_t watchdog_counter = RTT_RETRY_COUNTER;
	do
	{
			processed = SEGGER_RTT_WriteNoLock(0, &p_buf[idx], length);
			idx += processed;
			length -= processed;
			if (processed == 0)
			{
					// If RTT is not connected then ensure that logger does not block
					watchdog_counter--;
					if (watchdog_counter == 0)
					{
							break;
					}
			}
	} while (length);	

}

uint32_t rtt_log_hexdump(const uint8_t *p_data, uint16_t len)
{
	if (p_data == NULL)
	{
			return ERR_NULL;
	}	
	if (len == 0)
	{
			return ERR_INVALID_LENGTH;
	}	
	
    char     str[HEX_BUFFER_SIZE];
    char   * p_hex_part;
    char   * p_char_part;
    uint8_t  c;
    uint32_t byte_in_line;
    uint32_t buffer_len    = 0;
    uint32_t byte_cnt      = 0;
		uint32_t length        = len;
	
    do
    {
        uint32_t hex_part_offset  = buffer_len;
        uint32_t char_part_offset = hex_part_offset + HEXDUMP_BYTES_PER_LINE*HEXDUMP_HEXBYTE_AREA + 1;
        p_hex_part  = &str[hex_part_offset];
        p_char_part = &str[char_part_offset];


        for (byte_in_line = 0; byte_in_line < HEXDUMP_BYTES_PER_LINE; byte_in_line++)
        {
            if (byte_cnt >= length)
            {
                // file the blanks
                *p_hex_part++  = ' ';
                *p_hex_part++  = ' ';
                *p_hex_part++  = ' ';
                *p_char_part++ = ' ';
            }
            else
            {

                c = p_data[byte_cnt];


                byte2hex(c, p_hex_part);
                p_hex_part    += 2; // move the pointer since byte in hex was added.
                *p_hex_part++  = ' ';
                *p_char_part++ = isprint(c) ? c : '.';
                byte_cnt++;
            }
        }
        *p_char_part++ = '\r';
        *p_char_part++ = '\n';
        *p_hex_part++  = ' ';
        buffer_len    += 0 +
                         (HEXDUMP_BYTES_PER_LINE * HEXDUMP_HEXBYTE_AREA + 1) + // space for hex dump and separator between hexdump and string
                         HEXDUMP_BYTES_PER_LINE +                              // space for stringS dump
                         2;                                                    // space for new line


        serial_tx((uint8_t *)str, buffer_len);

        buffer_len = 0;
    }
    while (byte_cnt < length);	

	return ERR_SUCCESS;
}
















