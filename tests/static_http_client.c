/* Independent bounded wire assertions for the directory-serving tool. */
#include "../src/vm/platform.h"
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static int connect_to(unsigned port)
{
    struct sockaddr_in address = {.sin_family = AF_INET,
                                  .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
                                  .sin_port = htons((uint16_t)port)};
    struct timeval timeout = {.tv_sec = 4};
    struct timespec delay = {.tv_nsec = 50000000};
    for (size_t attempt = 0; attempt < 100; attempt++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            return -1;
        }
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) ||
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))) {
            close(fd);
            return -1;
        }
        if (!connect(fd, (struct sockaddr *)&address, sizeof(address))) {
            return fd;
        }
        int error = errno;
        close(fd);
        if (error != ECONNREFUSED && error != EINTR) {
            return -1;
        }
        nanosleep(&delay, NULL);
    }
    return -1;
}

static int exchange(unsigned port, const char *method, const char *target, unsigned status,
                    const char *header, const char *body, size_t body_length)
{
    int fd = connect_to(port);
    if (fd < 0) {
        return 1;
    }
    char request[4096];
    int length = snprintf(request, sizeof(request), "%s %s HTTP/1.1\r\nHost: localhost\r\n\r\n",
                          method, target);
    if (length < 0 || (size_t)length >= sizeof(request)) {
        close(fd);
        return 1;
    }
    size_t sent = 0;
    while (sent < (size_t)length) {
        ssize_t count = send(fd, request + sent, (size_t)length - sent, 0);
        if (count <= 0) {
            close(fd);
            return 1;
        }
        sent += (size_t)count;
    }
    size_t capacity = 1048576 + 4096, received = 0;
    char *response = malloc(capacity);
    if (!response) {
        close(fd);
        return 1;
    }
    int failed = 0;
    while (received + 1 < capacity) {
        ssize_t count = read(fd, response + received, capacity - received - 1);
        if (!count) {
            break;
        }
        if (count < 0) {
            failed = 1;
            break;
        }
        received += (size_t)count;
    }
    close(fd);
    response[received] = 0;
    char expected[64];
    snprintf(expected, sizeof(expected), "HTTP/1.1 %u ", status);
    char *separator = strstr(response, "\r\n\r\n");
    failed |= strncmp(response, expected, strlen(expected)) != 0 || !separator ||
              !strstr(response, "\r\nCache-Control: no-store\r\n") ||
              !strstr(response, "\r\nX-Robots-Tag: noindex\r\n") ||
              (header && !strstr(response, header));
    if (separator) {
        size_t actual = received - (size_t)(separator + 4 - response);
        if (!strcmp(method, "HEAD")) {
            failed |= actual != 0;
        } else if (body) {
            failed |= actual != body_length || memcmp(separator + 4, body, body_length);
        } else if (body_length) {
            failed |= actual != body_length;
            for (size_t i = 0; i < actual; i++) {
                failed |= separator[4 + i] != 0;
            }
        }
    }
    if (failed) {
        fprintf(stderr, "static HTTP assertion failed: %s %s expected %u\n%.160s\n",
                method, target, status, response);
    }
    free(response);
    return failed;
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    alarm(30);
    if (argc != 3) {
        return 2;
    }
    char *end;
    unsigned long parsed = strtoul(argv[1], &end, 10);
    if (*end || !parsed || parsed > 65535) {
        return 2;
    }
    unsigned port = (unsigned)parsed;
    const char *mode = argv[2];
    if (!strcmp(mode, "changed")) {
        return exchange(port, "GET", "/hello.txt", 200, NULL, "edited", 6);
    }
    if (!strcmp(mode, "root-changed")) {
        return exchange(port, "GET", "/", 503, NULL, NULL, 0);
    }
    if (!strcmp(mode, "empty")) {
        return exchange(port, "GET", "/", 404, NULL, NULL, 0);
    }
    if (strcmp(mode, "files")) {
        return 2;
    }
    int failed = 0;
    failed |= exchange(port, "GET", "/", 200, "Content-Type: text/html; charset=utf-8\r\n", "root page", 9);
    failed |= exchange(port, "GET", "/sub?x=1", 308, "Location: /sub/?x=1\r\n", "", 0);
    failed |= exchange(port, "HEAD", "/sub?x=1", 308, "Location: /sub/?x=1\r\n", NULL, 0);
    failed |= exchange(port, "GET", "/sub/?x=1", 200, NULL, "sub page", 8);
    failed |= exchange(port, "GET", "/hello.txt?ignored=yes", 200, NULL, "hello\0world", 11);
    failed |= exchange(port, "HEAD", "/hello.txt", 200, "Content-Length: 11\r\n", NULL, 0);
    failed |= exchange(port, "GET", "/inside", 200, NULL, "hello\0world", 11);
    failed |= exchange(port, "GET", "/a%20b.txt", 200, NULL, "space", 5);
    failed |= exchange(port, "GET", "/%C3%A9.txt", 200, NULL, "unicode", 7);
    failed |= exchange(port, "GET", "/%252e.txt", 200, NULL, "literal percent", 15);
    failed |= exchange(port, "GET", "/asset.bin", 200, "Content-Length: 700000\r\n", NULL, 700000);
    failed |= exchange(port, "GET", "/style.CSS", 200, "Content-Type: text/css; charset=utf-8\r\n", "css", 3);
    failed |= exchange(port, "GET", "/large.bin", 503, NULL, NULL, 0);
    failed |= exchange(port, "HEAD", "/large.bin", 503, NULL, NULL, 0);
    failed |= exchange(port, "GET", "/framing.bin", 503, NULL, NULL, 0);
    const char *missing[] = {"/missing", "/empty/", "/outside", "/absolute", "/loop", "/pipe", "/hello.txt/"};
    for (size_t i = 0; i < sizeof(missing) / sizeof(*missing); i++) {
        failed |= exchange(port, "GET", missing[i], 404, NULL, NULL, 0);
    }
    const char *invalid[] = {"/%2e%2e/secret", "/../secret", "/%FF", "/a%2fb", "/a%5cb", "/%00", "/%", "//remote/path"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
        failed |= exchange(port, "GET", invalid[i], 400, NULL, NULL, 0);
    }
    const char *methods[] = {"POST", "PUT", "DELETE", "OPTIONS"};
    for (size_t i = 0; i < sizeof(methods) / sizeof(*methods); i++) {
        failed |= exchange(port, methods[i], "/", 405, "Allow: GET, HEAD\r\n", NULL, 0);
    }
    failed |= exchange(port, "HEAD", "/missing", 404, NULL, NULL, 0);
    failed |= exchange(port, "HEAD", "/%FF", 400, NULL, NULL, 0);
    /* An idle admitted client must not prevent another complete request. */
    int idle = connect_to(port);
    if (idle < 0) {
        failed = 1;
    } else {
        failed |= exchange(port, "GET", "/", 200, NULL, "root page", 9);
        close(idle);
    }
    return failed ? 1 : 0;
}
