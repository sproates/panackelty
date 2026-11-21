#include "platform.h"
#include "console_read.h"
#include "buffer.h"
#include "host_types.h"
#include "value.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef __wasi__
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#endif

#define INPUT_BYTES 65536
struct VMConsoleRead {
    size_t limit, count;
    int fd, native_error;
    const char *error;
    bool admitted, done, abandoned, accepted;
    uint8_t *buffer;
};

static Value *input_error(const char *code, int native)
{
    char *names[] = {"operation", "code", "native_code"};
    Value *fields[] = {value_data(V_STR, (const uint8_t *)"console_read", 12),
        value_data(V_STR, (const uint8_t *)code, strlen(code)), value_size((size_t)native)};
    Value *record = fields[0] && fields[1] && fields[2] ? named_value(V_RECORD, "HostError", names, fields, 3) : NULL;
    for (size_t i = 0; i < 3; i++) release(fields[i]);
    return host_variant("Error", record);
}

#ifndef __wasi__
/* Stdin is process-owned, not invocation-owned. Keep consumed bytes until an
 * execution acknowledges delivery, including cancellation and allocation failure. */
static pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t input_changed = PTHREAD_COND_INITIALIZER;
static VMConsoleRead *active;
static uint8_t pending[INPUT_BYTES];
static size_t pending_count;
static bool pending_eof;
static int pending_error;

static void free_read(VMConsoleRead *operation)
{
    if (operation->fd >= 0) close(operation->fd);
    free(operation->buffer);
    free(operation);
}

static void *input_worker(void *context)
{
    VMConsoleRead *operation = context;
    ssize_t count = -1;
    int error = 0;
    for (;;) {
        pthread_mutex_lock(&input_mutex);
        bool cancelled = operation->abandoned;
        pthread_mutex_unlock(&input_mutex);
        if (cancelled) break;
        struct pollfd descriptor = {.fd = operation->fd, .events = POLLIN};
        int ready = poll(&descriptor, 1, 50);
        if (ready < 0) { if (errno == EINTR) continue; error = errno; break; }
        if (!ready) continue;
        pthread_mutex_lock(&input_mutex);
        cancelled = operation->abandoned;
        pthread_mutex_unlock(&input_mutex);
        if (cancelled) break;
        /* Some hosts report POLLNVAL for valid, non-pollable devices such as
         * /dev/null. Let read distinguish EOF from an invalid descriptor. The
         * worker owns this descriptor until read completes. */
        count = read(operation->fd, operation->buffer, operation->limit);
        if (count >= 0) break;
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) { error = errno; break; }
    }
    close(operation->fd);
    operation->fd = -1;
    pthread_mutex_lock(&input_mutex);
    if (count > 0) {
        memcpy(pending, operation->buffer, (size_t)count);
        pending_count = (size_t)count;
    } else if (count == 0) pending_eof = true;
    else if (error) pending_error = error;
    operation->done = true;
    bool abandoned = operation->abandoned;
    if (abandoned) active = NULL;
    pthread_cond_broadcast(&input_changed);
    pthread_mutex_unlock(&input_mutex);
    if (abandoned) free_read(operation);
    return NULL;
}
#endif

VMConsoleRead *console_read_start(VM *vm, const Value *limit)
{
    if (limit->kind != V_NAT) { vm->error = "VM trap: console read limit requires Nat"; return NULL; }
    VMConsoleRead *operation = calloc(1, sizeof(*operation));
    if (!operation) return NULL;
    operation->fd = -1;
    if (!value_index(limit, &operation->limit) || !operation->limit || operation->limit > INPUT_BYTES) {
        operation->error = "out_of_range"; operation->done = true; return operation;
    }
#ifdef __wasi__
    operation->error = "unsupported_host"; operation->done = true;
#else
    pthread_mutex_lock(&input_mutex);
    if (active) {
        operation->error = "busy"; operation->done = true;
        pthread_mutex_unlock(&input_mutex);
        return operation;
    }
    active = operation;
    operation->admitted = true;
    if (pending_count || pending_eof || pending_error) {
        operation->done = true;
        pthread_mutex_unlock(&input_mutex);
        return operation;
    }
    pthread_mutex_unlock(&input_mutex);
    operation->buffer = malloc(operation->limit);
    if (!operation->buffer) { operation->done = true; console_read_destroy(operation); return NULL; }
    operation->fd = fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, 0);
    if (operation->fd < 0) {
        operation->error = "io_error"; operation->native_error = errno; operation->done = true;
        return operation;
    }
    pthread_attr_t attributes;
    int error = pthread_attr_init(&attributes);
    if (!error) {
        error = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
        pthread_t worker;
        if (!error) error = pthread_create(&worker, &attributes, input_worker, operation);
        pthread_attr_destroy(&attributes);
    }
    if (error) { operation->error = "resource_limit"; operation->native_error = error; operation->done = true; }
#endif
    return operation;
}

bool console_read_poll(VMConsoleRead *operation)
{
#ifndef __wasi__
    pthread_mutex_lock(&input_mutex);
#endif
    bool done = operation->done;
#ifndef __wasi__
    pthread_mutex_unlock(&input_mutex);
#endif
    return done;
}

void console_read_wait(VMConsoleRead *operation)
{
#ifndef __wasi__
    pthread_mutex_lock(&input_mutex);
    while (!operation->done) pthread_cond_wait(&input_changed, &input_mutex);
    pthread_mutex_unlock(&input_mutex);
#else
    (void)operation;
#endif
}

Value *console_read_result(VMConsoleRead *operation)
{
    if (!console_read_poll(operation)) return NULL;
    if (operation->error) return input_error(operation->error, operation->native_error);
#ifndef __wasi__
    pthread_mutex_lock(&input_mutex);
    Value *result;
    if (pending_error) result = input_error("io_error", pending_error);
    else if (pending_count) {
        operation->count = pending_count < operation->limit ? pending_count : operation->limit;
        result = host_variant("Ok", host_variant("Some", value_data(V_BYTES, pending, operation->count)));
    } else result = host_variant("Ok", named_value(V_VARIANT, "None", NULL, NULL, 0));
    pthread_mutex_unlock(&input_mutex);
    return result;
#else
    return NULL;
#endif
}

void console_read_accept(VMConsoleRead *operation)
{
#ifndef __wasi__
    pthread_mutex_lock(&input_mutex);
    if (operation->admitted && operation->done && !operation->accepted && !operation->error) {
        if (operation->count) {
            pending_count -= operation->count;
            memmove(pending, pending + operation->count, pending_count);
        } else { pending_eof = false; pending_error = 0; }
        operation->accepted = true;
    }
    pthread_mutex_unlock(&input_mutex);
#else
    (void)operation;
#endif
}

void console_read_destroy(VMConsoleRead *operation)
{
    if (!operation) return;
#ifndef __wasi__
    pthread_mutex_lock(&input_mutex);
    bool done = operation->done;
    if (!done) operation->abandoned = true;
    else if (operation->admitted) active = NULL;
    pthread_mutex_unlock(&input_mutex);
    if (done) free_read(operation);
#else
    free(operation);
#endif
}

size_t console_read_active_count(void)
{
#ifndef __wasi__
    pthread_mutex_lock(&input_mutex);
    size_t count = active ? 1 : 0;
    pthread_mutex_unlock(&input_mutex);
    return count;
#else
    return 0;
#endif
}

Value *console_input_call(VM *vm, const char *name, Value **arguments)
{
    (void)name;
    VMConsoleRead *operation = console_read_start(vm, arguments[0]);
    if (!operation) return NULL;
    console_read_wait(operation);
    Value *result = console_read_result(operation);
    if (result) console_read_accept(operation);
    console_read_destroy(operation);
    return result;
}

Value *console_legacy_read_line(VM *vm)
{
    Buffer buffer = {0};
#ifdef __wasi__
    /* Preserve the legacy synchronous operation where native workers are not
     * available. Bounded console.read reports unsupported_host on this target. */
    (void)vm;
    int byte;
    while ((byte = fgetc(stdin)) != EOF && byte != '\n') {
        char character = (char)byte;
        if (byte != '\r' && !buffer_add(&buffer, &character, 1)) {
            free(buffer.data);
            return NULL;
        }
    }
    Value *result = value_data(V_STR, (uint8_t *)(buffer.data ? buffer.data : ""), buffer.length);
    free(buffer.data);
    return result;
#else
    Value *limit = value_size(INPUT_BYTES);
    if (!limit) return NULL;
    bool complete = false, failed = false;
    while (!complete && !failed) {
        VMConsoleRead *operation = console_read_start(vm, limit);
        if (!operation) { failed = true; break; }
        console_read_wait(operation);
        Value *result = console_read_result(operation);
        if (!result) failed = true;
        else if (!strcmp(result->as.named.name, "Error")) {
            Value *error = result->as.named.values[0];
            if (!strcmp((char *)error->as.named.values[1]->as.bytes.data, "busy")) vm->error = "VM trap: stdin is already being read";
            complete = true;
        } else {
            Value *option = result->as.named.values[0];
            if (!strcmp(option->as.named.name, "None")) complete = true;
            else {
                Value *bytes = option->as.named.values[0];
                size_t used = 0;
                while (used < bytes->as.bytes.length && !complete && !failed) {
                    char byte = (char)bytes->as.bytes.data[used++];
                    if (byte == '\n') complete = true;
                    else if (byte != '\r' && !buffer_add(&buffer, &byte, 1)) failed = true;
                }
                operation->count = used;
            }
        }
        if (!failed && !vm->error) console_read_accept(operation);
        release(result);
        console_read_destroy(operation);
    }
    release(limit);
    Value *result = failed || vm->error ? NULL : value_data(V_STR, (uint8_t *)(buffer.data ? buffer.data : ""), buffer.length);
    free(buffer.data);
    return result;
#endif
}
