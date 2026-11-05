#include "gzip_stream.h"
#include "host_types.h"
#include <stdlib.h>
#include <string.h>
#ifndef __EMSCRIPTEN__
#include <zlib.h>
#define GZIP_HANDLES 64
#define GZIP_CHUNK 65536
struct Decoder { z_stream stream; size_t remaining; bool live, boundary, finished, failed; };
struct VMGzipStreams { struct Decoder slots[GZIP_HANDLES]; size_t count; };
#endif
static Value *gzip_error(const char *name, const char *code)
{
    char *fields[] = {"operation", "code", "native_code"};
    Value *values[] = {value_data(V_STR, (const uint8_t *)name, strlen(name)),
        value_data(V_STR, (const uint8_t *)code, strlen(code)), value_size(0)};
    Value *record = values[0] && values[1] && values[2] ? named_value(V_RECORD, "HostError", fields, values, 3) : NULL;
    for (size_t i = 0; i < 3; i++) release(values[i]);
    return host_variant("Error", record);
}
void gzip_stream_destroy(VM *vm)
{
#ifndef __EMSCRIPTEN__
    if (!vm->gzip_streams) return;
    for (size_t i = 0; i < vm->gzip_streams->count; i++) {
        if (vm->gzip_streams->slots[i].live) inflateEnd(&vm->gzip_streams->slots[i].stream);
    }
    free(vm->gzip_streams); vm->gzip_streams = NULL;
#else
    (void)vm;
#endif
}
Value *gzip_stream_call(VM *vm, const char *name, Value **a)
{
#ifdef __EMSCRIPTEN__
    (void)vm; (void)a;
    return gzip_error(name, "unsupported");
#else
    size_t number;
    if (a[0]->kind != V_NAT || !pn_big_fits_size(&a[0]->as.integer, &number)) return gzip_error(name, "invalid_argument");
    if (!strcmp(name, "gzip_open")) {
        if (number > 256u * 1024u * 1024u) return gzip_error(name, "output_limit");
        if (!vm->gzip_streams) vm->gzip_streams = calloc(1, sizeof(*vm->gzip_streams));
        if (!vm->gzip_streams || vm->gzip_streams->count == GZIP_HANDLES) return gzip_error(name, "resource_limit");
        size_t slot = vm->gzip_streams->count++;
        struct Decoder *decoder = &vm->gzip_streams->slots[slot];
        if (inflateInit2(&decoder->stream, 15 + 16) != Z_OK) return gzip_error(name, "decoder_unavailable");
        decoder->remaining = number; decoder->live = true;
        return host_variant("Ok", value_size(slot + 1));
    }
    if (!number || !vm->gzip_streams || number > vm->gzip_streams->count || !vm->gzip_streams->slots[number - 1].live)
        return gzip_error(name, "invalid_handle");
    struct Decoder *decoder = &vm->gzip_streams->slots[number - 1];
    if (!strcmp(name, "gzip_close")) {
        inflateEnd(&decoder->stream); decoder->live = false;
        return host_variant("Ok", value_new(V_UNIT));
    }
    if (strcmp(name, "gzip_step") || a[1]->kind != V_BYTES || a[1]->as.bytes.length > GZIP_CHUNK || a[2]->kind != V_BOOL)
        return gzip_error(name, "invalid_argument");
    if (decoder->failed || decoder->finished) return gzip_error(name, "decoder_finished");
    size_t length = a[1]->as.bytes.length;
    if (decoder->boundary && length) {
        if (inflateReset2(&decoder->stream, 15 + 16) != Z_OK) return gzip_error(name, "decoder_unavailable");
        decoder->boundary = false;
    }
    uint8_t output[GZIP_CHUNK];
    size_t consumed = 0, produced = 0;
    if (!decoder->boundary) {
        decoder->stream.next_in = a[1]->as.bytes.data;
        decoder->stream.avail_in = (uInt)length;
        decoder->stream.next_out = output;
        decoder->stream.avail_out = GZIP_CHUNK;
        int status = inflate(&decoder->stream, Z_NO_FLUSH);
        consumed = length - decoder->stream.avail_in;
        produced = GZIP_CHUNK - decoder->stream.avail_out;
        /* Never retain borrowed Bytes pointers after returning to the VM. */
        decoder->stream.next_in = NULL; decoder->stream.avail_in = 0;
        decoder->stream.next_out = NULL; decoder->stream.avail_out = 0;
        if (status != Z_OK && status != Z_STREAM_END && status != Z_BUF_ERROR) {
            decoder->failed = true; return gzip_error(name, "invalid_gzip");
        }
        if (produced > decoder->remaining) { decoder->failed = true; return gzip_error(name, "output_limit"); }
        decoder->remaining -= produced;
        decoder->boundary = status == Z_STREAM_END;
        if (a[2]->as.boolean && consumed == length && !decoder->boundary && produced < GZIP_CHUNK) {
            decoder->failed = true; return gzip_error(name, "truncated_gzip");
        }
    }
    decoder->finished = a[2]->as.boolean && consumed == length && decoder->boundary;
    char *fields[] = {"consumed", "data", "finished"};
    Value *values[] = {value_size(consumed), value_data(V_BYTES, output, produced), value_bool(decoder->finished)};
    Value *record = values[0] && values[1] && values[2] ? named_value(V_RECORD, "GzipChunk", fields, values, 3) : NULL;
    for (size_t i = 0; i < 3; i++) release(values[i]);
    return host_variant("Ok", record);
#endif
}
