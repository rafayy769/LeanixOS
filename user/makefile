include $(TOP_DIR)/config.mk

SOURCES = $(shell find . -name "*.c")

CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-pic -I$(TOP_DIR)/libc/include
LDFLAGS = -m elf_i386 -nostdlib -static -T user.ld -flto

# libc gets statically linked in each user program (already built by the main makefile)
LIBC 		   = ../libc/libc.a

USER_PROGS = ${SOURCES:.c=}

BUILD_DIR = build

.PHONY: all clean

all: $(BUILD_DIR) $(USER_PROGS)

# Linking: Create each program from its corresponding object file
$(USER_PROGS): %: $(BUILD_DIR)/%.o $(LIBC)
	$(TRACE_LD)
	$(Q) $(LD) $(LDFLAGS) -o $@ $< $(LIBC)

# Compilation: Create object files
$(BUILD_DIR)/%.o: %.c
	$(TRACE_CC)
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	$(TRACE_MKDIR)
	$(Q) mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(USER_PROGS)