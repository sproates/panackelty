/* Async source effects and typed host completions must survive forged bytecode. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "tasks.h"
#include "verify.h"

#include <assert.h>
#include <string.h>

static char *parameters[] = {"fail"};
static Instruction read_code[] = {{.op = OP_LOAD, .name = "fail"},
                                  {.op = OP_AWAIT_CALL, .name = "async_fake_read", .arity = 1},
                                  {.op = OP_RETURN}};
static Instruction helper_code[] = {{.op = OP_LOAD, .name = "fail"},
                                    {.op = OP_AWAIT_CALL, .name = "read", .arity = 1},
                                    {.op = OP_RETURN}};
static Function async_functions[] = {
    {.name = "helper", .is_async = true, .param_count = 1, .params = parameters,
     .ins = helper_code, .ins_count = 3},
    {.name = "read", .is_async = true, .param_count = 1, .params = parameters,
     .ins = read_code, .ins_count = 3}};
static Program async_program = {.count = 2, .functions = async_functions};

static void completion_lifetimes(void)
{
    /* Success, typed error, malformed result, and cancellation at every boundary. */
    for (size_t mode = 0; mode < 8; mode++) {
        const char *error;
        VM vm = {.program = &async_program};
        VMTasks *tasks = vm_tasks_create(&vm, 2, 1, &error);
        assert(tasks && !error);
        Value *input = value_bool(mode == 1);
        Value *result = mode == 2 ? value_size(42) : vm_fake_read_result(mode == 1);
        assert(input && result);
        VMTaskId first = vm_tasks_spawn(tasks, (VMTaskId){0}, &async_functions[0], &input,
                                       UINT64_MAX, &error);
        VMTaskId second = vm_tasks_spawn(tasks, (VMTaskId){0}, &async_functions[1], &input,
                                        UINT64_MAX, &error);
        assert(first.slot && second.slot && !error);
        if (mode == 3) {
            assert(vm_tasks_cancel(tasks, first));
        }
        for (size_t step = 0; step < 8; step++) {
            assert(vm_tasks_pump(tasks, 0, 1));
        }
        VMOperationId operation, other;
        Value *borrowed;
        assert(vm_tasks_pending(tasks, second, &other, &borrowed) && borrowed == input);
        /* Independent work reaches a wait even while the first task is waiting. */
        assert(vm_tasks_status(tasks, second) == TASK_WAITING);
        if (mode != 3) {
            assert(vm_tasks_pending(tasks, first, &operation, &borrowed));
            assert(!vm_tasks_complete(tasks, operation, NULL));
            VMOperationId wrong = operation;
            wrong.task.session++;
            assert(!vm_tasks_complete_read(tasks, wrong, result));
            wrong = operation;
            wrong.generation++;
            assert(!vm_tasks_complete_read(tasks, wrong, result));
            if (mode == 4) {
                assert(vm_tasks_cancel(tasks, first));
            } else {
                assert(vm_tasks_complete_read(tasks, operation, result));
                assert(result->refs == 2);
                assert(!vm_tasks_complete_read(tasks, operation, result));
                assert(!vm_tasks_complete_read(tasks, other, result)); /* full queue */
                if (mode == 5) {
                    assert(vm_tasks_cancel(tasks, first));
                }
                if (mode == 6) { /* destroy with a typed result queued */
                    assert(vm_tasks_destroy(tasks));
                    assert(result->refs == 1 && input->refs == 1);
                    release(result);
                    release(input);
                    continue;
                }
                assert(vm_tasks_pump(tasks, 0, mode == 7 ? 0 : 100));
                if (mode == 7) {
                    assert(vm_tasks_cancel(tasks, first));
                }
            }
            assert(!vm_tasks_complete_read(tasks, operation, result));
        }
        VMTaskStatus expected = mode >= 3 ? TASK_CANCELLED : mode == 2 ? TASK_TRAPPED : TASK_COMPLETED;
        assert(vm_tasks_status(tasks, first) == expected);
        if (expected == TASK_COMPLETED) {
            assert(vm_tasks_result(tasks, first) == result);
        }
        if (mode == 2) {
            assert(strstr(vm_tasks_error(tasks, first), "completion type"));
        }
        Value *valid = vm_fake_read_result(false);
        assert(valid && vm_tasks_complete_read(tasks, other, valid));
        release(valid);
        assert(vm_tasks_pump(tasks, 0, 100));
        assert(vm_tasks_status(tasks, second) == TASK_COMPLETED);
        VMTaskStats stats = vm_tasks_stats(tasks);
        assert(stats.registered == stats.released && !stats.queued);
        assert(vm_tasks_destroy(tasks));
        assert(input->refs == 1 && result->refs == 1);
        release(input);
        release(result);
    }
}

static void completion_schemas(void)
{
    Value *bytes = vm_fake_read_result(false);
    Value *error = vm_fake_read_result(true);
    assert(bytes && error && vm_fake_read_result_valid(bytes) && vm_fake_read_result_valid(error));
    Value *wrong = named_value(V_VARIANT, "Ok", NULL, error->as.named.values, 1);
    assert(wrong && !vm_fake_read_result_valid(wrong));
    release(wrong);
    wrong = named_value(V_VARIANT, "Error", NULL, bytes->as.named.values, 1);
    assert(wrong && !vm_fake_read_result_valid(wrong));
    release(wrong);
    wrong = named_value(V_VARIANT, "Other", NULL, bytes->as.named.values, 1);
    assert(wrong && !vm_fake_read_result_valid(wrong));
    release(wrong);
    wrong = named_value(V_VARIANT, "Ok", NULL, NULL, 0);
    assert(wrong && !vm_fake_read_result_valid(wrong));
    release(wrong);
    release(bytes);
    release(error);
}

static void call_effect_matrix(void)
{
    /* Independently enumerate all 18 caller/callee/activation combinations. */
    for (size_t caller = 0; caller < 3; caller++) {
        for (size_t callee = 0; callee < 3; callee++) {
            for (size_t await = 0; await < 2; await++) {
                bool allowed = await ? caller == 2 && callee == 2
                                     : callee != 2 && (caller == 0 || callee == 1);
                for (size_t indirect = 0; indirect < 2; indirect++) {
                    Instruction target_code[] = {{.op = OP_CALL, .name = "$unit"}, {.op = OP_RETURN}};
                    Instruction code[3] = {0};
                    size_t index = 0;
                    if (indirect) {
                        code[index++] = (Instruction){.op = OP_CONST,
                            .constant = {.tag = 3, .text = "target", .text_length = 6}};
                    }
                    code[index++] = (Instruction){.op = indirect ? (await ? OP_AWAIT_VALUE : OP_CALL_VALUE)
                                                                : (await ? OP_AWAIT_CALL : OP_CALL),
                                                  .name = "target"};
                    code[index++] = (Instruction){.op = OP_RETURN};
                    Function functions[] = {
                        {.name = "main", .pure = caller == 1, .is_async = caller == 2,
                         .ins = code, .ins_count = index},
                        {.name = "target", .pure = callee == 1, .is_async = callee == 2,
                         .ins = target_code, .ins_count = 2}};
                    Program program = {.count = 2, .functions = functions};
                    const char *error = NULL;
                    bool verified = verify(&program, &error);
                    if (!indirect) {
                        assert(verified == allowed);
                    }
                    /* Deliberately execute even rejected direct edges: runtime must also fail closed. */
                    VM vm = {.program = &program};
                    Value *result = execute(&vm, &functions[0], NULL);
                    assert((result != NULL) == allowed);
                    assert((vm.error == NULL) == allowed);
                    release(result);
                }
            }
        }
    }
}

typedef struct {
    VMExecution *execution;
    Value *result;
} AsyncHost;

static bool reentrant_read(void *context, Value *input, const char **error)
{
    AsyncHost *host = context;
    (void)error;
    assert(input->kind == V_BOOL);
    assert(vm_execution_advance(host->execution, 1) == VM_BUSY);
    assert(!vm_execution_complete_read(host->execution, host->result));
    assert(!vm_execution_destroy(host->execution));
    return true;
}

static void read_execution_contracts(void)
{
    const char *error;
    VM vm = {.program = &async_program};
    Value *input = value_bool(false);
    AsyncHost host = {.result = vm_fake_read_result(false)};
    assert(input && host.result);
    host.execution = vm_execution_create_async(&vm, &async_functions[1], &input,
                                               reentrant_read, &host, &error);
    assert(host.execution && !error);
    assert(vm_execution_advance(host.execution, 100) == VM_WAITING);
    assert(vm_execution_advance(host.execution, 0) == VM_WAITING);
    assert(!vm_execution_complete_print(host.execution, NULL));
    assert(vm_execution_complete_read(host.execution, host.result));
    assert(!vm_execution_complete_read(host.execution, host.result));
    assert(vm_execution_advance(host.execution, 100) == VM_COMPLETED);
    assert(vm_execution_result(host.execution) == host.result);
    assert(vm_execution_destroy(host.execution));
    assert(host.result->refs == 1 && input->refs == 1);
    release(host.result);
    release(input);

    input = value_size(1); /* forged Bool argument */
    host.execution = vm_execution_create_async(&vm, &async_functions[1], &input,
                                               NULL, NULL, &error);
    assert(host.execution && vm_execution_advance(host.execution, 100) == VM_TRAPPED);
    assert(strstr(vm.error, "requires Bool"));
    assert(vm_execution_destroy(host.execution));
    release(input);

    Instruction wrong_main[] = {{.op = OP_CONST, .constant = {.tag = 4}}, {.op = OP_RETURN}};
    Function function = {.name = "main", .is_async = true, .ins = wrong_main, .ins_count = 2};
    vm.error = NULL;
    assert(!execute(&vm, &function, NULL) && strstr(vm.error, "return Unit"));
}

static void independent_progress(void)
{
    const char *error;
    VM vm = {.program = &async_program};
    VMTasks *tasks = vm_tasks_create(&vm, 2, 1, &error);
    Value *input = value_bool(false);
    Value *result = vm_fake_read_result(false);
    assert(tasks && input && result);
    VMTaskId first = vm_tasks_spawn(tasks, (VMTaskId){0}, &async_functions[0], &input,
                                   UINT64_MAX, &error);
    VMTaskId second = vm_tasks_spawn(tasks, (VMTaskId){0}, &async_functions[1], &input,
                                    UINT64_MAX, &error);
    assert(first.slot && second.slot && vm_tasks_pump(tasks, 0, 100));
    VMOperationId operation;
    Value *borrowed;
    assert(vm_tasks_pending(tasks, second, &operation, &borrowed));
    assert(vm_tasks_complete_read(tasks, operation, result));
    assert(vm_tasks_pump(tasks, 0, 100));
    assert(vm_tasks_status(tasks, second) == TASK_COMPLETED);
    assert(vm_tasks_status(tasks, first) == TASK_WAITING);
    assert(vm_tasks_destroy(tasks));
    assert(input->refs == 1 && result->refs == 1);
    release(input);
    release(result);
}

void async_contracts(void)
{
    independent_progress();
    read_execution_contracts();
    completion_lifetimes();
    completion_schemas();
    call_effect_matrix();
}
