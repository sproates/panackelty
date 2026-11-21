#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "console_read.h"
#include "value.h"
#include "verify.h"
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void idle(void)
{
    struct timespec delay = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 5000 && console_read_active_count(); i++) nanosleep(&delay, NULL);
    assert(!console_read_active_count());
}
static Value *read_value(VM *vm, size_t count)
{
    Value *limit = value_size(count);
    Value *result = console_input_call(vm, "console_input", &limit);
    release(limit);
    assert(result && !vm->error);
    return result;
}
static void chunk(Value *result, const char *expected, size_t size)
{
    assert(result && !strcmp(result->as.named.name, "Ok"));
    Value *option = result->as.named.values[0];
    assert(!strcmp(option->as.named.name, "Some"));
    Value *bytes = option->as.named.values[0];
    assert(bytes->kind == V_BYTES && bytes->as.bytes.length == size);
    assert(!memcmp(bytes->as.bytes.data, expected, size));
    release(result);
}
static void end(VM *vm)
{
    Value *result = read_value(vm, 8);
    assert(!strcmp(result->as.named.name, "Ok") && !strcmp(result->as.named.values[0]->as.named.name, "None"));
    release(result);
}
void console_read_contracts(void)
{
    int saved = dup(STDIN_FILENO), pipefd[2];
    assert(saved >= 0);
    VM vm = {0};
    for (size_t limit = 0; limit <= 65537; limit += 65537) {
        Value *result = read_value(&vm, limit);
        assert(!strcmp(result->as.named.name, "Error")); release(result);
    }
    assert(pipe(pipefd) == 0 && dup2(pipefd[0], STDIN_FILENO) >= 0); close(pipefd[0]);
    assert(write(pipefd[1], "abcdef", 6) == 6); close(pipefd[1]);
    Value *limit = value_size(4);
    VMConsoleRead *operation = console_read_start(&vm, limit);
    assert(operation); console_read_wait(operation);
    chunk(console_read_result(operation), "abcd", 4);
    // Inspecting a result does not acknowledge it. Destroy must retain the bytes.
    console_read_destroy(operation);
    chunk(read_value(&vm, 2), "ab", 2);
    chunk(read_value(&vm, 4), "cd", 2);
    chunk(read_value(&vm, 4), "ef", 2);
    end(&vm);
    release(limit);

    assert(pipe(pipefd) == 0 && dup2(pipefd[0], STDIN_FILENO) >= 0); close(pipefd[0]);
    limit = value_size(8);
    operation = console_read_start(&vm, limit);
    assert(operation && !console_read_poll(operation));
    Value *busy = read_value(&vm, 1);
    assert(!strcmp(busy->as.named.name, "Error"));
    Value *code = busy->as.named.values[0]->as.named.values[1];
    assert(code->as.bytes.length == 4 && !memcmp(code->as.bytes.data, "busy", 4)); release(busy);
    console_read_destroy(operation);
    idle();
    assert(write(pipefd[1], "xyz", 3) == 3); close(pipefd[1]);
    chunk(read_value(&vm, 8), "xyz", 3); end(&vm);
    release(limit);

    for (size_t i = 0; i < 20; i++) {
        assert(pipe(pipefd) == 0 && dup2(pipefd[0], STDIN_FILENO) >= 0); close(pipefd[0]);
        limit = value_size(8);
        operation = console_read_start(&vm, limit);
        assert(operation);
        assert(write(pipefd[1], "race", 4) == 4); close(pipefd[1]);
        console_read_destroy(operation); idle();
        chunk(read_value(&vm, 8), "race", 4); end(&vm); release(limit);
    }
    assert(pipe(pipefd) == 0 && dup2(pipefd[0], STDIN_FILENO) >= 0); close(pipefd[0]);
    assert(write(pipefd[1], "a\r\nb\nc", 6) == 6); close(pipefd[1]);
    Value *line = console_legacy_read_line(&vm);
    assert(line && line->as.bytes.length == 1 && line->as.bytes.data[0] == 'a'); release(line);
    chunk(read_value(&vm, 1), "b", 1);
    line = console_legacy_read_line(&vm);
    assert(line && line->as.bytes.length == 0); release(line);
    chunk(read_value(&vm, 1), "c", 1); end(&vm);

    char *parameters[] = {"limit"};
    Instruction main_code[] = {{.op = OP_RETURN}};
    Instruction read_code[] = {{.op = OP_LOAD, .name = "limit"},
        {.op = OP_AWAIT_CALL, .name = "console_input_async", .arity = 1}, {.op = OP_RETURN}};
    Function functions[] = {{.name = "main", .ins = main_code, .ins_count = 1},
        {.name = "reader", .is_async = true, .params = parameters, .param_count = 1, .ins = read_code, .ins_count = 3}};
    Program program = {.functions = functions, .count = 2};
    const char *error = NULL;
    assert(verify(&program, &error));
    for (size_t mode = 0; mode < 3; mode++) {
        assert(pipe(pipefd) == 0 && dup2(pipefd[0], STDIN_FILENO) >= 0); close(pipefd[0]);
        VM embedded = {.program = &program};
        limit = value_size(4);
        VMExecution *execution = vm_execution_create_async(&embedded, &functions[1], &limit, NULL, NULL, &error);
        assert(execution && !error && vm_execution_enable_console(execution));
        if (mode) assert(vm_execution_enable_console_input(execution));
        VMExecutionStatus status = vm_execution_advance(execution, 10);
        if (!mode) {
            assert(status == VM_TRAPPED && strstr(embedded.error, "console input is unavailable"));
            close(pipefd[1]);
        } else {
            assert(status == VM_WAITING);
            assert(!vm_execution_complete_print(execution, NULL));
            assert(write(pipefd[1], "done", 4) == 4); close(pipefd[1]);
            if (mode == 1) {
                struct timespec delay = {.tv_nsec = 1000000};
                for (size_t i = 0; i < 5000 && status == VM_WAITING; i++) {
                    assert(vm_execution_poll_console_input(execution));
                    status = vm_execution_advance(execution, 10);
                    if (status == VM_WAITING) nanosleep(&delay, NULL);
                }
                assert(status == VM_COMPLETED);
                chunk(retain(vm_execution_result(execution)), "done", 4);
            }
        }
        assert(vm_execution_destroy(execution)); idle();
        if (mode == 2) chunk(read_value(&vm, 4), "done", 4);
        end(&vm); release(limit);
    }
    assert(dup2(saved, STDIN_FILENO) >= 0); close(saved);
    assert(!console_read_active_count());
}
