#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp8266_peri.h"
#include "../espcomm/espcomm.h"
#include "cbuf.h"

cbuf_t uart_cbuf;
uint8_t uart_buf[4096];
uint8_t s_decompressed_buf[32768];

bootloader_packet s_packet;
uint8_t s_packet_payload[4096];

#define die(...) \
    do {\
        for (volatile int i = 0; i < 1000000; ++i);\
        ets_printf(__VA_ARGS__);\
        ets_putc('\r');\
        ets_putc('\n');\
    } while(true);


int read_uart() {
    size_t rx_count;
    while ((rx_count = (USS(0) >> USRXC) & 0xff) > 0 && cbuf_room(&uart_cbuf)) {
        cbuf_write(&uart_cbuf, (char) U0F);
    }

    return cbuf_read(&uart_cbuf);
}

int read_uart_timed() {
    int c = 0;
    do {
        c = read_uart();
    } while(c == -1);

    return c;
}

void expect_c0() {
    int val = read_uart_timed();
    if (val != 0xc0) {
        die("framing symbol expected, %d", val);
    }
}

void read_slip(uint8_t* dst, size_t size) {
    for (size_t i = 0; i < size; ++i, ++dst) {
        int c = read_uart_timed();
        if (c == 0xdb) {
            c = read_uart_timed();
            if (c == 0xdc) {
                *dst = 0xc0;
            } else if (c == 0xdd) {
                *dst = 0xdb;
            } else {
                die("invalid slip sequence");
            }
        } else {
            *dst = c;
        }
    }
}

// uint32_t espcomm_calc_checksum(unsigned char *data, uint16_t data_size)
// {
//     uint16_t cnt;
//     uint32_t result;
//
//     result = 0xEF;
//
//     for(cnt = 0; cnt < data_size; cnt++)
//     {
//         result ^= data[cnt];
//     }
//
//     return result;
// }

void read_slip_packet(bootloader_packet* pkt, size_t data_size) {

    expect_c0();

    read_slip((uint8_t*) pkt, 8);
    if (pkt->size > data_size) {
        die("slip packet too large");
    }

    read_slip(pkt->data, pkt->size);

    expect_c0();
}

void write_slip(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; ++i, ++data) {
        int c = *data;
        if (c == 0xc0) {
            ets_putc(0xdb);
            ets_putc(0xdc);
        }
        else if (c == 0xdb) {
            ets_putc(0xdb);
            ets_putc(0xdd);
        }
        else {
            ets_putc(c);
        }
    }
}

void write_slip_packet(bootloader_packet* pkt) {
    ets_putc(0xc0);
    write_slip((uint8_t*) pkt, 8);
    write_slip(pkt->data, pkt->size);
    ets_putc(0xc0);
}

void read_packet() {

    //
    // for(int j = 0; j < 3; ++j) {
    //     for (volatile int i = 0; i < 5000000; ++i);
    //     ets_printf("\r\n %d %d %d %d", (int) pkt.direction, (int) pkt.command, (int) pkt.size, pkt.checksum);
    // }
}

#define FLASH_SECTOR_SIZE 4096
#define FLASH_BLOCK_SIZE (4096*16)

int SPIEraseAreaEx(const uint32_t start, const uint32_t size)
{
    if (start & (FLASH_SECTOR_SIZE - 1) != 0) {
        return 1;
    }

    const uint32_t sectors_per_block = FLASH_BLOCK_SIZE / FLASH_SECTOR_SIZE;
    uint32_t current_sector = start / FLASH_SECTOR_SIZE;
    uint32_t sector_count = (size + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE;
    const uint32_t end = current_sector + sector_count;

    for (; current_sector < end && (current_sector & (sectors_per_block-1));
        ++current_sector, --sector_count) {
        if (SPIEraseSector(current_sector)) {
            return 2;
        }
    }

    for (;current_sector + sectors_per_block <= end;
        current_sector += sectors_per_block,
        sector_count -= sectors_per_block) {
        if (SPIEraseBlock(current_sector / sectors_per_block)) {
            return 3;
        }
    }

    for (; current_sector < end;
        ++current_sector, --sector_count) {
        if (SPIEraseSector(current_sector)) {
            return 4;
        }
    }

    return 0;
}

void decompress(uint8_t* src, size_t src_size, size_t *src_offset, uint8_t* dst, size_t dst_size, size_t *dst_offset) {

}

void main(void)
{
    cbuf_init(&uart_cbuf, uart_buf, sizeof(uart_buf));
    s_packet.data = s_packet_payload;
    size_t offset;
    size_t size;
    size_t decompressed_offset;
    size_t compressed_offset;
    uint32_t* payload = (uint32_t*) s_packet_payload;
    while(true) {
        read_slip_packet(&s_packet, sizeof(s_packet_payload));
        if (s_packet.command == 0x02) {
            size = payload[0];
            offset = payload[3];
            decompressed_offset = 0;
            compressed_offset = 0;
            // SPIEraseAreaEx(offset, size);
            s_packet.direction = 1;
            s_packet.size = 2;
            payload[0] = 0;
            write_slip_packet(&s_packet);
        }
        else if (s_packet.command == 0x03) {
            // decompress(s_packet_payload + 16, s_packet.size - 16,
            //     &compressed_offset,
            //     s_decompressed_buf, sizeof(s_decompressed_buf),
            //     &decompressed_offset);
            // //SPIWrite(offset, s_decompressed_buf, sizeof(s_decompressed_buf));
            s_packet.direction = 1;
            s_packet.size = 2;
            payload[0] = 0;
            write_slip_packet(&s_packet);
        }
        else if (s_packet.command == 0x04) {
            // decompress(s_packet_payload + 16, s_packet.size - 16,
            //     &compressed_offset,
            //     s_decompressed_buf, sizeof(s_decompressed_buf),
            //     &decompressed_offset);
            // //SPIWrite(offset, s_decompressed_buf, sizeof(s_decompressed_buf));
            s_packet.direction = 1;
            s_packet.size = 2;
            payload[0] = 0;
            write_slip_packet(&s_packet);
            return;
        }
        else {
            die("packet: %d %d %x %d", s_packet.direction, s_packet.size, s_packet.checksum, s_packet.command);
        }
    }
}
