#include "platform.h"

#include "numeric.h"
#include "http_state.h"
#include "http_runtime.h"
#include "builtins.h"
#include "console_write.h"
#include "tcp_server.h"

#include <stdlib.h>
#include <string.h>

#ifndef __wasi__
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#endif

#define SERVER_CLIENTS 256
#define SERVER_CONCURRENCY 32
#define SERVER_BYTES (1024 * 1024)
#define SERVER_READ_CHUNK 16384
#define SERVER_TIMEOUT 60000

typedef enum { CLIENT_FREE, CLIENT_READ, CLIENT_START, CLIENT_RUN, CLIENT_STREAM_READ, CLIENT_INTERIM_WRITE, CLIENT_WRITE } ClientState;

typedef struct {
    ClientState state;
    int fd;
    size_t outcome, received, sent;
    uint64_t deadline;
    uint8_t *request;
    Value *response, *http_connection;
    size_t interim_sent;
    VM vm;
    VMExecution *execution;
    bool fake_pending, fake_fail;
    size_t read_limit;
    bool stream_eof;
} ServerClient;

struct VMTcpServer {
    bool filesystem_enabled, console_enabled, continuous, routed;
    bool ready_pending, ready_console;
    Function *ready_handler;
    Value *ready_context;
    VM ready_vm;
    VMExecution *ready_execution;
    size_t succeeded, failed;
    VM template_vm;
    Function *handler;
    Value *context;
    VMTcpServerLimits limits;
    ServerClient clients[SERVER_CONCURRENCY];
    Value *outcomes[SERVER_CLIENTS], *result;
    size_t admitted, active, next_io, next_handler;
    uint64_t admission_deadline, drain_deadline;
    int listener;
    bool outbound, streaming, stopping, done;
    const char *error;
};

static void close_fd(int *fd)
{
#ifndef __wasi__
    if (*fd >= 0) {
        close(*fd);
    }
#endif
    *fd = -1;
}

static void client_clear(ServerClient *client)
{
    close_fd(&client->fd);
    if (client->http_connection) http_connection_end(client->http_connection->as.http_connection);
    vm_execution_destroy(client->execution);
    vm_coverage_run_close(client->vm.coverage_run);
    free(client->request);
    release(client->response);
    release(client->http_connection);
    *client = (ServerClient){.fd = -1};
}

static void clear_ready(VMTcpServer *server)
{
    vm_execution_destroy(server->ready_execution);
    server->ready_execution = NULL;
    vm_coverage_run_close(server->ready_vm.coverage_run);
    server->ready_vm.coverage_run = NULL;
    server->ready_pending = false;
}

static void close_all(VMTcpServer *server)
{
    clear_ready(server);
    close_fd(&server->listener);
    for (size_t i = 0; i < SERVER_CONCURRENCY; i++) {
        client_clear(&server->clients[i]);
    }
    server->active = 0;
}

static void trap(VMTcpServer *server, const char *error)
{
    server->error = error;
    server->done = true;
    close_all(server);
}

static Value *result_value(bool ok, Value *payload)
{
    if (!payload) {
        return NULL;
    }
    Value *result = named_value(V_VARIANT, ok ? "Ok" : "Error", NULL, &payload, 1);
    release(payload);
    return result;
}

static Value *error_value(const char *error)
{
    return result_value(false, value_data(V_STR, (const uint8_t *)error, strlen(error)));
}

static void fail(VMTcpServer *server, const char *error)
{
    server->result = error_value(error);
    server->done = true;
    close_all(server);
    if (!server->result) {
        trap(server, "native VM out of memory");
    }
}

#ifndef __wasi__
static void finish_client(VMTcpServer *server, ServerClient *client, const char *error)
{
    if (server->continuous) {
        size_t *counter = error ? &server->failed : &server->succeeded;
        if (*counter < SIZE_MAX) {
            (*counter)++;
        }
        client_clear(client);
        server->active--;
        return;
    }
    Value *outcome = error ? error_value(error) : result_value(true, value_new(V_UNIT));
    if (!outcome) {
        trap(server, "native VM out of memory");
        return;
    }
    server->outcomes[client->outcome] = outcome;
    client_clear(client);
    server->active--;
}

static const char http_internal_error[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 22\r\nConnection: close\r\n\r\nInternal Server Error\n";
static const char http_timeout_error[] = "HTTP/1.1 504 Gateway Timeout\r\nContent-Length: 16\r\nConnection: close\r\n\r\nGateway Timeout\n";

static void http_log(const uint8_t *message, size_t length)
{
    /* Reuse bounded detached host writes: a blocked stderr must never stall
     * the listener or extend a request deadline. Logging is best effort. */
    if (length > 512) length = 512;
    Value *text = value_data(V_STR, message, length);
    if (!text) return;
    VM logging = {0};
    VMConsoleWrite *operation = console_write_start(&logging, text, true);
    release(text);
    if (operation) console_write_destroy(operation);
}

static void http_attach_response(ServerClient *client)
{
    if (!client->response && client->http_connection) {
        VMHttpWriter *writer = client->http_connection->as.http_connection->writer->as.http_writer;
        if (writer->completed) client->response = retain(writer->wire);
    }
}

static void http_deadline_response(ServerClient *client)
{
    if (client->fd < 0) return;
    http_attach_response(client);
    const uint8_t *data = (const uint8_t *)http_timeout_error;
    size_t length = sizeof(http_timeout_error) - 1;
    if (client->response) {
        data = client->response->as.bytes.data + client->sent;
        length = client->response->as.bytes.length - client->sent;
    }
#ifdef MSG_NOSIGNAL
    (void)send(client->fd, data, length, MSG_NOSIGNAL);
#else
    (void)send(client->fd, data, length, 0);
#endif
}

static void http_handler_finished(VMTcpServer *server, ServerClient *client, bool trapped)
{
    VMHttpWriter *writer = client->http_connection->as.http_connection->writer->as.http_writer;
    if (trapped) {
        const char *message = client->vm.error ? client->vm.error : "HTTP request failed";
        http_log((const uint8_t *)message, strlen(message));
    } else {
        Value *result = vm_execution_result(client->execution);
        if (!result || result->kind != V_VARIANT || result->as.named.count != 1) {
            trap(server, "VM trap: invalid HTTP worker result");
            return;
        }
        Value *payload = result->as.named.values[0];
        if (!strcmp(result->as.named.name, "Error") && payload->kind == V_STR) {
            http_log(payload->as.bytes.data, payload->as.bytes.length);
        } else if (strcmp(result->as.named.name, "Ok") || payload->kind != V_UNIT) {
            trap(server, "VM trap: invalid HTTP worker result");
            return;
        } else if (!writer->completed) {
            static const char missing[] = "HTTP handler returned without sending a response";
            http_log((const uint8_t *)missing, sizeof(missing) - 1);
        }
    }
    if (!writer->completed) {
        writer->wire = value_data(V_BYTES, (const uint8_t *)http_internal_error, sizeof(http_internal_error) - 1);
        if (!writer->wire) { trap(server, "native VM out of memory"); return; }
        writer->completed = true;
    }
    http_attach_response(client);
    writer->ended = true;
    client->http_connection->as.http_connection->execution = NULL;
    vm_execution_destroy(client->execution);
    client->execution = NULL;
    if (client->sent == client->response->as.bytes.length) finish_client(server, client, NULL);
    else client->state = CLIENT_WRITE;
}

static bool clock_ms(uint64_t *now)
{
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0 || time.tv_sec < 0) {
        return false;
    }
    *now = (uint64_t)time.tv_sec * 1000 + (uint64_t)time.tv_nsec / 1000000;
    return *now <= UINT64_MAX - SERVER_TIMEOUT;
}

static bool configure(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0 || fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
        return false;
    }
#ifdef SO_NOSIGPIPE
    int enabled = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled)) < 0) {
        return false;
    }
#endif
    return true;
}

static void stop_at(VMTcpServer *server, uint64_t now)
{
    if (server->stopping) {
        return;
    }
    server->stopping = true;
    server->drain_deadline = now + server->limits.drain_ms;
    close_fd(&server->listener);
}

static bool deadlines(VMTcpServer *server, uint64_t *now)
{
    if (server->done) {
        return false;
    }
    if (!clock_ms(now)) {
        fail(server, "TCP server clock failed");
        return false;
    }
    if (!server->continuous && !server->stopping && *now >= server->admission_deadline) {
        stop_at(server, server->admission_deadline);
    }
    for (size_t i = 0; i < server->limits.concurrency; i++) {
        ServerClient *client = &server->clients[i];
        if (client->state == CLIENT_FREE) {
            continue;
        }
        if (*now >= client->deadline) {
            if (server->routed) http_deadline_response(client);
            finish_client(server, client, "TCP client timed out");
        } else if (server->stopping && *now >= server->drain_deadline) {
            finish_client(server, client, "TCP server drain cancelled client");
        }
        if (server->done) {
            return false;
        }
    }
    if (server->stopping && !server->active) {
        clear_ready(server);
        if (server->continuous) {
            char *names[] = {"accepted", "succeeded", "failed"};
            Value *values[] = {value_size(server->admitted), value_size(server->succeeded),
                               value_size(server->failed)};
            Value *summary = values[0] && values[1] && values[2]
                                 ? named_value(V_RECORD, "TcpServiceSummary", names, values, 3)
                                 : NULL;
            for (size_t i = 0; i < 3; i++) {
                release(values[i]);
            }
            server->result = result_value(true, summary);
        } else {
            server->result =
                result_value(true, value_sequence(V_ARRAY, server->outcomes, server->admitted));
        }
        server->done = true;
        if (!server->result) {
            trap(server, "native VM out of memory");
        }
        return false;
    }
    return true;
}
#endif

static VMTcpServer *tcp_server_start_mode(const VM *vm, Function *handler, const Value *context,
                                          const Value *address, size_t port,
                                          VMTcpServerLimits limits, bool outbound, bool streaming,
                                          bool continuous)
{
    VMTcpServer *server = calloc(1, sizeof(*server));
    if (!server) {
        return NULL;
    }
    server->listener = -1;
    for (size_t i = 0; i < SERVER_CONCURRENCY; i++) {
        server->clients[i].fd = -1;
    }
    server->limits = limits;
    server->continuous = continuous;
    bool found = false;
    if (vm && vm->program) {
        for (size_t i = 0; i < vm->program->count; i++) {
            if (handler == &vm->program->functions[i]) {
                found = true;
            }
        }
    }
    if (!found || !handler->is_async || handler->pure || handler->param_count != 1 ||
        (streaming && !context)) {
        trap(server, "VM trap: invalid TCP server handler");
        return server;
    }
    if (!address || address->kind != V_STR || !port || port > 65535 ||
        (!continuous && (!limits.clients || limits.clients > SERVER_CLIENTS ||
                         limits.concurrency > limits.clients)) ||
        !limits.concurrency || limits.concurrency > SERVER_CONCURRENCY ||
        limits.request_limit > SERVER_BYTES || limits.response_limit > SERVER_BYTES ||
        !limits.client_timeout_ms || limits.client_timeout_ms > SERVER_TIMEOUT ||
        (!continuous &&
         (!limits.admission_timeout_ms || limits.admission_timeout_ms > SERVER_TIMEOUT)) ||
        limits.drain_ms > SERVER_TIMEOUT) {
        fail(server, "invalid TCP server arguments");
        return server;
    }
    server->template_vm = *vm;
    if (!vm->coverage_run) vm_coverage_gap(vm->coverage);
    server->template_vm.coverage = NULL;
    server->handler = handler;
    server->context = retain((Value *)context);
    server->outbound = outbound;
    server->streaming = streaming;
#ifdef __wasi__
    fail(server, "TCP server is unavailable on this host");
#else
    char text[INET_ADDRSTRLEN];
    size_t length = address->as.bytes.length;
    if (!length || length >= sizeof(text) || memchr(address->as.bytes.data, 0, length)) {
        fail(server, "invalid IPv4 address");
        return server;
    }
    memcpy(text, address->as.bytes.data, length);
    text[length] = 0;
    struct sockaddr_in bind_address = {.sin_family = AF_INET, .sin_port = htons((uint16_t)port)};
    if (inet_pton(AF_INET, text, &bind_address.sin_addr) != 1) {
        fail(server, "invalid IPv4 address");
        return server;
    }
    uint64_t now;
    if (!clock_ms(&now)) {
        fail(server, "TCP server clock failed");
        return server;
    }
    server->admission_deadline = now + limits.admission_timeout_ms;
    int reuse = 1;
    server->listener = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listener < 0 || !configure(server->listener) ||
        setsockopt(server->listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        fail(server, "TCP server socket failed");
    } else if (bind(server->listener, (struct sockaddr *)&bind_address, sizeof(bind_address)) < 0) {
        fail(server, "TCP server bind failed");
    } else if (listen(server->listener, (int)limits.concurrency) < 0) {
        fail(server, "TCP server listen failed");
    } else if (!clock_ms(&now)) {
        fail(server, "TCP server clock failed");
    } else {
        server->admission_deadline = now + limits.admission_timeout_ms;
    }
#endif
    return server;
}

VMTcpServer *tcp_server_start(const VM *vm, Function *handler, const Value *address, size_t port,
                              VMTcpServerLimits limits, bool outbound)
{
    return tcp_server_start_mode(vm, handler, NULL, address, port, limits, outbound, false, false);
}

VMTcpServer *tcp_server_start_stream(const VM *vm, Function *handler, const Value *context,
                                     const Value *address, size_t port,
                                     VMTcpServerLimits limits, bool outbound)
{
    return tcp_server_start_mode(vm, handler, context, address, port, limits, outbound, true,
                                 false);
}

VMTcpServer *tcp_server_start_service(const VM *vm, Function *handler, const Value *context,
                                      const Value *address, size_t port, VMTcpServiceLimits limits,
                                      bool outbound)
{
    VMTcpServerLimits normalized = {0,
                                    limits.concurrency,
                                    limits.request_limit,
                                    limits.response_limit,
                                    limits.client_timeout_ms,
                                    0,
                                    limits.drain_ms};
    return tcp_server_start_mode(vm, handler, context, address, port, normalized, outbound, true,
                                 true);
}

VMTcpServer *tcp_server_start_http(const VM *vm, Function *worker, Value *handle, bool outbound)
{
    if (!handle || handle->kind != V_HTTP_STATE || !handle->as.http_state) return NULL;
    VMHttpState *state = handle->as.http_state;
    size_t port;
    if (state->phase != HTTP_CREATED || state->setup_error || !value_index(state->port, &port)) return NULL;
    VMTcpServerLimits limits = {16, 16, SERVER_BYTES, SERVER_BYTES, 10000, 10000, 1000};
    VMTcpServer *server = tcp_server_start_mode(vm, worker, handle, state->address, port,
                                               limits, outbound, true, true);
    if (server && !server->done) {
        server->routed = true;
        state->phase = HTTP_LISTENING;
        state->resource = server;
    }
    return server;
}

#ifndef __wasi__
static void accept_client(VMTcpServer *server, uint64_t now)
{
    if (server->stopping || (!server->routed && server->active == server->limits.concurrency)) {
        return;
    }
    /* Admission sequence also identifies poll snapshots. Never wrap and risk
     * delivering an old descriptor's readiness to a reused client slot. */
    if (server->admitted == SIZE_MAX) {
        fail(server, "TCP service connection sequence exhausted");
        return;
    }
    size_t slot = 0;
    while (slot < server->limits.concurrency && server->clients[slot].state != CLIENT_FREE) slot++;
    int fd = accept(server->listener, NULL, NULL);
    if (fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR && errno != ECONNABORTED) {
            fail(server, "TCP server accept failed");
        }
        return;
    }
    if (slot == server->limits.concurrency) {
        static const char busy[] = "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 20\r\nConnection: close\r\n\r\nService Unavailable\n";
        if (configure(fd)) {
#ifdef MSG_NOSIGNAL
            (void)send(fd, busy, sizeof(busy) - 1, MSG_NOSIGNAL);
#else
            (void)send(fd, busy, sizeof(busy) - 1, 0);
#endif
        }
        close(fd);
        return;
    }
    ServerClient *client = &server->clients[slot];
    *client = (ServerClient){.state = server->streaming ? CLIENT_START : CLIENT_READ,
                             .fd = fd,
                             .outcome = server->admitted++,
                             .deadline = now + server->limits.client_timeout_ms};
    server->active++;
    if (!configure(fd)) {
        finish_client(server, client, "TCP client socket failed");
    } else {
        client->request = server->streaming
                              ? NULL
                              : malloc(server->limits.request_limit ? server->limits.request_limit : 1);
        if (!server->streaming && !client->request) {
            trap(server, "native VM out of memory");
        }
    }
    if (!server->continuous && !server->done && server->admitted == server->limits.clients) {
        stop_at(server, now);
    }
}

static void read_client(VMTcpServer *server, ServerClient *client)
{
    uint8_t bytes[16384];
    ssize_t count = recv(client->fd, bytes, sizeof(bytes), 0);
    if (count > 0) {
        if ((size_t)count > server->limits.request_limit - client->received) {
            finish_client(server, client, "TCP request limit exceeded");
        } else {
            memcpy(client->request + client->received, bytes, (size_t)count);
            client->received += (size_t)count;
        }
    } else if (!count) {
        client->state = CLIENT_START;
    } else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
        finish_client(server, client, "TCP client read failed");
    }
}

static void complete_stream_read(VMTcpServer *server, ServerClient *client, Value *result)
{
    client->state = CLIENT_RUN;
    client->read_limit = 0;
    if (!result || !vm_execution_complete_read(client->execution, result)) {
        release(result);
        trap(server, "VM trap: invalid TCP server read completion");
        return;
    }
    release(result);
}

static void read_stream_chunk(VMTcpServer *server, ServerClient *client)
{
    Value *payload = NULL;
    if (client->stream_eof) {
        payload = value_data(V_BYTES, NULL, 0);
    } else {
        uint8_t bytes[SERVER_READ_CHUNK];
        ssize_t count = recv(client->fd, bytes, client->read_limit, 0);
        if (count > 0) {
            if (!server->routed && (size_t)count > server->limits.request_limit - client->received) {
                client->stream_eof = true;
                complete_stream_read(server, client,
                                     error_value("TCP request limit exceeded"));
                return;
            }
            if (!server->routed) client->received += (size_t)count;
            payload = value_data(V_BYTES, bytes, (size_t)count);
        } else if (!count) {
            client->stream_eof = true;
            payload = value_data(V_BYTES, NULL, 0);
        } else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return;
        } else {
            complete_stream_read(server, client, error_value("TCP client read failed"));
            return;
        }
    }
    Value *result = result_value(true, payload);
    if (!result) {
        trap(server, "native VM out of memory");
        return;
    }
    complete_stream_read(server, client, result);
}

static void write_interim(VMTcpServer *server, ServerClient *client)
{
    static const char response[] = "HTTP/1.1 100 Continue\r\n\r\n";
#ifdef MSG_NOSIGNAL
    int flags = MSG_NOSIGNAL;
#else
    int flags = 0;
#endif
    ssize_t count = send(client->fd, response + client->interim_sent,
                         sizeof(response) - 1 - client->interim_sent, flags);
    if (count > 0) client->interim_sent += (size_t)count;
    else if (!count || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
        finish_client(server, client, "HTTP interim write failed");
        return;
    }
    if (client->interim_sent == sizeof(response) - 1) {
        complete_stream_read(server, client, result_value(true, value_data(V_BYTES, NULL, 0)));
    }
}

static void write_client(VMTcpServer *server, ServerClient *client)
{
    size_t remaining = client->response->as.bytes.length - client->sent;
    if (remaining) {
        if (remaining > 16384) {
            remaining = 16384;
        }
#ifdef MSG_NOSIGNAL
        int flags = MSG_NOSIGNAL;
#else
        int flags = 0;
#endif
        ssize_t count =
            send(client->fd, client->response->as.bytes.data + client->sent, remaining, flags);
        if (count > 0) {
            client->sent += (size_t)count;
        } else if (!count || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
            finish_client(server, client, "TCP client write failed");
            return;
        }
    }
    if (client->sent == client->response->as.bytes.length) {
        if (server->routed && client->execution) close_fd(&client->fd);
        else finish_client(server, client, NULL);
    }
}
#endif

void tcp_server_poll(VMTcpServer *server, unsigned max_wait_ms)
{
#ifndef __wasi__
    uint64_t now;
    if (!deadlines(server, &now)) {
        return;
    }
    if (server->ready_pending) {
        if (server->ready_execution) {
            vm_execution_poll_network(server->ready_execution, 0);
        }
        if (max_wait_ms) {
            poll(NULL, 0, 1);
        }
        deadlines(server, &now);
        return;
    }
    struct pollfd fds[SERVER_CONCURRENCY + 1];
    size_t slots[SERVER_CONCURRENCY + 1], identities[SERVER_CONCURRENCY + 1], count = 0;
    uint64_t until = server->stopping     ? server->drain_deadline
                     : server->continuous ? UINT64_MAX
                                          : server->admission_deadline;
    unsigned wait = max_wait_ms > INT_MAX ? INT_MAX : max_wait_ms;
    if (!server->stopping && (server->routed || server->active < server->limits.concurrency)) {
        fds[count] = (struct pollfd){.fd = server->listener, .events = POLLIN};
        slots[count++] = SERVER_CONCURRENCY;
    }
    for (size_t j = 0; j < server->limits.concurrency; j++) {
        size_t i = (server->next_io + j) % server->limits.concurrency;
        ServerClient *client = &server->clients[i];
        if (client->state == CLIENT_FREE) {
            continue;
        }
        if (client->deadline < until) {
            until = client->deadline;
        }
        if (server->routed) http_attach_response(client);
        if (client->state == CLIENT_START) {
            wait = 0;
        } else if (client->state == CLIENT_RUN) {
            vm_execution_poll_network(client->execution, 0);
            VMExecutionStatus status = vm_execution_advance(client->execution, 0);
            if (status != VM_WAITING || client->fake_pending) {
                wait = 0;
            }
            if (server->routed && client->fd >= 0) {
                fds[count] = (struct pollfd){.fd = client->fd, .events = client->response ? POLLOUT : 0};
                identities[count] = client->outcome;
                slots[count++] = i;
            }
            /* Outbound exchange descriptors belong to the child execution.
             * Recheck at 1ms while pending instead of blocking another client. */
            if (status == VM_WAITING && wait > 1) {
                wait = 1;
            }
        } else {
            fds[count] = (struct pollfd){.fd = client->fd,
                                         .events = (client->state == CLIENT_READ ||
                                                    client->state == CLIENT_STREAM_READ)
                                                       ? POLLIN
                                                       : POLLOUT};
            if (server->routed && client->response) fds[count].events |= POLLOUT;
            identities[count] = client->outcome;
            slots[count++] = i;
        }
    }
    if (until - now < wait) {
        wait = (unsigned)(until - now);
    }
    int ready = poll(fds, count, (int)wait);
    if (ready < 0) {
        if (errno != EINTR) {
            fail(server, "TCP server poll failed");
        }
        return;
    }
    /* Expiry wins over readiness, and may release slots represented by fds. */
    if (!deadlines(server, &now) || !ready) {
        return;
    }
    for (size_t i = 0; i < count && !server->done; i++) {
        /* Accepting the final client can start a zero-grace drain midway
         * through this readiness snapshot. Cancellation precedes further I/O. */
        if (!deadlines(server, &now)) {
            return;
        }
        if (!fds[i].revents) {
            continue;
        }
        if (slots[i] == SERVER_CONCURRENCY) {
            if (!server->stopping) {
                if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    fail(server, "TCP server listener failed");
                } else {
                    accept_client(server, now);
                }
            }
        } else {
            ServerClient *client = &server->clients[slots[i]];
            /* Acceptance above may reuse a just-expired slot. Never deliver its
             * predecessor's readiness to the new connection. */
            if (client->fd != fds[i].fd || client->outcome != identities[i] ||
                client->state == CLIENT_FREE) {
                continue;
            }
            if (server->routed && client->response && (fds[i].revents & POLLOUT)) {
                write_client(server, client);
                if (client->state == CLIENT_FREE || client->fd < 0) continue;
            }
            if ((fds[i].revents & POLLNVAL) ||
                (server->routed && (fds[i].revents & POLLERR)) ||
                (server->routed && client->state == CLIENT_RUN && (fds[i].revents & POLLHUP))) {
                finish_client(server, client, "TCP client socket failed");
            } else if (client->state == CLIENT_READ) {
                read_client(server, client);
            } else if (client->state == CLIENT_STREAM_READ) {
                read_stream_chunk(server, client);
            } else if (client->state == CLIENT_INTERIM_WRITE) {
                write_interim(server, client);
            } else if (!server->routed && client->state == CLIENT_WRITE) {
                write_client(server, client);
            }
        }
    }
    server->next_io = (server->next_io + 1) % server->limits.concurrency;
    deadlines(server, &now);
#else
    (void)server;
    (void)max_wait_ms;
#endif
}

#ifndef __wasi__
static bool server_read_wait(void *context, Value *value, const char **error)
{
    ServerClient *client = context;
    if (value->kind == V_BOOL) {
        client->fake_pending = true;
        client->fake_fail = value->as.boolean;
        return true;
    }
    size_t limit;
    if (!value_index(value, &limit) || !limit || limit > SERVER_READ_CHUNK ||
        client->state != CLIENT_RUN || !client->execution) {
        *error = "VM trap: invalid TCP server read limit";
        return false;
    }
    client->read_limit = limit;
    client->state = CLIENT_STREAM_READ;
    return true;
}

static void start_handler(VMTcpServer *server, ServerClient *client)
{
    Value *request = server->streaming
                         ? retain(server->context)
                         : value_data(V_BYTES, client->request, client->received);
    if (server->routed && request) {
        release(request);
        request = value_new(V_HTTP_CONNECTION);
        if (request) {
            request->as.http_connection = calloc(1, sizeof(VMHttpConnection));
            if (request->as.http_connection) {
                request->as.http_connection->server = server->context->as.http_state;
                request->as.http_connection->client = client;
                request->as.http_connection->writer = value_new(V_HTTP_WRITER);
                if (request->as.http_connection->writer) {
                    request->as.http_connection->writer->as.http_writer = http_writer_create();
                }
            }
            if (!request->as.http_connection || !request->as.http_connection->writer ||
                !request->as.http_connection->writer->as.http_writer) {
                release(request);
                request = NULL;
            }
        }
        client->http_connection = retain(request);
    }
    if (!request) {
        trap(server, "native VM out of memory");
        return;
    }
    client->vm = server->template_vm;
    client->vm.execution = NULL;
    client->vm.error = NULL;
    client->vm.coverage_run = vm_coverage_child(server->template_vm.coverage_run,
        client->vm.program, NULL, 0, "server\n");
    client->vm.coverage = vm_coverage_run_collector(client->vm.coverage_run);
    const char *error;
    client->execution = vm_execution_create_async(&client->vm, server->handler, &request, server_read_wait,
                                                  client, &error);
    if (client->http_connection) client->http_connection->as.http_connection->execution = client->execution;
    release(request);
    free(client->request);
    client->request = NULL;
    if (!client->execution) {
        trap(server, error);
    } else {
        if (server->streaming && !vm_execution_enable_server_reads(client->execution)) {
            trap(server, "VM trap: TCP server read capability setup failed");
            return;
        }
        if (server->filesystem_enabled) {
            vm_execution_enable_filesystem(client->execution);
        }
        if (server->console_enabled) {
            vm_execution_enable_console(client->execution);
        }
        if (server->outbound) {
            vm_execution_enable_tcp(client->execution);
        }
        client->state = CLIENT_RUN;
    }
}

static void handler_result(VMTcpServer *server, ServerClient *client)
{
    if (server->routed) { http_handler_finished(server, client, false); return; }
    Value *result = vm_execution_result(client->execution);
    if (!vm_fake_read_result_valid(result)) {
        trap(server, "VM trap: invalid TCP server handler result");
        return;
    }
    Value *payload = result->as.named.values[0];
    if (!strcmp(result->as.named.name, "Error")) {
        /* Preserve the handler's string exactly, including embedded NUL bytes. */
        if (server->continuous) {
            finish_client(server, client, "TCP handler returned an error");
        } else {
            server->outcomes[client->outcome] = retain(result);
            client_clear(client);
            server->active--;
        }
    } else if (payload->as.bytes.length > server->limits.response_limit) {
        finish_client(server, client, "TCP response limit exceeded");
    } else {
        client->response = retain(payload);
        vm_execution_destroy(client->execution);
        client->execution = NULL;
        client->state = CLIENT_WRITE;
    }
}
#endif

bool tcp_server_set_ready_handler(VMTcpServer *server, Function *handler, const Value *context,
                                  bool console)
{
    if (!server || server->done || !server->continuous || server->admitted ||
        server->ready_handler || !context) {
        return false;
    }
    bool found = false;
    for (size_t i = 0; i < server->template_vm.program->count; i++) {
        if (handler == &server->template_vm.program->functions[i]) {
            found = true;
        }
    }
    if (!found || !handler->is_async || handler->pure || handler->param_count != 1) {
        return false;
    }
    server->ready_handler = handler;
    server->ready_context = retain((Value *)context);
    server->ready_console = console;
    server->ready_pending = true;
    return true;
}

#ifndef __wasi__
static void advance_ready(VMTcpServer *server, size_t budget)
{
    if (!server->ready_execution) {
        server->ready_vm = server->template_vm;
        server->ready_vm.execution = NULL;
        server->ready_vm.error = NULL;
        server->ready_vm.coverage_run = vm_coverage_child(
            server->template_vm.coverage_run, server->ready_vm.program, NULL, 0, "server-ready\n");
        server->ready_vm.coverage = vm_coverage_run_collector(server->ready_vm.coverage_run);
        const char *error = NULL;
        server->ready_execution = vm_execution_create_async(
            &server->ready_vm, server->ready_handler, &server->ready_context, NULL, NULL, &error);
        if (!server->ready_execution) {
            trap(server, error ? error : "native VM out of memory");
            return;
        }
        if (server->ready_console) {
            vm_execution_enable_console(server->ready_execution);
        }
    }
    vm_execution_poll_network(server->ready_execution, 0);
    VMExecutionStatus status = vm_execution_advance(server->ready_execution, budget);
    if (status == VM_COMPLETED) {
        Value *result = vm_execution_result(server->ready_execution);
        if (!result || result->kind != V_VARIANT || result->as.named.count != 1 ||
            (strcmp(result->as.named.name, "Ok") && strcmp(result->as.named.name, "Error"))) {
            trap(server, "VM trap: invalid service readiness result");
            return;
        }
        Value *payload = result->as.named.values[0];
        if (!strcmp(result->as.named.name, "Error") && payload->kind == V_STR) {
            server->result = retain(result);
            server->done = true;
            close_all(server);
        } else if (!strcmp(result->as.named.name, "Ok") && payload->kind == V_UNIT) {
            clear_ready(server);
        } else {
            trap(server, "VM trap: invalid service readiness result");
        }
    } else if (status != VM_WAITING && status != VM_YIELDED) {
        trap(server, server->ready_vm.error ? server->ready_vm.error : "VM trap: readiness failed");
    }
}
#endif

void tcp_server_advance(VMTcpServer *server, size_t budget)
{
#ifndef __wasi__
    uint64_t now;
    if (!deadlines(server, &now)) {
        return;
    }
    if (server->ready_pending) {
        if (budget) {
            advance_ready(server, budget);
        }
        return;
    }
    size_t idle = 0;
    while (budget && deadlines(server, &now) && idle < server->limits.concurrency) {
        ServerClient *client = &server->clients[server->next_handler];
        server->next_handler = (server->next_handler + 1) % server->limits.concurrency;
        idle++;
        if (client->state == CLIENT_START) {
            start_handler(server, client);
        }
        if (server->done) {
            return;
        }
        if (client->state != CLIENT_RUN) {
            continue;
        }
        if (client->fake_pending) {
            Value *result = vm_fake_read_result(client->fake_fail);
            client->fake_pending = false;
            vm_execution_complete_read(client->execution, result);
            release(result);
        }
        VMExecutionStatus status = vm_execution_advance(client->execution, 1);
        budget--;
        if (!deadlines(server, &now)) {
            return;
        }
        if (client->state != CLIENT_RUN) {
            continue;
        }
        if (status == VM_YIELDED) {
            idle = 0;
        } else if (status == VM_COMPLETED) {
            handler_result(server, client);
        } else if (status == VM_EXITED && server->routed && client->vm.http_runtime) {
            struct VMHttpRuntime *runtime = client->vm.http_runtime;
            runtime->exit_requested = true;
            runtime->exit_status = vm_execution_exit_status(client->execution);
            finish_client(server, client, "process exit");
            http_runtime_stop(runtime);
        } else if (status == VM_TRAPPED && server->routed && client->vm.fault_kind == VM_FAULT_REQUEST) {
            http_handler_finished(server, client, true);
        } else if (status != VM_WAITING) {
            trap(server, client->vm.error ? client->vm.error : "VM trap: TCP handler failed");
        }
    }
#else
    (void)server;
    (void)budget;
#endif
}

void tcp_server_enable_filesystem(VMTcpServer *server)
{
    server->filesystem_enabled = true;
}

void tcp_server_enable_console(VMTcpServer *server)
{
    server->console_enabled = true;
}

void tcp_server_stop(VMTcpServer *server)
{
#ifndef __wasi__
    uint64_t now;
    if (deadlines(server, &now)) {
        stop_at(server, now);
        deadlines(server, &now);
    }
#else
    (void)server;
#endif
}

bool tcp_server_http_continue(VM *vm, Value *token)
{
    VMHttpConnection *connection = http_live_connection(vm, token);
    if (!connection) return false;
    ServerClient *client = connection->client;
    if (!connection->admitted || !client || client->execution != vm->execution ||
        client->state != CLIENT_RUN || client->interim_sent) {
        vm->error = "VM trap: invalid HTTP interim response state";
        return false;
    }
    client->state = CLIENT_INTERIM_WRITE;
    return true;
}

bool tcp_server_done(const VMTcpServer *server)
{
    return server->done;
}

Value *tcp_server_result(const VMTcpServer *server)
{
    return server->result;
}

const char *tcp_server_error(const VMTcpServer *server)
{
    return server->error;
}

void tcp_server_destroy(VMTcpServer *server)
{
    if (!server) {
        return;
    }
    close_all(server);
    for (size_t i = 0; i < SERVER_CLIENTS; i++) {
        release(server->outcomes[i]);
    }
    release(server->result);
    if (server->routed && server->context) {
        VMHttpState *state = server->context->as.http_state;
        state->resource = NULL;
        if (state->phase != HTTP_DESTROYED) state->phase = HTTP_CLOSED;
    }
    release(server->context);
    release(server->ready_context);
    free(server);
}
