#include "driver/serial.h"
#include <stdio.h>

#include <stddef.h>
#include <stdarg.h>

/* this library directly calls the underlying terminal functions */
#include <init/tty.h>
#include <init/syscall.h>

int printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024]; // A buffer to hold the formatted string
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // time to write the string to the terminal
    terminal_writestring(buffer); // Write the formatted string to the terminal
#ifndef TESTING
    serial_puts(buffer);    // Also write to serial port for logging (only for non-testing builds)
#endif
    
    return len; // Return the number of characters written
}

int getlinek(char *buf, size_t size)
{
    if (buf == NULL || size == 0) {
        return -1; // Invalid parameters
    }

    terminal_read(buf, size - 1); // Read input from the terminal
    
    return 0; // Return success
}