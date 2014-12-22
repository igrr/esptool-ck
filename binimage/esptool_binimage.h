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
    unsigned char       dummy[2];
    uint32_t            entry;
    FILE                *image_file;
    binary_segment      *segments;
} bin_image;


/*
** function prototypes
*/

/*
** initializes the internal firmware image representation
** fname: the file name to which the image will finally be  saved
** entry: the code entry point address
** returns 1 on success, 0 on failure
*/
int binimage_prepare(const char *fname, uint32_t entry);

/*
** specify a new code entry address
*/
void bimage_set_entry(uint32_t entry);

/*
** write the binary  firmware image to disk
** padsize: specifies to what power-of-two blocksize the image needs to be padded
** returns 1 on success, 0 on failure
*/
int binimage_write_close(uint32_t padsize);


/*
** add a new segment to the firmware image
** address: specifies the load address of the given segment
** size: specifies the size of the binary data segment in bytes
** data: a pointer to an array of bytes containing the binary segment data
** returns 1 on success, 0 on failure
*/
int binimage_add_segment(uint32_t address, uint32_t size, unsigned char *data);

#endif
