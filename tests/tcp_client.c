#include "../src/vm/platform.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static int connect_to(unsigned port)
{
    for (size_t attempt = 0; attempt < 400; attempt++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        assert(fd >= 0);
        struct sockaddr_in address = {.sin_family = AF_INET,
                                      .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
                                      .sin_port = htons((uint16_t)port)};
        if (!connect(fd, (struct sockaddr *)&address, sizeof(address))) {
            return fd;
        }
        close(fd);
        struct timespec delay = {.tv_nsec = 10000000};
        nanosleep(&delay, NULL);
    }
    fprintf(stderr, "server never accepted a connection\n");
    exit(1);
}

static void exchange(int fd, const unsigned char *request, size_t length, int expect_error)
{
    size_t sent = 0;
    while (sent < length) {
        size_t chunk = length - sent;
        if (chunk > 137) {
            chunk = 137;
        }
        ssize_t n = send(fd, request + sent, chunk, 0);
        if (n < 0 && expect_error) {
            break;
        }
        assert(n > 0);
        sent += (size_t)n;
    }
    if (shutdown(fd, SHUT_WR) < 0) {
        assert(expect_error);
    }
    unsigned char response[32768];
    size_t received = 0;
    for (;;) {
        struct pollfd ready = {.fd = fd, .events = POLLIN};
        assert(poll(&ready, 1, 4000) == 1);
        ssize_t n = recv(fd, response + received, sizeof(response) - received, 0);
        if (!n || (n < 0 && expect_error && errno == ECONNRESET)) {
            break;
        }
        assert(n > 0);
        received += (size_t)n;
        assert(received < sizeof(response));
    }
    if (expect_error) {
        assert(received == 0);
    } else {
        assert(received == length && !memcmp(response, request, length));
    }
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    if (argc == 2 && !strcmp(argv[1], "port")) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        assert(fd >= 0);
        struct sockaddr_in address = {.sin_family = AF_INET,
                                      .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
        assert(!bind(fd, (struct sockaddr *)&address, sizeof(address)));
        socklen_t size = sizeof(address);
        assert(!getsockname(fd, (struct sockaddr *)&address, &size));
        printf("%u\n", (unsigned)ntohs(address.sin_port));
        close(fd);
        return 0;
    }
    assert(argc == 3);
    unsigned port = (unsigned)strtoul(argv[1], NULL, 10);
    unsigned char request[24576];
    for (size_t i = 0; i < sizeof(request); i++) {
        request[i] = (unsigned char)(i % 251);
    }
    if (!strcmp(argv[2], "example")) {
        int fd = connect_to(port);
        exchange(fd, request, 3, 0);
        close(fd);
        return 0;
    }
    int slow = connect_to(port);
    if (!strcmp(argv[2], "busy")) {
        assert(send(slow, "x", 1, 0) == 1 && !shutdown(slow, SHUT_WR));
    }
    int fast = connect_to(port);
    int error = strcmp(argv[2], "echo") && strcmp(argv[2], "empty") && strcmp(argv[2], "busy");
    size_t length = (!strcmp(argv[2], "empty") || !strcmp(argv[2], "busy")) ? 0 : sizeof(request);
    /* The first client withholds EOF until the second gets its reply. A serial
     * server times out the first client and cannot satisfy its expected report. */
    exchange(fast, request, length, error);
    close(fast);
    exchange(slow, request, 0,
             (!strcmp(argv[2], "handler-error") || !strcmp(argv[2], "busy") ||
              !strcmp(argv[2], "nested")));
    close(slow);
    return 0;
}
