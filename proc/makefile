include $(TOP_DIR)/config.mk

C_SOURCES   = 
ASM_SOURCES = 

BUILD_DIR = build

C_OBJECTS   = elf.o process.o tss.o
ASM_OBJECTS = proc_utils.o

TARGET  = proc.o

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(C_OBJECTS) $(ASM_OBJECTS)
	$(TRACE_LD)
	$(Q) $(LD) $(MODULE_LDFLAGS) -Map=$(TARGET).map -o $@ $^

$(BUILD_DIR)/%.o: %.c
	$(TRACE_CC)
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	$(TRACE_AS)
	$(Q) $(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR):
	$(TRACE_MKDIR)
	$(Q) mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(TARGET).map