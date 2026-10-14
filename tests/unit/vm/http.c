#ifdef NDEBUG
#undef NDEBUG
#endif
#include "http_transport.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static Value *text(const char *s)
{
    return value_data(V_STR, (const uint8_t *)s, strlen(s));
}

static Value *bytes(const char *s)
{
    return value_data(V_BYTES, (const uint8_t *)s, strlen(s));
}

static int listener(size_t *port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd >= 0);
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
    };
    assert(!bind(fd, (struct sockaddr *)&address, sizeof(address)) && !listen(fd, 8));
    socklen_t size = sizeof(address);
    assert(!getsockname(fd, (struct sockaddr *)&address, &size));
    *port = ntohs(address.sin_port);
    return fd;
}

static bool ready(int fd, short events, int timeout)
{
    struct pollfd descriptor = {.fd = fd, .events = events};
    int result;
    do {
        result = poll(&descriptor, 1, timeout);
    } while (result < 0 && errno == EINTR);
    assert(result >= 0);
    return result > 0 && (descriptor.revents & (events | POLLHUP | POLLERR));
}

static bool contains(const uint8_t *data, size_t length, const char *text)
{
    size_t text_length = strlen(text);
    if (text_length > length) return false;
    for (size_t i = 0; i <= length - text_length; i++) {
        if (!memcmp(data + i, text, text_length)) return true;
    }
    return false;
}

static void arguments(Value **args, size_t port, bool tls)
{
    args[0] = text("127.0.0.1");
    args[1] = value_size(port);
    args[2] = value_bool(tls);
    args[3] = bytes("GET / HTTP/1.1\r\nHost: test\r\nConnection: close\r\n\r\n");
    args[4] = value_size(1024);
    args[5] = value_size(5000);
    for (size_t i = 0; i < 6; i++) assert(args[i]);
}

static void release_arguments(Value **args)
{
    for (size_t i = 0; i < 6; i++) release(args[i]);
}

static void assert_invalid(Value **args)
{
    VMHttpTransport *transport = http_transport_start(args);
    assert(transport && http_transport_poll(transport, 0));
    Value *result = http_transport_result(transport);
    assert(result && result->kind == V_VARIANT && !strcmp(result->as.named.name, "Error"));
    assert(result->as.named.values[0]->as.bytes.length == strlen("invalid HTTP transport arguments"));
    assert(!memcmp(result->as.named.values[0]->as.bytes.data,
                   "invalid HTTP transport arguments", strlen("invalid HTTP transport arguments")));
    http_transport_destroy(transport);
}

static void malformed_arguments(void)
{
    size_t port;
    int server = listener(&port);
    for (size_t index = 0; index < 6; index++) {
        Value *args[6];
        arguments(args, port, false);
        release(args[index]);
        switch (index) {
        case 0: args[index] = value_bool(true); break;
        case 1: args[index] = text("80"); break;
        case 2: args[index] = value_size(1); break;
        case 3: args[index] = text("not bytes"); break;
        case 4: args[index] = value_bool(true); break;
        default: args[index] = value_bool(true); break;
        }
        assert(args[index]);
        assert_invalid(args);
        release_arguments(args);
    }
    Value *args[6];
    arguments(args, 0, false);
    assert_invalid(args);
    release_arguments(args);
    arguments(args, port, false);
    release(args[5]);
    args[5] = value_size(0);
    assert(args[5]);
    assert_invalid(args);
    release_arguments(args);
    arguments(args, port, false);
    release(args[0]);
    args[0] = text("bad host");
    assert(args[0]);
    assert_invalid(args);
    release_arguments(args);
    assert(!ready(server, POLLIN, 0));
    close(server);
}

static void cancel_incomplete_connection(bool tls)
{
    size_t port;
    int server = listener(&port);
    Value *args[6];
    arguments(args, port, tls);
    VMHttpTransport *transport = http_transport_start(args);
    assert(transport);
    int peer = -1;
    uint8_t input[2048];
    size_t received = 0;
    bool request_started = false;
    const char *end = "\r\n\r\n";
    for (size_t step = 0; step < 1000 && !request_started; step++) {
        assert(!http_transport_poll(transport, 1));
        if (peer < 0 && ready(server, POLLIN, 0)) {
            peer = accept(server, NULL, NULL);
            assert(peer >= 0);
        }
        if (peer < 0 || !ready(peer, POLLIN, 2)) continue;
        ssize_t count = read(peer, input + received, sizeof(input) - received);
        assert(count > 0 && (size_t)count <= sizeof(input) - received);
        if (tls) {
            request_started = input[0] == 0x16; /* TLS handshake record */
        } else {
            received += (size_t)count;
            request_started = received >= strlen(end) &&
                contains(input, received, end);
        }
    }
    assert(peer >= 0 && request_started);
    http_transport_destroy(transport);

    bool closed = false;
    for (size_t step = 0; step < 100 && !closed; step++) {
        if (!ready(peer, POLLIN, 20)) continue;
        ssize_t count = read(peer, input, sizeof(input));
        if (!count) closed = true;
        else if (count < 0 && errno != EINTR) closed = true;
    }
    assert(closed);
    close(peer);
    close(server);
    release_arguments(args);
}

void http_transport_contracts(void)
{
    malformed_arguments();
    for (size_t repeat = 0; repeat < 8; repeat++) {
        cancel_incomplete_connection(false);
        cancel_incomplete_connection(true);
    }
}
