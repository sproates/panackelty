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
