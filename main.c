/**********************************************************************************
 **********************************************************************************
 ***
 ***    esptool main routine
 ***    - parses the command line parameters and executes the given commands
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "infohelper.h"
#include "argparse.h"

int main(int argc, char **argv)
{
    int num_args;
    int num_args_parsed;
    int cur_arg;
    char **arg_ptr;
    
    num_args = argc-1;
    arg_ptr = argv;
    arg_ptr++;
    
    if(argc < 2)
    {
        printf("**ERROR: No arguments given. Exiting...\r\n\r\n");
        return 0;
    }

    infohelper_set_argverbosity(num_args, arg_ptr);
    
    iprintf(0, "esptool v0.0.2 - (c) 2014 Ch. Klippel <ck@atelier-klippel.de>\r\n");
    iprintf(0, "This program is licensed under the GPL v2\r\n");
    iprintf(0, "See the file LICENSE for details\r\n\r\n");
    
    while(num_args)
    {
        num_args_parsed = parse_arg(num_args, arg_ptr);
        if(num_args_parsed == 0)
        {
            goto EXITERROR;
        }
        
        num_args -= num_args_parsed;
        arg_ptr += num_args_parsed;
    }

    iprintf(0, "\r\n");
    close_elf_object();
    binimage_write_close(16);
    return 0;
    
EXITERROR:
    close_elf_object();
    binimage_write_close(16);
    exit -1;
}
