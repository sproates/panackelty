#ifndef PANACKELTY_DIGEST_H
#define PANACKELTY_DIGEST_H
#include "vm.h"
Value *digest_call(VM *vm, const char *name, Value **arguments);
void digest_owner_release(VMDigests *owner);
Value *digest_resource_call(VM *vm, const char *name, Value **arguments);
Value *digest_hash_call(VM *vm, const char *name, Value **arguments);
void digest_destroy(VM *vm);
#endif
