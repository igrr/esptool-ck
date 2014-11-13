TARGET_ARCH	= LINUX 
CFLAGS		= -Os
SDK_LIBDIR	=
SDK_LIBS	=
SDK_INCLUDES	=

CC		:= gcc
LD		:= gcc

MODULES		:= infohelper elf binimage argparse


-include local/Makefile.local.$(TARGET_ARCH)
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix build/,$(MODULES))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,build/%.o,$(SRC))
INCLUDES	:= $(addprefix -I,$(SRC_DIR))

vpath %.c $(SRC_DIR)

define make-goal
$1/%.o: %.c
	$(CC) $(INCLUDES) $(SDK_INCLUDES) $(CFLAGS) -D$(TARGET_ARCH) -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET)

$(TARGET): $(OBJ) main.c
	gcc $(INCLUDES) $(SDK_INCLUDES) $(CFLAGS) $(SDK_LIBDIR) $(SDK_LIBS) -D$(TARGET_ARCH) $(TARGET_FLAGS) $^ -o $@
	strip $(TARGET)


checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))
