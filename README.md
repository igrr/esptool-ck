**Esptool** is a tool to create firmware files for the ESP8266 chip and flash the firmware to the chip over serial port. Esptool runs on Windows, Linux and Mac OS X.

Esptool reads the compiled program in ELF format, extracts code and data sections, and either dumps a section to a file or assembles the firmware file from several segments. Esptool also communicates with the ESP8266 bootloader to upload firmware files to flash. Esptool can automatically put the board into UART bootloader mode using a variety of methods.

Build status
------------
[![Build status](http://img.shields.io/travis/igrr/esptool-ck.svg)](https://travis-ci.org/igrr/esptool-ck)


Usage
-----
```
esptool <arguments>
```

The program interprets the arguments given on the command line, and in the order they are given.
The following commands are currently available:

Argument | Description
---------|-------------
```-eo <filename>``` | Open an ELF object file, parse it and cache some of the information found therein. Works only if there is no ELF file currently opened.
```-es <section> <filename>``` | Read the given section from the ELF file and make a raw dump into the specified file.
```-ec``` | Close the currently opened ELF file
```-bo <filename> ```| Prepare a firmware file in the format that is understood by the ESP chip. Works only if an ELF file is opened, and if no firmware file is prepared yet. Upon -bo the tool will start out with an empty image where only the main header is set up. The result of the operations done on the firmware image is saved when the it is finally closed using -bc command.
```-bm <qio/qout/dio/dout>``` | Set the flash chip interface mode. Default is QIO. This parameter is stored in the binary image header, along with the flash size and flash frequency. The ROM bootloader in the ESP8266 uses the value of these parameters in order to know how to talk to the flash chip.
```-bz <512K/256K/1M/2M/4M/8M/16M/32M>``` | Set the flash chip size. Default is 512K.
```-bf <40/26/20/80>``` | Set the flash chip frequency, in MHz. Default is 40M.
```-bs <section>``` | Read the specified section from the ELF file and append it to the firmware image. Sections will appear in the firmware image in the exact same order as the -bs commands are executed.
```-bp <size>``` | Finalize the firmware image, padding it with `0xaa` value until it is at least `<size>` bytes long. Unlike `-bc`, this doesn't close the file. This option can be used to combine bootloader with the rest of the application
```-br <size>``` | Pad all the following sections to multiples of `<size>`. Default is 4 bytes. This option can be used to place sections on specific boundaries, e.g. 4k or 64k.
```-bc``` | Close the firmware image and save the result as file to disk.
```-v``` | Increase verbosity level of the tool. Add more v's to increase it even more, e.g. -vv, -vvv.
```-q``` | Disable most of the output.
```-cp <device>``` | Select the serial port device to use for communicating with the ESP. Default is /dev/ttyUSB0 on Linux, COM1 on Windows, /dev/tty.usbserial on Mac OS X.
```-cd <board>``` | Select the reset method to use for resetting the board. Currently supported methods are listed below.
```-cb <baudrate>``` | Select the baudrate to use, default is 115200.
```-ca <address>``` | Address in flash memory to upload the data to. This address is interpreted as hexadecimal. Default is 0x00000000.
```-cf <filename>``` | Upload the file to flash. Parameters that set the port, baud rate, and address must precede the -cf command.
```-cp <size>``` | Pad last written section of firmware image to the given size, in bytes.
```-cz <size>``` | Erase `size` bytes of flash starting from address set using `-ca` flag. Erase region boundaries must be aligned to 4kB.
```-ce``` | Erase flash
```-cr``` | Reset chip into app using the selected reset method

Supported boards
----------------

Name       | Description
-----------|-------------
none       | No DTR/RTS manipulation
ck         | RTS controls RESET or CH_PD, DTR controls GPIO0
wifio      | TXD controls GPIO0 via PNP transistor and DTR controls RESET via a capacitor
nodemcu    | GPIO0 and RESET controlled using two NPN transistors as in [NodeMCU devkit](https://raw.githubusercontent.com/nodemcu/nodemcu-devkit/master/Documents/NODEMCU_DEVKIT_SCH.png).
dtrset     | DTR is set during serial communications

Examples
--------

##### Create firmware files from the ELF output
Input: ```app.elf```, output: ```app_00000.bin```, ```app_40000.bin```.
Note the option for 4M flash size.
```
esptool -bz 4M -eo app.elf -bo app_00000.bin -bs .text -bs .data -bs .rodata -bc -ec -eo app.elf -es .irom0.text app_40000.bin -ec
```

##### Upload the firmware to the board connected to COM5 port at 115200 baud, using manual reset method.
```
esptool -cp COM5 -cd none -cb 115200 -ca 0x00000 -cf 00000.bin -ca 0x40000 -cf 40000.bin
```
or, equivalent:
```
esptool -cp COM5 -cf 00000.bin -ca 0x40000 -cf 40000.bin
```

##### Erase flash on a NodeMCU board
```
esptool -cp /dev/ttyUSB0 -cd nodemcu -ce
```

License
-------
Copyright (C) 2014 Christian Klippel <ck@atelier-klippel.de>.

This code is licensed under GPL v2.
