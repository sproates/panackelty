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
VMStageLease *filesystem_stage_reserve(VM *vm, Value **failure);
Value *filesystem_stage_leased_create(VMStageLease *lease, Value **arguments);
void filesystem_stage_cancel_created(VMStageLease *lease);
void filesystem_stage_lease_release(VMStageLease *lease);
void filesystem_stage_destroy(VM *vm);
/* Embedded shutdown transfers cleanup to a bounded worker without host I/O. */
void filesystem_stage_destroy_deferred(VM *vm);
size_t filesystem_stage_pending_cleanup(void);
/* Blocking adapter shutdown only; never call from the embedded event loop. */
void filesystem_stage_drain_cleanup(void);
#endif
