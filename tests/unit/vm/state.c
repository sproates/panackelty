#ifdef NDEBUG
#undef NDEBUG
#endif
#include "state.h"
#include "builtins.h"
#include "vm.h"
#include "tasks.h"
#include <string.h>
#include <assert.h>

static void reservations_preserve_snapshots(void)
{
    Value *zero = value_size(0), *one = value_size(1), *two = value_size(2);
    StateStatus status;
    Value *cell = state_create(zero, &status), *other = state_create(zero, &status);
    assert(cell && other && status == STATE_OK);
    Value *alias = retain(cell), *snapshot = state_read(cell, &status);
    assert(snapshot == zero && value_equal(alias, cell) && !value_equal(cell, other));
    Value *left = value_sequence(V_ARRAY, &cell, 1);
    Value *right = value_sequence(V_ARRAY, &alias, 1);
    assert(value_equal(left, right));
    release(left); release(right);
    StateReservation first = {0}, second = {0}, independent = {0};
    assert(state_reserve(cell, &first) == STATE_OK);
    assert(!state_read(alias, &status) && status == STATE_BUSY);
    assert(state_reserve(alias, &second) == STATE_BUSY && !second.cell);
    assert(state_reserve(other, &independent) == STATE_OK);
    assert(value_equal(alias, cell)); /* Identity does not wait on the owner. */
    StateReservation forged = {.cell = cell};
    assert(!state_reserved_read(&forged));
    assert(state_commit(&forged, two) == STATE_INVALID);
    state_abort(&forged);
    assert(!state_read(cell, &status) && status == STATE_BUSY);
    Value *current = state_reserved_read(&first);
    assert(current == zero);
    release(current);
    assert(state_commit(&first, one) == STATE_OK && !first.cell);
    assert(snapshot == zero); /* The old immutable snapshot survives commit. */
    assert(state_reserve(alias, &second) == STATE_OK);
    current = state_reserved_read(&second);
    assert(current == one);
    release(current);
    assert(state_commit(&second, two) == STATE_OK);
    state_abort(&second); /* Cancellation after commit cannot undo it. */
    current = state_read(alias, &status);
    assert(current == two && status == STATE_OK);
    release(current);
    assert(state_commit(&independent, one) == STATE_OK);
    assert(state_reserve(cell, &first) == STATE_OK);
    assert(state_commit(&first, other) == STATE_INVALID);
    state_abort(&first); /* Failed transformation leaves the old value. */
    current = state_read(cell, &status);
    assert(current == two);
    release(current);
    assert(state_reserve(cell, &first) == STATE_OK);
    release(alias);
    release(cell); /* Reservation keeps the final owner alive until abort. */
    state_abort(&first);
    release(other); release(snapshot); release(zero); release(one); release(two);
}

static void payloads_exclude_capabilities_and_seal_storage(void)
{
    StateStatus status;
    CaptureCheck check;
    Value *zero = value_size(0), *empty = value_sequence(V_ARRAY, NULL, 0);
    Value *array = value_array_append(empty, zero);
    release(empty);
    char *payload_fields[] = {"items"};
    Value *payload = named_value(V_RECORD, "Payload", payload_fields, &array, 1);
    Value *cell = state_create(payload, &status);
    release(payload);
    assert(cell && status == STATE_OK);
    Value *closure = value_closure("body", &cell, 1, &check);
    assert(closure && check == CAPTURE_ALLOWED);
    Value *extension = value_array_append(array, closure);
    assert(extension && extension->as.sequence.storage != array->as.sequence.storage);
    assert(value_state_payload_check(array) == CAPTURE_ALLOWED);
    assert(!state_create(extension, &status) && status == STATE_INVALID);
    char *fields[] = {"cell"};
    Value *nested = named_value(V_RECORD, "Box", fields, &cell, 1);
    assert(!state_create(nested, &status) && status == STATE_INVALID);
    release(nested);
    release(extension); release(closure); release(cell); release(array);

    /* A hidden resource already present before admission is also rejected. */
    empty = value_sequence(V_ARRAY, NULL, 0);
    array = value_array_append(empty, zero);
    release(empty);
    Value *resource = value_new(V_ITER);
    resource->as.iterator.iterable = value_sequence(V_ARRAY, NULL, 0);
    extension = value_array_append(array, resource);
    assert(extension && extension->as.sequence.storage == array->as.sequence.storage);
    assert(!state_create(array, &status) && status == STATE_INVALID);
    release(extension);
    cell = state_create(array, &status);
    assert(cell && status == STATE_OK);
    release(resource); release(cell); release(array); release(zero);
    assert(!state_create(NULL, &status) && status == STATE_INVALID);
    assert(!state_read(NULL, &status) && status == STATE_INVALID);
}

static void scheduler_reservations(void);
static void collection_keys_exclude_cells_and_callables(void);
static void captured_transforms_and_payload_rejection(void);
static void creation_effects(void);
static void deadlines_release_cells(void);
static void forged_effects_release_reservations(void);

void state_cell_contracts(void)
{
    scheduler_reservations();
    collection_keys_exclude_cells_and_callables();
    captured_transforms_and_payload_rejection();
    creation_effects();
    deadlines_release_cells();
    forged_effects_release_reservations();
    reservations_preserve_snapshots();
    payloads_exclude_capabilities_and_seal_storage();
}

/* Drive exact instruction boundaries instead of relying on wall-clock sleeps. */
static void scheduler_reservations(void)
{
    char *cell_params[] = {"cell"}, *value_params[] = {"value"};
    uint32_t one = 1;
    Instruction increment[] = {
        {.op = OP_LOAD, .name = "value"},
        {.op = OP_CONST, .constant = {.tag = 0, .number = {.sign = 1, .len = 1, .limbs = &one}}},
        {.op = OP_BINARY, .code = 0}, {.op = OP_RETURN}};
    Instruction update[] = {
        {.op = OP_LOAD, .name = "cell"},
        {.op = OP_CONST, .constant = {.tag = 3, .text = "increment", .text_length = 9}},
        {.op = OP_CALL, .name = "state_update", .arity = 2}, {.op = OP_RETURN}};
    Instruction async_update[4];
    memcpy(async_update, update, sizeof(update));
    async_update[2] = (Instruction){.op = OP_AWAIT_CALL,
        .name = "state_update_async", .arity = 2};
    Instruction read[] = {{.op = OP_LOAD, .name = "cell"},
        {.op = OP_CALL, .name = "state_read", .arity = 1}, {.op = OP_RETURN}};
    Function functions[] = {
        {.name = "increment", .params = value_params, .param_count = 1,
         .ins = increment, .ins_count = 4, .pure = true},
        {.name = "update", .params = cell_params, .param_count = 1, .ins = update, .ins_count = 4},
        {.name = "async_update", .params = cell_params, .param_count = 1,
         .ins = async_update, .ins_count = 4, .is_async = true},
        {.name = "read", .params = cell_params, .param_count = 1, .ins = read, .ins_count = 3}};
    Program program = {.functions = functions, .count = 4};
    for (size_t mode = 0; mode < 4; mode++) {
        StateStatus status;
        Value *zero = value_size(0), *cell = state_create(zero, &status);
        Value *independent = state_create(zero, &status);
        release(zero);
        VM owner_vm = {.program = &program}, waiter_vm = {.program = &program};
        VM reader_vm = {.program = &program}, unrelated_vm = {.program = &program};
        const char *error = NULL;
        VMExecution *owner = vm_execution_create(&owner_vm, &functions[1], &cell, NULL, NULL, &error);
        VMExecution *waiter = vm_execution_create(&waiter_vm, &functions[2], &cell, NULL, NULL, &error);
        VMExecution *reader = vm_execution_create(&reader_vm, &functions[3], &cell, NULL, NULL, &error);
        VMExecution *unrelated = vm_execution_create(&unrelated_vm, &functions[1], &independent, NULL, NULL, &error);
        assert(owner && waiter && reader && unrelated && !error);
        assert(vm_execution_advance(owner, 3) == VM_YIELDED);
        assert(vm_execution_advance(waiter, 3) == VM_WAITING);
        assert(vm_execution_advance(reader, 2) == VM_WAITING);
        assert(!vm_execution_complete_print(reader, NULL));
        assert(vm_execution_poll_network(waiter, 0));
        assert(vm_execution_advance(waiter, 1) == VM_WAITING);
        assert(vm_execution_advance(unrelated, 8) == VM_COMPLETED);
        size_t number;
        assert(value_index(vm_execution_result(unrelated), &number) && number == 1);
        assert(vm_execution_destroy(unrelated));
        assert(vm_execution_advance(owner, 2) == VM_YIELDED); /* Mid-transform. */
        if (mode == 1) {
            assert(vm_execution_destroy(owner)); owner = NULL;
        } else if (mode == 2) {
            increment[3].op = OP_MATCH_FAIL;
            assert(vm_execution_advance(owner, 2) == VM_TRAPPED);
            increment[3].op = OP_RETURN;
        } else {
            if (mode == 3) { assert(vm_execution_destroy(waiter)); waiter = NULL; }
            assert(vm_execution_advance(owner, 2) == VM_YIELDED); /* Committed, caller not returned. */
            assert(vm_execution_destroy(owner)); owner = NULL;
        }
        if (waiter) {
            assert(vm_execution_poll_network(waiter, 0));
            assert(vm_execution_advance(waiter, 5) == VM_COMPLETED);
            assert(value_index(vm_execution_result(waiter), &number));
            assert(number == (mode == 0 ? 2 : 1));
            assert(vm_execution_destroy(waiter));
        }
        assert(vm_execution_poll_network(reader, 0));
        assert(vm_execution_advance(reader, 1) == VM_COMPLETED);
        assert(value_index(vm_execution_result(reader), &number));
        assert(number == (mode == 0 ? 2 : 1));
        assert(vm_execution_destroy(reader));
        if (owner) assert(vm_execution_destroy(owner));
        assert(cell->refs == 1 && independent->refs == 1);
        release(cell); release(independent);
    }
}

static void deadlines_release_cells(void)
{
    char *params[] = {"cell", "callback"}, *value_params[] = {"value"};
    Instruction loop[] = {{.op = OP_JUMP, .target = 0}};
    Instruction identity[] = {{.op = OP_LOAD, .name = "value"}, {.op = OP_RETURN}};
    Instruction update[] = {{.op = OP_LOAD, .name = "cell"},
        {.op = OP_LOAD, .name = "callback"},
        {.op = OP_CALL, .name = "state_update", .arity = 2}, {.op = OP_RETURN}};
    Instruction read[] = {{.op = OP_LOAD, .name = "cell"},
        {.op = OP_CALL, .name = "state_read", .arity = 1}, {.op = OP_RETURN}};
    Function functions[] = {
        {.name = "loop", .params = value_params, .param_count = 1, .pure = true,
         .ins = loop, .ins_count = 1},
        {.name = "identity", .params = value_params, .param_count = 1, .pure = true,
         .ins = identity, .ins_count = 2},
        {.name = "update", .params = params, .param_count = 2, .ins = update, .ins_count = 4},
        {.name = "read", .params = params, .param_count = 1, .ins = read, .ins_count = 3}};
    Program program = {.functions = functions, .count = 4};
    VM vm = {.program = &program};
    const char *error = NULL;
    VMTasks *tasks = vm_tasks_create(&vm, 4, 4, &error);
    assert(tasks && !error);
    StateStatus status;
    Value *cell = state_create(value_unit(), &status), *other = state_create(value_bool(true), &status);
    Value *endless = value_data(V_STR, (const uint8_t *)"loop", 4);
    Value *callback = value_data(V_STR, (const uint8_t *)"identity", 8);
    Value *owner_args[] = {cell, endless}, *waiter_args[] = {cell, callback};
    VMTaskId root = {0};
    VMTaskId owner = vm_tasks_spawn(tasks, root, &functions[2], owner_args, 3, &error);
    assert(owner.slot && !error);
    assert(vm_tasks_pump(tasks, 0, 3)); /* Acquire before admitting contenders. */
    VMTaskId waiter = vm_tasks_spawn(tasks, root, &functions[2], waiter_args, 2, &error);
    VMTaskId reader = vm_tasks_spawn(tasks, root, &functions[3], &cell, UINT64_MAX, &error);
    VMTaskId unrelated = vm_tasks_spawn(tasks, root, &functions[3], &other, UINT64_MAX, &error);
    assert(waiter.slot && reader.slot && unrelated.slot && !error);
    assert(vm_tasks_pump(tasks, 1, 100));
    assert(vm_tasks_status(tasks, owner) == TASK_READY);
    assert(vm_tasks_status(tasks, waiter) == TASK_WAITING);
    assert(vm_tasks_status(tasks, reader) == TASK_WAITING);
    assert(vm_tasks_status(tasks, unrelated) == TASK_COMPLETED);
    assert(vm_tasks_result(tasks, unrelated) == value_bool(true));
    assert(vm_tasks_pump(tasks, 2, 0));
    assert(vm_tasks_status(tasks, waiter) == TASK_CANCELLED);
    assert(!state_read(cell, &status) && status == STATE_BUSY);
    assert(vm_tasks_pump(tasks, 3, 10));
    assert(vm_tasks_pump(tasks, 3, 10)); /* A prior zero-budget poll may leave READY. */
    assert(vm_tasks_status(tasks, owner) == TASK_CANCELLED);
    assert(vm_tasks_status(tasks, reader) == TASK_COMPLETED);
    assert(vm_tasks_result(tasks, reader) == value_unit());
    assert(vm_tasks_destroy(tasks));
    assert(cell->refs == 1 && other->refs == 1);
    release(cell); release(other); release(endless); release(callback);
}

static void forged_effects_release_reservations(void)
{
    char *params[] = {"value"};
    Instruction bad[] = {{.op = OP_LOAD, .name = "value"},
        {.op = OP_CALL, .name = "state_create", .arity = 1}, {.op = OP_RETURN}};
    Instruction update[] = {{.op = OP_LOAD, .name = "value"},
        {.op = OP_CONST, .constant = {.tag = 3, .text = "bad", .text_length = 3}},
        {.op = OP_CALL, .name = "state_update", .arity = 2}, {.op = OP_RETURN}};
    Function functions[] = {
        {.name = "bad", .params = params, .param_count = 1, .pure = true, .ins = bad, .ins_count = 3},
        {.name = "update", .params = params, .param_count = 1, .ins = update, .ins_count = 4}};
    Program program = {.functions = functions, .count = 2};
    const char *error = NULL;
    /* Runtime checks below deliberately bypass verification. */
    StateStatus status;
    Value *cell = state_create(value_unit(), &status);
    for (size_t mode = 0; mode < 3; mode++) {
        /* Defence at invocation as well as verification: an impure callback,
         * a pure-labelled body doing effects, and a pure caller doing update. */
        functions[0].pure = mode != 0;
        functions[1].pure = mode == 2;
        VM vm = {.program = &program};
        VMExecution *run = vm_execution_create(&vm, &functions[1], &cell, NULL, NULL, &error);
        assert(run && vm_execution_advance(run, 10) == VM_TRAPPED && vm.error);
        assert(vm_execution_destroy(run));
        Value *snapshot = state_read(cell, &status);
        assert(snapshot == value_unit() && status == STATE_OK && cell->refs == 1);
        release(snapshot);
    }
    release(cell);
}

static void captured_transforms_and_payload_rejection(void)
{
    char *params[] = {"replacement", "value"}, *caller_params[] = {"cell", "callback"};
    Instruction replacement[] = {{.op = OP_LOAD, .name = "replacement"}, {.op = OP_RETURN}};
    Instruction update[] = {{.op = OP_LOAD, .name = "cell"}, {.op = OP_LOAD, .name = "callback"},
        {.op = OP_CALL, .name = "state_update", .arity = 2}, {.op = OP_RETURN}};
    Function functions[] = {
        {.name = "replacement", .params = params, .param_count = 2, .pure = true,
         .ins = replacement, .ins_count = 2},
        {.name = "update", .params = caller_params, .param_count = 2, .ins = update, .ins_count = 4}};
    Program program = {.functions = functions, .count = 2};
    for (size_t mode = 0; mode < 2; mode++) {
        CaptureCheck check;
        Value *payload = mode ? value_closure("replacement", NULL, 0, &check) : value_size(5);
        Value *callback = value_closure("replacement", &payload, 1, &check);
        release(payload);
        StateStatus status;
        Value *cell = state_create(value_unit(), &status);
        Value *args[] = {cell, callback};
        VM vm = {.program = &program};
        const char *error = NULL;
        VMExecution *run = vm_execution_create(&vm, &functions[1], args, NULL, NULL, &error);
        assert(run && !error);
        release(callback); /* Invocation owns the retained capture environment. */
        assert(vm_execution_advance(run, 8) == (mode ? VM_TRAPPED : VM_COMPLETED));
        Value *snapshot = state_read(cell, &status);
        size_t number;
        assert(status == STATE_OK);
        if (mode) assert(snapshot == value_unit() && vm.error);
        else assert(value_index(snapshot, &number) && number == 5);
        release(snapshot);
        assert(vm_execution_destroy(run) && cell->refs == 1);
        release(cell);
    }
}

static void creation_effects(void)
{
    char *params[] = {"value"};
    Instruction create[] = {{.op = OP_LOAD, .name = "value"},
        {.op = OP_CALL, .name = "state_create", .arity = 1}, {.op = OP_RETURN}};
    Function function = {.name = "create", .params = params, .param_count = 1,
                         .ins = create, .ins_count = 3};
    Program program = {.functions = &function, .count = 1};
    for (size_t mode = 0; mode < 4; mode++) {
        function.pure = mode == 2;
        function.is_async = mode == 1 || mode == 3;
        create[1].op = mode == 1 ? OP_AWAIT_CALL : OP_CALL;
        create[1].name = mode == 1 ? "state_create_async" : "state_create";
        VM vm = {.program = &program};
        Value *initial = value_unit();
        const char *error = NULL;
        VMExecution *run = vm_execution_create(&vm, &function, &initial, NULL, NULL, &error);
        assert(run && !error);
        assert(vm_execution_advance(run, 3) == (mode < 2 ? VM_COMPLETED : VM_TRAPPED));
        if (mode < 2) {
            StateStatus status;
            Value *snapshot = state_read(vm_execution_result(run), &status);
            assert(snapshot == initial && status == STATE_OK);
            release(snapshot);
        }
        assert(vm_execution_destroy(run));
    }
}

static void collection_keys_exclude_cells_and_callables(void)
{
    StateStatus status;
    CaptureCheck check;
    Value *cell = state_create(value_unit(), &status);
    Value *callback = value_closure("body", NULL, 0, &check);
    Value *array = value_sequence(V_ARRAY, &cell, 1);
    Value *empty = value_sequence(V_ARRAY, NULL, 0);
    Value *appended = value_array_append(empty, cell);
    Value *joined = value_array_concat(empty, array);
    char *fields[] = {"value"};
    Value *record = named_value(V_RECORD, "Box", fields, &cell, 1);
    Value *map = value_sequence(V_MAP, NULL, 0), *set = value_sequence(V_SET, NULL, 0);
    Value *bad[] = {cell, callback, array, appended, joined, record};
    for (size_t i = 0; i < sizeof(bad) / sizeof(*bad); i++) {
        VM vm = {0};
        Value *arguments[] = {map, bad[i], value_unit()};
        assert(!builtin_call(&vm, "map_put", arguments) && vm.error);
        vm.error = NULL;
        arguments[0] = set;
        assert(!builtin_call(&vm, "set_add", arguments) && vm.error);
        assert(map->as.sequence.count == 0 && set->as.sequence.count == 0);
    }
    Value *unit = value_unit();
    Value *key = named_value(V_RECORD, "Box", fields, &unit, 1);
    Value *arguments[] = {map, key, cell};
    VM vm = {0};
    Value *stored = builtin_call(&vm, "map_put", arguments);
    assert(stored && !vm.error && stored->key_forbidden && !key->key_forbidden);
    arguments[0] = stored;
    Value *found = builtin_call(&vm, "map_get", arguments);
    assert(found == cell && !vm.error);
    release(found); release(stored); release(key);
    release(map); release(set); release(record); release(joined); release(appended);
    release(empty); release(array); release(callback); release(cell);
}
