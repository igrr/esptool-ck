/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "cbuf.h"

void cbuf_init(cbuf_t* cbuf, uint8_t* buf, size_t size)
{
    cbuf->buf = buf;
    cbuf->size = size;
    cbuf->bufend = buf + size;
    cbuf->begin = buf;
    cbuf->end = buf;
}

size_t cbuf_size(cbuf_t* cbuf)
{
    if (cbuf->end >= cbuf->begin)
        return cbuf->end - cbuf->begin;

    return cbuf->size - (cbuf->begin - cbuf->end);
}

size_t cbuf_room(cbuf_t* cbuf)
{
    if (cbuf->end >= cbuf->begin)
        return cbuf->size - (cbuf->end - cbuf->begin) - 1;

    return cbuf->begin - cbuf->end - 1;
}

int cbuf_peek(cbuf_t* cbuf)
{
    if (cbuf->end == cbuf->begin)
        return -1;

    return *cbuf->begin;
}

int cbuf_read(cbuf_t* cbuf)
{
    if (cbuf_size(cbuf) == 0)
        return -1;

    char result = *cbuf->begin;
    if (++(cbuf->begin) == cbuf->bufend)
        cbuf->begin = cbuf->buf;
    return result;
}

size_t cbuf_readn(cbuf_t* cbuf, char* dst, size_t size)
{
    size_t bytes_available = cbuf_size(cbuf);
    size_t size_to_read = (size < bytes_available) ? size : bytes_available;
    size_t size_read = size_to_read;
    if (cbuf->end < cbuf->begin && size_to_read > cbuf->bufend - cbuf->begin)
    {
        size_t top_size = cbuf->bufend - cbuf->begin;
        memcpy(dst, cbuf->begin, top_size);
        cbuf->begin = cbuf->buf;
        size_to_read -= top_size;
        dst += top_size;
    }
    memcpy(dst, cbuf->begin, size_to_read);
    cbuf->begin += size_to_read;
    if (cbuf->begin == cbuf->bufend)
        cbuf->begin = cbuf->buf;
    return size_read;
}

size_t cbuf_write(cbuf_t* cbuf, char c)
{
    if (cbuf_room(cbuf) == 0)
        return 0;

    *cbuf->end = c;
    if (++cbuf->end == cbuf->bufend)
        cbuf->end = cbuf->buf;
    return 1;
}

size_t cbuf_writen(cbuf_t* cbuf, const char* src, size_t size)
{
    size_t bytes_available = cbuf_room(cbuf);
    size_t size_to_write = (size < bytes_available) ? size : bytes_available;
    size_t size_written = size_to_write;
    if (cbuf->end > cbuf->begin && size_to_write > cbuf->bufend - cbuf->end)
    {
        size_t top_size = cbuf->bufend - cbuf->end;
        memcpy(cbuf->end, src, top_size);
        cbuf->end = cbuf->buf;
        size_to_write -= top_size;
        src += top_size;
    }
    memcpy(cbuf->end, src, size_to_write);
    cbuf->end += size_to_write;
    if (cbuf->end == cbuf->bufend)
        cbuf->end = cbuf->buf;
    return size_written;
}

void cbuf_flush(cbuf_t* cbuf)
{
    cbuf->begin = cbuf->buf;
    cbuf->end = cbuf->buf;
}
