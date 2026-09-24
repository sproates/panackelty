#ifndef PANACKELTY_HOST_TYPES_H
#define PANACKELTY_HOST_TYPES_H

#include "vm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Opaque paths and exact time values at the host boundary. */

Value *host_type_call(VM *vm, const char *name, Value **a);
/* Consumes payload, including when allocating the result fails. */
Value *host_variant(const char *name, Value *payload);

#endif
