/* Internal host-driven execution contracts; no OS event loop is required. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "buffer.h"
#include "decode.h"
#include "render.h"
#include "verify.h"
#include "vm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void expect_nat(Value *value, size_t expected)
{
    size_t actual;
    assert(value && value->kind == V_NAT && value_index(value, &actual) && actual == expected);
}

static void result_and_argument_lifetimes(void)
{
    char *params[] = {"value"};
    Instruction code[] = {{.op = OP_LOAD, .name = "value"}, {.op = OP_RETURN}};
    Function fn = {.name = "main", .params = params, .param_count = 1,
                   .ins = code, .ins_count = 2};
    Program program = {.functions = &fn, .count = 1};
    VM vm = {.program = &program};
    Value *argument = value_size(17);
    const char *error;
    VMExecution *run = vm_execution_create(&vm, &fn, &argument, NULL, NULL, &error);
    assert(run && !error && argument->refs == 2);
    assert(vm_execution_advance(run, 0) == VM_YIELDED && argument->refs == 2);
    assert(vm_execution_advance(run, 1) == VM_YIELDED && argument->refs == 3);
    assert(!vm_execution_result(run));
    assert(!vm_execution_create(&vm, &fn, &argument, NULL, NULL, &error) && error);
    assert(!vm.error); /* A rejected second start must not poison the first. */
    assert(vm_execution_advance(run, 1) == VM_COMPLETED && argument->refs == 2);
    assert(vm_execution_advance(run, 0) == VM_COMPLETED);
    assert(vm_execution_advance(run, 10) == VM_COMPLETED);
    assert(vm_execution_result(run) == argument);
    assert(vm_execution_destroy(run) && !vm.execution && argument->refs == 1);
    for (size_t budget = 0; budget <= 1; budget++) {
        run = vm_execution_create(&vm, &fn, &argument, NULL, NULL, &error);
        assert(run && vm_execution_advance(run, budget) == VM_YIELDED);
        assert(vm_execution_destroy(run) && argument->refs == 1);
    }
    /* Trap releases all frame references, and a fresh invocation resets error. */
    code[1].op = OP_MATCH_FAIL;
    run = vm_execution_create(&vm, &fn, &argument, NULL, NULL, &error);
    assert(run && vm_execution_advance(run, 1) == VM_YIELDED);
    assert(vm_execution_advance(run, 1) == VM_TRAPPED && vm.error);
    assert(argument->refs == 1 && !vm_execution_result(run));
    assert(vm_execution_advance(run, 1) == VM_TRAPPED);
    assert(vm_execution_destroy(run));
    code[1].op = OP_RETURN;
    run = vm_execution_create(&vm, &fn, &argument, NULL, NULL, &error);
    assert(run && !vm.error && vm_execution_advance(run, 2) == VM_COMPLETED);
    assert(vm_execution_destroy(run));
    release(argument);
}

static void direct_indirect_calls_and_interleaved_sessions(void)
{
    char *params[] = {"value"};
    Instruction identity[] = {{.op = OP_LOAD, .name = "value"}, {.op = OP_RETURN}};
    Instruction outer[] = {
        {.op = OP_CONST, .constant = {.tag = 3, .text = "identity", .text_length = 8}},
        {.op = OP_LOAD, .name = "value"},
        {.op = OP_CALL, .name = "identity", .arity = 1},
        {.op = OP_CALL_VALUE, .arity = 1},
        {.op = OP_RETURN}};
    Function functions[] = {
        {.name = "identity", .params = params, .param_count = 1, .ins = identity, .ins_count = 2},
        {.name = "main", .params = params, .param_count = 1, .ins = outer, .ins_count = 5}};
    Program program = {.functions = functions, .count = 2};
    for (size_t budget = 1; budget <= 11; budget++) {
        VM first = {.program = &program}, second = {.program = &program};
        Value *a = value_size(11), *b = value_size(29);
        const char *error;
        VMExecution *one = vm_execution_create(&first, &functions[1], &a, NULL, NULL, &error);
        VMExecution *two = vm_execution_create(&second, &functions[1], &b, NULL, NULL, &error);
        assert(one && two);
        VMExecutionStatus left = VM_YIELDED, right = VM_YIELDED;
        size_t turns = 0;
        while (left == VM_YIELDED || right == VM_YIELDED) {
            assert(++turns <= 9);
            left = vm_execution_advance(one, budget);
            right = vm_execution_advance(two, 1);
        }
        assert(left == VM_COMPLETED && right == VM_COMPLETED && turns == 9);
        expect_nat(vm_execution_result(one), 11);
        expect_nat(vm_execution_result(two), 29);
        assert(vm_execution_destroy(one) && vm_execution_destroy(two));
        assert(a->refs == 1 && b->refs == 1);
        release(a);
        release(b);
    }
}

static void iterators_survive_yields(void)
{
    char *params[] = {"items"};
    Instruction code[] = {
        {.op = OP_LOAD, .name = "items"},
        {.op = OP_ITER_INIT, .name = "cursor"},
        {.op = OP_ITER_NEXT, .name = "cursor", .name2 = "item", .target = 6},
        {.op = OP_LOAD, .name = "item"},
        {.op = OP_STORE, .name = "last"},
        {.op = OP_JUMP, .target = 2},
        {.op = OP_LOAD, .name = "last"}, {.op = OP_RETURN}};
    Function fn = {.name = "main", .params = params, .param_count = 1,
                   .ins = code, .ins_count = 8};
    Program program = {.functions = &fn, .count = 1};
    Value *first = value_size(3), *last = value_size(8);
    Value *items[] = {first, last};
    Value *array = value_sequence(V_ARRAY, items, 2);
    uint8_t bytes[] = {3, 8};
    Value *byte_values = value_data(V_BYTES, bytes, sizeof(bytes));
    Value *range = value_new(V_RANGE);
    assert(range && pn_big_from_digits(&range->as.range.start, "3", 1, 1));
    assert(pn_big_from_digits(&range->as.range.end, "9", 1, 1));
    Value *iterables[] = {array, byte_values, range};
    for (size_t i = 0; i < 3; i++) {
        VM vm = {.program = &program};
        const char *error;
        VMExecution *run = vm_execution_create(&vm, &fn, &iterables[i], NULL, NULL, &error);
        assert(run);
        size_t steps = 0, expected = i == 2 ? 29 : 13;
        VMExecutionStatus status;
        do {
            status = vm_execution_advance(run, 1);
            assert(++steps <= expected);
        } while (status == VM_YIELDED);
        assert(status == VM_COMPLETED && steps == expected);
        expect_nat(vm_execution_result(run), 8);
        assert(vm_execution_destroy(run) && iterables[i]->refs == 1);
        run = vm_execution_create(&vm, &fn, &iterables[i], NULL, NULL, &error);
        assert(run && vm_execution_advance(run, 3) == VM_YIELDED);
        assert(vm_execution_destroy(run) && iterables[i]->refs == 1);
        release(iterables[i]);
    }
    assert(first->refs == 1 && last->refs == 1);
    release(first);
    release(last);
}

static void deep_calls_and_suspended_cleanup(void)
{
    /* A long call chain forces frame-array growth without native recursion. */
    enum { DEPTH = 300 };
    Function functions[DEPTH];
    Instruction code[DEPTH][3];
    char names[DEPTH][16];
    char *params[] = {"value"};
    for (size_t i = 0; i < DEPTH; i++) {
        snprintf(names[i], sizeof(names[i]), "fn%zu", i);
        code[i][0] = (Instruction){.op = OP_LOAD, .name = "value"};
        code[i][1] = (Instruction){.op = OP_CALL, .name = names[i + (i + 1 < DEPTH)], .arity = 1};
        code[i][2] = (Instruction){.op = OP_RETURN};
        functions[i] = (Function){.name = names[i], .params = params, .param_count = 1,
                                  .ins = code[i], .ins_count = 3};
    }
    code[DEPTH - 1][1].op = OP_RETURN;
    Program program = {.functions = functions, .count = DEPTH};
    Value *value = value_size(42);
    for (size_t stop = 0; stop < 2; stop++) {
        VM vm = {.program = &program};
        const char *error;
        VMExecution *run = vm_execution_create(&vm, functions, &value, NULL, NULL, &error);
        assert(run && vm_execution_advance(run, 2 * (DEPTH - 1)) == VM_YIELDED);
        assert(value->refs == DEPTH + 1);
        if (!stop) {
            assert(vm_execution_advance(run, DEPTH + 1) == VM_COMPLETED);
            expect_nat(vm_execution_result(run), 42);
        }
        assert(vm_execution_destroy(run) && value->refs == 1);
    }
    release(value);
}

typedef struct {
    VM *vm;
    Function *function;
    VMExecution *execution;
    size_t calls;
    bool fail;
} FakeHost;

static Value *fake_host(void *context, const char *name, Value **arguments,
                       size_t arity, const char **error)
{
    FakeHost *host = context;
    assert(!strcmp(name, "print") && arity == 1);
    expect_nat(arguments[0], 23);
    host->calls++;
    assert(vm_execution_advance(host->execution, 1) == VM_BUSY);
    assert(!vm_execution_destroy(host->execution));
    const char *busy;
    assert(!vm_execution_create(host->vm, host->function, arguments, NULL, NULL, &busy));
    assert(busy && !host->vm->error);
    assert(!execute(host->vm, host->function, arguments) && !host->vm->error);
    if (host->fail) {
        *error = "fake host failed";
        return NULL;
    }
    return value_void();
}

static void host_policy_and_exit(void)
{
    char *params[] = {"value"};
    Instruction code[] = {{.op = OP_LOAD, .name = "value"},
                           {.op = OP_CALL, .name = "print", .arity = 1}, {.op = OP_RETURN}};
    Function fn = {.name = "main", .params = params, .param_count = 1,
                   .ins = code, .ins_count = 3};
    Program program = {.functions = &fn, .count = 1};
    Value *value = value_size(23);
    VM vm = {.program = &program};
    const char *error;
    FakeHost host = {.vm = &vm, .function = &fn};
    for (size_t failure = 0; failure < 2; failure++) {
        host.fail = failure != 0;
        host.execution = vm_execution_create(&vm, &fn, &value, fake_host, &host, &error);
        assert(host.execution && vm_execution_advance(host.execution, 1) == VM_YIELDED);
        assert(vm_execution_advance(host.execution, 1) == (failure ? VM_TRAPPED : VM_YIELDED));
        if (!failure) {
            assert(vm_execution_advance(host.execution, 1) == VM_COMPLETED);
        } else {
            assert(!strcmp(vm.error, "fake host failed"));
        }
        assert(vm_execution_destroy(host.execution) && value->refs == 1);
    }
    assert(host.calls == 2);
    /* Never call native exit, even with a host adapter configured. */
    code[1].name = "process_exit";
    VMExecution *run = vm_execution_create(&vm, &fn, &value, fake_host, &host, &error);
    assert(run && vm_execution_advance(run, 2) == VM_EXITED);
    assert(vm_execution_exit_status(run) == 23 && !vm.error && !vm_execution_result(run));
    assert(vm_execution_advance(run, 99) == VM_EXITED && host.calls == 2);
    assert(vm_execution_destroy(run) && value->refs == 1);
    code[1].name = "read_file";
    run = vm_execution_create(&vm, &fn, &value, NULL, NULL, &error);
    assert(run && vm_execution_advance(run, 2) == VM_TRAPPED);
    assert(strstr(vm.error, "host service is unavailable"));
    assert(vm_execution_destroy(run));
    /* Reject before decoding, even when an adapter would accept the service. */
    code[1].name = "run_bytecode";
    run = vm_execution_create(&vm, &fn, &value, fake_host, &host, &error);
    assert(run && vm_execution_advance(run, 2) == VM_TRAPPED && host.calls == 2);
    assert(strstr(vm.error, "nested execution is unavailable"));
    assert(vm_execution_destroy(run));
    release(value);
}

void resumable_contracts(void)
{
    result_and_argument_lifetimes();
    direct_indirect_calls_and_interleaved_sessions();
    iterators_survive_yields();
    deep_calls_and_suspended_cleanup();
    host_policy_and_exit();
}

/* Optional benchmark driver: capture output in a fake host, then write it only
 * after execution. Used to compare different instruction budgets on fixed bytes.
 */
static Value *capture_print(void *context, const char *name, Value **arguments,
                           size_t arity, const char **error)
{
    Buffer *output = context;
    if (strcmp(name, "print") || arity != 1) {
        *error = "unsupported benchmark host service";
        return NULL;
    }
    if (!render(output, arguments[0], false) || !buffer_text(output, "\n")) {
        *error = "native VM out of memory";
        return NULL;
    }
    return value_void();
}

void resumable_artifact(const char *path, size_t budget)
{
    assert(budget > 0);
    FILE *file = fopen(path, "rb");
    assert(file && !fseek(file, 0, SEEK_END));
    long length = ftell(file);
    assert(length > 0 && length < 1048576 && !fseek(file, 0, SEEK_SET));
    uint8_t *bytes = malloc((size_t)length);
    assert(bytes && fread(bytes, 1, (size_t)length, file) == (size_t)length && !fclose(file));
    Program program = {0};
    const char *error = NULL;
    assert(decode(bytes, (size_t)length, &program, &error) && verify(&program, &error));
    free(bytes);
    VM vm = {.program = &program};
    Buffer output = {0};
    VMExecution *run = vm_execution_create(&vm, program_function(&program, "main"), NULL,
                                          capture_print, &output, &error);
    assert(run && !error);
    VMExecutionStatus status;
    do {
        status = vm_execution_advance(run, budget);
    } while (status == VM_YIELDED);
    assert(status == VM_COMPLETED && !vm.error);
    assert(vm_execution_result(run)->kind == V_VOID);
    assert(vm_execution_destroy(run));
    free_program(&program);
    assert(fwrite(output.data, 1, output.length, stdout) == output.length);
    free(output.data);
}
