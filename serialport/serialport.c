/**********************************************************************************
 **********************************************************************************
 ***
 ***    serialport.c
 ***    - low level functions to access the serial port
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

#include "serialport.h"
#include "infohelper.h"

static int serial_port = -1;
static struct termios term;
static unsigned int old_timeout;

void serialport_set_baudrate(unsigned int baudrate)
{
    switch(baudrate)
    {
        case 2400:
            cfsetispeed(&term,B2400);
            cfsetospeed(&term,B2400);
            break;
            
        case 4800:
            cfsetispeed(&term,B4800);
            cfsetospeed(&term,B4800);
            break;
            
        case 9600:
            cfsetispeed(&term,B9600);
            cfsetospeed(&term,B9600);
            break;
            
        case 19200:
            cfsetispeed(&term,B38400);
            cfsetospeed(&term,B38400);
            break;
            
        case 38400:
            cfsetispeed(&term,B38400);
            cfsetospeed(&term,B38400);
            break;
            
        case 57600:
            cfsetispeed(&term,B57600);
            cfsetospeed(&term,B57600);
            break;
            
        case 115200:
            cfsetispeed(&term,B115200);
            cfsetospeed(&term,B115200);
            break;
            
        case 230400:
            cfsetispeed(&term,B230400);
            cfsetospeed(&term,B230400);
            break;
            
        case 460800:
            cfsetispeed(&term,B460800);
            cfsetospeed(&term,B460800);
            break;
            
        case 921600:
            cfsetispeed(&term,B921600);
            cfsetospeed(&term,B921600);
            break;
            
        default:
            break;
    }
}

void serialport_set_timeout(unsigned int timeout)
{
    if(timeout != old_timeout)
    {
        term.c_cc[VMIN]  = 0;
        term.c_cc[VTIME] = timeout;

        info_printf(4, "setting timeout %i\r\n", timeout);
        
        if (tcsetattr(serial_port, TCSANOW, &term)!=0)
        {
            info_printf(4, "set timeout failed\r\n");
        }
        
        old_timeout = timeout;
    }
}

int serialport_open(char *device, unsigned int baudrate)
{
    serial_port = open(device,O_RDWR);
    
    if(serial_port<0) 
    {
        info_printf(-1, "cannot acces %s\n",device);
        return serial_port;
    }

    serialport_set_dtr(1);
    serialport_set_rts(1);

    if(tcgetattr(serial_port,&term) != 0)
    {
        info_printf(-1, "Initial getattr failed\r\n");
        return 0;
    }

    serialport_set_baudrate(baudrate);

    term.c_cflag = (term.c_cflag & ~CSIZE) | CS8;
    term.c_cflag |= CLOCAL | CREAD;
    
    term.c_cflag &= ~(PARENB | PARODD);
    term.c_cflag &= ~CSTOPB;
    
    term.c_iflag = IGNBRK;
    
    term.c_iflag &= ~(IXON | IXOFF);
    
    term.c_lflag = 0;
    
    term.c_oflag = 0;
    
    
    term.c_cc[VMIN]=0;
    term.c_cc[VTIME]=1;
    old_timeout = 1;
    
    
    
    if (tcsetattr(serial_port, TCSANOW, &term)!=0)
    {
        info_printf(-1, "setattr stage 1 failed");
        return 0;
    }

    
    if (tcgetattr(serial_port, &term)!=0)
    {
        info_printf(-1, "getattr failed");
        return 0;
    }
    
    term.c_cflag &= ~CRTSCTS;
    
    if (tcsetattr(serial_port, TCSANOW, &term)!=0)
    {
        info_printf(-1, "setattr stage 2 failed");
        return 0;
    }
    
    return serial_port;
}

static unsigned char subst_C0[2] = { 0xDB, 0xDC };
static unsigned char subst_DB[2] = { 0xDB, 0xDD };

int serialport_send_slip(unsigned char *data, unsigned int size)
{
    unsigned int sent;
    unsigned char cur_byte;
    
    sent = 0;
    
    while(sent != size)
    {
        cur_byte = *data++;
        if(cur_byte == 0xC0)
        {
            if(write(serial_port, subst_C0, 2) != 2)
            {
                info_printf(-1, "failed substituting 0xC0\r\n");
                return 0;
            }
        }
        else if(cur_byte == 0xDB)
        {
            if(write(serial_port, subst_DB, 2) != 2)
            {
                info_printf(-1, "failed substituting 0xDB\r\n");
                return 0;
            }
        }
        else
        {
            if(write(serial_port, &cur_byte, 1) != 1)
            {
                info_printf(-1, "failed sending byte %i\r\n", sent);
                return 0;
            }
        }
        sent++;
    }

    tcdrain(serial_port);
    
    return sent;
}

int serialport_receive_slip(unsigned char *data, unsigned int size)
{
    unsigned int received;
    unsigned char cur_byte;
    
    received = 0;
    
    while(received != size)
    {
        if(read(serial_port, &cur_byte, 1) != 1)
        {
            info_printf(-1, "failed reading byte\r\n");
            return 0;
        }
        
        if(cur_byte == 0xDB)
        {
            if(read(serial_port, &cur_byte, 1) != 1)
            {
                info_printf(-1, "failed reading byte for unslip\r\n");
                return 0;
            }
            
            if(cur_byte == 0xDC)
            {
                *data++ = 0xC0;
            }
            else if(cur_byte == 0xDD)
            {
                *data++ = 0xDB;
            }
            else
            {
                info_printf(-1, "unslip sequence wrong\r\n");
                return 0;
            }
        }
        else
        {
            *data++ = cur_byte;
        }
        
        received++;
    }

    return received;
}

int serialport_send_C0(void)
{
    unsigned char b;

    b = 0xC0;
    
    if(write(serial_port, &b, 1) != 1)
    {
        info_printf(-1, "failed sending 0xC0\r\n");
        return 0;
    }
    serialport_drain();
    return 1;
}

int serialport_receive_C0(void)
{
    unsigned char b;
    
    b = 0x00;
    
    if(read(serial_port, &b, 1) != 1)
    {
        return 0;
    }
    
    if(b != 0xC0)
    {
        return 0;
    }
    
    return 1;
}

void serialport_flush(void)
{
    static unsigned char b;
    unsigned int t;
    
    if(serial_port)
    {
        t = term.c_cc[VTIME];
        
        serialport_set_timeout(1);
        
        tcdrain(serial_port);
        
        while(read(serial_port, &b, 1) > 0) { tcflush(serial_port, TCIOFLUSH); };
       
        serialport_set_timeout(t);
    }
}

void serialport_drain(void)
{
    if(serial_port)
    {
        tcdrain(serial_port);
    }
}

int serialport_close(void)
{
    if(serial_port)
    {
        tcdrain(serial_port);
        tcflush(serial_port, TCIOFLUSH);
        close(serial_port);
        return 1;
    }
    else
    {
        return 0;
    }
}

void serialport_set_dtr(unsigned char val)
{
    int mcs;
    
    if(serial_port)
    {
        ioctl (serial_port, TIOCMGET, &mcs);
        
        if(val)
        {
            mcs |= TIOCM_DTR;
            ioctl (serial_port, TIOCMSET, &mcs);
        }
        else
        {
            mcs &= ~TIOCM_DTR;
            ioctl (serial_port, TIOCMSET, &mcs);
        }
    }
}

void serialport_set_rts(unsigned char val)
{
    int mcs;
    
    if(serial_port)
    {
        ioctl (serial_port, TIOCMGET, &mcs);
        
        if(val)
        {
            mcs |= TIOCM_RTS;
            ioctl (serial_port, TIOCMSET, &mcs);
        }
        else
        {
            mcs &= ~TIOCM_RTS;
            ioctl (serial_port, TIOCMSET, &mcs);
        }
    }
}

