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
#define FILE_BYTES (128u * 1024u * 1024u)
#define FILE_PENDING_BYTES (512u * 1024u * 1024u)

typedef struct {
    const char *name, *arguments;
    size_t output_limit;
} FilesystemOperation;

static const FilesystemOperation FILESYSTEM_OPERATIONS[] = {
    {"fs_read_beneath", "PPN", ROOT_READ_BYTES},
    {"fs_root_identity", "P", 0},
    {"fs_metadata", "P", 0},
    {"fs_read", "PN", FILE_BYTES},
    {"fs_write", "PB", 0},
    {"fs_list", "P", ROOT_READ_BYTES},
    {"fs_create_directory", "P", 0},
    {"fs_remove_file", "P", 0},
    {"fs_remove_directory", "P", 0},
    {"fs_temp_directory", "P", 0},
    {"fs_temp_file", "P", 0},
    {"fs_directories_overlap", "PP", 0}
};

struct VMRootRead {
    Value *arguments[3], *result;
    const char *error;
    bool done, abandoned, admitted;
    const FilesystemOperation *operation;
    size_t reserved_bytes;
#ifndef __wasi__
    pthread_mutex_t mutex;
    pthread_cond_t completed;
#endif
};

static Value *read_failure(const char *operation, const char *code)
{
    char *names[] = {"operation", "code", "native_code"};
    Value *fields[] = {
        value_data(V_STR, (const uint8_t *)operation, strlen(operation)),
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
static atomic_size_t pending_bytes;

static void free_read(VMRootRead *read)
{
    for (size_t i = 0; i < 3; i++) {
        release(read->arguments[i]);
    }
    release(read->result);
    pthread_cond_destroy(&read->completed);
    pthread_mutex_destroy(&read->mutex);
    bool admitted = read->admitted;
    size_t reserved_bytes = read->reserved_bytes;
    free(read);
    if (admitted) {
        atomic_fetch_sub(&pending_bytes, reserved_bytes);
        atomic_fetch_sub(&active_reads, 1);
    }
}

static void *read_worker(void *context)
{
    VMRootRead *read = context;
    VM isolated = {0};
    Value *result = host_capability_call(
        &isolated, read->operation->name, read->arguments);
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
    return filesystem_async_start(vm, identity_only ? "fs_root_identity" : "fs_read_beneath", arguments);
}

VMRootRead *filesystem_async_start(VM *vm, const char *name, Value **arguments)
{
    const FilesystemOperation *operation = NULL;
    for (size_t i = 0; i < sizeof(FILESYSTEM_OPERATIONS) / sizeof(*FILESYSTEM_OPERATIONS); i++) {
        if (!strcmp(name, FILESYSTEM_OPERATIONS[i].name)) operation = &FILESYSTEM_OPERATIONS[i];
    }
    if (!operation) {
        vm->error = "VM trap: unknown asynchronous filesystem operation";
        return NULL;
    }
    size_t argument_count = strlen(operation->arguments);
    size_t reservation = operation->output_limit;
    bool out_of_range = false;
    for (size_t i = 0; i < argument_count; i++) {
        Value *value = arguments[i];
        char kind = operation->arguments[i];
        if ((kind == 'P' && (value->kind != V_PATH ||
              memchr(value->as.bytes.data, 0, value->as.bytes.length))) ||
            (kind == 'N' && value->kind != V_NAT) || (kind == 'B' && value->kind != V_BYTES)) {
            vm->error = "VM trap: invalid asynchronous filesystem arguments";
            return NULL;
        }
        if (kind == 'P') out_of_range |= value->as.bytes.length > ROOT_READ_PATH;
        if (kind == 'B') {
            out_of_range |= value->as.bytes.length > FILE_BYTES;
            if (!out_of_range) reservation += value->as.bytes.length;
        }
        if (kind == 'N') {
            size_t limit = 0;
            out_of_range |= !value_index(value, &limit) || limit > operation->output_limit;
            if (!out_of_range) reservation = limit;
        }
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
    read->operation = operation;
    if (out_of_range) {
        read->result = read_failure(name, "out_of_range");
        read->done = true;
        return read;
    }
#ifdef __wasi__
    (void)reservation;
    read->result = read_failure(name, "unsupported_host");
    read->done = true;
#else
    unsigned count = atomic_load(&active_reads);
    while (count < ROOT_READ_WORKERS &&
           !atomic_compare_exchange_weak(&active_reads, &count, count + 1)) {
    }
    if (count == ROOT_READ_WORKERS) {
        read->result = read_failure(name, "resource_limit");
        read->done = true;
        return read;
    }
    read->admitted = true;
    size_t bytes = atomic_load(&pending_bytes);
    while (reservation <= FILE_PENDING_BYTES - bytes &&
           !atomic_compare_exchange_weak(&pending_bytes, &bytes, bytes + reservation)) {
    }
    if (reservation > FILE_PENDING_BYTES - bytes) {
        read->result = read_failure(name, "resource_limit");
        read->done = true;
        return read;
    }
    read->reserved_bytes = reservation;
    for (size_t i = 0; i < argument_count; i++) {
        Value *argument = arguments[i];
        if (operation->arguments[i] == 'N') {
            size_t number = 0;
            (void)value_index(argument, &number);
            read->arguments[i] = value_size(number);
        } else {
            read->arguments[i] = value_data(argument->kind, argument->as.bytes.data, argument->as.bytes.length);
        }
        if (!read->arguments[i]) {
            free_read(read);
            return NULL;
        }
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
        read->result = read_failure(name, "resource_limit");
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
