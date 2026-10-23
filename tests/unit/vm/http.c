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
#include <signal.h>
#include <sys/wait.h>

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

static void deadline_readiness_race(void)
{
    const uint64_t deadline = 100;
    assert(http_transport_classify_wait(99, deadline, 1, POLLIN) == VM_HTTP_WAIT_READY);
    assert(http_transport_classify_wait(100, deadline, 1, POLLIN) == VM_HTTP_WAIT_TIMEOUT);
    assert(http_transport_classify_wait(101, deadline, 1, POLLIN) == VM_HTTP_WAIT_TIMEOUT);
    assert(http_transport_classify_wait(99, deadline, 0, 0) == VM_HTTP_WAIT_PENDING);
    assert(http_transport_classify_wait(99, deadline, 1, POLLNVAL) == VM_HTTP_WAIT_INVALID);
    assert(http_transport_classify_wait(100, deadline, 1, POLLNVAL) == VM_HTTP_WAIT_TIMEOUT);
}

static Value *stream_payload(Value *result, const char *variant)
{
    assert(result && result->kind == V_VARIANT);
    assert(!strcmp(result->as.named.name, variant));
    return result->as.named.values[0];
}

static void bounded_stream(bool limited)
{
    size_t port;
    int server = listener(&port);
    pid_t child = fork();
    assert(child >= 0);
    if (!child) {
        signal(SIGPIPE, SIG_IGN);
        int peer = accept(server, NULL, NULL);
        if (peer < 0) _exit(1);
        uint8_t input[2048];
        size_t used = 0;
        while (!contains(input, used, "\r\n\r\n")) {
            ssize_t n = read(peer, input + used, sizeof(input) - used);
            if (n <= 0) _exit(2);
            used += (size_t)n;
        }
        uint8_t block[8192];
        memset(block, 0x5a, sizeof(block));
        size_t total = 32 * 1024 * 1024, sent = 0;
        while (sent < total) {
            ssize_t n = write(peer, block, sizeof(block));
            if (n <= 0) break;
            sent += (size_t)n;
        }
        close(peer);
        close(server);
        _exit(limited || sent == total ? 0 : 3);
    }
    close(server);
    VM vm = {0}, other = {0};
    Value *args[6];
    arguments(args, port, false);
    release(args[4]); args[4] = value_size(limited ? 100 : 32 * 1024 * 1024);
    Value *opened = http_stream_call(&vm, "http_stream_open", args);
    Value *handle = stream_payload(opened, "Ok");
    Value *read_args[] = {handle};
    Value *wrong_owner = http_stream_call(&other, "http_stream_read", read_args);
    stream_payload(wrong_owner, "Error"); release(wrong_owner);
    size_t received = 0;
    for (;;) {
        Value *result = http_stream_call(&vm, "http_stream_read", read_args);
        if (limited) { stream_payload(result, "Error"); release(result); break; }
        Value *chunk = stream_payload(result, "Ok");
        assert(chunk->kind == V_BYTES && chunk->as.bytes.length <= 16384);
        size_t size = chunk->as.bytes.length;
        for (size_t i = 0; i < size; i++) assert(chunk->as.bytes.data[i] == 0x5a);
        received += size;
        release(result);
        if (!size) break;
    }
    assert(limited || received == 32 * 1024 * 1024);
    Value *closed = http_stream_call(&vm, "http_stream_close", read_args);
    stream_payload(closed, "Ok"); release(closed);
    closed = http_stream_call(&vm, "http_stream_read", read_args);
    stream_payload(closed, "Error"); release(closed);
    release(opened); release_arguments(args);
    http_stream_destroy(&vm);
    assert(!vm.http_streams);
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

void http_transport_contracts(void)
{
    VM stream_vm = {0};
    Value *stream_args[6];
    arguments(stream_args, 1, false);
    release(stream_args[5]); stream_args[5] = value_size(1);
    Value *opened = http_stream_call(&stream_vm, "http_stream_open", stream_args);
    Value *stream_handle[] = {stream_payload(opened, "Ok")};
    (void)poll(NULL, 0, 5);
    Value *expired = http_stream_call(&stream_vm, "http_stream_read", stream_handle);
    Value *message = stream_payload(expired, "Error");
    assert(contains(message->as.bytes.data, message->as.bytes.length, "timed out"));
    release(expired); release(opened); release_arguments(stream_args);
    http_stream_destroy(&stream_vm);
    http_stream_destroy(&stream_vm);
    bounded_stream(false);
    bounded_stream(true);
    deadline_readiness_race();
    malformed_arguments();
    for (size_t repeat = 0; repeat < 8; repeat++) {
        cancel_incomplete_connection(false);
        cancel_incomplete_connection(true);
    }
}
