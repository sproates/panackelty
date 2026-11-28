#include "digest.h"
#include "host_types.h"
#include <stdlib.h>
#include <string.h>

#ifndef __EMSCRIPTEN__
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#if OPENSSL_VERSION_MAJOR < 3
#error "Panackelty requires OpenSSL 3 or later"
#endif
#define DIGEST_HANDLES 65536
struct DigestSlot { EVP_MD_CTX *context; bool typed; };
struct VMDigests { struct DigestSlot *slots; size_t count, refs; };
#endif

static Value *digest_error(const char *operation, const char *code)
{
    char *names[] = {"operation", "code", "native_code"};
    Value *fields[] = {
        value_data(V_STR, (const uint8_t *)operation, strlen(operation)),
        value_data(V_STR, (const uint8_t *)code, strlen(code)), value_size(0)
    };
    Value *record = fields[0] && fields[1] && fields[2]
        ? named_value(V_RECORD, "HostError", names, fields, 3) : NULL;
    for (size_t i = 0; i < 3; i++) release(fields[i]);
    return host_variant("Error", record);
}

void digest_owner_release(VMDigests *owner)
{
#ifndef __EMSCRIPTEN__
    if (owner && !--owner->refs) { free(owner->slots); free(owner); }
#else
    (void)owner;
#endif
}

void digest_destroy(VM *vm)
{
#ifndef __EMSCRIPTEN__
    VMDigests *owner = vm->digests;
    if (!owner) return;
    vm->digests = NULL;
    for (size_t i = 0; i < owner->count; i++) {
        EVP_MD_CTX_free(owner->slots[i].context);
        owner->slots[i].context = NULL;
    }
    digest_owner_release(owner);
#else
    (void)vm;
#endif
}

/* Every mutation processes at most 64 KiB and runs atomically on the VM owner
 * thread. Async dispatch yields after the mutation, before its caller resumes. */
static Value *digest_operation(VM *vm, const char *name, Value **args, bool typed, unsigned op)
{
#ifdef __EMSCRIPTEN__
    (void)vm; (void)args; (void)typed; (void)op;
    return digest_error(name, "unsupported");
#else
    if (op == 0) {
        if (!vm->digests) {
            vm->digests = calloc(1, sizeof(*vm->digests));
            if (vm->digests) vm->digests->refs = 1;
        }
        VMDigests *owner = vm->digests;
        if (!owner || owner->count == DIGEST_HANDLES) return digest_error(name, "resource_limit");
        size_t slot = owner->count;
        struct DigestSlot *slots = realloc(owner->slots, (slot + 1) * sizeof(*slots));
        if (!slots) return digest_error(name, "resource_limit");
        owner->slots = slots;
        EVP_MD_CTX *context = EVP_MD_CTX_new();
        if (!context || EVP_DigestInit_ex2(context, EVP_sha256(), NULL) != 1) {
            EVP_MD_CTX_free(context);
            return digest_error(name, "digest_unavailable");
        }
        Value *handle = typed ? value_new(V_SHA256_CONTEXT) : value_size(slot + 1);
        if (handle && typed) {
            handle->as.sha256.owner = owner;
            handle->as.sha256.slot = slot;
            owner->refs++;
        }
        Value *result = host_variant("Ok", handle);
        if (!result) { EVP_MD_CTX_free(context); return NULL; }
        slots[slot] = (struct DigestSlot){.context = context, .typed = typed};
        owner->count++;
        return result;
    }
    VMDigests *owner = vm->digests;
    size_t slot;
    if (typed) {
        if (args[0]->kind != V_SHA256_CONTEXT || args[0]->as.sha256.owner != owner)
            return digest_error(name, "invalid_handle");
        slot = args[0]->as.sha256.slot;
    } else {
        size_t number;
        if (!value_index(args[0], &number) || !number) return digest_error(name, "invalid_handle");
        slot = number - 1;
    }
    if (!owner || slot >= owner->count || owner->slots[slot].typed != typed || !owner->slots[slot].context)
        return digest_error(name, "invalid_handle");
    EVP_MD_CTX *context = owner->slots[slot].context;
    if (op == 1) {
        if (args[1]->kind != V_BYTES || args[1]->as.bytes.length > 65536)
            return digest_error(name, "invalid_argument");
        if (EVP_DigestUpdate(context, args[1]->as.bytes.data, args[1]->as.bytes.length) != 1) {
            EVP_MD_CTX_free(context); owner->slots[slot].context = NULL;
            return digest_error(name, "digest_failed");
        }
        Value *result = host_variant("Ok", value_unit());
        // Never leave a mutated context usable if its acknowledgement was lost.
        if (!result) { EVP_MD_CTX_free(context); owner->slots[slot].context = NULL; }
        return result;
    }
    if (op == 2) {
        uint8_t bytes[EVP_MAX_MD_SIZE];
        unsigned int length = 0;
        int result = EVP_DigestFinal_ex(context, bytes, &length);
        EVP_MD_CTX_free(context); owner->slots[slot].context = NULL;
        if (result != 1 || length != 32) return digest_error(name, "digest_failed");
        return host_variant("Ok", value_data(V_BYTES, bytes, length));
    }
    EVP_MD_CTX_free(context); owner->slots[slot].context = NULL;
    return host_variant("Ok", value_unit());
#endif
}

Value *digest_call(VM *vm, const char *name, Value **args)
{
    unsigned op = !strcmp(name, "digest_sha256_open") ? 0 : !strcmp(name, "digest_update") ? 1 :
        !strcmp(name, "digest_finish") ? 2 : 3;
    if (op == 3 && strcmp(name, "digest_close")) return digest_error(name, "invalid_argument");
    return digest_operation(vm, name, args, false, op);
}

Value *digest_resource_call(VM *vm, const char *name, Value **args)
{
    unsigned op;
    if (!strcmp(name, "sha256_create") || !strcmp(name, "sha256_create_async")) op = 0;
    else if (!strcmp(name, "sha256_update") || !strcmp(name, "sha256_update_async")) op = 1;
    else if (!strcmp(name, "sha256_finish") || !strcmp(name, "sha256_finish_async")) op = 2;
    else if (!strcmp(name, "sha256_close") || !strcmp(name, "sha256_close_async")) op = 3;
    else return digest_error(name, "invalid_argument");
    return digest_operation(vm, name, args, true, op);
}

Value *digest_hash_call(VM *vm, const char *name, Value **args)
{
    (void)name;
    if (args[0]->kind != V_BYTES || args[0]->as.bytes.length > 16u * 1024u * 1024u) {
        vm->error = "VM trap: SHA-256 hash requires Bytes of at most 16 MiB; use a context for larger input";
        return NULL;
    }
#ifdef __EMSCRIPTEN__
    vm->error = "VM trap: SHA-256 is unavailable on this host";
    return NULL;
#else
    uint8_t bytes[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    EVP_MD_CTX *context = EVP_MD_CTX_new();
    bool ok = context && EVP_DigestInit_ex2(context, EVP_sha256(), NULL) == 1 &&
        EVP_DigestUpdate(context, args[0]->as.bytes.data, args[0]->as.bytes.length) == 1 &&
        EVP_DigestFinal_ex(context, bytes, &length) == 1 && length == 32;
    EVP_MD_CTX_free(context);
    if (!ok) { vm->error = "VM trap: SHA-256 unavailable"; return NULL; }
    return value_data(V_BYTES, bytes, length);
#endif
}
