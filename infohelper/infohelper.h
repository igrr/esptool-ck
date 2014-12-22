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
** print an info/warning/error string
** warnings (error = 0) and errors (error < 0) are printed always
** any messages with error > 0 are printed only if error is <= the current infolevel
*/
int info_printf(int error, const char *fromat, ...) __attribute__ ((format (printf, 2, 3)));
        
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

void infohelper_print_progress(char *msg, float cval, float maxval);

#endif
