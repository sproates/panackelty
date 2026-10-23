#include "digest.h"
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
    puts("PASS SHA-256 vector, stale handles, chunk bound, close and VM cleanup");
    return 0;
}
