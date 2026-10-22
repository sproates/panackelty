#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "platform.h"

#include "buffer.h"
#include "host_types.h"
#include "package_archive.h"
#include "utf8.h"
#include "value.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

#ifdef __APPLE__
#include <sys/stdio.h>
#else
#include <sys/syscall.h>
#include <linux/fs.h>
#endif

#define PA_COMPRESSED_LIMIT (64u * 1024u * 1024u)
#define PA_EXPANDED_LIMIT (256u * 1024u * 1024u)
#define PA_ENTRY_LIMIT 16384u
#define PA_PATH_LIMIT 4096u

typedef struct {
    uint32_t state[8];
    uint64_t bits;
    uint8_t block[64];
    size_t used;
} PaSha256;

typedef struct {
    char *path;
    uint8_t type;
    uint8_t *contents;
    size_t length;
    bool explicit_entry;
} PaEntry;

typedef struct {
    PaEntry *items;
    size_t count;
    size_t capacity;
    size_t expanded;
    uint8_t *manifest;
    size_t manifest_length;
} PaArchive;

static const uint32_t PA_SHA_K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

static uint32_t pa_rotr(uint32_t x, unsigned n)
{
    return (x >> n) | (x << (32u - n));
}

static void pa_sha_transform(PaSha256 *s, const uint8_t *block)
{
    uint32_t w[64];
    for (size_t i = 0; i < 16; i++) {
        size_t p = i * 4;
        w[i] = ((uint32_t)block[p] << 24) | ((uint32_t)block[p + 1] << 16) |
               ((uint32_t)block[p + 2] << 8) | (uint32_t)block[p + 3];
    }
    for (size_t i = 16; i < 64; i++) {
        uint32_t a = pa_rotr(w[i - 15], 7) ^ pa_rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t b = pa_rotr(w[i - 2], 17) ^ pa_rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + a + w[i - 7] + b;
    }
    uint32_t a = s->state[0], b = s->state[1], c = s->state[2], d = s->state[3];
    uint32_t e = s->state[4], f = s->state[5], g = s->state[6], h = s->state[7];
    for (size_t i = 0; i < 64; i++) {
        uint32_t s1 = pa_rotr(e, 6) ^ pa_rotr(e, 11) ^ pa_rotr(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t t1 = h + s1 + ch + PA_SHA_K[i] + w[i];
        uint32_t s0 = pa_rotr(a, 2) ^ pa_rotr(a, 13) ^ pa_rotr(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = s0 + maj;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    s->state[0] += a; s->state[1] += b; s->state[2] += c; s->state[3] += d;
    s->state[4] += e; s->state[5] += f; s->state[6] += g; s->state[7] += h;
}

static void pa_sha_init(PaSha256 *s)
{
    static const uint32_t initial[8] = {0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u,
        0xa54ff53au, 0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u};
    memcpy(s->state, initial, sizeof(initial));
    s->bits = 0;
    s->used = 0;
}

static void pa_sha_update(PaSha256 *s, const void *data, size_t length)
{
    const uint8_t *bytes = data;
    for (size_t i = 0; i < length; i++) {
        s->block[s->used++] = bytes[i];
        if (s->used == sizeof(s->block)) {
            pa_sha_transform(s, s->block);
            s->bits += 512;
            s->used = 0;
        }
    }
}

static void pa_sha_final(PaSha256 *s, uint8_t out[32])
{
    size_t i = s->used;
    s->block[i++] = 0x80;
    if (i > 56) {
        while (i < 64) s->block[i++] = 0;
        pa_sha_transform(s, s->block);
        i = 0;
    }
    while (i < 56) s->block[i++] = 0;
    s->bits += (uint64_t)s->used * 8;
    for (size_t j = 0; j < 8; j++) s->block[63 - j] = (uint8_t)(s->bits >> (j * 8));
    pa_sha_transform(s, s->block);
    for (size_t j = 0; j < 8; j++) {
        out[j * 4] = (uint8_t)(s->state[j] >> 24);
        out[j * 4 + 1] = (uint8_t)(s->state[j] >> 16);
        out[j * 4 + 2] = (uint8_t)(s->state[j] >> 8);
        out[j * 4 + 3] = (uint8_t)s->state[j];
    }
}

static void pa_hex(const uint8_t digest[32], char out[65])
{
    static const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < 32; i++) {
        out[i * 2] = digits[digest[i] >> 4];
        out[i * 2 + 1] = digits[digest[i] & 15];
    }
    out[64] = 0;
}

static void pa_sha_hex(const void *data, size_t length, char out[65])
{
    PaSha256 state;
    uint8_t digest[32];
    pa_sha_init(&state);
    pa_sha_update(&state, data, length);
    pa_sha_final(&state, digest);
    pa_hex(digest, out);
}

static Value *pa_text(const char *text)
{
    return value_data(V_STR, (const uint8_t *)text, strlen(text));
}

static Value *pa_error(const char *operation, const char *code, int native_code)
{
    char *fields[] = {"operation", "code", "native_code"};
    Value *values[] = {pa_text(operation), pa_text(code), value_size((size_t)native_code)};
    Value *record = values[0] && values[1] && values[2]
        ? named_value(V_RECORD, "HostError", fields, values, 3) : NULL;
    for (size_t i = 0; i < 3; i++) release(values[i]);
    return host_variant("Error", record);
}

static Value *pa_ok(Value *value)
{
    return host_variant("Ok", value);
}

static char *pa_copy_bytes(const uint8_t *bytes, size_t length)
{
    char *copy = malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, bytes, length);
    copy[length] = 0;
    return copy;
}

static bool pa_read_file(const char *path, uint8_t **contents, size_t *length, const char **code, int *native)
{
    int fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) { *code = errno == ENOENT ? "not_found" : "io_error"; *native = errno; return false; }
    struct stat info;
    if (fstat(fd, &info)) { *native = errno; *code = "io_error"; close(fd); return false; }
    if (!S_ISREG(info.st_mode)) { *code = "not_regular_file"; *native = 0; close(fd); return false; }
    if (info.st_size < 0 || (uintmax_t)info.st_size > PA_COMPRESSED_LIMIT) {
        *code = "resource_limit"; *native = 0; close(fd); return false;
    }
    size_t cap = (size_t)info.st_size;
    uint8_t *data = malloc(cap ? cap : 1);
    if (!data) { *code = "out_of_memory"; *native = 0; close(fd); return false; }
    size_t at = 0;
    while (at < cap) {
        ssize_t n = read(fd, data + at, cap - at);
        if (n > 0) at += (size_t)n;
        else if (n < 0 && errno == EINTR) continue;
        else { *code = "io_error"; *native = n < 0 ? errno : EIO; free(data); close(fd); return false; }
    }
    uint8_t extra;
    ssize_t n;
    do { n = read(fd, &extra, 1); } while (n < 0 && errno == EINTR);
    if (n != 0) { *code = "source_changed"; *native = n < 0 ? errno : 0; free(data); close(fd); return false; }
    if (close(fd)) { *code = "io_error"; *native = errno; free(data); return false; }
    *contents = data;
    *length = cap;
    return true;
}

static bool pa_inflate_gzip(const uint8_t *input, size_t input_length, uint8_t **output, size_t *output_length)
{
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    if (inflateInit2(&stream, 15 + 16) != Z_OK) return false;
    size_t capacity = 65536, length = 0;
    uint8_t *data = malloc(capacity);
    if (!data) { inflateEnd(&stream); return false; }
    stream.next_in = (Bytef *)input;
    stream.avail_in = (uInt)input_length;
    int result = Z_OK;
    while (result == Z_OK) {
        if (length == capacity) {
            if (capacity >= PA_EXPANDED_LIMIT) { result = Z_MEM_ERROR; break; }
            size_t next = capacity * 2;
            if (next > PA_EXPANDED_LIMIT) next = PA_EXPANDED_LIMIT;
            uint8_t *grown = realloc(data, next);
            if (!grown) { result = Z_MEM_ERROR; break; }
            data = grown; capacity = next;
        }
        stream.next_out = data + length;
        stream.avail_out = (uInt)(capacity - length);
        result = inflate(&stream, Z_NO_FLUSH);
        length = capacity - stream.avail_out;
        if (result == Z_STREAM_END) break;
        if (result != Z_OK || (stream.avail_in == 0 && stream.avail_out > 0)) break;
    }
    bool ok = result == Z_STREAM_END && stream.avail_in == 0 && length <= PA_EXPANDED_LIMIT;
    inflateEnd(&stream);
    if (!ok) { free(data); return false; }
    *output = data;
    *output_length = length;
    return true;
}

static void pa_archive_free(PaArchive *archive)
{
    for (size_t i = 0; i < archive->count; i++) {
        free(archive->items[i].path);
        free(archive->items[i].contents);
    }
    free(archive->items);
    free(archive->manifest);
    memset(archive, 0, sizeof(*archive));
}

static ssize_t pa_find_entry(const PaArchive *archive, const char *path)
{
    for (size_t i = 0; i < archive->count; i++) {
        if (!strcmp(archive->items[i].path, path)) return (ssize_t)i;
    }
    return -1;
}

static bool pa_add_entry(PaArchive *archive, const char *path, uint8_t type,
                         const uint8_t *contents, size_t length, bool explicit_entry)
{
    ssize_t found = pa_find_entry(archive, path);
    if (found >= 0) {
        PaEntry *previous = &archive->items[(size_t)found];
        if (previous->type == 'D' && type == 'D' && !previous->explicit_entry && explicit_entry) {
            previous->explicit_entry = true;
            return true;
        }
        return false;
    }
    if (archive->count >= PA_ENTRY_LIMIT || length > PA_EXPANDED_LIMIT - archive->expanded) return false;
    if (archive->count == archive->capacity) {
        size_t next = archive->capacity ? archive->capacity * 2 : 64;
        if (next > PA_ENTRY_LIMIT) next = PA_ENTRY_LIMIT;
        PaEntry *grown = realloc(archive->items, next * sizeof(*grown));
        if (!grown) return false;
        archive->items = grown; archive->capacity = next;
    }
    char *path_copy = strdup(path);
    uint8_t *data_copy = NULL;
    if (length) {
        data_copy = malloc(length);
        if (data_copy) memcpy(data_copy, contents, length);
    }
    if (!path_copy || (length && !data_copy)) { free(path_copy); free(data_copy); return false; }
    archive->items[archive->count++] = (PaEntry){path_copy, type, data_copy, length, explicit_entry};
    archive->expanded += length;
    return true;
}

static bool pa_normalize_path(const uint8_t *path, size_t length, bool directory, char **normalized)
{
    if (!length || length >= PA_PATH_LIMIT || path[0] == '/' || memchr(path, 0, length) ||
        memchr(path, '\\', length) || !utf8(path, length)) return false;
    char *copy = pa_copy_bytes(path, length);
    if (!copy) return false;
    size_t used = 0, start = 0;
    for (size_t i = 0; i <= length; i++) {
        if (i == length || copy[i] == '/') {
            size_t part = i - start;
            if (part == 0 || (part == 1 && copy[start] == '.')) {
                if (part == 0 && i != length) { free(copy); return false; }
            } else if (part == 2 && copy[start] == '.' && copy[start + 1] == '.') {
                free(copy); return false;
            } else {
                if (used) copy[used++] = '/';
                memmove(copy + used, copy + start, part);
                used += part;
            }
            start = i + 1;
        }
    }
    if (used && copy[used - 1] == '/') used--;
    copy[used] = 0;
    if (!used) {
        free(copy);
        if (!directory) return false;
        *normalized = strdup("");
        return *normalized != NULL;
    }
    *normalized = copy;
    return true;
}

static bool pa_octal(const uint8_t *text, size_t length, uint64_t *value)
{
    size_t i = 0;
    while (i < length && (text[i] == ' ' || text[i] == 0)) i++;
    uint64_t result = 0;
    bool any = false;
    for (; i < length && text[i] != 0 && text[i] != ' '; i++) {
        if (text[i] < '0' || text[i] > '7' || result > (UINT64_MAX - 7) / 8) return false;
        result = result * 8 + (uint64_t)(text[i] - '0');
        any = true;
    }
    *value = any ? result : 0;
    return true;
}

static bool pa_tar_checksum(const uint8_t *header)
{
    uint64_t declared = 0;
    if (!pa_octal(header + 148, 8, &declared)) return false;
    uint64_t actual = 0;
    for (size_t i = 0; i < 512; i++) actual += (i >= 148 && i < 156) ? ' ' : header[i];
    return declared == actual;
}

static size_t pa_field_length(const uint8_t *field, size_t capacity)
{
    size_t length = 0;
    while (length < capacity && field[length]) length++;
    return length;
}

static bool pa_tar_name(const uint8_t *header, uint8_t **name, size_t *length)
{
    size_t base_length = pa_field_length(header, 100);
    size_t prefix_length = pa_field_length(header + 345, 155);
    if (!base_length) return false;
    size_t total = base_length + (prefix_length ? prefix_length + 1 : 0);
    uint8_t *value = malloc(total);
    if (!value) return false;
    size_t at = 0;
    if (prefix_length) { memcpy(value, header + 345, prefix_length); at += prefix_length; value[at++] = '/'; }
    memcpy(value + at, header, base_length);
    *name = value; *length = total;
    return true;
}

static bool pa_pax_record(const uint8_t *bytes, size_t length, char **path)
{
    size_t at = 0;
    while (at < length) {
        size_t record_start = at;
        uint64_t record_length = 0;
        bool digits = false;
        while (at < length && bytes[at] >= '0' && bytes[at] <= '9') {
            unsigned digit = (unsigned)(bytes[at++] - '0');
            if (record_length > (UINT64_MAX - digit) / 10) return false;
            record_length = record_length * 10 + digit;
            digits = true;
        }
        if (!digits || at >= length || bytes[at++] != ' ' ||
            record_length < at - record_start + 2 || record_length > length - record_start)
            return false;
        size_t record_end = record_start + (size_t)record_length;
        if (bytes[record_end - 1] != '\n') return false;
        size_t key_start = at;
        while (at < record_end && bytes[at] != '=') at++;
        if (at == key_start || at >= record_end - 1) return false;
        size_t key_length = at - key_start;
        size_t value_start = ++at;
        size_t value_length = record_end - value_start - 1;
        const uint8_t *key = bytes + key_start;
        bool is_path = key_length == 4 && !memcmp(key, "path", 4);
        bool harmless = (key_length == 5 && !memcmp(key, "atime", 5)) ||
            (key_length == 5 && !memcmp(key, "ctime", 5)) ||
            (key_length == 5 && !memcmp(key, "mtime", 5)) ||
            (key_length == 3 && !memcmp(key, "uid", 3)) ||
            (key_length == 3 && !memcmp(key, "gid", 3)) ||
            (key_length == 5 && !memcmp(key, "uname", 5)) ||
            (key_length == 5 && !memcmp(key, "gname", 5)) ||
            (key_length == 7 && !memcmp(key, "comment", 7)) ||
            (key_length == 7 && !memcmp(key, "charset", 7)) ||
            (key_length == 9 && !memcmp(key, "hdrcharset", 9)) ||
            (key_length > 17 && !memcmp(key, "LIBARCHIVE.xattr.", 17)) ||
            (key_length > 13 && !memcmp(key, "SCHILY.xattr.", 13));
        if (is_path) {
            if (*path || !value_length || memchr(bytes + value_start, 0, value_length) ||
                !utf8(bytes + value_start, value_length)) return false;
            *path = pa_copy_bytes(bytes + value_start, value_length);
            if (!*path) return false;
        } else if (!harmless) {
            return false;
        }
        at = record_end;
    }
    return true;
}

static bool pa_parse_tar(const uint8_t *bytes, size_t length, PaArchive *archive)
{
    size_t at = 0;
    bool ended = false;
    char *pax_path = NULL;
    while (at + 512 <= length) {
        const uint8_t *header = bytes + at;
        bool zero = true;
        for (size_t i = 0; i < 512; i++) if (header[i]) { zero = false; break; }
        if (zero) { ended = true; break; }
        if (!pa_tar_checksum(header)) { free(pax_path); return false; }
        uint64_t file_size = 0;
        if (!pa_octal(header + 124, 12, &file_size) || file_size > SIZE_MAX) {
            free(pax_path); return false;
        }
        uint8_t type_flag = header[156];
        if (type_flag == 'x') {
            if (pax_path || file_size > length - at - 512) { free(pax_path); return false; }
            if (!pa_pax_record(bytes + at + 512, (size_t)file_size, &pax_path)) {
                free(pax_path); return false;
            }
            size_t padded_extension = ((size_t)file_size + 511u) & ~((size_t)511u);
            if (padded_extension > length - at - 512) { free(pax_path); return false; }
            at += 512 + padded_extension;
            continue;
        }
        uint8_t entry_type = (type_flag == 0 || type_flag == '0') ? 'F'
            : type_flag == '5' ? 'D' : 0;
        if (!entry_type || (entry_type == 'D' && file_size != 0)) { free(pax_path); return false; }
        uint8_t *name = NULL;
        size_t name_length = 0;
        if (pax_path) {
            name = (uint8_t *)pax_path;
            name_length = strlen(pax_path);
            pax_path = NULL;
        } else if (!pa_tar_name(header, &name, &name_length)) {
            return false;
        }
        char *normalized = NULL;
        bool valid_path = pa_normalize_path(name, name_length, entry_type == 'D', &normalized);
        free(name);
        if (!valid_path) return false;
        at += 512;
        if (file_size > length - at) { free(normalized); return false; }
        if (normalized[0] == 0) {
            free(normalized);
            if (entry_type != 'D') return false;
        } else if (!pa_add_entry(archive, normalized, entry_type, bytes + at,
                                 (size_t)file_size, true)) {
            free(normalized); return false;
        } else {
            free(normalized);
        }
        size_t padded = ((size_t)file_size + 511u) & ~((size_t)511u);
        if (padded > length - at) return false;
        at += padded;
    }
    if (!ended || pax_path) { free(pax_path); return false; }
    for (size_t i = at; i < length; i++) if (bytes[i]) return false;
    return true;
}

static char pa_ascii_fold(char value);

static int pa_entry_compare(const void *left, const void *right)
{
    const PaEntry *a = left, *b = right;
    size_t i = 0;
    while (a->path[i] && b->path[i]) {
        char ac = pa_ascii_fold(a->path[i]);
        char bc = pa_ascii_fold(b->path[i]);
        if (ac != bc) return ac < bc ? -1 : 1;
        i++;
    }
    if (a->path[i] || b->path[i]) return a->path[i] ? 1 : -1;
    return strcmp(a->path, b->path);
}

static char pa_ascii_fold(char value)
{
    return value >= 'A' && value <= 'Z' ? (char)(value + ('a' - 'A')) : value;
}

static bool pa_ascii_case_equal(const char *a, const char *b)
{
    while (*a && *b) {
        if (pa_ascii_fold(*a++) != pa_ascii_fold(*b++)) return false;
    }
    return *a == *b;
}

static bool pa_add_parent_directories(PaArchive *archive)
{
    size_t original = archive->count;
    for (size_t i = 0; i < original; i++) {
        const char *path = archive->items[i].path;
        for (size_t j = 0; path[j]; j++) {
            if (path[j] == '/') {
                char *parent = pa_copy_bytes((const uint8_t *)path, j);
                if (!parent) return false;
                ssize_t found = pa_find_entry(archive, parent);
                if (found >= 0) {
                    if (archive->items[(size_t)found].type != 'D') { free(parent); return false; }
                } else if (!pa_add_entry(archive, parent, 'D', NULL, 0, false)) {
                    free(parent); return false;
                }
                free(parent);
            }
        }
    }
    qsort(archive->items, archive->count, sizeof(*archive->items), pa_entry_compare);
    for (size_t i = 1; i < archive->count; i++) {
        if (pa_ascii_case_equal(archive->items[i - 1].path, archive->items[i].path)) return false;
    }
    ssize_t manifest = pa_find_entry(archive, "panack.json");
    if (manifest < 0 || archive->items[(size_t)manifest].type != 'F' ||
        archive->items[(size_t)manifest].length > 65536) return false;
    archive->manifest_length = archive->items[(size_t)manifest].length;
    archive->manifest = malloc(archive->manifest_length ? archive->manifest_length : 1);
    if (!archive->manifest) return false;
    if (archive->manifest_length) memcpy(archive->manifest,
        archive->items[(size_t)manifest].contents, archive->manifest_length);
    return true;
}

static void pa_u64(PaSha256 *hash, uint64_t value)
{
    uint8_t bytes[8];
    for (size_t i = 0; i < 8; i++) bytes[7 - i] = (uint8_t)(value >> (i * 8));
    pa_sha_update(hash, bytes, sizeof(bytes));
}

static void pa_tree_digest(const PaArchive *archive, char out[65])
{
    PaSha256 hash;
    uint8_t digest[32];
    static const char prefix[] = "PANACK-PACKAGE-TREE\0v1\0";
    pa_sha_init(&hash);
    pa_sha_update(&hash, prefix, sizeof(prefix) - 1);
    for (size_t i = 0; i < archive->count; i++) {
        const PaEntry *entry = &archive->items[i];
        uint8_t type = entry->type;
        size_t path_length = strlen(entry->path);
        pa_sha_update(&hash, &type, 1);
        pa_u64(&hash, path_length);
        pa_sha_update(&hash, entry->path, path_length);
        pa_u64(&hash, entry->length);
        if (entry->length) pa_sha_update(&hash, entry->contents, entry->length);
    }
    pa_sha_final(&hash, digest);
    pa_hex(digest, out);
}

static bool pa_parse_archive_path(const char *path, PaArchive *archive,
                                  char archive_digest[65], char tree_digest[65],
                                  const char **code, int *native)
{
    uint8_t *compressed = NULL, *expanded = NULL;
    size_t compressed_length = 0, expanded_length = 0;
    if (!pa_read_file(path, &compressed, &compressed_length, code, native)) return false;
    if (!compressed_length) { *code = "unsupported_archive"; *native = 0; free(compressed); return false; }
    pa_sha_hex(compressed, compressed_length, archive_digest);
    if (!pa_inflate_gzip(compressed, compressed_length, &expanded, &expanded_length)) {
        *code = "unsupported_archive"; *native = 0; free(compressed); return false;
    }
    free(compressed);
    if (!pa_parse_tar(expanded, expanded_length, archive) || !pa_add_parent_directories(archive)) {
        *code = archive->count >= PA_ENTRY_LIMIT || archive->expanded >= PA_EXPANDED_LIMIT
            ? "resource_limit" : "invalid_archive";
        *native = 0; free(expanded); return false;
    }
    free(expanded);
    pa_tree_digest(archive, tree_digest);
    return true;
}

static bool pa_write_all(int fd, const uint8_t *data, size_t length)
{
    size_t at = 0;
    while (at < length) {
        ssize_t n = write(fd, data + at, length - at);
        if (n > 0) at += (size_t)n;
        else if (n < 0 && errno == EINTR) continue;
        else return false;
    }
    return true;
}

static bool pa_make_parent_dirs(int root, const char *path)
{
    char *copy = strdup(path);
    if (!copy) return false;
    int current = dup(root);
    if (current < 0) { free(copy); return false; }
    char *slash = strchr(copy, '/');
    while (slash) {
        *slash = 0;
        if (mkdirat(current, copy, 0700) && errno != EEXIST) { close(current); free(copy); return false; }
        int next = openat(current, copy, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (next < 0) { close(current); free(copy); return false; }
        close(current); current = next;
        *slash = '/';
        slash = strchr(slash + 1, '/');
    }
    close(current); free(copy);
    return true;
}

static bool pa_extract_archive(const PaArchive *archive, int root)
{
    bool ok = true;
    for (size_t i = 0; i < archive->count && ok; i++) {
        const PaEntry *entry = &archive->items[i];
        if (!pa_make_parent_dirs(root, entry->path)) { ok = false; break; }
        if (entry->type == 'D') {
            if (mkdirat(root, entry->path, 0700) && errno != EEXIST) { ok = false; break; }
            int fd = openat(root, entry->path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (fd < 0) ok = false;
            else close(fd);
        } else {
            int fd = openat(root, entry->path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
            if (fd < 0) { ok = false; break; }
            if (!pa_write_all(fd, entry->contents, entry->length) || fchmod(fd, 0644)) ok = false;
            if (close(fd)) ok = false;
        }
    }
    return ok;
}

static void pa_remove_tree_fd(int parent, const char *name)
{
    int fd = openat(parent, name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) { (void)unlinkat(parent, name, 0); return; }
    DIR *directory = fdopendir(fd);
    if (!directory) { close(fd); return; }
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        struct stat info;
        if (!fstatat(dirfd(directory), entry->d_name, &info, AT_SYMLINK_NOFOLLOW) && S_ISDIR(info.st_mode))
            pa_remove_tree_fd(dirfd(directory), entry->d_name);
        else
            (void)unlinkat(dirfd(directory), entry->d_name, 0);
    }
    closedir(directory);
    (void)unlinkat(parent, name, AT_REMOVEDIR);
}

static int pa_open_absolute_directory(const char *path)
{
    if (!path || path[0] != '/') { errno = EINVAL; return -1; }
    int current = open("/", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (current < 0) return -1;
    size_t length = strlen(path), start = 1;
    for (size_t i = 1; i <= length; i++) {
        if (i == length || path[i] == '/') {
            size_t part = i - start;
            if (part == 0 || (part == 1 && path[start] == '.')) { start = i + 1; continue; }
            if (part == 2 && path[start] == '.' && path[start + 1] == '.') { close(current); errno = EINVAL; return -1; }
            char *name = pa_copy_bytes((const uint8_t *)path + start, part);
            if (!name) { close(current); return -1; }
            int next = openat(current, name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            free(name);
            if (next < 0) { close(current); return -1; }
            close(current); current = next; start = i + 1;
        }
    }
    return current;
}

static bool pa_install_archive(const PaArchive *archive, const char *destination)
{
    char *path = strdup(destination);
    if (!path) return false;
    char *slash = strrchr(path, '/');
    if (!slash || !slash[1]) { free(path); errno = EINVAL; return false; }
    char *leaf = strdup(slash + 1);
    if (!leaf) { free(path); return false; }
    if (!strcmp(leaf, ".") || !strcmp(leaf, "..")) { free(leaf); free(path); errno = EINVAL; return false; }
    if (slash == path) slash[1] = 0; else *slash = 0;
    char *resolved_parent = realpath(path, NULL);
    int parent = resolved_parent ? pa_open_absolute_directory(resolved_parent) : -1;
    free(path);
    if (parent < 0) { free(resolved_parent); free(leaf); return false; }
    char template[PA_PATH_LIMIT];
    int n = snprintf(template, sizeof(template), "%s/.panack-restore-XXXXXX", resolved_parent);
    free(resolved_parent);
    if (n < 0 || (size_t)n >= sizeof(template) || !mkdtemp(template)) { close(parent); free(leaf); return false; }
    char *stage_leaf = strrchr(template, '/');
    if (!stage_leaf) { close(parent); free(leaf); return false; }
    stage_leaf++;
    int stage_fd = openat(parent, stage_leaf, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (stage_fd < 0) { pa_remove_tree_fd(parent, stage_leaf); close(parent); free(leaf); return false; }
    bool extracted = pa_extract_archive(archive, stage_fd);
    close(stage_fd);
    if (!extracted) {
        pa_remove_tree_fd(parent, stage_leaf); close(parent); free(leaf); errno = EINVAL; return false;
    }
    struct stat target;
    if (fstatat(parent, leaf, &target, AT_SYMLINK_NOFOLLOW) == 0) {
        if (!S_ISDIR(target.st_mode)) { pa_remove_tree_fd(parent, stage_leaf); close(parent); free(leaf); errno = ELOOP; return false; }
#ifdef __APPLE__
        if (renameatx_np(parent, stage_leaf, parent, leaf, RENAME_SWAP)) {
            pa_remove_tree_fd(parent, stage_leaf); close(parent); free(leaf); return false;
        }
#else
        if (syscall(SYS_renameat2, parent, stage_leaf, parent, leaf, RENAME_EXCHANGE)) {
            pa_remove_tree_fd(parent, stage_leaf); close(parent); free(leaf); return false;
        }
#endif
        pa_remove_tree_fd(parent, stage_leaf);
    } else if (errno == ENOENT) {
        if (renameat(parent, stage_leaf, parent, leaf)) {
            pa_remove_tree_fd(parent, stage_leaf); close(parent); free(leaf); return false;
        }
    } else {
        pa_remove_tree_fd(parent, stage_leaf); close(parent); free(leaf); return false;
    }
    close(parent); free(leaf);
    return true;
}

static Value *pa_inspect(VM *vm, Value **a)
{
    (void)vm;
    if (a[0]->kind != V_PATH || memchr(a[0]->as.bytes.data, 0, a[0]->as.bytes.length))
        return pa_error("package_archive_inspect", "invalid_argument", 0);
    char *path = pa_copy_bytes(a[0]->as.bytes.data, a[0]->as.bytes.length);
    if (!path) return pa_error("package_archive_inspect", "out_of_memory", 0);
    PaArchive archive = {0};
    char archive_digest[65], tree_digest[65];
    const char *code = "invalid_archive";
    int native = 0;
    if (!pa_parse_archive_path(path, &archive, archive_digest, tree_digest, &code, &native)) {
        pa_archive_free(&archive); free(path);
        return pa_error("package_archive_inspect", code, native);
    }
    free(path);
    Buffer output = {0};
    bool ok = buffer_text(&output, archive_digest) && buffer_text(&output, "\n") &&
              buffer_text(&output, tree_digest) && buffer_text(&output, "\n") &&
              buffer_add(&output, (const char *)archive.manifest, archive.manifest_length);
    pa_archive_free(&archive);
    if (!ok) { free(output.data); return pa_error("package_archive_inspect", "out_of_memory", 0); }
    Value *result = pa_ok(value_data(V_BYTES, (const uint8_t *)output.data, output.length));
    free(output.data);
    return result;
}

static bool pa_digest_text(Value *value, const char **text)
{
    if (value->kind != V_STR || value->as.bytes.length != 64 ||
        memchr(value->as.bytes.data, 0, value->as.bytes.length)) return false;
    *text = (const char *)value->as.bytes.data;
    for (size_t i = 0; i < 64; i++) {
        char c = (*text)[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
    }
    return true;
}

static Value *pa_restore(VM *vm, Value **a)
{
    (void)vm;
    const char *expected_archive = NULL, *expected_tree = NULL;
    if (a[0]->kind != V_PATH || a[1]->kind != V_PATH ||
        memchr(a[0]->as.bytes.data, 0, a[0]->as.bytes.length) ||
        memchr(a[1]->as.bytes.data, 0, a[1]->as.bytes.length) ||
        !pa_digest_text(a[2], &expected_archive) || !pa_digest_text(a[3], &expected_tree))
        return pa_error("package_archive_restore", "invalid_argument", 0);
    char *source = pa_copy_bytes(a[0]->as.bytes.data, a[0]->as.bytes.length);
    char *destination = pa_copy_bytes(a[1]->as.bytes.data, a[1]->as.bytes.length);
    if (!source || !destination) { free(source); free(destination); return pa_error("package_archive_restore", "out_of_memory", 0); }
    PaArchive archive = {0};
    char actual_archive[65], actual_tree[65];
    const char *code = "invalid_archive";
    int native = 0;
    if (!pa_parse_archive_path(source, &archive, actual_archive, actual_tree, &code, &native)) {
        pa_archive_free(&archive); free(source); free(destination);
        return pa_error("package_archive_restore", code, native);
    }
    free(source);
    if (strcmp(expected_archive, actual_archive) || strcmp(expected_tree, actual_tree)) {
        pa_archive_free(&archive); free(destination);
        return pa_error("package_archive_restore", "digest_mismatch", 0);
    }
    bool installed = pa_install_archive(&archive, destination);
    int error = errno;
    pa_archive_free(&archive); free(destination);
    return installed ? pa_ok(value_new(V_UNIT))
        : pa_error("package_archive_restore", error == ELOOP ? "path_escape" : "io_error", error);
}

Value *package_archive_call(VM *vm, const char *name, Value **arguments)
{
    if (!strcmp(name, "package_archive_inspect")) return pa_inspect(vm, arguments);
    if (!strcmp(name, "package_archive_restore")) return pa_restore(vm, arguments);
    return NULL;
}
