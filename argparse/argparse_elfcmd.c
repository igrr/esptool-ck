/**********************************************************************************
 **********************************************************************************
 ***
 ***    argparse_elfcmd.c
 ***    - parsing of ELF file related commands
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

#include "argparse_elfcmd.h"
#include "esptool_elf.h"
#include "esptool_elf_object.h"

int argparse_elfcmd(int num_args, char **arg_ptr)
{
    char *cur_cmd;
    
    if(arg_ptr[0][1] == 'e' && num_args--)
    {
        cur_cmd = &arg_ptr[0][2];
        arg_ptr++;
        
        switch(*cur_cmd++)
        {
            // open ELF file
            case 'o':
                if(num_args < 1)
                {
                    return 0;
                }
                else if(create_elf_object(arg_ptr[0]))
                {
                    return 2;
                }
                break;
                
            // command to save a section from the ELF to a new file
            case 's':
                if(num_args < 2)
                {
                    return 0;
                }
                else if(save_elf_section_bindata(arg_ptr[0], arg_ptr[1]))
                {
                    return 3;
                }
                break;

            // close ELF file
            case 'c':
                if(close_elf_object())
                {
                    return 1;
                }
                break;
                
            default:
                return 0;
                break;
        }
    }
    return 0;
}
