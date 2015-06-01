/**********************************************************************************
 **********************************************************************************
 ***
 ***    esptool_binimage.c
 ***    - implementation of functions to handle firmware files for the ESP8266
 ***
 ***    Copyright (C) 2014 Christian Klippel <ck@atelier-klippel.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "infohelper.h"
#include "esptool_binimage.h"

static bin_image b_image = {
 .magic             = 0xe9,
 .num_segments      = 0,
 .flash_mode        = FLASH_MODE_QIO,
 .flash_size_freq   = FLASH_SIZE_512K | FLASH_FREQ_40
};

unsigned int total_size = 0;

int binimage_add_segment(uint32_t address, uint32_t size, unsigned char *data)
{
    if(!data)
    {
        LOGERR("no data for binimage segment #%i", b_image.num_segments);
        return 0;
    }
    
    if(b_image.segments == 0)
    {
        b_image.segments = malloc(size);
        if(b_image.segments == NULL)
        {
            LOGERR("can't allocate 0x%08X bytes for binimage segment #%i",
                      b_image.segments[b_image.num_segments].size, b_image.num_segments);
            return 0;
        }
    }
    else
    {
        b_image.segments = realloc(b_image.segments, (b_image.num_segments+1)*sizeof(binary_segment));
        if(b_image.segments == NULL)
        {
            LOGERR("can't allocate 0x%08X more bytes for binimage segment #%i",
                      b_image.segments[b_image.num_segments].size, b_image.num_segments);
            return 0;
        }
    }

    b_image.segments[b_image.num_segments].address = address;
    b_image.segments[b_image.num_segments].size = size;
    b_image.segments[b_image.num_segments].data = data;

    LOGINFO("added segment #%i to binimage for address 0x%08X with size 0x%08X",
            b_image.num_segments,
            b_image.segments[b_image.num_segments].address,
            b_image.segments[b_image.num_segments].size);
    
    b_image.num_segments++;
    return 1;
}

int binimage_prepare(const char *fname, uint32_t entry)
{
    if(b_image.image_file)
    {
        return 0;
    }
    
    b_image.entry = entry;    
    
    if(fname[0])
    {
        b_image.image_file = fopen(fname, "wb");
        if(b_image.image_file == NULL)
        {
            LOGERR("cant open binimage file \"%s\" for writing, aborting", fname);
            return 0;
        }
    }
    else
    {
        return 0;
    }
    
    b_image.segments = 0;
    b_image.num_segments = 0;
    total_size = 0;
    
    LOGINFO("created structure for binimage \"%s\" with entry address 0x%08X", fname, b_image.entry);
    
    return 1;
}

void bimage_set_entry(uint32_t entry)
{
    b_image.entry = entry;
    LOGINFO("set bimage entry to 0x%08X", b_image.entry);
}

int binimage_write(uint32_t padsize, bool close)
{
    unsigned int cnt, cnt2;
    unsigned char chksum;
    
    chksum = 0xEF;
    
    if(b_image.image_file == 0)
    {
        return 0;
    }
    
    if(fwrite((unsigned char*)&b_image, 1, 8, b_image.image_file) != 8)
    {
        LOGERR("cant write main header to binimage file, aborting");
        fclose(b_image.image_file);
        b_image.image_file = 0;
        return 0;
    }
    
    total_size = 8;
    
    for(cnt = 0; cnt < b_image.num_segments; cnt++)
    {
        if(fwrite((unsigned char*)&b_image.segments[cnt], 1, 8, b_image.image_file) != 8)
        {
            LOGERR("cant write header for segment  #%i to binimage file, aborting", cnt);
            fclose(b_image.image_file);
            b_image.image_file = 0;
            return 0;
        }
        
        total_size += 8;
        
        if(fwrite(b_image.segments[cnt].data, 1, b_image.segments[cnt].size, b_image.image_file) != b_image.segments[cnt].size)
        {
            LOGERR("cant write data block for segment  #%i to binimage file, aborting", cnt);
            fclose(b_image.image_file);
            b_image.image_file = 0;
            return 0;
        }
        
        total_size += b_image.segments[cnt].size;
        for(cnt2 = 0; cnt2 < b_image.segments[cnt].size; cnt2++)
        {
            chksum ^= b_image.segments[cnt].data[cnt2];
        }
    }
    
    padsize--;
    
    while(++total_size & padsize)
    {
        if(fputc(0x00, b_image.image_file) == EOF)
        {
            LOGERR("cant write padding byte 0x00 at 0x%08X to binimage file, aborting", total_size);
            fclose(b_image.image_file);
            b_image.image_file = 0;
            return 0;
        }
        cnt++;
    }
    
    if(fputc(chksum, b_image.image_file) == EOF)
    {
        LOGERR("cant write checksum byte 0x%02X at 0x%08X to binimage file, aborting", chksum, total_size);
        fclose(b_image.image_file);
        b_image.image_file = 0;
        return 0;
    }

    LOGINFO("saved binimage file, total size is %i bytes, checksum byte is 0x%02X", total_size, chksum);
    
    if (close) 
    {
        fclose(b_image.image_file);
        b_image.image_file = 0;
    }
    
    if(b_image.segments)
    {
        for(cnt = 0; cnt < b_image.num_segments; cnt++)
        {
            if(b_image.segments[cnt].data)
            {
                LOGDEBUG("releasing memory used for segment %i in binimage", cnt);
                free(b_image.segments[cnt].data);
            }
        }
        if(b_image.segments)
        {
            LOGDEBUG("releasing memory used for binimage segment pointers");
            free(b_image.segments);
            b_image.segments = 0;
            b_image.num_segments = 0;
        }
    }

    return 1;
}

int binimage_write_close(uint32_t padsize)
{
    return binimage_write(padsize, true);
}

int binimage_write_padto(uint32_t padsize, uint32_t address)
{
    if (binimage_write(padsize, false) == 0)
        return 0;

    LOGDEBUG("binimage_write_padto: total:%x addr:%x", total_size, address);
    if (address < total_size) 
    {
        LOGERR("binimage_write_padto: address is less than size written");
        return 0;
    }

    while (total_size < address) 
    {
        if (fputc(0xaa, b_image.image_file) == EOF)
            return 0;
        ++total_size;
    }

    return 1;
}

#define INVALID_VAL 0xff

unsigned char binimage_parse_flash_mode(const char* str);
unsigned char binimage_parse_flash_size(const char* str);
unsigned char binimage_parse_flash_freq(const char* str);

const char* binimage_flash_mode_to_str(unsigned char mode);
const char* binimage_flash_size_to_str(unsigned char size);
const char* binimage_flash_freq_to_str(unsigned char freq);


int binimage_set_flash_mode(const char* modestr)
{
    unsigned char mode = binimage_parse_flash_mode(modestr);
    if (mode == INVALID_VAL)
    {
        LOGERR("invalid flash mode value: %s", modestr);
        return 0;
    }

    LOGINFO("setting flash mode from %s to %s", 
            binimage_flash_mode_to_str(b_image.flash_mode),
            binimage_flash_mode_to_str(mode));

    b_image.flash_mode = mode;
    return 1;
}

int binimage_set_flash_size(const char* sizestr)
{
    unsigned char size = binimage_parse_flash_size(sizestr);
    if (size == INVALID_VAL)
    {
        LOGERR("invalid flash size value: %s", sizestr);
        return 0;
    }

    LOGINFO("setting flash size from %s to %s", 
            binimage_flash_size_to_str(b_image.flash_size_freq & 0xf0),
            binimage_flash_size_to_str(size));
    
    b_image.flash_size_freq = size | (b_image.flash_size_freq & 0x0f);
    return 1;
}

int binimage_set_flash_freq(const char* freqstr)
{
    unsigned char freq = binimage_parse_flash_freq(freqstr);
    if (freq == INVALID_VAL)
    {
        LOGERR("invalid flash frequency value: %s", freqstr);
        return 0;
    }

    LOGINFO("setting flash frequency from %s to %s", 
            binimage_flash_freq_to_str(b_image.flash_size_freq & 0x0f),
            binimage_flash_freq_to_str(freq));
    
    b_image.flash_size_freq = (b_image.flash_size_freq & 0xf0) | freq;
    return 1;
}

static const char* flash_mode_str[] = {"qio", "qout", "dio", "dout"};
static const char* flash_size_str[] = {"512K", "256K", "1M", "2M", "4M", "8M", "16M", "32M"};

unsigned char binimage_parse_flash_mode(const char* str)
{
    const int n = sizeof(flash_mode_str)/sizeof(const char*);
    for (int i = 0; i < n; ++i) 
    {
        if (strcasecmp(str, flash_mode_str[i]) == 0) 
        {
            return (unsigned char) i;
        }
    }
    return INVALID_VAL;
}

unsigned char binimage_parse_flash_size(const char* str)
{
    const int n = sizeof(flash_size_str)/sizeof(const char*);
    for (int i = 0; i < n; ++i) 
    {
        if (strcasecmp(str, flash_size_str[i]) == 0) 
        {
            return (unsigned char) i << 4;
        }
    }
    return INVALID_VAL;
}

unsigned char binimage_parse_flash_freq(const char* str)
{
    int val = atoi(str);
    switch (val) 
    {
        case 40: return FLASH_FREQ_40;
        case 26: return FLASH_FREQ_26;
        case 20: return FLASH_FREQ_20;
        case 80: return FLASH_FREQ_80;
        default: return INVALID_VAL;
    }
}

const char* binimage_flash_mode_to_str(unsigned char mode)
{
    if (mode > FLASH_MODE_DOUT)
        return "";

    return flash_mode_str[mode];
}

const char* binimage_flash_size_to_str(unsigned char size)
{
    if ((size >> 4) > FLASH_SIZE_32M)
        return "";
    return flash_size_str[size >> 4];
}

const char* binimage_flash_freq_to_str(unsigned char freq)
{
    switch (freq)
    {
        case FLASH_FREQ_40: return "40";
        case FLASH_FREQ_26: return "26";
        case FLASH_FREQ_20: return "20";
        case FLASH_FREQ_80: return "80";
        default: return "";
    }
}

