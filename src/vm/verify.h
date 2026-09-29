#ifndef PANACKELTY_VERIFY_H
#define PANACKELTY_VERIFY_H

#include "program.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Semantic verification is independent of source compilation and runtime checks. */

/* Borrows a decoded program; failure does not free it. */
bool verify(Program *p, const char **error);

bool verify_call_edge(const Function *caller, const Function *callee,
                      bool builtin_pure, bool builtin_async, bool awaited);

#endif
