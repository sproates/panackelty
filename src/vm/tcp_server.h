#ifndef PANACKELTY_TCP_SERVER_H
#define PANACKELTY_TCP_SERVER_H

#include "vm.h"

/* Internal, single-owner-thread API. The verified program and VM snapshots must
 * outlive the server. The server owns sockets, child executions and reports. */
typedef struct VMTcpServer VMTcpServer;

typedef struct {
    size_t clients, concurrency, request_limit, response_limit;
    size_t client_timeout_ms, admission_timeout_ms, drain_ms;
} VMTcpServerLimits;

VMTcpServer *tcp_server_start(const VM *vm, Function *handler, const Value *address, size_t port,
                              VMTcpServerLimits limits, bool outbound);
/* Socket work only; no bytecode. A zero wait never blocks for readiness. */
void tcp_server_poll(VMTcpServer *server, unsigned max_wait_ms);
/* At most budget bytecode instructions, round robin; no blocking socket calls. */
void tcp_server_advance(VMTcpServer *server, size_t budget);
void tcp_server_stop(VMTcpServer *server);
bool tcp_server_done(const VMTcpServer *server);
Value *tcp_server_result(const VMTcpServer *server);     /* borrowed */
const char *tcp_server_error(const VMTcpServer *server); /* static trap, or NULL */
void tcp_server_destroy(VMTcpServer *server);

#endif
