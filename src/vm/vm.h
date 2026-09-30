#ifndef PANACKELTY_VM_H
#define PANACKELTY_VM_H

#include "program.h"
#include "value.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Invocation context borrows its program, arguments, and environment snapshot.
 * error points to a static diagnostic; callees never allocate an error string.
 */

typedef struct VMExecution VMExecution;

typedef struct {
    Program *program;
    int argc;
    char **argv;
    size_t env_count;
    char **environment;
    const char *error;
    VMExecution *execution;
} VM;

/* Experimental internal API, not a stable embedder ABI. All calls stay on one
 * thread. VM/program/snapshots/context must outlive the execution. Functions
 * belong to a verified program; argument arrays match their parameter count.
 */
typedef enum { VM_YIELDED, VM_COMPLETED, VM_TRAPPED, VM_EXITED, VM_BUSY, VM_WAITING } VMExecutionStatus;

/* Trusted immediate host adapter for existing effectful services only. Borrow
 * arguments, return an owned result or NULL with a static error. Must not block,
 * destroy the active VM or invoke builtin_call to bypass the embedded profile.
 * No pending I/O is supported. process_exit/nested bytecode are intercepted.
 */
typedef Value *(*VMHostCall)(void *context, const char *name, Value **arguments,
                           size_t arity, const char **error);

/* Create retains arguments. NULL + error on failure; a busy VM is unchanged.
 * error is a required out-parameter. One execution per VM until destroyed.
 */
VMExecution *vm_execution_create(VM *vm, Function *fn, Value **arguments,
                                VMHostCall host_call, void *context, const char **error);
/* Zero budget does no work. Terminal outcomes are sticky. Re-entry returns
 * VM_BUSY without changing the active execution. Budget is not wall-clock time.
 */
VMExecutionStatus vm_execution_advance(VMExecution *execution, size_t budget);
/* Fixed fake pending print service for lifecycle experiments only. The callback
 * borrows the printed value and must retain it if needed while waiting. True
 * registers a wait; false supplies a static error. No synchronous CLI change.
 */
typedef bool (*VMServiceWait)(void *context, Value *value, const char **error);
VMExecution *vm_execution_create_pending(VM *vm, Function *fn, Value **arguments, VMServiceWait wait,
                                         void *context, const char **error);
/* Deliver a Void acknowledgement or static failure on the owning thread, outside
 * advance. Returns false for re-entry/non-waiting state; never resumes inline.
 * Allocation failure becomes a sticky trap. */
bool vm_execution_complete_print(VMExecution *execution, const char *error);
/* Fixed typed fake read. Its callback borrows a Bool input; completion borrows
 * Result[Bytes,Str] and retains it on successful delivery. Wrong schemas trap.
 * Print acknowledgement and read completion APIs cannot complete each other. */
VMExecution *vm_execution_create_async(VM *vm, Function *fn, Value **arguments,
                                       VMServiceWait wait, void *context, const char **error);
bool vm_execution_complete_read(VMExecution *execution, Value *result);
/* Native TCP is opt-in for embedded executions. Poll runs bounded socket work
 * on the owner thread, never bytecode. Zero wait interleaves independent VMs.
 * Destroying an execution cancels and closes its pending connection. External
 * read/print completions cannot complete a TCP wait. */
bool vm_execution_enable_tcp(VMExecution *execution);
bool vm_execution_poll_tcp(VMExecution *execution, unsigned max_wait_ms);
/* Listening is a separate opt-in. Pump dispatches at most budget child steps;
 * stop closes admission then drains. Destroy cancels synchronously. */
bool vm_execution_enable_server(VMExecution *execution);
bool vm_execution_pump_server(VMExecution *execution, size_t budget, unsigned max_wait_ms);
bool vm_execution_stop_server(VMExecution *execution);
bool vm_fake_read_result_valid(const Value *result);
Value *vm_fake_read_result(bool fail);
/* Borrowed result, valid until destroy; NULL unless completed. */
Value *vm_execution_result(const VMExecution *execution);
/* Valid only after VM_EXITED. */
size_t vm_execution_exit_status(const VMExecution *execution);
/* Release frames/results, including a suspended invocation. Refuses re-entry.
 * On success the pointer is invalid; the VM may then be used again.
 */
bool vm_execution_destroy(VMExecution *execution);

Value *execute(VM *vm, Function *fn, Value **arguments);

#endif
