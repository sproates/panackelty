#ifdef NDEBUG
#undef NDEBUG
#endif
#include "fault_injection.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdio.h>

/* Fixed bookkeeping avoids recursively allocating from inside the allocator. */
static void *allocations[32768];
static size_t live;
static atomic_size_t attempts, fail_at;
static pthread_mutex_t allocation_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_bool triggered;
static _Atomic(const char *) syscall_name;
static atomic_size_t syscall_at, syscall_seen;
static atomic_int syscall_error;
static atomic_bool descriptors[4096];
static atomic_size_t descriptor_count, child_count;
static atomic_bool pause_reads, read_paused;

static pthread_once_t fork_lock_once = PTHREAD_ONCE_INIT;

static void lock_for_fork(void)
{
    pthread_mutex_lock(&allocation_lock);
}

static void unlock_after_fork(void)
{
    pthread_mutex_unlock(&allocation_lock);
}

static void register_fork_lock(void)
{
    assert(pthread_atfork(lock_for_fork, unlock_after_fork, unlock_after_fork) == 0);
}

/* Change injection schedules only after workers have quiesced. Bookkeeping
 * remains safe while an owner and filesystem workers allocate concurrently. */
void fault_reset(size_t allocation)
{
    pthread_once(&fork_lock_once, register_fork_lock);
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
    pthread_mutex_lock(&allocation_lock);
    size_t count = live;
    pthread_mutex_unlock(&allocation_lock);
    return count;
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
    pthread_mutex_lock(&allocation_lock);
    void *result = size && allocation_fails() ? NULL : record(malloc(size));
    pthread_mutex_unlock(&allocation_lock);
    return result;
}

void *fault_calloc(size_t count, size_t size)
{
    pthread_mutex_lock(&allocation_lock);
    void *result = count && size && allocation_fails() ? NULL : record(calloc(count, size));
    pthread_mutex_unlock(&allocation_lock);
    return result;
}

void *fault_realloc(void *pointer, size_t size)
{
    pthread_mutex_lock(&allocation_lock);
    if (allocation_fails()) {
        pthread_mutex_unlock(&allocation_lock);
        return NULL;
    }
    if (!pointer) {
        void *result = record(realloc(NULL, size));
        pthread_mutex_unlock(&allocation_lock);
        return result;
    }
    assert(size != 0);
    size_t index = slot(pointer);
    void *next = realloc(pointer, size);
    if (next) {
        allocations[index] = next;
    }
    pthread_mutex_unlock(&allocation_lock);
    return next;
}

void fault_free(void *pointer)
{
    pthread_mutex_lock(&allocation_lock);
    if (pointer) {
        allocations[slot(pointer)] = NULL;
        live--;
    }
    free(pointer);
    pthread_mutex_unlock(&allocation_lock);
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
            assert(fds[i] >= 0 && (size_t)fds[i] < sizeof(descriptors) / sizeof(descriptors[0]));
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

void fault_pause_reads(bool pause)
{
    atomic_store(&pause_reads, pause);
}

bool fault_read_paused(void)
{
    return atomic_load(&read_paused);
}

ssize_t fault_read(int fd, void *buffer, size_t count)
{
    if (atomic_load(&pause_reads)) {
        atomic_store(&read_paused, true);
        struct timespec delay = {.tv_nsec = 1000000};
        while (atomic_load(&pause_reads)) {
            nanosleep(&delay, NULL);
        }
        atomic_store(&read_paused, false);
    }
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
    bool tracked = fd >= 0 && (size_t)fd < sizeof(descriptors) / sizeof(descriptors[0]) &&
                   atomic_exchange(&descriptors[fd], false);
    int result = close(fd);
    if (tracked) {
        if (!result) {
            descriptor_count--;
        } else {
            descriptors[fd] = true;
        }
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
        assert((size_t)fd < sizeof(descriptors) / sizeof(descriptors[0]) && !descriptors[fd]);
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

int fault_socket(int domain, int type, int protocol)
{
    if (syscall_fails("socket")) return -1;
    int fd = socket(domain, type, protocol);
    if (fd >= 0) {
        assert((size_t)fd < sizeof(descriptors) / sizeof(descriptors[0]) && !descriptors[fd]);
        descriptors[fd] = true;
        descriptor_count++;
    }
    return fd;
}

int fault_accept(int fd, struct sockaddr *address, socklen_t *length)
{
    if (syscall_fails("accept")) {
        return -1;
    }
    int peer = accept(fd, address, length);
    if (peer >= 0) {
        assert((size_t)peer < sizeof(descriptors) / sizeof(descriptors[0]) && !descriptors[peer]);
        descriptors[peer] = true;
        descriptor_count++;
    }
    return peer;
}

int fault_bind(int fd, const struct sockaddr *address, socklen_t length)
{
    return syscall_fails("bind") ? -1 : bind(fd, address, length);
}

int fault_listen(int fd, int backlog)
{
    return syscall_fails("listen") ? -1 : listen(fd, backlog);
}

ssize_t fault_recv(int fd, void *data, size_t size, int flags)
{
    return syscall_fails("recv") ? -1 : recv(fd, data, size, flags);
}

ssize_t fault_send(int fd, const void *data, size_t size, int flags)
{
    return syscall_fails("send") ? -1 : send(fd, data, size, flags);
}

int fault_pthread_create(pthread_t *thread, const pthread_attr_t *attributes,
                         void *(*start)(void *), void *argument)
{
    return syscall_fails("pthread_create") ? errno
                                           : pthread_create(thread, attributes, start, argument);
}
