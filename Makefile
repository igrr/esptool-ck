CFLAGS		+= -std=gnu99 -Os -Wall
CXXFLAGS	+= -std=c++11 -Os -Wall


ifeq ($(OS),Windows_NT)
    TARGET_OS := WINDOWS
    DIST_SUFFIX := windows
    ARCHIVE_CMD := 7z a
    ARCHIVE_EXTENSION := zip
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        TARGET_OS := LINUX
        UNAME_M := $(shell uname -m)
	    ifeq ($(UNAME_M),x86_64)
	        DIST_SUFFIX := linux64
	    endif
	    ifeq ($(UNAME_M),i686)
	        DIST_SUFFIX := linux32
	    endif
        ifeq ($(UNAME_M),armv6l)
            DIST_SUFFIX := linux-armhf
        endif
    endif
    ifeq ($(UNAME_S),Darwin)
        TARGET_OS := OSX
        DIST_SUFFIX := osx
    endif
    ifeq ($(UNAME_S),FreeBSD)
        TARGET_OS := FREEBSD
        DIST_SUFFIX := freebsd
    endif
    ARCHIVE_CMD := tar czf
    ARCHIVE_EXTENSION := tar.gz
endif

VERSION ?= $(shell git describe --always)

MODULES		:= infohelper elf binimage argparse serialport espcomm

-include local/Makefile.local.$(TARGET_OS)

OBJECTS		:= \
	argparse/argparse.o \
	argparse/argparse_binimagecmd.o \
	argparse/argparse_commcmd.o \
	argparse/argparse_elfcmd.o \
	binimage/esptool_binimage.o \
	elf/esptool_elf.o \
	elf/esptool_elf_object.o \
	espcomm/delay.o \
	espcomm/espcomm.o \
	espcomm/espcomm_boards.o \
	infohelper/infohelper.o \
	serialport/serialport.o \
	main.o

INCLUDES	:= $(addprefix -I,$(MODULES))

CFLAGS += $(TARGET_CFLAGS)
CXXFLAGS += $(TARGET_CXXFLAGS)
LDFLAGS += $(TARGET_LDFLAGS)
CPPFLAGS += $(INCLUDES) $(SDK_INCLUDES) -D$(TARGET_OS) -DVERSION=\"$(VERSION)\"

DIST_NAME := esptool-$(VERSION)-$(DIST_SUFFIX)
DIST_DIR := $(DIST_NAME)
DIST_ARCHIVE := $(DIST_NAME).$(ARCHIVE_EXTENSION)


.PHONY: all checkdirs clean dist

all: $(TARGET)

dist: $(TARGET) $(DIST_DIR)
	cp $(TARGET) $(DIST_DIR)/
	$(ARCHIVE_CMD) $(DIST_ARCHIVE) $(DIST_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)
	strip $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@

$(DIST_DIR):
	@mkdir -p $@

clean:
	@rm -f $(OBJECTS)
	@rm -f $(TARGET)
	@rm -rf esptool-*
