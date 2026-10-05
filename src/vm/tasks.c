#include "tasks.h"

#include <stdlib.h>

typedef struct {
    VMTasks *owner;
    VM vm;
    VMExecution *execution;
    size_t slot, parent;
    uint64_t deadline, generation;
    VMTaskStatus status, outcome;
    Value *result, *input;
    const char *error;
    size_t exit_status;
    bool pending, queued, async_read;
} Task;

typedef struct {
    VMOperationId operation;
    const char *error;
    Value *result;
} Completion;

struct VMTasks {
    VM template_vm;
    Task *tasks;
    Completion *events;
    size_t count, capacity, event_capacity, event_head, event_count, cursor;
    uint64_t identity, now;
    size_t registered, released;
    bool pumping;
};

/* This experiment, including session creation, belongs to one owning thread.
 * Exhaustion fails closed; addresses are never used as durable identities. */
static uint64_t last_session;

static const Task *lookup(const VMTasks *tasks, VMTaskId id)
{
    if (!tasks || id.session != tasks->identity || !id.slot || id.slot > tasks->count) {
        return NULL;
    }
    return &tasks->tasks[id.slot - 1];
}

static Task *mutable_task(VMTasks *tasks, VMTaskId id)
{
    return (Task *)lookup(tasks, id);
}

static bool terminal(const Task *task)
{
    return task->status >= TASK_COMPLETED;
}

static void release_request(Task *task)
{
    if (task->pending) {
        task->pending = false;
        task->queued = false;
        release(task->input);
        task->input = NULL;
        task->owner->released++;
    }
}

static bool register_request(void *context, Value *value, const char **error)
{
    Task *task = context;
    if (task->pending || task->generation == UINT64_MAX || task->owner->registered == SIZE_MAX) {
        *error = "task pending-operation limit reached";
        return false;
    }
    task->generation++;
    task->input = retain(value);
    task->pending = true;
    task->owner->registered++;
    return true;
}

VMTasks *vm_tasks_create(const VM *template_vm, size_t task_limit, size_t event_limit,
                         const char **error)
{
    *error = NULL;
    if (!template_vm || !template_vm->program || template_vm->execution || !task_limit ||
        !event_limit || task_limit > SIZE_MAX / sizeof(Task) ||
        event_limit > SIZE_MAX / sizeof(Completion) || last_session == UINT64_MAX) {
        *error = "invalid task session configuration";
        return NULL;
    }
    VMTasks *tasks = calloc(1, sizeof(VMTasks));
    if (tasks) {
        tasks->tasks = calloc(task_limit, sizeof(Task));
        tasks->events = calloc(event_limit, sizeof(Completion));
        if (!tasks->tasks || !tasks->events) {
            free(tasks->tasks);
            free(tasks->events);
            free(tasks);
            tasks = NULL;
        }
    }
    if (!tasks) {
        *error = "native VM out of memory";
        return NULL;
    }
    tasks->template_vm = *template_vm;
    vm_coverage_gap(template_vm->coverage);
    tasks->template_vm.coverage = NULL;
    tasks->template_vm.error = NULL;
    tasks->capacity = task_limit;
    tasks->event_capacity = event_limit;
    tasks->identity = ++last_session;
    return tasks;
}

VMTaskId vm_tasks_spawn(VMTasks *tasks, VMTaskId parent, Function *function, Value **arguments,
                        uint64_t deadline, const char **error)
{
    *error = NULL;
    const Task *ancestor = lookup(tasks, parent);
    bool root = !parent.session && !parent.slot;
    if (!tasks || tasks->pumping || tasks->count == tasks->capacity ||
        (!root &&
         (!ancestor || (ancestor->status != TASK_READY && ancestor->status != TASK_WAITING)))) {
        *error = "task admission rejected";
        return (VMTaskId){0};
    }
    if (ancestor && ancestor->deadline < deadline) {
        deadline = ancestor->deadline;
    }
    Task *task = &tasks->tasks[tasks->count];
    *task = (Task){.owner = tasks,
                   .vm = tasks->template_vm,
                   .slot = tasks->count + 1,
                   .parent = parent.slot,
                   .deadline = deadline,
                   .status = TASK_READY};
    task->async_read = function->is_async;
    task->execution = function->is_async
        ? vm_execution_create_async(&task->vm, function, arguments, register_request, task, error)
        : vm_execution_create_pending(&task->vm, function, arguments, register_request, task, error);
    if (!task->execution) {
        *task = (Task){0};
        return (VMTaskId){0};
    }
    tasks->count++;
    return (VMTaskId){tasks->identity, task->slot};
}

static bool descendant(const VMTasks *tasks, const Task *task, size_t parent)
{
    size_t slot = task->slot;
    while (slot && slot != parent) {
        slot = tasks->tasks[slot - 1].parent;
    }
    return slot == parent;
}

/* Retain results before destroying frames; only completed scopes expose them. */
static void finish_body(Task *task, VMTaskStatus outcome)
{
    if (task->execution) {
        if (outcome == TASK_COMPLETED) {
            task->result = retain(vm_execution_result(task->execution));
        } else if (outcome == TASK_EXITED) {
            task->exit_status = vm_execution_exit_status(task->execution);
        } else if (outcome == TASK_TRAPPED) {
            task->error = task->vm.error;
        }
        vm_execution_destroy(task->execution);
        task->execution = NULL;
    }
    if (outcome != TASK_COMPLETED) {
        release(task->result);
        task->result = NULL;
    }
    release_request(task);
    task->outcome = outcome;
    task->status = TASK_JOINING;
}

static void cancel_tree(VMTasks *tasks, size_t root)
{
    /* Reverse creation order is a non-recursive child-before-parent teardown. */
    for (size_t i = tasks->count; i-- > 0;) {
        Task *task = &tasks->tasks[i];
        if (!terminal(task) && descendant(tasks, task, root)) {
            finish_body(task, TASK_CANCELLED);
            task->status = TASK_CANCELLED;
        }
    }
}

static void cancel_children(VMTasks *tasks, size_t parent)
{
    for (size_t i = tasks->count; i-- > 0;) {
        Task *task = &tasks->tasks[i];
        if (task->slot != parent && !terminal(task) && descendant(tasks, task, parent)) {
            finish_body(task, TASK_CANCELLED);
            task->status = TASK_CANCELLED;
        }
    }
}

static void settle(VMTasks *tasks)
{
    /* Parent ids precede children, so one reverse traversal propagates failure
     * and joins all ancestors without recursive native calls. */
    for (size_t i = tasks->count; i-- > 0;) {
        Task *task = &tasks->tasks[i];
        if (task->status == TASK_JOINING) {
            bool joined = true;
            for (size_t child = i + 1; child < tasks->count; child++) {
                if (tasks->tasks[child].parent == task->slot && !terminal(&tasks->tasks[child])) {
                    joined = false;
                }
            }
            if (joined) {
                task->status = task->outcome;
            }
        }
        if (terminal(task) && task->status != TASK_COMPLETED && task->parent) {
            Task *parent = &tasks->tasks[task->parent - 1];
            if (!terminal(parent) && parent->outcome != TASK_TRAPPED &&
                parent->outcome != TASK_EXITED) {
                finish_body(parent, TASK_TRAPPED);
                parent->error = "child task did not complete successfully";
                cancel_children(tasks, parent->slot);
            }
        }
    }
}

bool vm_tasks_cancel(VMTasks *tasks, VMTaskId id)
{
    Task *task = mutable_task(tasks, id);
    if (!task || tasks->pumping) {
        return false;
    }
    cancel_tree(tasks, task->slot);
    settle(tasks);
    return true;
}

bool vm_tasks_pending(const VMTasks *tasks, VMTaskId id, VMOperationId *operation, Value **input)
{
    const Task *task = lookup(tasks, id);
    if (!task || !task->pending) {
        return false;
    }
    *operation = (VMOperationId){id, task->generation};
    *input = task->input;
    return true;
}

static bool complete(VMTasks *tasks, VMOperationId operation, const char *error, Value *result, bool read)
{
    Task *task = mutable_task(tasks, operation.task);
    if (!task || tasks->pumping || task->async_read != read || !task->pending || task->queued ||
        task->generation != operation.generation || tasks->event_count == tasks->event_capacity) {
        return false;
    }
    size_t tail = (tasks->event_head + tasks->event_count) % tasks->event_capacity;
    tasks->events[tail] = (Completion){operation, error, retain(result)};
    tasks->event_count++;
    task->queued = true;
    return true;
}

static void deliver_completions(VMTasks *tasks)
{
    while (tasks->event_count) {
        Completion event = tasks->events[tasks->event_head];
        tasks->event_head = (tasks->event_head + 1) % tasks->event_capacity;
        tasks->event_count--;
        Task *task = mutable_task(tasks, event.operation.task);
        if (!task || !task->pending || task->generation != event.operation.generation) {
            release(event.result);
            continue;
        }
        if (task->async_read) {
            vm_execution_complete_read(task->execution, event.result);
        } else {
            vm_execution_complete_print(task->execution, event.error);
        }
        release(event.result);
        release_request(task);
        if (task->vm.error) {
            finish_body(task, TASK_TRAPPED);
            cancel_children(tasks, task->slot);
            settle(tasks);
        } else {
            task->status = TASK_READY;
        }
    }
}

bool vm_tasks_pump(VMTasks *tasks, uint64_t now, size_t budget)
{
    if (!tasks || tasks->pumping || now < tasks->now) {
        return false;
    }
    tasks->pumping = true;
    tasks->now = now;
    deliver_completions(tasks);
    for (size_t i = 0; i < tasks->count; i++) {
        Task *task = &tasks->tasks[i];
        if (!terminal(task) && task->deadline != UINT64_MAX && task->deadline <= now) {
            cancel_tree(tasks, task->slot);
        }
    }
    settle(tasks);
    while (budget && tasks->count) {
        Task *task = NULL;
        for (size_t scanned = 0; scanned < tasks->count; scanned++) {
            size_t slot = tasks->cursor;
            tasks->cursor = (tasks->cursor + 1) % tasks->count;
            if (tasks->tasks[slot].status == TASK_READY) {
                task = &tasks->tasks[slot];
                break;
            }
        }
        if (!task) {
            break;
        }
        budget--;
        VMExecutionStatus status = vm_execution_advance(task->execution, 1);
        if (status == VM_WAITING) {
            task->status = TASK_WAITING;
        } else if (status != VM_YIELDED) {
            VMTaskStatus outcome = status == VM_COMPLETED ? TASK_COMPLETED
                                   : status == VM_EXITED  ? TASK_EXITED
                                                          : TASK_TRAPPED;
            finish_body(task, outcome);
            if (outcome != TASK_COMPLETED) {
                cancel_children(tasks, task->slot);
            }
            settle(tasks);
        }
    }
    tasks->pumping = false;
    return true;
}

VMTaskStatus vm_tasks_status(const VMTasks *tasks, VMTaskId id)
{
    const Task *task = lookup(tasks, id);
    return task ? task->status : TASK_INVALID;
}

Value *vm_tasks_result(const VMTasks *tasks, VMTaskId id)
{
    const Task *task = lookup(tasks, id);
    return task && task->status == TASK_COMPLETED ? task->result : NULL;
}

const char *vm_tasks_error(const VMTasks *tasks, VMTaskId id)
{
    const Task *task = lookup(tasks, id);
    return task ? task->error : NULL;
}

size_t vm_tasks_exit_status(const VMTasks *tasks, VMTaskId id)
{
    const Task *task = lookup(tasks, id);
    return task ? task->exit_status : 0;
}

uint64_t vm_tasks_next_deadline(const VMTasks *tasks)
{
    uint64_t deadline = UINT64_MAX;
    for (size_t i = 0; i < tasks->count; i++) {
        const Task *task = &tasks->tasks[i];
        if (!terminal(task) && task->deadline < deadline) {
            deadline = task->deadline;
        }
    }
    return deadline;
}

VMTaskStats vm_tasks_stats(const VMTasks *tasks)
{
    return (VMTaskStats){tasks->registered, tasks->released, tasks->event_count};
}

bool vm_tasks_destroy(VMTasks *tasks)
{
    if (!tasks) {
        return true;
    }
    if (tasks->pumping) {
        return false;
    }
    for (size_t i = tasks->count; i-- > 0;) {
        Task *task = &tasks->tasks[i];
        vm_execution_destroy(task->execution);
        release_request(task);
        release(task->result);
    }
    while (tasks->event_count) {
        release(tasks->events[tasks->event_head].result);
        tasks->event_head = (tasks->event_head + 1) % tasks->event_capacity;
        tasks->event_count--;
    }
    free(tasks->events);
    free(tasks->tasks);
    free(tasks);
    return true;
}

bool vm_tasks_complete(VMTasks *tasks, VMOperationId operation, const char *error)
{
    return complete(tasks, operation, error, NULL, false);
}

bool vm_tasks_complete_read(VMTasks *tasks, VMOperationId operation, Value *result)
{
    return complete(tasks, operation, NULL, result, true);
}
