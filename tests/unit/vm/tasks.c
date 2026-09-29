/* Deterministic host/lifecycle contracts, independent of an OS event loop. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "tasks.h"

#include "buffer.h"
#include "decode.h"
#include "render.h"
#include "verify.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *params[] = {"value"};
static Instruction identity[] = {{.op = OP_LOAD, .name = "value"}, {.op = OP_RETURN}};
static Instruction printing[] = {{.op = OP_LOAD, .name = "value"},
                                 {.op = OP_CALL, .name = "print", .arity = 1},
                                 {.op = OP_RETURN}};
static Instruction repeated[] = {{.op = OP_LOAD, .name = "value"},
                                 {.op = OP_CALL, .name = "print", .arity = 1},
                                 {.op = OP_STORE, .name = "first"},
                                 {.op = OP_LOAD, .name = "value"},
                                 {.op = OP_CALL, .name = "print", .arity = 1},
                                 {.op = OP_RETURN}};
static Instruction indirect[] = {
    {.op = OP_CONST, .constant = {.tag = 3, .text = "print", .text_length = 5}},
    {.op = OP_LOAD, .name = "value"},
    {.op = OP_CALL_VALUE, .arity = 1},
    {.op = OP_RETURN}};
static Instruction nested[] = {{.op = OP_LOAD, .name = "value"},
                               {.op = OP_CALL, .name = "printing", .arity = 1},
                               {.op = OP_RETURN}};
static Instruction trap[] = {{.op = OP_MATCH_FAIL}};
static Instruction looping[] = {{.op = OP_JUMP, .target = 0}};
static Instruction exiting[] = {{.op = OP_LOAD, .name = "value"},
                                {.op = OP_CALL, .name = "process_exit", .arity = 1}};
static Function functions[] = {
    {.name = "identity", .params = params, .param_count = 1, .ins = identity, .ins_count = 2},
    {.name = "printing", .params = params, .param_count = 1, .ins = printing, .ins_count = 3},
    {.name = "repeated", .params = params, .param_count = 1, .ins = repeated, .ins_count = 6},
    {.name = "indirect", .params = params, .param_count = 1, .ins = indirect, .ins_count = 4},
    {.name = "trap", .params = params, .param_count = 1, .ins = trap, .ins_count = 1},
    {.name = "loop", .params = params, .param_count = 1, .ins = looping, .ins_count = 1},
    {.name = "exit", .params = params, .param_count = 1, .ins = exiting, .ins_count = 2},
    {.name = "nested", .params = params, .param_count = 1, .ins = nested, .ins_count = 3}};
static Program program = {.functions = functions, .count = 8};
static const VMTaskId root = {0};

static VMTasks *session(size_t capacity, size_t events)
{
    VM vm = {.program = &program};
    const char *error;
    VMTasks *tasks = vm_tasks_create(&vm, capacity, events, &error);
    assert(tasks && !error);
    return tasks;
}

static VMTaskId spawn(VMTasks *tasks, VMTaskId parent, size_t function, Value *value,
                      uint64_t deadline)
{
    const char *error;
    VMTaskId id = vm_tasks_spawn(tasks, parent, &functions[function], &value, deadline, &error);
    assert(id.slot && !error);
    return id;
}

static VMOperationId pending(VMTasks *tasks, VMTaskId task, Value *expected)
{
    VMOperationId operation;
    Value *input;
    assert(vm_tasks_pending(tasks, task, &operation, &input) && input == expected);
    assert(vm_tasks_status(tasks, task) == TASK_WAITING);
    return operation;
}

static void scoped_join_and_failure(void)
{
    Value *value = value_size(42);
    for (size_t fail = 0; fail < 3; fail++) {
        VMTasks *tasks = session(4, 4);
        VMTaskId parent = spawn(tasks, root, 0, value, UINT64_MAX);
        VMTaskId child = spawn(tasks, parent, 7, value, UINT64_MAX);
        VMTaskId grandchild = spawn(tasks, child, 3, value, UINT64_MAX);
        VMTaskId sibling = spawn(tasks, parent, 1, value, UINT64_MAX);
        assert(vm_tasks_pump(tasks, 0, 100));
        assert(vm_tasks_status(tasks, parent) == TASK_JOINING);
        assert(!vm_tasks_result(tasks, parent));
        VMOperationId first = pending(tasks, child, value);
        VMOperationId second = pending(tasks, grandchild, value);
        VMOperationId third = pending(tasks, sibling, value);
        assert(vm_tasks_complete(tasks, first, NULL));
        assert(vm_tasks_pump(tasks, 0, 100));
        assert(vm_tasks_status(tasks, child) == TASK_JOINING);
        if (fail == 1) {
            assert(vm_tasks_complete(tasks, second, "fake operation failed"));
        } else if (fail == 2) {
            assert(vm_tasks_cancel(tasks, grandchild));
        } else {
            assert(vm_tasks_complete(tasks, second, NULL));
            assert(vm_tasks_complete(tasks, third, NULL));
        }
        assert(vm_tasks_pump(tasks, 0, 100));
        if (fail) {
            assert(vm_tasks_status(tasks, parent) == TASK_TRAPPED);
            assert(vm_tasks_status(tasks, child) == TASK_TRAPPED);
            assert(vm_tasks_status(tasks, sibling) == TASK_CANCELLED);
            assert(vm_tasks_status(tasks, grandchild) ==
                   (fail == 1 ? TASK_TRAPPED : TASK_CANCELLED));
            if (fail == 1) {
                assert(!strcmp(vm_tasks_error(tasks, grandchild), "fake operation failed"));
            }
            assert(!vm_tasks_complete(tasks, third, NULL));
        } else {
            assert(vm_tasks_status(tasks, parent) == TASK_COMPLETED);
            assert(vm_tasks_status(tasks, child) == TASK_COMPLETED);
            assert(vm_tasks_result(tasks, parent) == value);
            assert(vm_tasks_result(tasks, child)->kind == V_VOID);
            assert(vm_tasks_cancel(tasks, parent));
            assert(vm_tasks_status(tasks, parent) == TASK_COMPLETED);
        }
        VMTaskStats stats = vm_tasks_stats(tasks);
        assert(stats.registered == 3 && stats.released == 3 && !stats.queued);
        assert(vm_tasks_destroy(tasks) && value->refs == 1);
    }
    release(value);
}

static void stale_operations_and_backpressure(void)
{
    Value *value = value_size(9);
    VMTasks *tasks = session(2, 1), *other = session(1, 1);
    VMTaskId first = spawn(tasks, root, 2, value, UINT64_MAX);
    VMTaskId second = spawn(tasks, root, 1, value, UINT64_MAX);
    VMTaskId foreign = spawn(other, root, 1, value, UINT64_MAX);
    assert(vm_tasks_pump(tasks, 0, 100) && vm_tasks_pump(other, 0, 100));
    VMOperationId old = pending(tasks, first, value);
    VMOperationId next = pending(tasks, second, value);
    assert(!vm_tasks_complete(other, old, NULL));
    assert(vm_tasks_status(tasks, foreign) == TASK_INVALID);
    VMOperationId forged = old;
    forged.generation++;
    assert(!vm_tasks_complete(tasks, forged, NULL));
    assert(vm_tasks_complete(tasks, old, NULL));
    assert(!vm_tasks_complete(tasks, old, NULL));
    assert(!vm_tasks_complete(tasks, next, NULL));
    assert(vm_tasks_status(tasks, first) == TASK_WAITING); /* enqueue is not execution */
    assert(vm_tasks_pump(tasks, 0, 100));
    VMOperationId current = pending(tasks, first, value);
    assert(current.generation == old.generation + 1);
    assert(!vm_tasks_complete(tasks, old, NULL));
    assert(vm_tasks_complete(tasks, next, NULL));
    assert(vm_tasks_pump(tasks, 0, 0));
    assert(vm_tasks_status(tasks, second) == TASK_READY);
    assert(vm_tasks_complete(tasks, current, NULL));
    assert(vm_tasks_pump(tasks, 0, 100));
    assert(vm_tasks_status(tasks, first) == TASK_COMPLETED);
    assert(vm_tasks_status(tasks, second) == TASK_COMPLETED);
    const char *error;
    assert(!vm_tasks_spawn(tasks, root, functions, &value, UINT64_MAX, &error).slot && error);
    assert(!vm_tasks_spawn(other, first, functions, &value, UINT64_MAX, &error).slot);
    assert(vm_tasks_stats(tasks).registered == 3 && vm_tasks_stats(tasks).released == 3);
    assert(vm_tasks_destroy(tasks));
    tasks = session(1, 1);
    assert(!vm_tasks_complete(tasks, current, NULL)); /* destroyed session identity */
    assert(vm_tasks_destroy(tasks) && vm_tasks_destroy(other) && value->refs == 1);
    release(value);
}

static void admission_and_completed_child_lifetime(void)
{
    Value *value = value_size(31);
    VMTasks *tasks = session(5, 2);
    VMTaskId parent = spawn(tasks, root, 1, value, UINT64_MAX);
    VMTaskId completed = spawn(tasks, parent, 0, value, UINT64_MAX);
    assert(vm_tasks_pump(tasks, 0, 100));
    assert(vm_tasks_result(tasks, completed) == value);
    const char *error;
    assert(!vm_tasks_spawn(tasks, completed, functions, &value, UINT64_MAX, &error).slot);
    VMTaskId late_child = spawn(tasks, parent, 1, value, UINT64_MAX);
    assert(vm_tasks_complete(tasks, pending(tasks, parent, value), NULL));
    assert(vm_tasks_pump(tasks, 0, 100));
    assert(vm_tasks_status(tasks, parent) == TASK_JOINING);
    assert(!vm_tasks_spawn(tasks, parent, functions, &value, UINT64_MAX, &error).slot);
    assert(vm_tasks_cancel(tasks, parent));
    assert(vm_tasks_status(tasks, late_child) == TASK_CANCELLED);
    assert(vm_tasks_status(tasks, completed) == TASK_COMPLETED);
    assert(vm_tasks_result(tasks, completed) == value);
    assert(vm_tasks_destroy(tasks) && value->refs == 1);
    release(value);
}

static void cancellation_at_each_transition(void)
{
    Value *value = value_size(11);
    for (size_t stage = 0; stage < 6; stage++) {
        for (size_t destroy = 0; destroy < 2; destroy++) {
            VMTasks *tasks = session(2, 2);
            VMTaskId parent = spawn(tasks, root, 0, value, UINT64_MAX);
            VMTaskId child = spawn(tasks, parent, 1, value, UINT64_MAX);
            VMOperationId operation = {0};
            if (stage) {
                assert(vm_tasks_pump(tasks, 0, stage == 1 ? 2 : 100));
            }
            if (stage >= 2) {
                operation = pending(tasks, child, value);
            }
            if (stage >= 3) {
                assert(vm_tasks_complete(tasks, operation, NULL));
            }
            if (stage >= 4) {
                assert(vm_tasks_pump(tasks, 0, stage == 4 ? 0 : 100));
            }
            if (!destroy) {
                assert(vm_tasks_cancel(tasks, parent));
                assert(vm_tasks_cancel(tasks, parent));
                assert(vm_tasks_status(tasks, parent) ==
                       (stage == 5 ? TASK_COMPLETED : TASK_CANCELLED));
                assert(vm_tasks_status(tasks, child) ==
                       (stage == 5 ? TASK_COMPLETED : TASK_CANCELLED));
                assert(!vm_tasks_complete(tasks, operation, NULL));
                assert(vm_tasks_pump(tasks, 0, 100)); /* stale queued event drains safely */
                VMTaskStats stats = vm_tasks_stats(tasks);
                assert(stats.registered == stats.released && !stats.queued);
            }
            assert(vm_tasks_destroy(tasks) && value->refs == 1);
        }
    }
    release(value);
}

static void deadlines_fairness_and_terminal_outcomes(void)
{
    Value *value = value_size(7);
    VMTasks *tasks = session(5, 2);
    VMTaskId parent = spawn(tasks, root, 5, value, 10);
    VMTaskId child = spawn(tasks, parent, 1, value, 100);
    VMTaskId earlier = spawn(tasks, parent, 5, value, 3);
    VMTaskId independent = spawn(tasks, root, 0, value, UINT64_MAX);
    assert(vm_tasks_next_deadline(tasks) == 3);
    assert(vm_tasks_pump(tasks, 0, 8));
    assert(vm_tasks_status(tasks, independent) == TASK_COMPLETED); /* CPU loop cannot starve it */
    VMOperationId operation = pending(tasks, child, value);
    assert(vm_tasks_pump(tasks, 3, 0));
    assert(vm_tasks_status(tasks, earlier) == TASK_CANCELLED);
    assert(vm_tasks_status(tasks, child) == TASK_CANCELLED);
    assert(vm_tasks_status(tasks, parent) == TASK_TRAPPED);
    assert(!vm_tasks_complete(tasks, operation, NULL));
    assert(vm_tasks_next_deadline(tasks) == UINT64_MAX);
    assert(!vm_tasks_pump(tasks, 2, 100));
    assert(vm_tasks_destroy(tasks));

    tasks = session(3, 1);
    parent = spawn(tasks, root, 0, value, 10);
    child = spawn(tasks, parent, 1, value, 100);
    independent = spawn(tasks, root, 5, value, UINT64_MAX);
    assert(vm_tasks_next_deadline(tasks) == 10);
    assert(vm_tasks_pump(tasks, 9, 20));
    operation = pending(tasks, child, value);
    assert(vm_tasks_complete(tasks, operation, NULL));
    assert(vm_tasks_pump(tasks, 10, 0)); /* delivered operation does not defeat task deadline */
    assert(vm_tasks_status(tasks, parent) == TASK_CANCELLED);
    assert(vm_tasks_status(tasks, child) == TASK_CANCELLED);
    assert(vm_tasks_status(tasks, independent) == TASK_READY);
    assert(vm_tasks_destroy(tasks));

    for (size_t function = 4; function <= 6; function += 2) {
        tasks = session(2, 1);
        parent = spawn(tasks, root, function, value, UINT64_MAX);
        child = spawn(tasks, parent, 1, value, UINT64_MAX);
        assert(vm_tasks_pump(tasks, 0, 100));
        assert(vm_tasks_status(tasks, parent) == (function == 4 ? TASK_TRAPPED : TASK_EXITED));
        assert(vm_tasks_status(tasks, child) == TASK_CANCELLED);
        if (function == 6) {
            assert(vm_tasks_exit_status(tasks, parent) == 7);
        } else {
            assert(vm_tasks_error(tasks, parent));
        }
        assert(vm_tasks_pump(tasks, 0, 100));
        assert(vm_tasks_destroy(tasks));
    }
    assert(value->refs == 1);
    release(value);
}

typedef struct {
    VMExecution *execution;
    Value *input;
} PendingHost;

static bool reentrant_wait(void *context, Value *input, const char **error)
{
    PendingHost *host = context;
    (void)error;
    host->input = retain(input);
    assert(vm_execution_advance(host->execution, 1) == VM_BUSY);
    assert(!vm_execution_complete_print(host->execution, NULL));
    assert(!vm_execution_destroy(host->execution));
    return true;
}

static void pending_execution_rejects_reentry(void)
{
    VM vm = {.program = &program};
    Value *value = value_size(5);
    PendingHost host = {0};
    const char *error;
    host.execution =
        vm_execution_create_pending(&vm, &functions[3], &value, reentrant_wait, &host, &error);
    assert(host.execution && !error);
    assert(!vm_execution_complete_print(host.execution, NULL));
    assert(vm_execution_advance(host.execution, 100) == VM_WAITING);
    assert(vm_execution_advance(host.execution, 100) == VM_WAITING);
    assert(host.input == value);
    assert(vm_execution_complete_print(host.execution, NULL));
    assert(!vm_execution_complete_print(host.execution, NULL));
    assert(vm_execution_advance(host.execution, 100) == VM_COMPLETED);
    assert(vm_execution_result(host.execution)->kind == V_VOID);
    release(host.input);
    assert(vm_execution_destroy(host.execution) && value->refs == 1);
    release(value);
}

void task_contracts(void)
{
    const char *error;
    VM vm = {.program = &program};
    assert(!vm_tasks_create(&vm, 0, 1, &error) && error);
    assert(!vm_tasks_create(&vm, 1, 0, &error) && error);
    assert(!vm_tasks_create(&vm, SIZE_MAX, 1, &error) && error);
    assert(!vm_tasks_create(&vm, 1, SIZE_MAX, &error) && error);
    scoped_join_and_failure();
    stale_operations_and_backpressure();
    admission_and_completed_child_lifetime();
    cancellation_at_each_transition();
    deadlines_fairness_and_terminal_outcomes();
    pending_execution_rejects_reentry();
}

/* Replay the independently expected bytecode corpus through pending host calls,
 * keeping the original stdout oracle. One instruction per pump forces
 * suspension across locals, calls and iteration, not just synthetic flat
 * instruction lists. */
void task_artifact(const char *path)
{
    FILE *file = fopen(path, "rb");
    assert(file && !fseek(file, 0, SEEK_END));
    long length = ftell(file);
    assert(length > 0 && length < 1048576 && !fseek(file, 0, SEEK_SET));
    uint8_t *bytes = malloc((size_t)length);
    assert(bytes && fread(bytes, 1, (size_t)length, file) == (size_t)length && !fclose(file));
    Program loaded = {0};
    const char *error;
    assert(decode(bytes, (size_t)length, &loaded, &error) && verify(&loaded, &error));
    free(bytes);
    VM vm = {.program = &loaded};
    VMTasks *tasks = vm_tasks_create(&vm, 1, 1, &error);
    assert(tasks && !error);
    VMTaskId task =
        vm_tasks_spawn(tasks, root, program_function(&loaded, "main"), NULL, UINT64_MAX, &error);
    assert(task.slot && !error);
    Buffer output = {0};
    size_t turns = 0;
    while (vm_tasks_status(tasks, task) == TASK_READY ||
           vm_tasks_status(tasks, task) == TASK_WAITING) {
        assert(++turns < 1000000 && vm_tasks_pump(tasks, 0, 1));
        VMOperationId operation;
        Value *input;
        if (vm_tasks_pending(tasks, task, &operation, &input)) {
            assert(render(&output, input, false) && buffer_text(&output, "\n"));
            assert(vm_tasks_complete(tasks, operation, NULL));
        }
    }
    assert(vm_tasks_status(tasks, task) == TASK_COMPLETED);
    assert(vm_tasks_result(tasks, task)->kind == V_VOID);
    VMTaskStats stats = vm_tasks_stats(tasks);
    assert(stats.registered && stats.registered == stats.released && !stats.queued);
    assert(vm_tasks_destroy(tasks));
    free_program(&loaded);
    assert(fwrite(output.data, 1, output.length, stdout) == output.length);
    free(output.data);
}
