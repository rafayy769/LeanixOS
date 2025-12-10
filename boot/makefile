include $(TOP_DIR)/config.mk

BUILD_DIR 	= build

C_SOURCES     = boot.S
ASM_SOURCES   = boot_print.s protected_start.s mmap.s memsize.s stage2.s

ASM_OBJECTS  = $(ASM_SOURCES:%.s=$(BUILD_DIR)/%.o)
TARGET_ELF   = $(BUILD_DIR)/bootloader.elf
TARGET       = bootloader.bin
LDFLAGS     := $(LDFLAGS) -T stage2.lds

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(TARGET_ELF)
	$(TRACE_OBJCOPY)
	$(Q) $(OBJCOPY) -O binary $< $@

$(TARGET_ELF): $(BUILD_DIR)/boot.o $(ASM_OBJECTS)
	$(TRACE_LD)
	$(Q) $(LD) $(LDFLAGS) -Map=$(TARGET_ELF).map -o $@ $^

$(BUILD_DIR)/%.e.o: $(BUILD_DIR)/%.e
	$(TRACE_AS)
	$(Q) $(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.S
	$(TRACE_CC)
	$(Q) $(CC) -m16 -g -gdwarf-4 -ggdb3 -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	$(TRACE_AS)
	$(Q) $(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR):
	$(TRACE_MKDIR)
	$(Q) mkdir -p $(BUILD_DIR)
	
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(TARGET_ELF).map