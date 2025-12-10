include $(TOP_DIR)/config.mk

C_SOURCES   = 
ASM_SOURCES = 

BUILD_DIR = build

C_OBJECTS   = idt.o interrupts.o isr.o keyboard.o shell.o syscall.o timer.o tty.o vga.o hfs.o
ASM_OBJECTS = 

TARGET  = impl.o

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(ASM_OBJECTS)
	$(TRACE_LD)
	$(Q) $(LD) $(MODULE_LDFLAGS) -Map=$(TARGET).map -o $@ $^

clean:
	rm -f $(TARGET) $(TARGET).map

