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

const char heartbeat[] = { "-\\|/-\\|/" };
static unsigned int beat = 0;
static char infolevel  = 1;

int info_printf(int error, const char *format, ...) 
{
    va_list v;
    int done = 0;

    va_start(v, format);

    if(error < 0)
    {
        if(error > -100)
        {
            printf("** ERROR: ");
            done = vprintf(format, v);
        }
#ifdef DEBUG
        else if(error == -100)
        {
            printf(">> DEBUG: ");
            done = vprintf(format, v);
        }
#endif
    }
    else if(error == 0)
    {
        printf("** Warning: ");
        done = vprintf(format, v);
    }
    else if(infolevel >= error)
    {
        printf("Info: ");
        done = vprintf(format, v);
    }
    else if(error > 100)
    {
        error -= 100;
        if(infolevel >= error)
        {
            done = vprintf(format, v);
        }
    }
    
    va_end(v);
    fflush(stdout);
    return done;
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

void infohelper_print_progress(char *msg, float cval, float maxval)
{
    unsigned int cur, cnt;

    if(cval == 0)
    {
        beat = 0;
    }
    
    cur = (35 / maxval) * cval;

    cnt = 35-cur;
    
    if(msg)
    {
        info_printf(1, "%s: [", msg);
    }
    else
    {
        info_printf(1, "[");
    }

    while(cur--)
    {
        info_printf(101, "*");
    }

    if(cnt)
    {
        if(cval)
        {
            info_printf(101, "%c",heartbeat[beat]);
        }
        else
        {
            info_printf(101, " ");
        }
    }
    else
    {
        info_printf(101, "*");
    }

    while(cnt--)
    {
        info_printf(101, " ");
    }

    info_printf(101, "\b]\r");
    fflush(stdout);
    
    beat++;
    beat &= 0x07;
}
