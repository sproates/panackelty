#include "numeric.h"

#include "value.h"

#include <stdlib.h>
#include <string.h>

/* Exact arithmetic and numeric value construction; see numeric.h for ownership. */

static bool big_abs_copy(PnBigInt *out, const PnBigInt *in)
{
    if (!pn_big_copy(out, in)) {
        return false;
    }
    if (out->len) {
        out->sign = 1;
    }
    return true;
}

static bool big_gcd(PnBigInt *out, const PnBigInt *left, const PnBigInt *right)
{
    PnBigInt a = {0}, b = {0};
    if (!big_abs_copy(&a, left) || !big_abs_copy(&b, right)) {
        pn_big_free(&a);
        pn_big_free(&b);
        return false;
    }
    while (!pn_big_is_zero(&b)) {
        PnBigInt q, r;
        if (!pn_big_divmod(&q, &r, &a, &b)) {
            pn_big_free(&a);
            pn_big_free(&b);
            return false;
        }
        pn_big_free(&q);
        pn_big_free(&a);
        a = b;
        b = r;
        if (b.len) {
            b.sign = 1;
        }
    }
    pn_big_free(&b);
    *out = a;
    return true;
}

static bool decimal_align(const Decimal *a, const Decimal *b, PnBigInt *left, PnBigInt *right,
                          int *exponent)
{
    *exponent = a->exponent < b->exponent ? a->exponent : b->exponent;
    pn_big_init(left);
    pn_big_init(right);
    if (!pn_big_copy(left, &a->coefficient) || !pn_big_copy(right, &b->coefficient)) {
        pn_big_free(left);
        pn_big_free(right);
        return false;
    }
    if (!pn_big_pow10(left, (size_t)(a->exponent - *exponent)) ||
        !pn_big_pow10(right, (size_t)(b->exponent - *exponent))) {
        pn_big_free(left);
        pn_big_free(right);
        return false;
    }
    return true;
}

Value *value_decimal(PnBigInt *coefficient, int exponent)
{
    Value *v = value_new(V_DEC);
    if (!v) {
        return NULL;
    }
    v->as.decimal.coefficient = *coefficient;
    v->as.decimal.exponent = exponent;
    pn_big_init(coefficient);
    return v;
}

static Value *decimal_result(PnBigInt *coefficient, int exponent)
{
    Value *result = value_decimal(coefficient, exponent);
    pn_big_free(coefficient);
    return result;
}

Value *decimal_binary(uint8_t op, const Decimal *a, const Decimal *b, const char **error)
{
    PnBigInt left, right, result, q, r;
    int exponent;
    if (op == 0 || op == 1 || op == 4) {
        if (!decimal_align(a, b, &left, &right, &exponent)) {
            return NULL;
        }
        if (op == 0) {
            if (!pn_big_add(&result, &left, &right)) {
                goto fail_align;
            }
        } else if (op == 1) {
            if (!pn_big_sub(&result, &left, &right)) {
                goto fail_align;
            }
        } else {
            if (pn_big_is_zero(&right)) {
                *error = "VM trap: division by zero";
                goto fail_align;
            }
            if (!pn_big_divmod(&q, &r, &left, &right)) {
                goto fail_align;
            }
            pn_big_free(&q);
            result = r;
        }
        pn_big_free(&left);
        pn_big_free(&right);
        return decimal_result(&result, exponent);
    }
    if (op == 2) {
        if (!pn_big_mul(&result, &a->coefficient, &b->coefficient)) {
            return NULL;
        }
        return decimal_result(&result, a->exponent + b->exponent);
    }
    if (op == 3) {
        if (pn_big_is_zero(&b->coefficient)) {
            *error = "VM trap: division by zero";
            return NULL;
        }
        PnBigInt gcd = {0}, den = {0}, num = {0};
        if (!big_gcd(&gcd, &a->coefficient, &b->coefficient) ||
            !pn_big_divmod(&num, &r, &a->coefficient, &gcd)) {
            pn_big_free(&gcd);
            return NULL;
        }
        pn_big_free(&r);
        if (!pn_big_divmod(&den, &r, &b->coefficient, &gcd)) {
            pn_big_free(&gcd);
            pn_big_free(&num);
            return NULL;
        }
        pn_big_free(&r);
        pn_big_free(&gcd);
        if (den.sign < 0) {
            den.sign = -den.sign;
            num.sign = -num.sign;
        }
        /* A reduced fraction has a finite decimal expansion only when its
         * denominator contains no prime factors other than two and five.
         */
        size_t twos = 0, fives = 0;
        while (!pn_big_is_one(&den)) {
            PnBigInt probe;
            if (!pn_big_copy(&probe, &den)) {
                pn_big_free(&num);
                pn_big_free(&den);
                return NULL;
            }
            uint32_t rem = pn_big_div_small(&probe, 2);
            if (!rem) {
                pn_big_free(&den);
                den = probe;
                twos++;
                continue;
            }
            pn_big_free(&probe);
            if (!pn_big_copy(&probe, &den)) {
                pn_big_free(&num);
                pn_big_free(&den);
                return NULL;
            }
            rem = pn_big_div_small(&probe, 5);
            if (!rem) {
                pn_big_free(&den);
                den = probe;
                fives++;
                continue;
            }
            pn_big_free(&probe);
            *error = "VM trap: non-terminating decimal division requires explicit rounding";
            pn_big_free(&num);
            pn_big_free(&den);
            return NULL;
        }
        pn_big_free(&den);
        size_t places = twos > fives ? twos : fives;
        for (size_t i = twos; i < places; i++) {
            if (!pn_big_mul_small(&num, 2)) {
                pn_big_free(&num);
                return NULL;
            }
        }
        for (size_t i = fives; i < places; i++) {
            if (!pn_big_mul_small(&num, 5)) {
                pn_big_free(&num);
                return NULL;
            }
        }
        int exponent = a->exponent - b->exponent - (int)places;
        /* Division returns the shortest exact fractional scale, matching the
         * reference VM. Other arithmetic retains the operands' scale.
         */
        if (pn_big_is_zero(&num)) {
            exponent = 0;
        } else {
            while (exponent < 0 && num.limbs[0] % 10 == 0) {
                pn_big_div_small(&num, 10);
                exponent++;
            }
        }
        return decimal_result(&num, exponent);
    }
    return NULL;
fail_align:
    pn_big_free(&left);
    pn_big_free(&right);
    return NULL;
}

static size_t coefficient_digits(const PnBigInt *number)
{
    if (!number->len) {
        return 1;
    }
    size_t digits = (number->len - 1) * 9 + 1;
    for (uint32_t top = number->limbs[number->len - 1]; top >= 10; top /= 10) {
        digits++;
    }
    return digits;
}

static uint32_t coefficient_digit(const PnBigInt *number, size_t digits, size_t position)
{
    static const uint32_t powers[] = {1,      10,      100,      1000,     10000,
                                      100000, 1000000, 10000000, 100000000};
    if (position >= digits) {
        return 0;
    }
    size_t offset = digits - position - 1;
    return number->limbs[offset / 9] / powers[offset % 9] % 10;
}

int decimal_compare(const Decimal *a, const Decimal *b)
{
    const PnBigInt *left = &a->coefficient, *right = &b->coefficient;
    if (left->sign != right->sign) {
        return left->sign < right->sign ? -1 : 1;
    }
    if (!left->sign) {
        return 0;
    }

    /* Compare decimal magnitudes and then significant digits, padding with zero.
     * Unlike scale alignment, comparison needs no allocation and cannot silently
     * report equality when memory is exhausted. Negative zero compares as zero.
     */
    size_t left_digits = coefficient_digits(left), right_digits = coefficient_digits(right);
    int64_t left_magnitude = (int64_t)left_digits + a->exponent;
    int64_t right_magnitude = (int64_t)right_digits + b->exponent;
    if (left_magnitude != right_magnitude) {
        return (left_magnitude < right_magnitude ? -1 : 1) * left->sign;
    }
    size_t count = left_digits > right_digits ? left_digits : right_digits;
    for (size_t i = 0; i < count; i++) {
        uint32_t ld = coefficient_digit(left, left_digits, i);
        uint32_t rd = coefficient_digit(right, right_digits, i);
        if (ld != rd) {
            return (ld < rd ? -1 : 1) * left->sign;
        }
    }
    return 0;
}

Value *value_rat(const PnBigInt *n, const PnBigInt *d, const char **error)
{
    PnBigInt gcd = {0}, r = {0}, nn = {0}, dd = {0};
    Value *out = NULL;
    if (pn_big_is_zero(d)) {
        *error = "VM trap: division by zero";
        return NULL;
    }
    if (!big_gcd(&gcd, n, d)) {
        goto done;
    }
    if (!pn_big_divmod(&nn, &r, n, &gcd)) {
        goto done;
    }
    pn_big_free(&r);
    if (!pn_big_divmod(&dd, &r, d, &gcd)) {
        goto done;
    }
    if (dd.sign < 0) {
        dd.sign = 1;
        nn.sign = -nn.sign;
    }
    out = value_new(V_RAT);
    if (out) {
        out->as.rational.numerator = nn;
        out->as.rational.denominator = dd;
        pn_big_init(&nn);
        pn_big_init(&dd);
    }
done:
    pn_big_free(&gcd);
    pn_big_free(&r);
    pn_big_free(&nn);
    pn_big_free(&dd);
    return out;
}

static bool rational_operand(const Value *v)
{
    return v->kind == V_RAT || v->kind == V_NAT || v->kind == V_INT;
}

static Value *rational_binary(uint8_t op, const Value *a, const Value *b, const char **error)
{
    PnBigInt one = {0}, left = {0}, right = {0}, n = {0}, d = {0};
    Value *out = NULL;
    if (!pn_big_from_u64(&one, 1)) {
        return NULL;
    }
    const PnBigInt *an = a->kind == V_RAT ? &a->as.rational.numerator : &a->as.integer;
    const PnBigInt *ad = a->kind == V_RAT ? &a->as.rational.denominator : &one;
    const PnBigInt *bn = b->kind == V_RAT ? &b->as.rational.numerator : &b->as.integer;
    const PnBigInt *bd = b->kind == V_RAT ? &b->as.rational.denominator : &one;
    if (op >= 5 || op == 0 || op == 1) {
        if (!pn_big_mul(&left, an, bd) || !pn_big_mul(&right, bn, ad)) {
            goto done;
        }
        if (op >= 5) {
            int c = pn_big_compare(&left, &right);
            out = value_bool(op == 5   ? c == 0
                             : op == 6 ? c != 0
                             : op == 7 ? c < 0
                             : op == 8 ? c <= 0
                             : op == 9 ? c > 0
                                       : c >= 0);
            goto done;
        }
        if (!(op == 0 ? pn_big_add(&n, &left, &right) : pn_big_sub(&n, &left, &right)) ||
            !pn_big_mul(&d, ad, bd)) {
            goto done;
        }
    } else if (op == 2 || op == 3) {
        if (!pn_big_mul(&n, an, op == 2 ? bn : bd) || !pn_big_mul(&d, ad, op == 2 ? bd : bn)) {
            goto done;
        }
    } else {
        *error = "VM trap: Rat does not support remainder";
        goto done;
    }
    out = value_rat(&n, &d, error);
done:
    pn_big_free(&one);
    pn_big_free(&left);
    pn_big_free(&right);
    pn_big_free(&n);
    pn_big_free(&d);
    return out;
}

static bool comparable(const Value *a, const Value *b)
{
    return (a->kind == V_DEC && b->kind == V_DEC) ||
           ((a->kind == V_NAT || a->kind == V_INT) && (b->kind == V_NAT || b->kind == V_INT)) ||
           (a->kind == V_STR && b->kind == V_STR);
}

static int compare_values(const Value *a, const Value *b)
{
    if (a->kind == V_DEC && b->kind == V_DEC) {
        return decimal_compare(&a->as.decimal, &b->as.decimal);
    }
    if ((a->kind == V_NAT || a->kind == V_INT) && (b->kind == V_NAT || b->kind == V_INT)) {
        return pn_big_compare(&a->as.integer, &b->as.integer);
    }
    if (a->kind == V_STR && b->kind == V_STR) {
        size_t n =
            a->as.bytes.length < b->as.bytes.length ? a->as.bytes.length : b->as.bytes.length;
        int c = memcmp(a->as.bytes.data, b->as.bytes.data, n);
        return c ? c
                 : (a->as.bytes.length > b->as.bytes.length) -
                       (a->as.bytes.length < b->as.bytes.length);
    }
    return 0;
}

Value *binary_value(uint8_t op, Value *a, Value *b, const char **error)
{
    if (op <= 10 && rational_operand(a) && rational_operand(b) &&
        (a->kind == V_RAT || b->kind == V_RAT || op == 3)) {
        return rational_binary(op, a, b, error);
    }

    if (op == 11 || op == 12) {
        if (a->kind != V_BOOL || b->kind != V_BOOL) {
            *error = "VM trap: Boolean operator requires Bool";
            return NULL;
        }
        return value_bool(op == 11 ? a->as.boolean && b->as.boolean
                                   : a->as.boolean || b->as.boolean);
    }
    if (op >= 5) {
        bool result;
        if (op == 5 || op == 6) {
            result = value_equal(a, b) ^ (op == 6);
        } else {
            if (!comparable(a, b)) {
                *error = "VM trap: incompatible comparison operands";
                return NULL;
            }
            int cmp = compare_values(a, b);
            result = op == 7 ? cmp < 0 : op == 8 ? cmp <= 0 : op == 9 ? cmp > 0 : cmp >= 0;
        }
        return value_bool(result);
    }
    if (op == 0 && a->kind == V_STR && b->kind == V_STR) {
        if (a->as.bytes.length > SIZE_MAX - b->as.bytes.length) {
            return NULL;
        }
        size_t n = a->as.bytes.length + b->as.bytes.length;
        uint8_t *data = malloc(n);
        if (!data) {
            return NULL;
        }
        memcpy(data, a->as.bytes.data, a->as.bytes.length);
        memcpy(data + a->as.bytes.length, b->as.bytes.data, b->as.bytes.length);
        Value *v = value_data(V_STR, data, n);
        free(data);
        return v;
    }
    if (a->kind == V_DEC && b->kind == V_DEC) {
        return decimal_binary(op, &a->as.decimal, &b->as.decimal, error);
    }
    if ((a->kind == V_NAT || a->kind == V_INT) && (b->kind == V_NAT || b->kind == V_INT)) {
        PnBigInt result, q, r;
        bool ok = false;
        if (op == 0) {
            ok = pn_big_add(&result, &a->as.integer, &b->as.integer);
        } else if (op == 1) {
            ok = pn_big_sub(&result, &a->as.integer, &b->as.integer);
        } else if (op == 2) {
            ok = pn_big_mul(&result, &a->as.integer, &b->as.integer);
        } else {
            if (pn_big_is_zero(&b->as.integer)) {
                *error = "VM trap: division by zero";
                return NULL;
            }
            if (!pn_big_divmod(&q, &r, &a->as.integer, &b->as.integer)) {
                return NULL;
            }
            /* Integer / already returned a Rat above; only remainder reaches here. */
            {
                if (!pn_big_is_zero(&r) && a->as.integer.sign != b->as.integer.sign) {
                    PnBigInt adjusted;
                    if (!pn_big_add(&adjusted, &r, &b->as.integer)) {
                        pn_big_free(&q);
                        pn_big_free(&r);
                        return NULL;
                    }
                    pn_big_free(&r);
                    r = adjusted;
                }
                result = r;
                pn_big_free(&q);
            }
            ok = true;
        }
        if (!ok) {
            return NULL;
        }
        ValueKind kind = a->kind == V_INT || b->kind == V_INT ? V_INT : V_NAT;
        if (kind == V_NAT && result.sign < 0) {
            pn_big_free(&result);
            *error = "VM trap: Nat underflow";
            return NULL;
        }
        Value *v = value_big(kind, &result);
        pn_big_free(&result);
        return v;
    }
    *error = "VM trap: incompatible binary operands";
    return NULL;
}
