#include "gzip_stream.h"
#include "host_types.h"
#include <stdlib.h>
#include <string.h>
#ifndef __EMSCRIPTEN__
#include <zlib.h>
#define GZIP_HANDLES 64
#define GZIP_CHUNK 65536
struct Decoder { z_stream stream; size_t remaining; bool live, boundary, finished, failed, typed; };
struct VMGzipStreams { struct Decoder slots[GZIP_HANDLES]; size_t count, refs; };
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
void gzip_owner_release(VMGzipStreams *owner)
{
#ifndef __EMSCRIPTEN__
    if (owner && !--owner->refs) free(owner);
#else
    (void)owner;
#endif
}
void gzip_stream_destroy(VM *vm)
{
#ifndef __EMSCRIPTEN__
    if (!vm->gzip_streams) return;
    for (size_t i = 0; i < vm->gzip_streams->count; i++) {
        if (vm->gzip_streams->slots[i].live) inflateEnd(&vm->gzip_streams->slots[i].stream);
        vm->gzip_streams->slots[i].live = false;
    }
    gzip_owner_release(vm->gzip_streams); vm->gzip_streams = NULL;
#else
    (void)vm;
#endif
}
static Value *gzip_operation(VM *vm, const char *name, Value **a, bool typed, unsigned op)
{
#ifdef __EMSCRIPTEN__
    (void)vm; (void)a; (void)typed; (void)op;
    return gzip_error(name, "unsupported");
#else
    size_t number;
    if (op == 0) {
        if (!value_index(a[0], &number)) return gzip_error(name, "invalid_argument");
        if (number > 256u * 1024u * 1024u) return gzip_error(name, "output_limit");
        if (!vm->gzip_streams) {
            vm->gzip_streams = calloc(1, sizeof(*vm->gzip_streams));
            if (vm->gzip_streams) vm->gzip_streams->refs = 1;
        }
        if (!vm->gzip_streams || vm->gzip_streams->count == GZIP_HANDLES) return gzip_error(name, "resource_limit");
        size_t slot = vm->gzip_streams->count;
        struct Decoder *decoder = &vm->gzip_streams->slots[slot];
        if (inflateInit2(&decoder->stream, 15 + 16) != Z_OK) return gzip_error(name, "decoder_unavailable");
        decoder->remaining = number; decoder->live = true;
        decoder->typed = typed;
        Value *handle = typed ? value_new(V_GZIP_DECODER) : value_size(slot + 1);
        if (handle && typed) {
            handle->as.gzip.owner = vm->gzip_streams;
            handle->as.gzip.slot = slot;
            vm->gzip_streams->refs++;
        }
        Value *result = host_variant("Ok", handle);
        if (!result) {
            inflateEnd(&decoder->stream);
            memset(decoder, 0, sizeof(*decoder));
            return NULL;
        }
        vm->gzip_streams->count++;
        return result;
    }
    if (typed) {
        if (a[0]->kind != V_GZIP_DECODER || a[0]->as.gzip.owner != vm->gzip_streams)
            return gzip_error(name, "invalid_handle");
        number = a[0]->as.gzip.slot + 1;
    } else if (!value_index(a[0], &number)) return gzip_error(name, "invalid_argument");
    if (!number || !vm->gzip_streams || number > vm->gzip_streams->count || !vm->gzip_streams->slots[number - 1].live || vm->gzip_streams->slots[number - 1].typed != typed)
        return gzip_error(name, "invalid_handle");
    struct Decoder *decoder = &vm->gzip_streams->slots[number - 1];
    if (op == 2) {
        inflateEnd(&decoder->stream); decoder->live = false;
        return host_variant("Ok", value_unit());
    }
    if (op != 1 || a[1]->kind != V_BYTES || a[1]->as.bytes.length > GZIP_CHUNK || a[2]->kind != V_BOOL)
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
    Value *result = host_variant("Ok", record);
    // Lost output must never be mistaken for a successfully acknowledged step.
    if (!result) { inflateEnd(&decoder->stream); decoder->live = false; }
    return result;
#endif
}

Value *gzip_stream_call(VM *vm, const char *name, Value **args)
{
    unsigned op;
    if (!strcmp(name, "gzip_open")) op = 0;
    else if (!strcmp(name, "gzip_step")) op = 1;
    else if (!strcmp(name, "gzip_close")) op = 2;
    else return gzip_error(name, "invalid_argument");
    return gzip_operation(vm, name, args, false, op);
}

Value *gzip_resource_call(VM *vm, const char *name, Value **args)
{
    unsigned op;
    if (!strcmp(name, "gzip_decoder_create") || !strcmp(name, "gzip_decoder_create_async")) op = 0;
    else if (!strcmp(name, "gzip_decoder_decode") || !strcmp(name, "gzip_decoder_decode_async")) op = 1;
    else if (!strcmp(name, "gzip_decoder_close") || !strcmp(name, "gzip_decoder_close_async")) op = 2;
    else return gzip_error(name, "invalid_argument");
    return gzip_operation(vm, name, args, true, op);
}
