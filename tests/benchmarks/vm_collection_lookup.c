/* Opt-in map/set lookup benchmark; see docs/VM_COLLECTION_INDEX_TRIAL.md. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "builtins.h"
#include "value.h"
#include "vm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
    assert(argc == 4);
    bool map = !strcmp(argv[1], "map");
    assert(map || !strcmp(argv[1], "set"));
    char *end = NULL;
    unsigned long parsed = strtoul(argv[2], &end, 10);
    assert(end && !*end && parsed > 0 && parsed <= 4096);
    size_t count = (size_t)parsed, stride = map ? 2 : 1;
    bool cold = !strcmp(argv[3], "cold"), update = !strcmp(argv[3], "update");
    assert(cold || update || !strcmp(argv[3], "hot"));
    assert(!update || map);
    Value **keys = calloc(count + 1, sizeof(Value *));
    Value **items = calloc(count * stride, sizeof(Value *));
    assert(keys && items);
    Value *payload = value_size(123);
    assert(payload);
    for (size_t i = 0; i <= count; i++) {
        char text[32];
        int length = snprintf(text, sizeof(text), "key-%zu", i);
        keys[i] = value_data(V_STR, (const uint8_t *)text, (size_t)length);
        assert(keys[i]);
        if (i < count) {
            items[i * stride] = keys[i];
            if (map) items[i * stride + 1] = payload;
        }
    }
    ValueKind kind = map ? V_MAP : V_SET;
    Value *collection = value_sequence(kind, items, count * stride);
    assert(collection);
    const Builtin *find = builtin(map ? "map_has" : "set_has");
    const Builtin *put = builtin("map_put");
    VM vm = {0};
    size_t checksum = 0, expected = 0, rounds = cold || update ? 1000 : 50000;
    struct timespec start, finish;
    assert(!clock_gettime(CLOCK_MONOTONIC, &start));
    for (size_t i = 0; i < rounds; i++) {
        if (cold) {
            release(collection);
            collection = value_sequence(kind, items, count * stride);
            assert(collection);
        } else if (update) {
            Value *args[] = {collection, keys[i % count], payload};
            Value *next = put->call(&vm, put->name, args);
            assert(next && !vm.error);
            release(collection);
            collection = next;
        }
        size_t key = i % (count + 1);
        Value *args[] = {collection, keys[key]};
        Value *result = find->call(&vm, find->name, args);
        assert(result && !vm.error && result->kind == V_BOOL);
        assert(result->as.boolean == (key < count));
        checksum += result->as.boolean;
        expected += key < count;
        release(result);
    }
    assert(!clock_gettime(CLOCK_MONOTONIC, &finish));
    assert(checksum == expected);
    printf("%.9f %zu\n", (finish.tv_sec - start.tv_sec) +
           (finish.tv_nsec - start.tv_nsec) / 1e9, checksum);
    release(collection);
    release(payload);
    for (size_t i = 0; i <= count; i++) release(keys[i]);
    free(keys);
    free(items);
}
