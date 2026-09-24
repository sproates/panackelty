#ifndef PANACKELTY_HOST_H
#define PANACKELTY_HOST_H

#include "vm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Host I/O and invocation context; legacy string-path services support compiler bootstrap. */

Value *host_call(VM *vm, const char *name, Value **a);
bool snapshot_environment(char ***out, size_t *count);
void free_environment(char **values, size_t count);

#endif
