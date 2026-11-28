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
    size_t sent, received, limit, total;
    uint64_t deadline;
    bool done, connecting, streaming, paused, detailed_errors;
#ifndef __wasi__
    CURLM *multi;
    CURL *easy;
    char error_buffer[CURL_ERROR_SIZE];
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

static bool monotonic_ns(uint64_t *result)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return false;
    *result = (uint64_t)now.tv_sec * 1000000000 + (uint64_t)now.tv_nsec;
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
/* Keep compatibility strings unchanged. Typed streams retain bounded libcurl
 * diagnostics before cleanup destroys the easy handle. Never retain request bytes. */
static void finish_curl(VMHttpTransport *transport, const char *category, CURLcode code)
{
    if (!transport->detailed_errors) { finish(transport, category); return; }
    char message[CURL_ERROR_SIZE + 256];
    const char *detail = transport->error_buffer[0] ? transport->error_buffer : curl_easy_strerror(code);
    snprintf(message, sizeof(message), "%s\nlibcurl error %d: %s", category, (int)code, detail);
    finish(transport, message);
}

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

static VMHttpTransport *transport_start(Value **arguments, bool streaming, bool duration)
{
    VMHttpTransport *transport = calloc(1, sizeof(*transport));
    if (!transport) return NULL;
    transport->streaming = streaming;
    transport->detailed_errors = duration;
#ifndef __wasi__
    transport->socket = CURL_SOCKET_BAD;
#endif
    size_t port, timeout;
    if (!valid_host(arguments[0]) || arguments[1]->kind != V_NAT ||
        arguments[2]->kind != V_BOOL || arguments[3]->kind != V_BYTES ||
        arguments[4]->kind != V_NAT ||
        arguments[5]->kind != (duration ? V_DURATION : V_NAT) ||
        !value_index(arguments[1], &port) || !value_index(arguments[4], &transport->limit) ||
        !pn_big_fits_size(&arguments[5]->as.integer, &timeout) || !port || port > 65535 ||
        arguments[3]->as.bytes.length > (streaming ? 65536 : HTTP_TRANSPORT_BYTE_LIMIT) ||
        transport->limit > (streaming ? 268435456 : HTTP_TRANSPORT_BYTE_LIMIT) || !timeout ||
        timeout > (duration ? UINT64_C(60000000000) : HTTP_TRANSPORT_TIMEOUT_LIMIT)) {
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
    uint64_t timeout_ns = duration ? timeout : (uint64_t)timeout * 1000000;
    uint64_t now = 0;
    if (!monotonic_ns(&now) || UINT64_MAX - now < timeout_ns) {
        finish(transport, "HTTP transport clock failed");
        return transport;
    }
    transport->deadline = now + timeout_ns;
    /* libcurl accepts milliseconds. Round its backstop up; our authoritative
     * monotonic deadline retains the exact Duration, including sub-ms values. */
    long timeout_ms = (long)((timeout_ns + 999999) / 1000000);
    transport->response = malloc(streaming ? HTTP_TRANSPORT_CHUNK : (transport->limit ? transport->limit : 1));
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
    SETOPT(CURLOPT_ERRORBUFFER, transport->error_buffer);
    SETOPT(CURLOPT_URL, url);
    SETOPT(CURLOPT_CONNECT_ONLY, 1L);
    SETOPT(CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_1_1);
    SETOPT(CURLOPT_NOSIGNAL, 1L);
    SETOPT(CURLOPT_TIMEOUT_MS, timeout_ms);
    SETOPT(CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
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

VMHttpTransport *http_transport_start(Value **arguments)
{
    return transport_start(arguments, false, false);
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
    if (!monotonic_ns(&now)) {
        finish(transport, "HTTP transport clock failed");
        return false;
    }
    if (now >= transport->deadline) {
        finish(transport, "HTTP transport timed out");
        return false;
    }
    uint64_t remaining = (transport->deadline - now + 999999) / 1000000;
    unsigned wait = max_wait_ms > INT_MAX ? INT_MAX : max_wait_ms;
    if (remaining < wait) wait = (unsigned)remaining;
    struct pollfd descriptor = {.fd = transport->socket, .events = events};
    int ready = poll(&descriptor, 1, (int)wait);
    if (ready < 0 && errno != EINTR) {
        finish(transport, "HTTP transport polling failed");
        return false;
    }
    if (!monotonic_ns(&now)) finish(transport, "HTTP transport clock failed");
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
    if (!monotonic_ns(&now)) finish(transport, "HTTP transport clock failed");
    else if (now >= transport->deadline) finish(transport, "HTTP transport timed out");
    if (transport->done) return true;
    int queued = 0;
    CURLMsg *message;
    while ((message = curl_multi_info_read(transport->multi, &queued))) {
        if (message->msg != CURLMSG_DONE) continue;
        if (message->data.result != CURLE_OK) {
            finish_curl(transport, connect_error(message->data.result), message->data.result);
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
    if (!monotonic_ns(&now)) finish(transport, "HTTP transport clock failed");
    else if (now >= transport->deadline) finish(transport, "HTTP transport timed out");
    if (transport->done) return true;
    uint64_t remaining = (transport->deadline - now + 999999) / 1000000;
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
    if (transport->done || transport->paused) return true;
#ifdef __wasi__
    (void)max_wait_ms;
    finish(transport, "HTTP transport is unavailable on this host");
#else
    uint64_t now;
    if (!monotonic_ns(&now)) finish(transport, "HTTP transport clock failed");
    else if (now >= transport->deadline) finish(transport, "HTTP transport timed out");
    if (transport->done) return true;
    if (transport->connecting) return poll_connect(transport, max_wait_ms);
    if (!wait_socket(transport, transport->sent < transport->request->as.bytes.length ? POLLOUT : POLLIN,
                     max_wait_ms)) return transport->done;
    if (transport->sent < transport->request->as.bytes.length) {
        size_t left = transport->request->as.bytes.length - transport->sent;
        if (left > HTTP_TRANSPORT_CHUNK) left = HTTP_TRANSPORT_CHUNK;
        size_t sent = 0;
        transport->error_buffer[0] = 0;
        CURLcode code = curl_easy_send(transport->easy,
                                      transport->request->as.bytes.data + transport->sent,
                                      left, &sent);
        if (code == CURLE_OK) {
            transport->sent += sent;
            return false;
        }
        if (code != CURLE_AGAIN) finish_curl(transport, "HTTP transport write failed", code);
        return transport->done;
    }
    uint8_t buffer[HTTP_TRANSPORT_CHUNK];
    size_t received = 0;
    transport->error_buffer[0] = 0;
    CURLcode code = curl_easy_recv(transport->easy, buffer, sizeof(buffer), &received);
    if (code == CURLE_OK) {
        if (!received) {
            finish(transport, NULL);
        } else if (received > transport->limit - transport->total) {
            finish(transport, "HTTP response limit exceeded");
        } else {
            memcpy(transport->response + transport->received, buffer, received);
            transport->received += received;
            transport->total += received;
            if (transport->streaming) {
                Value *chunk = value_data(V_BYTES, transport->response, transport->received);
                transport->result = chunk ? named_value(V_VARIANT, "Ok", NULL, &chunk, 1) : NULL;
                release(chunk);
                transport->paused = true;
            }
        }
    } else if (code != CURLE_AGAIN) {
        finish_curl(transport, "HTTP transport read failed", code);
    }
#endif
    return transport->done || transport->paused;
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

/* Tables and leases are owner-thread resources, not worker-pool tasks. Slots
 * are never reused, so a copied handle cannot accidentally name a new stream. */
#define HTTP_STREAM_HANDLES 512
struct VMHttpStreams {
    VMHttpTransport *slots[HTTP_STREAM_HANDLES];
    bool busy[HTTP_STREAM_HANDLES], typed[HTTP_STREAM_HANDLES];
    size_t count, refs;
    bool closing;
};
struct VMHttpStreamRead { VMHttpStreams *owner; size_t slot; };

static Value *stream_error(const char *message)
{
    Value *text = value_data(V_STR, (const uint8_t *)message, strlen(message));
    Value *result = text ? named_value(V_VARIANT, "Error", NULL, &text, 1) : NULL;
    release(text);
    return result;
}

static void close_slot(VMHttpStreams *owner, size_t slot)
{
    http_transport_destroy(owner->slots[slot]);
    owner->slots[slot] = NULL;
}

void http_stream_owner_release(VMHttpStreams *owner)
{
    if (owner && !--owner->refs) free(owner);
}

static Value *open_stream(VM *vm, Value **arguments, bool typed)
{
    if (!vm->http_streams) {
        vm->http_streams = calloc(1, sizeof(*vm->http_streams));
        if (!vm->http_streams) return NULL;
        vm->http_streams->refs = 1;
    }
    VMHttpStreams *owner = vm->http_streams;
    if (owner->closing || owner->count == HTTP_STREAM_HANDLES)
        return stream_error("HTTP stream resource limit exceeded");
    VMHttpTransport *stream = transport_start(arguments, true, typed);
    if (!stream) return NULL;
    if (stream->done) {
        Value *result = retain(stream->result);
        http_transport_destroy(stream);
        return result;
    }
    size_t slot = owner->count;
    Value *handle = typed ? value_new(V_HTTP_STREAM) : value_size(slot + 1);
    if (handle && typed) {
        handle->as.http_stream.owner = owner;
        handle->as.http_stream.slot = slot;
        owner->refs++;
    }
    Value *result = handle ? named_value(V_VARIANT, "Ok", NULL, &handle, 1) : NULL;
    release(handle);
    if (!result) { http_transport_destroy(stream); return NULL; }
    owner->slots[slot] = stream;
    owner->typed[slot] = typed;
    owner->count++;
    return result;
}

static bool stream_slot(VM *vm, Value *handle, bool typed, size_t *slot)
{
    VMHttpStreams *owner = vm->http_streams;
    if (!owner || owner->closing) return false;
    if (typed) {
        if (handle->kind != V_HTTP_STREAM || handle->as.http_stream.owner != owner) return false;
        *slot = handle->as.http_stream.slot;
    } else {
        size_t number;
        if (!value_index(handle, &number) || !number) return false;
        *slot = number - 1;
    }
    return *slot < owner->count && owner->typed[*slot] == typed && owner->slots[*slot];
}

static Value *close_stream(VM *vm, Value *handle, bool typed)
{
    size_t slot;
    if (!stream_slot(vm, handle, typed, &slot)) return stream_error("invalid HTTP stream handle");
    if (vm->http_streams->busy[slot]) return stream_error("HTTP stream read is already in progress");
    close_slot(vm->http_streams, slot);
    Value *unit = value_unit();
    Value *result = named_value(V_VARIANT, "Ok", NULL, &unit, 1);
    release(unit);
    return result;
}

static VMHttpStreamRead *start_read(VM *vm, Value *handle, bool typed, Value **error)
{
    *error = NULL;
    size_t slot;
    if (!stream_slot(vm, handle, typed, &slot)) {
        *error = stream_error("invalid HTTP stream handle");
        return NULL;
    }
    VMHttpStreams *owner = vm->http_streams;
    if (owner->busy[slot]) {
        *error = stream_error("HTTP stream read is already in progress");
        return NULL;
    }
    VMHttpStreamRead *read = malloc(sizeof(*read));
    if (!read) return NULL;
    *read = (VMHttpStreamRead){.owner = owner, .slot = slot};
    owner->refs++;
    owner->busy[slot] = true;
    VMHttpTransport *stream = owner->slots[slot];
    if (stream->paused) {
        release(stream->result);
        stream->result = NULL;
        stream->received = 0;
        stream->paused = false;
    }
    return read;
}

Value *http_stream_open_typed(VM *vm, Value **arguments)
{
    return open_stream(vm, arguments, true);
}

Value *http_stream_close_typed(VM *vm, Value *handle)
{
    return close_stream(vm, handle, true);
}

VMHttpStreamRead *http_stream_read_start(VM *vm, Value *handle, Value **error)
{
    return start_read(vm, handle, true, error);
}

bool http_stream_read_poll(VMHttpStreamRead *read, unsigned max_wait_ms)
{
    return read->owner->closing || http_transport_poll(read->owner->slots[read->slot], max_wait_ms);
}

void http_stream_read_cancel(VMHttpStreamRead *read)
{
    if (!read) return;
    close_slot(read->owner, read->slot);
    read->owner->busy[read->slot] = false;
    http_stream_owner_release(read->owner);
    free(read);
}

Value *http_stream_read_finish(VMHttpStreamRead *read)
{
    VMHttpStreams *owner = read->owner;
    VMHttpTransport *stream = owner->slots[read->slot];
    if (owner->closing) {
        Value *result = stream_error("HTTP stream invocation has ended");
        http_stream_read_cancel(read);
        return result;
    }
    if (!stream->done && !stream->paused) return NULL; /* Still owned by caller. */
    Value *result = retain(stream->result);
    if (!result) { http_stream_read_cancel(read); return NULL; }
    owner->busy[read->slot] = false;
    http_stream_owner_release(owner);
    free(read);
    return result;
}

Value *http_stream_typed_call(VM *vm, const char *name, Value **arguments)
{
    if (!strcmp(name, "http_stream_open_typed")) return http_stream_open_typed(vm, arguments);
    if (!strcmp(name, "http_stream_close_typed")) return http_stream_close_typed(vm, arguments[0]);
    if (strcmp(name, "http_stream_read_typed")) return stream_error("invalid HTTP stream operation");
    Value *error = NULL;
    VMHttpStreamRead *read = http_stream_read_start(vm, arguments[0], &error);
    if (!read) return error;
    while (!http_stream_read_poll(read, 20)) { }
    return http_stream_read_finish(read);
}

/* Compatibility pull API. Typed and numeric slots cannot be interchanged. */
Value *http_stream_call(VM *vm, const char *name, Value **arguments)
{
    if (!strcmp(name, "http_stream_open")) return open_stream(vm, arguments, false);
    if (!strcmp(name, "http_stream_close")) return close_stream(vm, arguments[0], false);
    if (strcmp(name, "http_stream_read")) return stream_error("invalid HTTP stream operation");
    Value *error = NULL;
    VMHttpStreamRead *read = start_read(vm, arguments[0], false, &error);
    if (!read) return error;
    while (!http_stream_read_poll(read, 20)) { }
    return http_stream_read_finish(read);
}

void http_stream_destroy(VM *vm)
{
    VMHttpStreams *owner = vm->http_streams;
    if (!owner) return;
    vm->http_streams = NULL;
    owner->closing = true;
    for (size_t i = 0; i < owner->count; i++) {
        /* A pending lease retains its transport until it observes teardown. */
        if (!owner->busy[i]) close_slot(owner, i);
    }
    http_stream_owner_release(owner);
}
