#include "builtins_internal.h"
#include "utf8.h"
#include "value.h"

#include <stdlib.h>
#include <string.h>

/* Text and byte operations preserve code-point semantics and runtime bounds checks. */

Value *builtins_text_call(VM *vm, const char *name, Value **a)
{
    if (!strcmp(name, "len")) {
        REQUIRE(a[0]->kind == V_STR || a[0]->kind == V_BYTES || a[0]->kind == V_ARRAY,
                "VM trap: len requires Str, Bytes, or Array");
        size_t n = a[0]->kind == V_STR     ? a[0]->as.bytes.characters
                   : a[0]->kind == V_BYTES ? a[0]->as.bytes.length
                                           : a[0]->as.sequence.count;
        return value_size(n);
    }

    if (!strcmp(name, "concat")) {
        REQUIRE(a[0]->kind == V_ARRAY && a[1]->kind == V_ARRAY, "VM trap: concat requires arrays");
        size_t n = a[0]->as.sequence.count, m = a[1]->as.sequence.count;
        Value **items = malloc((n + m) * sizeof(Value *));
        if (!items && n + m) {
            return NULL;
        }
        memcpy(items, a[0]->as.sequence.items, n * sizeof(Value *));
        memcpy(items + n, a[1]->as.sequence.items, m * sizeof(Value *));
        Value *v = value_sequence(V_ARRAY, items, n + m);
        free(items);
        return v;
    }

    if (!strcmp(name, "slice")) {
        REQUIRE(a[0]->kind == V_STR && a[1]->kind == V_NAT && a[2]->kind == V_NAT,
                "VM trap: slice requires Str and Nat bounds");
        size_t start, end, bs, be;
        if (!value_index(a[1], &start) || !value_index(a[2], &end) || start > end) {
            vm->error = "VM trap: invalid string slice";
            return NULL;
        }
        size_t count = a[0]->as.bytes.characters;
        if (end > count) {
            vm->error = "VM trap: invalid string slice";
            return NULL;
        }
        if (start == count) {
            bs = a[0]->as.bytes.length;
        } else if (!utf8_offset(a[0], start, &bs, &be)) {
            return NULL;
        }
        if (end == count) {
            be = a[0]->as.bytes.length;
        } else {
            size_t ignored;
            if (!utf8_offset(a[0], end, &be, &ignored)) {
                return NULL;
            }
        }
        return value_data(V_STR, a[0]->as.bytes.data + bs, be - bs);
    }

    if (!strcmp(name, "starts_with") || !strcmp(name, "starts_with_at")) {
        REQUIRE(a[0]->kind == V_STR && a[1]->kind == V_STR, "VM trap: starts_with requires Str");
        if (!strcmp(name, "starts_with_at")) {
            REQUIRE(a[2]->kind == V_NAT, "VM trap: starts_with_at requires Nat offset");
        }
        size_t offset = 0, bs = 0, be;
        if (!strcmp(name, "starts_with_at")) {
            if (!value_index(a[2], &offset)) {
                return value_bool(false);
            }
            if (offset == a[0]->as.bytes.characters) {
                bs = a[0]->as.bytes.length;
            } else if (!utf8_offset(a[0], offset, &bs, &be)) {
                return value_bool(false);
            }
        }
        bool result = a[1]->as.bytes.length <= a[0]->as.bytes.length - bs &&
                      !memcmp(a[0]->as.bytes.data + bs, a[1]->as.bytes.data, a[1]->as.bytes.length);
        return value_bool(result);
    }

    if (!strcmp(name, "reverse")) {
        REQUIRE(a[0]->kind == V_STR, "VM trap: reverse requires Str");
        size_t n = a[0]->as.bytes.length, source = 0, destination = n;
        uint8_t *data = malloc(n ? n : 1);
        if (!data) {
            return NULL;
        }
        while (source < n) {
            uint8_t c = a[0]->as.bytes.data[source];
            size_t width = c < 0x80 ? 1 : (c & 0xe0) == 0xc0 ? 2 : (c & 0xf0) == 0xe0 ? 3 : 4;
            destination -= width;
            memcpy(data + destination, a[0]->as.bytes.data + source, width);
            source += width;
        }
        Value *v = value_data(V_STR, data, n);
        free(data);
        return v;
    }

    if (!strcmp(name, "is_digit") || !strcmp(name, "is_letter") || !strcmp(name, "is_whitespace")) {
        REQUIRE(a[0]->kind == V_STR, "VM trap: character class requires Str");
        uint8_t c = a[0]->as.bytes.length == 1 ? a[0]->as.bytes.data[0] : 0;
        bool result = !strcmp(name, "is_digit") ? c >= '0' && c <= '9'
                      : !strcmp(name, "is_letter")
                          ? (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
                          : c == ' ' || c == '\t' || c == '\r' || c == '\n';
        return value_bool(result);
    }

    if (!strcmp(name, "byte_append")) {
        REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_NAT,
                "VM trap: byte_append requires Bytes and Nat");
        size_t byte;
        if (!value_index(a[1], &byte) || byte > 255) {
            vm->error = "VM trap: byte value is outside 0..255";
            return NULL;
        }
        size_t n = a[0]->as.bytes.length;
        uint8_t *data = malloc(n + 1);
        if (!data) {
            return NULL;
        }
        memcpy(data, a[0]->as.bytes.data, n);
        data[n] = (uint8_t)byte;
        Value *v = value_data(V_BYTES, data, n + 1);
        free(data);
        return v;
    }

    if (!strcmp(name, "bytes_concat")) {
        REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_BYTES,
                "VM trap: bytes_concat requires Bytes");
        size_t n = a[0]->as.bytes.length, m = a[1]->as.bytes.length;
        uint8_t *data = malloc(n + m);
        if (!data && n + m) {
            return NULL;
        }
        memcpy(data, a[0]->as.bytes.data, n);
        memcpy(data + n, a[1]->as.bytes.data, m);
        Value *v = value_data(V_BYTES, data, n + m);
        free(data);
        return v;
    }

    if (!strcmp(name, "byte_len")) {
        REQUIRE(a[0]->kind == V_BYTES, "VM trap: byte_len requires Bytes");
        return value_size(a[0]->as.bytes.length);
    }

    if (!strcmp(name, "byte_get")) {
        REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_NAT,
                "VM trap: byte_get requires Bytes and Nat");
        size_t i;
        if (!value_index(a[1], &i) || i >= a[0]->as.bytes.length) {
            vm->error = "VM trap: byte index is out of bounds";
            return NULL;
        }
        return value_size(a[0]->as.bytes.data[i]);
    }

    if (!strcmp(name, "utf8_encode")) {
        REQUIRE(a[0]->kind == V_STR, "VM trap: utf8_encode requires Str");
        return value_data(V_BYTES, a[0]->as.bytes.data, a[0]->as.bytes.length);
    }

    if (!strcmp(name, "utf8_decode")) {
        REQUIRE(a[0]->kind == V_BYTES, "VM trap: utf8_decode requires Bytes");
        if (!utf8(a[0]->as.bytes.data, a[0]->as.bytes.length)) {
            vm->error = "VM trap: invalid UTF-8";
            return NULL;
        }
        return value_data(V_STR, a[0]->as.bytes.data, a[0]->as.bytes.length);
    }

    if (!strcmp(name, "nat_from_str")) {
        REQUIRE(a[0]->kind == V_STR, "VM trap: nat_from_str requires Str");
        if (!a[0]->as.bytes.length) {
            vm->error = "VM trap: text is not a Nat";
            return NULL;
        }
        for (size_t i = 0; i < a[0]->as.bytes.length; i++) {
            if (a[0]->as.bytes.data[i] < '0' || a[0]->as.bytes.data[i] > '9') {
                vm->error = "VM trap: text is not a Nat";
                return NULL;
            }
        }
        PnBigInt n;
        if (!pn_big_from_digits(&n, (char *)a[0]->as.bytes.data, a[0]->as.bytes.length, 1)) {
            return NULL;
        }
        Value *v = value_big(V_NAT, &n);
        pn_big_free(&n);
        return v;
    }
    vm->error = "VM trap: unknown builtin";
    return NULL;
}
