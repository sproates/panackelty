#include "render.h"

#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Language value rendering, including exact decimal scale and escaped display forms. */

static bool render_decimal(Buffer *b, const Decimal *d);

static bool render_decimal(Buffer *b, const Decimal *d)
{
    char *raw = pn_big_string(&d->coefficient);
    if (!raw) {
        return false;
    }
    char *digits = raw;
    bool negative = raw[0] == '-' || d->negative_zero;
    if (raw[0] == '-') {
        digits++;
    }
    size_t n = strlen(digits);
    bool ok = true;
    if (negative) {
        ok = buffer_text(b, "-");
    }
    if (ok && d->exponent >= 0) {
        ok = buffer_text(b, digits);
        for (int i = 0; ok && i < d->exponent; i++) {
            ok = buffer_text(b, "0");
        }
    } else if (ok) {
        size_t places = (size_t)(-d->exponent);
        if (places < n) {
            ok = buffer_add(b, digits, n - places) && buffer_text(b, ".") &&
                 buffer_add(b, digits + n - places, places);
        } else {
            ok = buffer_text(b, "0.");
            for (size_t i = n; ok && i < places; i++) {
                ok = buffer_text(b, "0");
            }
            if (ok) {
                ok = buffer_text(b, digits);
            }
        }
    }
    free(raw);
    return ok;
}

bool render(Buffer *b, const Value *v, bool display)
{
    char *number = NULL;
    switch (v->kind) {
    case V_PATH:
        return buffer_text(b, "<Path>");
    case V_INSTANT:
        return buffer_text(b, "<Instant>");
    case V_DURATION: {
        char *text = pn_big_string(&v->as.integer);
        if (!text) {
            return false;
        }
        bool ok = buffer_text(b, text) && buffer_text(b, "ns");
        free(text);
        return ok;
    }

    case V_NAT:
    case V_INT:
        number = pn_big_string(&v->as.integer);
        if (!number) {
            return false;
        }
        {
            bool ok = buffer_text(b, number);
            free(number);
            return ok;
        }
    case V_UNIT:
        return buffer_text(b, "()");
    case V_RAT: {
        char *n = pn_big_string(&v->as.rational.numerator);
        char *d = pn_big_string(&v->as.rational.denominator);
        bool ok = n && d && buffer_text(b, n) && buffer_text(b, "/") && buffer_text(b, d);
        free(n);
        free(d);
        return ok;
    }
    case V_DEC:
        return render_decimal(b, &v->as.decimal);
    case V_STR:
        if (display && !buffer_text(b, "\"")) {
            return false;
        }
        if (!buffer_add(b, (char *)v->as.bytes.data, v->as.bytes.length)) {
            return false;
        }
        return !display || buffer_text(b, "\"");
    case V_BOOL:
        return buffer_text(b, v->as.boolean ? "true" : "false");
    case V_VOID:
        return buffer_text(b, "void");
    case V_BYTES: {
        if (!buffer_text(b, "bytes(")) {
            return false;
        }
        char hex[3];
        for (size_t i = 0; i < v->as.bytes.length; i++) {
            snprintf(hex, sizeof(hex), "%02x", v->as.bytes.data[i]);
            if (!buffer_text(b, hex)) {
                return false;
            }
        }
        return buffer_text(b, ")");
    }
    case V_ARRAY:
    case V_SET: {
        if (!buffer_text(b, v->kind == V_SET ? "set{" : "[")) {
            return false;
        }
        for (size_t i = 0; i < v->as.sequence.count; i++) {
            if (i && !buffer_text(b, ", ")) {
                return false;
            }
            if (!render(b, v->as.sequence.items[i], true)) {
                return false;
            }
        }
        return buffer_text(b, v->kind == V_SET ? "}" : "]");
    }
    case V_MAP:
        if (!buffer_text(b, "{")) {
            return false;
        }
        for (size_t i = 0; i < v->as.sequence.count; i += 2) {
            if (i && !buffer_text(b, ", ")) {
                return false;
            }
            if (!render(b, v->as.sequence.items[i], true) || !buffer_text(b, ": ") ||
                !render(b, v->as.sequence.items[i + 1], true)) {
                return false;
            }
        }
        return buffer_text(b, "}");
    case V_VARIANT:
        if (!buffer_text(b, v->as.named.name) || !buffer_text(b, "(")) {
            return false;
        }
        for (size_t i = 0; i < v->as.named.count; i++) {
            if (i && !buffer_text(b, ", ")) {
                return false;
            }
            if (!render(b, v->as.named.values[i], true)) {
                return false;
            }
        }
        return buffer_text(b, ")");
    case V_RECORD:
        if (!buffer_text(b, v->as.named.name) || !buffer_text(b, "(")) {
            return false;
        }
        for (size_t i = 0; i < v->as.named.count; i++) {
            if (i && !buffer_text(b, ", ")) {
                return false;
            }
            if (!buffer_text(b, v->as.named.names[i]) || !buffer_text(b, ": ") ||
                !render(b, v->as.named.values[i], true)) {
                return false;
            }
        }
        return buffer_text(b, ")");
    default:
        return buffer_text(b, "<value>");
    }
}
