#include "platform.h"

#include "console_write.h"
#include "host_types.h"
#include "numeric.h"
#include "value.h"

#include <stdlib.h>
#include <string.h>
#ifndef __wasi__
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <unistd.h>
#endif

#define CONSOLE_WORKERS 4
#define CONSOLE_BYTES 16384

struct VMConsoleWrite {
    uint8_t *bytes;
    size_t length;
    int fd;
    bool admitted, done, abandoned;
    Value *result;
    const char *operation_name;
#ifndef __wasi__
    pthread_mutex_t mutex;
    pthread_cond_t completed;
#endif
};

static Value *write_error(const char *operation, const char *code, int native)
{
    char *names[] = {"operation", "code", "native_code"};
    Value *fields[] = {value_data(V_STR, (const uint8_t *)operation, strlen(operation)),
                       value_data(V_STR, (const uint8_t *)code, strlen(code)),
                       value_size((size_t)native)};
    Value *problem = fields[0] && fields[1] && fields[2]
                         ? named_value(V_RECORD, "HostError", names, fields, 3)
                         : NULL;
    for (size_t i = 0; i < 3; i++) {
        release(fields[i]);
    }
    return problem ? host_variant("Error", problem) : NULL;
}

#ifndef __wasi__
static atomic_uint active_writes;

static void free_write(VMConsoleWrite *operation)
{
    if (operation->fd >= 0) {
        close(operation->fd);
    }
    free(operation->bytes);
    release(operation->result);
    pthread_cond_destroy(&operation->completed);
    pthread_mutex_destroy(&operation->mutex);
    bool admitted = operation->admitted;
    free(operation);
    if (admitted) {
        atomic_fetch_sub(&active_writes, 1);
    }
}

static void *write_worker(void *context)
{
    VMConsoleWrite *operation = context;
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGPIPE);
    int error = pthread_sigmask(SIG_BLOCK, &signals, NULL);
    size_t sent = 0;
    while (!error && sent < operation->length) {
        ssize_t count = write(operation->fd, operation->bytes + sent, operation->length - sent);
        if (count > 0) {
            sent += (size_t)count;
        } else if (!count) {
            error = EIO;
        } else if (errno != EINTR) {
            error = errno;
        }
    }
    /* Consume the worker's pending broken-pipe signal before the detached
     * thread exits; blocking it alone can leave it pending at thread teardown. */
    if (error == EPIPE) {
        sigset_t pending;
        if (!sigpending(&pending) && sigismember(&pending, SIGPIPE)) {
            int caught;
            sigwait(&signals, &caught);
        }
    }
    if (close(operation->fd) && !error) {
        error = errno;
    }
    operation->fd = -1;
    Value *result = error ? write_error(operation->operation_name, "io_error", error)
                          : host_variant("Ok", value_unit());
    pthread_mutex_lock(&operation->mutex);
    operation->result = result;
    operation->done = true;
    bool abandoned = operation->abandoned;
    pthread_cond_signal(&operation->completed);
    pthread_mutex_unlock(&operation->mutex);
    if (abandoned) {
        free_write(operation);
    }
    return NULL;
}
#endif

VMConsoleWrite *console_write_start(VM *vm, const Value *message, bool standard_error)
{
    return console_write_start_mode(vm, message, standard_error, true, true);
}

VMConsoleWrite *console_write_start_mode(VM *vm, const Value *message, bool standard_error,
                                         bool newline, bool legacy)
{
    if (message->kind != V_STR) {
        vm->error = "VM trap: console output requires Str";
        return NULL;
    }
    VMConsoleWrite *operation = calloc(1, sizeof(*operation));
    if (!operation) {
        return NULL;
    }
    operation->fd = -1;
    operation->operation_name = legacy ? (standard_error ? "host_stderr" : "host_stdout")
        : standard_error ? (newline ? "console_write_error_line" : "console_write_error")
                         : (newline ? "console_write_line" : "console_write");
#ifndef __wasi__
    if (pthread_mutex_init(&operation->mutex, NULL)) {
        free(operation);
        return NULL;
    }
    if (pthread_cond_init(&operation->completed, NULL)) {
        pthread_mutex_destroy(&operation->mutex);
        free(operation);
        return NULL;
    }
#endif
    if (message->as.bytes.length > CONSOLE_BYTES) {
        operation->result = write_error(operation->operation_name, "out_of_range", 0);
        operation->done = true;
        return operation;
    }
#ifdef __wasi__
    operation->result = write_error(operation->operation_name, "unsupported_host", 0);
    operation->done = true;
#else
    unsigned count = atomic_load(&active_writes);
    while (count < CONSOLE_WORKERS &&
           !atomic_compare_exchange_weak(&active_writes, &count, count + 1)) {
    }
    if (count == CONSOLE_WORKERS) {
        operation->result = write_error(operation->operation_name, "resource_limit", 0);
        operation->done = true;
        return operation;
    }
    operation->admitted = true;
    operation->length = message->as.bytes.length + (newline ? 1 : 0);
    operation->bytes = malloc(operation->length ? operation->length : 1);
    if (!operation->bytes) {
        free_write(operation);
        return NULL;
    }
    memcpy(operation->bytes, message->as.bytes.data, message->as.bytes.length);
    if (newline) operation->bytes[message->as.bytes.length] = '\n';
    operation->fd = fcntl(standard_error ? STDERR_FILENO : STDOUT_FILENO, F_DUPFD_CLOEXEC, 0);
    if (operation->fd < 0) {
        operation->result = write_error(operation->operation_name, "io_error", errno);
        operation->done = true;
        return operation;
    }
#ifdef F_SETNOSIGPIPE
    /* Darwin pipe writes can deliver SIGPIPE process-wide despite a worker's
     * signal mask. Suppress it on this owned descriptor without changing the
     * process-wide signal disposition. */
    if (fcntl(operation->fd, F_SETNOSIGPIPE, 1) < 0) {
        operation->result = write_error(operation->operation_name, "io_error", errno);
        operation->done = true;
        return operation;
    }
#endif
    pthread_attr_t attributes;
    if (pthread_attr_init(&attributes)) {
        free_write(operation);
        return NULL;
    }
    int error = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
    pthread_t worker;
    if (!error) {
        error = pthread_create(&worker, &attributes, write_worker, operation);
    }
    pthread_attr_destroy(&attributes);
    if (error) {
        operation->result = write_error(operation->operation_name, "resource_limit", error);
        operation->done = true;
    }
#endif
    return operation;
}

bool console_write_poll(VMConsoleWrite *operation)
{
#ifndef __wasi__
    pthread_mutex_lock(&operation->mutex);
#endif
    bool done = operation->done;
#ifndef __wasi__
    pthread_mutex_unlock(&operation->mutex);
#endif
    return done;
}

void console_write_wait(VMConsoleWrite *operation)
{
#ifndef __wasi__
    pthread_mutex_lock(&operation->mutex);
    while (!operation->done) {
        pthread_cond_wait(&operation->completed, &operation->mutex);
    }
    pthread_mutex_unlock(&operation->mutex);
#else
    (void)operation;
#endif
}

Value *console_write_result(VMConsoleWrite *operation)
{
    return operation->result;
}

void console_write_destroy(VMConsoleWrite *operation)
{
    if (!operation) {
        return;
    }
#ifndef __wasi__
    pthread_mutex_lock(&operation->mutex);
    operation->abandoned = true;
    bool done = operation->done;
    pthread_mutex_unlock(&operation->mutex);
    if (done) {
        free_write(operation);
    }
#else
    release(operation->result);
    free(operation);
#endif
}

size_t console_write_active_count(void)
{
#ifndef __wasi__
    return atomic_load(&active_writes);
#else
    return 0;
#endif
}

Value *console_output_call(VM *vm, const char *name, Value **arguments)
{
    (void)name;
    if (arguments[1]->kind != V_BOOL || arguments[2]->kind != V_BOOL) {
        vm->error = "VM trap: console output flags require Bool";
        return NULL;
    }
    VMConsoleWrite *operation = console_write_start_mode(vm, arguments[0],
        arguments[1]->as.boolean, arguments[2]->as.boolean, false);
    if (!operation) {
        if (!vm->error) vm->error = "native VM out of memory";
        return NULL;
    }
    console_write_wait(operation);
    Value *result = retain(console_write_result(operation));
    console_write_destroy(operation);
    if (!result) vm->error = "native VM out of memory";
    return result;
}
