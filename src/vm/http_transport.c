#include "platform.h"
#include "http_transport.h"

#include <stdlib.h>
#include <string.h>

#ifndef __wasi__
#include <curl/curl.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdatomic.h>
#include <time.h>
#endif

#define HTTP_TRANSPORT_BYTE_LIMIT (16 * 1024 * 1024)
#define HTTP_TRANSPORT_TIMEOUT_LIMIT 60000
#define HTTP_TRANSPORT_CHUNK 16384

struct VMHttpTransport {
    Value *request, *result;
    uint8_t *response;
    size_t sent, received, limit;
    uint64_t deadline;
    bool done, connecting;
#ifndef __wasi__
    CURLM *multi;
    CURL *easy;
    curl_socket_t socket;
#endif
};

#ifndef __wasi__
static atomic_flag curl_init_lock = ATOMIC_FLAG_INIT;
static bool curl_initialized;

static bool initialize_curl(void)
{
    while (atomic_flag_test_and_set_explicit(&curl_init_lock, memory_order_acquire)) { }
    if (!curl_initialized) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK) {
            const curl_version_info_data *version = curl_version_info(CURLVERSION_NOW);
            if (version && (version->features & CURL_VERSION_ASYNCHDNS)) {
                curl_initialized = true;
            } else {
                curl_global_cleanup();
            }
        }
    }
    bool initialized = curl_initialized;
    atomic_flag_clear_explicit(&curl_init_lock, memory_order_release);
    return initialized;
}

static bool monotonic_ms(uint64_t *result)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return false;
    *result = (uint64_t)now.tv_sec * 1000 + (uint64_t)now.tv_nsec / 1000000;
    return true;
}

#endif

static bool valid_host(const Value *host)
{
    if (!host || host->kind != V_STR || !host->as.bytes.length ||
        host->as.bytes.length > 253 || memchr(host->as.bytes.data, 0, host->as.bytes.length)) {
        return false;
    }
    size_t label_length = 0;
    bool label_starts_hyphen = false;
    for (size_t i = 0; i < host->as.bytes.length; i++) {
        uint8_t c = host->as.bytes.data[i];
        if (c == '.') {
            if (!label_length || label_starts_hyphen || host->as.bytes.data[i - 1] == '-') return false;
            label_length = 0;
            label_starts_hyphen = false;
        } else {
            bool alphanumeric = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                (c >= '0' && c <= '9');
            if (!alphanumeric && c != '-') return false;
            if (!label_length) label_starts_hyphen = c == '-';
            if (++label_length > 63) return false;
        }
    }
    return label_length && !label_starts_hyphen && host->as.bytes.data[host->as.bytes.length - 1] != '-';
}

#ifndef __wasi__
static void cleanup_curl(VMHttpTransport *transport)
{
    if (transport->easy && transport->multi) curl_multi_remove_handle(transport->multi, transport->easy);
    if (transport->easy) {
        curl_easy_cleanup(transport->easy);
        transport->easy = NULL;
    }
    if (transport->multi) {
        curl_multi_cleanup(transport->multi);
        transport->multi = NULL;
    }
    transport->socket = CURL_SOCKET_BAD;
}
#endif

static void finish(VMHttpTransport *transport, const char *error)
{
#ifndef __wasi__
    cleanup_curl(transport);
#endif
    transport->done = true;
    Value *payload = value_data(error ? V_STR : V_BYTES,
                                error ? (const uint8_t *)error : transport->response,
                                error ? strlen(error) : transport->received);
    if (payload) {
        transport->result = named_value(V_VARIANT, error ? "Error" : "Ok", NULL, &payload, 1);
        release(payload);
    }
}

#ifndef __wasi__
static const char *connect_error(CURLcode result)
{
    if (result == CURLE_OPERATION_TIMEDOUT) return "HTTP transport timed out";
    if (result == CURLE_COULDNT_RESOLVE_HOST) return "HTTP DNS failed";
    if (result == CURLE_PEER_FAILED_VERIFICATION || result == CURLE_SSL_CACERT_BADFILE ||
        result == CURLE_SSL_ISSUER_ERROR) return "HTTP TLS verification failed";
    if (result == CURLE_SSL_CONNECT_ERROR || result == CURLE_USE_SSL_FAILED ||
        result == CURLE_SSL_CERTPROBLEM) return "HTTP TLS failed";
    return "HTTP connection failed";
}

#endif

VMHttpTransport *http_transport_start(Value **arguments)
{
    VMHttpTransport *transport = calloc(1, sizeof(*transport));
    if (!transport) return NULL;
#ifndef __wasi__
    transport->socket = CURL_SOCKET_BAD;
#endif
    size_t port, timeout;
    if (!valid_host(arguments[0]) || arguments[1]->kind != V_NAT ||
        arguments[2]->kind != V_BOOL || arguments[3]->kind != V_BYTES ||
        arguments[4]->kind != V_NAT || arguments[5]->kind != V_NAT ||
        !value_index(arguments[1], &port) || !value_index(arguments[4], &transport->limit) ||
        !value_index(arguments[5], &timeout) || !port || port > 65535 ||
        arguments[3]->as.bytes.length > HTTP_TRANSPORT_BYTE_LIMIT ||
        transport->limit > HTTP_TRANSPORT_BYTE_LIMIT || !timeout ||
        timeout > HTTP_TRANSPORT_TIMEOUT_LIMIT) {
        finish(transport, "invalid HTTP transport arguments");
        return transport;
    }
#ifdef __wasi__
    finish(transport, "HTTP transport is unavailable on this host");
#else
    if (!initialize_curl()) {
        finish(transport, "HTTP asynchronous DNS is unavailable");
        return transport;
    }
    uint64_t now = 0;
    if (!monotonic_ms(&now) || UINT64_MAX - now < timeout) {
        finish(transport, "HTTP transport clock failed");
        return transport;
    }
    transport->deadline = now + timeout;
    transport->response = malloc(transport->limit ? transport->limit : 1);
    if (!transport->response) {
        transport->done = true;
        return transport;
    }
    transport->request = retain(arguments[3]);
    transport->multi = curl_multi_init();
    transport->easy = curl_easy_init();
    if (!transport->multi || !transport->easy) {
        finish(transport, "HTTP transport initialization failed");
        return transport;
    }
    char url[300];
    int url_length = snprintf(url, sizeof(url), "%s://%.*s:%zu/",
                              arguments[2]->as.boolean ? "https" : "http",
                              (int)arguments[0]->as.bytes.length,
                              arguments[0]->as.bytes.data, port);
    if (url_length < 0 || (size_t)url_length >= sizeof(url)) {
        finish(transport, "invalid HTTP transport arguments");
        return transport;
    }
    CURLcode easy_result = CURLE_OK;
#define SETOPT(option, value) do { \
    if (easy_result == CURLE_OK) easy_result = curl_easy_setopt(transport->easy, option, value); \
} while (0)
    SETOPT(CURLOPT_URL, url);
    SETOPT(CURLOPT_CONNECT_ONLY, 1L);
    SETOPT(CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_1_1);
    SETOPT(CURLOPT_NOSIGNAL, 1L);
    SETOPT(CURLOPT_TIMEOUT_MS, (long)timeout);
    SETOPT(CURLOPT_CONNECTTIMEOUT_MS, (long)timeout);
    SETOPT(CURLOPT_PROXY, "");
    SETOPT(CURLOPT_NOPROXY, "*");
    SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
    SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
#if LIBCURL_VERSION_NUM >= 0x075500
    SETOPT(CURLOPT_PROTOCOLS_STR, "http,https");
#else
    SETOPT(CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
#endif
    const char *ca_bundle = getenv("CURL_CA_BUNDLE");
    if (ca_bundle && *ca_bundle) SETOPT(CURLOPT_CAINFO, ca_bundle);
#undef SETOPT
    if (easy_result != CURLE_OK || curl_multi_add_handle(transport->multi, transport->easy) != CURLM_OK) {
        finish(transport, "HTTP transport initialization failed");
        return transport;
    }
    transport->connecting = true;
#endif
    return transport;
}

#ifndef __wasi__
VMHttpWaitOutcome http_transport_classify_wait(uint64_t now, uint64_t deadline,
                                               int poll_result, short revents)
{
    if (now >= deadline) return VM_HTTP_WAIT_TIMEOUT;
    if (poll_result > 0 && (revents & POLLNVAL)) return VM_HTTP_WAIT_INVALID;
    if (poll_result > 0) return VM_HTTP_WAIT_READY;
    return VM_HTTP_WAIT_PENDING;
}

static bool wait_socket(VMHttpTransport *transport, short events, unsigned max_wait_ms)
{
    uint64_t now;
    if (!monotonic_ms(&now)) {
        finish(transport, "HTTP transport clock failed");
        return false;
    }
    if (now >= transport->deadline) {
        finish(transport, "HTTP transport timed out");
        return false;
    }
    uint64_t remaining = transport->deadline - now;
    unsigned wait = max_wait_ms > INT_MAX ? INT_MAX : max_wait_ms;
    if (remaining < wait) wait = (unsigned)remaining;
    struct pollfd descriptor = {.fd = transport->socket, .events = events};
    int ready = poll(&descriptor, 1, (int)wait);
    if (ready < 0 && errno != EINTR) {
        finish(transport, "HTTP transport polling failed");
        return false;
    }
    if (!monotonic_ms(&now)) finish(transport, "HTTP transport clock failed");
    else {
        VMHttpWaitOutcome outcome = http_transport_classify_wait(
            now, transport->deadline, ready, descriptor.revents);
        if (outcome == VM_HTTP_WAIT_TIMEOUT) finish(transport, "HTTP transport timed out");
        else if (outcome == VM_HTTP_WAIT_INVALID) finish(transport, "HTTP connection failed");
        else return outcome == VM_HTTP_WAIT_READY;
    }
    return false;
}

static bool poll_connect(VMHttpTransport *transport, unsigned max_wait_ms)
{
    int running = 0;
    CURLMcode code = curl_multi_perform(transport->multi, &running);
    if (code != CURLM_OK) {
        finish(transport, "HTTP connection failed");
        return transport->done;
    }
    uint64_t now = 0;
    if (!monotonic_ms(&now)) finish(transport, "HTTP transport clock failed");
    else if (now >= transport->deadline) finish(transport, "HTTP transport timed out");
    if (transport->done) return true;
    int queued = 0;
    CURLMsg *message;
    while ((message = curl_multi_info_read(transport->multi, &queued))) {
        if (message->msg != CURLMSG_DONE) continue;
        if (message->data.result != CURLE_OK) {
            finish(transport, connect_error(message->data.result));
            return transport->done;
        }
        if (curl_easy_getinfo(transport->easy, CURLINFO_ACTIVESOCKET, &transport->socket) != CURLE_OK ||
            transport->socket == CURL_SOCKET_BAD) {
            finish(transport, "HTTP connection failed");
            return transport->done;
        }
        transport->connecting = false;
        return false;
    }
    if (!monotonic_ms(&now)) finish(transport, "HTTP transport clock failed");
    else if (now >= transport->deadline) finish(transport, "HTTP transport timed out");
    if (transport->done) return true;
    uint64_t remaining = transport->deadline - now;
    unsigned wait = max_wait_ms > INT_MAX ? INT_MAX : max_wait_ms;
    if (remaining < wait) wait = (unsigned)remaining;
    int descriptors = 0;
    code = curl_multi_poll(transport->multi, NULL, 0, (int)wait, &descriptors);
    if (code != CURLM_OK) finish(transport, "HTTP transport polling failed");
    return transport->done;
}
#endif

bool http_transport_poll(VMHttpTransport *transport, unsigned max_wait_ms)
{
    if (transport->done) return true;
#ifdef __wasi__
    (void)max_wait_ms;
    finish(transport, "HTTP transport is unavailable on this host");
#else
    uint64_t now;
    if (!monotonic_ms(&now)) finish(transport, "HTTP transport clock failed");
    else if (now >= transport->deadline) finish(transport, "HTTP transport timed out");
    if (transport->done) return true;
    if (transport->connecting) return poll_connect(transport, max_wait_ms);
    if (!wait_socket(transport, transport->sent < transport->request->as.bytes.length ? POLLOUT : POLLIN,
                     max_wait_ms)) return transport->done;
    if (transport->sent < transport->request->as.bytes.length) {
        size_t left = transport->request->as.bytes.length - transport->sent;
        if (left > HTTP_TRANSPORT_CHUNK) left = HTTP_TRANSPORT_CHUNK;
        size_t sent = 0;
        CURLcode code = curl_easy_send(transport->easy,
                                      transport->request->as.bytes.data + transport->sent,
                                      left, &sent);
        if (code == CURLE_OK) {
            transport->sent += sent;
            return false;
        }
        if (code != CURLE_AGAIN) finish(transport, "HTTP transport write failed");
        return transport->done;
    }
    uint8_t buffer[HTTP_TRANSPORT_CHUNK];
    size_t received = 0;
    CURLcode code = curl_easy_recv(transport->easy, buffer, sizeof(buffer), &received);
    if (code == CURLE_OK) {
        if (!received) {
            finish(transport, NULL);
        } else if (received > transport->limit - transport->received) {
            finish(transport, "HTTP response limit exceeded");
        } else {
            memcpy(transport->response + transport->received, buffer, received);
            transport->received += received;
        }
    } else if (code != CURLE_AGAIN) {
        finish(transport, "HTTP transport read failed");
    }
#endif
    return transport->done;
}

Value *http_transport_result(const VMHttpTransport *transport)
{
    return transport->result;
}

void http_transport_destroy(VMHttpTransport *transport)
{
    if (!transport) return;
#ifndef __wasi__
    cleanup_curl(transport);
#endif
    release(transport->request);
    release(transport->result);
    free(transport->response);
    free(transport);
}
