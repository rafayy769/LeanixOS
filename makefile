include config.mk

TOP_DIR		  := $(shell pwd)

# system bootloader
BOOTSECTOR_DIR = boot
BOOTSECTOR     = $(BOOTSECTOR_DIR)/bootloader.bin
BOOTSECTOR_ELF = $(BOOTSECTOR_DIR)/build/bootloader.elf

# system/kernel directories and the objects
SYS_OBJ_DIRS   = init mm driver proc fs objs
SYS_OBJS       = init/init.o mm/mm.o driver/driver.o proc/proc.o fs/fs.o objs/impl.o
SYSTEM         = kernel.elf

# libraries and the directories
LIBS_DIRS 	   = libc
LIBC 		   = libc/libc.a
LIBK 		   = libc/libk.a
LIBS      	   = $(LIBC) $(LIBK)

# User-space programs
USER_DIRS      = user
USER_PROGS	   = user/echo user/hello user/greet user/shall

# Test files and the directories
TEST_DIR	   = tests
TESTS 		   = tests/tests.o

# output disk images
DISK_IMG	   = disk.img
FLPY_IMG	   = floppy.img
FS_IMG		   = disk2.img

# toolchain to use
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CC      	  := gcc
	AS      	  := as
	LD      	  := ld
	AR      	  := ar
	OBJCOPY 	  := objcopy
	OBJDUMP 	  := objdump
	GDB     	  := gdb
else ifeq ($(UNAME_S),Darwin)
	CC      	  := i686-elf-gcc
	AS      	  := i686-elf-as
	LD      	  := i686-elf-ld
	AR      	  := i686-elf-ar
	OBJCOPY 	  := i686-elf-objcopy
	OBJDUMP 	  := i686-elf-objdump
	GDB     	  := i386-elf-gdb
endif

CFLAGS        := -m32 -ffreestanding -nostdlib -nostdinc -fno-pic -I$(TOP_DIR)/include -I$(TOP_DIR)/libc/include -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
ASFLAGS       := --32
LDFLAGS       := -m elf_i386 -nmagic -nostdlib -static

# extra flags based on debug/release mode
ifeq ($(D),1)
  CFLAGS  += -g -gdwarf-4 -ggdb3 -DDEBUG
  ASFLAGS += -g
else
  CFLAGS  += -Os -O3 -DNDEBUG
  LDFLAGS += -s -flto
endif

# Check if we're building the test target, we only add tests compilation in 
# case of testing
ifeq (test,$(filter test,$(MAKECMDGOALS)))
    SYS_OBJ_DIRS += tests
    SYS_OBJS += tests/tests.o
    CFLAGS += -DTESTING
endif

KERNEL_LDFLAGS := $(LDFLAGS) -T kernel_high.lds -Map=$(SYSTEM).map
MODULE_LDFLAGS := $(LDFLAGS) -r

export CFLAGS
export ASFLAGS
export CC AS LD AR OBJCOPY OBJDUMP
export LDFLAGS MODULE_LDFLAGS

# collection of all the command line arguments. defaults are defined in config.mk
# exported so they are available in subdirs
export V	# verbosity level (0, 1, 2)
export D 	# debug mode (0, 1)
export TOP_DIR

# Emulation tools
QEMU          := qemu-system-i386
BOCHS         := bochs

QEMU_FLAGS    := -drive file=$(DISK_IMG),format=raw,index=0,if=ide -drive file=$(FS_IMG),format=raw,index=1,if=ide -drive file=$(FLPY_IMG),format=raw,index=0,if=floppy
BOCHS_FLAGS   := -q -f .bochsrc

.PHONY: clean qemu bochs qemu-dbg all $(SYS_OBJ_DIRS) $(LIBS_DIRS) $(USER_DIRS) $(USER_PROGS) $(SYSTEM) $(BOOTSECTOR) test

all: $(DISK_IMG)

$(DISK_IMG): $(BOOTSECTOR) $(SYSTEM)
	$(Q) dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	$(TRACE_DD)
	$(Q) dd if=$(BOOTSECTOR) of=$@ bs=512 count=2 seek=0 conv=notrunc status=none
ifeq ($(UNAME_S),Darwin)
	$(Q) dd if=$(SYSTEM) of=$@ bs=512 count=$$(($(shell stat -f%z $(SYSTEM))/512)) seek=2 conv=notrunc status=none
else
	$(Q) dd if=$(SYSTEM) of=$@ bs=512 count=$$(($(shell stat --printf="%s" $(SYSTEM))/512)) seek=2 conv=notrunc status=none
endif

$(BOOTSECTOR):
	$(Q) $(MAKE) -s -C $(BOOTSECTOR_DIR)

$(SYSTEM): $(SYS_OBJS) $(LIBK)
	$(TRACE_LD)
	$(Q) $(LD) $(KERNEL_LDFLAGS) -o $@ $^

# build the kernel objects
$(SYS_OBJS): $(SYS_OBJ_DIRS)

$(SYS_OBJ_DIRS):
	$(Q) for dir in $@; do $(MAKE) -s -C $$dir; done

# build the required libraries
$(LIBC): $(LIBS_DIRS)
$(LIBK): $(LIBS_DIRS)

$(LIBS_DIRS):
	$(Q) for dir in $@; do $(MAKE) -s -C $$dir; done

# a separate floppy disk image (a simple FAT12 filesystem will be added to it)
$(FLPY_IMG): $(USER_PROGS)
	$(TRACE_DD)
	$(Q) dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	$(Q) mkfs.fat -F 12 -n "FLPY" $@
	$(Q) mcopy -i $@ $(USER_PROGS) ::/

# build user-space programs
$(USER_PROGS): $(USER_DIRS)
	$(Q) $(MAKE) -s -C $(USER_DIRS)

# a separate filesystem disk image
$(FS_IMG):
	$(TRACE_DD)
	$(Q) dd if=/dev/zero of=$@ bs=512 count=2880 status=none

# run the system in an emulator

qemu: $(DISK_IMG) $(FLPY_IMG) $(FS_IMG)
	$(QEMU) $(QEMU_FLAGS) -chardev file,id=serial0_file,path=qemu-serial.log -serial chardev:serial0_file

bochs: $(DISK_IMG) $(FLPY_IMG) $(FS_IMG)
	$(BOCHS) $(BOCHS_FLAGS)

# run the system in qemu with gdb debugging enabled
qemu-dbg: $(DISK_IMG) $(FLPY_IMG)
	$(QEMU) $(QEMU_FLAGS) \
		-gdb tcp::26000 -S &
	$(GDB) $(SYSTEM) \
		-ex 'set confirm off' \
		-ex 'add-symbol-file $(BOOTSECTOR_ELF)' \
        -ex 'target remote :26000' \
        -ex 'set disassembly-flavor att' \
        -ex 'layout split' \
        -ex 'break kmain' \
        -ex 'continue'

# testing target

test: CFLAGS += -DTESTING
test: clean all $(FLPY_IMG) $(FS_IMG)
	$(Q) $(QEMU) $(QEMU_FLAGS) \
	-monitor tcp:127.0.0.1:4444,server,nowait \
	-serial tcp:127.0.0.1:5555,server,nowait

# Clean everything for a fresh rebuild
clean:
	rm -f $(DISK_IMG) $(FLPY_IMG) $(FS_IMG)
	rm -f $(SYSTEM) $(SYSTEM).map
	$(Q) for dir in $(BOOTSECTOR_DIR) $(SYS_OBJ_DIRS) $(LIBS_DIRS) $(TEST_DIR) $(USER_DIRS); do $(MAKE) -C $$dir clean; done
	rm -f *.log
