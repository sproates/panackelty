#ifndef PANACKELTY_ROOT_READ_H
#define PANACKELTY_ROOT_READ_H

#include "vm.h"

typedef struct VMRootRead VMRootRead;

/* A bounded native filesystem operation. Arguments are copied before dispatch;
 * no worker holds a VM, execution, program or shared reference-counted value.
 * Poll/result/destroy belong to the execution's owner thread. Destroy abandons
 * an unfinished operation without waiting for a possibly blocked host syscall.
 * Its worker keeps the admission slot and releases its own state on completion.
 */
VMRootRead *root_read_start(VM *vm, Value **arguments, bool identity_only);
bool root_read_poll(VMRootRead *read);
/* Only the single-operation CLI adapter uses this blocking wait. */
void root_read_wait(VMRootRead *read);
Value *root_read_result(VMRootRead *read);
const char *root_read_error(VMRootRead *read);
void root_read_destroy(VMRootRead *read);
/* Includes abandoned workers until their host operation has returned. */
size_t root_read_active_count(void);

#endif
