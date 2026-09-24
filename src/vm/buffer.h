#ifndef PANACKELTY_BUFFER_H
#define PANACKELTY_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Growable byte buffer shared by rendering and host stream collection. */

typedef struct {
    char *data;
    size_t length, capacity;
} Buffer;

/* Appends bytes and maintains a trailing NUL. Caller frees data, including on failure. */
bool buffer_add(Buffer *b, const char *s, size_t n);
bool buffer_text(Buffer *b, const char *s);

#endif
