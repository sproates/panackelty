#ifndef PANACKELTY_HOST_CAPABILITIES_H
#define PANACKELTY_HOST_CAPABILITIES_H

#include "vm.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

/* Typed POSIX services. Each call owns its descriptors and child-process cleanup. */

Value *host_process_run_cancelable(VM *vm, Value **arguments, bool isolated,
                                   const atomic_bool *cancelled, const char *coverage_ticket);
Value *host_capability_call(VM *vm, const char *name, Value **a);

/* A pending timer owns only a monotonic deadline. Poll never blocks. */
bool host_sleep_start(VM *vm, Value *duration, uint64_t *deadline, Value **result);
bool host_sleep_poll(uint64_t deadline, Value **result);
void host_sleep_wait(uint64_t deadline);

#endif
