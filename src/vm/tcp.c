#include "platform.h"
#include "tcp.h"

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

#define TCP_BYTE_LIMIT (1024 * 1024)
#define TCP_TIMEOUT_LIMIT 60000

struct VMTcpExchange {
    Value *request, *result;
    uint8_t *response;
    size_t sent, received, limit;
    uint64_t deadline;
    int fd;
    bool connecting, sent_eof, done;
};

static void close_socket(VMTcpExchange *exchange)
{
#ifndef __wasi__
    if (exchange->fd >= 0) {
        close(exchange->fd);
        exchange->fd = -1;
    }
#else
    (void)exchange;
#endif
}

static void finish(VMTcpExchange *exchange, const char *error)
{
    close_socket(exchange);
    exchange->done = true;
    Value *payload = value_data(error ? V_STR : V_BYTES,
        error ? (const uint8_t *)error : exchange->response,
        error ? strlen(error) : exchange->received);
    if (payload) {
        exchange->result = named_value(V_VARIANT, error ? "Error" : "Ok", NULL, &payload, 1);
        release(payload);
    }
}

#ifndef __wasi__
static bool monotonic_ms(uint64_t *result)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return false;
    *result = (uint64_t)now.tv_sec * 1000 + (uint64_t)now.tv_nsec / 1000000;
    return true;
}
#endif

VMTcpExchange *tcp_exchange_start(Value **arguments)
{
    VMTcpExchange *exchange = calloc(1, sizeof(*exchange));
    if (!exchange) return NULL;
    exchange->fd = -1;
    size_t port, timeout;
    if (arguments[0]->kind != V_STR || arguments[1]->kind != V_NAT ||
        arguments[2]->kind != V_BYTES || arguments[3]->kind != V_NAT ||
        arguments[4]->kind != V_NAT || !value_index(arguments[1], &port) ||
        !value_index(arguments[3], &exchange->limit) || !value_index(arguments[4], &timeout) ||
        port == 0 || port > 65535 || exchange->limit > TCP_BYTE_LIMIT ||
        arguments[2]->as.bytes.length > TCP_BYTE_LIMIT || timeout == 0 || timeout > TCP_TIMEOUT_LIMIT) {
        finish(exchange, "invalid TCP arguments");
        return exchange;
    }
#ifdef __wasi__
    finish(exchange, "TCP is unavailable on this host");
#else
    char address[INET_ADDRSTRLEN];
    size_t length = arguments[0]->as.bytes.length;
    struct sockaddr_in peer = {.sin_family = AF_INET, .sin_port = htons((uint16_t)port)};
    if (!length || length >= sizeof(address) || memchr(arguments[0]->as.bytes.data, 0, length)) {
        finish(exchange, "invalid IPv4 address");
        return exchange;
    }
    memcpy(address, arguments[0]->as.bytes.data, length);
    address[length] = 0;
    if (inet_pton(AF_INET, address, &peer.sin_addr) != 1) {
        finish(exchange, "invalid IPv4 address");
        return exchange;
    }
    uint64_t now;
    if (!monotonic_ms(&now) || UINT64_MAX - now < timeout) {
        finish(exchange, "TCP clock failed");
        return exchange;
    }
    exchange->deadline = now + timeout;
    exchange->response = malloc(exchange->limit ? exchange->limit : 1);
    if (!exchange->response) {
        exchange->done = true;
        return exchange;
    }
    exchange->request = retain(arguments[2]);
    exchange->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (exchange->fd < 0 || fcntl(exchange->fd, F_SETFL, O_NONBLOCK) < 0 ||
        fcntl(exchange->fd, F_SETFD, FD_CLOEXEC) < 0) {
        finish(exchange, "TCP socket failed");
        return exchange;
    }
#ifdef SO_NOSIGPIPE
    int enabled = 1;
    if (setsockopt(exchange->fd, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled)) < 0) {
        finish(exchange, "TCP socket failed");
        return exchange;
    }
#endif
    if (connect(exchange->fd, (struct sockaddr *)&peer, sizeof(peer)) < 0) {
        if (errno != EINPROGRESS && errno != EINTR) finish(exchange, "TCP connect failed");
        else exchange->connecting = true;
    }
#endif
    return exchange;
}

bool tcp_exchange_poll(VMTcpExchange *exchange, unsigned max_wait_ms)
{
    if (exchange->done) return true;
#ifndef __wasi__
    uint64_t now;
    if (!monotonic_ms(&now)) {
        finish(exchange, "TCP clock failed");
        return true;
    }
    if (now >= exchange->deadline) {
        finish(exchange, "TCP timed out");
        return true;
    }
    uint64_t remaining = exchange->deadline - now;
    unsigned wait = max_wait_ms > INT_MAX ? INT_MAX : max_wait_ms;
    if (remaining < wait) wait = (unsigned)remaining;
    struct pollfd descriptor = {.fd = exchange->fd, .events = POLLIN};
    if (exchange->connecting || !exchange->sent_eof) descriptor.events |= POLLOUT;
    int ready = poll(&descriptor, 1, (int)wait);
    if (ready < 0) {
        if (errno != EINTR) finish(exchange, "TCP poll failed");
        return exchange->done;
    }
    /* Deadlines win even when readiness arrives in the same poll. */
    if (!monotonic_ms(&now)) finish(exchange, "TCP clock failed");
    else if (now >= exchange->deadline) finish(exchange, "TCP timed out");
    if (exchange->done || ready == 0) return exchange->done;
    if (descriptor.revents & POLLNVAL) {
        finish(exchange, "TCP socket failed");
        return true;
    }
    if (exchange->connecting) {
        int error = 0;
        socklen_t size = sizeof(error);
        if (getsockopt(exchange->fd, SOL_SOCKET, SO_ERROR, &error, &size) < 0 || error) {
            finish(exchange, "TCP connect failed");
            return true;
        }
        exchange->connecting = false;
    }
    if (!exchange->sent_eof && (descriptor.revents & POLLOUT)) {
        size_t left = exchange->request->as.bytes.length - exchange->sent;
        if (left) {
            if (left > 16384) left = 16384;
#ifdef MSG_NOSIGNAL
            int flags = MSG_NOSIGNAL;
#else
            int flags = 0;
#endif
            ssize_t sent = send(exchange->fd, exchange->request->as.bytes.data + exchange->sent, left, flags);
            if (sent > 0) exchange->sent += (size_t)sent;
            else if (sent == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
                finish(exchange, "TCP write failed");
                return true;
            }
        }
        if (exchange->sent == exchange->request->as.bytes.length) {
            if (shutdown(exchange->fd, SHUT_WR) < 0) {
                finish(exchange, "TCP shutdown failed");
                return true;
            }
            exchange->sent_eof = true;
        }
    }
    if (descriptor.revents & (POLLIN | POLLHUP | POLLERR)) {
        uint8_t buffer[16384];
        ssize_t count = recv(exchange->fd, buffer, sizeof(buffer), 0);
        if (count > 0) {
            if ((size_t)count > exchange->limit - exchange->received) {
                finish(exchange, "TCP response limit exceeded");
            } else {
                memcpy(exchange->response + exchange->received, buffer, (size_t)count);
                exchange->received += (size_t)count;
            }
        } else if (count == 0) {
            finish(exchange, exchange->sent_eof ? NULL : "TCP peer closed before request completed");
        } else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            finish(exchange, "TCP read failed");
        }
    }
#else
    (void)max_wait_ms;
#endif
    return exchange->done;
}

Value *tcp_exchange_result(const VMTcpExchange *exchange)
{
    return exchange->result;
}

void tcp_exchange_destroy(VMTcpExchange *exchange)
{
    if (!exchange) return;
    close_socket(exchange);
    release(exchange->request);
    release(exchange->result);
    free(exchange->response);
    free(exchange);
}
