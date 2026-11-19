#include "fault_injection.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "numeric.h"
#include "root_read.h"
#include "filesystem_stage.h"
#include "value.h"

#include <assert.h>
#include <errno.h>
#include <dirent.h>
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

static void deferred_stage_cleanup(const char *directory)
{
    VM vm = {0};
    Instruction code[] = {{.op = OP_RETURN}};
    Function function = {.name = "main", .ins = code, .ins_count = 1};
    Program program = {.functions = &function, .count = 1};
    vm.program = &program;
    const char *error = NULL;
    VMExecution *execution = vm_execution_create(&vm, &function, NULL, NULL, NULL, &error);
    assert(execution && !error);
    Value *args[] = {value_data(V_PATH, (const uint8_t *)directory, strlen(directory))};
    Value *result = filesystem_stage_resource_call(&vm, "stage_create", args);
    assert(result && !strcmp(result->as.named.name, "Ok"));
    release(result);
    release(args[0]);
    fault_pause_fstat(true);
    assert(vm_execution_destroy(execution));
    struct timespec delay = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 5000 && !fault_fstat_paused(); i++) nanosleep(&delay, NULL);
    assert(fault_fstat_paused() && filesystem_stage_pending_cleanup() == 1);
    /* Destruction already returned while cleanup is blocked in a host call. */
    fault_pause_fstat(false);
    filesystem_stage_drain_cleanup();
    assert(!filesystem_stage_pending_cleanup());
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
    /* Every allocation boundary during temporary creation must either return
     * a resource or remove it when its result is abandoned. */
    /* No stage table has been created yet: fail cleanup-service startup, then
     * let the creation sweeps prove that a subsequent attempt can recover. */
    fault_reset(0);
    fault_syscall("pthread_create", 1, EIO);
    VM startup = {0};
    Value *refused = filesystem_stage_resource_call(&startup, "stage_create", arguments);
    assert(fault_triggered() && refused && !strcmp(refused->as.named.name, "Error"));
    assert(!startup.filesystem_stages);
    release(refused);
    assert(fault_live() == baseline);
    fault_reset(0);
    const char *temporary[] = {"fs_temp_file", "fs_temp_directory", "stage_create"};
    for (size_t kind = 0; kind < 3; kind++) {
        for (size_t failure = 1; failure < 256; failure++) {
            fault_reset(failure);
            VM temporary_vm = {0};
            pending = filesystem_async_start(&temporary_vm, temporary[kind], arguments);
            if (pending) {
                root_read_wait(pending);
                root_read_destroy(pending);
            }
            filesystem_stage_destroy(&temporary_vm);
            wait_for_workers();
            assert(fault_live() == baseline && fault_descriptors() == 0);
            DIR *entries = opendir(directory);
            assert(entries);
            struct dirent *entry;
            while ((entry = readdir(entries))) {
                assert(strncmp(entry->d_name, "panack-", 7));
                assert(strncmp(entry->d_name, ".panack-stage-", 14));
            }
            assert(closedir(entries) == 0);
            if (!fault_triggered()) break;
            assert(failure < 255);
        }
    }
    fault_reset(0);
    for (size_t i = 0; i < 3; i++) {
        release(arguments[i]);
    }
    assert(fault_live() == 0 && fault_descriptors() == 0);
    deferred_stage_cleanup(directory);
    assert(fault_live() == 0 && fault_descriptors() == 0);
    assert(unlink(filename) == 0 && rmdir(directory) == 0);
}
