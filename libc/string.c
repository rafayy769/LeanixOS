#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static char* next_token = NULL;

uint32_t strlen(const char* str) 
{
	uint32_t len = 0;
	while (str[len])
		len++;
	return len;
}

void* memset(void* dest, int c, uint32_t n) 
{
	uint8_t* ptr = (uint8_t*)dest;
	for (uint32_t i = 0; i < n; i++) 
	{
		ptr[i] = (uint8_t)c;
	}
	return dest;
}

int strcmp(const char* str1, const char* str2) 
{
	while (*str1 && (*str1 == *str2)) 
	{
		str1++;
		str2++;
	}
	return (int)(*(unsigned char*)str1 - *(unsigned char*)str2);
}

void* memcpy(void* dest, const void* src, uint32_t n) 
{
	uint8_t* d = (uint8_t*)dest;
	const uint8_t* s = (const uint8_t*)src;
	for (uint32_t i = 0; i < n; i++) 
	{
		d[i] = s[i];
	}
	return dest;
}

int32_t memcmp(const void* s1, const void* s2, uint32_t n)
{
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
    for (uint32_t i = 0; i < n; i++) 
    {
        if (p1[i] != p2[i]) 
        {
            return (int32_t)(p1[i] - p2[i]);
        }
    }
    return 0;
}

char* strcpy(char* dest, const char* src)
{
	char* original = dest;
	while (*src) 
	{
		*dest++ = *src++;
	}
	*dest = '\0'; // Null-terminate the destination string
	return original;
}

char* strncpy(char* dest, const char* src, uint32_t n)
{
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++) 
		dest[i] = src[i];
	for (; i < n; i++) 
		dest[i] = '\0';

	return dest;
}

char* strchr(const char* str, int c) {
	while (*str) {
		if (*str == (char)c) return (char*)str;
		str++;
	}
	return NULL;
}

char* strtok(char* str, const char* delim)
{
	if (str) 
	{
		next_token = str; // Start with the new string
	}

	if (!next_token) 
	{
		return NULL; // No more tokens
	}

	char* token_start = next_token;
	if (!*next_token) return NULL;

	token_start = next_token;

	while (*next_token && !strchr(delim, *next_token))
		next_token++;

	if (*next_token) {
		*next_token = '\0';
		next_token++;
	} else {
		next_token = NULL;
	}

	return token_start;
}

long strtol(const char* str, char** endptr, int base) {
    const char* s = str;
    long result = 0;
    bool negative = false;

    // Skip whitespace
    while (*s == ' ' || *s == '\t' || *s == '\n' ||
           *s == '\v' || *s == '\f' || *s == '\r') {
        s++;
    }

    // Handle sign
    if (*s == '-') {
        negative = true;
        s++;
    } else if (*s == '+') {
        s++;
    }

    // Detect base if not provided (base == 0)
    if (base == 0) {
        if (s[0] == '0') {
            if (s[1] == 'x' || s[1] == 'X') {
                base = 16;
                s += 2;
            } else {
                base = 8;
                s++;
            }
        } else {
            base = 10;
        }
    } else if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
    }

    // Convert digits
    while (*s) {
        int digit;
        if (*s >= '0' && *s <= '9') {
            digit = *s - '0';
        } else if (*s >= 'a' && *s <= 'z') {
            digit = *s - 'a' + 10;
        } else if (*s >= 'A' && *s <= 'Z') {
            digit = *s - 'A' + 10;
        } else {
            break;
        }

        if (digit >= base) {
            break;
        }

        result = result * base + digit;
        s++;
    }

    if (endptr) {
        *endptr = (char*)s;
    }

    return negative ? -result : result;
}

char *strtok_r(char *str, const char *delim, char **saveptr)
{
    char *token_start;
    
    /* Use saveptr if str is NULL (subsequent calls) */
    if (str == NULL) {
        str = *saveptr;
    }
    
    /* Handle NULL or empty string */
    if (str == NULL || *str == '\0') {
        *saveptr = NULL;
        return NULL;
    }
    
    /* Skip leading delimiters */
    while (*str != '\0') {
        const char *d = delim;
        int is_delim = 0;
        
        /* Check if current character is a delimiter */
        while (*d != '\0') {
            if (*str == *d) {
                is_delim = 1;
                break;
            }
            d++;
        }
        
        if (!is_delim) {
            break;
        }
        str++;
    }
    
    /* If we've reached the end, no more tokens */
    if (*str == '\0') {
        *saveptr = NULL;
        return NULL;
    }
    
    /* Mark the start of the token */
    token_start = str;
    
    /* Find the end of the token */
    while (*str != '\0') {
        const char *d = delim;
        int is_delim = 0;
        
        /* Check if current character is a delimiter */
        while (*d != '\0') {
            if (*str == *d) {
                is_delim = 1;
                break;
            }
            d++;
        }
        
        if (is_delim) {
            break;
        }
        str++;
    }
    
    /* If we found a delimiter, replace it with null terminator */
    if (*str != '\0') {
        *str = '\0';
        *saveptr = str + 1;
    } else {
        /* End of string reached */
        *saveptr = NULL;
    }
    
    return token_start;
}

char* strcat(char* dest, const char* src)
{
    char* d = dest;

    // Move `d` to the end of the current string in dest
    while (*d) {
        d++;
    }

    // Copy src to the end of dest
    while (*src) {
        *d++ = *src++;
    }

    // Null terminate the result
    *d = '\0';

    return dest;
}