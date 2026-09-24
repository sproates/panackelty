#ifndef PANACKELTY_RENDER_H
#define PANACKELTY_RENDER_H

#include "buffer.h"
#include "value.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Language value rendering, including exact decimal scale and escaped display forms. */

/* Borrows the value and appends to a caller-owned buffer. Failure may leave partial text. */
bool render(Buffer *b, const Value *v, bool display);

#endif
