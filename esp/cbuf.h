/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */
#ifndef __cbuf_h
#define __cbuf_h

#include <stdint.h>
#include <stddef.h>

typedef struct cbuf_ cbuf_t;

struct cbuf_ {
    size_t size;
    char*  buf;
    char*  bufend;
    char*  begin;
    char*  end;
};

void cbuf_init(cbuf_t* cbuf, uint8_t* buf, size_t size);   // space available for data will be size-1
size_t  cbuf_size(cbuf_t* cbuf);    // size of data in buffer
size_t  cbuf_room(cbuf_t* cbuf);    // free space in buffer
int    cbuf_peek(cbuf_t* cbuf);    // get a character without removing it
int    cbuf_read(cbuf_t* cbuf);
size_t  cbuf_readn(cbuf_t* cbuf, char* dst, size_t size);
size_t  cbuf_write(cbuf_t* cbuf, char c);
size_t  cbuf_writen(cbuf_t* cbuf, const char* src, size_t size);
void    cbuf_flush(cbuf_t* cbuf);   // clear buffer


#endif//__cbuf_h
