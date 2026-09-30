#ifndef PANACKELTY_TCP_H
#define PANACKELTY_TCP_H

#include "value.h"

/* One owner-thread operation owns its socket, retained request and bounded
 * response. No callbacks, threads or resource handles escape it. */
typedef struct VMTcpExchange VMTcpExchange;
VMTcpExchange *tcp_exchange_start(Value **arguments);
/* At most one bounded send/receive per call. max_wait_ms=0 never waits for I/O.
 * True means terminal (including allocation failure, indicated by NULL result). */
bool tcp_exchange_poll(VMTcpExchange *exchange, unsigned max_wait_ms);
Value *tcp_exchange_result(const VMTcpExchange *exchange);
void tcp_exchange_destroy(VMTcpExchange *exchange);

#endif
