#ifndef PANACKELTY_BUILTINS_H
#define PANACKELTY_BUILTINS_H

#include "vm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* One registry supplies arity, purity, and implementation selection. */

/* Internal execution routing; never encoded in bytecode. */
typedef enum {
    BUILTIN_ORDINARY,
    BUILTIN_HTTP_CONNECTION_CONTINUE,
    BUILTIN_HTTP_LISTEN_ORDINARY,
    BUILTIN_HTTP_LISTEN_CONTEXTUAL,
    BUILTIN_HTTP_CLOSE,
    BUILTIN_HTTP_DESTROY,
    BUILTIN_HTTP_CONNECTION_INVOKE,
    BUILTIN_HTTP_TRANSPORT_EXCHANGE,
    BUILTIN_CPU_RESOURCE_ASYNC,
    BUILTIN_HTTP_STREAM_OPEN,
    BUILTIN_HTTP_STREAM_READ,
    BUILTIN_HTTP_STREAM_CLOSE,
    BUILTIN_TCP_SERVE,
    BUILTIN_TCP_SERVICE,
    BUILTIN_TCP_SERVE_STREAM,
    BUILTIN_PROCESS_RUN,
    BUILTIN_PROCESS_RUN_ASYNC,
    BUILTIN_RUN_BYTECODE,
    BUILTIN_RUN_BYTECODE_ARGS,
    BUILTIN_CONSOLE_OUTPUT_ASYNC,
    BUILTIN_CONSOLE_INPUT_ASYNC,
    BUILTIN_HOST_SLEEP_ASYNC,
    BUILTIN_INSTANT_NOW_ASYNC,
    BUILTIN_ENVIRONMENT_LOOKUP_ASYNC,
    BUILTIN_HOST_ARGUMENTS_ASYNC,
    BUILTIN_HOST_EXIT_ASYNC,
    BUILTIN_HOST_STDOUT_ASYNC,
    BUILTIN_HOST_STDERR_ASYNC,
    BUILTIN_FS_READ_BENEATH_ASYNC,
    BUILTIN_FS_ROOT_IDENTITY_ASYNC,
    BUILTIN_FILESYSTEM_ASYNC,
    BUILTIN_TCP_SERVER_READ_CHUNK,
    BUILTIN_TCP_EXCHANGE,
    BUILTIN_ASYNC_FAKE_READ,
    BUILTIN_PROCESS_EXIT,
    BUILTIN_PRINT,
} BuiltinDispatch;

typedef struct Builtin {
    const char *name;
    uint8_t arity;
    bool pure;
    BuiltinDispatch dispatch;
    Value *(*call)(VM *vm, const char *name, Value **arguments);
} Builtin;

Value *builtin_call(VM *vm, const char *name, Value **arguments);
const Builtin *builtin(const char *name);
bool builtin_is_async(const Builtin *entry);
/* Mutates an already-owned resource shell without accessing the host. */
bool builtin_is_local_state(const Builtin *entry);
VMHttpConnection *http_live_connection(VM *vm, Value *value);
Function *http_connection_handler(VM *vm, Value **arguments, Value **call_arguments, VMCallable *callable);

#endif
