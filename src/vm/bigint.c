#include "bigint.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE UINT32_C(1000000000)

static void normalize(PnBigInt *value) {
    while (value->len && value->limbs[value->len - 1] == 0) value->len--;
    if (!value->len) value->sign = 0;
}

static bool resize(PnBigInt *value, size_t length) {
    if (length > SIZE_MAX / sizeof(uint32_t)) return false;
    uint32_t *next = length ? realloc(value->limbs, length * sizeof(uint32_t)) : NULL;
    if (length && !next) return false;
    if (length > value->len) memset(next + value->len, 0, (length - value->len) * sizeof(uint32_t));
    value->limbs = next;
    value->len = length;
    return true;
}

void pn_big_init(PnBigInt *value) { value->sign = 0; value->len = 0; value->limbs = NULL; }
void pn_big_free(PnBigInt *value) { free(value->limbs); pn_big_init(value); }

bool pn_big_copy(PnBigInt *out, const PnBigInt *value) {
    pn_big_init(out);
    if (!resize(out, value->len)) return false;
    memcpy(out->limbs, value->limbs, value->len * sizeof(uint32_t));
    out->sign = value->sign;
    return true;
}

bool pn_big_from_u64(PnBigInt *out, uint64_t value) {
    pn_big_init(out);
    if (!value) return true;
    if (!resize(out, value >= BASE ? 2 : 1)) return false;
    out->limbs[0] = (uint32_t)(value % BASE);
    if (out->len == 2) out->limbs[1] = (uint32_t)(value / BASE);
    out->sign = 1;
    return true;
}

bool pn_big_mul_small(PnBigInt *value, uint32_t factor) {
    if (!value->sign || factor == 1) return true;
    if (!factor) { value->len = 0; value->sign = 0; return true; }
    uint64_t carry = 0;
    for (size_t i = 0; i < value->len; i++) {
        uint64_t product = (uint64_t)value->limbs[i] * factor + carry;
        value->limbs[i] = (uint32_t)(product % BASE);
        carry = product / BASE;
    }
    if (carry) {
        size_t old = value->len;
        if (!resize(value, old + 1)) return false;
        value->limbs[old] = (uint32_t)carry;
    }
    return true;
}

bool pn_big_add_small(PnBigInt *value, uint32_t addend) {
    if (!addend) return true;
    if (!value->sign) { pn_big_free(value); return pn_big_from_u64(value, addend); }
    uint64_t carry = addend;
    size_t i = 0;
    while (carry && i < value->len) {
        uint64_t sum = value->limbs[i] + carry;
        value->limbs[i++] = (uint32_t)(sum % BASE);
        carry = sum / BASE;
    }
    if (carry) {
        size_t old = value->len;
        if (!resize(value, old + 1)) return false;
        value->limbs[old] = (uint32_t)carry;
    }
    return true;
}

bool pn_big_from_bytes(PnBigInt *out, const uint8_t *bytes, size_t length) {
    pn_big_init(out);
    for (size_t i = 0; i < length; i++) {
        if (!pn_big_mul_small(out, 256) || !pn_big_add_small(out, bytes[i])) { pn_big_free(out); return false; }
    }
    return true;
}

bool pn_big_from_digits(PnBigInt *out, const char *digits, size_t length, int sign) {
    pn_big_init(out);
    for (size_t i = 0; i < length; i++) {
        if (digits[i] < '0' || digits[i] > '9' || !pn_big_mul_small(out, 10) ||
            !pn_big_add_small(out, (uint32_t)(digits[i] - '0'))) { pn_big_free(out); return false; }
    }
    if (out->len) out->sign = sign < 0 ? -1 : 1;
    return true;
}

char *pn_big_string(const PnBigInt *value) {
    if (!value->sign) { char *z = malloc(2); if (z) memcpy(z, "0", 2); return z; }
    size_t cap = value->len * 9 + 2;
    char *text = malloc(cap);
    if (!text) return NULL;
    size_t at = 0;
    if (value->sign < 0) text[at++] = '-';
    at += (size_t)snprintf(text + at, cap - at, "%u", value->limbs[value->len - 1]);
    for (size_t i = value->len - 1; i-- > 0;) at += (size_t)snprintf(text + at, cap - at, "%09u", value->limbs[i]);
    return text;
}

static int abs_compare(const PnBigInt *a, const PnBigInt *b) {
    if (a->len != b->len) return a->len < b->len ? -1 : 1;
    for (size_t i = a->len; i-- > 0;) if (a->limbs[i] != b->limbs[i]) return a->limbs[i] < b->limbs[i] ? -1 : 1;
    return 0;
}

int pn_big_compare(const PnBigInt *a, const PnBigInt *b) {
    if (a->sign != b->sign) return a->sign < b->sign ? -1 : 1;
    int result = abs_compare(a, b);
    return a->sign < 0 ? -result : result;
}

static bool abs_add(PnBigInt *out, const PnBigInt *a, const PnBigInt *b) {
    size_t count = (a->len > b->len ? a->len : b->len) + 1;
    pn_big_init(out); if (!resize(out, count)) return false;
    uint64_t carry = 0;
    for (size_t i = 0; i + 1 < count; i++) {
        uint64_t sum = carry + (i < a->len ? a->limbs[i] : 0) + (i < b->len ? b->limbs[i] : 0);
        out->limbs[i] = (uint32_t)(sum % BASE); carry = sum / BASE;
    }
    out->limbs[count - 1] = (uint32_t)carry; normalize(out); return true;
}

static bool abs_sub(PnBigInt *out, const PnBigInt *a, const PnBigInt *b) {
    pn_big_init(out); if (!resize(out, a->len)) return false;
    int64_t borrow = 0;
    for (size_t i = 0; i < a->len; i++) {
        int64_t diff = (int64_t)a->limbs[i] - (i < b->len ? b->limbs[i] : 0) - borrow;
        if (diff < 0) { diff += BASE; borrow = 1; } else borrow = 0;
        out->limbs[i] = (uint32_t)diff;
    }
    normalize(out); return true;
}

bool pn_big_add(PnBigInt *out, const PnBigInt *a, const PnBigInt *b) {
    if (!a->sign) return pn_big_copy(out, b);
    if (!b->sign) return pn_big_copy(out, a);
    bool ok;
    if (a->sign == b->sign) { ok = abs_add(out, a, b); if (ok) out->sign = a->sign; return ok; }
    int cmp = abs_compare(a, b);
    if (!cmp) { pn_big_init(out); return true; }
    ok = cmp > 0 ? abs_sub(out, a, b) : abs_sub(out, b, a);
    if (ok) out->sign = cmp > 0 ? a->sign : b->sign;
    return ok;
}

bool pn_big_sub(PnBigInt *out, const PnBigInt *a, const PnBigInt *b) {
    PnBigInt neg = *b; neg.sign = -neg.sign; return pn_big_add(out, a, &neg);
}

bool pn_big_mul(PnBigInt *out, const PnBigInt *a, const PnBigInt *b) {
    pn_big_init(out); if (!a->sign || !b->sign) return true;
    if (a->len > SIZE_MAX - b->len || !resize(out, a->len + b->len)) return false;
    for (size_t i = 0; i < a->len; i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->len; j++) {
            uint64_t current = out->limbs[i + j] + (uint64_t)a->limbs[i] * b->limbs[j] + carry;
            out->limbs[i + j] = (uint32_t)(current % BASE); carry = current / BASE;
        }
        size_t k = i + b->len;
        while (carry) { uint64_t current = out->limbs[k] + carry; out->limbs[k++] = (uint32_t)(current % BASE); carry = current / BASE; }
    }
    out->sign = a->sign * b->sign; normalize(out); return true;
}

uint32_t pn_big_div_small(PnBigInt *value, uint32_t divisor) {
    uint64_t remainder = 0;
    for (size_t i = value->len; i-- > 0;) {
        uint64_t current = remainder * BASE + value->limbs[i];
        value->limbs[i] = (uint32_t)(current / divisor); remainder = current % divisor;
    }
    normalize(value); return (uint32_t)remainder;
}

static bool shift_base_add(PnBigInt *value, uint32_t limb) {
    if (!value->len && !limb) return true;
    size_t old = value->len; if (!resize(value, old + 1)) return false;
    memmove(value->limbs + 1, value->limbs, old * sizeof(uint32_t)); value->limbs[0] = limb; value->sign = 1; return true;
}

bool pn_big_divmod(PnBigInt *q, PnBigInt *r, const PnBigInt *a, const PnBigInt *b) {
    pn_big_init(q); pn_big_init(r); if (!b->sign) return false;
    PnBigInt aa, bb; if (!pn_big_copy(&aa, a) || !pn_big_copy(&bb, b)) return false;
    aa.sign = aa.len ? 1 : 0; bb.sign = bb.len ? 1 : 0;
    if (abs_compare(&aa, &bb) < 0) { pn_big_free(&aa); pn_big_free(&bb); return pn_big_copy(r, a); }
    if (!resize(q, aa.len)) { pn_big_free(&aa); pn_big_free(&bb); return false; }
    for (size_t pos = aa.len; pos-- > 0;) {
        if (!shift_base_add(r, aa.limbs[pos])) goto fail;
        uint32_t lo = 0, hi = BASE - 1, best = 0;
        while (lo <= hi) {
            uint32_t mid = lo + (hi - lo) / 2; PnBigInt probe;
            if (!pn_big_copy(&probe, &bb) || !pn_big_mul_small(&probe, mid)) goto fail;
            int cmp = abs_compare(&probe, r); pn_big_free(&probe);
            if (cmp <= 0) { best = mid; if (mid == BASE - 1) break; lo = mid + 1; }
            else { if (!mid) break; hi = mid - 1; }
        }
        q->limbs[pos] = best;
        if (best) { PnBigInt product, next; if (!pn_big_copy(&product, &bb) || !pn_big_mul_small(&product, best) || !abs_sub(&next, r, &product)) goto fail; pn_big_free(&product); pn_big_free(r); *r = next; }
    }
    q->sign = q->len ? a->sign * b->sign : 0; normalize(q); r->sign = r->len ? a->sign : 0;
    pn_big_free(&aa); pn_big_free(&bb); return true;
fail: pn_big_free(&aa); pn_big_free(&bb); pn_big_free(q); pn_big_free(r); return false;
}

bool pn_big_pow10(PnBigInt *value, size_t power) { while (power--) if (!pn_big_mul_small(value, 10)) return false; return true; }
bool pn_big_is_zero(const PnBigInt *value) { return value->sign == 0; }
bool pn_big_is_one(const PnBigInt *value) { return value->sign == 1 && value->len == 1 && value->limbs[0] == 1; }
bool pn_big_fits_size(const PnBigInt *value, size_t *out) {
    if (value->sign < 0) return false; size_t result = 0;
    for (size_t i = value->len; i-- > 0;) { if (result > (SIZE_MAX - value->limbs[i]) / BASE) return false; result = result * BASE + value->limbs[i]; }
    *out = result; return true;
}
