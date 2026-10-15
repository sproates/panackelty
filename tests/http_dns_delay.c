#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#else
#define _GNU_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

static void mark(const char *name)
{
    const char *path = getenv(name);
    if (!path || !*path) return;
    int descriptor = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (descriptor >= 0) close(descriptor);
}

static int delayed_getaddrinfo(const char *node, const char *service,
                               const struct addrinfo *hints, struct addrinfo **result)
{
    if (node && !strcmp(node, "p5-late-dns.panackelty.invalid")) {
        if (result) *result = NULL;
        mark("PANACK_HTTP_DNS_DELAY_STARTED");
        struct timespec delay = {.tv_sec = 0, .tv_nsec = 400000000};
        while (nanosleep(&delay, &delay) != 0 && errno == EINTR) { }
        mark("PANACK_HTTP_DNS_DELAY_FINISHED");
        return EAI_AGAIN;
    }
    if (node && !strcmp(node, "p5-unknown-dns.panackelty.invalid")) {
        if (result) *result = NULL;
        mark("PANACK_HTTP_DNS_FAILURE");
        return EAI_NONAME;
    }
    if (node && result) {
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        if (inet_pton(AF_INET, node, &address.sin_addr) == 1) {
            struct addrinfo *entry = calloc(1, sizeof(*entry));
            struct sockaddr_in *socket_address = calloc(1, sizeof(*socket_address));
            char *end = NULL;
            unsigned long port = service ? strtoul(service, &end, 10) : 0;
            if (!entry || !socket_address || !service || !end || *end || port > 65535) {
                free(entry);
                free(socket_address);
                return EAI_MEMORY;
            }
            socket_address->sin_family = AF_INET;
            socket_address->sin_port = htons((uint16_t)port);
            socket_address->sin_addr = address.sin_addr;
            entry->ai_family = AF_INET;
            entry->ai_socktype = hints && hints->ai_socktype ? hints->ai_socktype : SOCK_STREAM;
            entry->ai_protocol = hints ? hints->ai_protocol : 0;
            entry->ai_addrlen = sizeof(*socket_address);
            entry->ai_addr = (struct sockaddr *)socket_address;
            *result = entry;
            return 0;
        }
    }
    (void)service;
    (void)hints;
    if (result) *result = NULL;
    return EAI_NONAME;
}

#ifdef __APPLE__
#define DYLD_INTERPOSE(replacement, replacee) \
    __attribute__((used)) static struct { \
        const void *replacement; \
        const void *replacee; \
    } interpose_##replacee __attribute__((section("__DATA,__interpose"))) = { \
        (const void *)(unsigned long)&replacement, \
        (const void *)(unsigned long)&replacee \
    }
DYLD_INTERPOSE(delayed_getaddrinfo, getaddrinfo);
#else
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints, struct addrinfo **result)
{
    return delayed_getaddrinfo(node, service, hints, result);
}
#endif
