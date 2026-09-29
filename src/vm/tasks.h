#ifndef PANACKELTY_TASKS_H
#define PANACKELTY_TASKS_H

#include "vm.h"

/* Experimental, single-thread, fake-host lifecycle harness. No stable ABI,
 * source spawning, OS callbacks or network service is implied. Async functions
 * use fixed typed read completions; ordinary functions retain fake print waits. The template
 * VM's verified program and snapshots must outlive the session. */
typedef struct VMTasks VMTasks;

typedef struct {
    uint64_t session;
    size_t slot;
} VMTaskId;

typedef struct {
    VMTaskId task;
    uint64_t generation;
} VMOperationId;

typedef enum {
    TASK_INVALID,
    TASK_READY,
    TASK_WAITING,
    TASK_JOINING,
    TASK_COMPLETED,
    TASK_TRAPPED,
    TASK_EXITED,
    TASK_CANCELLED
} VMTaskStatus;

typedef struct {
    size_t registered, released, queued;
} VMTaskStats;

/* Fixed total admissions per session (slots are not reused), bounded event ring.
 * Zero/overflowing limits and an active template VM are rejected. */
VMTasks *vm_tasks_create(const VM *template_vm, size_t task_limit, size_t event_limit,
                         const char **error);
/* {0,0} creates a root. A child requires a READY or WAITING parent. Earlier
 * ancestor deadlines are inherited. UINT64_MAX means no deadline; units are
 * arbitrary virtual monotonic ticks. Returns {0,0} on failure with no admission.
 */
VMTaskId vm_tasks_spawn(VMTasks *tasks, VMTaskId parent, Function *function, Value **arguments,
                        uint64_t deadline, const char **error);
/* FIFO completions, then expired deadlines, then round-robin bytecode dispatch.
 * Zero budget still processes events/deadlines. Time may not move backwards.
 * Cancellation after enqueue but before delivery wins; deadlines cancel a task
 * even if its operation was just delivered. Budget counts dispatched VM steps,
 * not cleanup/event work or wall-clock time. Returns false on invalid/re-entry.
 */
bool vm_tasks_pump(VMTasks *tasks, uint64_t now, size_t budget);
bool vm_tasks_cancel(VMTasks *tasks, VMTaskId task);
VMTaskStatus vm_tasks_status(const VMTasks *tasks, VMTaskId task);
/* Borrowed task result/error, valid until session destruction. Results exist
 * only for TASK_COMPLETED. Errors are static strings. Exit status is meaningful
 * only for TASK_EXITED. Records remain inspectable in task-creation order. */
Value *vm_tasks_result(const VMTasks *tasks, VMTaskId task);
const char *vm_tasks_error(const VMTasks *tasks, VMTaskId task);
size_t vm_tasks_exit_status(const VMTasks *tasks, VMTaskId task);
/* Exposes the pending fake print or read and its retained input. Complete is a typed
 * Void acknowledgement or a static error, never arbitrary VM data. Enqueue
 * rejects stale/wrong-session/duplicate operations and a full queue without
 * changing the wait; retry after pumping. No VM execution occurs inline. */
bool vm_tasks_pending(const VMTasks *tasks, VMTaskId task, VMOperationId *operation, Value **input);
bool vm_tasks_complete(VMTasks *tasks, VMOperationId operation, const char *error);
/* Typed async completion is retained only if enqueued, including invalid data
 * whose delivery must trap. Cancellation discards queued results safely. */
bool vm_tasks_complete_read(VMTasks *tasks, VMOperationId operation, Value *result);
uint64_t vm_tasks_next_deadline(const VMTasks *tasks);
VMTaskStats vm_tasks_stats(const VMTasks *tasks);
/* Cancels all remaining work and releases each fake request once. Fake requests
 * have no external producers. Real backends must quiesce before freeing a session;
 * calling any API with a destroyed session pointer is invalid. */
bool vm_tasks_destroy(VMTasks *tasks);

#endif
