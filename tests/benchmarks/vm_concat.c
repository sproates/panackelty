/* Opt-in operation benchmark; see docs/VM_CONCAT_TRIAL.md. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "builtins.h"
#include "numeric.h"
#include "value.h"
#include "vm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
    assert(argc == 2);
    uint8_t data[4096];
    memset(data, 'x', sizeof(data));
    Value *item = value_size(7), *items[512];
    assert(item);
    for (size_t i = 0; i < 512; i++) {
        items[i] = item;
    }
    bool array = !strcmp(argv[1], "array"), str = !strcmp(argv[1], "string");
    bool append = !strcmp(argv[1], "append");
    assert(array || str || append || !strcmp(argv[1], "bytes"));
    Value *a = array ? value_sequence(V_ARRAY, items, 512)
                     : value_data(str ? V_STR : V_BYTES, data, sizeof(data));
    assert(a);
    Value *args[] = {a, append ? item : a};
    const Builtin *fn = str ? NULL : builtin(array ? "concat" : append ? "byte_append" : "bytes_concat");
    VM vm = {0};
    const char *error = NULL;
    size_t sum = 0;
    struct timespec start, end;
    assert(clock_gettime(CLOCK_MONOTONIC, &start) == 0);
    for (size_t i = 0; i < 200000; i++) {
        Value *v = str ? binary_value(0, a, a, &error) : fn->call(&vm, fn->name, args);
        assert(v && !error && !vm.error);
        size_t n = array ? v->as.sequence.count : v->as.bytes.length;
        assert(n == (array ? 1024 : append ? 4097 : 8192));
        sum += n;
        release(v);
    }
    assert(clock_gettime(CLOCK_MONOTONIC, &end) == 0);
    printf("%s %.9f %zu\n", argv[1],
           (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9, sum);
    release(a);
    release(item);
}
