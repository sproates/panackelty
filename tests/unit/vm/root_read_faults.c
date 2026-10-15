#include "fault_injection.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "numeric.h"
#include "root_read.h"
#include "value.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

static void wait_for_workers(void)
{
    struct timespec pause = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 5000 && root_read_active_count(); i++) {
        nanosleep(&pause, NULL);
    }
    assert(!root_read_active_count());
}

static void complete_operation(Value **arguments)
{
    VM vm = {0};
    VMRootRead *read = root_read_start(&vm, arguments, false);
    if (read) {
        root_read_wait(read);
        root_read_destroy(read);
    }
    wait_for_workers();
}

void root_read_faults(void)
{
    fault_reset(0);
    char directory[] = "/tmp/panack-root-faults-XXXXXX";
    assert(mkdtemp(directory));
    char filename[256];
    assert(snprintf(filename, sizeof(filename), "%s/file", directory) > 0);
    FILE *file = fopen(filename, "wb");
    assert(file && fwrite("data", 1, 4, file) == 4 && fclose(file) == 0);
    Value *arguments[] = {value_data(V_PATH, (const uint8_t *)directory, strlen(directory)),
                          value_data(V_PATH, (const uint8_t *)"file", 4), value_size(4)};
    size_t baseline = fault_live();
    size_t tested = 0;
    for (size_t failure = 1; failure < 256; failure++) {
        fault_reset(failure);
        complete_operation(arguments);
        assert(fault_live() == baseline && fault_descriptors() == 0);
        tested++;
        if (!fault_triggered()) {
            break;
        }
    }
    assert(tested > 10 && tested < 255);
    const char *calls[] = {"open", "fstat", "read", "pthread_create"};
    for (size_t i = 0; i < sizeof(calls) / sizeof(calls[0]); i++) {
        fault_reset(0);
        fault_syscall(calls[i], 1, EIO);
        complete_operation(arguments);
        assert(fault_triggered());
        assert(fault_live() == baseline && fault_descriptors() == 0);
    }
    fault_reset(0);
    fault_pause_reads(true);
    VM vm = {0};
    VMRootRead *pending = root_read_start(&vm, arguments, false);
    assert(pending);
    struct timespec pause = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 5000 && !fault_read_paused(); i++) {
        nanosleep(&pause, NULL);
    }
    assert(fault_read_paused() && !root_read_poll(pending));
    root_read_destroy(pending);
    assert(root_read_active_count() == 1 && fault_descriptors() > 0);
    fault_pause_reads(false);
    wait_for_workers();
    assert(fault_live() == baseline && fault_descriptors() == 0);
    for (size_t i = 0; i < 3; i++) {
        release(arguments[i]);
    }
    assert(fault_live() == 0 && fault_descriptors() == 0);
    assert(unlink(filename) == 0 && rmdir(directory) == 0);
}
