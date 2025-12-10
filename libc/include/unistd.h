#ifndef __LIBC_UNISTD_H
#define __LIBC_UNISTD_H

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2

typedef int32_t pid_t;
typedef int32_t ssize_t;

/* Function declarations */
// ssize_t read(int fd, void *buf, size_t count);
// ssize_t write(int fd, const void *buf, size_t count);
void _exit(int status);
ssize_t read(char *buf, size_t count);
ssize_t write(const char *buf, size_t count);
int fork(void);
int exec(const char *filename);
pid_t getpid(void);
unsigned sleep(unsigned seconds);
int execve(const char *path, char *const argv[], char *const envp[]);


#endif /* __LIBC_UNISTD_H */
