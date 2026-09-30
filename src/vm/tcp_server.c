#include "platform.h"

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
#define SERVER_TIMEOUT 60000

typedef enum { CLIENT_FREE, CLIENT_READ, CLIENT_START, CLIENT_RUN, CLIENT_WRITE } ClientState;

typedef struct {
    ClientState state;
    int fd;
    size_t outcome, received, sent;
    uint64_t deadline;
    uint8_t *request;
    Value *response;
    VM vm;
    VMExecution *execution;
    bool fake_pending, fake_fail;
} ServerClient;

struct VMTcpServer {
    VM template_vm;
    Function *handler;
    VMTcpServerLimits limits;
    ServerClient clients[SERVER_CONCURRENCY];
    Value *outcomes[SERVER_CLIENTS], *result;
    size_t admitted, active, next_io, next_handler;
    uint64_t admission_deadline, drain_deadline;
    int listener;
    bool outbound, stopping, done;
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
    vm_execution_destroy(client->execution);
    free(client->request);
    release(client->response);
    *client = (ServerClient){.fd = -1};
}

static void close_all(VMTcpServer *server)
{
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
    Value *outcome = error ? error_value(error) : result_value(true, value_new(V_UNIT));
    if (!outcome) {
        trap(server, "native VM out of memory");
        return;
    }
    server->outcomes[client->outcome] = outcome;
    client_clear(client);
    server->active--;
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
    if (!server->stopping && *now >= server->admission_deadline) {
        stop_at(server, server->admission_deadline);
    }
    for (size_t i = 0; i < server->limits.concurrency; i++) {
        ServerClient *client = &server->clients[i];
        if (client->state == CLIENT_FREE) {
            continue;
        }
        if (*now >= client->deadline) {
            finish_client(server, client, "TCP client timed out");
        } else if (server->stopping && *now >= server->drain_deadline) {
            finish_client(server, client, "TCP server drain cancelled client");
        }
        if (server->done) {
            return false;
        }
    }
    if (server->stopping && !server->active) {
        server->result =
            result_value(true, value_sequence(V_ARRAY, server->outcomes, server->admitted));
        server->done = true;
        if (!server->result) {
            trap(server, "native VM out of memory");
        }
        return false;
    }
    return true;
}
#endif

VMTcpServer *tcp_server_start(const VM *vm, Function *handler, const Value *address, size_t port,
                              VMTcpServerLimits limits, bool outbound)
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
    bool found = false;
    if (vm && vm->program) {
        for (size_t i = 0; i < vm->program->count; i++) {
            if (handler == &vm->program->functions[i]) {
                found = true;
            }
        }
    }
    if (!found || !handler->is_async || handler->pure || handler->param_count != 1) {
        trap(server, "VM trap: invalid TCP server handler");
        return server;
    }
    if (!address || address->kind != V_STR || !port || port > 65535 || !limits.clients ||
        limits.clients > SERVER_CLIENTS || !limits.concurrency ||
        limits.concurrency > SERVER_CONCURRENCY || limits.concurrency > limits.clients ||
        limits.request_limit > SERVER_BYTES || limits.response_limit > SERVER_BYTES ||
        !limits.client_timeout_ms || limits.client_timeout_ms > SERVER_TIMEOUT ||
        !limits.admission_timeout_ms || limits.admission_timeout_ms > SERVER_TIMEOUT ||
        limits.drain_ms > SERVER_TIMEOUT) {
        fail(server, "invalid TCP server arguments");
        return server;
    }
    server->template_vm = *vm;
    server->handler = handler;
    server->outbound = outbound;
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

#ifndef __wasi__
static void accept_client(VMTcpServer *server, uint64_t now)
{
    if (server->stopping || server->active == server->limits.concurrency) {
        return;
    }
    size_t slot = 0;
    while (server->clients[slot].state != CLIENT_FREE) {
        slot++;
    }
    int fd = accept(server->listener, NULL, NULL);
    if (fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR && errno != ECONNABORTED) {
            fail(server, "TCP server accept failed");
        }
        return;
    }
    ServerClient *client = &server->clients[slot];
    *client = (ServerClient){.state = CLIENT_READ,
                             .fd = fd,
                             .outcome = server->admitted++,
                             .deadline = now + server->limits.client_timeout_ms};
    server->active++;
    if (!configure(fd)) {
        finish_client(server, client, "TCP client socket failed");
    } else {
        client->request = malloc(server->limits.request_limit ? server->limits.request_limit : 1);
        if (!client->request) {
            trap(server, "native VM out of memory");
        }
    }
    if (!server->done && server->admitted == server->limits.clients) {
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
        finish_client(server, client, NULL);
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
    struct pollfd fds[SERVER_CONCURRENCY + 1];
    size_t slots[SERVER_CONCURRENCY + 1], identities[SERVER_CONCURRENCY + 1], count = 0;
    uint64_t until = server->stopping ? server->drain_deadline : server->admission_deadline;
    unsigned wait = max_wait_ms > INT_MAX ? INT_MAX : max_wait_ms;
    if (!server->stopping && server->active < server->limits.concurrency) {
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
        if (client->state == CLIENT_START) {
            wait = 0;
        } else if (client->state == CLIENT_RUN) {
            vm_execution_poll_tcp(client->execution, 0);
            VMExecutionStatus status = vm_execution_advance(client->execution, 0);
            if (status != VM_WAITING || client->fake_pending) {
                wait = 0;
            }
            /* Outbound exchange descriptors belong to the child execution.
             * Recheck at 1ms while pending instead of blocking another client. */
            else if (wait > 1) {
                wait = 1;
            }
        } else {
            fds[count] = (struct pollfd){.fd = client->fd,
                                         .events = client->state == CLIENT_READ ? POLLIN : POLLOUT};
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
            if (fds[i].revents & POLLNVAL) {
                finish_client(server, client, "TCP client socket failed");
            } else if (client->state == CLIENT_READ) {
                read_client(server, client);
            } else if (client->state == CLIENT_WRITE) {
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
static bool fake_wait(void *context, Value *value, const char **error)
{
    (void)error;
    ServerClient *client = context;
    client->fake_pending = true;
    client->fake_fail = value->as.boolean;
    return true;
}

static void start_handler(VMTcpServer *server, ServerClient *client)
{
    Value *request = value_data(V_BYTES, client->request, client->received);
    if (!request) {
        trap(server, "native VM out of memory");
        return;
    }
    client->vm = server->template_vm;
    client->vm.execution = NULL;
    client->vm.error = NULL;
    const char *error;
    client->execution = vm_execution_create_async(&client->vm, server->handler, &request, fake_wait,
                                                  client, &error);
    release(request);
    free(client->request);
    client->request = NULL;
    if (!client->execution) {
        trap(server, error);
    } else {
        if (server->outbound) {
            vm_execution_enable_tcp(client->execution);
        }
        client->state = CLIENT_RUN;
    }
}

static void handler_result(VMTcpServer *server, ServerClient *client)
{
    Value *result = vm_execution_result(client->execution);
    if (!vm_fake_read_result_valid(result)) {
        trap(server, "VM trap: invalid TCP server handler result");
        return;
    }
    Value *payload = result->as.named.values[0];
    if (!strcmp(result->as.named.name, "Error")) {
        /* Preserve the handler's string exactly, including embedded NUL bytes. */
        server->outcomes[client->outcome] = retain(result);
        client_clear(client);
        server->active--;
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

void tcp_server_advance(VMTcpServer *server, size_t budget)
{
#ifndef __wasi__
    uint64_t now;
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
        } else if (status != VM_WAITING) {
            trap(server, client->vm.error ? client->vm.error : "VM trap: TCP handler failed");
        }
    }
#else
    (void)server;
    (void)budget;
#endif
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
    free(server);
}
