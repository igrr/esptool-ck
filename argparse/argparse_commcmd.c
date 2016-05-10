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
#include "infohelper.h"
#include "esptool_elf.h"
#include "esptool_elf_object.h"
#include "esptool_binimage.h"
#include "espcomm.h"

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

            case 'd':
                if (num_args < 1)
                {
                    return 0;
                }
                if (espcomm_set_board(arg_ptr[0]))
                {
                    return 2;
                }

            case 'e':
                if (espcomm_erase_flash())
                {
                    return 2;
                }

            case 'c':
                if (num_args < 1)
                {
                    return 0;
                }
                if (espcomm_set_chip(arg_ptr[0]))
                {
                    return 2;
                }

            case 'r':
                if (espcomm_reset())
                {
                    return 1;
                }
                return 0;

            default:
                return 0;
                break;
        }
    }
    return 0;
}
