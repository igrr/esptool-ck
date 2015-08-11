/**********************************************************************************
 **********************************************************************************
 ***
 ***    argparse_commcmd.c
 ***    - parsing of comms related commands (flash upload, etc)
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
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "infohelper.h"
#include "esptool_elf.h"
#include "esptool_elf_object.h"
#include "esptool_binimage.h"
#include "espcomm.h"


bool upload_elf_section(const char* name)
{
    int entry = get_elf_entry();
    int section = get_elf_secnum_by_name(name);
    if (section == 0) {
        LOGERR("failed to find section %s", name);
        return false;
    }

    int start = get_elf_section_addr(section);
    int size = get_elf_section_size(section);
    LOGDEBUG("loading section %s, start=%x, size=%d", name, start, size);
    if (entry < start || entry >= start + size)
    {
        LOGDEBUG("not starting app for this section");
        entry = 0;
    }
    else
    {
        LOGDEBUG("will start app with entry=%x", entry);
    }
    uint8_t* data = get_elf_section_bindata(section, 4);
    bool res = espcomm_upload_mem_to_RAM(data, size, start, entry);
    free(data);
    if (!res)
    {
        LOGERR("espcomm_upload_mem_to_RAM failed");
        return false;
    }
    return true;
}

bool upload_file_compressed(const char* name)
{
    create_elf_object("esp/stub.elf");
    upload_elf_section(".rodata");
    upload_elf_section(".text");
    return espcomm_upload_file_compressed(name);
}

int argparse_commcmd(int num_args, char **arg_ptr)
{
    char *cur_cmd;

    if(arg_ptr[0][1] == 'c' && num_args--)
    {
        cur_cmd = &arg_ptr[0][2];
        arg_ptr++;

        switch(*cur_cmd++)
        {
            case 'p':
                if(num_args < 1)
                {
                    return 0;
                }
                if(espcomm_set_port(arg_ptr[0]))
                {
                    return 2;
                }
                break;

            case 'b':
                if(num_args < 1)
                {
                    return 0;
                }
                if(espcomm_set_baudrate(arg_ptr[0]))
                {
                    return 2;
                }
                break;

            case 'a':
                if(num_args < 1)
                {
                    return 0;
                }
                if(espcomm_set_address(arg_ptr[0]))
                {
                    return 2;
                }
                break;

            case 'f':
                if(num_args < 1)
                {
                    return 0;
                }
                if(espcomm_upload_file(arg_ptr[0]))
                {
                    return 2;
                }
                break;

            case 'e':
                if (num_args < 1)
                {
                    return 0;
                }
                if (upload_elf_section(arg_ptr[0]))
                {
                    return 2;
                }
                break;

            case 'z':
                if(num_args < 1)
                {
                    return 0;
                }
                if(upload_file_compressed(arg_ptr[0]))
                {
                    return 2;
                }
                break;


            case 'd':
                if (num_args < 1)
                {
                    return 0;
                }
                if (espcomm_set_board(arg_ptr[0]))
                {
                    return 2;
                }

            default:
                return 0;
                break;
        }
    }
    return 0;
}
