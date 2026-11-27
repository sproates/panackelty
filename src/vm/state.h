#ifndef PANACKELTY_STATE_H
#define PANACKELTY_STATE_H

#include "value.h"

/* Scheduler-owned reservations: zero-initialize, never copy or move while held,
 * and abort on every cancellation/trap/destruction path. No OS lock is held. */
typedef struct { Value *cell; } StateReservation;
typedef enum { STATE_OK, STATE_BUSY, STATE_INVALID, STATE_OUT_OF_MEMORY } StateStatus;

Value *state_create(Value *initial, StateStatus *status);
/* Successful reads return one owned snapshot; BUSY never exposes the payload. */
Value *state_read(Value *cell, StateStatus *status);
StateStatus state_reserve(Value *cell, StateReservation *reservation);
Value *state_reserved_read(const StateReservation *reservation);
/* Success atomically replaces the payload and releases the reservation.
 * Failure leaves both unchanged; the execution must abort while unwinding. */
StateStatus state_commit(StateReservation *reservation, Value *replacement);
void state_abort(StateReservation *reservation);

#endif
