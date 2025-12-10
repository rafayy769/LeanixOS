#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H

#include <sys/cdefs.h>
#include <stdint.h>

//! Expects a string of characters (null-terminated), and returns the length 
//!   of the string.
uint32_t strlen(const char* str);

//! Compares two strings and returns 0 if they are equal, a positive value if
//! 	the first string is greater, and a negative value if the second string 
//! 	is greater.
int 	strcmp(const char* str1, const char* str2);

//! Compares up to n characters of two strings and returns 0 if they are equal,
//! 	a positive value if the first string is greater, and a negative value if
//! 	the second string is greater.
char* 	strcpy(char* dest, const char* src);

//! Copies up to n characters from the source string to the destination string.
char* 	strncpy(char* dest, const char* src, uint32_t n);

//! Tokenizes the input string `str` based on the given delimiter
char* 	strtok(char* str, const char* delim);

//! Finds the first occurrence of the character `c` in the string `str`
char* 	strchr(const char* str, int c);

//! Converts the string `str` to a long integer based on the specified `base`
long  	strtol(const char* str, char** endptr, int base);

//! Appends the source string `src` to the destination string `dest`
char* 	strcat(char* dest, const char* src);

//!Re-entrant version of strtok, tokenizes str in place.
char* 	strtok_r(char* str, const char* delim, char** saveptr);

//! Fills the first `n` bytes of the memory area pointed to by `dest` with
//!   the constant byte `c`
void* 	memset(void* dest, int c, uint32_t n);

//! Copies `n` bytes from memory area `src` to memory area `dest`
void* 	memcpy(void* dest, const void* src, uint32_t n);

//! Compares the first `n` bytes of memory areas `s1` and `s2`
int32_t memcmp(const void* s1, const void* s2, uint32_t n);

#endif // _LIBC_STRING_H