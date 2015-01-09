/**********************************************************************************
 **********************************************************************************
 ***
 ***    espcomm.h
 ***    - include file for espcomm.c
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

#ifndef ESPCOMM_H
#define ESPCOMM_H

enum
{
    CMD0                        = 0x00,
    CMD1                        = 0x01,
    FLASH_DOWNLOAD_BEGIN        = 0x02,
    FLASH_DOWNLOAD_DATA         = 0x03,
    FLASH_DOWNLOAD_DONE         = 0x04,
    RAM_DOWNLOAD_BEGIN          = 0x05,
    RAM_DOWNLOAD_END            = 0x06,
    RAM_DOWNLOAD_DATA           = 0x07,
    SYNC_FRAME                  = 0x08,
    WRITE_REGISTER              = 0x09,
    READ_REGISTER               = 0x0A
};


typedef struct
{
    uint8_t     direction;
    uint8_t     command;
    uint16_t    size;
    union
    {
        uint32_t    checksum;
        uint32_t    response;
    };
    
    unsigned char *data;
    
} bootloader_packet;

#define BLOCKSIZE_FLASH         0x0400
#define BLOCKSIZE_RAM           0x1800

int espcomm_set_port(char *port);
int espcomm_set_baudrate(const char *baudrate);
int espcomm_set_address(const char *address);
int espcomm_set_board(const char* name);

int espcomm_open(void);
void espcomm_close(void);

int espcomm_upload_file(char *name);
int espcomm_file_uploaded();
int espcomm_start_app(int reboot);

#endif
