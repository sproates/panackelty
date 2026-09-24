#ifdef NDEBUG
#undef NDEBUG
#endif
#include "fault_injection.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

/* Fixed bookkeeping avoids recursively allocating from inside the allocator. */
static void *allocations[32768];
static size_t live, attempts, fail_at;
static bool triggered;
static const char *syscall_name;
static size_t syscall_at, syscall_seen;
static int syscall_error;
static bool descriptors[4096];
static size_t descriptor_count, child_count;

void fault_reset(size_t allocation)
{
    attempts = 0;
    fail_at = allocation;
    triggered = false;
    syscall_name = NULL;
    syscall_seen = 0;
}

size_t fault_attempts(void)
{
    return attempts;
}

size_t fault_live(void)
{
    return live;
}

bool fault_triggered(void)
{
    return triggered;
}

size_t fault_descriptors(void)
{
    return descriptor_count;
}

size_t fault_children(void)
{
    return child_count;
}

static bool allocation_fails(void)
{
    if (++attempts != fail_at) {
        return false;
    }
    triggered = true;
    errno = ENOMEM;
    return true;
}

static size_t slot(void *pointer)
{
    for (size_t i = 0; i < sizeof(allocations) / sizeof(*allocations); i++) {
        if (allocations[i] == pointer) {
            return i;
        }
    }
    fprintf(stderr, "untracked pointer or exhausted allocation bookkeeping\n");
    abort();
}

static void *record(void *pointer)
{
    if (pointer) {
        allocations[slot(NULL)] = pointer;
        live++;
    }
    return pointer;
}

void *fault_malloc(size_t size)
{
    return size && allocation_fails() ? NULL : record(malloc(size));
}

void *fault_calloc(size_t count, size_t size)
{
    return count && size && allocation_fails() ? NULL : record(calloc(count, size));
}

void *fault_realloc(void *pointer, size_t size)
{
    if (allocation_fails()) {
        return NULL;
    }
    if (!pointer) {
        return record(realloc(NULL, size));
    }
    assert(size != 0);
    size_t index = slot(pointer);
    void *next = realloc(pointer, size);
    if (next) {
        allocations[index] = next;
    }
    return next;
}

void fault_free(void *pointer)
{
    if (pointer) {
        allocations[slot(pointer)] = NULL;
        live--;
    }
    free(pointer);
}

char *fault_strdup(const char *text)
{
    size_t length = strlen(text) + 1;
    char *copy = fault_malloc(length);
    if (copy) {
        memcpy(copy, text, length);
    }
    return copy;
}

void fault_syscall(const char *name, size_t occurrence, int error)
{
    syscall_name = name;
    syscall_at = occurrence;
    syscall_seen = 0;
    syscall_error = error;
}

static bool syscall_fails(const char *name)
{
    if (!syscall_name || strcmp(syscall_name, name) || ++syscall_seen != syscall_at) {
        return false;
    }
    triggered = true;
    errno = syscall_error;
    return true;
}

int fault_clock_gettime(clockid_t clock, struct timespec *value)
{
    return syscall_fails("clock_gettime") ? -1 : clock_gettime(clock, value);
}

int fault_pipe(int fds[2])
{
    if (syscall_fails("pipe")) {
        return -1;
    }
    int result = pipe(fds);
    if (!result) {
        for (size_t i = 0; i < 2; i++) {
            assert(fds[i] >= 0 && (size_t)fds[i] < sizeof(descriptors));
            assert(!descriptors[fds[i]]);
            descriptors[fds[i]] = true;
            descriptor_count++;
        }
    }
    return result;
}

pid_t fault_fork(void)
{
    if (syscall_fails("fork")) {
        return -1;
    }
    pid_t child = fork();
    if (child > 0) {
        child_count++;
    }
    return child;
}

int fault_poll(struct pollfd *fds, nfds_t count, int timeout)
{
    return syscall_fails("poll") ? -1 : poll(fds, count, timeout);
}

ssize_t fault_read(int fd, void *buffer, size_t count)
{
    return syscall_fails("read") ? -1 : read(fd, buffer, count);
}

ssize_t fault_write(int fd, const void *buffer, size_t count)
{
    return syscall_fails("write") ? -1 : write(fd, buffer, count);
}

int fault_nanosleep(const struct timespec *requested, struct timespec *remaining)
{
    if (syscall_fails("nanosleep")) {
        if (remaining) {
            *remaining = *requested;
        }
        return -1;
    }
    return nanosleep(requested, remaining);
}

int fault_close(int fd)
{
    int result = close(fd);
    if (!result && fd >= 0 && (size_t)fd < sizeof(descriptors) && descriptors[fd]) {
        descriptors[fd] = false;
        descriptor_count--;
    }
    return result;
}

pid_t fault_waitpid(pid_t child, int *status, int options)
{
    pid_t result = waitpid(child, status, options);
    if (result > 0) {
        assert(child_count);
        child_count--;
    }
    return result;
}

int fault_open(const char *path, int flags, ...)
{
    if (syscall_fails("open")) {
        return -1;
    }
    int fd;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        int mode = va_arg(args, int);
        va_end(args);
        fd = open(path, flags, (mode_t)mode);
    } else {
        fd = open(path, flags);
    }
    if (fd >= 0) {
        assert((size_t)fd < sizeof(descriptors) && !descriptors[fd]);
        descriptors[fd] = true;
        descriptor_count++;
    }
    return fd;
}

int fault_fstat(int fd, struct stat *info)
{
    return syscall_fails("fstat") ? -1 : fstat(fd, info);
}

int fault_ftruncate(int fd, off_t size)
{
    return syscall_fails("ftruncate") ? -1 : ftruncate(fd, size);
}
