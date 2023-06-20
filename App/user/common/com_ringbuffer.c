#include "com_ringbuffer.h"

bool ring_buffer_init(struct ring_buffer_header *header,
		uint8_t *buffer, uint32_t size)
{
    if (buffer && (size & (size - 1)) == 0) {
        header->buffer = buffer;
        header->wr_idx = 0;
        header->rd_idx = 0;
        header->size = size;
        header->mask = size - 1;

        return true;
    }

    return false;
}

bool ring_buffer_is_empty(struct ring_buffer_header *header)
{
    return (header->wr_idx == header->rd_idx);
}

bool ring_buffer_is_full(struct ring_buffer_header *header)
{
    return (header->size <=
            (header->wr_idx - header->rd_idx));
}

uint32_t ring_buffer_space_left(struct ring_buffer_header *header)
{
    return (header->size -
            (header->wr_idx - header->rd_idx));
}

uint32_t ring_buffer_data_left(struct ring_buffer_header *header)
{
    return (header->wr_idx - header->rd_idx);
}

static inline void _ring_buffer_put(struct ring_buffer_header *header,
        uint8_t ch)
{
    header->buffer[header->mask & header->wr_idx++] = ch;
}

void ring_buffer_put(struct ring_buffer_header *header, uint8_t ch)
{
    _ring_buffer_put(header, ch);
}

static inline void _ring_buffer_get(struct ring_buffer_header *header,
        uint8_t *ch)
{
    *ch = header->buffer[header->mask & header->rd_idx++];
}

void ring_buffer_get(struct ring_buffer_header *header, uint8_t *ch)
{
    _ring_buffer_get(header, ch);
}

uint32_t ring_buffer_write(struct ring_buffer_header *header,
        uint8_t *buf, uint32_t len)
{
    if (!buf || !len) {
        return 0;
    }

    if (len > ring_buffer_space_left(header)) {
        return 0;
    }

    for (uint32_t i = 0; i < len; i++) {
        ring_buffer_put(header, buf[i]);
    }

    return len;
}

uint32_t ring_buffer_read(struct ring_buffer_header *header,
		uint8_t *buf, uint32_t len)
{
    if (!buf || !len) {
        return 0;
    }

    if (len > ring_buffer_data_left(header)) {
        len = ring_buffer_data_left(header);
    }

    for (uint32_t i = 0; i < len; i++) {
        ring_buffer_get(header, buf + i);
    }

    return len;
}

