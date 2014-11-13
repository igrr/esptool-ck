#include "infohelper.h"

int parse_arg(int num_args, char **arg_ptr)
{
    char *cur_cmd;
    
    if(arg_ptr[0][0] == '-' && num_args)
    {
        switch(arg_ptr[0][1])
        {
            case 'd':
                iprintf(-2, "USING -d IS DEPRECATED, USE -es INSTEAD. FIXING COMMAND\r\n");
                arg_ptr[0][1] = 'e';
                arg_ptr[0][2] = 's';
            case 'e':
                return argparse_elfcmd(num_args, arg_ptr);
                break;
                
            case 'b':
                return argparse_binimagecmd(num_args, arg_ptr);
                break;

            default:
                return 0;
                break;
                        
        }
    }
    return 0;
}
