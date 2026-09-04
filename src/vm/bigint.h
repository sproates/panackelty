#ifndef PANACKELTY_BIGINT_H
#define PANACKELTY_BIGINT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    int sign;
    size_t len;
    uint32_t *limbs;
} PnBigInt;

void pn_big_init(PnBigInt *value);
void pn_big_free(PnBigInt *value);
bool pn_big_copy(PnBigInt *out, const PnBigInt *value);
bool pn_big_from_u64(PnBigInt *out, uint64_t value);
bool pn_big_from_bytes(PnBigInt *out, const uint8_t *bytes, size_t length);
bool pn_big_from_digits(PnBigInt *out, const char *digits, size_t length, int sign);
char *pn_big_string(const PnBigInt *value);
int pn_big_compare(const PnBigInt *left, const PnBigInt *right);
bool pn_big_add(PnBigInt *out, const PnBigInt *left, const PnBigInt *right);
bool pn_big_sub(PnBigInt *out, const PnBigInt *left, const PnBigInt *right);
bool pn_big_mul(PnBigInt *out, const PnBigInt *left, const PnBigInt *right);
bool pn_big_divmod(PnBigInt *quotient, PnBigInt *remainder,
                   const PnBigInt *left, const PnBigInt *right);
bool pn_big_mul_small(PnBigInt *value, uint32_t factor);
bool pn_big_add_small(PnBigInt *value, uint32_t addend);
uint32_t pn_big_div_small(PnBigInt *value, uint32_t divisor);
bool pn_big_pow10(PnBigInt *value, size_t power);
bool pn_big_is_zero(const PnBigInt *value);
bool pn_big_is_one(const PnBigInt *value);
bool pn_big_fits_size(const PnBigInt *value, size_t *out);

#endif
