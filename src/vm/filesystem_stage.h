#ifndef PANACKELTY_FILESYSTEM_STAGE_H
#define PANACKELTY_FILESYSTEM_STAGE_H
#include "vm.h"
Value *filesystem_stage_call(VM *vm, const char *name, Value **arguments);
Value *filesystem_stage_resource_call(VM *vm, const char *name, Value **arguments);
void filesystem_stage_owner_release(VMStageOwner *owner);
/* Acquire on the VM owner thread; thereafter a worker owns the lease. Arguments
 * and result values must be worker-local. Release can run after VM destruction. */
typedef struct VMStageLease VMStageLease;
VMStageLease *filesystem_stage_acquire(VM *vm, Value *resource, Value **failure);
Value *filesystem_stage_leased_call(VMStageLease *lease, const char *name, Value **arguments);
void filesystem_stage_lease_release(VMStageLease *lease);
void filesystem_stage_destroy(VM *vm);
#endif
