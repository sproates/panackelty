#ifndef PANACKELTY_NUMERIC_H
#define PANACKELTY_NUMERIC_H

#include "value.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Exact arithmetic returns an owned value. Arithmetic operands are borrowed. */

Value *binary_value(uint8_t op, Value *a, Value *b, const char **error);
Value *decimal_binary(uint8_t op, const Decimal *a, const Decimal *b, const char **error);
/* Moves coefficient storage on success and resets it; failure leaves it caller-owned. */
Value *value_decimal(PnBigInt *coefficient, int exponent);
Value *value_rat(const PnBigInt *n, const PnBigInt *d, const char **error);
int decimal_compare(const Decimal *a, const Decimal *b);

#endif
