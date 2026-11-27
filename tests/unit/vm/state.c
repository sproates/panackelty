#ifdef NDEBUG
#undef NDEBUG
#endif
#include "state.h"
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

void state_cell_contracts(void)
{
    reservations_preserve_snapshots();
    payloads_exclude_capabilities_and_seal_storage();
}
