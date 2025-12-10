include $(TOP_DIR)/config.mk

SOURCES = \
	unitstd.c \
	string.c \
	vsprintf.c

KERNEL_SOURCES = \
	list.c stdio.c

USER_SOURCES = \
	stdio.c

BUILD_DIR 		 = build
KERNEL_BUILD_DIR = $(BUILD_DIR)/kernel
USER_BUILD_DIR   = $(BUILD_DIR)/user

OBJECTS=$(SOURCES:%.c=$(BUILD_DIR)/%.o)
KOBJECTS=$(KERNEL_SOURCES:%.c=$(KERNEL_BUILD_DIR)/%.o)
UOBJECTS=$(USER_SOURCES:%.c=$(USER_BUILD_DIR)/%.o)

# libc by default is user space, but a kernel libc is also built
TARGET=libc.a
KTARGET=libk.a

all: $(BUILD_DIR) $(TARGET) $(KTARGET)

$(TARGET): $(OBJECTS) $(UOBJECTS)
	$(TRACE_AR)
	$(Q) $(AR) rcs $@ $^

$(KTARGET): $(OBJECTS) $(KOBJECTS)
	$(TRACE_AR)
	$(Q) $(AR) rcs $@ $^

$(BUILD_DIR)/%.o: %.c
	$(TRACE_CC)
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(USER_BUILD_DIR)/%.o: %.c
	$(TRACE_CC)
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BUILD_DIR)/%.o: kernel/%.c
	$(TRACE_CC)
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	$(TRACE_MKDIR)
	$(Q) mkdir -p $(BUILD_DIR)
	$(Q) mkdir -p $(USER_BUILD_DIR)
	$(Q) mkdir -p $(KERNEL_BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(KTARGET)