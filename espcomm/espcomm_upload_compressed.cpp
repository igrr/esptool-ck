/**********************************************************************************
 **********************************************************************************
 ***
 ***    espcomm_upload_compressed.c
 ***    - upload compressed image to ESP
 ***
 ***    Copyright (C) 2015 Ivan Grokhotkov <ivan@esp8266.com>
 ***
 ***    This program is free software; you can redistribute it and/or modify
 ***    it under the terms of the GNU General Public License as published by
 ***    the Free Software Foundation; either version 2 of the License, or
 ***    (at your option) any later version.
 ***
 ***    This program is distributed in the hope that it will be useful,
 ***    but WITHOUT ANY WARRANTY; without even the implied warranty of
 ***    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ***    GNU General Public License for more details.
 ***
 ***    You should have received a copy of the GNU General Public License along
 ***    with this program; if not, write to the Free Software Foundation, Inc.,
 ***    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ***
 **/

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

extern "C" {
#include "infohelper.h"
#include "espcomm.h"
#include "serialport.h"
#include "espcomm_boards.h"
#include "delay.h"
}

#include <memory>

#include "miniz-config-host.h"
#include "miniz.c"


static tinfl_decompressor s_decompressor;
static uint8_t s_output_buf[TINFL_LZ_DICT_SIZE];


static uint8_t* cmp_ptr = 0;
void flush_output(size_t size) {
    LOGDEBUG("flushing output (%d)", size);
    if (memcmp(cmp_ptr, s_output_buf, size) != 0) {
        LOGERR("memcmp error!");
    }
    cmp_ptr += size;
}

void decompress_block(uint8_t* compressed, size_t compressed_size,
                      size_t* dst_offset, bool last)
{
    size_t src_offset = 0;
    size_t written = 0;
    int flag = (last)?0:TINFL_FLAG_HAS_MORE_INPUT;
    while (true) {
        size_t in_buf_size = compressed_size - src_offset;
        size_t dst_buf_size = sizeof(s_output_buf) - *dst_offset;
        LOGDEBUG("src offset: %u, dst offset: %u, flag: %d", src_offset, *dst_offset, flag);
        LOGDEBUG("before: in_buf_size: %u, dst_buf_size: %u", in_buf_size, dst_buf_size);
        tinfl_status status = tinfl_decompress(&s_decompressor,
            (const mz_uint8*)compressed + src_offset, &in_buf_size,
            s_output_buf, s_output_buf + *dst_offset, &dst_buf_size, flag);

        src_offset += in_buf_size;
        written += dst_buf_size;
        LOGDEBUG("after: status: %d in_buf_size: %u, dst_buf_size: %u", status, in_buf_size, dst_buf_size);
        if (dst_buf_size) {
            LOGDEBUG("decompressed %u -> %u", in_buf_size, dst_buf_size);
        }
        *dst_offset += dst_buf_size;
        if (*dst_offset == sizeof(s_output_buf)) {
            flush_output(sizeof(s_output_buf));
            *dst_offset = 0;
        }
        if (src_offset == compressed_size)
            break;
        if (status != TINFL_STATUS_HAS_MORE_OUTPUT)
            break;
    }
}

extern "C" bool test_decompress(uint8_t* compressed, size_t compressed_size,
                                uint8_t* orig, size_t orig_size) {

    LOGDEBUG("decompressor size: %u, buf size: %u", sizeof(s_decompressor), sizeof(s_output_buf));

    tinfl_init(&s_decompressor);
    cmp_ptr = orig;
    size_t dst_offset = 0;
    const size_t decompress_block_size = 1024;
    for (size_t pos = 0; pos < compressed_size; pos += decompress_block_size) {
        LOGDEBUG("Block %d (start at %d)", pos/decompress_block_size, pos);
        size_t left = compressed_size - pos;
        size_t will_decompress = (left < decompress_block_size) ? left : decompress_block_size;
        decompress_block(compressed, will_decompress, &dst_offset, pos + decompress_block_size >= compressed_size);
        compressed += will_decompress;
    }
    return true;
}

extern "C" bool espcomm_upload_file_compressed(const char* name) {
    struct stat st;
    if (stat(name, &st) != 0) {
        LOGERR("Failed to stat input file");
    }

    FILE* file_in = fopen(name, "rb");
    if (!file_in) {
        LOGERR("Failed to open input file");
        return false;
    }

    size_t file_size = (uint32_t) st.st_size;

    std::unique_ptr<uint8_t[]> input_buf (new uint8_t[file_size]);
    auto result = fread(input_buf.get(), 1, file_size, file_in);
    fclose(file_in);

    if (result != file_size) {
        LOGERR("Failed to read input file");
        return false;
    }

    std::unique_ptr<uint8_t[]> output_buf (new uint8_t[file_size]);
// void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);
    size_t output_size = tdefl_compress_mem_to_mem(output_buf.get(), file_size, input_buf.get(), file_size, TDEFL_DEFAULT_MAX_PROBES);

    LOGDEBUG("Compressed file: %u -> %u", file_size, output_size);
    // test_decompress(output_buf.get(), output_size, input_buf.get(), file_size);
    espcomm_upload_mem(output_buf.get(), output_size);
    return true;
}
