#include "http_runtime.h"
#include "http_state.h"
#include "tcp_server.h"

#include <stdlib.h>
#include <string.h>

struct VMHttpResource {
    Value *handle;
    VMTcpServer *server;
    bool close_requested, destroy_requested;
    VMHttpResource *next;
};

static Value *outcome(const char *error)
{
    Value *payload = error ? value_data(V_STR, (const uint8_t *)error, strlen(error)) : value_new(V_UNIT);
    if (!payload) return NULL;
    Value *result = named_value(V_VARIANT, error ? "Error" : "Ok", NULL, &payload, 1);
    release(payload);
    return result;
}

Value *http_runtime_listen(VM *vm, Value *handle, Function *worker, bool contextual,
                           bool outbound, bool filesystem, bool console)
{
    if (!vm->http_runtime) return outcome("HTTP resource runtime is unavailable");
    if (handle->kind != V_HTTP_STATE || !handle->as.http_state) {
        vm->error = "VM trap: invalid HTTP listener handle";
        return NULL;
    }
    VMHttpState *state = handle->as.http_state;
    if (state->phase != HTTP_CREATED) return outcome("HTTP server is no longer Created");
    if (state->setup_error) return outcome(state->setup_error);
    VMHttpResource *resource = calloc(1, sizeof(*resource));
    if (!resource) return NULL;
    state->contextual = contextual;
    resource->server = tcp_server_start_http(vm, worker, handle, outbound);
    if (!resource->server) { free(resource); return NULL; }
    if (tcp_server_error(resource->server)) {
        vm->error = tcp_server_error(resource->server);
        tcp_server_destroy(resource->server);
        free(resource);
        return NULL;
    }
    if (tcp_server_done(resource->server)) {
        Value *result = retain(tcp_server_result(resource->server));
        tcp_server_destroy(resource->server);
        free(resource);
        return result;
    }
    if (filesystem) tcp_server_enable_filesystem(resource->server);
    if (console) tcp_server_enable_console(resource->server);
    resource->handle = retain(handle);
    resource->next = vm->http_runtime->pending;
    vm->http_runtime->pending = resource;
    return outcome(NULL);
}

bool http_runtime_request_close(struct VMHttpRuntime *runtime, Value *handle, bool destroy)
{
    VMHttpState *state = handle->as.http_state;
    if (!state->resource) {
        if (destroy) state->phase = HTTP_DESTROYED;
        else if (state->phase == HTTP_CREATED) state->phase = HTTP_CLOSED;
        return true;
    }
    if (!runtime) return false;
    VMHttpResource *lists[] = {runtime->active, runtime->pending};
    for (size_t list = 0; list < 2; list++) {
        for (VMHttpResource *item = lists[list]; item; item = item->next) {
            if (item->handle == handle) {
                item->close_requested = true;
                item->destroy_requested = item->destroy_requested || destroy;
                state->phase = item->destroy_requested ? HTTP_DESTROYED : HTTP_CLOSING;
                return true;
            }
        }
    }
    return false;
}

bool http_runtime_active(const struct VMHttpRuntime *runtime)
{
    return runtime && (runtime->active || runtime->pending);
}

static void release_resource(VMHttpResource *resource)
{
    tcp_server_destroy(resource->server);
    release(resource->handle);
    free(resource);
}

void http_runtime_pump(struct VMHttpRuntime *runtime, size_t budget, unsigned wait_ms)
{
    while (runtime->pending) {
        VMHttpResource *resource = runtime->pending;
        runtime->pending = resource->next;
        resource->next = runtime->active;
        runtime->active = resource;
    }
    VMHttpResource **cursor = &runtime->active;
    bool waited = false;
    while (*cursor) {
        VMHttpResource *resource = *cursor;
        if (resource->close_requested) tcp_server_stop(resource->server);
        if (!resource->destroy_requested) {
            tcp_server_poll(resource->server, waited ? 0 : wait_ms);
            waited = true;
            tcp_server_advance(resource->server, budget);
        }
        if (tcp_server_error(resource->server)) runtime->error = tcp_server_error(resource->server);
        if (!resource->destroy_requested && tcp_server_done(resource->server) && !runtime->error) {
            Value *result = tcp_server_result(resource->server);
            if (result && result->kind == V_VARIANT && !strcmp(result->as.named.name, "Error")) {
                runtime->error = "HTTP listener failed";
            }
        }
        if (resource->destroy_requested || tcp_server_done(resource->server)) {
            *cursor = resource->next;
            release_resource(resource);
        } else cursor = &resource->next;
        if (runtime->error) break;
    }
}

void http_runtime_stop(struct VMHttpRuntime *runtime)
{
    VMHttpResource *lists[] = {runtime->active, runtime->pending};
    for (size_t list = 0; list < 2; list++) {
        for (VMHttpResource *resource = lists[list]; resource; resource = resource->next) {
            resource->close_requested = true;
            VMHttpState *state = resource->handle->as.http_state;
            if (state->phase != HTTP_DESTROYED) state->phase = HTTP_CLOSING;
        }
    }
}

void http_runtime_destroy(struct VMHttpRuntime *runtime)
{
    VMHttpResource *lists[] = {runtime->active, runtime->pending};
    for (size_t list = 0; list < 2; list++) {
        VMHttpResource *resource = lists[list];
        while (resource) {
            VMHttpResource *next = resource->next;
            release_resource(resource);
            resource = next;
        }
    }
    runtime->active = NULL;
    runtime->pending = NULL;
}
