#include <stdio.h>
#include <fcntl.h>
#include <asm/termios.h>
extern int ioctl (int __fd, unsigned long int __request, ...) __THROW;

int weirdbaud (int serial_port, int speed) {
	struct termios2 tio;
	ioctl(serial_port, TCGETS2, &tio);
	tio.c_cflag &= ~CBAUD;
	tio.c_cflag |= BOTHER;
	tio.c_ispeed = speed;
	tio.c_ospeed = speed;
	int r = ioctl(serial_port, TCSETS2, &tio);
	return r;
}
