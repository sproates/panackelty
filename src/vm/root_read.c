#include "platform.h"

#include "host_capabilities.h"
#include "host_types.h"
#include "numeric.h"
#include "root_read.h"
#include "value.h"

#include <stdlib.h>
#include <string.h>
#ifndef __wasi__
#include <pthread.h>
#include <stdatomic.h>
#endif

#define ROOT_READ_WORKERS 32
#define ROOT_READ_BYTES (16u * 1024u * 1024u)
#define ROOT_READ_PATH 4096u

struct VMRootRead {
    Value *arguments[3], *result;
    const char *error;
    bool done, abandoned, admitted, identity_only;
#ifndef __wasi__
    pthread_mutex_t mutex;
    pthread_cond_t completed;
#endif
};

static Value *read_failure(bool identity_only, const char *code)
{
    char *names[] = {"operation", "code", "native_code"};
    Value *fields[] = {
        value_data(V_STR, (const uint8_t *)(identity_only ? "fs_root_identity" : "fs_read_beneath"),
                   identity_only ? 16 : 15),
        value_data(V_STR, (const uint8_t *)code, strlen(code)), value_size(0)};
    Value *record = fields[0] && fields[1] && fields[2]
                        ? named_value(V_RECORD, "HostError", names, fields, 3)
                        : NULL;
    for (size_t i = 0; i < 3; i++) {
        release(fields[i]);
    }
    return record ? host_variant("Error", record) : NULL;
}

#ifndef __wasi__
static atomic_uint active_reads;

static void free_read(VMRootRead *read)
{
    for (size_t i = 0; i < 3; i++) {
        release(read->arguments[i]);
    }
    release(read->result);
    pthread_cond_destroy(&read->completed);
    pthread_mutex_destroy(&read->mutex);
    bool admitted = read->admitted;
    free(read);
    if (admitted) {
        atomic_fetch_sub(&active_reads, 1);
    }
}

static void *read_worker(void *context)
{
    VMRootRead *read = context;
    VM isolated = {0};
    Value *result = host_capability_call(
        &isolated, read->identity_only ? "fs_root_identity" : "fs_read_beneath", read->arguments);
    pthread_mutex_lock(&read->mutex);
    read->result = result;
    read->error = isolated.error;
    read->done = true;
    bool abandoned = read->abandoned;
    pthread_cond_signal(&read->completed);
    pthread_mutex_unlock(&read->mutex);
    /* After publishing completion the owner may free the operation. */
    if (abandoned) {
        free_read(read);
    }
    return NULL;
}
#endif

VMRootRead *root_read_start(VM *vm, Value **arguments, bool identity_only)
{
    if (arguments[0]->kind != V_PATH ||
        memchr(arguments[0]->as.bytes.data, 0, arguments[0]->as.bytes.length) ||
        (!identity_only &&
         (arguments[1]->kind != V_PATH || arguments[2]->kind != V_NAT ||
          memchr(arguments[1]->as.bytes.data, 0, arguments[1]->as.bytes.length)))) {
        vm->error = "VM trap: invalid asynchronous filesystem arguments";
        return NULL;
    }
    VMRootRead *read = calloc(1, sizeof(*read));
    if (!read) {
        return NULL;
    }
#ifndef __wasi__
    if (pthread_mutex_init(&read->mutex, NULL)) {
        free(read);
        return NULL;
    }
    if (pthread_cond_init(&read->completed, NULL)) {
        pthread_mutex_destroy(&read->mutex);
        free(read);
        return NULL;
    }
#endif
    read->identity_only = identity_only;
    size_t limit = 0;
    if (arguments[0]->as.bytes.length > ROOT_READ_PATH ||
        (!identity_only && (arguments[1]->as.bytes.length > ROOT_READ_PATH ||
                            !value_index(arguments[2], &limit) || limit > ROOT_READ_BYTES))) {
        read->result = read_failure(identity_only, "out_of_range");
        read->done = true;
        return read;
    }
#ifdef __wasi__
    read->result = read_failure(identity_only, "unsupported_host");
    read->done = true;
#else
    unsigned count = atomic_load(&active_reads);
    while (count < ROOT_READ_WORKERS &&
           !atomic_compare_exchange_weak(&active_reads, &count, count + 1)) {
    }
    if (count == ROOT_READ_WORKERS) {
        read->result = read_failure(identity_only, "resource_limit");
        read->done = true;
        return read;
    }
    read->admitted = true;
    read->arguments[0] =
        value_data(V_PATH, arguments[0]->as.bytes.data, arguments[0]->as.bytes.length);
    read->arguments[1] = identity_only ? value_data(V_PATH, (const uint8_t *)"", 0)
                                       : value_data(V_PATH, arguments[1]->as.bytes.data,
                                                    arguments[1]->as.bytes.length);
    read->arguments[2] = value_size(limit);
    if (!read->arguments[0] || !read->arguments[1] || !read->arguments[2]) {
        free_read(read);
        return NULL;
    }
    pthread_attr_t attributes;
    if (pthread_attr_init(&attributes)) {
        free_read(read);
        return NULL;
    }
    int error = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
    pthread_t worker;
    if (!error) {
        error = pthread_create(&worker, &attributes, read_worker, read);
    }
    pthread_attr_destroy(&attributes);
    if (error) {
        read->result = read_failure(identity_only, "resource_limit");
        read->done = true;
    }
#endif
    return read;
}

bool root_read_poll(VMRootRead *read)
{
#ifndef __wasi__
    pthread_mutex_lock(&read->mutex);
#endif
    bool done = read->done;
#ifndef __wasi__
    pthread_mutex_unlock(&read->mutex);
#endif
    return done;
}

void root_read_wait(VMRootRead *read)
{
#ifndef __wasi__
    pthread_mutex_lock(&read->mutex);
    while (!read->done) {
        pthread_cond_wait(&read->completed, &read->mutex);
    }
    pthread_mutex_unlock(&read->mutex);
#else
    (void)read;
#endif
}

Value *root_read_result(VMRootRead *read)
{
    return read->result;
}

const char *root_read_error(VMRootRead *read)
{
    return read->error;
}

void root_read_destroy(VMRootRead *read)
{
    if (!read) {
        return;
    }
#ifndef __wasi__
    pthread_mutex_lock(&read->mutex);
    read->abandoned = true;
    bool done = read->done;
    pthread_mutex_unlock(&read->mutex);
    if (done) {
        free_read(read);
    }
#else
    release(read->result);
    free(read);
#endif
}

size_t root_read_active_count(void)
{
#ifndef __wasi__
    return atomic_load(&active_reads);
#else
    return 0;
#endif
}
