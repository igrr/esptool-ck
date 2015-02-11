CFLAGS		= -std=gnu99 -Os -Wall
SDK_LIBDIR	=
SDK_LIBS	=
SDK_INCLUDES=

ifeq ($(OS),Windows_NT)
    TARGET_OS := WINDOWS
    DIST_SUFFIX := windows
    ZIP_CMD := 7z a
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        TARGET_OS := LINUX
        UNAME_P := $(shell uname -p)
	    ifeq ($(UNAME_P),x86_64)
	        DIST_SUFFIX := linux64
	    endif
	    ifneq ($(filter %86,$(UNAME_P)),)
	        DIST_SUFFIX := linux32
	    endif
    endif
    ifeq ($(UNAME_S),Darwin)
        TARGET_OS := OSX
        DIST_SUFFIX := osx
    endif
    ZIP_CMD := zip -r
endif

VERSION ?= $(shell git describe --always)

MODULES		:= infohelper elf binimage argparse serialport espcomm

-include local/Makefile.local.$(TARGET_OS)
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix build/,$(MODULES))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,build/%.o,$(SRC))
INCLUDES	:= $(addprefix -I,$(SRC_DIR))

CFLAGS += $(TARGET_CFLAGS)

CCOPTS := $(INCLUDES) $(SDK_INCLUDES) $(CFLAGS) -D$(TARGET_OS) -DVERSION=\"$(VERSION)\" 

DIST_NAME := esptool-$(VERSION)-$(DIST_SUFFIX)
DIST_DIR := $(DIST_NAME)
DIST_ZIP := $(DIST_NAME).zip

vpath %.c $(SRC_DIR)

define make-goal
$1/%.o: %.c
	$(CC) $(CCOPTS)-c $$< -o $$@
endef

.PHONY: all checkdirs clean dist

all: checkdirs $(TARGET)

dist: checkdirs $(TARGET) $(DIST_DIR)
	cp $(TARGET) $(DIST_DIR)/
	$(ZIP_CMD) $(DIST_ZIP) $(DIST_DIR)

$(TARGET): $(OBJ) main.c
	gcc $(SDK_LIBDIR) $(SDK_LIBS) $(CCOPTS) $^ -o $@
	strip $(TARGET)

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

$(DIST_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@rm -rf esptool-*

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))
