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
#include "esptool_elf_object.h"
#include "esptool_binimage.h"

static bin_image b_image;

static int binimage_add_segment(uint32_t address, uint32_t size, unsigned char *data)
{

    if(data)
    {
        if(b_image.segments == 0)
        {
            b_image.segments = malloc(size);
            if(b_image.segments == NULL)
            {
                info_printf(-1, "can't allocate 0x%08X bytes for binimage segment #%i\r\n", b_image.segments[b_image.num_segments].size,
                                                                                        b_image.num_segments);
                return 0;
            }
            
        }
        else
        {
            b_image.segments = realloc(b_image.segments, (b_image.num_segments+1)*sizeof(binary_segment));
            if(b_image.segments == NULL)
            {
                info_printf(-1, "can't allocate 0x%08X more bytes for binimage segment #%i\r\n", b_image.segments[b_image.num_segments].size,
                                                                                             b_image.num_segments);
                return 0;
            }
        }

        b_image.segments[b_image.num_segments].address = address;
        b_image.segments[b_image.num_segments].size = size;
        b_image.segments[b_image.num_segments].data = data;

        info_printf(2, "added segment #%i to binimage for address 0x%08X with size 0x%08X\r\n", b_image.num_segments, 
                                                                                            b_image.segments[b_image.num_segments].address,
                                                                                            b_image.segments[b_image.num_segments].size);
        
        b_image.num_segments++;
        return 1;
    }
    else
    {
        info_printf(-1, "no data for binimage segment #%i\r\n", b_image.num_segments);
        return 0;
    }
}

int binimagecmd_add_named_elfsegment(char *sname, uint32_t padsize)
{
    uint32_t snum;
    uint32_t addr;
    uint32_t size;
    uint32_t pad;
    
    snum = get_elf_secnum_by_name(sname);
    addr = get_elf_section_addr(snum);
    size = get_elf_section_size(snum);
    if(snum)
    {
        print_elf_section_info(snum);
        pad = get_elf_section_size(snum);
        padsize--;
        
        while(pad & padsize)
        {
            pad++;
        }
        
        if(pad > size)
        {
            binimage_add_segment(get_elf_section_addr(snum), pad, get_elf_section_bindata(snum, pad));
            info_printf(2, "added section %s at 0x%08X size 0x%08X with padding 0x%08X\r\n", get_elf_section_name(snum), addr, size, pad-size);
        }
        else
        {
            binimage_add_segment(get_elf_section_addr(snum), size, get_elf_section_bindata(snum, size));
            info_printf(2, "added section %s at 0x%08X size 0x%08X\r\n", get_elf_section_name(snum), addr, size);
        }
    }
    
    return snum;
}

int binimage_prepare(char *fname, uint32_t entry)
{
    if(b_image.image_file)
    {
        return 0;
    }
    
    b_image.magic = 0xE9;
    b_image.num_segments = 0;
    b_image.dummy[0] = 0x00;
    b_image.dummy[1] = 0x00;
    b_image.entry = entry;
    
    
    if(fname[0])
    {
        b_image.image_file = fopen( fname, "wb");
        if(b_image.image_file == NULL)
        {
            info_printf(-1, "cant open binimage file \"%s\" for writing, aborting\r\n", fname);
            return 0;
        }
    }
    else
    {
        return 0;
    }
    
    b_image.segments = 0;
    
    info_printf(2, "created structure for binimage \"%s\" with entry address 0x%08X\r\n", fname, b_image.entry);
    
    return 1;
}

void bimage_set_entry(uint32_t entry)
{
    b_image.entry = entry;
    info_printf(2, "set bimage entry to 0x%08X\r\n", b_image.entry);
}

int binimage_write_close(uint32_t padsize)
{
    unsigned int cnt, cnt2, total_size;
    unsigned  char chksum;
    
    chksum = 0xEF;
    
    if(b_image.image_file == 0)
    {
        return 0;
    }
    
    if(fwrite((unsigned char*)&b_image, 1, 8, b_image.image_file) != 8)
    {
        info_printf(-1, "cant write main header to binimage file, aborting\r\n");
        fclose(b_image.image_file);
        b_image.image_file = 0;
        return 0;
    }
    
    total_size = 8;
    
    for(cnt = 0; cnt < b_image.num_segments; cnt++)
    {
        if(fwrite((unsigned char*)&b_image.segments[cnt], 1, 8, b_image.image_file) != 8)
        {
            info_printf(-1, "cant write header for segment  #%i to binimage file, aborting\r\n", cnt);
            fclose(b_image.image_file);
            b_image.image_file = 0;
            return 0;
        }
        
        total_size += 8;
        
        if(fwrite(b_image.segments[cnt].data, 1, b_image.segments[cnt].size, b_image.image_file) != b_image.segments[cnt].size)
        {
            info_printf(-1, "cant write data block for segment  #%i to binimage file, aborting\r\n", cnt);
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
            info_printf(-1, "cant write padding byte 0x00 at 0x%08X to binimage file, aborting\r\n", total_size);
            fclose(b_image.image_file);
            b_image.image_file = 0;
            return 0;
        }
        cnt++;
    }
    
    if(fputc(chksum, b_image.image_file) == EOF)
    {
        info_printf(-1, "cant write checksum byte 0x%02X at 0x%08X to binimage file, aborting\r\n", chksum, total_size);
        fclose(b_image.image_file);
        b_image.image_file = 0;
        return 0;
    }

    info_printf(2, "saved binimage file, total size is %i bytes, checksum byte is 0x%02X\r\n", total_size, chksum);
    
    fclose(b_image.image_file);
    b_image.image_file = 0;
    
    if(b_image.segments)
    {
        for(cnt = 0; cnt < b_image.num_segments; cnt++)
        {
            if(b_image.segments[cnt].data)
            {
                info_printf(2, "releasing memory used for segment %i in binimage\r\n", cnt);
                free(b_image.segments[cnt].data);
            }
        }
        if(b_image.segments)
        {
            info_printf(2, "releasing memory used for binimage segment pointers\r\n");
            free(b_image.segments);
        }
    }

    return 1;
}

