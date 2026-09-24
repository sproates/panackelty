#ifndef PANACKELTY_VERIFY_H
#define PANACKELTY_VERIFY_H

#include "program.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Semantic verification is independent of source compilation and runtime checks. */

/* Borrows a decoded program; failure does not free it. */
bool verify(Program *p, const char **error);

#endif
