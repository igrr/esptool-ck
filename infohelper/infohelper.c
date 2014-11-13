/**********************************************************************************
 **********************************************************************************
 ***
 ***    infohelper.c
 ***    - implementation of a crude verbosity-controllabe info system
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
#include <stdarg.h>
#include "infohelper.h"

static char infolevel  = 1;

int printf_1(int err)
{
    printf("**ERROR: %d\r\n", err);
    return 0;
}

int printf_2(int error, char *string)
{
    if(error < -1)
    {
        printf("**WARNING: %s", string);
    }
    else if(error < 0)
    {
        printf("**ERROR: %s", string);
    }
    else if(error < infolevel)
    {
        printf("Info: %s", string);
    }
    
    return 0;
}

int printf_3(int error, char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

int printf_3(int error, char *fmt, ...)
{
    va_list v;
    

    if(error < -1)
    {
        printf("**WARNING: ");
        va_start(v, fmt);
        vprintf(fmt, v);
        va_end(v);
    }
    else if(error < 0)
    {
        printf("**ERROR: ");
        va_start(v, fmt);
        vprintf(fmt, v);
        va_end(v);
    }
    else if(error < infolevel)
    {
        printf("Info: ");
        va_start(v, fmt);
        vprintf(fmt, v);
        va_end(v);
    }
    return 0;
}

void infohelper_set_infolevel(char lvl)
{
    infolevel = lvl;
}

void infohelper_increase_infolevel(void)
{
    if(infolevel < 20)
    {
        infolevel++;
    }
}

void infohelper_set_argverbosity(int num_args, char **arg_ptr)
{
    char *cur_arg;

    infolevel = 0;
    
    while(num_args--)
    {
        cur_arg = arg_ptr[num_args];
        
        if(*cur_arg++ == '-')
        {
            if(*cur_arg == 'q')
            {
                infolevel = 0;
                return;
            }
            else while(*cur_arg++ == 'v')
            {
                infolevel++;
            }
        }
    }
}
