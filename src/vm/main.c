#include "platform.h"

#include "decode.h"
#include "host.h"
#include "program.h"
#include "value.h"
#include "verify.h"
#include "vm.h"

#include <stdlib.h>
#include <string.h>

/* Native runner entry point: read, decode, verify, then optionally execute. */

static bool read_file(const char *path, uint8_t **data, size_t *length)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        return false;
    }
    if (fseek(f, 0, SEEK_END) || (*length = (size_t)ftell(f)) > MAX_ARTIFACT ||
        fseek(f, 0, SEEK_SET)) {
        fclose(f);
        return false;
    }
    *data = malloc(*length ? *length : 1);
    bool ok = *data && fread(*data, 1, *length, f) == *length;
    fclose(f);
    return ok;
}

int main(int argc, char **argv)
{
    bool session_mode = argc >= 2 && !strcmp(argv[1], "coverage-session");
    const char *ticket = getenv("PANACK_COVERAGE_TICKET");
    const char *artifacts[16], *inventories[16];
    size_t identities = 0;
    int session_arguments = 0;
    if (session_mode) {
        int i = 3;
        while (i + 1 < argc && strcmp(argv[i], "--") && identities < 16) {
            artifacts[identities] = argv[i++];
            inventories[identities++] = argv[i++];
        }
        if (!identities || i >= argc || strcmp(argv[i], "--")) {
            fprintf(stderr, "usage: panack coverage-session DIRECTORY ARTIFACT INVENTORY [ARTIFACT INVENTORY ...] -- [arguments...]\n");
            return 2;
        }
        session_arguments = i + 1;
    }
    bool coverage_mode = argc >= 2 && !strcmp(argv[1], "coverage-run");
    if (!session_mode && (argc < 3 || (coverage_mode ? argc < 5 :
        (strcmp(argv[1], "check") && strcmp(argv[1], "run"))))) {
        fprintf(stderr, "usage: panack-vm <check|run> <program.bc> [arguments...]\n"
                        "       panack-vm coverage-run ARTIFACT.bc INVENTORY RAW [arguments...]\n");
        return 2;
    }
    uint8_t *data = NULL;
    size_t length = 0;
    if (!read_file(session_mode ? artifacts[0] : argv[2], &data, &length)) {
        fprintf(stderr, "error: could not read artifact\n");
        return 1;
    }
    Program p;
    const char *error = NULL;
    bool ok = decode(data, length, &p, &error) && verify(&p, &error);
    if (!ok) {
        fprintf(stderr, "error: %s\n", error);
        free(data);
        free_program(&p);
        return 1;
    }
    if (!strcmp(argv[1], "check")) {
        free(data);
        free_program(&p);
        puts("ok");
        return 0;
    }
    char **environment = NULL;
    size_t env_count = 0;
    if (!snapshot_environment(&environment, &env_count)) {
        fprintf(stderr, "error: native VM out of memory\n");
        free(data);
        free_program(&p);
        return 1;
    }
    uint8_t *inventory = NULL;
    size_t inventory_size = 0, raw_size = 0;
    FILE *raw = NULL;
    VMCoverage *coverage = NULL;
    if (coverage_mode) {
        coverage = vm_coverage_create(&p, VM_COVERAGE_MAX_CELLS, UINT64_MAX);
        if (!coverage || !read_file(argv[3], &inventory, &inventory_size) ||
            !vm_coverage_raw_size(coverage, length, inventory_size, &raw_size) ||
            !(raw = fopen(argv[4], "wbx"))) {
            fprintf(stderr, "error: could not initialise bounded coverage output\n");
            vm_coverage_destroy(coverage);
            free(inventory);
            free(data);
            free_environment(environment, env_count);
            free_program(&p);
            return 1;
        }
    }
    VMCoverageRun *coverage_run = NULL;
    if (session_mode || (ticket && *ticket)) {
        coverage_run = session_mode
            ? vm_coverage_session_create(argv[2], identities, artifacts, inventories, &p)
            : vm_coverage_session_inherit(ticket, data, length, &p);
        if (!coverage_run || !vm_coverage_run_matches(coverage_run, data, length)) {
            vm_coverage_run_close(coverage_run);
            fprintf(stderr, "error: coverage session registration unavailable\n");
            if (raw) fclose(raw);
            vm_coverage_destroy(coverage);
            free(inventory); free(data); free_environment(environment, env_count); free_program(&p);
            return 1;
        }
        if (coverage) {
            vm_coverage_run_close(coverage_run);
            fprintf(stderr, "error: cannot nest coverage-run inside a session\n");
            fclose(raw); vm_coverage_destroy(coverage);
            free(inventory); free(data); free_environment(environment, env_count); free_program(&p);
            return 1;
        }
        coverage = vm_coverage_run_collector(coverage_run);
    }
    if (!coverage_mode) {
        free(data);
        data = NULL;
    }
    int argument_start = session_mode ? session_arguments : coverage_mode ? 5 : 3;
    VM vm = {.program = &p, .argc = argc - argument_start, .argv = argv + argument_start,
             .coverage = coverage, .coverage_run = coverage_run,
             .env_count = env_count, .environment = environment};
    Value *result = execute(&vm, program_function(&p, "main"), NULL);
    int status = result ? 0 : 1;
    if (!result) fprintf(stderr, "error: %s\n", vm.error ? vm.error : "native VM failure");
    if (raw) {
        bool written = vm_coverage_write(coverage, raw, data, length, inventory, inventory_size);
        if (fclose(raw)) written = false;
        if (!written) {
            fprintf(stderr, "error: could not write complete coverage record\n");
            status = 1;
        }
    }
    if (coverage_run) {
        if (!vm_coverage_run_close(coverage_run)) {
            fprintf(stderr, "error: coverage session record incomplete\n");
            status = 1;
        }
    } else vm_coverage_destroy(coverage);
    free(inventory);
    free(data);
    release(result);
    free_environment(environment, env_count);
    free_program(&p);
    return status;
}
