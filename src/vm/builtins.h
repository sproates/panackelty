#ifndef PANACKELTY_BUILTINS_H
#define PANACKELTY_BUILTINS_H

#include "vm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* One registry supplies arity, purity, and implementation selection. */

typedef struct {
    const char *name;
    uint8_t arity;
    bool pure;
    Value *(*call)(VM *vm, const char *name, Value **arguments);
} Builtin;

Value *builtin_call(VM *vm, const char *name, Value **arguments);
const Builtin *builtin(const char *name);

#endif
