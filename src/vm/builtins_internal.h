#ifndef PANACKELTY_BUILTINS_INTERNAL_H
#define PANACKELTY_BUILTINS_INTERNAL_H

#include "vm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Internal builtin implementation contracts. */

/* Each handler borrows arguments and returns an owned value or NULL. */
#define REQUIRE(condition, message)                                                                \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            vm->error = (message);                                                                 \
            return NULL;                                                                           \
        }                                                                                          \
    } while (0)
#define REQUIRE_PATH(value, message)                                                               \
    do {                                                                                           \
        REQUIRE((value)->kind == V_STR, (message));                                                \
        REQUIRE(!memchr((value)->as.bytes.data, 0, (value)->as.bytes.length),                      \
                "VM trap: path contains NUL byte");                                                \
    } while (0)

Value *builtins_collections_call(VM *vm, const char *name, Value **a);
Value *builtins_numeric_call(VM *vm, const char *name, Value **a);
Value *builtins_text_call(VM *vm, const char *name, Value **a);
Value *builtins_vm_call(VM *vm, const char *name, Value **a);

#endif
