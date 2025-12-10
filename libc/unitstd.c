#include <unistd.h>
#include <stddef.h>

#include <sys/syscall.h>

//! Macros to define syscall functions
//! simply calls the interrupt handler with the syscall number in eax
//! and params in regs in the order: ebx, ecx, edx, esi, edi

#define _DEFN_SYSCALL_P0(fn, num) \
    ssize_t fn () {         \
        ssize_t ret;        \
        asm volatile (      \
            "int $0x80"     \
            : "=a" (ret)    \
            : "0" (num));   \
        return ret;         \
    }

#define _DEFN_SYSCALL_P1(fn, num, P1) \
    ssize_t fn (P1 p1) { 				\
        ssize_t ret; 					\
        asm volatile ( 					\
            "int $0x80" 				\
            : "=a" (ret) 				\
            : "0" (num), "b" ((int)p1));\
        return ret; 					\
    }

#define _DEFN_SYSCALL_P2(fn, num, P1, P2) \
    ssize_t fn (P1 p1, P2 p2) { 	\
        ssize_t ret; 				\
        asm volatile (				\
            "int $0x80"				\
            : "=a" (ret)			\
            : "0" (num), "b" ((int)p1), "c" ((int)p2)); \
        return ret; 			\
    }

#define _DEFN_SYSCALL_P3(fn, num, P1, P2, P3) \
    ssize_t fn (P1 p1, P2 p2, P3 p3) { 	\
        ssize_t ret; 					\
        asm volatile (					\
            "int $0x80"					\
            : "=a" (ret)				\
            : "0" (num), "b" ((int)p1), "c" ((int)p2), "d" ((int)p3)); \
        return ret; 				\
    }

#define _DEFN_SYSCALL_P4(fn, num, P1, P2, P3, P4) \
    ssize_t fn (P1 p1, P2 p2, P3 p3, P4 p4) { 			  \
        ssize_t ret; 						  			  \
        asm volatile (								  	  \
            "int $0x80"								  	  \
            : "=a" (ret)							  	  \
            : "0" (num), "b" ((int)p1), "c" ((int)p2),	  \
              "d" ((int)p3), "S" ((int)p4)); 		  	  \
        return ret; 								  	  \
    }

// TODO: syscallP5 remains to be defined

//! Define the syscall functions
_DEFN_SYSCALL_P2 ( write, SYS_write, const char*, size_t );
_DEFN_SYSCALL_P2 ( read,  SYS_read, char*, size_t );
_DEFN_SYSCALL_P0 ( fork,  SYS_fork );
_DEFN_SYSCALL_P1 ( exec,  SYS_exec, const char* );