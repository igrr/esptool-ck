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

static void print_help()
{
    const char* help = "\
esptool v" VERSION " - (c) 2014 Ch. Klippel <ck@atelier-klippel.de>\n\
ESP8266/ESP32 build and upload helper tool\n\
Maintained by Ivan Grokhotkov: https://github.com/igrr/esptool-ck\n\
\n\
The program interprets arguments given on the command line, and in the order\n\
they are given.\n\
\n\
-eo <filename>	\n\
    Open an ELF object file, parse it and cache some of the information found \n\
    therein. Works only if there is no ELF file currently opened.\n\
\n\
-es <section> <filename> \n\
    Read the given section from the ELF file and make a raw dump into the \n\
    specified file.\n\
-ec\n\
    Close the currently opened ELF file.\n\
\n\
-bo <filename> \n\
    Prepare a firmware file in the format that is understood by the ESP chip. \n\
    Works only if an ELF file is opened, and if no firmware file is prepared yet.\n\
    Upon -bo the tool will start out with an empty image where only the main \n\
    header is set up. The result of the operations done on the firmware image \n\
    is saved when the it is finally closed using -bc command.\n\
\n\
-bm <qio|qout|dio|dout> \n\
    Set the flash chip interface mode. Default is QIO. \n\
    This parameter is stored in the binary image header, along with the flash size\n\
    and flash frequency. The ROM bootloader in the ESP8266 uses the value \n\
    of these parameters in order to know how to talk to the flash chip.\n\
\n\
-bz <512K|256K|1M|2M|4M|8M|16M|32M>	\n\
    Set the flash chip size. Default is 512K.\n\
\n\
-bf <40|26|20|80>\n\
    Set the flash chip frequency, in MHz. Default is 40M.\n\
\n\
-bs <section>\n\
    Read the specified section from the ELF file and append it to the \n\
    firmware image. Sections will appear in the firmware image in the exact \n\
    same order as the -bs commands are executed.\n\
\n\
-bp <size>\n\
    Finalize the firmware image, padding it with '0xaa' value until it is at least\n\
    <size> bytes long. Unlike -bc, this doesn't close the file.\n\
    This option can be used to combine bootloader with the rest of the application\n\
\n\
-br <size>\n\
    Pad all the following sections to multiples of <size>. Default is 4 bytes.\n\
    This option can be used to place sections on specific boundaries, e.g. 4k or 64k.\n\
\n\
-bc\n\
    Close the firmware image and save the result as file to disk.\n\
\n\
-v\n\
    Increase verbosity level of the tool.\n\
    Add more v's to increase it even more, e.g. -vv, -vvv.\n\
\n\
-q\n\
    Disable most of the output.\n\
\n\
-cp <device>\n\
    Select the serial port device to use for communicating with the ESP.\n\
    Default is /dev/ttyUSB0 on Linux, COM1 on Windows, /dev/tty.usbserial on Mac OS.\n\
\n\
-cd <board>\n\
    Select the reset method to use for resetting the board.\n\
    Currently supported methods are: none, ck, nodemcu, wifio.\n\
\n\
-cb <baudrate>\n\
    Select the baudrate to use, default is 115200.\n\
\n\
-ca <address>\n\
    Address in flash memory to upload the data to.\n\
    This address is interpreted as hexadecimal. Default is 0x00000000.\n\
\n\
-cf <filename>\n\
    Upload the file to flash. Parameters that set the port, baud rate, and address\n\
    must preceed the -cf command.\n\
\n\
-cr\n\
    Reset chip into app using the selected reset method.\n\
\n\
-ce\n\
    Erase flash.\n\
\n\
-cc <chip>\n\
    Select chip to upload to. Currently supported values: 'esp8266' (default), 'esp32'\n\
\n";
    INFO(help);
}

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

            case 'q':
                return 1;
                break;

            case '?':
            case 'h':
            	print_help();
                return 1;
            	break;

            default:
                return 0;
                break;
        }
    }

    return 0;
}
