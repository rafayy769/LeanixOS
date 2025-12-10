#include <stdio.h>

#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024]; // A buffer to hold the formatted string
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);   

    // time to write the string to the terminal
    write(buffer, len);
    
    return len; // Return the number of characters written
}

int getline(char *buf, size_t size)
{
    if (buf == NULL || size == 0) {
        return -1; // Invalid parameters
    }

    read(buf, size - 1); // Read input from the terminal
    
    return 0; // Return success
}