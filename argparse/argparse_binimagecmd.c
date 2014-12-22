/**********************************************************************************
 **********************************************************************************
 ***
 ***    argparse_binimagecmd.c
 ***    - parsing of command related to binary flash image functions
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

#include "infohelper.h"
#include "esptool_elf_object.h"
#include "esptool_binimage.h"

int argparse_binimagecmd(int num_args, char **arg_ptr)
{
    char *cur_cmd;
    
    if(arg_ptr[0][1] == 'b' && num_args--)
    {
        cur_cmd = &arg_ptr[0][2];
        arg_ptr++;

        switch(*cur_cmd++)
        {
            case 'o':
                if(num_args < 1)
                {
                    return 0;
                }
                if(binimage_prepare(arg_ptr[0], get_elf_entry()))
                {
                    return 2;
                }
                break;
                
            case 's':
                if(num_args < 1)
                {
                    return 0;
                }
                if(binimagecmd_add_named_elfsegment(arg_ptr[0], 4))
                {
                    return 2;
                }
                break;

            case 'c':
                if(binimage_write_close(16))
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
