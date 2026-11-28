#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "console_write.h"
#include "numeric.h"
#include "value.h"
#include "verify.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void checked_console_contracts(void)
{
    char filename[] = "/tmp/panack-checked-console-XXXXXX";
    int output = mkstemp(filename);
    assert(output >= 0 && unlink(filename) == 0);
    fflush(stdout);
    fflush(stderr);
    int saved_out = dup(STDOUT_FILENO), saved_err = dup(STDERR_FILENO);
    assert(saved_out >= 0 && saved_err >= 0);
    assert(dup2(output, STDOUT_FILENO) >= 0 && dup2(output, STDERR_FILENO) >= 0);
    Value *message = value_data(V_STR, (const uint8_t *)"x\0y", 3);
    VM vm = {0};
    for (size_t ordinary = 0; ordinary < 2; ordinary++) {
        for (size_t mode = 0; mode < 4; mode++) {
            Value *flags[] = {message, value_bool(mode >= 2), value_bool(mode % 2)};
            Value *result;
            if (ordinary) result = console_output_call(&vm, "console_output", flags);
            else {
                VMConsoleWrite *write = console_write_start_mode(&vm, message, mode >= 2, mode % 2, false);
                assert(write);
                console_write_wait(write);
                result = retain(console_write_result(write));
                console_write_destroy(write);
            }
            assert(result && !strcmp(result->as.named.name, "Ok"));
            release(result);
            release(flags[1]);
            release(flags[2]);
        }
    }
    assert(dup2(saved_out, STDOUT_FILENO) >= 0 && dup2(saved_err, STDERR_FILENO) >= 0);
    close(saved_out); close(saved_err);
    assert(lseek(output, 0, SEEK_SET) == 0);
    char bytes[29];
    const char expected[] = "x\0yx\0y\nx\0yx\0y\nx\0yx\0y\nx\0yx\0y\n";
    assert(read(output, bytes, sizeof(bytes)) == 28 && !memcmp(bytes, expected, 28));
    close(output);
    release(message);
    /* Raw empty output still validates the descriptor and returns a checked error. */
    saved_out = dup(STDOUT_FILENO);
    assert(saved_out >= 0 && close(STDOUT_FILENO) == 0);
    message = value_data(V_STR, (const uint8_t *)"", 0);
    Value *args[] = {message, value_bool(false), value_bool(false)};
    Value *result = console_output_call(&vm, "console_output", args);
    assert(result && !strcmp(result->as.named.name, "Error"));
    Value *problem = result->as.named.values[0];
    assert(!strcmp((char *)problem->as.named.values[0]->as.bytes.data, "console_write"));
    assert(!strcmp((char *)problem->as.named.values[1]->as.bytes.data, "io_error"));
    release(result);
    assert(dup2(saved_out, STDOUT_FILENO) >= 0); close(saved_out);
    release(args[1]); release(args[2]); release(message);
}

void console_write_contracts(void)
{
    checked_console_contracts();
    char filename[] = "/tmp/panack-console-XXXXXX";
    int output = mkstemp(filename);
    assert(output >= 0 && unlink(filename) == 0);
    fflush(stdout);
    int saved = dup(STDOUT_FILENO);
    assert(saved >= 0 && dup2(output, STDOUT_FILENO) >= 0);
    char *parameters[] = {"message"};
    Instruction main_code[] = {{.op = OP_RETURN}};
    Instruction output_code[] = {{.op = OP_LOAD, .name = "message"},
                                 {.op = OP_AWAIT_CALL, .name = "host_stdout_async", .arity = 1},
                                 {.op = OP_RETURN}};
    Function functions[] = {{.name = "main", .ins = main_code, .ins_count = 1},
                            {.name = "output",
                             .is_async = true,
                             .params = parameters,
                             .param_count = 1,
                             .ins = output_code,
                             .ins_count = 3}};
    Program program = {.functions = functions, .count = 2};
    const char *error = NULL;
    assert(verify(&program, &error));
    Value *message = value_data(V_STR, (const uint8_t *)"hello\0world", 11);
    for (size_t mode = 0; mode < 2; mode++) {
        VM vm = {.program = &program};
        VMExecution *execution =
            vm_execution_create_async(&vm, &functions[1], &message, NULL, NULL, &error);
        assert(execution && !error);
        assert(vm_execution_enable_tcp(execution));
        assert(vm_execution_enable_filesystem(execution));
        if (mode) {
            assert(vm_execution_enable_console(execution));
        }
        VMExecutionStatus status = vm_execution_advance(execution, 10);
        if (!mode) {
            assert(status == VM_TRAPPED && vm.error);
            assert(!strcmp(vm.error, "VM trap: console service is unavailable in this execution"));
        } else {
            assert(status == VM_WAITING);
            Value *fake = vm_fake_read_result(false);
            assert(!vm_execution_complete_read(execution, fake));
            release(fake);
            struct timespec delay = {.tv_nsec = 1000000};
            for (size_t i = 0; i < 5000 && status == VM_WAITING; i++) {
                assert(vm_execution_poll_console(execution));
                status = vm_execution_advance(execution, 10);
                if (status == VM_WAITING) {
                    nanosleep(&delay, NULL);
                }
            }
            assert(status == VM_COMPLETED && !vm.error);
            Value *result = vm_execution_result(execution);
            assert(result && !strcmp(result->as.named.name, "Ok"));
        }
        assert(vm_execution_destroy(execution));
    }
    release(message);
    assert(dup2(saved, STDOUT_FILENO) >= 0);
    close(saved);
    assert(lseek(output, 0, SEEK_SET) == 0);
    char bytes[13];
    assert(read(output, bytes, sizeof(bytes)) == 12 && !memcmp(bytes, "hello\0world\n", 12));
    close(output);

    VM vm = {0};
    message = value_size(0);
    assert(!console_write_start(&vm, message, false) && vm.error);
    release(message);
    uint8_t *large = calloc(16385, 1);
    assert(large);
    message = value_data(V_STR, large, 16385);
    free(large);
    VMConsoleWrite *operation = console_write_start(&vm, message, false);
    assert(operation && console_write_poll(operation));
    Value *result = console_write_result(operation);
    assert(result && !strcmp(result->as.named.name, "Error"));
    Value *code = result->as.named.values[0]->as.named.values[1];
    assert(!strcmp((char *)code->as.bytes.data, "out_of_range"));
    console_write_destroy(operation);
    release(message);
    /* A closed consumer produces a typed error without delivering SIGPIPE to
     * the process. The worker owns a duplicate of the pipe's writing end. */
    int pipe_fds[2];
    assert(pipe(pipe_fds) == 0);
    saved = dup(STDOUT_FILENO);
    assert(saved >= 0 && dup2(pipe_fds[1], STDOUT_FILENO) >= 0);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    message = value_data(V_STR, (const uint8_t *)"closed", 6);
    operation = console_write_start(&vm, message, false);
    assert(operation);
    struct timespec delay = {.tv_nsec = 1000000};
    size_t attempts = 0;
    while (!console_write_poll(operation) && attempts++ < 5000) {
        nanosleep(&delay, NULL);
    }
    assert(console_write_poll(operation));
    result = console_write_result(operation);
    assert(result && !strcmp(result->as.named.name, "Error"));
    Value *problem = result->as.named.values[0];
    assert(!strcmp((char *)problem->as.named.values[1]->as.bytes.data, "io_error"));
    size_t native_code;
    assert(value_index(problem->as.named.values[2], &native_code) && native_code == EPIPE);
    console_write_destroy(operation);
    release(message);
    assert(dup2(saved, STDOUT_FILENO) >= 0);
    close(saved);
    assert(console_write_active_count() == 0);
}
