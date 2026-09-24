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
    if (argc < 3 || (strcmp(argv[1], "check") && strcmp(argv[1], "run"))) {
        fprintf(stderr, "usage: panack-vm <check|run> <program.bc> [arguments...]\n");
        return 2;
    }
    uint8_t *data = NULL;
    size_t length = 0;
    if (!read_file(argv[2], &data, &length)) {
        fprintf(stderr, "error: could not read artifact\n");
        return 1;
    }
    Program p;
    const char *error = NULL;
    bool ok = decode(data, length, &p, &error) && verify(&p, &error);
    free(data);
    if (!ok) {
        fprintf(stderr, "error: %s\n", error);
        free_program(&p);
        return 1;
    }
    if (!strcmp(argv[1], "check")) {
        free_program(&p);
        puts("ok");
        return 0;
    }
    char **environment = NULL;
    size_t env_count = 0;
    if (!snapshot_environment(&environment, &env_count)) {
        fprintf(stderr, "error: native VM out of memory\n");
        free_program(&p);
        return 1;
    }
    VM vm = {&p, argc - 3, argv + 3, env_count, environment, NULL};
    Value *result = execute(&vm, program_function(&p, "main"), NULL);
    if (!result) {
        fprintf(stderr, "error: %s\n", vm.error ? vm.error : "native VM failure");
        free_environment(environment, env_count);
        free_program(&p);
        return 1;
    }
    release(result);
    free_environment(environment, env_count);
    free_program(&p);
    return 0;
}
