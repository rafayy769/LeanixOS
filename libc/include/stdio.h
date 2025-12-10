#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdarg.h>

//! Writes formatted data to a string buffer.
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

//! printf function to print to the console
int printf(const char *fmt, ...);

//! kernel mode printf
int printk(const char *fmt, ...);

//! reads a line from standard input into the buffer `buf` of size `size`
int getline(char *buf, size_t size);

//! reads a line from standard input into the buffer `buf` of size `size` (kernel mode)
int getlinek(char *buf, size_t size);

#endif // _LIBC_STDIO_H