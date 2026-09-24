#ifndef PANACKELTY_TEST_FAULT_INJECTION_H
#define PANACKELTY_TEST_FAULT_INJECTION_H

/* Force-included only in test objects, never in the shipped VM. */
#include "platform.h"

#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void fault_reset(size_t allocation);
size_t fault_attempts(void);
size_t fault_live(void);
bool fault_triggered(void);
void fault_syscall(const char *name, size_t occurrence, int error);
size_t fault_descriptors(void);
size_t fault_children(void);
void *fault_malloc(size_t size);
void *fault_calloc(size_t count, size_t size);
void *fault_realloc(void *pointer, size_t size);
void fault_free(void *pointer);
char *fault_strdup(const char *text);
int fault_clock_gettime(clockid_t clock, struct timespec *value);
int fault_pipe(int descriptors[2]);
pid_t fault_fork(void);
int fault_poll(struct pollfd *fds, nfds_t count, int timeout);
ssize_t fault_read(int fd, void *buffer, size_t count);
ssize_t fault_write(int fd, const void *buffer, size_t count);
int fault_nanosleep(const struct timespec *requested, struct timespec *remaining);
int fault_close(int fd);
int fault_open(const char *path, int flags, ...);
int fault_fstat(int fd, struct stat *info);
int fault_ftruncate(int fd, off_t size);
pid_t fault_waitpid(pid_t child, int *status, int options);

#ifdef PANACK_TEST_INJECT
#define malloc fault_malloc
#define calloc fault_calloc
#define realloc fault_realloc
#define free fault_free
#define strdup fault_strdup
#define clock_gettime fault_clock_gettime
#define pipe fault_pipe
#define fork fault_fork
#define poll fault_poll
#define read fault_read
#define write fault_write
#define nanosleep fault_nanosleep
#define close fault_close
#define open fault_open
#define fstat fault_fstat
#define ftruncate fault_ftruncate
#define waitpid fault_waitpid
#endif
#endif
