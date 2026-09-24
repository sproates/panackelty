#include "decode.h"

#include "program.h"
#include "utf8.h"

#include <stdlib.h>
#include <string.h>

/* Bounded version-8 decoding. No runtime values or host operations are created here. */

#define MAX_FUNCTIONS 4096u
#define MAX_INSTRUCTIONS 4000000u
#define MAX_NAME 1024u
#define MAX_TEXT (1024u * 1024u)
#define MAX_DIGITS 4096u

typedef struct {
    const uint8_t *data;
    size_t len, at;
    const char *error;
} Reader;

static void fail(Reader *r, const char *message)
{
    if (!r->error) {
        r->error = message;
    }
}

static bool take(Reader *r, size_t n, const uint8_t **out)
{
    if (n > r->len - r->at) {
        fail(r, "truncated data");
        return false;
    }
    *out = r->data + r->at;
    r->at += n;
    return true;
}

static bool u8(Reader *r, uint8_t *out)
{
    const uint8_t *p;
    if (!take(r, 1, &p)) {
        return false;
    }
    *out = p[0];
    return true;
}

static bool u16(Reader *r, uint16_t *out)
{
    const uint8_t *p;
    if (!take(r, 2, &p)) {
        return false;
    }
    *out = (uint16_t)((p[0] << 8) | p[1]);
    return true;
}

static bool u32(Reader *r, uint32_t *out)
{
    const uint8_t *p;
    if (!take(r, 4, &p)) {
        return false;
    }
    *out = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
    return true;
}

static char *text(Reader *r, bool name)
{
    uint32_t n32 = 0;
    uint16_t n16 = 0;
    size_t n;
    if (name) {
        if (!u16(r, &n16)) {
            return NULL;
        }
        n = n16;
        if (n > MAX_NAME) {
            fail(r, "name exceeds limit");
            return NULL;
        }
    } else {
        if (!u32(r, &n32)) {
            return NULL;
        }
        n = n32;
        if (n > MAX_TEXT) {
            fail(r, "text exceeds limit");
            return NULL;
        }
    }
    const uint8_t *p;
    if (!take(r, n, &p)) {
        return NULL;
    }
    if (!utf8(p, n)) {
        fail(r, "invalid UTF-8");
        return NULL;
    }
    char *v = malloc(n + 1);
    if (!v) {
        fail(r, "out of memory");
        return NULL;
    }
    memcpy(v, p, n);
    v[n] = 0;
    return v;
}

static bool read_nat(Reader *r, PnBigInt *out)
{
    uint16_t n;
    const uint8_t *p;
    pn_big_init(out);
    if (!u16(r, &n)) {
        return false;
    }
    size_t max_bytes = (MAX_DIGITS * 3322u + 7999u) / 8000u;
    if (n > max_bytes) {
        fail(r, "integer exceeds digit limit");
        return false;
    }
    if (!take(r, n, &p)) {
        return false;
    }
    if (n && p[0] == 0) {
        fail(r, "non-minimal integer");
        return false;
    }
    if (!pn_big_from_bytes(out, p, n)) {
        fail(r, "out of memory");
        return false;
    }
    char *digits = pn_big_string(out);
    if (!digits) {
        fail(r, "out of memory");
        return false;
    }
    if (strlen(digits) > MAX_DIGITS) {
        fail(r, "integer exceeds digit limit");
    }
    free(digits);
    return !r->error;
}

static bool read_decimal(Reader *r, Constant *out)
{
    uint8_t sign;
    uint16_t ex, digits;
    const uint8_t *p;
    if (!u8(r, &sign) || !u16(r, &ex) || !u16(r, &digits)) {
        return false;
    }
    out->boolean = sign != 0;
    out->exponent = (int16_t)ex;
    if (sign > 1 || !digits || out->exponent > 4096 || out->exponent < -4096 ||
        digits > MAX_DIGITS) {
        fail(r, "invalid decimal");
        return false;
    }
    size_t bytes = (digits + 1) / 2;
    if (!take(r, bytes, &p)) {
        return false;
    }
    char *value = malloc(digits);
    if (!value) {
        fail(r, "out of memory");
        return false;
    }
    for (size_t i = 0; i < digits; i++) {
        uint8_t d = (i & 1) ? p[i / 2] & 15 : p[i / 2] >> 4;
        if (d > 9) {
            free(value);
            fail(r, "invalid decimal digit");
            return false;
        }
        value[i] = (char)('0' + d);
    }
    if ((digits & 1) && (p[bytes - 1] & 15) != 15) {
        free(value);
        fail(r, "invalid decimal padding");
        return false;
    }
    if (digits > 1 && value[0] == '0') {
        free(value);
        fail(r, "non-minimal decimal coefficient");
        return false;
    }
    bool ok = pn_big_from_digits(&out->number, value, digits, sign ? -1 : 1);
    free(value);
    if (!ok) {
        fail(r, "out of memory");
    }
    return ok;
}

static bool read_constant(Reader *r, Constant *out)
{
    uint8_t sign;
    memset(out, 0, sizeof(*out));
    pn_big_init(&out->number);
    if (!u8(r, &out->tag)) {
        return false;
    }
    switch (out->tag) {
    case 0:
        return read_nat(r, &out->number);
    case 1:
        if (!u8(r, &sign) || sign > 1) {
            fail(r, "invalid integer sign");
            return false;
        }
        if (!read_nat(r, &out->number)) {
            return false;
        }
        if (sign && pn_big_is_zero(&out->number)) {
            fail(r, "negative zero integer");
            return false;
        }
        if (sign) {
            out->number.sign = -1;
        }
        return true;
    case 2:
        return read_decimal(r, out);
    case 3: {
        size_t start = r->at;
        out->text = text(r, false);
        if (out->text) {
            out->text_length = r->at - start - 4;
        }
        return out->text != NULL;
    }
    case 4:
        if (!u8(r, &sign) || sign > 1) {
            fail(r, "invalid Bool constant");
            return false;
        }
        out->boolean = sign != 0;
        return true;
    case 5:
        return true;
    default:
        fail(r, "unknown constant tag");
        return false;
    }
}

static char **strings(Reader *r, uint16_t count, bool names)
{
    char **items = calloc(count, sizeof(char *));
    if (count && !items) {
        fail(r, "out of memory");
        return NULL;
    }
    for (uint16_t i = 0; i < count && !r->error; i++) {
        items[i] = text(r, names);
    }
    if (r->error) {
        for (uint16_t i = 0; i < count; i++) {
            free(items[i]);
        }
        free(items);
        return NULL;
    }
    return items;
}

static bool instruction(Reader *r, Instruction *in)
{
    uint16_t count;
    uint32_t ignored;
    memset(in, 0, sizeof(*in));
    if (!u8(r, &in->op)) {
        return false;
    }
    switch (in->op) {
    case OP_CONST:
        return read_constant(r, &in->constant);
    case OP_LOAD:
    case OP_STORE:
    case OP_ITER_INIT:
    case OP_FIELD_GET:
        in->name = text(r, true);
        return !r->error;
    case OP_POP:
    case OP_MAKE_RANGE:
    case OP_INDEX_GET:
    case OP_MATCH_FAIL:
    case OP_RETURN:
        return true;
    case OP_CALL_VALUE:
        return u8(r, &in->arity);
    case OP_UNARY:
        if (!u8(r, &in->code) || in->code > 1) {
            fail(r, "unknown unary operator");
            return false;
        }
        return true;
    case OP_BINARY:
        if (!u8(r, &in->code) || in->code > 12) {
            fail(r, "unknown binary operator");
            return false;
        }
        return true;
    case OP_MAKE_ARRAY:
        if (!u16(r, &count)) {
            return false;
        }
        in->count = count;
        return true;
    case OP_INTERPOLATE:
        if (!u16(r, &count)) {
            return false;
        }
        in->count = count;
        in->items = strings(r, count, false);
        return !r->error;
    case OP_ITER_NEXT:
        in->name = text(r, true);
        in->name2 = text(r, true);
        return in->name && in->name2 && u32(r, &in->target);
    case OP_MAKE_RECORD:
        in->name = text(r, true);
        if (!in->name || !u16(r, &count)) {
            return false;
        }
        in->count = count;
        in->items = strings(r, count, true);
        return !r->error;
    case OP_MAKE_VARIANT:
        in->name = text(r, true);
        in->name2 = text(r, true);
        if (!in->name || !in->name2 || !u16(r, &count)) {
            return false;
        }
        in->count = count;
        return true;
    case OP_MATCH_VARIANT:
        in->name = text(r, true);
        return in->name && u32(r, &in->target);
    case OP_CALL:
        in->name = text(r, true);
        return in->name && u8(r, &in->arity);
    case OP_JUMP_FALSE:
    case OP_JUMP:
        return u32(r, &in->target);
    default:
        (void)ignored;
        fail(r, "unknown bytecode opcode");
        return false;
    }
}

bool decode(const uint8_t *data, size_t length, Program *p, const char **error)
{
    memset(p, 0, sizeof(*p));
    Reader r = {data, length, 0, NULL};
    const uint8_t *magic;
    uint16_t version, count;
    if (length > MAX_ARTIFACT) {
        *error = "artifact exceeds size limit";
        return false;
    }
    if (!take(&r, 9, &magic) || memcmp(magic, "PANACKBC\0", 9)) {
        *error = "not a Panackelty bytecode file";
        return false;
    }
    if (!u16(&r, &version)) {
        *error = r.error;
        return false;
    }
    if (version != 8) {
        *error = "unsupported bytecode version";
        return false;
    }
    if (!u16(&r, &count)) {
        *error = r.error;
        return false;
    }
    if (count > MAX_FUNCTIONS) {
        *error = "function count exceeds limit";
        return false;
    }
    p->count = count;
    p->functions = calloc(count, sizeof(Function));
    if (count && !p->functions) {
        *error = "out of memory";
        return false;
    }
    size_t total = 0;
    for (size_t i = 0; i < count && !r.error; i++) {
        Function *f = &p->functions[i];
        uint8_t flags, params;
        uint32_t ins;
        f->name = text(&r, true);
        if (!f->name || !u8(&r, &flags) || flags > 1 || !u8(&r, &params)) {
            if (!r.error) {
                fail(&r, "invalid function flags");
            }
            break;
        }
        f->pure = flags == 1;
        f->param_count = params;
        f->params = calloc(params, sizeof(char *));
        if (params && !f->params) {
            fail(&r, "out of memory");
            break;
        }
        for (size_t j = 0; j < params && !r.error; j++) {
            f->params[j] = text(&r, true);
        }
        if (!u32(&r, &ins)) {
            break;
        }
        if (ins > 1000000u || total + ins > MAX_INSTRUCTIONS) {
            fail(&r, "instruction count exceeds limit");
            break;
        }
        total += ins;
        f->ins_count = ins;
        f->ins = calloc(ins, sizeof(Instruction));
        if (ins && !f->ins) {
            fail(&r, "out of memory");
            break;
        }
        for (size_t j = 0; j < ins && !r.error; j++) {
            instruction(&r, &f->ins[j]);
        }
    }
    if (!r.error && r.at != r.len) {
        fail(&r, "trailing data");
    }
    if (r.error) {
        *error = r.error;
        free_program(p);
        return false;
    }
    return true;
}
