#include "platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "coverage_session.h"
#include "tasks.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifndef __wasi__
#include <dirent.h>
#include <unistd.h>
#endif

#ifndef __wasi__
static void fixture(const char *path, const char *text)
{
    FILE *f = fopen(path, "wb");
    assert(f && fwrite(text, 1, strlen(text), f) == strlen(text) && !fclose(f));
}
static unsigned terminal(const char *directory, const char *id)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/raw-%s", directory, id);
    FILE *f = fopen(path, "rb");
    assert(f && !fseek(f, -2, SEEK_END));
    unsigned result = (unsigned)fgetc(f);
    assert(fgetc(f) == 0 && !fclose(f));
    return result;
}
static void clean(const char *directory)
{
    DIR *dir = opendir(directory);
    assert(dir);
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        assert(!unlink(path));
    }
    assert(!closedir(dir) && !rmdir(directory));
}
#endif
void coverage_session_contracts(void)
{
#ifndef __wasi__
    char work[] = "/tmp/panack-coverage-session-XXXXXX";
    assert(mkdtemp(work));
    char artifact[1024], inventory[1024], directory[1024];
    snprintf(artifact, sizeof(artifact), "%s/artifact", work);
    snprintf(inventory, sizeof(inventory), "%s/inventory", work);
    snprintf(directory, sizeof(directory), "%s/session", work);
    fixture(artifact, "artifact"); fixture(inventory, "inventory");
    const char *artifacts[] = {artifact}, *inventories[] = {inventory};
    Instruction code[] = {{.op = OP_CONST, .constant = {.tag = 5}}, {.op = OP_RETURN}};
    Function function = {.name = "main", .ins = code, .ins_count = 2};
    Program program = {.count = 1, .functions = &function};
    VMCoverageRun *root = vm_coverage_session_create(directory, 1, artifacts, inventories, &program);
    assert(root && !vm_coverage_session_create(directory, 1, artifacts, inventories, &program));
    VM vm = {.program = &program, .coverage_run = root, .coverage = vm_coverage_run_collector(root)};
    const char *error;
    VMTasks *tasks = vm_tasks_create(&vm, 3, 2, &error);
    assert(tasks && !error);
    VMTaskId first = vm_tasks_spawn(tasks, (VMTaskId){0}, &function, NULL, UINT64_MAX, &error);
    assert(first.slot);
    VMTaskId nested = vm_tasks_spawn(tasks, first, &function, NULL, UINT64_MAX, &error);
    assert(nested.slot && vm_tasks_pump(tasks, 0, 20));
    assert(vm_tasks_status(tasks, nested) == TASK_COMPLETED);
    assert(vm_tasks_status(tasks, first) == TASK_COMPLETED);
    VMTaskId cancelled = vm_tasks_spawn(tasks, (VMTaskId){0}, &function, NULL, UINT64_MAX, &error);
    assert(cancelled.slot && vm_tasks_cancel(tasks, cancelled));
    assert(vm_tasks_destroy(tasks));
    assert(terminal(directory, "0.1") == 1 && terminal(directory, "0.1.1") == 1 && terminal(directory, "0.2") == 0);
    char *ticket = vm_coverage_process_ticket(root);
    assert(ticket);
    VMCoverageRun *process = vm_coverage_session_inherit(ticket, (const uint8_t *)"artifact", 8, &program);
    assert(process && !vm_coverage_session_inherit(ticket, (const uint8_t *)"artifact", 8, &program));
    free(ticket);
    VM child = {.program = &program, .coverage_run = process, .coverage = vm_coverage_run_collector(process)};
    Value *result = execute(&child, &function, NULL);
    assert(result); release(result);
    assert(vm_coverage_run_close(process) && terminal(directory, "0.3") == 1);
    result = execute(&vm, &function, NULL);
    assert(result && vm_coverage_entries(vm.coverage, 0) == 1); release(result);
    assert(vm_coverage_run_close(root) && terminal(directory, "0") == 1);
    clean(directory); clean(work);
#endif
}

#ifndef __wasi__
static char server_work[128], server_directory[256];
VMCoverageRun *coverage_server_begin(Program *program)
{
    strcpy(server_work, "/tmp/panack-coverage-server-XXXXXX");
    assert(mkdtemp(server_work));
    char artifact[256], inventory[256];
    snprintf(artifact, sizeof(artifact), "%s/artifact", server_work);
    snprintf(inventory, sizeof(inventory), "%s/inventory", server_work);
    snprintf(server_directory, sizeof(server_directory), "%s/session", server_work);
    fixture(artifact, "artifact"); fixture(inventory, "inventory");
    const char *artifacts[] = {artifact}, *inventories[] = {inventory};
    VMCoverageRun *run = vm_coverage_session_create(server_directory, 1, artifacts, inventories, program);
    assert(run);
    return run;
}
static uint64_t read_counter(FILE *file)
{
    uint64_t result = 0;
    for (size_t i = 0; i < 8; i++) { int byte = fgetc(file); assert(byte >= 0); result = (result << 8) | (unsigned)byte; }
    return result;
}
void coverage_server_end(VMCoverageRun *run)
{
    assert(vm_coverage_run_close(run));
    for (size_t i = 1; i <= 3; i++) {
        char id[16], path[512];
        snprintf(id, sizeof(id), "0.%zu", i);
        assert(terminal(server_directory, id) == 1);
        snprintf(path, sizeof(path), "%s/raw-%s", server_directory, id);
        FILE *file = fopen(path, "rb");
        /* Envelope(40+id), raw header(23+8 artifact+9 inventory), main(12+2*32). */
        assert(file && !fseek(file, 40 + (long)strlen(id) + 40, SEEK_SET));
        assert(read_counter(file) == 0); /* Each handler owns a distinct collector. */
        assert(!fseek(file, 68, SEEK_CUR));
        assert(read_counter(file) == 1); /* reply entered once, despite slot reuse. */
        assert(!fseek(file, 4, SEEK_CUR));
        for (size_t pc = 0; pc < 3; pc++) {
            assert(read_counter(file) == 1 && read_counter(file) == 1);
            assert(read_counter(file) == 0 && read_counter(file) == 0);
        }
        assert(!fclose(file));
    }
    clean(server_directory); clean(server_work);
}
#endif
