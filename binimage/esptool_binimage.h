/**********************************************************************************
 **********************************************************************************
 ***
 ***    esptool_binimage.h
 ***    - defines and prototypes for handling firmware files for the ESP8266
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

#ifndef ESPTOOL_BINIMAGE_H
#define ESPTOOL_BINIMAGE_H

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

/*
** structs used to build and maintain internal representation
** of the binary firmware image
*/

/*
** structure holding a given chunk of binary data
*/

typedef struct {
    uint32_t            address;
    uint32_t            size;
    unsigned char       *data;
} binary_segment;

/*
** structure specifying the binary firmware image
** also stores the list of chunks used
*/

typedef struct {
    unsigned char       magic;
    unsigned char       num_segments;

    /* SPI Flash Interface (0 = QIO, 1 = QOUT, 2 = DIO, 0x3 = DOUT) */
    unsigned char       flash_mode;

    /* High four bits: 0 = 512K, 1 = 256K, 2 = 1M, 3 = 2M, 4 = 4M,
       Low four bits:  0 = 40MHz, 1= 26MHz, 2 = 20MHz, 0xf = 80MHz */
    unsigned char		flash_size_freq;

    uint32_t            entry;
    FILE                *image_file;
    binary_segment      *segments;
} bin_image;

#define FLASH_MODE_QIO  0
#define FLASH_MODE_QOUT 1
#define FLASH_MODE_DIO  2
#define FLASH_MODE_DOUT 3

#define FLASH_SIZE_512K 0 << 4
#define FLASH_SIZE_256K 1 << 4
#define FLASH_SIZE_1M   2 << 4
#define FLASH_SIZE_2M   3 << 4
#define FLASH_SIZE_4M   4 << 4
#define FLASH_SIZE_8M   8 << 4
#define FLASH_SIZE_16M  9 << 4

// flash frequency in MHz
#define FLASH_FREQ_40   0x0
#define FLASH_FREQ_26   0x1
#define FLASH_FREQ_20   0x2
#define FLASH_FREQ_80   0xf

/*
** function prototypes
*/

/*
** initializes the internal firmware image representation
** fname: the file name to which the image will finally be saved
** entry: the code entry point address
** returns 1 on success, 0 on failure
*/
int binimage_prepare(const char *fname, uint32_t entry);

/*
** specify a new code entry address
*/
void bimage_set_entry(uint32_t entry);

/*
** write the binary firmware image to disk
** padsize: specifies to what power-of-two blocksize the image needs to be padded
** returns 1 on success, 0 on failure
*/
int binimage_write_close(uint32_t padsize);


int binimage_write_padto(uint32_t padsize, uint32_t address);



/*
** add a new segment to the firmware image
** address: specifies the load address of the given segment
** size: specifies the size of the binary data segment in bytes
** data: a pointer to an array of bytes containing the binary segment data
** returns 1 on success, 0 on failure
*/
int binimage_add_segment(uint32_t address, uint32_t size, unsigned char *data);


/*
** update the image header which specifies flash chip configuration
** mode: one of "qio", "qout", "dio", "dout" (case-insensitive)
** size: one of "512K", "256K", "1M", "2M", "4M" (case-insensitive)
** freq: one of "40", "26", "20", "80"
** if these methods are not called, the binary image will use the following parameters:
** QIO, 512K, 40MHz
** return 1 on success, 0 on failure
*/
int binimage_set_flash_mode(const char* mode);
int binimage_set_flash_size(const char* size);
int binimage_set_flash_freq(const char* freq);
int binimage_set_header_layout(const char* layout);

#endif
