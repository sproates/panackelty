#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "console_write.h"
#include "numeric.h"
#include "value.h"
#include "verify.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void console_write_contracts(void)
{
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
    assert(console_write_active_count() == 0);
}
