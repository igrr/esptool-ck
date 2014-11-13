int argparse_elfcmd(int num_args, char **arg_ptr)
{
    char *cur_cmd;
    
    if(arg_ptr[0][1] == 'e' && num_args--)
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
                if(create_elf_object(arg_ptr[0]))
                {
                    return 2;
                }
                break;
                
            case 's':
                if(num_args < 2)
                {
                    return 0;
                }
                if(save_elf_section_bindata(arg_ptr[0], arg_ptr[1]))
                {
                    return 3;
                }
                break;

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
