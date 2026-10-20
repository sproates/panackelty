#include "builtins_internal.h"
#include "http_state.h"
#include <stdlib.h>

/* These operations are emitted only by the designated toolchain wrapper.
 * Runtime checks remain necessary because bytecode is untrusted. */
Value *builtins_http_state_call(VM *vm, const char *name, Value **a)
{
    if (!strcmp(name, "http_state_create")) {
        REQUIRE(a[0]->kind == V_STR && a[1]->kind == V_NAT,
                "VM trap: invalid HTTP state constructor operands");
        Value *value = value_new(V_HTTP_STATE);
        if (!value) return NULL;
        value->as.http_state = http_state_create(a[0], a[1], a[2]);
        if (!value->as.http_state) { release(value); return NULL; }
        return value;
    }
    REQUIRE(a[0]->kind == V_HTTP_STATE && a[0]->as.http_state,
            "VM trap: invalid HTTP server handle");
    VMHttpState *state = a[0]->as.http_state;
    if (!strcmp(name, "http_state_context")) return retain(state->context);
    if (!strcmp(name, "http_state_setup_error")) {
        Value *message = state->setup_error
            ? value_data(V_STR, (const uint8_t *)state->setup_error, strlen(state->setup_error))
            : NULL;
        if (state->setup_error && !message) return NULL;
        Value *result = named_value(V_VARIANT, message ? "Some" : "None", NULL,
                                    message ? &message : NULL, message ? 1 : 0);
        release(message);
        return result;
    }
    if (state->phase != HTTP_CREATED) {
        vm->fault_kind = VM_FAULT_REQUEST;
        vm->error = "VM trap: HTTP registration requires a Created server";
        return NULL;
    }
    if (!strcmp(name, "http_state_fail_setup")) {
        http_state_set_error(state, "invalid route URI reference");
        return value_new(V_UNIT);
    }
    if (!strcmp(name, "http_state_register")) {
        REQUIRE(a[1]->kind == V_STR && a[2]->kind == V_STR && a[3]->kind == V_STR,
                "VM trap: invalid HTTP registration operands");
        if (!http_state_register(state, a[1], a[2], a[3])) return NULL;
        return value_new(V_UNIT);
    }
    vm->error = "VM trap: unknown HTTP state operation";
    return NULL;
}

Value *builtins_http_writer_call(VM *vm, const char *name, Value **a)
{
    REQUIRE(a[0]->kind == V_HTTP_WRITER && a[0]->as.http_writer,
            "VM trap: invalid HTTP response writer");
    VMHttpWriter *writer = a[0]->as.http_writer;
    if (writer->ended) {
        vm->fault_kind = VM_FAULT_REQUEST;
        vm->error = "VM trap: HTTP response writer has expired";
        return NULL;
    }
    if (!strcmp(name, "http_writer_get_status")) return value_size(writer->status);
    if (!strcmp(name, "http_writer_get_headers")) return retain(writer->headers);
    if (writer->completed) {
        vm->fault_kind = VM_FAULT_REQUEST;
        vm->error = "VM trap: HTTP response already completed";
        return NULL;
    }
    if (!strcmp(name, "http_writer_reject")) {
        vm->fault_kind = VM_FAULT_REQUEST;
        vm->error = "VM trap: invalid HTTP response";
        return NULL;
    }
    if (!strcmp(name, "http_writer_status")) {
        REQUIRE(a[1]->kind == V_NAT, "VM trap: invalid HTTP status operand");
        size_t status;
        if (!value_index(a[1], &status) || status < 200 || status > 599) {
            vm->fault_kind = VM_FAULT_REQUEST;
            vm->error = "VM trap: HTTP status must be in 200..599";
            return NULL;
        }
        writer->status = status;
        return value_new(V_UNIT);
    }
    if (!strcmp(name, "http_writer_headers")) {
        REQUIRE(a[1]->kind == V_ARRAY, "VM trap: invalid HTTP headers operand");
        for (size_t i = 0; i < a[1]->as.sequence.count; i++) {
            Value *pair = a[1]->as.sequence.items[i];
            REQUIRE(pair->kind == V_ARRAY && pair->as.sequence.count == 2 &&
                    pair->as.sequence.items[0]->kind == V_STR &&
                    pair->as.sequence.items[1]->kind == V_STR,
                    "VM trap: invalid HTTP header pair");
        }
        Value *headers = retain(a[1]);
        release(writer->headers);
        writer->headers = headers;
        return value_new(V_UNIT);
    }
    if (!strcmp(name, "http_writer_commit")) {
        REQUIRE(a[1]->kind == V_BYTES, "VM trap: invalid HTTP wire operand");
        writer->wire = retain(a[1]);
        writer->completed = true;
        return value_new(V_UNIT);
    }
    vm->error = "VM trap: unknown HTTP writer operation";
    return NULL;
}

VMHttpConnection *http_live_connection(VM *vm, Value *value)
{
    REQUIRE(value->kind == V_HTTP_CONNECTION && value->as.http_connection,
            "VM trap: invalid HTTP connection handle");
    VMHttpConnection *connection = value->as.http_connection;
    if (connection->ended || !connection->server || connection->execution != vm->execution) {
        vm->fault_kind = VM_FAULT_REQUEST;
        vm->error = "VM trap: HTTP connection has expired or belongs to another request";
        return NULL;
    }
    return connection;
}

Value *builtins_http_connection_call(VM *vm, const char *name, Value **a)
{
    if (!strcmp(name, "http_bytes_slice")) {
        REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_NAT && a[2]->kind == V_NAT,
                "VM trap: invalid HTTP byte slice operands");
        size_t start, end;
        if (!value_index(a[1], &start) || !value_index(a[2], &end) || start > end || end > a[0]->as.bytes.length) {
            vm->fault_kind = VM_FAULT_REQUEST;
            vm->error = "VM trap: invalid HTTP byte slice";
            return NULL;
        }
        return value_data(V_BYTES, end == start ? NULL : a[0]->as.bytes.data + start, end - start);
    }
    VMHttpConnection *connection = http_live_connection(vm, a[0]);
    if (!connection) return NULL;
    VMHttpState *state = connection->server;
    if (!strcmp(name, "http_body_size")) return value_size(connection->body_length);
    if (!strcmp(name, "http_body")) return value_data(V_BYTES, connection->body, connection->body_length);
    if (!strcmp(name, "http_body_append")) {
        REQUIRE(connection->admitted && a[1]->kind == V_BYTES && a[2]->kind == V_NAT && a[3]->kind == V_NAT,
                "VM trap: invalid HTTP body append operands");
        size_t start, end;
        REQUIRE(value_index(a[2], &start) && value_index(a[3], &end) && start <= end && end <= a[1]->as.bytes.length,
                "VM trap: invalid HTTP body range");
        size_t length = end - start;
        if (length > 1048576 - connection->body_length) return value_bool(false);
        size_t required = connection->body_length + length;
        if (required > connection->body_capacity) {
            size_t capacity = connection->body_capacity ? connection->body_capacity : 16384;
            while (capacity < required) capacity *= 2;
            uint8_t *next = realloc(connection->body, capacity);
            if (!next) return NULL;
            connection->body = next;
            connection->body_capacity = capacity;
        }
        if (length) memcpy(connection->body + connection->body_length, a[1]->as.bytes.data + start, length);
        connection->body_length = required;
        return value_bool(true);
    }
    if (!strcmp(name, "http_connection_writer")) return retain(connection->writer);
    if (!strcmp(name, "http_connection_fallback")) return value_bool(state->fallback != NULL);
    if (!strcmp(name, "http_connection_admit")) {
        if (connection->admitted) return value_bool(true);
        if (state->admitted == 4) return value_bool(false);
        connection->admitted = true;
        state->admitted++;
        return value_bool(true);
    }
    if (!strcmp(name, "http_connection_routes")) {
        Value *routes = value_sequence(V_ARRAY, NULL, 0);
        if (!routes) return NULL;
        for (size_t i = 0; i < state->route_count; i++) {
            Value *fields[] = {state->routes[i].method, state->routes[i].path};
            Value *pair = value_sequence(V_ARRAY, fields, 2);
            Value *next = pair ? value_array_append(routes, pair) : NULL;
            release(pair);
            release(routes);
            if (!next) return NULL;
            routes = next;
        }
        return routes;
    }
    vm->error = "VM trap: unknown HTTP connection operation";
    return NULL;
}

Function *http_connection_handler(VM *vm, Value **a, Value **call_args)
{
    VMHttpConnection *connection = http_live_connection(vm, a[0]);
    if (!connection) return NULL;
    REQUIRE(connection->admitted, "VM trap: HTTP handler invoked without admission");
    REQUIRE(a[1]->kind == V_VARIANT, "VM trap: invalid HTTP route selection");
    VMHttpState *state = connection->server;
    Value *handler = NULL;
    if (!strcmp(a[1]->as.named.name, "None") && !a[1]->as.named.count) {
        handler = state->fallback;
    } else if (!strcmp(a[1]->as.named.name, "Some") && a[1]->as.named.count == 1) {
        size_t index;
        REQUIRE(a[1]->as.named.values[0]->kind == V_NAT &&
                value_index(a[1]->as.named.values[0], &index) && index < state->route_count,
                "VM trap: invalid HTTP route index");
        handler = state->routes[index].handler;
    }
    REQUIRE(handler && handler->kind == V_STR, "VM trap: missing HTTP handler");
    Function *target = NULL;
    for (size_t i = 0; i < vm->program->count; i++) {
        Function *candidate = &vm->program->functions[i];
        if (strlen(candidate->name) == handler->as.bytes.length &&
            !memcmp(candidate->name, handler->as.bytes.data, handler->as.bytes.length)) {
            target = candidate;
            break;
        }
    }
    REQUIRE(target && target->is_async && !target->pure &&
            target->param_count == (state->contextual ? 3u : 2u),
            "VM trap: invalid HTTP handler effect or arity");
    size_t index = 0;
    if (state->contextual) call_args[index++] = state->context;
    call_args[index++] = a[2];
    call_args[index] = a[3];
    return target;
}
