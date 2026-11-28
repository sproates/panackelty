#include "platform.h"
#include "digest.h"
#include "gzip_stream.h"
#include <stdlib.h>
#include <time.h>
#include <zlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static Value *payload(Value *result, const char *variant)
{
    assert(result && result->kind == V_VARIANT);
    assert(!strcmp(result->as.named.name, variant));
    assert(result->as.named.count == 1);
    return result->as.named.values[0];
}

static void typed_digest(void)
{
    VM vm = {0}, other = {0};
    Value *opened = digest_resource_call(&vm, "sha256_create", NULL);
    Value *foreign = digest_resource_call(&other, "sha256_create", NULL);
    Value *handle = payload(opened, "Ok");
    assert(handle->kind == V_SHA256_CONTEXT);
    Value *data = value_data(V_BYTES, (const uint8_t *)"abc", 3);
    Value *args[] = {handle, data};
    Value *result = digest_resource_call(&other, "sha256_update", args);
    payload(result, "Error"); release(result);
    Value *number = value_size(1), *forged[] = {number};
    result = digest_call(&vm, "digest_close", forged);
    payload(result, "Error"); release(result); release(number);
    result = digest_resource_call(&vm, "sha256_update", args);
    payload(result, "Ok"); release(result);
    result = digest_resource_call(&vm, "sha256_finish", args);
    Value *hash = digest_hash_call(&vm, "sha256_hash", &data);
    assert(hash && value_equal(hash, payload(result, "Ok")));
    release(result); release(hash);
    result = digest_resource_call(&vm, "sha256_finish", args);
    payload(result, "Error"); release(result);
    digest_destroy(&vm); digest_destroy(&other);
    release(opened); release(foreign); release(data);
}

static size_t compressed_hello(uint8_t *output, size_t capacity)
{
    z_stream encoder = {0};
    assert(deflateInit2(&encoder, 6, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) == Z_OK);
    encoder.next_in = (uint8_t *)"hello"; encoder.avail_in = 5;
    encoder.next_out = output; encoder.avail_out = (uInt)capacity;
    assert(deflate(&encoder, Z_FINISH) == Z_STREAM_END);
    size_t length = capacity - encoder.avail_out;
    deflateEnd(&encoder);
    return length;
}

static void typed_gzip(void)
{
    uint8_t compressed[128];
    size_t length = compressed_hello(compressed, sizeof(compressed));
    for (unsigned mode = 0; mode < 4; mode++) {
        VM vm = {0}, other = {0};
        Value *limit = value_size(mode == 2 ? 4 : 100);
        Value *opened = gzip_resource_call(&vm, "gzip_decoder_create", &limit);
        Value *foreign = gzip_resource_call(&other, "gzip_decoder_create", &limit);
        Value *handle = payload(opened, "Ok");
        assert(handle->kind == V_GZIP_DECODER);
        if (mode == 3) compressed[length - 8] ^= 1;
        Value *data = value_data(V_BYTES, compressed, mode == 1 ? length - 1 : length);
        Value *args[] = {handle, data, value_bool(true)};
        Value *result = gzip_resource_call(&other, "gzip_decoder_decode", args);
        payload(result, "Error"); release(result);
        Value *number = value_size(1);
        result = gzip_stream_call(&vm, "gzip_close", &number);
        payload(result, "Error"); release(result); release(number);
        result = gzip_resource_call(&vm, "gzip_decoder_decode", args);
        Value *chunk = payload(result, mode ? "Error" : "Ok");
        if (!mode) {
            assert(chunk->as.named.values[1]->as.bytes.length == 5);
            assert(!memcmp(chunk->as.named.values[1]->as.bytes.data, "hello", 5));
            assert(chunk->as.named.values[2]->as.boolean);
        }
        release(result);
        result = gzip_resource_call(&vm, "gzip_decoder_close", args);
        payload(result, "Ok"); release(result);
        result = gzip_resource_call(&vm, "gzip_decoder_decode", args);
        payload(result, "Error"); release(result);
        gzip_stream_destroy(&vm); gzip_stream_destroy(&other);
        release(opened); release(foreign); release(limit); release(data);
    }
}

static void typed_gzip_boundaries(void)
{
    const size_t total = 200000;
    uint8_t *plain = malloc(total), compressed[1024];
    assert(plain); memset(plain, 'A', total);
    z_stream encoder = {0};
    assert(deflateInit2(&encoder, 6, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) == Z_OK);
    encoder.next_in = plain; encoder.avail_in = (uInt)total;
    encoder.next_out = compressed; encoder.avail_out = sizeof(compressed);
    assert(deflate(&encoder, Z_FINISH) == Z_STREAM_END);
    size_t length = sizeof(compressed) - encoder.avail_out;
    deflateEnd(&encoder); free(plain);
    VM vm = {0};
    Value *limit = value_size(total);
    Value *opened = gzip_resource_call(&vm, "gzip_decoder_create", &limit);
    size_t consumed = 0, produced = 0, steps = 0;
    bool finished = false;
    while (!finished) {
        Value *data = value_data(V_BYTES, compressed + consumed, length - consumed);
        Value *args[] = {payload(opened, "Ok"), data, value_bool(true)};
        Value *result = gzip_resource_call(&vm, "gzip_decoder_decode_async", args);
        Value *chunk = payload(result, "Ok");
        size_t took;
        assert(value_index(chunk->as.named.values[0], &took));
        consumed += took;
        Value *output = chunk->as.named.values[1];
        assert(output->as.bytes.length <= 65536);
        for (size_t i = 0; i < output->as.bytes.length; i++) assert(output->as.bytes.data[i] == 'A');
        produced += output->as.bytes.length;
        finished = chunk->as.named.values[2]->as.boolean;
        assert(++steps < 10);
        release(result); release(data);
    }
    assert(consumed == length && produced == total && steps == 4);
    gzip_stream_destroy(&vm);
    release(opened); release(limit);
}

static void async_step_yields(bool gzip, unsigned cancel)
{
    uint8_t compressed[128];
    size_t size = compressed_hello(compressed, sizeof(compressed));
    char *params[] = {"resource", "data", "finish"};
    Instruction code[] = {{.op = OP_LOAD, .name = "resource"}, {.op = OP_LOAD, .name = "data"},
        {.op = OP_LOAD, .name = "finish"},
        {.op = OP_AWAIT_CALL, .name = "gzip_decoder_decode_async", .arity = 3}, {.op = OP_RETURN}};
    if (!gzip) {
        code[2] = (Instruction){.op = OP_AWAIT_CALL, .name = "sha256_update_async", .arity = 2};
        code[3] = (Instruction){.op = OP_RETURN};
    }
    Function fn = {.name = "step", .is_async = true, .params = params, .param_count = gzip ? 3 : 2,
        .ins = code, .ins_count = gzip ? 5 : 4};
    Program program = {.functions = &fn, .count = 1};
    VM vm = {.program = &program};
    Value *limit = value_size(100);
    Value *opened = gzip ? gzip_resource_call(&vm, "gzip_decoder_create", &limit)
                         : digest_resource_call(&vm, "sha256_create", NULL);
    Value *data = value_data(V_BYTES, compressed, size);
    Value *args[] = {payload(opened, "Ok"), data, value_bool(true)};
    const char *error = NULL;
    VMExecution *execution = vm_execution_create(&vm, &fn, args, NULL, NULL, &error);
    assert(execution && !error);
    if (cancel != 1) {
        assert(vm_execution_advance(execution, 10000) == VM_YIELDED);
        assert(!vm_execution_result(execution));
        if (cancel != 2) {
            VMExecutionStatus status = vm_execution_advance(execution, 10000);
            if (status != VM_COMPLETED) fprintf(stderr, "resource %d status %d error %s\n", gzip, status, vm.error ? vm.error : "none");
            assert(status == VM_COMPLETED);
            payload(vm_execution_result(execution), "Ok");
        }
    }
    assert(vm_execution_destroy(execution));
    assert(!vm.digests && !vm.gzip_streams);
    release(opened); release(data); release(limit);
}

static void hash_bounds_and_timing(void)
{
    VM vm = {0};
    uint8_t *data = calloc(16 * 1024 * 1024 + 1, 1);
    assert(data);
    Value *bytes = value_data(V_BYTES, data, 16 * 1024 * 1024);
    struct timespec start, end;
    assert(!clock_gettime(CLOCK_MONOTONIC, &start));
    for (size_t i = 0; i < 10; i++) {
        Value *hash = digest_hash_call(&vm, "sha256_hash", &bytes);
        assert(hash && hash->as.bytes.length == 32 && !vm.error);
        release(hash);
    }
    assert(!clock_gettime(CLOCK_MONOTONIC, &end));
    double ms = ((double)(end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000.0) / 10;
    printf("SHA-256 16 MiB one-shot: mean %.3f ms across 10 runs\n", ms);
    release(bytes);
    bytes = value_data(V_BYTES, data, 16 * 1024 * 1024 + 1);
    assert(!digest_hash_call(&vm, "sha256_hash", &bytes) && vm.error);
    release(bytes); free(data);
}

int main(void)
{
    const uint8_t expected[] = {
        0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
        0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad
    };
    VM vm = {0};
    Value *opened = digest_call(&vm, "digest_sha256_open", NULL);
    Value *handle = payload(opened, "Ok");
    Value *bytes = value_data(V_BYTES, (const uint8_t *)"abc", 3);
    Value *arguments[] = {handle, bytes};
    Value *updated = digest_call(&vm, "digest_update", arguments);
    payload(updated, "Ok");
    release(updated);
    Value *finished = digest_call(&vm, "digest_finish", arguments);
    Value *hash = payload(finished, "Ok");
    assert(hash->kind == V_BYTES && hash->as.bytes.length == 32);
    assert(!memcmp(hash->as.bytes.data, expected, 32));
    release(finished);
    Value *stale = digest_call(&vm, "digest_update", arguments);
    payload(stale, "Error");
    release(stale);
    release(bytes);
    release(opened);

    opened = digest_call(&vm, "digest_sha256_open", NULL);
    arguments[0] = payload(opened, "Ok");
    uint8_t large[65537] = {0};
    arguments[1] = value_data(V_BYTES, large, sizeof(large));
    updated = digest_call(&vm, "digest_update", arguments);
    payload(updated, "Error");
    release(updated);
    release(arguments[1]);
    Value *closed = digest_call(&vm, "digest_close", arguments);
    payload(closed, "Ok");
    release(closed);
    closed = digest_call(&vm, "digest_close", arguments);
    payload(closed, "Error");
    release(closed);
    release(opened);

    /* Destruction must also release unfinished contexts. */
    opened = digest_call(&vm, "digest_sha256_open", NULL);
    payload(opened, "Ok");
    release(opened);
    digest_destroy(&vm);
    assert(!vm.digests);
    digest_destroy(&vm);
    typed_digest();
    typed_gzip();
    typed_gzip_boundaries();
    for (unsigned cancel = 0; cancel < 3; cancel++) {
        async_step_yields(false, cancel);
        async_step_yields(true, cancel);
    }
    hash_bounds_and_timing();
    puts("PASS SHA-256 vector, stale handles, chunk bound, close and VM cleanup");
    return 0;
}
