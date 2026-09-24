#include "builtins_internal.h"
#include "value.h"

#include <stdlib.h>
#include <string.h>

/* Persistent collection operations retain children without mutating their inputs. */

Value *builtins_collections_call(VM *vm, const char *name, Value **a)
{
    if (!strcmp(name, "append")) {
        REQUIRE(a[0]->kind == V_ARRAY, "VM trap: append requires Array");
        size_t n = a[0]->as.sequence.count;
        Value **items = malloc((n + 1) * sizeof(Value *));
        if (!items) {
            return NULL;
        }
        memcpy(items, a[0]->as.sequence.items, n * sizeof(Value *));
        items[n] = a[1];
        Value *v = value_sequence(V_ARRAY, items, n + 1);
        free(items);
        return v;
    }

    if (!strcmp(name, "map") || !strcmp(name, "set") || !strcmp(name, "bytes")) {
        return !strcmp(name, "bytes")
                   ? value_data(V_BYTES, NULL, 0)
                   : value_sequence(!strcmp(name, "map") ? V_MAP : V_SET, NULL, 0);
    }

    if (!strcmp(name, "$method_has")) {
        REQUIRE(a[0]->kind == V_MAP || a[0]->kind == V_SET, "VM trap: has requires Map or Set");
    }

    if (!strcmp(name, "map_has") || !strcmp(name, "map_get") || !strcmp(name, "$method_get") ||
        (!strcmp(name, "$method_has") && a[0]->kind == V_MAP)) {
        REQUIRE(a[0]->kind == V_MAP, "VM trap: map operation requires Map");
        for (size_t i = a[0]->as.sequence.count; i >= 2; i -= 2) {
            if (value_equal(a[0]->as.sequence.items[i - 2], a[1])) {
                return !strcmp(name, "map_has") || !strcmp(name, "$method_has")
                           ? value_bool(true)
                           : retain(a[0]->as.sequence.items[i - 1]);
            }
        }
        if (!strcmp(name, "map_has") || !strcmp(name, "$method_has")) {
            return value_bool(false);
        }
        vm->error = "VM trap: map key was not found";
        return NULL;
    }

    if (!strcmp(name, "map_put") || !strcmp(name, "$method_put")) {
        REQUIRE(a[0]->kind == V_MAP, "VM trap: map_put requires Map");
        size_t old = a[0]->as.sequence.count, count = 0;
        Value **items = malloc((old + 2) * sizeof(Value *));
        if (!items) {
            return NULL;
        }
        for (size_t i = 0; i < old; i += 2) {
            if (!value_equal(a[0]->as.sequence.items[i], a[1])) {
                items[count++] = a[0]->as.sequence.items[i];
                items[count++] = a[0]->as.sequence.items[i + 1];
            }
        }
        items[count++] = a[1];
        items[count++] = a[2];
        Value *v = value_sequence(V_MAP, items, count);
        free(items);
        return v;
    }

    if (!strcmp(name, "set_has") || !strcmp(name, "$method_has")) {
        REQUIRE(a[0]->kind == V_SET, "VM trap: set_has requires Set");
        for (size_t i = 0; i < a[0]->as.sequence.count; i++) {
            if (value_equal(a[0]->as.sequence.items[i], a[1])) {
                return value_bool(true);
            }
        }
        return value_bool(false);
    }

    if (!strcmp(name, "set_add") || !strcmp(name, "$method_add")) {
        REQUIRE(a[0]->kind == V_SET, "VM trap: set_add requires Set");
        size_t n = a[0]->as.sequence.count;
        for (size_t i = 0; i < n; i++) {
            if (value_equal(a[0]->as.sequence.items[i], a[1])) {
                return retain(a[0]);
            }
        }
        Value **items = malloc((n + 1) * sizeof(Value *));
        if (!items) {
            return NULL;
        }
        memcpy(items, a[0]->as.sequence.items, n * sizeof(Value *));
        items[n] = a[1];
        Value *v = value_sequence(V_SET, items, n + 1);
        free(items);
        return v;
    }
    vm->error = "VM trap: unknown builtin";
    return NULL;
}
