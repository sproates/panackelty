#ifndef PANACKELTY_HTTP_TRANSPORT_H
#define PANACKELTY_HTTP_TRANSPORT_H

#include "value.h"

/* A cancellable owner-thread HTTP transport operation. The adapter owns its
 * libcurl handles and buffers; it returns only raw response bytes or a stable
 * transport category. HTTP framing remains in the Panackelty package. */
typedef struct VMHttpTransport VMHttpTransport;
VMHttpTransport *http_transport_start(Value **arguments);
bool http_transport_poll(VMHttpTransport *transport, unsigned max_wait_ms);
Value *http_transport_result(const VMHttpTransport *transport);
void http_transport_destroy(VMHttpTransport *transport);

#endif
