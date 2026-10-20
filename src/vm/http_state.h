#ifndef PANACKELTY_HTTP_STATE_H
#define PANACKELTY_HTTP_STATE_H

#include "value.h"

/* Runtime-only identity; never encoded as a language integer or bytecode constant.
 * Value owns these shells. Active resources retain the Value, not a second shell.
 * Package methods provide URI/header policy and retain concrete callback types. */
typedef enum {
    HTTP_CREATED, HTTP_LISTENING, HTTP_CLOSING, HTTP_CLOSED, HTTP_DESTROYED
} VMHttpPhase;

typedef struct {
    Value *method, *path, *handler;
} VMHttpRoute;

struct VMHttpState {
    VMHttpPhase phase;
    bool contextual;
    Value *address, *port, *context, *fallback;
    VMHttpRoute *routes;
    size_t route_count, route_capacity, admitted;
    const char *setup_error;
    void *resource; /* Non-owning; resource clears this before releasing its owner. */
};

struct VMHttpWriter {
    bool ended, completed;
    size_t status;
    Value *headers, *wire;
};

struct VMHttpConnection {
    bool ended, admitted;
    VMHttpState *server; /* Borrowed only while !ended. */
    void *client;
    struct VMExecution *execution;
    Value *writer;
    uint8_t *body;
    size_t body_length, body_capacity;
};

VMHttpState *http_state_create(Value *address, Value *port, Value *context);
void http_state_free(VMHttpState *state);
/* Returns false only for allocation failure. Invalid/duplicate setup is sticky.
 * Caller must reject non-Created state before invoking this operation. */
bool http_state_register(VMHttpState *state, Value *method, Value *path, Value *handler);
void http_state_set_error(VMHttpState *state, const char *error);
VMHttpWriter *http_writer_create(void);
void http_writer_free(VMHttpWriter *writer);
void http_connection_free(VMHttpConnection *connection);
/* Invalidate before recycling the client slot; retained aliases stay expired. */
void http_connection_end(VMHttpConnection *connection);

#endif
