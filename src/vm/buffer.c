#include "buffer.h"

#include <stdlib.h>
#include <string.h>

/* Growable byte buffer shared by rendering and host stream collection. */

bool buffer_add(Buffer *b, const char *s, size_t n)
{
    if (n > SIZE_MAX - b->length - 1) {
        return false;
    }
    size_t need = b->length + n + 1;
    if (need > b->capacity) {
        size_t cap = b->capacity ? b->capacity : 64;
        while (cap < need) {
            if (cap > SIZE_MAX / 2) {
                return false;
            }
            cap *= 2;
        }
        char *next = realloc(b->data, cap);
        if (!next) {
            return false;
        }
        b->data = next;
        b->capacity = cap;
    }
    memcpy(b->data + b->length, s, n);
    b->length += n;
    b->data[b->length] = 0;
    return true;
}

bool buffer_text(Buffer *b, const char *s)
{
    return buffer_add(b, s, strlen(s));
}
