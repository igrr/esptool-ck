# OS detection. Not used in CI builds
ifndef TARGET_OS
ifeq ($(OS),Windows_NT)
	TARGET_OS := win32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		UNAME_M := $(shell uname -m)
		ifeq ($(UNAME_M),x86_64)
			TARGET_OS := linux64
		endif
		ifeq ($(UNAME_M),i686)
			TARGET_OS := linux32
		endif
		ifeq ($(UNAME_M),armv6l)
			TARGET_OS := linux-armhf
		endif
	endif
	ifeq ($(UNAME_S),Darwin)
		TARGET_OS := osx
	endif
	ifeq ($(UNAME_S),FreeBSD)
		TARGET_OS := freebsd
	endif
endif
endif # TARGET_OS

# OS-specific settings and build flags
ifeq ($(TARGET_OS),win32)
	ARCHIVE ?= zip
	TARGET := esptool.exe
	TARGET_LDFLAGS = -Wl,-static -static-libgcc
else
	ARCHIVE ?= tar
	TARGET := esptool
endif

ifeq ($(TARGET_OS),osx)
	TARGET_CFLAGS   = -mmacosx-version-min=10.6 -arch i386 -arch x86_64
	TARGET_CXXFLAGS = -mmacosx-version-min=10.6 -arch i386 -arch x86_64
	TARGET_LDFLAGS  = -mmacosx-version-min=10.6 -arch i386 -arch x86_64
endif

# Packaging into archive (for 'dist' target)
ifeq ($(ARCHIVE), zip)
	ARCHIVE_CMD := zip -r
	ARCHIVE_EXTENSION := zip
endif
ifeq ($(ARCHIVE), tar)
	ARCHIVE_CMD := tar czf
	ARCHIVE_EXTENSION := tar.gz
endif

VERSION ?= $(shell git describe --always)

MODULES := infohelper elf binimage argparse serialport espcomm

OBJECTS := \
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

INCLUDES := $(addprefix -I,$(MODULES))

override CFLAGS := -std=gnu99 -Os -Wall $(TARGET_CFLAGS) $(CFLAGS)
override CXXFLAGS := -std=c++11 -Os -Wall $(TARGET_CXXFLAGS) ($CXXFLAGS)
override LDFLAGS := $(TARGET_LDFLAGS) $(LDFLAGS)
override CPPFLAGS := $(INCLUDES) $(SDK_INCLUDES) -DVERSION=\"$(VERSION)\" $(CPPFLAGS)

DIST_NAME := esptool-$(VERSION)-$(TARGET_OS)
DIST_DIR := $(DIST_NAME)
DIST_ARCHIVE := $(DIST_NAME).$(ARCHIVE_EXTENSION)


all: $(TARGET)

dist: $(DIST_ARCHIVE)

$(DIST_ARCHIVE): $(TARGET) $(DIST_DIR)
	cp $(TARGET) $(DIST_DIR)/
	$(ARCHIVE_CMD) $(DIST_ARCHIVE) $(DIST_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)
	strip $(TARGET) 2>/dev/null \
	|| $(CROSS_TRIPLE)-strip $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@

$(DIST_DIR):
	@mkdir -p $@

clean:
	@rm -f $(OBJECTS)
	@rm -f $(TARGET)
	@rm -rf $(DIST_DIR)
	@rm -f $(DIST_ARCHIVE)

.PHONY: all clean dist
