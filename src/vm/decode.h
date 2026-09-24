#ifndef PANACKELTY_DECODE_H
#define PANACKELTY_DECODE_H

#include "program.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Bounded version-8 decoding. No runtime values or host operations are created here. */

#define MAX_ARTIFACT (16u * 1024u * 1024u)

bool decode(const uint8_t *data, size_t length, Program *p, const char **error);

#endif
