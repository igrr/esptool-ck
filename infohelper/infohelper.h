/**********************************************************************************
 **********************************************************************************
 ***
 ***    infohelper.h
 ***    - defines and prototypes for a crude verbositiy-controllabe info output
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

#ifndef INFOHELPER_H
#define INFOHELPER_H

/*
** some defines needed to make an overloaded printf() suitable for
** info messages with selectable verbosity.
*/

#define printf_4        printf_3
#define printf_5        printf_3
#define printf_6        printf_3
#define printf_7        printf_3
#define printf_8        printf_3
#define printf_9        printf_3
#define printf_10       printf_3

#define COUNT_PARMS2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _, ...) _
#define COUNT_PARMS(...)\
        COUNT_PARMS2(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define CAT(A, B) CAT2(A, B)
#define CAT2(A, B) A ## B

#define iprintf(...)\
        CAT(printf_, COUNT_PARMS(__VA_ARGS__))(__VA_ARGS__)

/*
** set verbositiy level
** 0 = only error messages
** 1 = minimal messages
** 2 = lot's of messages
** 3 = a few more
*/
void infohelper_set_infolevel(char lvl);

/*
** increase verbositylevel by 1
*/
void infohelper_increase_infolevel(void);

/*
** set verbosity level according to arguments given
*/
void infohelper_set_argverbosity(int num_args, char **arg_ptr);

#endif
