#include "platform.h"
#include "process_async.h"
#include "host_capabilities.h"
#include "host_types.h"
#include "numeric.h"
#include <stdlib.h>
#include <string.h>
#ifndef __wasi__
#include <pthread.h>
#include <stdatomic.h>
#endif

#define PROCESS_WORKERS 4
#define PROCESS_COPY_LIMIT (32u * 1024u * 1024u)
#define PROCESS_IO_LIMIT (16u * 1024u * 1024u)

struct VMProcess {
    VM worker_vm;
    Value *arguments[7], *result;
    char *ticket;
    bool isolated, done, abandoned, admitted;
#ifndef __wasi__
    atomic_bool cancelled;
    pthread_mutex_t mutex;
    pthread_cond_t completed;
#endif
};

static Value *process_error(bool isolated, const char *code, int native)
{
    const char *operation = isolated ? "process_run_isolated" : "process_run";
    char *names[] = {"operation", "code", "native_code"};
    Value *fields[] = {value_data(V_STR, (const uint8_t *)operation, strlen(operation)),
                       value_data(V_STR, (const uint8_t *)code, strlen(code)), value_size((size_t)native)};
    Value *problem = fields[0] && fields[1] && fields[2]
        ? named_value(V_RECORD, "HostError", names, fields, 3) : NULL;
    for (size_t i = 0; i < 3; i++) release(fields[i]);
    return host_variant("Error", problem);
}

#ifndef __wasi__
static atomic_uint active_processes;
static pthread_mutex_t drain_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t drained = PTHREAD_COND_INITIALIZER;

static void free_process(VMProcess *process)
{
    for (size_t i = 0; i < 7; i++) release(process->arguments[i]);
    for (size_t i = 0; i < process->worker_vm.env_count; i++) free(process->worker_vm.environment[i]);
    free(process->worker_vm.environment);
    free(process->ticket);
    release(process->result);
    pthread_cond_destroy(&process->completed);
    pthread_mutex_destroy(&process->mutex);
    bool admitted = process->admitted;
    free(process);
    if (admitted) {
        pthread_mutex_lock(&drain_mutex);
        atomic_fetch_sub(&active_processes, 1);
        pthread_cond_broadcast(&drained);
        pthread_mutex_unlock(&drain_mutex);
    }
}

static Value *copy_operand(Value *value)
{
    if (value->kind == V_ARRAY) {
        Value *copy = value_new(V_ARRAY);
        if (!copy) return NULL;
        size_t count = value->as.sequence.count;
        copy->as.sequence.items = calloc(count ? count : 1, sizeof(Value *));
        if (!copy->as.sequence.items) { release(copy); return NULL; }
        for (size_t i = 0; i < count; i++) {
            Value *item = value->as.sequence.items[i];
            copy->as.sequence.items[i] = value_data(V_STR, item->as.bytes.data, item->as.bytes.length);
            if (!copy->as.sequence.items[i]) { release(copy); return NULL; }
            copy->as.sequence.count++;
        }
        return copy;
    }
    if (value->kind == V_NAT || value->kind == V_DURATION)
        return value_big(value->kind, &value->as.integer);
    return value_data(value->kind, value->as.bytes.data, value->as.bytes.length);
}

static void *process_worker(void *context)
{
    VMProcess *process = context;
    Value *result = host_process_run_cancelable(&process->worker_vm, process->arguments,
        process->isolated, &process->cancelled, process->ticket);
    pthread_mutex_lock(&process->mutex);
    process->result = result;
    process->done = true;
    bool abandoned = process->abandoned;
    pthread_cond_signal(&process->completed);
    pthread_mutex_unlock(&process->mutex);
    if (abandoned) free_process(process);
    return NULL;
}

static bool add_bytes(size_t *total, size_t amount)
{
    if (amount > PROCESS_COPY_LIMIT - *total) return false;
    *total += amount;
    return true;
}
#endif

VMProcess *process_async_start(VM *vm, Value **arguments, bool isolated)
{
    const ValueKind kinds[] = {V_PATH, V_ARRAY, V_BYTES, V_PATH, V_ARRAY, V_DURATION, V_NAT};
    for (size_t i = 0; i < 7; i++) {
        if (arguments[i]->kind != kinds[i]) { vm->error = "VM trap: invalid process operands"; return NULL; }
    }
    VMProcess *process = calloc(1, sizeof(*process));
    if (!process) return NULL;
    process->isolated = isolated;
    if (vm->coverage && !vm->coverage_run) vm_coverage_gap(vm->coverage);
#ifndef __wasi__
    atomic_init(&process->cancelled, false);
    if (pthread_mutex_init(&process->mutex, NULL)) { free(process); return NULL; }
    if (pthread_cond_init(&process->completed, NULL)) {
        pthread_mutex_destroy(&process->mutex); free(process); return NULL;
    }
    const char *problem = NULL;
    size_t timeout, limit, copied = 0;
    if (arguments[5]->as.integer.sign < 0) problem = "negative_duration";
    else if (!pn_big_fits_size(&arguments[5]->as.integer, &timeout) || timeout > UINT64_C(31536000000000000)) problem = "out_of_range";
    else if (!timeout) problem = "timeout";
    if (!pn_big_fits_size(&arguments[6]->as.integer, &limit) || limit > PROCESS_IO_LIMIT ||
        arguments[2]->as.bytes.length > PROCESS_IO_LIMIT || (!isolated && vm->env_count > 4096)) problem = "out_of_range";
    for (size_t i = 0; i < 7 && !problem; i++) {
        Value *value = arguments[i];
        if (value->kind == V_ARRAY) {
            if (value->as.sequence.count > 4096) { problem = "out_of_range"; break; }
            for (size_t j = 0; j < value->as.sequence.count; j++) {
                Value *item = value->as.sequence.items[j];
                if (item->kind != V_STR) { vm->error = "VM trap: process entries require Str"; free_process(process); return NULL; }
                if (!add_bytes(&copied, item->as.bytes.length)) { problem = "out_of_range"; break; }
            }
        } else if (value->kind == V_PATH || value->kind == V_BYTES) {
            if (!add_bytes(&copied, value->as.bytes.length)) problem = "out_of_range";
        }
    }
    for (size_t i = 0; !isolated && i < vm->env_count && !problem; i++)
        if (!add_bytes(&copied, strlen(vm->environment[i]))) problem = "out_of_range";
    if (problem) { process->result = process_error(isolated, problem, 0); process->done = true; return process; }
    unsigned count = atomic_load(&active_processes);
    while (count < PROCESS_WORKERS && !atomic_compare_exchange_weak(&active_processes, &count, count + 1)) {}
    if (count == PROCESS_WORKERS) {
        process->result = process_error(isolated, "resource_limit", 0); process->done = true; return process;
    }
    process->admitted = true;
    for (size_t i = 0; i < 7; i++) {
        process->arguments[i] = copy_operand(arguments[i]);
        if (!process->arguments[i]) { free_process(process); return NULL; }
    }
    if (!isolated) {
        process->worker_vm.environment = calloc(vm->env_count ? vm->env_count : 1, sizeof(char *));
        if (!process->worker_vm.environment) { free_process(process); return NULL; }
        for (size_t i = 0; i < vm->env_count; i++) {
            process->worker_vm.environment[i] = strdup(vm->environment[i]);
            if (!process->worker_vm.environment[i]) { free_process(process); return NULL; }
            process->worker_vm.env_count++;
        }
        if (vm->coverage_run) {
            process->ticket = vm_coverage_process_ticket(vm->coverage_run);
            if (!process->ticket) vm_coverage_gap(vm->coverage);
        }
    } else if (vm->coverage_run) vm_coverage_gap(vm->coverage);
    pthread_attr_t attributes;
    if (pthread_attr_init(&attributes)) { free_process(process); return NULL; }
    int error = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
    pthread_t worker;
    if (!error) error = pthread_create(&worker, &attributes, process_worker, process);
    pthread_attr_destroy(&attributes);
    if (error) { process->result = process_error(isolated, "resource_limit", error); process->done = true; }
#else
    process->result = process_error(isolated, "unsupported_host", 0); process->done = true;
#endif
    return process;
}

bool process_async_poll(VMProcess *process)
{
#ifndef __wasi__
    pthread_mutex_lock(&process->mutex);
#endif
    bool done = process->done;
#ifndef __wasi__
    pthread_mutex_unlock(&process->mutex);
#endif
    return done;
}
void process_async_wait(VMProcess *process)
{
#ifndef __wasi__
    pthread_mutex_lock(&process->mutex);
    while (!process->done) pthread_cond_wait(&process->completed, &process->mutex);
    pthread_mutex_unlock(&process->mutex);
#else
    (void)process;
#endif
}
Value *process_async_result(VMProcess *process) { return process->result; }
const char *process_async_error(VMProcess *process) { return process->worker_vm.error; }
void process_async_destroy(VMProcess *process)
{
    if (!process) return;
#ifndef __wasi__
    atomic_store(&process->cancelled, true);
    pthread_mutex_lock(&process->mutex);
    process->abandoned = true;
    bool done = process->done;
    pthread_mutex_unlock(&process->mutex);
    if (done) free_process(process);
#else
    release(process->result); free(process);
#endif
}
size_t process_async_active_count(void)
{
#ifndef __wasi__
    return atomic_load(&active_processes);
#else
    return 0;
#endif
}
void process_async_drain(void)
{
#ifndef __wasi__
    pthread_mutex_lock(&drain_mutex);
    while (atomic_load(&active_processes)) pthread_cond_wait(&drained, &drain_mutex);
    pthread_mutex_unlock(&drain_mutex);
#endif
}
