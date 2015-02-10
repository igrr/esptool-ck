[![Linux build status](http://img.shields.io/travis/igrr/esptool-ck.svg)](https://travis-ci.org/igrr/esptool-ck)
[![Windows build status](http://img.shields.io/appveyor/ci/igrr/esptool-ck.svg)](https://ci.appveyor.com/project/igrr/esptool-ck)


**Esptool** is a tool to create firmware files for the ESP8266 chip and flash the firmware to the chip over serial port. Esptool runs on Windows, Linux and Mac OS X.

Esptool reads the compiled program in ELF format, extracts code and data sections, and either dumps a section to a file or assembles the firmware file from several segments. Esptool also communicates with the ESP8266 bootloader to upload firmware files to flash. Esptool can automatically put the board into UART bootloader mode using a variety of methods.

Usage
-----
```
esptool <arguments>
```

The program interprets the arguments given on the command line, and in the order they are given.
The following commands are currently available:

Argument | Description
---------|-------------
```-eo <filename>``` | Opens an ELF object file, parses it and caches some of the information found therein. Only works if there is no ELF file currently opened.
```-es <section> <filename>``` | Reads the given section from the ELF file and makes a raw dump into the specified file.
```-ec``` | Closes the currently opened ELF file
```-bo <filename> ```| Prepares an firmware file in the format that is understood by the ESP chip. Only works if an ELF file is opened, and if no firmware file is prepared yet. The result of the operations done on the firmware image are saved when the it is finally closed using -bc command.
```-bs <section>``` | Reads the specified section from the ELF file and appends it to the firmware image.
```-bc``` | Closes the firmware image and saves the result as file to disk.
```-v``` | Increase verbosity level of the tool. Add more v's to increase it even more, e.g. -vv, -vvv.
```-q``` | Disable most of the output.
```-cp <device>``` | Select the serial port device to use for communicating with the ESP. Default is /dev/ttyUSB0 on Linux, COM1 on Windows, /dev/tty.usbserial on Mac OS X.
```-cd <board>``` | Select the reset method to use for resetting the board. Currently supported methods are listed below.
```-cb <baudrate>``` | Select the baudrate to use, default is 115200.
```-ca <address>``` | Address in flash memory to upload the data to. This address is interpreted as hexadecimal. Default is 0x00000000.
```-cf <filename>``` | Upload the file to flash

Upon -bo the tool will start out with an empty image where only
the main header is set accordingly. Sections appear in the
firmware image in the exact same order as the -bs commands
are executed.

Parameters are executed in the order they appear. That means
that if, for example, you want a different port or baudrate
for the flash-upload, those parameters must appear before
the -cf parameter.

Supported boards
----------------

Name       | Description
-----------|-------------
none       | No DTR/RTS manipulation
ck         | RTS controls RESET or CH_PD, DTR controls GPIO0
wifio      | TXD controls GPIO0 via PNP transistor and DTR controls RESET via a capacitor

Support for NodeMCU board is comming up.

Examples
--------

##### Create firmware files from the ELF output
Input: ```app.elf```, output: ```app_00000.bin```, ```app_40000.bin```.
```
esptool -eo app.elf -bo 00000.bin -bs .text -bs .data -bs .rodata -bc -ec -eo app.elf -es .irom0.text 40000.bin -ec
```

##### Upload the firmware to the board connected to COM5 port at 115200 baud, using manual reset method.
```
esptool -cp COM5 -cd none -cb 115200 -ca 0x00000 -cf 00000.bin -ca 0x40000 -cf 40000.bin
```
or, equivalent:
```
esptool -cp COM5 -cf 00000.bin -ca 0x40000 -cf 40000.bin
```


License
-------
Copyright (C) 2014 Christian Klippel <ck@atelier-klippel.de>.

This code is licensed under GPL v2.


