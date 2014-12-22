/**********************************************************************************
 **********************************************************************************
 ***
 ***    argparse.c
 ***    - simple parser for command line arguments
 ***      checks which command submodule an argument is for and then dispatches
 ***      processing to the apropriate parser
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

#include "infohelper.h"
#include "argparse_binimagecmd.h"
#include "argparse_commcmd.h"
#include "argparse_elfcmd.h"

// main parser state machine, returns the number of arguments used by
// the called sub-parser
#include "argparse.h"

int parse_arg(int num_args, char **arg_ptr)
{
    if(arg_ptr[0][0] == '-' && num_args)
    {
        switch(arg_ptr[0][1])
        {
            // ELF file related commands
            case 'd':
                LOGWARN("USING -d IS DEPRECATED, USE -es INSTEAD. FIXING COMMAND");
                arg_ptr[0][1] = 'e';
                arg_ptr[0][2] = 's';
            case 'e':
                return argparse_elfcmd(num_args, arg_ptr);
                break;
                
            // binary flash image related commands
            case 'b':
                return argparse_binimagecmd(num_args, arg_ptr);
                break;
                
            // comms related commands (flash upload, etc.)
            case 'c':
                return argparse_commcmd(num_args, arg_ptr);
                break;

            // verbosity setting commands
            case 'v':
                return 1;
                break;

            default:
                return 0;
                break;
        }
    }
    
    return 0;
}
