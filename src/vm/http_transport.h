#ifndef PANACKELTY_HTTP_TRANSPORT_H
#define PANACKELTY_HTTP_TRANSPORT_H

#include "value.h"
#include "vm.h"
#include <stdint.h>

typedef enum {
    VM_HTTP_WAIT_PENDING,
    VM_HTTP_WAIT_READY,
    VM_HTTP_WAIT_TIMEOUT,
    VM_HTTP_WAIT_INVALID
} VMHttpWaitOutcome;

/* Resolve the readiness/deadline boundary after a host poll. Kept in the
 * private transport interface so the boundary can be tested deterministically. */
VMHttpWaitOutcome http_transport_classify_wait(uint64_t now, uint64_t deadline,
                                               int poll_result, short revents);

/* A cancellable owner-thread HTTP transport operation. The adapter owns its
 * libcurl handles and buffers; it returns only raw response bytes or a stable
 * transport category. HTTP framing remains in the Panackelty package. */
typedef struct VMHttpTransport VMHttpTransport;
VMHttpTransport *http_transport_start(Value **arguments);
bool http_transport_poll(VMHttpTransport *transport, unsigned max_wait_ms);
Value *http_transport_result(const VMHttpTransport *transport);
void http_transport_destroy(VMHttpTransport *transport);
Value *http_stream_call(VM *vm, const char *name, Value **arguments);
void http_stream_destroy(VM *vm);

#endif
