#include "platform.h"

#include "buffer.h"
#include "builtins.h"
#include "console_write.h"
#include "host.h"
#include "http_transport.h"
#include "http_runtime.h"
#include "filesystem_stage.h"
#include "gzip_stream.h"
#include "digest.h"
#include "http_state.h"
#include "numeric.h"
#include "program.h"
#include "render.h"
#include "root_read.h"
#include "tcp.h"
#include "tcp_server.h"
#include "utf8.h"
#include "value.h"
#include "verify.h"
#include "vm.h"

#include <stdlib.h>
#include <string.h>
#ifndef __wasi__
#include <signal.h>
#endif

/* Stack-machine execution. Frames own stack slots and local references. */

typedef struct {
    char *name;
    Value *value;
} Local;

typedef struct {
    Function *function;
    size_t coverage_function, coverage_pending_pc;
    bool coverage_pending;
    size_t pc, stack_count, stack_capacity, local_count, local_capacity;
    Value **stack;
    Local *locals;
} Frame;

struct VMExecution {
    VM *vm;
    Frame *frames;
    size_t frame_count, frame_capacity;
    VMExecutionStatus status;
    Value *result;
    size_t exit_status;
    bool running, embedded;
    VMHostCall host_call;
    VMServiceWait print_wait, read_wait;
    bool waiting_read, fake_fail, network_enabled, server_read_enabled;
    VMTcpExchange *tcp;
    VMHttpTransport *http;
    VMTcpServer *server;
    VMRootRead *root_read;
    VMConsoleWrite *console_write;
    bool console_enabled, yield_requested;
    Value *http_waiting;
#ifndef __wasi__
    bool owns_stop_signals;
    struct sigaction previous_interrupt, previous_terminate;
#endif
    bool filesystem_enabled;
    bool server_enabled;
    void *host_context;
};

/* Consumes the reference, including on allocation failure. */
static bool stack_push(Frame *frame, Value *v)
{
    if (frame->stack_count == frame->stack_capacity) {
        size_t cap = frame->stack_capacity ? frame->stack_capacity * 2 : 16;
        Value **next = realloc(frame->stack, cap * sizeof(Value *));
        if (!next) {
            release(v);
            return false;
        }
        frame->stack = next;
        frame->stack_capacity = cap;
    }
    frame->stack[frame->stack_count++] = v;
    return true;
}

/* Transfers the stack reference to the caller, or records an underflow trap. */
static Value *stack_pop(Frame *frame, VM *vm)
{
    if (!frame->stack_count) {
        vm->error = "VM trap: operand stack underflow";
        return NULL;
    }
    return frame->stack[--frame->stack_count];
}

/* Local lookup borrows the reference; callers retain it before pushing. */
static Value *local_get(Frame *frame, const char *name)
{
    for (size_t i = frame->local_count; i-- > 0;) {
        if (!strcmp(frame->locals[i].name, name)) {
            return frame->locals[i].value;
        }
    }
    return NULL;
}

/* Replacing a local releases its old reference. Consumes value even on failure. */
static bool local_put(Frame *frame, const char *name, Value *value)
{
    for (size_t i = 0; i < frame->local_count; i++) {
        if (!strcmp(frame->locals[i].name, name)) {
            release(frame->locals[i].value);
            frame->locals[i].value = value;
            return true;
        }
    }
    if (frame->local_count == frame->local_capacity) {
        size_t cap = frame->local_capacity ? frame->local_capacity * 2 : 16;
        Local *next = realloc(frame->locals, cap * sizeof(Local));
        if (!next) {
            release(value);
            return false;
        }
        frame->locals = next;
        frame->local_capacity = cap;
    }
    frame->locals[frame->local_count].name = copy_text(name);
    if (!frame->locals[frame->local_count].name) {
        release(value);
        return false;
    }
    frame->locals[frame->local_count++].value = value;
    return true;
}

static void frame_free(Frame *frame)
{
    for (size_t i = 0; i < frame->stack_count; i++) {
        release(frame->stack[i]);
    }
    for (size_t i = 0; i < frame->local_count; i++) {
        free(frame->locals[i].name);
        release(frame->locals[i].value);
    }
    free(frame->stack);
    free(frame->locals);
}

static Value *constant_value(const Constant *c)
{
    if (c->tag == 0) {
        return value_big(V_NAT, &c->number);
    }
    if (c->tag == 1) {
        return value_big(V_INT, &c->number);
    }
    if (c->tag == 2) {
        PnBigInt n;
        if (!pn_big_copy(&n, &c->number)) {
            return NULL;
        }
        Value *v = value_decimal(&n, c->exponent);
        pn_big_free(&n);
        if (v && c->boolean && pn_big_is_zero(&v->as.decimal.coefficient)) {
            v->as.decimal.negative_zero = true;
        }
        return v;
    }
    if (c->tag == 3) {
        return value_data(V_STR, (uint8_t *)c->text, c->text_length);
    }
    if (c->tag == 4) {
        return value_bool(c->boolean);
    }
    return value_void();
}

/* Borrow arguments; each successfully installed local owns a retained reference.
 * Register partial frames before initialization so every failure is destroyable.
 */
static bool frame_push(VMExecution *execution, Function *function, Value **arguments)
{
    if (!function || (function->param_count && !arguments)) {
        execution->vm->error = "VM trap: invalid function invocation";
        return false;
    }
    if (execution->frame_count == execution->frame_capacity) {
        size_t capacity = execution->frame_capacity ? execution->frame_capacity * 2 : 8;
        if (capacity < execution->frame_capacity || capacity > SIZE_MAX / sizeof(Frame)) {
            execution->vm->error = "native VM out of memory";
            return false;
        }
        Frame *frames = realloc(execution->frames, capacity * sizeof(Frame));
        if (!frames) {
            execution->vm->error = "native VM out of memory";
            return false;
        }
        execution->frames = frames;
        execution->frame_capacity = capacity;
    }
    Frame *frame = &execution->frames[execution->frame_count++];
    *frame = (Frame){.function = function};
    for (size_t i = 0; i < function->param_count; i++) {
        if (!arguments[i]) {
            execution->vm->error = "VM trap: invalid function argument";
            return false;
        }
        if (!local_put(frame, function->params[i], retain(arguments[i]))) {
            execution->vm->error = "native VM out of memory";
            return false;
        }
    }
    if (execution->vm->coverage) {
        frame->coverage_function = vm_coverage_function(execution->vm->coverage, function);
        vm_coverage_enter(execution->vm->coverage, frame->coverage_function);
    }
    return true;
}

static void coverage_complete_pending(VMExecution *execution)
{
    if (!execution->vm->coverage || !execution->frame_count) return;
    Frame *frame = &execution->frames[execution->frame_count - 1];
    if (frame->coverage_pending) {
        vm_coverage_finish(execution->vm->coverage, frame->coverage_function,
                           frame->coverage_pending_pc, false);
        frame->coverage_pending = false;
    }
}

static void frames_clear(VMExecution *execution)
{
    while (execution->frame_count) {
        frame_free(&execution->frames[--execution->frame_count]);
    }
}

#ifndef __wasi__
static volatile sig_atomic_t service_stop_requested;
static bool service_signal_owner;

static void request_service_stop(int signal_number)
{
    (void)signal_number;
    service_stop_requested = 1;
}

static bool install_stop_signals(VMExecution *execution)
{
    if (service_signal_owner) {
        return false;
    }
    struct sigaction action = {0};
    action.sa_handler = request_service_stop;
    sigemptyset(&action.sa_mask);
    service_stop_requested = 0;
    if (sigaction(SIGINT, &action, &execution->previous_interrupt)) {
        return false;
    }
    if (sigaction(SIGTERM, &action, &execution->previous_terminate)) {
        sigaction(SIGINT, &execution->previous_interrupt, NULL);
        return false;
    }
    service_signal_owner = true;
    execution->owns_stop_signals = true;
    return true;
}
#endif

static void restore_stop_signals(VMExecution *execution)
{
#ifndef __wasi__
    if (execution->owns_stop_signals) {
        sigaction(SIGINT, &execution->previous_interrupt, NULL);
        sigaction(SIGTERM, &execution->previous_terminate, NULL);
        execution->owns_stop_signals = false;
        service_signal_owner = false;
        service_stop_requested = 0;
    }
#else
    (void)execution;
#endif
}

static Function *execution_handler(VM *vm, const Value *callable)
{
    if (callable->kind != V_STR) {
        return NULL;
    }
    for (size_t i = 0; i < vm->program->count; i++) {
        Function *candidate = &vm->program->functions[i];
        if (!builtin(candidate->name) && strlen(candidate->name) == callable->as.bytes.length &&
            !memcmp(candidate->name, callable->as.bytes.data, callable->as.bytes.length)) {
            return candidate;
        }
    }
    return NULL;
}

static Value *service_call(VMExecution *execution, Value **arguments)
{
    VM *vm = execution->vm;
    if (execution->embedded && !execution->server_enabled) {
        vm->error = "VM trap: TCP service is unavailable in this execution";
        return NULL;
    }
    Value *record = arguments[4];
    const char *fields[] = {"concurrency", "request_limit", "response_limit", "client_timeout_ms",
                            "drain_ms"};
    size_t values[5], port;
    if (arguments[0]->kind != V_STR || arguments[1]->kind != V_NAT ||
        !value_index(arguments[1], &port) || record->kind != V_RECORD ||
        strcmp(record->as.named.name, "TcpServiceLimits") || record->as.named.count != 5) {
        vm->error = "VM trap: invalid TCP service arguments";
        return NULL;
    }
    for (size_t i = 0; i < 5; i++) {
        Value *value = record->as.named.values[i];
        if (strcmp(record->as.named.names[i], fields[i]) || value->kind != V_NAT ||
            !value_index(value, &values[i])) {
            vm->error = "VM trap: invalid TCP service limits";
            return NULL;
        }
    }
    Function *handler = execution_handler(vm, arguments[2]);
    Function *ready = execution_handler(vm, arguments[5]);
    if (!ready || !ready->is_async || ready->pure || ready->param_count != 1) {
        vm->error = "VM trap: invalid service readiness callback";
        return NULL;
    }
    VMTcpServiceLimits limits = {values[0], values[1], values[2], values[3], values[4]};
    execution->server =
        tcp_server_start_service(vm, handler, arguments[3], arguments[0], port, limits,
                                 !execution->embedded || execution->network_enabled);
    if (!execution->server) {
        vm->error = "native VM out of memory";
        return NULL;
    }
    if (!execution->embedded || execution->filesystem_enabled) {
        tcp_server_enable_filesystem(execution->server);
    }
    if (!execution->embedded || execution->console_enabled) {
        tcp_server_enable_console(execution->server);
    }
    if (!tcp_server_done(execution->server)) {
        char *names[] = {"address", "port"};
        Value *fields[] = {arguments[0], arguments[1]};
        Value *context = named_value(V_RECORD, "TcpServiceReady", names, fields, 2);
        bool configured = context && tcp_server_set_ready_handler(execution->server, ready, context,
                                                                  !execution->embedded ||
                                                                      execution->console_enabled);
        release(context);
#ifndef __wasi__
        if (configured && !execution->embedded) {
            configured = install_stop_signals(execution);
        }
#endif
        if (!configured) {
            tcp_server_destroy(execution->server);
            execution->server = NULL;
            vm->error = "VM trap: service readiness setup failed";
            return NULL;
        }
    }
    execution->status = VM_WAITING;
    return NULL;
}

/* Source types do not survive in v9; check the full limits schema and resolve
 * the actual named handler against this execution's verified program. */
static Value *server_call_mode(VMExecution *execution, Value **arguments, bool streaming)
{
    VM *vm = execution->vm;
    if (execution->embedded && !execution->server_enabled) {
        vm->error = "VM trap: TCP server is unavailable in this execution";
        return NULL;
    }
    const char *fields[] = {"clients",        "concurrency",       "request_limit",
                            "response_limit", "client_timeout_ms", "admission_timeout_ms",
                            "drain_ms"};
    size_t record_index = streaming ? 4 : 3;
    Value *record = arguments[record_index];
    size_t values[7], port;
    if (arguments[0]->kind != V_STR || arguments[1]->kind != V_NAT ||
        !value_index(arguments[1], &port) || record->kind != V_RECORD ||
        strcmp(record->as.named.name, "TcpServerLimits") || record->as.named.count != 7) {
        vm->error = "VM trap: invalid TCP server arguments";
        return NULL;
    }
    for (size_t i = 0; i < 7; i++) {
        Value *value = record->as.named.values[i];
        if (strcmp(record->as.named.names[i], fields[i]) || value->kind != V_NAT ||
            !value_index(value, &values[i])) {
            vm->error = "VM trap: invalid TCP server limits";
            return NULL;
        }
    }
    Function *handler = NULL;
    Value *callable = arguments[2];
    if (callable->kind == V_STR) {
        for (size_t i = 0; i < vm->program->count; i++) {
            Function *candidate = &vm->program->functions[i];
            if (!builtin(candidate->name) && strlen(candidate->name) == callable->as.bytes.length &&
                !memcmp(candidate->name, callable->as.bytes.data, callable->as.bytes.length)) {
                handler = candidate;
            }
        }
    }
    VMTcpServerLimits limits = {values[0], values[1], values[2], values[3],
                                values[4], values[5], values[6]};
    execution->server = streaming
                            ? tcp_server_start_stream(vm, handler, arguments[3], arguments[0], port,
                                                      limits,
                                                      !execution->embedded || execution->network_enabled)
                            : tcp_server_start(vm, handler, arguments[0], port, limits,
                                               !execution->embedded || execution->network_enabled);
    if (execution->server && (!execution->embedded || execution->filesystem_enabled)) {
        tcp_server_enable_filesystem(execution->server);
    }
    if (execution->server && (!execution->embedded || execution->console_enabled)) {
        tcp_server_enable_console(execution->server);
    }
    if (!execution->server) {
        vm->error = "native VM out of memory";
    } else {
        execution->status = VM_WAITING;
    }
    return NULL;
}

static Value *server_call(VMExecution *execution, Value **arguments)
{
    return server_call_mode(execution, arguments, false);
}

static Value *http_unit_result(void)
{
    Value *unit = value_new(V_UNIT);
    if (!unit) return NULL;
    Value *result = named_value(V_VARIANT, "Ok", NULL, &unit, 1);
    release(unit);
    return result;
}

static Function *http_worker_function(VM *vm, Value *name)
{
    if (name->kind != V_STR) return NULL;
    for (size_t i = 0; i < vm->program->count; i++) {
        Function *function = &vm->program->functions[i];
        if (strlen(function->name) == name->as.bytes.length &&
            !memcmp(function->name, name->as.bytes.data, name->as.bytes.length) &&
            function->is_async && !function->pure && function->param_count == 1) return function;
    }
    return NULL;
}

static Value *execution_builtin(VMExecution *execution, const Builtin *entry, Value **arguments,
                                bool *called_function)
{
    *called_function = false;
    if (!strcmp(entry->name, "http_connection_continue")) {
        if (tcp_server_http_continue(execution->vm, arguments[0])) {
            execution->waiting_read = true;
            execution->status = VM_WAITING;
        }
        return NULL;
    }
    if (!strcmp(entry->name, "http_listen_ordinary") || !strcmp(entry->name, "http_listen_contextual")) {
        VM *vm = execution->vm;
        Function *worker = http_worker_function(vm, arguments[1]);
        if (!worker) { vm->error = "VM trap: invalid HTTP worker"; return NULL; }
        if (execution->embedded && !execution->server_enabled) {
            vm->error = "VM trap: listener capability is unavailable";
            return NULL;
        }
        Value *result = http_runtime_listen(vm, arguments[0], worker,
            !strcmp(entry->name, "http_listen_contextual"),
            !execution->embedded || execution->network_enabled,
            !execution->embedded || execution->filesystem_enabled,
            !execution->embedded || execution->console_enabled);
#ifndef __wasi__
        if (http_runtime_active(vm->http_runtime) && !vm->http_runtime->owner->owns_stop_signals &&
            !install_stop_signals(vm->http_runtime->owner)) {
            release(result);
            vm->error = "HTTP signal setup failed";
            return NULL;
        }
#endif
        execution->yield_requested = true;
        return result;
    }
    if (!strcmp(entry->name, "http_close") || !strcmp(entry->name, "http_destroy")) {
        VM *vm = execution->vm;
        if (arguments[0]->kind != V_HTTP_STATE || !arguments[0]->as.http_state ||
            !http_runtime_request_close(vm->http_runtime, arguments[0], !strcmp(entry->name, "http_destroy"))) {
            vm->error = "VM trap: invalid HTTP shutdown handle";
            return NULL;
        }
        if (!arguments[0]->as.http_state->resource) return http_unit_result();
        execution->http_waiting = retain(arguments[0]);
        execution->status = VM_WAITING;
        return NULL;
    }
    if (!strcmp(entry->name, "http_connection_invoke")) {
        Value *call_arguments[3];
        Function *handler = http_connection_handler(execution->vm, arguments, call_arguments);
        if (handler) {
            *called_function = true;
            frame_push(execution, handler, call_arguments);
        }
        return NULL;
    }
    VM *vm = execution->vm;
    if (vm->coverage && !vm->coverage_run && (!strcmp(entry->name, "$http_transport_exchange") ||
        !strcmp(entry->name, "$tcp_serve") || !strcmp(entry->name, "tcp_service") ||
        !strcmp(entry->name, "$tcp_serve_stream") || !strcmp(entry->name, "process_run") ||
        !strcmp(entry->name, "run_bytecode") || !strcmp(entry->name, "run_bytecode_args"))) {
        vm_coverage_gap(vm->coverage);
    }
    if (!strcmp(entry->name, "tcp_service")) {
        return service_call(execution, arguments);
    }
    if (!strcmp(entry->name, "host_arguments_async")) {
        if (execution->embedded && !execution->console_enabled) {
            vm->error = "VM trap: invocation service is unavailable in this execution";
            return NULL;
        }
        return host_call(vm, "command_args", NULL);
    }
    if (!strcmp(entry->name, "host_exit_async")) {
        if (arguments[0]->kind != V_NAT || !value_index(arguments[0], &execution->exit_status) ||
            execution->exit_status > 255) {
            vm->error = "VM trap: invalid process status";
        } else if (execution->embedded || http_runtime_active(vm->http_runtime)) {
            execution->status = VM_EXITED;
        } else {
            return host_call(vm, "process_exit", arguments);
        }
        return NULL;
    }
    if (!strcmp(entry->name, "host_stdout_async") || !strcmp(entry->name, "host_stderr_async")) {
        if (execution->embedded && !execution->console_enabled) {
            vm->error = "VM trap: console service is unavailable in this execution";
        } else {
            execution->console_write =
                console_write_start(vm, arguments[0], !strcmp(entry->name, "host_stderr_async"));
            if (!execution->console_write && !vm->error) {
                vm->error = "native VM out of memory";
            }
            if (execution->console_write) {
                execution->status = VM_WAITING;
            }
        }
        return NULL;
    }
    if (!strcmp(entry->name, "fs_read_beneath_async") ||
        !strcmp(entry->name, "fs_root_identity_async")) {
        if (execution->embedded && !execution->filesystem_enabled) {
            vm->error = "VM trap: filesystem service is unavailable in this execution";
        } else {
            execution->root_read =
                root_read_start(vm, arguments, !strcmp(entry->name, "fs_root_identity_async"));
            if (!execution->root_read && !vm->error) {
                vm->error = "native VM out of memory";
            }
            if (execution->root_read) {
                execution->status = VM_WAITING;
            }
        }
        return NULL;
    }
    if (!strcmp(entry->name, "$tcp_serve")) {
        return server_call(execution, arguments);
    }
    if (!strcmp(entry->name, "$tcp_serve_stream")) {
        return server_call_mode(execution, arguments, true);
    }
    if (!strcmp(entry->name, "$tcp_server_read_chunk")) {
        size_t limit;
        if (arguments[0]->kind != V_NAT || !value_index(arguments[0], &limit) || !limit ||
            limit > 16384 || !execution->embedded || !execution->read_wait ||
            !execution->server_read_enabled) {
            vm->error = "VM trap: TCP server read is unavailable in this execution";
        } else if (execution->read_wait(execution->host_context, arguments[0], &vm->error)) {
            execution->waiting_read = true;
            execution->status = VM_WAITING;
        } else if (!vm->error) {
            vm->error = "VM trap: TCP server read registration failed";
        }
        return NULL;
    }
    if (!strcmp(entry->name, "$tcp_exchange")) {
        if (execution->embedded && !execution->network_enabled) {
            vm->error = "VM trap: TCP service is unavailable in this execution";
        } else {
            execution->tcp = tcp_exchange_start(arguments);
            if (!execution->tcp) vm->error = "native VM out of memory";
            else {
                execution->waiting_read = true;
                execution->status = VM_WAITING;
            }
        }
        return NULL;
    }
    if (!strcmp(entry->name, "$http_transport_exchange")) {
        if (execution->embedded && !execution->network_enabled) {
            vm->error = "VM trap: HTTP transport is unavailable in this execution";
        } else {
            execution->http = http_transport_start(arguments);
            if (!execution->http) vm->error = "native VM out of memory";
            else {
                execution->waiting_read = true;
                execution->status = VM_WAITING;
            }
        }
        return NULL;
    }
    if (!strcmp(entry->name, "async_fake_read")) {
        if (arguments[0]->kind != V_BOOL) {
            vm->error = "VM trap: fake read requires Bool";
        } else if (!execution->embedded ||
                   (execution->read_wait &&
                    execution->read_wait(execution->host_context, arguments[0], &vm->error))) {
            execution->waiting_read = true;
            execution->fake_fail = arguments[0]->as.boolean;
            execution->status = VM_WAITING;
        } else if (!vm->error) {
            vm->error = "VM trap: async service is unavailable";
        }
        return NULL;
    }
    if (!strcmp(entry->name, "process_exit") &&
        (execution->embedded || http_runtime_active(vm->http_runtime))) {
        if (arguments[0]->kind != V_NAT || !value_index(arguments[0], &execution->exit_status)) {
            vm->error = "VM trap: invalid process status";
        } else {
            execution->status = VM_EXITED;
        }
        return NULL;
    }
    if (!execution->embedded || entry->pure || builtin_is_local_state(entry)) {
        return builtin_call(vm, entry->name, arguments);
    }
    if (!strcmp(entry->name, "run_bytecode") || !strcmp(entry->name, "run_bytecode_args")) {
        vm->error = "VM trap: nested execution is unavailable in resumable mode";
        return NULL;
    }
    if (execution->print_wait && !strcmp(entry->name, "print")) {
        if (execution->print_wait(execution->host_context, arguments[0], &vm->error)) {
            execution->status = VM_WAITING;
        } else if (!vm->error) {
            vm->error = "VM trap: pending print registration failed";
        }
        return NULL;
    }
    if (!execution->host_call) {
        vm->error = "VM trap: host service is unavailable in resumable mode";
        return NULL;
    }
    return execution->host_call(execution->host_context, entry->name, arguments, entry->arity,
                                &vm->error);
}

VMExecutionStatus vm_execution_advance(VMExecution *execution, size_t budget)
{
    if (execution->running) {
        return VM_BUSY;
    }
    if (execution->status == VM_WAITING && execution->http_waiting &&
        !execution->http_waiting->as.http_state->resource) {
        Value *result = http_unit_result();
        if (!result || !stack_push(&execution->frames[execution->frame_count - 1], result)) {
            execution->vm->error = "native VM out of memory";
            execution->status = VM_TRAPPED;
            vm_coverage_terminal(execution->vm->coverage, 2);
            frames_clear(execution);
        } else {
            execution->status = VM_YIELDED;
            coverage_complete_pending(execution);
        }
        release(execution->http_waiting);
        execution->http_waiting = NULL;
    }
    if (execution->status != VM_YIELDED) {
        return execution->status;
    }
    execution->running = true;
    VM *vm = execution->vm;
    /* One unit is one dispatched instruction, including calls and returns.
     * Frame pointers are reacquired after any push that may grow the array.
     */
    while (budget && !vm->error && execution->status == VM_YIELDED) {
        Frame *frame = &execution->frames[execution->frame_count - 1];
        Function *function = frame->function;
        if (frame->pc >= function->ins_count) {
            vm->error = "VM trap: function did not return";
            break;
        }
        budget--;
        size_t coverage_pc = frame->pc, coverage_function = frame->coverage_function;
        size_t coverage_depth = execution->frame_count;
        bool coverage_branched = false;
        if (vm->coverage) vm_coverage_attempt(vm->coverage, coverage_function, coverage_pc);
        Instruction *instruction = &function->ins[frame->pc++];
        Value *a = NULL, *b = NULL, *v = NULL;
        size_t index, start, end;
        switch (instruction->op) {
        case OP_CONST:
            v = constant_value(&instruction->constant);
            if (!v || !stack_push(frame, v)) {
                goto oom;
            }
            break;
        case OP_LOAD:
            v = local_get(frame, instruction->name);
            if (!v) {
                vm->error = "VM trap: uninitialized local";
                break;
            }
            if (!stack_push(frame, retain(v))) {
                goto oom;
            }
            break;
        case OP_STORE:
            v = stack_pop(frame, vm);
            if (v && !local_put(frame, instruction->name, v)) {
                goto oom;
            }
            break;
        case OP_POP:
            v = stack_pop(frame, vm);
            release(v);
            break;
        case OP_UNARY:
            a = stack_pop(frame, vm);
            if (!a) {
                break;
            }
            if (instruction->code == 1) {
                if (a->kind != V_BOOL) {
                    vm->error = "VM trap: ! requires Bool";
                } else {
                    v = value_bool(!a->as.boolean);
                }
            } else if (a->kind == V_DEC) {
                PnBigInt n;
                if (pn_big_copy(&n, &a->as.decimal.coefficient)) {
                    n.sign = -n.sign;
                    v = value_decimal(&n, a->as.decimal.exponent);
                    pn_big_free(&n);
                }
            } else if (a->kind == V_RAT) {
                PnBigInt n = a->as.rational.numerator;
                n.sign = -n.sign;
                v = value_rat(&n, &a->as.rational.denominator, &vm->error);
            } else if (a->kind == V_NAT || a->kind == V_INT) {
                PnBigInt n;
                if (pn_big_copy(&n, &a->as.integer)) {
                    n.sign = -n.sign;
                    v = value_big(V_INT, &n);
                    pn_big_free(&n);
                }
            } else {
                vm->error = "VM trap: unary - requires numeric value";
            }
            release(a);
            if ((!v && !vm->error) || (v && !stack_push(frame, v))) {
                goto oom;
            }
            break;
        case OP_BINARY:
            b = stack_pop(frame, vm);
            a = stack_pop(frame, vm);
            if (!a || !b) {
                release(a);
                release(b);
                break;
            }
            v = binary_value_classified(instruction->code, a, b, &vm->error, &vm->fault_kind);
            release(a);
            release(b);
            if ((!v && !vm->error) || (v && !stack_push(frame, v))) {
                goto oom;
            }
            break;
        case OP_MAKE_RANGE:
            b = stack_pop(frame, vm);
            a = stack_pop(frame, vm);
            if (!a || !b) {
                release(a);
                release(b);
                break;
            }
            if (a->kind != V_NAT || b->kind != V_NAT) {
                vm->error = "VM trap: range bounds require Nat";
                release(a);
                release(b);
                break;
            }
            v = value_new(V_RANGE);
            if (!v || !pn_big_copy(&v->as.range.start, &a->as.integer) ||
                !pn_big_copy(&v->as.range.end, &b->as.integer)) {
                release(a);
                release(b);
                release(v);
                goto oom;
            }
            release(a);
            release(b);
            if (!stack_push(frame, v)) {
                goto oom;
            }
            break;
        case OP_MAKE_ARRAY: {
            Value **items = calloc(instruction->count, sizeof(Value *));
            if (instruction->count && !items) {
                goto oom;
            }
            for (size_t i = instruction->count; i-- > 0;) {
                items[i] = stack_pop(frame, vm);
            }
            if (vm->error) {
                for (size_t i = 0; i < instruction->count; i++) {
                    release(items[i]);
                }
                free(items);
                break;
            }
            v = value_sequence(V_ARRAY, items, instruction->count);
            for (size_t i = 0; i < instruction->count; i++) {
                release(items[i]);
            }
            free(items);
            if (!v || !stack_push(frame, v)) {
                goto oom;
            }
            break;
        }
        case OP_INDEX_GET:
            b = stack_pop(frame, vm);
            a = stack_pop(frame, vm);
            if (!a || !b) {
                release(a);
                release(b);
                break;
            }
            if (!value_index(b, &index)) {
                vm->error = "VM trap: invalid index";
                if (b->kind == V_NAT) {
                    vm->fault_kind = VM_FAULT_REQUEST;
                }
            } else if (a->kind == V_ARRAY) {
                if (index >= a->as.sequence.count) {
                    vm->error = "VM trap: index is out of bounds";
                    vm->fault_kind = VM_FAULT_REQUEST;
                } else {
                    v = retain(a->as.sequence.items[index]);
                }
            } else if (a->kind == V_BYTES) {
                if (index >= a->as.bytes.length) {
                    vm->error = "VM trap: index is out of bounds";
                    vm->fault_kind = VM_FAULT_REQUEST;
                } else {
                    v = value_size(a->as.bytes.data[index]);
                }
            } else if (a->kind == V_STR) {
                if (!utf8_offset(a, index, &start, &end)) {
                    vm->error = "VM trap: index is out of bounds";
                    vm->fault_kind = VM_FAULT_REQUEST;
                } else {
                    v = value_data(V_STR, a->as.bytes.data + start, end - start);
                }
            } else {
                vm->error = "VM trap: value is not indexable";
            }
            release(a);
            release(b);
            if ((!v && !vm->error) || (v && !stack_push(frame, v))) {
                goto oom;
            }
            break;
        case OP_INTERPOLATE: {
            size_t count = instruction->count ? instruction->count - 1 : 0;
            Value **values = calloc(count, sizeof(Value *));
            if (count && !values) {
                goto oom;
            }
            for (size_t i = count; i-- > 0;) {
                values[i] = stack_pop(frame, vm);
            }
            Buffer out = {0};
            bool rendered = true;
            for (size_t i = 0; i < instruction->count && !vm->error; i++) {
                if (!buffer_text(&out, instruction->items[i])) {
                    rendered = false;
                    break;
                }
                if (i < count && !render(&out, values[i], false)) {
                    rendered = false;
                    break;
                }
            }
            for (size_t i = 0; i < count; i++) {
                release(values[i]);
            }
            free(values);
            if (!rendered) {
                free(out.data);
                goto oom;
            }
            v = value_data(V_STR, (uint8_t *)(out.data ? out.data : ""), out.length);
            free(out.data);
            if (!v || !stack_push(frame, v)) {
                goto oom;
            }
            break;
        }
        case OP_ITER_INIT:
            a = stack_pop(frame, vm);
            if (!a) {
                break;
            }
            v = value_new(V_ITER);
            if (!v) {
                release(a);
                goto oom;
            }
            v->as.iterator.iterable = a;
            pn_big_init(&v->as.iterator.cursor);
            if (a->kind == V_RANGE && !pn_big_copy(&v->as.iterator.cursor, &a->as.range.start)) {
                release(v);
                goto oom;
            }
            if (!local_put(frame, instruction->name, v)) {
                goto oom;
            }
            break;
        case OP_ITER_NEXT:
            v = local_get(frame, instruction->name);
            if (!v || v->kind != V_ITER) {
                vm->error = "VM trap: invalid iterator";
                break;
            }
            a = v->as.iterator.iterable;
            if (a->kind == V_RANGE) {
                if (pn_big_compare(&v->as.iterator.cursor, &a->as.range.end) >= 0) {
                    coverage_branched = true;
                    frame->pc = instruction->target;
                    break;
                }
                b = value_big(V_NAT, &v->as.iterator.cursor);
                if (!pn_big_add_small(&v->as.iterator.cursor, 1)) {
                    release(b);
                    goto oom;
                }
            } else if (a->kind == V_ARRAY) {
                if (v->as.iterator.index >= a->as.sequence.count) {
                    coverage_branched = true;
                    frame->pc = instruction->target;
                    break;
                }
                b = retain(a->as.sequence.items[v->as.iterator.index++]);
            } else if (a->kind == V_BYTES) {
                if (v->as.iterator.index >= a->as.bytes.length) {
                    coverage_branched = true;
                    frame->pc = instruction->target;
                    break;
                }
                b = value_size(a->as.bytes.data[v->as.iterator.index++]);
            } else {
                vm->error = "VM trap: value is not iterable";
                break;
            }
            if (!b || !local_put(frame, instruction->name2, b)) {
                goto oom;
            }
            break;
        case OP_MAKE_RECORD: {
            Value **values = calloc(instruction->count, sizeof(Value *));
            if (instruction->count && !values) {
                goto oom;
            }
            for (size_t i = instruction->count; i-- > 0;) {
                values[i] = stack_pop(frame, vm);
            }
            v = named_value(V_RECORD, instruction->name, instruction->items, values,
                            instruction->count);
            for (size_t i = 0; i < instruction->count; i++) {
                release(values[i]);
            }
            free(values);
            if (!v || !stack_push(frame, v)) {
                goto oom;
            }
            break;
        }
        case OP_FIELD_GET:
            a = stack_pop(frame, vm);
            if (!a) {
                break;
            }
            if (a->kind != V_RECORD) {
                vm->error = "VM trap: field access requires a record";
            } else {
                for (size_t i = 0; i < a->as.named.count; i++) {
                    if (!strcmp(a->as.named.names[i], instruction->name)) {
                        v = retain(a->as.named.values[i]);
                        break;
                    }
                }
            }
            if (!v && !vm->error) {
                vm->error = "VM trap: record has no field";
            }
            release(a);
            if ((!v && !vm->error) || (v && !stack_push(frame, v))) {
                goto oom;
            }
            break;
        case OP_MAKE_VARIANT: {
            Value **values = calloc(instruction->count, sizeof(Value *));
            if (instruction->count && !values) {
                goto oom;
            }
            for (size_t i = instruction->count; i-- > 0;) {
                values[i] = stack_pop(frame, vm);
            }
            v = named_value(V_VARIANT, instruction->name2, NULL, values, instruction->count);
            for (size_t i = 0; i < instruction->count; i++) {
                release(values[i]);
            }
            free(values);
            if (!v || !stack_push(frame, v)) {
                goto oom;
            }
            break;
        }
        case OP_MATCH_VARIANT:
            a = stack_pop(frame, vm);
            if (!a) {
                break;
            }
            if (a->kind != V_VARIANT) {
                vm->error = "VM trap: match subject is not an enum";
            } else if (strcmp(a->as.named.name, instruction->name)) {
                coverage_branched = true;
                frame->pc = instruction->target;
            } else {
                for (size_t i = 0; i < a->as.named.count; i++) {
                    if (!stack_push(frame, retain(a->as.named.values[i]))) {
                        release(a);
                        goto oom;
                    }
                }
            }
            release(a);
            break;
        case OP_MATCH_FAIL:
            vm->error = "VM trap: enum match was not exhaustive";
            break;
        case OP_AWAIT_CALL:
        case OP_CALL: {
            bool called_function = false;
            Value **args = calloc(instruction->arity, sizeof(Value *));
            if (instruction->arity && !args) {
                goto oom;
            }
            for (size_t i = instruction->arity; i-- > 0;) {
                args[i] = stack_pop(frame, vm);
            }
            if (vm->error) {
                for (size_t i = 0; i < instruction->arity; i++) {
                    release(args[i]);
                }
                free(args);
                break;
            }
            const Builtin *built = builtin(instruction->name);
            Function *called = built ? NULL : program_function(vm->program, instruction->name);
            if ((!built && !called) ||
                instruction->arity != (built ? built->arity : called->param_count) ||
                !verify_call_edge(function, called, built && built->pure,
                                  builtin_is_async(built), builtin_is_local_state(built),
                                  instruction->op == OP_AWAIT_CALL)) {
                vm->error = "VM trap: invalid async call effect or signature";
            } else if (built) {
                v = execution_builtin(execution, built, args, &called_function);
            } else {
                frame_push(execution, called, args);
            }
            for (size_t i = 0; i < instruction->arity; i++) {
                release(args[i]);
            }
            free(args);
            if (built && !called_function && execution->status == VM_YIELDED &&
                ((!v && !vm->error) || (v && !stack_push(frame, v)))) {
                goto oom;
            }
            break;
        }
        case OP_JUMP_FALSE:
            a = stack_pop(frame, vm);
            if (!a) {
                break;
            }
            if (a->kind != V_BOOL) {
                vm->error = "VM trap: conditional requires Bool";
            } else if (!a->as.boolean) {
                coverage_branched = true;
                frame->pc = instruction->target;
            }
            release(a);
            break;
        case OP_JUMP:
            frame->pc = instruction->target;
            break;
        case OP_RETURN:
            v = stack_pop(frame, vm);
            if (!v) {
                break;
            }
            if (function->is_async && !strcmp(function->name, "main") && v->kind != V_UNIT) {
                release(v);
                vm->error = "VM trap: async main must return Unit";
                break;
            }
            frame_free(frame);
            execution->frame_count--;
            if (!execution->frame_count) {
                execution->result = v;
                execution->status = VM_COMPLETED;
            } else if (!stack_push(&execution->frames[execution->frame_count - 1], v)) {
                goto oom;
            }
            coverage_complete_pending(execution);
            break;
        case OP_AWAIT_VALUE:
        case OP_CALL_VALUE: {
            bool called_function = false;
            Value **args = calloc(instruction->arity, sizeof(Value *));
            if (instruction->arity && !args) {
                goto oom;
            }
            for (size_t i = instruction->arity; i-- > 0;) {
                args[i] = stack_pop(frame, vm);
            }
            Value *callable = stack_pop(frame, vm);
            if (vm->error) {
                for (size_t i = 0; i < instruction->arity; i++) {
                    release(args[i]);
                }
                free(args);
                release(callable);
                break;
            }
            if (!callable || callable->kind != V_STR) {
                vm->error = "VM trap: indirect call requires a callable";
            } else {
                char *callee = malloc(callable->as.bytes.length + 1);
                if (!callee) {
                    for (size_t i = 0; i < instruction->arity; i++) {
                        release(args[i]);
                    }
                    free(args);
                    release(callable);
                    goto oom;
                }
                memcpy(callee, callable->as.bytes.data, callable->as.bytes.length);
                callee[callable->as.bytes.length] = 0;
                const Builtin *built = builtin(callee);
                Function *called = program_function(vm->program, callee);
                if (!built && !called) {
                    vm->error = "VM trap: indirect call target was not found";
                } else if (instruction->arity != (built ? built->arity : called->param_count)) {
                    vm->error = "VM trap: indirect call arity mismatch";
                } else if (memchr(callable->as.bytes.data, 0, callable->as.bytes.length) ||
                           !verify_call_edge(function, built ? NULL : called, built && built->pure,
                                             builtin_is_async(built), builtin_is_local_state(built),
                                             instruction->op == OP_AWAIT_VALUE)) {
                    vm->error = function->pure
                        ? "VM trap: pure function invokes impure callable"
                        : "VM trap: invalid async callable effect";
                } else {
                    if (built) {
                        v = execution_builtin(execution, built, args, &called_function);
                    } else {
                        called_function = true;
                        frame_push(execution, called, args);
                    }
                }
                free(callee);
            }
            for (size_t i = 0; i < instruction->arity; i++) {
                release(args[i]);
            }
            free(args);
            release(callable);
            if (!called_function && execution->status == VM_YIELDED &&
                ((!v && !vm->error) || (v && !stack_push(frame, v)))) {
                goto oom;
            }
            break;
        }
        default:
            vm->error = "VM trap: unknown instruction";
            break;
        }
        if (vm->coverage && !vm->error && execution->status != VM_EXITED) {
            if (execution->frame_count > coverage_depth || execution->status == VM_WAITING) {
                Frame *caller = &execution->frames[coverage_depth - 1];
                caller->coverage_pending = true;
                caller->coverage_pending_pc = coverage_pc;
            } else {
                vm_coverage_finish(vm->coverage, coverage_function, coverage_pc, coverage_branched);
            }
        }

        if (execution->yield_requested) { execution->yield_requested = false; break; }
    }
    if (vm->error) {
        execution->status = VM_TRAPPED;
    }
    if (execution->status != VM_YIELDED && execution->status != VM_WAITING) {
        vm_coverage_terminal(vm->coverage, execution->status == VM_COMPLETED ? 1 :
                             execution->status == VM_TRAPPED ? 2 : 3);
        frames_clear(execution);
    }
    execution->running = false;
    return execution->status;
oom:
    vm->error = "native VM out of memory";
    execution->status = VM_TRAPPED;
    vm_coverage_terminal(vm->coverage, 2);
    frames_clear(execution);
    execution->running = false;
    return VM_TRAPPED;
}

VMExecution *vm_execution_create(VM *vm, Function *function, Value **arguments,
                                VMHostCall host_call, void *host_context, const char **error)
{
    *error = NULL;
    if (vm->execution) {
        *error = "VM invocation is already active";
        return NULL;
    }
    VMExecution *execution = calloc(1, sizeof(VMExecution));
    if (!execution) {
        *error = "native VM out of memory";
        return NULL;
    }
    *execution = (VMExecution){.vm = vm, .embedded = true, .status = VM_YIELDED,
                               .host_call = host_call, .host_context = host_context};
    vm->error = NULL;
    vm->fault_kind = VM_FAULT_FATAL;
    vm->execution = execution;
    vm_coverage_start(vm->coverage, vm->program);
    if (!frame_push(execution, function, arguments)) {
        *error = vm->error;
        vm_coverage_terminal(vm->coverage, 2);
        vm_execution_destroy(execution);
        return NULL;
    }
    return execution;
}

VMExecution *vm_execution_create_pending(VM *vm, Function *function, Value **arguments,
                                         VMServiceWait wait, void *context, const char **error)
{
    VMExecution *execution = vm_execution_create(vm, function, arguments, NULL, context, error);
    if (execution) {
        execution->print_wait = wait;
    }
    return execution;
}

bool vm_execution_complete_print(VMExecution *execution, const char *error)
{
    if (execution->running || execution->status != VM_WAITING || execution->waiting_read ||
        execution->server || execution->http_waiting) {
        return false;
    }
    VM *vm = execution->vm;
    vm->error = error;
    if (!error) {
        Value *acknowledgement = value_void();
        if (!acknowledgement ||
            !stack_push(&execution->frames[execution->frame_count - 1], acknowledgement)) {
            vm->error = "native VM out of memory";
        }
    }
    execution->status = vm->error ? VM_TRAPPED : VM_YIELDED;
    if (!vm->error) coverage_complete_pending(execution);
    else vm_coverage_terminal(vm->coverage, 2);
    if (vm->error) {
        frames_clear(execution);
    }
    return true;
}

Value *vm_execution_result(const VMExecution *execution)
{
    return execution->result;
}

size_t vm_execution_exit_status(const VMExecution *execution)
{
    return execution->exit_status;
}

bool vm_execution_destroy(VMExecution *execution)
{
    if (!execution) {
        return true;
    }
    if (execution->running) {
        return false;
    }
    tcp_exchange_destroy(execution->tcp);
    http_transport_destroy(execution->http);
    root_read_destroy(execution->root_read);
    console_write_destroy(execution->console_write);
    tcp_server_destroy(execution->server);
    restore_stop_signals(execution);
    filesystem_stage_destroy(execution->vm);
    gzip_stream_destroy(execution->vm);
    http_stream_destroy(execution->vm);
    digest_destroy(execution->vm);
    frames_clear(execution);
    free(execution->frames);
    release(execution->result);
    release(execution->http_waiting);
    execution->vm->execution = NULL;
    free(execution);
    return true;
}

/* Legacy synchronous adapter: same dispatch loop, no second execution engine.
 * Nested bytecode keeps its existing synchronous CLI behavior.
 */
Value *execute(VM *vm, Function *function, Value **arguments)
{
    if (vm->execution) return NULL;
    VMExecution execution = {.vm = vm, .status = VM_YIELDED};
    struct VMHttpRuntime runtime = {.owner = &execution};
    bool owns_runtime = vm->http_runtime == NULL;
    if (owns_runtime) vm->http_runtime = &runtime;
    vm->execution = &execution;
    vm->fault_kind = VM_FAULT_FATAL;
    vm_coverage_start(vm->coverage, vm->program);
    if (frame_push(&execution, function, arguments)) {
        for (;;) {
            if (owns_runtime && http_runtime_active(&runtime)) {
#ifndef __wasi__
                if (execution.owns_stop_signals && service_stop_requested) {
                    runtime.exit_requested = true;
                    runtime.exit_status = 0;
                    http_runtime_stop(&runtime);
                }
#endif
                if (execution.status == VM_EXITED || runtime.exit_requested) http_runtime_stop(&runtime);
                http_runtime_pump(&runtime, 1024, execution.status == VM_YIELDED ? 0 : 1);
                if (runtime.error) { vm->error = runtime.error; break; }
            }
            bool active = owns_runtime && http_runtime_active(&runtime);
            if (execution.status == VM_COMPLETED || execution.status == VM_EXITED || runtime.exit_requested) {
                if (active) continue;
                break;
            }
            VMExecutionStatus status = vm_execution_advance(&execution, active ? 1024 : SIZE_MAX);
            if (status == VM_YIELDED) continue;
            if ((status == VM_COMPLETED || status == VM_EXITED) && owns_runtime && http_runtime_active(&runtime)) continue;
            if (status == VM_WAITING && execution.http_waiting) continue;
            if (status == VM_WAITING && execution.server) {
                vm_execution_pump_server(&execution, 1024, active ? 1 : 10);
                continue;
            }
            if (status == VM_WAITING && execution.console_write) {
                if (!active) console_write_wait(execution.console_write);
                vm_execution_poll_console(&execution);
                continue;
            }
            if (status == VM_WAITING && execution.root_read) {
                if (!active) root_read_wait(execution.root_read);
                vm_execution_poll_filesystem(&execution);
                continue;
            }
            if (status != VM_WAITING || !execution.waiting_read) break;
            if (execution.tcp || execution.http) {
                vm_execution_poll_network(&execution, active ? 1 : 1000);
                continue;
            }
            Value *result = vm_fake_read_result(execution.fake_fail);
            vm_execution_complete_read(&execution, result);
            release(result);
        }
    }
    if (vm->error) vm_coverage_terminal(vm->coverage, 2);
    if (owns_runtime) {
        http_runtime_destroy(&runtime);
        vm->http_runtime = NULL;
    }
    tcp_exchange_destroy(execution.tcp);
    http_transport_destroy(execution.http);
    root_read_destroy(execution.root_read);
    console_write_destroy(execution.console_write);
    tcp_server_destroy(execution.server);
    restore_stop_signals(&execution);
    filesystem_stage_destroy(vm);
    gzip_stream_destroy(vm);
    http_stream_destroy(vm);
    digest_destroy(vm);
    frames_clear(&execution);
    free(execution.frames);
    release(execution.http_waiting);
    vm->execution = NULL;
    if (owns_runtime && !vm->error && (execution.status == VM_EXITED || runtime.exit_requested)) {
        Value *code = value_size(runtime.exit_requested ? runtime.exit_status : execution.exit_status);
        if (!code) vm->error = "native VM out of memory";
        else { Value *ignored = host_call(vm, "process_exit", &code); release(ignored); release(code); }
    }
    return execution.result;
}

VMExecution *vm_execution_create_async(VM *vm, Function *fn, Value **arguments,
                                       VMServiceWait wait, void *context, const char **error)
{
    VMExecution *execution = vm_execution_create(vm, fn, arguments, NULL, context, error);
    if (execution) {
        execution->read_wait = wait;
    }
    return execution;
}

bool vm_execution_enable_server_reads(VMExecution *execution)
{
    if (!execution || execution->running || execution->status != VM_YIELDED ||
        !execution->embedded || !execution->read_wait) {
        return false;
    }
    execution->server_read_enabled = true;
    return true;
}

bool vm_fake_read_result_valid(const Value *result)
{
    if (!result || result->kind != V_VARIANT || result->as.named.count != 1 ||
        !result->as.named.values[0]) {
        return false;
    }
    const Value *payload = result->as.named.values[0];
    return (!strcmp(result->as.named.name, "Ok") && payload->kind == V_BYTES) ||
           (!strcmp(result->as.named.name, "Error") && payload->kind == V_STR);
}

Value *vm_fake_read_result(bool fail)
{
    const char *text = fail ? "fake I/O error" : "fake bytes";
    Value *payload = value_data(fail ? V_STR : V_BYTES, (const uint8_t *)text, strlen(text));
    if (!payload) {
        return NULL;
    }
    Value *result = named_value(V_VARIANT, fail ? "Error" : "Ok", NULL, &payload, 1);
    release(payload);
    return result;
}

bool vm_execution_complete_read(VMExecution *execution, Value *result)
{
    if (execution->running || execution->status != VM_WAITING || !execution->waiting_read ||
        execution->tcp || execution->http) {
        return false;
    }
    execution->waiting_read = false;
    if (!vm_fake_read_result_valid(result)) {
        execution->vm->error = result ? "VM trap: invalid async completion type" : "native VM out of memory";
    } else if (!stack_push(&execution->frames[execution->frame_count - 1], retain(result))) {
        execution->vm->error = "native VM out of memory";
    }
    execution->status = execution->vm->error ? VM_TRAPPED : VM_YIELDED;
    if (!execution->vm->error) coverage_complete_pending(execution);
    else vm_coverage_terminal(execution->vm->coverage, 2);
    if (execution->vm->error) {
        frames_clear(execution);
    }
    return true;
}

/* Opt-in network capability for native embedders. The CLI enables this through
 * its synchronous adapter. Existing embedded/fake-task sessions stay isolated. */
bool vm_execution_enable_tcp(VMExecution *execution)
{
    if (execution->running || execution->status != VM_YIELDED) return false;
    execution->network_enabled = true;
    return true;
}

bool vm_execution_poll_tcp(VMExecution *execution, unsigned max_wait_ms)
{
    if (execution->running || execution->status != VM_WAITING || !execution->tcp) return false;
    if (tcp_exchange_poll(execution->tcp, max_wait_ms)) {
        VMTcpExchange *tcp = execution->tcp;
        execution->tcp = NULL;
        vm_execution_complete_read(execution, tcp_exchange_result(tcp));
        tcp_exchange_destroy(tcp);
    }
    return true;
}

bool vm_execution_poll_http(VMExecution *execution, unsigned max_wait_ms)
{
    if (execution->running || execution->status != VM_WAITING || !execution->http) return false;
    if (http_transport_poll(execution->http, max_wait_ms)) {
        VMHttpTransport *http = execution->http;
        execution->http = NULL;
        vm_execution_complete_read(execution, http_transport_result(http));
        http_transport_destroy(http);
    }
    return true;
}

bool vm_execution_enable_console(VMExecution *execution)
{
    if (execution->running || execution->status != VM_YIELDED) {
        return false;
    }
    execution->console_enabled = true;
    return true;
}

bool vm_execution_poll_console(VMExecution *execution)
{
    if (execution->running || execution->status != VM_WAITING || !execution->console_write) {
        return false;
    }
    if (!console_write_poll(execution->console_write)) {
        return true;
    }
    Value *result = console_write_result(execution->console_write);
    if (!result || !stack_push(&execution->frames[execution->frame_count - 1], retain(result))) {
        execution->vm->error = "native VM out of memory";
    }
    console_write_destroy(execution->console_write);
    execution->console_write = NULL;
    execution->status = execution->vm->error ? VM_TRAPPED : VM_YIELDED;
    if (execution->vm->error) {
        vm_coverage_terminal(execution->vm->coverage, 2);
        frames_clear(execution);
    } else {
        coverage_complete_pending(execution);
    }
    return true;
}

bool vm_execution_enable_filesystem(VMExecution *execution)
{
    if (execution->running || execution->status != VM_YIELDED) {
        return false;
    }
    execution->filesystem_enabled = true;
    return true;
}

bool vm_execution_poll_filesystem(VMExecution *execution)
{
    if (execution->running || execution->status != VM_WAITING || !execution->root_read) {
        return false;
    }
    if (!root_read_poll(execution->root_read)) {
        return true;
    }
    /* The native adapter constructs this typed result. Fake/network completion
     * entry points cannot inject a filesystem result into this wait. */
    Value *result = root_read_result(execution->root_read);
    execution->vm->error = root_read_error(execution->root_read);
    if (!execution->vm->error &&
        (!result || !stack_push(&execution->frames[execution->frame_count - 1], retain(result)))) {
        execution->vm->error = "native VM out of memory";
    }
    root_read_destroy(execution->root_read);
    execution->root_read = NULL;
    execution->status = execution->vm->error ? VM_TRAPPED : VM_YIELDED;
    if (execution->vm->error) {
        vm_coverage_terminal(execution->vm->coverage, 2);
        frames_clear(execution);
    } else {
        coverage_complete_pending(execution);
    }
    return true;
}

bool vm_execution_poll_network(VMExecution *execution, unsigned max_wait_ms)
{
    if (execution->console_write) {
        return vm_execution_poll_console(execution);
    }
    if (execution->root_read) {
        return vm_execution_poll_filesystem(execution);
    }
    if (execution->tcp) return vm_execution_poll_tcp(execution, max_wait_ms);
    if (execution->http) return vm_execution_poll_http(execution, max_wait_ms);
    return false;
}

/* Server pumping is separate from socket-only TCP polling because it dispatches
 * bounded child bytecode. No nested synchronous interpreter is used. */
bool vm_execution_enable_server(VMExecution *execution)
{
    if (execution->running || execution->status != VM_YIELDED) {
        return false;
    }
    execution->server_enabled = true;
    return true;
}

bool vm_execution_stop_server(VMExecution *execution)
{
    if (execution->running || !execution->server) {
        return false;
    }
    tcp_server_stop(execution->server);
    return true;
}

bool vm_execution_pump_server(VMExecution *execution, size_t budget, unsigned max_wait_ms)
{
    if (execution->running || execution->status != VM_WAITING || !execution->server) {
        return false;
    }
    execution->running = true;
#ifndef __wasi__
    if (execution->owns_stop_signals && service_stop_requested) {
        tcp_server_stop(execution->server);
    }
#endif
    tcp_server_poll(execution->server, max_wait_ms);
    tcp_server_advance(execution->server, budget);
    if (tcp_server_done(execution->server)) {
        execution->vm->error = tcp_server_error(execution->server);
        if (!execution->vm->error) {
            Value *result = tcp_server_result(execution->server);
            if (!result ||
                !stack_push(&execution->frames[execution->frame_count - 1], retain(result))) {
                execution->vm->error = "native VM out of memory";
            }
        }
        tcp_server_destroy(execution->server);
        execution->server = NULL;
        restore_stop_signals(execution);
        execution->status = execution->vm->error ? VM_TRAPPED : VM_YIELDED;
        if (!execution->vm->error) coverage_complete_pending(execution);
        else vm_coverage_terminal(execution->vm->coverage, 2);
        if (execution->vm->error) {
            frames_clear(execution);
        }
    }
    execution->running = false;
    return true;
}
