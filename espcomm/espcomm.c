/**********************************************************************************
 **********************************************************************************
 ***
 ***    espcomm.c
 ***    - routines to access the bootloader in the ESP
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

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "infohelper.h"
#include "espcomm.h"
#include "serialport.h"

bootloader_packet send_packet;
bootloader_packet receive_packet;


#ifdef LINUX
static char default_port[] = { "/dev/ttyUSB0\0" };
#else
#ifdef WINDOWS
static char default_port[] = { "/dev/ttyS0\0" };
#else
#undef TOOLPORT
#endif
#endif

static char *espcomm_port = default_port;
static unsigned int espcomm_baudrate = 115200;
static uint32_t espcomm_address = 0x00000;

const command_response responses[NUM_CMDS] =
{
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0601 },
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0000 },
    { 0x20120707, 0x0000 }
};

static unsigned char sync_frame[36] = { 0x07, 0x07, 0x12, 0x20,
                               0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
                               0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
                               0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
                               0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
                               
static uint32_t flash_packet[BLOCKSIZE_FLASH+32];
//static uint32_t ram_packet[BLOCKSIZE_RAM+32];

static void espcomm_enter_boot(void)
{
    // reset device into bootloader mode
    serialport_set_rts(0);
    serialport_set_dtr(0);
    
    usleep(10);
    
    serialport_set_dtr(1);
    
    usleep(10);
}

static void espcomm_reset_to_exec(void)
{
    // reset device into normal mode
    serialport_set_rts(1);
    serialport_set_dtr(0);
    usleep(10);
    serialport_set_dtr(1);
    usleep(10);
}

uint32_t espcomm_calc_checksum(unsigned char *data, uint16_t data_size)
{
    uint16_t cnt;
    uint32_t result;
    
    result = 0xEF;
    
    for(cnt = 0; cnt < data_size; cnt++)
    {
        result ^= data[cnt];
    }
    
    return result;
}

static uint32_t espcomm_send_command(unsigned char command, unsigned char *data, uint16_t data_size)
{
    uint32_t result;
    uint32_t cnt;

    result = 0;
    
    send_packet.direction = 0x00;
    send_packet.command = command;
    send_packet.size = data_size;
    
    serialport_send_C0();

    info_printf(-100, "espcomm_cmd: sending command header\r\n");
    
    serialport_send_slip((unsigned char*)&send_packet, 8);
    
    if(data_size)
    {
        info_printf(-100, "espcomm_cmd: sending command payload\r\n");
        serialport_send_slip(data, data_size);
    }
    else
    {
        info_printf(-100, "espcomm_cmd: no payload\r\n");
    }
    
    serialport_send_C0();

    serialport_drain();
    
    if(serialport_receive_C0())
    {
        if(serialport_receive_slip((unsigned char*)&receive_packet, 8))
        {
            if(receive_packet.size)
            {
                info_printf(-100, "espcomm cmd: receiving %i bytes of data\r\n", receive_packet.size);
                if(receive_packet.data)
                {
                    free(receive_packet.data);
                    receive_packet.data = NULL;
                }
                
                receive_packet.data = malloc(receive_packet.size);
                
                if(serialport_receive_slip(receive_packet.data, receive_packet.size) == 0)
                {
                    info_printf(-100, "espcomm_cmd: cant receive slip payload data\r\n");
                    return 0;
                }
                else
                {
                    info_printf(-100, "espcomm_cmd: received %x bytes: ", receive_packet.size);
                    for(cnt = 0; cnt < receive_packet.size; cnt++)
                    {
                        info_printf(-100, "0x%02X ", receive_packet.data[cnt]);
                    }
                    info_printf(-100, "\r\n");
                }
            }
            
            if(serialport_receive_C0())
            {
                if(receive_packet.direction == 0x01 &&
                    receive_packet.command == command)
                {
                    info_printf(-100, "** 0x%08X 0x%04X >> 0x%08X 0x%04X ** ", receive_packet.response, receive_packet.uint16_data[0], responses[command].cmd_response, responses[command].payload_response);
                    
                    if(receive_packet.response == responses[command].cmd_response)
                    {
                        if(receive_packet.size)
                        {
                            if(receive_packet.uint16_data[0] == responses[command].payload_response)
                            {
                                result = 1;
                            }
                            else
                            {
                                info_printf(-100, "wrong payload response\r\n");
                                result = 0;
                            }
                        }
                        else
                        {
                            result = 1;
                        }
                    }
                    else
                    {
                        info_printf(-100, "wrong cmd response\r\n");
                        result = 0;
                    }
                }
                else
                {
                    info_printf(-100,  "espcomm cmd: wrong direction/command: 0x%02X 0x%02X\r\n", receive_packet.direction, receive_packet.command);
                }

#ifdef DEBUG                
                printf("CMD 0x%02X - RESP 0x%08X", command, receive_packet.response);
                if(receive_packet.size)
                {
                    printf(" - PAYLOAD ");
                    for(cnt = 0; cnt < receive_packet.size; cnt++)
                    {
                        printf("0x%02X ", receive_packet.data[cnt]);
                    }
                    printf("\r\n");
                }
#endif                
                
            }
            else
            {
                info_printf(-100, "espcomm cmd: no final C0\r\n");
            }
        }
        else
        {
            info_printf(-100, "espcomm cmd: can't receive command header response\r\n");
        }
    }
    
    info_printf(-100, "espcomm_cmd: response 0x%08X\r\n", result);
    return result;
}


static int espcomm_sync(void)
{
    unsigned char retry, retry2;
    
    retry2 = 0;
    
    while(retry2++ < 4)
    {
        retry = 0;

        espcomm_enter_boot();
        serialport_flush();
        
        while(retry++ < 2)
        {
            send_packet.checksum = espcomm_calc_checksum((unsigned char*)&sync_frame, 36);
            
            serialport_flush();
            
            if(espcomm_send_command(SYNC_FRAME, (unsigned char*) &sync_frame, 36))
            {
                serialport_flush();
                return 1;
            }
        }
    }
    
    return 0;
}

int espcomm_open(void)
{
    
    if(serialport_open(espcomm_port, espcomm_baudrate))
    {
       info_printf(3, "opening bootloader!\r\n");
        return espcomm_sync();
    }
    
    return 0;
}

void espcomm_close(void)
{
    info_printf(3, "closing bootloader!\r\n");
    espcomm_reset_to_exec();
    serialport_close();
}

int espcomm_start_flash(uint32_t size, uint32_t address)
{
    flash_packet[0] = size;
    flash_packet[1] = 0x00000200;
    flash_packet[2] = BLOCKSIZE_FLASH;
    flash_packet[3] = address;
    
    send_packet.checksum = espcomm_calc_checksum((unsigned char*) flash_packet, 16);
    return espcomm_send_command(FLASH_DOWNLOAD_BEGIN, (unsigned char*) &flash_packet, 16);
}

void espcom_progress(char *msg, uint32_t current, uint32_t max)
{
    
}

int espcomm_upload_file(char *name)
{
    FILE *f;
    struct stat st;
    uint32_t fsize;
    uint32_t ftotal;
    uint32_t fdone;
    
    uint32_t cnt;
    uint32_t res;
    
    if(stat(name, &st) == 0)
    {
        if(espcomm_open())
        {
            fsize = st.st_size;

            f = fopen( name, "rb");
            
            cnt = 0;
            flash_packet[0] = 0;
            flash_packet[1] = 0;
            flash_packet[2] = 0;
            flash_packet[3] = 0;

            if(f)
            {
                ftotal = fsize;
                fdone = 0;

                info_printf(1, "uploading %i bytes from %s to flash at 0x%08X\r\n", fsize, name, espcomm_address);
            

                infohelper_print_progress("Erasing Flash", 0, ftotal);
                serialport_set_timeout(1000);
                espcomm_start_flash(fsize, espcomm_address);
                serialport_set_timeout(1);
                
                infohelper_print_progress("Writing Flash", 0, ftotal);
                
                while(fsize)
                {
                    flash_packet[0] = fread(&flash_packet[4], 1, BLOCKSIZE_FLASH, f);
                    fsize -= flash_packet[0];
            
                    flash_packet[1] = cnt;
                    flash_packet[2] = 0;
                    flash_packet[3] = 0;
                    
                    send_packet.checksum = espcomm_calc_checksum((unsigned char *) &flash_packet[4], flash_packet[0]);
                    res = espcomm_send_command(FLASH_DOWNLOAD_DATA, (unsigned char*) &flash_packet, flash_packet[0]+16);
                    
                    if(res == 0)
                    {
                        res = espcomm_send_command(FLASH_DOWNLOAD_DONE, (unsigned char*) &flash_packet, 4);
                        fclose(f);
                        espcomm_close();

                        return 0;
                    }
                    
                    cnt++;
                    fdone += flash_packet[0];
                    
                    infohelper_print_progress("Writing Flash", fdone, ftotal);

                    
                }
                
                flash_packet[0] = 0;
                res = espcomm_send_command(FLASH_DOWNLOAD_DONE, (unsigned char*) &flash_packet, 4);

                info_printf(101, "\r\n");
            }
            
            fclose(f);
            espcomm_close();
            return 1;
        }
    }
    
    return 0;
}


int espcomm_set_port(char *port)
{
    info_printf(4, "setting port from %s to %s\r\n", espcomm_port, port);
    espcomm_port = port;
    return 1;
}

int espcomm_set_baudrate(const char *baudrate)
{
    info_printf(4, "setting baudrate from %i to %i\r\n", espcomm_baudrate, (unsigned int)strtol(baudrate, NULL, 10));
    espcomm_baudrate = strtol(baudrate, NULL, 10);
    return 1;
}

int espcomm_set_address(const char *address)
{
    info_printf(4, "setting address from 0x%08X to 0x%08X\r\n", espcomm_address, (unsigned int)strtol(address, NULL, 16));
    espcomm_address = strtol(address, NULL, 16);
    return 1;
}
