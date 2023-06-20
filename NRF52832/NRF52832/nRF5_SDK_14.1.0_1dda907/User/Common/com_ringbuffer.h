
#ifndef _COM_RINGBUFFER_
#define _COM_RINGBUFFER_

#include <stdint.h>
#include <stdbool.h>

struct ring_buffer_header
{
	uint8_t *buffer;
	uint32_t wr_idx;
	uint32_t rd_idx;
	uint32_t size;
	uint32_t mask;
};

bool ring_buffer_init(struct ring_buffer_header *header,
		uint8_t *buffer, uint32_t size);
bool ring_buffer_is_empty(struct ring_buffer_header *header);
bool ring_buffer_is_full(struct ring_buffer_header *header);
uint32_t ring_buffer_space_left(struct ring_buffer_header *header);
uint32_t ring_buffer_data_left(struct ring_buffer_header *header);

void ring_buffer_put(struct ring_buffer_header *header, uint8_t ch);
void ring_buffer_get(struct ring_buffer_header *header, uint8_t *ch);

uint32_t ring_buffer_write(struct ring_buffer_header *header,
		uint8_t *buf, uint32_t len);
uint32_t ring_buffer_read(struct ring_buffer_header *header,
		uint8_t *buf, uint32_t len);
#endif

