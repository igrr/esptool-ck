#include <stddef.h>
#include <string.h>
#include "espcomm_boards.h"
#include "serialport.h"
#include "delay.h"

struct espcomm_board_
{
    const char* name;
    void (*reset_into_bootloader)(void);
    void (*reset_into_app)(void);
};

static espcomm_board_t s_boards[];
static size_t s_boards_count;

espcomm_board_t* espcomm_board_by_name(const char* name)
{
    for (size_t i = 0; i < s_boards_count; ++i)
    {
        espcomm_board_t* board = s_boards + i;
        if (strcmp(board->name, name) == 0)
            return board;
    }
    return 0;
}
espcomm_board_t* espcomm_board_first()
{
    return s_boards;
}

espcomm_board_t* espcomm_board_next(espcomm_board_t* board)
{
    if (++board < s_boards + s_boards_count)
        return board;

    return 0;
}

const char* espcomm_board_name(espcomm_board_t* board)
{
    return (board)?board->name:"";
}

void espcomm_board_reset_into_bootloader(espcomm_board_t* board)
{
    if (board && board->reset_into_bootloader)
        (*board->reset_into_bootloader)();
}
void espcomm_board_reset_into_app(espcomm_board_t* board)
{
    if (board && board->reset_into_app)
        (*board->reset_into_app)();
}



/// board definitions go below
//
//  _rb = reset into bootloader
//  _ra = reset into app
//
//

// "ck" board: dtr pulls down gpio0, rts pulls down reset
// also supports reset with RC circuit triggered by break signal

void board_ck_rb()
{
    serialport_set_rts(1);
    serialport_set_dtr(1);
    serialport_send_break();
    espcomm_delay_ms(5);
    serialport_set_rts(0);
    espcomm_delay_ms(250);          // wait for break to finish
    serialport_set_dtr(0);
}

void board_ck_ra()
{
    serialport_set_dtr(0);
    serialport_set_rts(1);
    espcomm_delay_ms(5);
    serialport_set_rts(0);
}

// WIFIO board, rev 2: TXD controls gpio0 via a pnp, and DTR controls rst via a capacitor

void board_wifio_rb()
{
    serialport_set_dtr(0);
    espcomm_delay_ms(5);
    serialport_set_dtr(1);
    espcomm_delay_ms(5);
    serialport_set_dtr(0);
    serialport_send_break();
}

void board_wifio_ra()
{
    serialport_set_dtr(0);
    espcomm_delay_ms(5);
    serialport_set_dtr(1);
}


void board_nodemcu_rb()
{
    serialport_set_rts(1);
    serialport_set_dtr(0);
    espcomm_delay_ms(5);
    serialport_set_rts(0);
    serialport_set_dtr(1);
    espcomm_delay_ms(50);
    serialport_set_dtr(0);
}

void board_nodemcu_ra()
{
    serialport_set_dtr(0);
    serialport_set_rts(1);
    espcomm_delay_ms(5);
    serialport_set_rts(0);
}

/// list of all boards

static espcomm_board_t s_boards[] = {
    { "none",   0,                  0               },
    { "ck",     &board_ck_rb,       &board_ck_ra    },
    { "wifio",  &board_wifio_rb,    &board_wifio_ra},
    { "nodemcu",   &board_nodemcu_rb,     &board_nodemcu_ra},
};

static size_t s_boards_count = sizeof(s_boards) / sizeof(espcomm_board_t);
