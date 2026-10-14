#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define REQUEST_LIMIT 65536

static volatile sig_atomic_t stopping;

static void stop_server(int signal_number)
{
    (void)signal_number;
    stopping = 1;
}

static int send_all(int descriptor, const char *bytes, size_t length)
{
    size_t sent = 0;
    while (sent < length) {
        ssize_t result = send(descriptor, bytes + sent, length - sent, 0);
        if (result < 0 && errno == EINTR) continue;
        if (result <= 0) return -1;
        sent += (size_t)result;
    }
    return 0;
}

static bool headers_complete(const char *request, size_t length)
{
    for (size_t i = 0; i + 3 < length; i++) {
        if (request[i] == '\r' && request[i + 1] == '\n' &&
            request[i + 2] == '\r' && request[i + 3] == '\n') return true;
    }
    return false;
}

static void respond(int descriptor, const char *target)
{
    static const char ok[] =
        "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n"
        "X-Repeat: one\r\nX-Repeat: two\r\nConnection: close\r\n\r\nok";
    static const char missing[] =
        "HTTP/1.1 404 Not Found\r\nContent-Length: 7\r\nConnection: close\r\n\r\nmissing";
    static const char chunked[] =
        "HTTP/1.1 100 Continue\r\n\r\n"
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n"
        "X-Repeat: one\r\nX-Repeat: two\r\nConnection: close\r\n\r\n"
        "5;name=value\r\nhello\r\n0\r\nX-Trailer: ignored\r\n\r\n";
    static const char close_body[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nclose body";
    static const char truncated[] = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nx";
    static const char duplicate[] =
        "HTTP/1.1 200 OK\r\nContent-Length: 1\r\nContent-Length: 1\r\n"
        "Connection: close\r\n\r\nx";
    static const char oversize[] =
        "HTTP/1.1 200 OK\r\nContent-Length: 4\r\nConnection: close\r\n\r\nfour";
    const char *response = ok;
    size_t response_length = sizeof(ok) - 1;

    if (strncmp(target, "/delay", 6) == 0) {
        struct timespec delay = {.tv_sec = 0, .tv_nsec = 400000000};
        while (nanosleep(&delay, &delay) != 0 && errno == EINTR) { }
    }
    if (strncmp(target, "/chunked", 8) == 0) {
        response = chunked;
        response_length = sizeof(chunked) - 1;
    } else if (strncmp(target, "/close", 6) == 0) {
        response = close_body;
        response_length = sizeof(close_body) - 1;
    } else if (strncmp(target, "/truncated", 10) == 0) {
        response = truncated;
        response_length = sizeof(truncated) - 1;
    } else if (strncmp(target, "/duplicate", 10) == 0) {
        response = duplicate;
        response_length = sizeof(duplicate) - 1;
    } else if (strncmp(target, "/oversize", 9) == 0) {
        response = oversize;
        response_length = sizeof(oversize) - 1;
    } else if (strncmp(target, "/missing", 8) == 0) {
        response = missing;
        response_length = sizeof(missing) - 1;
    }
    (void)send_all(descriptor, response, response_length);
}

static int reserve_port(const char *path)
{
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) return 1;
    int enabled = 1;
    (void)setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(UINT32_C(0x7f000001));
    address.sin_port = 0;
    if (bind(listener, (struct sockaddr *)&address, sizeof(address)) != 0 || listen(listener, 16) != 0) {
        close(listener);
        return 1;
    }
    socklen_t address_length = sizeof(address);
    if (getsockname(listener, (struct sockaddr *)&address, &address_length) != 0) {
        close(listener);
        return 1;
    }
    FILE *port_file = fopen(path, "w");
    if (!port_file) {
        close(listener);
        return 1;
    }
    fprintf(port_file, "%u\n", (unsigned)ntohs(address.sin_port));
    if (fclose(port_file) != 0) {
        close(listener);
        return 1;
    }
    close(listener);
    return 0;
}

static int probe_port(const char *text)
{
    char *end = NULL;
    long port = strtol(text, &end, 10);
    if (!end || *end || port < 1 || port > 65535) return 2;
    int descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor < 0) return 1;
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(UINT32_C(0x7f000001));
    address.sin_port = htons((uint16_t)port);
    int result = connect(descriptor, (struct sockaddr *)&address, sizeof(address));
    close(descriptor);
    return result == 0 ? 0 : 1;
}

int main(int argc, char **argv)
{
    if (argc == 3 && strcmp(argv[1], "--reserve-port") == 0) return reserve_port(argv[2]);
    if (argc == 3 && strcmp(argv[1], "--probe") == 0) return probe_port(argv[2]);
    if (argc != 2) return 2;
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) return 1;
    int enabled = 1;
    (void)setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(UINT32_C(0x7f000001));
    address.sin_port = 0;
    if (bind(listener, (struct sockaddr *)&address, sizeof(address)) != 0 || listen(listener, 16) != 0) {
        close(listener);
        return 1;
    }
    socklen_t address_length = sizeof(address);
    if (getsockname(listener, (struct sockaddr *)&address, &address_length) != 0) {
        close(listener);
        return 1;
    }
    FILE *port_file = fopen(argv[1], "w");
    if (!port_file) {
        close(listener);
        return 1;
    }
    fprintf(port_file, "%u\n", (unsigned)ntohs(address.sin_port));
    if (fclose(port_file) != 0) {
        close(listener);
        return 1;
    }
    struct sigaction action = {0};
    action.sa_handler = stop_server;
    sigemptyset(&action.sa_mask);
    (void)sigaction(SIGTERM, &action, NULL);
    (void)sigaction(SIGINT, &action, NULL);
    action.sa_handler = SIG_IGN;
    (void)sigaction(SIGPIPE, &action, NULL);

    while (!stopping) {
        int client = accept(listener, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) continue;
            close(listener);
            return 1;
        }
        char request[REQUEST_LIMIT + 1];
        size_t used = 0;
        while (used < REQUEST_LIMIT && !headers_complete(request, used)) {
            ssize_t count = recv(client, request + used, REQUEST_LIMIT - used, 0);
            if (count < 0 && errno == EINTR) continue;
            if (count <= 0) break;
            used += (size_t)count;
        }
        request[used] = '\0';
        char *first_space = memchr(request, ' ', used);
        char *second_space = first_space ? memchr(first_space + 1, ' ',
            used - (size_t)(first_space + 1 - request)) : NULL;
        if (second_space) {
            *second_space = '\0';
            respond(client, first_space + 1);
        }
        shutdown(client, SHUT_RDWR);
        close(client);
    }
    close(listener);
    return 0;
}
