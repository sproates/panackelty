/* Loopback-only fixture, independent of the VM implementation. */
#include "../src/vm/platform.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    alarm(15);
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK)};
    socklen_t size = sizeof(address);
    if (server < 0 || bind(server, (struct sockaddr *)&address, size) ||
        getsockname(server, (struct sockaddr *)&address, &size)) return 1;
    if (strcmp(argv[1], "refuse") && listen(server, 2)) return 1;
    if (!strcmp(argv[1], "refuse")) close(server);
    printf("%u\n", ntohs(address.sin_port));
    fflush(stdout);
    if (!strcmp(argv[1], "refuse")) { pause(); return 0; }
    for (int attempt = 0; attempt < 2; attempt++) {
        int peer = accept(server, NULL, NULL);
        if (peer < 0) return 1;
        if (!strcmp(argv[1], "stall")) { sleep(2); close(peer); continue; }
        unsigned char request[131072];
        size_t total = 0;
        ssize_t n;
        while ((n = read(peer, request + total, sizeof(request) - total)) > 0) total += (size_t)n;
        if (n < 0 || total == sizeof(request)) return 1;
        if (!strcmp(argv[1], "limit")) total = 1;
        /* Deliberately fragment the reply, including binary data and EOF. */
        for (size_t sent = 0; sent < total;) {
            size_t count = total - sent;
            if (count > 137) count = 137;
            n = write(peer, request + sent, count);
            if (n <= 0) return 1;
            sent += (size_t)n;
        }
        close(peer);
    }
    close(server);
    return 0;
}
