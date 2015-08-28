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

            case '?':
            	INFO(\
"esptool\n" \
"-------\n" \
"-eo <filename>	Opens an ELF object file, parses it and caches some of the information found therein. Only works if there is no ELF file currently opened.\n" \
"-es <section> <filename> Reads the given section from the ELF file and makes a raw dump into the specified file.\n" \
"-ec            Closes the currently opened ELF file.\n" \
"-bo <filename> Prepares an firmware file in the format that is understood by the ESP chip. Only works if an ELF file is opened, and if no firmware file is prepared yet. Upon -bo the tool will start out with an empty image where only the main header is set up. The result of the operations done on the firmware image are saved when the it is finally closed using -bc command.\n" \
"-bm <qio|qout|dio|dout> Set the flash chip interface mode. Default is QIO. This parameter is stored in the binary image header, along with the flash size and flash frequency. The ROM bootloader in the ESP8266 uses the value of these parameters in order to know how to talk to the flash chip.\n" \
"-bz <512K|256K|1M|2M|4M|8M|16M|32M>	Set the flash chip size. Default is 512K.\n" \
"-bf <40|26|20|80> Set the flash chip frequency, in MHz. Default is 40M.\n" \
"-bs <section>  Reads the specified section from the ELF file and appends it to the firmware image. Sections will appear in the firmware image in the exact same order as the -bs commands are executed.\n" \
"-bc            Closes the firmware image and saves the result as file to disk.\n" \
"-v             Increase verbosity level of the tool. Add more v's to increase it even more, e.g. -vv, -vvv.\n" \
"-q             Disable most of the output.\n" \
"-cp <device>   Select the serial port device to use for communicating with the ESP. Default is /dev/ttyUSB0 on Linux, COM1 on Windows, /dev/tty.usbserial on Mac OS X.\n" \
"-cd <board>    Select the reset method to use for resetting the board. Currently supported methods are listed below.\n" \
"-cb <baudrate> Select the baudrate to use, default is 115200.\n" \
"-ca <address>  Address in flash memory to upload the data to. This address is interpreted as hexadecimal. Default is 0x00000000.\n" \
"-cf <filename> Upload the file to flash. Parameters that set the port, baud rate, and address must preceed the -cf command.\n");
            	break;

            default:
                return 0;
                break;
        }
    }
    
    return 0;
}
