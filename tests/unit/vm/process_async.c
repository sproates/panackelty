#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "process_async.h"
#include "numeric.h"
#include "verify.h"
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#ifdef PANACK_TEST_INJECT
#include "fault_injection.h"
#endif

static void inputs(Value **args, const char *command)
{
    Value *words[] = {value_data(V_STR, (const uint8_t *)"-c", 2),
                      value_data(V_STR, (const uint8_t *)command, strlen(command))};
    args[0] = value_data(V_PATH, (const uint8_t *)"/bin/sh", 7);
    args[1] = value_sequence(V_ARRAY, words, 2);
    release(words[0]); release(words[1]);
    args[2] = value_data(V_BYTES, (const uint8_t *)"", 0);
    args[3] = value_data(V_PATH, (const uint8_t *)"/", 1);
    args[4] = value_sequence(V_ARRAY, NULL, 0);
    args[5] = value_size(3000000000ULL); args[5]->kind = V_DURATION;
    args[6] = value_size(1024);
    for (size_t i = 0; i < 7; i++) assert(args[i]);
}
static void release_inputs(Value **args) { for (size_t i = 0; i < 7; i++) release(args[i]); }
static void finish(VMProcess *process)
{
    if (process) { process_async_wait(process); process_async_destroy(process); }
    process_async_drain();
}

#ifndef PANACK_TEST_INJECT
static void execution_contracts(void)
{
    char *names[] = {"exe", "args", "input", "cwd", "env", "timeout", "limit"};
    Instruction main_code[] = {{.op = OP_RETURN}};
    Instruction code[9] = {0};
    for (size_t i = 0; i < 7; i++) code[i] = (Instruction){.op = OP_LOAD, .name = names[i]};
    code[7] = (Instruction){.op = OP_AWAIT_CALL, .name = "process_run_isolated_async", .arity = 7};
    code[8] = (Instruction){.op = OP_RETURN};
    Function functions[] = {{.name = "main", .ins = main_code, .ins_count = 1},
                            {.name = "run", .is_async = true, .params = names, .param_count = 7, .ins = code, .ins_count = 9}};
    Program program = {.functions = functions, .count = 2};
    const char *error = NULL;
    assert(verify(&program, &error));
    for (size_t mode = 0; mode < 3; mode++) {
        VM vm = {.program = &program};
        Value *args[7]; inputs(args, "printf ok");
        VMExecution *run = vm_execution_create_async(&vm, &functions[1], args, NULL, NULL, &error);
        assert(run && !error);
        if (mode) assert(vm_execution_enable_process(run));
        VMExecutionStatus status = vm_execution_advance(run, 30);
        release_inputs(args);
        if (!mode) {
            assert(status == VM_TRAPPED && vm.error);
            assert(!strcmp(vm.error, "VM trap: subprocess service is unavailable in this execution"));
        } else {
            assert(status == VM_WAITING);
            assert(!vm_execution_complete_print(run, NULL));
            Value *fake = vm_fake_read_result(false);
            assert(!vm_execution_complete_read(run, fake)); release(fake);
            if (mode == 2) {
                struct timespec delay = {.tv_nsec = 1000000};
                for (size_t i = 0; i < 3000 && status == VM_WAITING; i++) {
                    assert(vm_execution_poll_network(run, 0));
                    status = vm_execution_advance(run, 30);
                    if (status == VM_WAITING) nanosleep(&delay, NULL);
                }
                assert(status == VM_COMPLETED && !vm.error);
                assert(!strcmp(vm_execution_result(run)->as.named.name, "Ok"));
            }
        }
        assert(vm_execution_destroy(run));
        process_async_drain();
    }
    functions[1].is_async = false;
    assert(!verify(&program, &error));
}

void process_async_contracts(void)
{
    execution_contracts();
    VM vm = {0};
    Value *args[7];
    inputs(args, "printf '%s' \"$PANACK_PROCESS_VALUE\"; exit 7");
    char entry[] = "PANACK_PROCESS_VALUE=before";
    char *environment[] = {entry};
    vm.environment = environment; vm.env_count = 1;
    VMProcess *process = process_async_start(&vm, args, false);
    assert(process && !vm.error);
    strcpy(entry, "PANACK_PROCESS_VALUE=after");
    release_inputs(args);
    process_async_wait(process);
    Value *result = process_async_result(process);
    assert(result && !strcmp(result->as.named.name, "Ok"));
    Value *output = result->as.named.values[0];
    size_t status;
    assert(value_index(output->as.named.values[0], &status) && status == 7);
    assert(output->as.named.values[2]->as.bytes.length == 6 &&
           !memcmp(output->as.named.values[2]->as.bytes.data, "before", 6));
    finish(process);

    inputs(args, "exit 0");
    VMProcess *retained[4];
    for (size_t i = 0; i < 4; i++) {
        retained[i] = process_async_start(&vm, args, true);
        assert(retained[i]); process_async_wait(retained[i]);
    }
    process = process_async_start(&vm, args, true);
    assert(process && process_async_poll(process));
    result = process_async_result(process);
    assert(result && !strcmp(result->as.named.name, "Error"));
    assert(!strcmp((char *)result->as.named.values[0]->as.named.values[1]->as.bytes.data, "resource_limit"));
    process_async_destroy(process);
    for (size_t i = 0; i < 4; i++) process_async_destroy(retained[i]);
    process_async_drain(); release_inputs(args);

    char path[] = "/tmp/panack-process-cancel-XXXXXX";
    int file = mkstemp(path); assert(file >= 0); close(file);
    char command[256];
    snprintf(command, sizeof(command), "echo $$ > '%s'; while :; do :; done", path);
    inputs(args, command);
    process = process_async_start(&vm, args, true);
    assert(process);
    release_inputs(args);
    pid_t child = 0;
    struct timespec delay = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 2000 && !child; i++) {
        FILE *input = fopen(path, "r");
        assert(input);
        int pid = 0;
        if (fscanf(input, "%d", &pid) == 1) child = (pid_t)pid;
        fclose(input);
        if (!child) nanosleep(&delay, NULL);
    }
    assert(child > 0 && !process_async_poll(process));
    process_async_destroy(process); // cancellation never waits for the child
    process_async_drain();
    assert(kill(child, 0) == -1 && errno == ESRCH);
    int child_status;
    assert(waitpid(child, &child_status, WNOHANG) == -1 && errno == ECHILD);
    assert(unlink(path) == 0 && process_async_active_count() == 0);

    inputs(args, "while :; do :; done");
    release(args[5]); args[5] = value_size(1000000); args[5]->kind = V_DURATION;
    process = process_async_start(&vm, args, true);
    assert(process); process_async_wait(process);
    result = process_async_result(process);
    assert(result && !strcmp(result->as.named.name, "Error"));
    assert(!strcmp((char *)result->as.named.values[0]->as.named.values[1]->as.bytes.data, "timeout"));
    finish(process); release_inputs(args);
}
#else
void process_async_faults(void)
{
    fault_reset(0);
    Value *args[7]; inputs(args, "printf ok");
    size_t baseline = fault_live();
    size_t attempted;
    for (attempted = 1; attempted < 250; attempted++) {
        fault_reset(attempted);
        VM vm = {0};
        VMProcess *process = process_async_start(&vm, args, true);
        finish(process);
        assert(fault_live() == baseline && !fault_descriptors() && !fault_children());
        if (!fault_triggered()) break;
    }
    assert(attempted > 10 && attempted < 250);
    const char *calls[] = {"pthread_create", "pipe", "fork", "poll", "read"};
    for (size_t i = 0; i < sizeof(calls) / sizeof(*calls); i++) {
        fault_reset(0); fault_syscall(calls[i], 1, EIO);
        VM vm = {0};
        finish(process_async_start(&vm, args, true));
        assert(fault_triggered() && fault_live() == baseline && !fault_descriptors() && !fault_children());
    }
    fault_reset(0);
    VM vm = {0};
    VMProcess *process = process_async_start(&vm, args, true);
    assert(process);
    process_async_destroy(process); process_async_drain();
    assert(fault_live() == baseline && !fault_descriptors() && !fault_children());
    release_inputs(args); fault_reset(0);
}
#endif
