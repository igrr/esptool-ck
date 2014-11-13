esptool is a tool to allow the creation and handling of firmware
files suitable for the ESP8266 chip.

It can read a code file in the ELF format that the compilers produce.
From there it is possible to extract various sections that are present
in these files, to either directly dump a section to a file or to
assemble a firmware file comprised of several segments.

While the codebase implements many functions, only a few are used
by the program currently. The program interprets the arguments
given on the commandline, and in the order they are given.

The following commands are currently available:

-eo <filename> :        Opens an ELF object file, parses it and caches
                        some of the information found therein.
                        Only works if there is no ELF file currently
                        opened.

-ec                     Closes the currently opened ELF file

-bo <filename>          Prepares an firmware file in the format
                        that is understood by the ESP chip.
                        Only works if an ELF file is opened, and
                        if no firmware file is prepared yet.
                        The result of the operations done on there
                        firmware image are saved when the it
                        is finally closed.

-bs <section>           Reads the specified section from the ELF
                        file and appends it to the firmware image.
                        
-bc                     Closes the firmware image and saves the result
                        as file to disk.

-d <section> <filename> Reads the given section from the ELF file
                        and makes a raw dump into the specified
                        file.

-v                      Increase verbosity level of the tool

When creating firmware images, the tool will automatically set the
required addresses, size-fields, etc. according to what it finds
in the ELF file.

Upon -bo it will start out with an empty image where only
the main header is set accordingly. Sections appear in the
firmware image in the exact same order as the -bs commands
are executed.

