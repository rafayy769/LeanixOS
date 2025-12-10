#ifndef __LIBC_SYSCALL_H
#define __LIBC_SYSCALL_H

#include <stdint.h>

/* syscall numbers */
#define SYS_read    0
#define SYS_write   1
#define SYS_fork    2
#define SYS_exec    3
#define SYS_getpid  4
#define SYS_sleep   5
#define SYS_execve  6

#endif /* __LIBC_SYSCALL_H */