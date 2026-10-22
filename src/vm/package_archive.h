#ifndef PANACKELTY_PACKAGE_ARCHIVE_H
#define PANACKELTY_PACKAGE_ARCHIVE_H

#include "vm.h"

/* Bounded, non-executing local package archive inspection and restoration. */
Value *package_archive_call(VM *vm, const char *name, Value **arguments);

#endif
