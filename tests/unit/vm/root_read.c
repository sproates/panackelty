#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "numeric.h"
#include "root_read.h"
#include "value.h"
#include "verify.h"

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static Value *path_value(const char *path)
{
    return value_data(V_PATH, (const uint8_t *)path, strlen(path));
}

static void expect_error(VMRootRead *read, const char *code)
{
    root_read_wait(read);
    Value *result = root_read_result(read);
    assert(!root_read_error(read) && result && result->kind == V_VARIANT);
    assert(!strcmp(result->as.named.name, "Error"));
    Value *problem = result->as.named.values[0];
    assert(!strcmp(problem->as.named.name, "HostError"));
    assert(!strcmp((char *)problem->as.named.values[1]->as.bytes.data, code));
    root_read_destroy(read);
}

static void await_idle(void)
{
    struct timespec pause = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 5000 && root_read_active_count(); i++) {
        nanosleep(&pause, NULL);
    }
    assert(root_read_active_count() == 0);
}

static void execution_contracts(Value **arguments)
{
    char *parameters[] = {"root", "relative", "limit"};
    Instruction code[] = {{.op = OP_LOAD, .name = "root"},
                          {.op = OP_LOAD, .name = "relative"},
                          {.op = OP_LOAD, .name = "limit"},
                          {.op = OP_AWAIT_CALL, .name = "fs_read_beneath_async", .arity = 3},
                          {.op = OP_RETURN}};
    Function function = {.name = "read",
                         .is_async = true,
                         .param_count = 3,
                         .params = parameters,
                         .ins = code,
                         .ins_count = 5};
    Instruction main_code[] = {{.op = OP_RETURN}};
    Function functions[] = {{.name = "main", .ins = main_code, .ins_count = 1}, function};
    Program program = {.functions = functions, .count = 2};
    const char *error = NULL;
    assert(verify(&program, &error));
    for (size_t mode = 0; mode < 3; mode++) {
        VM vm = {.program = &program};
        VMExecution *execution =
            vm_execution_create_async(&vm, &functions[1], arguments, NULL, NULL, &error);
        assert(execution && !error);
        assert(vm_execution_enable_tcp(execution));
        if (mode) {
            assert(vm_execution_enable_filesystem(execution));
        }
        VMExecutionStatus status = vm_execution_advance(execution, 20);
        if (!mode) {
            assert(
                status == VM_TRAPPED && vm.error &&
                !strcmp(vm.error, "VM trap: filesystem service is unavailable in this execution"));
        } else {
            assert(status == VM_WAITING);
            Value *fake = vm_fake_read_result(false);
            assert(!vm_execution_complete_read(execution, fake));
            release(fake);
            if (mode == 1) {
                struct timespec pause = {.tv_nsec = 1000000};
                for (size_t i = 0; i < 5000 && status == VM_WAITING; i++) {
                    assert(vm_execution_poll_filesystem(execution));
                    status = vm_execution_advance(execution, 20);
                    if (status == VM_WAITING) {
                        nanosleep(&pause, NULL);
                    }
                }
                assert(status == VM_COMPLETED && !vm.error);
                Value *result = vm_execution_result(execution);
                assert(result && !strcmp(result->as.named.name, "Ok"));
            }
        }
        assert(vm_execution_destroy(execution));
        assert(!vm.execution);
        await_idle();
    }
}

static void temporary_contracts(const char *directory)
{
    const char *operations[] = {"fs_temp_file", "fs_temp_directory"};
    Value *arguments[] = {path_value(directory)};
    for (size_t kind = 0; kind < 2; kind++) {
        for (size_t mode = 0; mode < 3; mode++) {
            VM vm = {0};
            VMRootRead *read = filesystem_async_start(&vm, operations[kind], arguments);
            assert(read && !vm.error);
            char name[512] = {0};
            Value *owned = NULL;
            if (mode) {
                root_read_wait(read);
                Value *result = root_read_result(read);
                assert(result && !strcmp(result->as.named.name, "Ok"));
                Value *path = result->as.named.values[0];
                assert(path->as.bytes.length < sizeof(name));
                memcpy(name, path->as.bytes.data, path->as.bytes.length);
                assert(access(name, F_OK) == 0);
                if (mode == 2) {
                    owned = retain(result);
                    root_read_accept(read);
                }
            }
            root_read_destroy(read);
            await_idle();
            if (mode == 2) {
                assert(access(name, F_OK) == 0);
                assert(!strcmp(owned->as.named.name, "Ok"));
                release(owned);
                assert((kind ? rmdir(name) : unlink(name)) == 0);
            } else if (mode) {
                assert(access(name, F_OK) != 0);
            }
            /* Immediate cancellation must leave no unreported temporary object. */
            DIR *entries = opendir(directory);
            assert(entries);
            struct dirent *entry;
            while ((entry = readdir(entries))) {
                assert(strncmp(entry->d_name, "panack-", 7));
            }
            assert(closedir(entries) == 0);
        }
    }
    /* An external writer can prevent directory removal. Preserve its contents
     * and report the exact path, rather than silently swallowing cleanup failure. */
    VM vm = {0};
    VMRootRead *read = filesystem_async_start(&vm, "fs_temp_directory", arguments);
    assert(read && !vm.error);
    root_read_wait(read);
    Value *result = root_read_result(read);
    assert(result && !strcmp(result->as.named.name, "Ok"));
    char name[512], child[540];
    snprintf(name, sizeof(name), "%s", result->as.named.values[0]->as.bytes.data);
    snprintf(child, sizeof(child), "%s/occupied", name);
    FILE *file = fopen(child, "wb");
    assert(file && fclose(file) == 0);
    FILE *diagnostic = tmpfile();
    int original_stderr = dup(STDERR_FILENO);
    assert(diagnostic && original_stderr >= 0);
    assert(fflush(stderr) == 0 && dup2(fileno(diagnostic), STDERR_FILENO) >= 0);
    root_read_destroy(read);
    await_idle();
    assert(fflush(stderr) == 0 && dup2(original_stderr, STDERR_FILENO) >= 0);
    assert(close(original_stderr) == 0);
    rewind(diagnostic);
    char message[2048] = {0};
    assert(fread(message, 1, sizeof(message) - 1, diagnostic) > 0);
    assert(strstr(message, name) && strstr(message, "after cancellation") && strstr(message, "errno"));
    assert(fclose(diagnostic) == 0);
    assert(unlink(child) == 0 && rmdir(name) == 0);
    release(arguments[0]);
}

void root_read_contracts(void)
{
    char directory[] = "/tmp/panack-root-read-XXXXXX";
    assert(mkdtemp(directory));
    char filename[256], linkname[256];
    assert(snprintf(filename, sizeof(filename), "%s/file", directory) > 0);
    FILE *file = fopen(filename, "wb");
    assert(file && fwrite("data", 1, 4, file) == 4 && fclose(file) == 0);
    Value *arguments[] = {path_value(directory), path_value("file"), value_size(4)};
    VM vm = {0};
    VMRootRead *read = root_read_start(&vm, arguments, false);
    assert(read && !vm.error);
    /* Inputs can be released while the host operation is still running. */
    for (size_t i = 0; i < 3; i++) {
        release(arguments[i]);
    }
    root_read_wait(read);
    assert(root_read_poll(read) && !root_read_error(read));
    Value *result = root_read_result(read);
    assert(result && !strcmp(result->as.named.name, "Ok"));
    Value *rooted = result->as.named.values[0];
    assert(!strcmp(rooted->as.named.name, "FilesystemRootRead"));
    Value *contents = rooted->as.named.values[1];
    assert(contents->as.bytes.length == 4 && !memcmp(contents->as.bytes.data, "data", 4));
    Value *identity = retain(rooted->as.named.values[0]);
    root_read_destroy(read);

    arguments[0] = path_value(directory);
    arguments[1] = path_value("file");
    arguments[2] = value_size(3);
    expect_error(root_read_start(&vm, arguments, false), "output_limit");
    release(arguments[2]);
    arguments[2] = value_size(4);
    read = root_read_start(&vm, arguments, true);
    root_read_wait(read);
    result = root_read_result(read);
    assert(result && !strcmp(result->as.named.name, "Ok"));
    Value *root_identity = result->as.named.values[0];
    assert(value_equal(identity, root_identity));
    root_read_destroy(read);
    release(identity);

    release(arguments[1]);
    arguments[1] = path_value("../escape");
    expect_error(root_read_start(&vm, arguments, false), "invalid_argument");
    release(arguments[1]);
    arguments[1] = path_value("missing");
    expect_error(root_read_start(&vm, arguments, false), "not_found");
    release(arguments[1]);
    arguments[1] = path_value("link");
    assert(snprintf(linkname, sizeof(linkname), "%s/link", directory) > 0);
    assert(symlink(filename, linkname) == 0);
    expect_error(root_read_start(&vm, arguments, false), "invalid_argument");
    assert(unlink(linkname) == 0);

    release(arguments[1]);
    arguments[1] = path_value("file");
    execution_contracts(arguments);
    VMRootRead *pending[32];
    for (size_t i = 0; i < 32; i++) {
        pending[i] = root_read_start(&vm, arguments, false);
        assert(pending[i]);
    }
    assert(root_read_active_count() == 32);
    expect_error(root_read_start(&vm, arguments, false), "resource_limit");
    for (size_t i = 0; i < 32; i++) {
        root_read_destroy(pending[i]);
    }
    await_idle();
    /* Exercise both completion/destroy orders without retaining VM state. */
    for (size_t i = 0; i < 64; i++) {
        read = root_read_start(&vm, arguments, false);
        assert(read);
        if (i % 2) {
            root_read_wait(read);
        }
        root_read_destroy(read);
        await_idle();
    }
    release(arguments[2]);
    arguments[2] = value_size(16u * 1024u * 1024u + 1);
    expect_error(root_read_start(&vm, arguments, false), "out_of_range");
    release(arguments[0]);
    arguments[0] = value_size(0);
    assert(!root_read_start(&vm, arguments, false) && vm.error);
    for (size_t i = 0; i < 3; i++) {
        release(arguments[i]);
    }
    await_idle();
    /* Completed but unconsumed reads retain their byte reservations. A missing
     * path avoids allocating the reserved output while testing admission. */
    Value *general[] = {path_value("/panack-missing-reservation-test"),
                        value_size(128u * 1024u * 1024u)};
    vm.error = NULL;
    VMRootRead *large[4];
    for (size_t i = 0; i < 4; i++) {
        large[i] = filesystem_async_start(&vm, "fs_read", general);
        assert(large[i] && !vm.error);
        root_read_wait(large[i]);
    }
    expect_error(filesystem_async_start(&vm, "fs_read", general), "resource_limit");
    for (size_t i = 0; i < 4; i++) root_read_destroy(large[i]);
    await_idle();
    expect_error(filesystem_async_start(&vm, "fs_read", general), "not_found");
    release(general[0]);
    release(general[1]);
    assert(!filesystem_async_start(&vm, "not_a_filesystem_operation", NULL) && vm.error);
    temporary_contracts(directory);
    assert(unlink(filename) == 0 && rmdir(directory) == 0);
    await_idle();
}
