#include "buffer.h"
#include "builtins.h"
#include "numeric.h"
#include "program.h"
#include "render.h"
#include "tcp.h"
#include "tcp_server.h"
#include "utf8.h"
#include "value.h"
#include "verify.h"
#include "vm.h"

#include <stdlib.h>
#include <string.h>

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
    bool waiting_read, fake_fail, network_enabled;
    VMTcpExchange *tcp;
    VMTcpServer *server;
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

/* Source types do not survive in v9; check the full limits schema and resolve
 * the actual named handler against this execution's verified program. */
static Value *server_call(VMExecution *execution, Value **arguments)
{
    VM *vm = execution->vm;
    if (execution->embedded && !execution->server_enabled) {
        vm->error = "VM trap: TCP server is unavailable in this execution";
        return NULL;
    }
    const char *fields[] = {"clients",        "concurrency",       "request_limit",
                            "response_limit", "client_timeout_ms", "admission_timeout_ms",
                            "drain_ms"};
    Value *record = arguments[3];
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
    execution->server = tcp_server_start(vm, handler, arguments[0], port, limits,
                                         !execution->embedded || execution->network_enabled);
    if (!execution->server) {
        vm->error = "native VM out of memory";
    } else {
        execution->status = VM_WAITING;
    }
    return NULL;
}

static Value *execution_builtin(VMExecution *execution, const Builtin *entry, Value **arguments)
{
    VM *vm = execution->vm;
    if (vm->coverage && (!strcmp(entry->name, "$tcp_serve") || !strcmp(entry->name, "process_run") ||
        !strcmp(entry->name, "run_bytecode") || !strcmp(entry->name, "run_bytecode_args"))) {
        vm_coverage_gap(vm->coverage);
    }
    if (!strcmp(entry->name, "$tcp_serve")) {
        return server_call(execution, arguments);
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
    if (!execution->embedded || entry->pure) {
        return builtin_call(vm, entry->name, arguments);
    }
    if (!strcmp(entry->name, "process_exit")) {
        if (arguments[0]->kind != V_NAT || !value_index(arguments[0], &execution->exit_status)) {
            vm->error = "VM trap: invalid process status";
        } else {
            execution->status = VM_EXITED;
        }
        return NULL;
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
            v = binary_value(instruction->code, a, b, &vm->error);
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
            } else if (a->kind == V_ARRAY) {
                if (index >= a->as.sequence.count) {
                    vm->error = "VM trap: index is out of bounds";
                } else {
                    v = retain(a->as.sequence.items[index]);
                }
            } else if (a->kind == V_BYTES) {
                if (index >= a->as.bytes.length) {
                    vm->error = "VM trap: index is out of bounds";
                } else {
                    v = value_size(a->as.bytes.data[index]);
                }
            } else if (a->kind == V_STR) {
                if (!utf8_offset(a, index, &start, &end)) {
                    vm->error = "VM trap: index is out of bounds";
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
                                  builtin_is_async(built),
                                  instruction->op == OP_AWAIT_CALL)) {
                vm->error = "VM trap: invalid async call effect or signature";
            } else if (built) {
                v = execution_builtin(execution, built, args);
            } else {
                frame_push(execution, called, args);
            }
            for (size_t i = 0; i < instruction->arity; i++) {
                release(args[i]);
            }
            free(args);
            if (built && execution->status == VM_YIELDED &&
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
                                             builtin_is_async(built),
                                             instruction->op == OP_AWAIT_VALUE)) {
                    vm->error = function->pure
                        ? "VM trap: pure function invokes impure callable"
                        : "VM trap: invalid async callable effect";
                } else {
                    if (built) {
                        v = execution_builtin(execution, built, args);
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
        execution->server) {
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
    tcp_server_destroy(execution->server);
    frames_clear(execution);
    free(execution->frames);
    release(execution->result);
    execution->vm->execution = NULL;
    free(execution);
    return true;
}

/* Legacy synchronous adapter: same dispatch loop, no second execution engine.
 * Nested bytecode keeps its existing synchronous CLI behavior.
 */
Value *execute(VM *vm, Function *function, Value **arguments)
{
    if (vm->execution) {
        return NULL;
    }
    VMExecution execution = {.vm = vm, .status = VM_YIELDED};
    vm->execution = &execution;
    vm_coverage_start(vm->coverage, vm->program);
    if (frame_push(&execution, function, arguments)) {
        for (;;) {
            VMExecutionStatus status = vm_execution_advance(&execution, SIZE_MAX);
            if (status == VM_YIELDED) {
                continue;
            }
            if (status == VM_WAITING && execution.server) {
                vm_execution_pump_server(&execution, 1024, 10);
                continue;
            }
            if (status != VM_WAITING || !execution.waiting_read) {
                break;
            }
            if (execution.tcp) {
                vm_execution_poll_tcp(&execution, 1000);
                continue;
            }
            Value *result = vm_fake_read_result(execution.fake_fail);
            vm_execution_complete_read(&execution, result);
            release(result);
        }
    }
    if (vm->error) vm_coverage_terminal(vm->coverage, 2);
    tcp_exchange_destroy(execution.tcp);
    tcp_server_destroy(execution.server);
    frames_clear(&execution);
    free(execution.frames);
    vm->execution = NULL;
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
    if (execution->running || execution->status != VM_WAITING || !execution->waiting_read || execution->tcp) {
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
