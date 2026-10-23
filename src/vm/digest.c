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

/* Handles are invocation-local and never reused. Completed contexts are freed
 * immediately; the bounded table retains only stale-handle identities. */
#define DIGEST_HANDLES 65536
struct VMDigests {
    EVP_MD_CTX **contexts;
    size_t count;
};
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

void digest_destroy(VM *vm)
{
#ifndef __EMSCRIPTEN__
    if (!vm->digests) return;
    for (size_t i = 0; i < vm->digests->count; i++) {
        EVP_MD_CTX_free(vm->digests->contexts[i]);
    }
    free(vm->digests->contexts);
    free(vm->digests);
    vm->digests = NULL;
#else
    (void)vm;
#endif
}

Value *digest_call(VM *vm, const char *name, Value **arguments)
{
#ifdef __EMSCRIPTEN__
    (void)vm;
    (void)arguments;
    return digest_error(name, "unsupported");
#else
    if (!strcmp(name, "digest_sha256_open")) {
        if (!vm->digests) vm->digests = calloc(1, sizeof(*vm->digests));
        if (!vm->digests) return digest_error(name, "resource_limit");
        if (vm->digests->count == DIGEST_HANDLES) {
            return digest_error(name, "resource_limit");
        }
        size_t count = vm->digests->count;
        EVP_MD_CTX **contexts = realloc(vm->digests->contexts,
            (count + 1) * sizeof(*contexts));
        if (!contexts) return digest_error(name, "resource_limit");
        vm->digests->contexts = contexts;
        EVP_MD_CTX *context = EVP_MD_CTX_new();
        if (!context || EVP_DigestInit_ex2(context, EVP_sha256(), NULL) != 1) {
            EVP_MD_CTX_free(context);
            return digest_error(name, "digest_unavailable");
        }
        contexts[count] = context;
        vm->digests->count++;
        return host_variant("Ok", value_size(count + 1));
    }
    size_t handle;
    if (arguments[0]->kind != V_NAT ||
        !pn_big_fits_size(&arguments[0]->as.integer, &handle) || !handle ||
        !vm->digests || handle > vm->digests->count ||
        !vm->digests->contexts[handle - 1]) {
        return digest_error(name, "invalid_handle");
    }
    EVP_MD_CTX *context = vm->digests->contexts[handle - 1];
    if (!strcmp(name, "digest_update")) {
        if (arguments[1]->kind != V_BYTES || arguments[1]->as.bytes.length > 65536) {
            return digest_error(name, "invalid_argument");
        }
        if (EVP_DigestUpdate(context, arguments[1]->as.bytes.data,
                            arguments[1]->as.bytes.length) != 1) {
            EVP_MD_CTX_free(context);
            vm->digests->contexts[handle - 1] = NULL;
            return digest_error(name, "digest_failed");
        }
        return host_variant("Ok", value_new(V_UNIT));
    }
    if (!strcmp(name, "digest_finish")) {
        uint8_t bytes[EVP_MAX_MD_SIZE];
        unsigned int length = 0;
        int result = EVP_DigestFinal_ex(context, bytes, &length);
        EVP_MD_CTX_free(context);
        vm->digests->contexts[handle - 1] = NULL;
        if (result != 1 || length != 32) return digest_error(name, "digest_failed");
        return host_variant("Ok", value_data(V_BYTES, bytes, length));
    }
    if (!strcmp(name, "digest_close")) {
        EVP_MD_CTX_free(context);
        vm->digests->contexts[handle - 1] = NULL;
        return host_variant("Ok", value_new(V_UNIT));
    }
    return digest_error(name, "invalid_argument");
#endif
}
