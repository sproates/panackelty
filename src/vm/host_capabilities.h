#ifndef PANACKELTY_HOST_CAPABILITIES_H
#define PANACKELTY_HOST_CAPABILITIES_H

#include "vm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Typed POSIX services. Each call owns its descriptors and child-process cleanup. */

Value *host_capability_call(VM *vm, const char *name, Value **a);

#endif
