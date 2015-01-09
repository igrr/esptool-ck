/**********************************************************************************
 **********************************************************************************
 ***
 ***    espcomm_boards.h
 ***    - board-specific upload methods interface
 ***
 ***    Copyright (C) 2015 Ivan Grokhotkov
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

#ifndef ESPCOMM_BOARDS_H
#define ESPCOMM_BOARDS_H

struct espcomm_board_;
typedef struct espcomm_board_ espcomm_board_t;

espcomm_board_t* espcomm_board_by_name(const char* name);
espcomm_board_t* espcomm_board_first();
espcomm_board_t* espcomm_board_next(espcomm_board_t* board);

const char* espcomm_board_name(espcomm_board_t* board);
void espcomm_board_reset_into_bootloader(espcomm_board_t* board);
void espcomm_board_reset_into_app(espcomm_board_t* board);




#endif//ESPCOMM_BOARDS_H
