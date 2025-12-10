include $(TOP_DIR)/config.mk

# find all C files in each subdirectory
SOURCES := $(shell find . -name "*.c")
OBJECTS := $(SOURCES:.c=.o)
OBJECTS += testmain.o

TARGET  = tests.o

CFLAGS += -I$(TOP_DIR)/tests

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(TRACE_LD)
	$(Q) $(LD) $(MODULE_LDFLAGS) -Map=$(TARGET).map -o $@ $^

%.o: %.c
	$(TRACE_CC)
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).map
