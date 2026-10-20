/* Independent loopback peer for the packaged HTTP server integration test. */
#include "../src/vm/platform.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static int listen_on_ephemeral_port(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port = 0,
    };
    if (fd < 0 || bind(fd, (struct sockaddr *)&address, sizeof(address))) return -1;
    socklen_t length = sizeof(address);
    if (getsockname(fd, (struct sockaddr *)&address, &length)) return -1;
    if (listen(fd, 2)) return -1;
    printf("%u\n", ntohs(address.sin_port));
    fflush(stdout);
    return fd;
}

static int connect_to(const char *port_text)
{
    char *end = NULL;
    long port = strtol(port_text, &end, 10);
    if (!end || *end || port < 1 || port > 65535) return -1;
    struct timeval timeout = {.tv_sec = 4, .tv_usec = 0};
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port = htons((uint16_t)port),
    };
    struct timespec retry = {.tv_sec = 0, .tv_nsec = 50000000};
    for (int attempt = 0; attempt < 80; attempt++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return -1;
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
            close(fd);
            return -1;
        }
        if (!connect(fd, (struct sockaddr *)&address, sizeof(address))) return fd;
        int problem = errno;
        close(fd);
        if (problem != ECONNREFUSED && problem != EINTR) return -1;
        nanosleep(&retry, NULL);
    }
    fprintf(stderr, "connect to loopback port %ld failed: %s\n", port, strerror(errno));
    return -1;
}

static int send_all(int fd, const void *data, size_t length)
{
    const unsigned char *bytes = data;
    for (size_t sent = 0; sent < length;) {
        ssize_t count = send(fd, bytes + sent, length - sent, 0);
        if (count <= 0) return -1;
        sent += (size_t)count;
    }
    return 0;
}

static int response_contains(int fd, const char *status, const char *body, int head_only)
{
    char response[32768];
    size_t total = 0;
    for (;;) {
        ssize_t count = read(fd, response + total, sizeof(response) - total - 1);
        if (count < 0) {
            if (errno == ECONNRESET && total > 0) break;
            perror("read response");
            return -1;
        }
        if (count == 0) break;
        total += (size_t)count;
        if (total == sizeof(response) - 1) return -1;
    }
    response[total] = '\0';
    const char *header_end = strstr(response, "\r\n\r\n");
    if (strstr(response, status) && strstr(response, body) &&
        (!head_only || (header_end && header_end[4] == '\0'))) return 0;
    fprintf(stderr, "unexpected HTTP response (%zu bytes); expected %s and body %s\n%s\n",
            total, status, body, response);
    return -1;
}

static int exchange(const char *port, const char *request, const char *status,
                    const char *body, int close_write)
{
    int fd = connect_to(port);
    if (fd < 0) return -1;
    int result = send_all(fd, request, strlen(request));
    if (!result && close_write && shutdown(fd, SHUT_WR)) result = -1;
    if (!result) result = response_contains(fd, status, body, !strncmp(request, "HEAD ", 5));
    close(fd);
    return result;
}

static int concurrent_exchange(const char *port)
{
    const char *request = "GET / HTTP/1.1\r\nHost: example.test\r\n\r\n";
    int first = connect_to(port);
    int second = connect_to(port);
    if (first < 0 || second < 0) return -1;
    int result = send_all(first, request, strlen(request));
    if (!result) result = send_all(second, request, strlen(request));
    if (!result) result = response_contains(first, "HTTP/1.1 200 OK", "\r\n\r\nok", 0);
    if (!result) result = response_contains(second, "HTTP/1.1 200 OK", "\r\n\r\nok", 0);
    close(first);
    close(second);
    return result;
}

static int routed_capacity(const char *port)
{
    int waiting[16];
    for (size_t i = 0; i < 16; i++) waiting[i] = -1;
    int result = 0;
    const char *head = "POST /echo HTTP/1.1\r\nHost: local\r\nExpect: 100-continue\r\nContent-Length: 3\r\n\r\n";
    for (size_t i = 0; !result && i < 4; i++) {
        waiting[i] = connect_to(port);
        if (waiting[i] < 0 || send_all(waiting[i], head, strlen(head))) { result = -1; break; }
        char interim[25];
        size_t used = 0;
        while (!result && used < sizeof(interim)) {
            ssize_t count = read(waiting[i], interim + used, sizeof(interim) - used);
            if (count <= 0) result = -1;
            else used += (size_t)count;
        }
        if (!result && memcmp(interim, "HTTP/1.1 100 Continue\r\n\r\n", sizeof(interim))) result = -1;
    }
    if (!result) result = exchange(port, head, "503 Service Unavailable", "Service Unavailable", 0);
    for (size_t i = 0; !result && i < 4; i++) {
        result = send_all(waiting[i], "abc", 3);
        if (!result) result = response_contains(waiting[i], "200 OK", "\r\n\r\nabc", 0);
    }
    for (size_t i = 0; i < 16; i++) { if (waiting[i] >= 0) close(waiting[i]); waiting[i] = -1; }
    if (result) return result;
    for (size_t i = 0; !result && i < 16; i++) {
        waiting[i] = connect_to(port);
        if (waiting[i] < 0 || send_all(waiting[i], "GET /hello HTTP/1.1\r\n", 21)) result = -1;
    }
    if (!result) result = exchange(port, "GET /hello HTTP/1.1\r\nHost: local\r\n\r\n",
                                  "503 Service Unavailable", "Service Unavailable", 0);
    for (size_t i = 0; i < 16; i++) if (waiting[i] >= 0) close(waiting[i]);
    return result;
}

static int routed_deadline(const char *port)
{
    const char *requests[] = {
        "GET /hello HTTP/1.1\r\n",
        "POST /echo HTTP/1.1\r\nHost: local\r\nContent-Length: 3\r\n\r\na",
        "GET /stall HTTP/1.1\r\nHost: local\r\n\r\n",
        "GET /sent-stall HTTP/1.1\r\nHost: local\r\n\r\n"
    };
    int peers[] = {-1, -1, -1, -1};
    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start)) return -1;
    int result = 0;
    for (size_t i = 0; !result && i < 4; i++) {
        peers[i] = connect_to(port);
        struct timeval timeout = {.tv_sec = 12};
        if (peers[i] < 0 || setsockopt(peers[i], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) ||
            send_all(peers[i], requests[i], strlen(requests[i]))) result = -1;
    }
    /* A committed response is delivered even while its handler remains busy. */
    if (!result) result = response_contains(peers[3], "200 OK", "first response", 0);
    if (!result && (clock_gettime(CLOCK_MONOTONIC, &end) || end.tv_sec - start.tv_sec > 3)) result = -1;
    for (size_t i = 0; !result && i < 3; i++) {
        result = response_contains(peers[i], "504 Gateway Timeout", "Gateway Timeout", 0);
    }
    if (!result) {
        if (clock_gettime(CLOCK_MONOTONIC, &end)) result = -1;
        else {
            double elapsed = (double)(end.tv_sec - start.tv_sec) +
                (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;
            /* Scheduling tolerance belongs to this observer, not the product deadline. */
            if (elapsed < 9.5 || elapsed > 12.0) {
                fprintf(stderr, "HTTP deadline elapsed %.3f seconds\n", elapsed);
                result = -1;
            }
        }
    }
    for (size_t i = 0; i < 4; i++) if (peers[i] >= 0) close(peers[i]);
    if (!result) result = exchange(port, "GET /hello HTTP/1.1\r\nHost: local\r\n\r\n", "200 OK", "hello", 0);
    return result;
}

static int routed_limits(const char *port)
{
    for (size_t length = 1048575; length <= 1048577; length++) {
        int fd = connect_to(port);
        if (fd < 0) return -1;
        char head[128], expected[32];
        int count = snprintf(head, sizeof(head), "POST /size HTTP/1.1\r\nHost: local\r\nContent-Length: %zu\r\n\r\n", length);
        int result = count < 0 ? -1 : send_all(fd, head, (size_t)count);
        char bytes[16384];
        memset(bytes, 'b', sizeof(bytes));
        if (length <= 1048576) {
            for (size_t sent = 0; !result && sent < length;) {
                size_t block = length - sent < sizeof(bytes) ? length - sent : sizeof(bytes);
                result = send_all(fd, bytes, block);
                sent += block;
            }
            snprintf(expected, sizeof(expected), "\r\n\r\n%zu", length);
            if (!result) result = response_contains(fd, "200 OK", expected, 0);
        } else if (!result) result = response_contains(fd, "413 Content Too Large", "Content Too Large", 0);
        close(fd);
        if (result) return -1;
    }
    for (size_t length = 16383; length <= 16385; length++) {
        const char *prefix = "GET /hello HTTP/1.1\r\nHost: local\r\nX-Large: ";
        char *head = malloc(length + 1);
        if (!head) return -1;
        memcpy(head, prefix, strlen(prefix));
        memset(head + strlen(prefix), 'a', length - strlen(prefix) - 4);
        memcpy(head + length - 4, "\r\n\r\n", 4);
        head[length] = 0;
        int result = exchange(port, head, length <= 16384 ? "200 OK" : "431 Request Header Fields Too Large",
                              length <= 16384 ? "hello" : "Request Header Fields Too Large", 0);
        free(head);
        if (result) return -1;
    }
    return 0;
}

static int routed_exchange(const char *port)
{
    struct { const char *request, *status, *body; } cases[] = {
        {"GET /hello HTTP/1.1\r\nHost: local\r\n\r\n", "200 OK", "\r\n\r\nhello"},
        {"GET /hello?raw=%2F HTTP/1.1\r\nHost: local\r\n\r\n", "200 OK", "hello"},
        {"GET /Hello HTTP/1.1\r\nHost: local\r\n\r\n", "404 Not Found", "Not Found"},
        {"PATCH /hello HTTP/1.1\r\nHost: local\r\n\r\n", "405 Method Not Allowed", "Allow: GET\r\n"},
        {"POST /echo HTTP/1.1\r\nHost: local\r\nContent-Length: 3\r\n\r\nabc", "200 OK", "\r\n\r\nabc"},
        {"POST /echo HTTP/1.1\r\nHost: local\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nabc\r\n0\r\n\r\n", "200 OK", "\r\n\r\nabc"},
        {"POST /echo HTTP/1.1\r\nHost: local\r\n\r\nx", "400 Bad Request", "Bad Request"},
        {"POST /echo HTTP/1.1\r\nHost: local\r\nContent-Length: 0\r\nContent-Length: 0\r\n\r\n", "400 Bad Request", "Bad Request"},
        {"POST /echo HTTP/1.1\r\nHost: local\r\nContent-Length: 1048577\r\nExpect: 100-continue\r\n\r\n", "413 Content Too Large", "Content Too Large"},
        {"POST /echo HTTP/1.1\r\nHost: local\r\nContent-Length: 0\r\nTransfer-Encoding: gzip\r\n\r\n", "400 Bad Request", "Bad Request"},
        {"POST /echo HTTP/1.1\r\nHost: local\r\nExpect: something\r\n\r\n", "417 Expectation Failed", "Expectation Failed"},
        {"GET /failure HTTP/1.1\r\nHost: local\r\n\r\n", "500 Internal Server Error", "Internal Server Error"},
        {"GET /trap HTTP/1.1\r\nHost: local\r\n\r\n", "500 Internal Server Error", "Internal Server Error"},
        {"GET /missing-send HTTP/1.1\r\nHost: local\r\n\r\n", "500 Internal Server Error", "Internal Server Error"},
        {"GET /sent-error HTTP/1.1\r\nHost: local\r\n\r\n", "200 OK", "first response"},
        {"GET /hello HTTP/1.1\r\nHost: local\r\n\r\n", "200 OK", "hello"},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        if (exchange(port, cases[i].request, cases[i].status, cases[i].body, 0)) return -1;
    }
    for (int chunked = 0; chunked < 2; chunked++) {
        int fd = connect_to(port);
        if (fd < 0) return -1;
        const char *head = chunked ?
            "POST /echo HTTP/1.1\r\nHost: local\r\nExpect: 100-continue\r\nTransfer-Encoding: chunked\r\n\r\n" :
            "POST /echo HTTP/1.1\r\nHost: local\r\nExpect: 100-continue\r\nContent-Length: 3\r\n\r\n";
        int result = send_all(fd, head, strlen(head));
        char interim[25];
        size_t received = 0;
        while (!result && received < sizeof(interim)) {
            ssize_t count = read(fd, interim + received, sizeof(interim) - received);
            if (count <= 0) result = -1;
            else received += (size_t)count;
        }
        if (!result && memcmp(interim, "HTTP/1.1 100 Continue\r\n\r\n", sizeof(interim))) result = -1;
        const char *body = chunked ? "3\r\nabc\r\n0\r\n\r\n" : "abc";
        /* Deliberately fragment the body/chunk framing across writes. */
        for (size_t i = 0; !result && body[i]; i++) result = send_all(fd, body + i, 1);
        if (!result) result = response_contains(fd, "200 OK", "\r\n\r\nabc", 0);
        close(fd);
        if (result) return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    alarm(20);
    if (argc == 2 && (!strcmp(argv[1], "port") || !strcmp(argv[1], "reserve"))) {
        int fd = listen_on_ephemeral_port();
        if (fd < 0) return 1;
        if (!strcmp(argv[1], "reserve")) for (;;) pause();
        close(fd);
        return 0;
    }
    if (argc != 3) return 2;
    const char *port = argv[1];
    const char *mode = argv[2];
    if (!strcmp(mode, "routed-limits")) return routed_limits(port) ? 1 : 0;
    if (!strcmp(mode, "routed-deadline")) return routed_deadline(port) ? 1 : 0;
    if (!strcmp(mode, "routed-capacity")) return routed_capacity(port) ? 1 : 0;
    if (!strcmp(mode, "routed")) return routed_exchange(port) ? 1 : 0;
    if (!strcmp(mode, "get"))
        return exchange(port, "GET / HTTP/1.1\r\nHost: example.test\r\n\r\n",
                        "HTTP/1.1 200 OK", "\r\n\r\nok", 0) ? 1 : 0;
    if (!strcmp(mode, "example"))
        return exchange(port, "GET / HTTP/1.1\r\nHost: example.test\r\n\r\n",
                        "HTTP/1.1 200 OK", "hello from Panackelty HTTP server", 0) ? 1 : 0;
    if (!strcmp(mode, "head"))
        return exchange(port, "HEAD / HTTP/1.1\r\nHost: example.test\r\n\r\n",
                        "HTTP/1.1 200 OK", "Content-Length: 2\r\n", 0) ? 1 : 0;
    if (!strcmp(mode, "head-body"))
        return exchange(port, "HEAD /echo HTTP/1.1\r\nHost: example.test\r\n"
                        "Content-Length: 2\r\n\r\nhi", "HTTP/1.1 200 OK",
                        "Content-Length: 2\r\n", 0) ? 1 : 0;
    if (!strcmp(mode, "head-chunked"))
        return exchange(port, "HEAD /echo HTTP/1.1\r\nHost: example.test\r\n"
                        "Transfer-Encoding: chunked\r\n\r\n2\r\nhi\r\n0\r\n\r\n",
                        "HTTP/1.1 200 OK", "Content-Length: 2\r\n", 0) ? 1 : 0;
    if (!strcmp(mode, "head-error"))
        return exchange(port, "HEAD /fail HTTP/1.1\r\nHost: example.test\r\n\r\n",
                        "HTTP/1.1 500 Internal Server Error", "Content-Length: 21\r\n", 0) ? 1 : 0;
    if (!strcmp(mode, "head-invalid-response"))
        return exchange(port, "HEAD /invalid HTTP/1.1\r\nHost: example.test\r\n\r\n",
                        "HTTP/1.1 500 Internal Server Error", "Content-Length: 21\r\n", 0) ? 1 : 0;
    if (!strcmp(mode, "head-malformed"))
        return exchange(port, "HEAD / HTTP/1.1\r\nHost: example.test\r\n"
                        "Content-Length: 0\r\nContent-Length: 0\r\n\r\n",
                        "HTTP/1.1 400 Bad Request", "Content-Length: 11\r\n", 0) ? 1 : 0;
    if (!strcmp(mode, "post"))
        return exchange(port, "POST /echo HTTP/1.1\r\nHost: example.test\r\n"
                        "Content-Length: 2\r\n\r\nhi", "HTTP/1.1 200 OK",
                        "\r\n\r\nhi", 0) ? 1 : 0;
    if (!strcmp(mode, "method"))
        return exchange(port, "PUT / HTTP/1.1\r\nHost: example.test\r\n\r\n",
                        "HTTP/1.1 405 Method Not Allowed", "Allow: GET, HEAD, POST", 0) ? 1 : 0;
    if (!strcmp(mode, "handler-error"))
        return exchange(port, "GET /fail HTTP/1.1\r\nHost: example.test\r\n\r\n",
                        "HTTP/1.1 500 Internal Server Error", "Internal Server Error", 0) ? 1 : 0;
    if (!strcmp(mode, "malformed"))
        return exchange(port, "POST / HTTP/1.1\r\nHost: example.test\r\n"
                        "Content-Length: 0\r\nContent-Length: 0\r\n\r\n",
                        "HTTP/1.1 400 Bad Request", "Bad Request", 0) ? 1 : 0;
    if (!strcmp(mode, "partial"))
        return exchange(port, "POST /echo HTTP/1.1\r\nHost: example.test\r\n"
                        "Content-Length: 4\r\n\r\nx", "HTTP/1.1 400 Bad Request",
                        "Bad Request", 1) ? 1 : 0;
    if (!strcmp(mode, "body-limit"))
        return exchange(port, "POST /echo HTTP/1.1\r\nHost: example.test\r\n"
                        "Content-Length: 131073\r\n\r\n", "HTTP/1.1 413 Content Too Large",
                        "Content Too Large", 0) ? 1 : 0;
    if (!strcmp(mode, "header-limit")) {
        int fd = connect_to(port);
        if (fd < 0) return 1;
        /* This framing test scans a 64 KiB header in the source VM. Allow the
         * fixture's 10-second client deadline to expire before our receive
         * timeout, so slow CI workers still observe its actual response/EOF.
         * The process-wide alarm remains the outer bound. */
        struct timeval timeout = {.tv_sec = 12, .tv_usec = 0};
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) {
            close(fd);
            return 1;
        }
        const char *prefix = "GET / HTTP/1.1\r\nHost: example.test\r\nX-Large: ";
        int result = send_all(fd, prefix, strlen(prefix));
        char block[4096];
        memset(block, 'a', sizeof(block));
        struct timespec throttle = {.tv_sec = 0, .tv_nsec = 1000000};
        for (size_t sent = 0; !result && sent < 65540;) {
            size_t count = 512;
            if (count > 65540 - sent) count = 65540 - sent;
            result = send_all(fd, block, count);
            sent += count;
            if (!result) nanosleep(&throttle, NULL);
        }
        if (!result) result = response_contains(fd, "HTTP/1.1 431 Request Header Fields Too Large",
                                                  "Request Header Fields Too Large", 0);
        close(fd);
        return result ? 1 : 0;
    }
    if (!strcmp(mode, "disconnect")) {
        int fd = connect_to(port);
        if (fd < 0) return 1;
        const char *partial = "GET / HTTP/1.1\r\nHost:";
        int result = send_all(fd, partial, strlen(partial));
        close(fd);
        return result ? 1 : 0;
    }
    if (!strcmp(mode, "concurrent")) return concurrent_exchange(port) ? 1 : 0;
    return 2;
}
