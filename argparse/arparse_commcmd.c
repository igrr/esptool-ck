#include <stdio.h>
#include "infohelper.h"
#include "esptool_elf.h"
#include "esptool_elf_object.h"
#include "esptool_binimage.h"
#include "../espcomm/espcomm.h"

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

            default:
                return 0;
                break;
        }
    }
    return 0;
}
