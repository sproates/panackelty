#include "http_state.h"

#include <stdlib.h>
#include <string.h>

static bool valid_address(const Value *address)
{
    if (address->kind != V_STR) return false;
    const uint8_t *text = address->as.bytes.data;
    size_t length = address->as.bytes.length, cursor = 0;
    for (size_t octet = 0; octet < 4; octet++) {
        size_t start = cursor, number = 0;
        while (cursor < length && text[cursor] >= '0' && text[cursor] <= '9') {
            number = number * 10 + (text[cursor++] - '0');
            if (number > 255 || cursor - start > 3) return false;
        }
        if (cursor == start || (cursor - start > 1 && text[start] == '0')) return false;
        if (octet < 3 && (cursor == length || text[cursor++] != '.')) return false;
    }
    return cursor == length;
}

void http_state_set_error(VMHttpState *state, const char *error)
{
    if (!state->setup_error) state->setup_error = error;
}

VMHttpState *http_state_create(Value *address, Value *port, Value *context)
{
    VMHttpState *state = calloc(1, sizeof(*state));
    if (!state) return NULL;
    state->address = retain(address);
    state->port = retain(port);
    state->context = retain(context);
    size_t number;
    if (!valid_address(address)) {
        http_state_set_error(state, "invalid IPv4 bind address");
    }
    if (port->kind != V_NAT || !value_index(port, &number) || !number || number > 65535) {
        http_state_set_error(state, "port must be in 1..65535");
    }
    return state;
}

bool http_state_register(VMHttpState *state, Value *method, Value *path, Value *handler)
{
    if (!method->as.bytes.length) {
        if (state->fallback) http_state_set_error(state, "duplicate fallback");
        else state->fallback = retain(handler);
        return true;
    }
    for (size_t i = 0; i < state->route_count; i++) {
        if (value_equal(method, state->routes[i].method) &&
            value_equal(path, state->routes[i].path)) {
            http_state_set_error(state, "duplicate method and path");
            return true;
        }
    }
    if (state->route_count == state->route_capacity) {
        size_t capacity = state->route_capacity ? state->route_capacity * 2 : 8;
        if (capacity < state->route_capacity || capacity > SIZE_MAX / sizeof(VMHttpRoute)) {
            return false;
        }
        VMHttpRoute *next = realloc(state->routes, capacity * sizeof(*next));
        if (!next) return false;
        state->routes = next;
        state->route_capacity = capacity;
    }
    state->routes[state->route_count++] = (VMHttpRoute){
        retain(method), retain(path), retain(handler)
    };
    return true;
}

void http_state_free(VMHttpState *state)
{
    if (!state) return;
    release(state->address);
    release(state->port);
    release(state->context);
    release(state->fallback);
    for (size_t i = 0; i < state->route_count; i++) {
        release(state->routes[i].method);
        release(state->routes[i].path);
        release(state->routes[i].handler);
    }
    free(state->routes);
    free(state);
}

VMHttpWriter *http_writer_create(void)
{
    VMHttpWriter *writer = calloc(1, sizeof(*writer));
    if (!writer) return NULL;
    writer->status = 200;
    writer->headers = value_sequence(V_ARRAY, NULL, 0);
    if (!writer->headers) { free(writer); return NULL; }
    return writer;
}

void http_writer_free(VMHttpWriter *writer)
{
    if (!writer) return;
    release(writer->headers);
    release(writer->wire);
    free(writer);
}

void http_connection_end(VMHttpConnection *connection)
{
    if (!connection) return;
    if (connection->admitted && connection->server) {
        connection->server->admitted--;
        connection->admitted = false;
    }
    connection->ended = true;
    connection->server = NULL;
    connection->client = NULL;
    connection->execution = NULL;
    if (connection->writer && connection->writer->as.http_writer) {
        connection->writer->as.http_writer->ended = true;
    }
}

void http_connection_free(VMHttpConnection *connection)
{
    if (!connection) return;
    http_connection_end(connection);
    release(connection->writer);
    free(connection->body);
    free(connection);
}
