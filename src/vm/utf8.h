#ifndef PANACKELTY_UTF8_H
#define PANACKELTY_UTF8_H

#include "value.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* UTF-8 validation and code-point offsets shared by decoding and execution. */

bool utf8(const uint8_t *s, size_t n);
bool utf8_offset(const Value *value, size_t index, size_t *start, size_t *end);

#endif
