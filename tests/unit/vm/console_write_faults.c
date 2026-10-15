#include "fault_injection.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "console_write.h"
#include "value.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

static void await_idle(void)
{
    struct timespec delay = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 5000 && console_write_active_count(); i++) {
        nanosleep(&delay, NULL);
    }
    assert(!console_write_active_count());
}

static void complete_write(Value *message)
{
    VM vm = {0};
    VMConsoleWrite *operation = console_write_start(&vm, message, false);
    if (operation) {
        console_write_wait(operation);
        console_write_destroy(operation);
    }
    await_idle();
}

void console_write_faults(void)
{
    fault_reset(0);
    fflush(stdout);
    int saved = dup(STDOUT_FILENO);
    int sink = open("/dev/null", O_WRONLY);
    assert(saved >= 0 && sink >= 0 && dup2(sink, STDOUT_FILENO) >= 0);
    close(sink);
    Value *message = value_data(V_STR, (const uint8_t *)"hello", 5);
    size_t baseline = fault_live();
    size_t tested = 0;
    for (size_t failure = 1; failure < 100; failure++) {
        fault_reset(failure);
        complete_write(message);
        assert(fault_live() == baseline && fault_descriptors() == 0);
        tested++;
        if (!fault_triggered()) {
            break;
        }
    }
    assert(tested > 3 && tested < 99);
    const char *calls[] = {"write", "fcntl", "pthread_create"};
    for (size_t i = 0; i < 3; i++) {
        fault_reset(0);
        fault_syscall(calls[i], 1, EIO);
        complete_write(message);
        assert(fault_triggered() && fault_live() == baseline && fault_descriptors() == 0);
    }
    fault_reset(0);
    fault_pause_writes(true);
    VM vm = {0};
    VMConsoleWrite *pending = console_write_start(&vm, message, false);
    assert(pending);
    struct timespec delay = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 5000 && !fault_write_paused(); i++) {
        nanosleep(&delay, NULL);
    }
    assert(fault_write_paused() && !console_write_poll(pending));
    console_write_destroy(pending);
    assert(console_write_active_count() == 1 && fault_descriptors() == 1);
    fault_pause_writes(false);
    await_idle();
    assert(fault_live() == baseline && fault_descriptors() == 0);

    VMConsoleWrite *retained[4];
    for (size_t i = 0; i < 4; i++) {
        retained[i] = console_write_start(&vm, message, false);
        assert(retained[i]);
        console_write_wait(retained[i]);
    }
    pending = console_write_start(&vm, message, false);
    assert(pending && console_write_poll(pending));
    Value *result = console_write_result(pending);
    assert(result && !strcmp(result->as.named.name, "Error"));
    Value *code = result->as.named.values[0]->as.named.values[1];
    assert(!strcmp((char *)code->as.bytes.data, "resource_limit"));
    console_write_destroy(pending);
    for (size_t i = 0; i < 4; i++) {
        console_write_destroy(retained[i]);
    }
    await_idle();
    release(message);
    assert(fault_live() == 0 && fault_descriptors() == 0);
    assert(dup2(saved, STDOUT_FILENO) >= 0);
    close(saved);
}
