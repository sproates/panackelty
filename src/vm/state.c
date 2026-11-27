#include "state.h"

static StateStatus payload_status(Value *value)
{
    switch (value_state_payload_check(value)) {
    case CAPTURE_ALLOWED: return STATE_OK;
    case CAPTURE_OUT_OF_MEMORY: return STATE_OUT_OF_MEMORY;
    default: return STATE_INVALID;
    }
}

Value *state_create(Value *initial, StateStatus *status)
{
    *status = payload_status(initial);
    if (*status != STATE_OK) return NULL;
    Value *cell = value_new(V_STATE);
    if (!cell) { *status = STATE_OUT_OF_MEMORY; return NULL; }
    cell->as.state.payload = retain(initial);
    return cell;
}

Value *state_read(Value *cell, StateStatus *status)
{
    *status = !cell || cell->kind != V_STATE || !cell->as.state.payload ? STATE_INVALID :
              cell->as.state.reservation ? STATE_BUSY : STATE_OK;
    return *status == STATE_OK ? retain(cell->as.state.payload) : NULL;
}

StateStatus state_reserve(Value *cell, StateReservation *reservation)
{
    if (!reservation || reservation->cell || !cell || cell->kind != V_STATE ||
        !cell->as.state.payload) return STATE_INVALID;
    if (cell->as.state.reservation) return STATE_BUSY;
    reservation->cell = retain(cell);
    cell->as.state.reservation = reservation;
    return STATE_OK;
}

static bool owns_reservation(const StateReservation *reservation)
{
    return reservation && reservation->cell && reservation->cell->kind == V_STATE &&
           reservation->cell->as.state.reservation == reservation;
}

Value *state_reserved_read(const StateReservation *reservation)
{
    return owns_reservation(reservation) ? retain(reservation->cell->as.state.payload) : NULL;
}

void state_abort(StateReservation *reservation)
{
    if (!owns_reservation(reservation)) return;
    Value *cell = reservation->cell;
    cell->as.state.reservation = NULL;
    reservation->cell = NULL;
    release(cell);
}

StateStatus state_commit(StateReservation *reservation, Value *replacement)
{
    if (!owns_reservation(reservation)) return STATE_INVALID;
    StateStatus status = payload_status(replacement);
    if (status != STATE_OK) return status;
    Value *cell = reservation->cell;
    Value *previous = cell->as.state.payload;
    cell->as.state.payload = retain(replacement);
    release(previous);
    state_abort(reservation);
    return STATE_OK;
}
