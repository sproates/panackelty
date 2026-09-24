#ifndef PANACKELTY_VM_H
#define PANACKELTY_VM_H

#include "program.h"
#include "value.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Invocation context borrows its program, arguments, and environment snapshot.
 * error points to a static diagnostic; callees never allocate an error string.
 */

typedef struct {
    Program *program;
    int argc;
    char **argv;
    size_t env_count;
    char **environment;
    const char *error;
} VM;

Value *execute(VM *vm, Function *fn, Value **arguments);

#endif
