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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "infohelper.h"
#include "espcomm.h"
#include "serialport.h"
#include "espcomm_boards.h"
#include "delay.h"

bootloader_packet send_packet;
bootloader_packet receive_packet;

static espcomm_board_t* espcomm_board = 0;
static bool sync_stage = false;
static bool upload_stage = false;
static bool espcomm_is_open = false;

static const char *espcomm_port =
#if defined(LINUX)
"/dev/ttyUSB0";
#elif defined(WINDOWS)
"COM1";
#elif defined(OSX)
"/dev/tty.usbserial";
#else
"";
#endif

static unsigned int espcomm_baudrate = 115200;
static uint32_t espcomm_address = 0x00000;

static unsigned char sync_frame[36] = { 0x07, 0x07, 0x12, 0x20,
                               0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
                               0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
                               0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
                               0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
                               
static uint32_t flash_packet[BLOCKSIZE_FLASH+32];
//static uint32_t ram_packet[BLOCKSIZE_RAM+32];

static int file_uploaded = 0;

static void espcomm_enter_boot(void)
{
    espcomm_board_reset_into_bootloader(espcomm_board);
}

static void espcomm_reset_to_exec(void)
{
    espcomm_board_reset_into_app(espcomm_board);
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

static uint32_t espcomm_send_command(unsigned char command, unsigned char *data, uint16_t data_size, int delay)
{
    uint32_t result;
    uint32_t cnt;

    result = 0;
    
    send_packet.direction = 0x00;
    send_packet.command = command;
    send_packet.size = data_size;
    
    serialport_send_C0();

    if (!upload_stage)
        LOGDEBUG("espcomm_cmd: sending command header");
    else
        LOGVERBOSE("espcomm_cmd: sending command header");

    serialport_send_slip((unsigned char*) &send_packet, 8);
    
    if(data_size)
    {
        if (!upload_stage)
            LOGDEBUG("espcomm_cmd: sending command payload");
        else
            LOGVERBOSE("espcomm_cmd: sending command payload");
        serialport_send_slip(data, data_size);
    }
    else
    {
        LOGDEBUG("espcomm_cmd: no payload");
    }
    
    serialport_send_C0();

    serialport_drain();

    LOGVERBOSE("waiting %dms...", delay);

    espcomm_delay_ms(delay);

    LOGVERBOSE("wait complete");
    
    if(serialport_receive_C0())
    {
        if(serialport_receive_slip((unsigned char*) &receive_packet, 8))
        {
            if(receive_packet.size)
            {
                if (!upload_stage)
                    LOGDEBUG("espcomm cmd: receiving %i bytes of data", receive_packet.size);
                else
                    LOGVERBOSE("espcomm cmd: receiving %i bytes of data", receive_packet.size);

                if(receive_packet.data)
                {
                    free(receive_packet.data);
                    receive_packet.data = NULL;
                }
                
                receive_packet.data = malloc(receive_packet.size);
                
                if(serialport_receive_slip(receive_packet.data, receive_packet.size) == 0)
                {
                    LOGWARN("espcomm_cmd: cant receive slip payload data");
                    return 0;
                }
                else
                {
                    LOGVERBOSE("espcomm_cmd: received %x bytes: ", receive_packet.size);
                    for(cnt = 0; cnt < receive_packet.size; cnt++)
                    {
                        LOGVERBOSE("0x%02X ", receive_packet.data[cnt]);
                    }
                }
            }
            
            if(serialport_receive_C0())
            {
                if(receive_packet.direction == 0x01 &&
                    receive_packet.command == command)
                {
                    result = receive_packet.response;
                }
                else
                {
                    LOGWARN("espcomm cmd: wrong direction/command: 0x%02X 0x%02X, expected 0x%02X 0x%02X", 
						receive_packet.direction, receive_packet.command, 1, command);
					return 0;
                }
            }
            else
            {
                if (!sync_stage)
                    LOGWARN("espcomm cmd: no final C0");
                else
                    LOGVERBOSE("espcomm cmd: no final C0");
				return 0;
            }
        }
        else
        {
            LOGWARN("espcomm cmd: can't receive command header response");
			return 0;
        }
    }
    else
    {
        if (!sync_stage)
            LOGWARN("espcomm cmd: didn't receive C0");
        else
            LOGVERBOSE("espcomm cmd: didn't receive C0");
		return 0;
    }
    
    LOGVERBOSE("espcomm cmd: response 0x%08X", result);
    return result;
}


static int espcomm_sync(void)
{
    sync_stage = true;
    for (int retry_boot = 0; retry_boot < 3; ++retry_boot)
    {
        LOGINFO("resetting board");
        espcomm_enter_boot();
		espcomm_delay_ms(45);
        for (int retry_sync = 0; retry_sync < 3; ++retry_sync)
        {
            LOGINFO("trying to connect");
            serialport_flush();

            send_packet.checksum = espcomm_calc_checksum((unsigned char*)&sync_frame, 36);
            if(espcomm_send_command(SYNC_FRAME, (unsigned char*) &sync_frame, 36, 0) == 0x20120707)
            {
				espcomm_delay_ms(10);
                serialport_flush();
                sync_stage = false;
                return 1;
            }
			
			espcomm_delay_ms(10);
        }
    }
    sync_stage = false;
    LOGWARN("espcomm_sync failed");
    return 0;
}

int espcomm_open(void)
{
	if (espcomm_is_open)
		return 1;
		
    if(serialport_open(espcomm_port, espcomm_baudrate))
    {
        LOGINFO("opening bootloader");
		if (espcomm_sync())
		{
			espcomm_is_open = true;
			return 1;
		}
    }
    
    return 0;
}

void espcomm_close(void)
{
    LOGINFO("closing bootloader");
    serialport_close();
}

int espcomm_start_flash(uint32_t size, uint32_t address)
{
    uint32_t res;
    
    flash_packet[0] = size;
    flash_packet[1] = 0x00000200;
    flash_packet[2] = BLOCKSIZE_FLASH;
    flash_packet[3] = address;
    
    send_packet.checksum = espcomm_calc_checksum((unsigned char*) flash_packet, 16);
    int delay = size / 1000 * 3 + 500;
    LOGDEBUG("calculated erase delay: %d", delay);
    res = espcomm_send_command(FLASH_DOWNLOAD_BEGIN, (unsigned char*) &flash_packet, 16, 1000);
    return res;
}


int espcomm_upload_file(char *name)
{
    LOGDEBUG("espcomm_upload_file");
    FILE *f;
    struct stat st;
    uint32_t fsize;
    uint32_t fdone;
    
    uint32_t cnt;
    uint32_t res;
    
    if(stat(name, &st) == 0)
    {
        LOGDEBUG("stat %s success", name);
        if(espcomm_open())
        {
            LOGDEBUG("espcomm_open");
            fsize = (uint32_t) st.st_size;

            f = fopen( name, "rb");
            
            cnt = 0;
            flash_packet[0] = 0;
            flash_packet[1] = 0;
            flash_packet[2] = 0;
            flash_packet[3] = 0;

            if(f)
            {
                fdone = 0;

                INFO("Uploading %i bytes from %s to flash at 0x%08X\n", fsize, name, espcomm_address);

                LOGDEBUG("erasing flash");
                res = espcomm_start_flash(fsize, espcomm_address);
				if (res == 0)
				{
					LOGWARN("espcomm_send_command(FLASH_DOWNLOAD_BEGIN) failed");
					fclose(f);
					espcomm_close();
					return 0;
				}
                
                LOGDEBUG("writing flash");
                upload_stage = true;
                while(fsize)
                {
                    flash_packet[0] = (uint32_t) fread(&flash_packet[4], 1, BLOCKSIZE_FLASH, f);
                    fsize -= flash_packet[0];
            
                    flash_packet[1] = cnt;
                    flash_packet[2] = 0;
                    flash_packet[3] = 0;
                    
                    send_packet.checksum = espcomm_calc_checksum((unsigned char *) &flash_packet[4], flash_packet[0]);
                    res = espcomm_send_command(FLASH_DOWNLOAD_DATA, (unsigned char*) &flash_packet, flash_packet[0]+16, 0);
                    
                    if(res == 0)
                    {
						LOGWARN("espcomm_send_command(FLASH_DOWNLOAD_DATA) failed");
                        res = espcomm_send_command(FLASH_DOWNLOAD_DONE, (unsigned char*) &flash_packet, 4, 0);
                        fclose(f);
                        espcomm_close();
                        upload_stage = false;
                        return 0;
                    }
                    
                    cnt++;
                    fdone += flash_packet[0];
                    INFO(".");
                    fflush(stdout);
                }
                upload_stage = false;
                INFO("\n");
            }
            file_uploaded = 1;
            fclose(f);
            //espcomm_close();
            return 1;
        }
        else
        {
            LOGERR("espcomm_open failed");
        }
            
    }
    else
    {
        LOGERR("stat %s failed", name);
    }
    return 0;
}

int espcomm_start_app(int reboot)
{
    if(!espcomm_open())
    {
        LOGDEBUG("espcomm_open failed");
    }

    if (reboot)
    {
        LOGINFO("starting app with reboot");
        flash_packet[0] = 0;
    }
    else
    {
        LOGINFO("starting app without reboot");
        flash_packet[0] = 1;
    }
    
    espcomm_send_command(FLASH_DOWNLOAD_DONE, (unsigned char*) &flash_packet, 4, 0);
    file_uploaded = 0;

    espcomm_close();
    return 1;
}

int espcomm_file_uploaded()
{
    return file_uploaded;
}

int espcomm_set_port(char *port)
{
    LOGDEBUG("setting port from %s to %s", espcomm_port, port);
    espcomm_port = port;
    return 1;
}

int espcomm_set_baudrate(const char *baudrate)
{
    uint32_t new_baudrate = (uint32_t) strtol(baudrate, NULL, 10);
    LOGDEBUG("setting baudrate from %i to %i", espcomm_baudrate, new_baudrate);
    espcomm_baudrate = new_baudrate;
    return 1;
}

int espcomm_set_address(const char *address)
{
    uint32_t new_address = (uint32_t) strtol(address, NULL, 16);
    LOGDEBUG("setting address from 0x%08X to 0x%08X", espcomm_address, new_address);
    espcomm_address = new_address;
    return 1;
}

int espcomm_set_board(const char* name)
{
    LOGDEBUG("setting board to %s", name);
    espcomm_board = espcomm_board_by_name(name);
    if (!espcomm_board)
    {
        LOGERR("unknown board: %s", name);
        INFO("known boards are: ");
        for (espcomm_board_t* b = espcomm_board_first(); b; b = espcomm_board_next(b))
        {
            INFO("%s ", espcomm_board_name(b));
        }
        INFO("\n");
    }
    return 1;
}

