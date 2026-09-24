#include "utf8.h"

#include <stdlib.h>
#include <string.h>

/* UTF-8 validation and code-point offsets shared by decoding and execution. */

bool utf8(const uint8_t *s, size_t n)
{
    size_t i = 0;
    while (i < n) {
        uint32_t c = s[i++];
        size_t need = 0;
        uint32_t min = 0;
        if (c < 0x80) {
            continue;
        }
        if ((c & 0xe0) == 0xc0) {
            need = 1;
            c &= 0x1f;
            min = 0x80;
        } else if ((c & 0xf0) == 0xe0) {
            need = 2;
            c &= 0x0f;
            min = 0x800;
        } else if ((c & 0xf8) == 0xf0) {
            need = 3;
            c &= 7;
            min = 0x10000;
        } else {
            return false;
        }
        if (need > n - i) {
            return false;
        }
        while (need--) {
            if ((s[i] & 0xc0) != 0x80) {
                return false;
            }
            c = (c << 6) | (s[i++] & 0x3f);
        }
        if (c < min || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff)) {
            return false;
        }
    }
    return true;
}

bool utf8_offset(const Value *value, size_t index, size_t *start, size_t *end)
{
    if (index >= value->as.bytes.characters) {
        return false;
    }
    if (value->as.bytes.ascii) {
        *start = index;
        *end = index + 1;
        return true;
    }
    const uint8_t *s = value->as.bytes.data;
    size_t n = value->as.bytes.length;
    size_t cp = 0, i = 0;
    while (i < n) {
        size_t at = i;
        uint8_t c = s[i++];
        if (c >= 0x80) {
            i += (c & 0xe0) == 0xc0 ? 1 : (c & 0xf0) == 0xe0 ? 2 : 3;
        }
        if (cp++ == index) {
            *start = at;
            *end = i;
            return true;
        }
    }
    return false;
}
