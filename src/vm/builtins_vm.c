#include "builtins_internal.h"
#include "decode.h"
#include "program.h"
#include "value.h"
#include "verify.h"
#include "vm.h"

#include <stdlib.h>
#include <string.h>

/* Nested bytecode execution shares host context but owns its decoded program. */

Value *builtins_vm_call(VM *vm, const char *name, Value **a)
{
    if (!strcmp(name, "run_bytecode")) {
        REQUIRE(a[0]->kind == V_BYTES, "VM trap: run_bytecode requires Bytes");
        Program nested;
        const char *error = NULL;
        if (!decode(a[0]->as.bytes.data, a[0]->as.bytes.length, &nested, &error) ||
            !verify(&nested, &error)) {
            vm->error = error;
            free_program(&nested);
            return NULL;
        }
        VM child = *vm;
        child.program = &nested;
        child.error = NULL;
        Value *result = execute(&child, program_function(&nested, "main"), NULL);
        if (!result) {
            vm->error = child.error;
            free_program(&nested);
            return NULL;
        }
        release(result);
        free_program(&nested);
        return value_void();
    }

    if (!strcmp(name, "run_bytecode_args")) {
        REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_ARRAY,
                "VM trap: run_bytecode_args requires Bytes and [Str]");
        Program nested;
        const char *error = NULL;
        if (!decode(a[0]->as.bytes.data, a[0]->as.bytes.length, &nested, &error) ||
            !verify(&nested, &error)) {
            vm->error = error;
            free_program(&nested);
            return NULL;
        }
        char **arguments = calloc(a[1]->as.sequence.count, sizeof(char *));
        if (a[1]->as.sequence.count && !arguments) {
            free_program(&nested);
            return NULL;
        }
        for (size_t i = 0; i < a[1]->as.sequence.count; i++) {
            Value *argument = a[1]->as.sequence.items[i];
            if (argument->kind != V_STR) {
                free(arguments);
                free_program(&nested);
                vm->error = "VM trap: run_bytecode_args requires [Str]";
                return NULL;
            }
            arguments[i] = (char *)argument->as.bytes.data;
        }
        VM child = *vm;
        child.program = &nested;
        child.argc = (int)a[1]->as.sequence.count;
        child.argv = arguments;
        child.error = NULL;
        Value *result = execute(&child, program_function(&nested, "main"), NULL);
        free(arguments);
        if (!result) {
            vm->error = child.error;
            free_program(&nested);
            return NULL;
        }
        release(result);
        free_program(&nested);
        return value_void();
    }
    vm->error = "VM trap: unknown builtin";
    return NULL;
}
