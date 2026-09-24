#include "platform.h"

#include "buffer.h"
#include "builtins_internal.h"
#include "host.h"
#include "render.h"
#include "utf8.h"
#include "value.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
extern char **environ;

/* Host I/O and invocation context; legacy string-path services support compiler bootstrap. */

static Value *read_stream(FILE *file, ValueKind kind, bool *io_error)
{
    Buffer b = {0};
    char chunk[4096];
    size_t n;
    *io_error = false;
    while ((n = fread(chunk, 1, sizeof(chunk), file)) > 0) {
        if (!buffer_add(&b, chunk, n)) {
            free(b.data);
            return NULL;
        }
    }
    if (ferror(file)) {
        free(b.data);
        *io_error = true;
        return NULL;
    }
    Value *v = value_data(kind, (uint8_t *)(b.data ? b.data : ""), b.length);
    free(b.data);
    return v;
}

static char *path_parent_text(const char *path)
{
    char *copy = copy_text(path);
    if (!copy) {
        return NULL;
    }
    size_t n = strlen(copy);
    while (n > 1 && copy[n - 1] == '/') {
        copy[--n] = 0;
    }
    char *slash = strrchr(copy, '/');
    if (!slash) {
        free(copy);
        return copy_text(".");
    }
    if (slash == copy) {
        slash[1] = 0;
    } else {
        *slash = 0;
    }
    return copy;
}

static char *path_suffix_text(const char *path)
{
    const char *slash = strrchr(path, '/'), *base = slash ? slash + 1 : path,
               *dot = strrchr(base, '.');
    return copy_text(dot && dot != base ? dot : "");
}

static char *path_normalize(const char *path)
{
    bool absolute = path[0] == '/';
    char *copy = copy_text(path), **parts = NULL;
    size_t count = 0, cap = 0;
    if (!copy) {
        return NULL;
    }
    char *at = copy;
    while (*at) {
        while (*at == '/') {
            at++;
        }
        if (!*at) {
            break;
        }
        char *start = at;
        while (*at && *at != '/') {
            at++;
        }
        if (*at) {
            *at++ = 0;
        }
        if (!strcmp(start, ".")) {
            continue;
        }
        if (!strcmp(start, "..") && count && strcmp(parts[count - 1], "..")) {
            count--;
            continue;
        }
        if (count == cap) {
            cap = cap ? cap * 2 : 8;
            char **next = realloc(parts, cap * sizeof(char *));
            if (!next) {
                free(parts);
                free(copy);
                return NULL;
            }
            parts = next;
        }
        parts[count++] = start;
    }
    Buffer out = {0};
    if (absolute) {
        buffer_text(&out, "/");
    }
    for (size_t i = 0; i < count; i++) {
        if (i) {
            buffer_text(&out, "/");
        }
        buffer_text(&out, parts[i]);
    }
    if (!out.length) {
        buffer_text(&out, absolute ? "/" : ".");
    }
    free(parts);
    free(copy);
    return out.data;
}

static char *path_join_text(const char *left, const char *right)
{
    if (right[0] == '/') {
        return path_normalize(right);
    }
    Buffer b = {0};
    buffer_text(&b, left);
    if (b.length && b.data[b.length - 1] != '/') {
        buffer_text(&b, "/");
    }
    buffer_text(&b, right);
    char *result = path_normalize(b.data);
    free(b.data);
    return result;
}

static const char *environment_value(VM *vm, const char *name)
{
    size_t n = strlen(name);
    for (size_t i = 0; i < vm->env_count; i++) {
        if (!strncmp(vm->environment[i], name, n) && vm->environment[i][n] == '=') {
            return vm->environment[i] + n + 1;
        }
    }
    return NULL;
}

bool snapshot_environment(char ***out, size_t *count)
{
    *count = 0;
    while (environ[*count]) {
        (*count)++;
    }
    *out = calloc(*count, sizeof(char *));
    if (*count && !*out) {
        return false;
    }
    for (size_t i = 0; i < *count; i++) {
        (*out)[i] = copy_text(environ[i]);
        if (!(*out)[i]) {
            for (size_t j = 0; j < i; j++) {
                free((*out)[j]);
            }
            free(*out);
            *out = NULL;
            *count = 0;
            return false;
        }
    }
    return true;
}

void free_environment(char **values, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        free(values[i]);
    }
    free(values);
}

Value *host_call(VM *vm, const char *name, Value **a)
{
    if (!strcmp(name, "print") || !strcmp(name, "eprint")) {
        Buffer b = {0};
        if (!render(&b, a[0], false)) {
            return NULL;
        }
        FILE *out = !strcmp(name, "print") ? stdout : stderr;
        fprintf(out, "%s\n", b.data ? b.data : "");
        free(b.data);
        return value_void();
    }

    if (!strcmp(name, "read_line")) {
        Buffer b = {0};
        int c;
        while ((c = fgetc(stdin)) != EOF && c != '\n') {
            if (c != '\r') {
                char ch = (char)c;
                if (!buffer_add(&b, &ch, 1)) {
                    free(b.data);
                    return NULL;
                }
            }
        }
        Value *v = value_data(V_STR, (uint8_t *)(b.data ? b.data : ""), b.length);
        free(b.data);
        return v;
    }

    if (!strcmp(name, "read_file") || !strcmp(name, "read_bytes")) {
        REQUIRE_PATH(a[0], "VM trap: file path requires Str");
        FILE *file = fopen((char *)a[0]->as.bytes.data, "rb");
        if (!file) {
            vm->error = "I/O error: could not read file";
            return NULL;
        }
        bool io_error;
        Value *v = read_stream(file, !strcmp(name, "read_file") ? V_STR : V_BYTES, &io_error);
        fclose(file);
        if (!v) {
            vm->error = io_error ? "I/O error: could not read file" : "native VM out of memory";
            return NULL;
        }
        if (v && v->kind == V_STR && !utf8(v->as.bytes.data, v->as.bytes.length)) {
            release(v);
            vm->error = "I/O error: file is not valid UTF-8";
            return NULL;
        }
        return v;
    }

    if (!strcmp(name, "write_file") || !strcmp(name, "write_bytes")) {
        REQUIRE_PATH(a[0], "VM trap: file path requires Str");
        REQUIRE((!strcmp(name, "write_file") && a[1]->kind == V_STR) ||
                    (!strcmp(name, "write_bytes") && a[1]->kind == V_BYTES),
                "VM trap: invalid file contents");
        FILE *file = fopen((char *)a[0]->as.bytes.data, "wb");
        if (!file) {
            vm->error = "I/O error: could not write file";
            return NULL;
        }
        bool ok =
            fwrite(a[1]->as.bytes.data, 1, a[1]->as.bytes.length, file) == a[1]->as.bytes.length;
        if (fclose(file) != 0) {
            ok = false;
        }
        if (!ok) {
            vm->error = "I/O error: could not write file";
            return NULL;
        }
        return value_void();
    }

    if (!strcmp(name, "command_args")) {
        Value **items = calloc((size_t)vm->argc, sizeof(Value *));
        if (vm->argc && !items) {
            return NULL;
        }
        for (int i = 0; i < vm->argc; i++) {
            items[i] = value_data(V_STR, (uint8_t *)vm->argv[i], strlen(vm->argv[i]));
        }
        Value *v = value_sequence(V_ARRAY, items, (size_t)vm->argc);
        for (int i = 0; i < vm->argc; i++) {
            release(items[i]);
        }
        free(items);
        return v;
    }

    if (!strcmp(name, "environment_has")) {
        REQUIRE(a[0]->kind == V_STR, "VM trap: environment key requires Str");
        return value_bool(environment_value(vm, (char *)a[0]->as.bytes.data) != NULL);
    }

    if (!strcmp(name, "environment_get")) {
        REQUIRE(a[0]->kind == V_STR, "VM trap: environment key requires Str");
        const char *value = environment_value(vm, (char *)a[0]->as.bytes.data);
        if (!value) {
            vm->error = "VM trap: environment variable was not found";
            return NULL;
        }
        return value_data(V_STR, (uint8_t *)value, strlen(value));
    }

    if (!strcmp(name, "path_parent")) {
        REQUIRE_PATH(a[0], "VM trap: path requires Str");
        char *s = path_parent_text((char *)a[0]->as.bytes.data);
        Value *v = s ? value_data(V_STR, (uint8_t *)s, strlen(s)) : NULL;
        free(s);
        return v;
    }

    if (!strcmp(name, "path_join")) {
        REQUIRE_PATH(a[0], "VM trap: path_join requires Str");
        REQUIRE_PATH(a[1], "VM trap: path_join requires Str");
        char *s = path_join_text((char *)a[0]->as.bytes.data, (char *)a[1]->as.bytes.data);
        Value *v = s ? value_data(V_STR, (uint8_t *)s, strlen(s)) : NULL;
        free(s);
        return v;
    }

    if (!strcmp(name, "path_suffix")) {
        REQUIRE_PATH(a[0], "VM trap: path requires Str");
        char *s = path_suffix_text((char *)a[0]->as.bytes.data);
        Value *v = s ? value_data(V_STR, (uint8_t *)s, strlen(s)) : NULL;
        free(s);
        return v;
    }

    if (!strcmp(name, "path_with_suffix")) {
        REQUIRE_PATH(a[0], "VM trap: path_with_suffix requires Str");
        REQUIRE_PATH(a[1], "VM trap: path_with_suffix requires Str");
        char *path = copy_text((char *)a[0]->as.bytes.data);
        char *suffix = path_suffix_text(path);
        if (!path || !suffix) {
            free(path);
            free(suffix);
            return NULL;
        }
        size_t base = strlen(path) - strlen(suffix);
        Buffer b = {0};
        buffer_add(&b, path, base);
        buffer_add(&b, (char *)a[1]->as.bytes.data, a[1]->as.bytes.length);
        Value *v = value_data(V_STR, (uint8_t *)b.data, b.length);
        free(path);
        free(suffix);
        free(b.data);
        return v;
    }

    if (!strcmp(name, "path_is_absolute")) {
        REQUIRE_PATH(a[0], "VM trap: path requires Str");
        return value_bool(a[0]->as.bytes.length && a[0]->as.bytes.data[0] == '/');
    }

    if (!strcmp(name, "path_resolve")) {
        REQUIRE_PATH(a[0], "VM trap: path requires Str");
        char *s;
        if (a[0]->as.bytes.length && a[0]->as.bytes.data[0] == '/') {
            s = path_normalize((char *)a[0]->as.bytes.data);
        } else {
            char cwd[4096];
            if (!getcwd(cwd, sizeof(cwd))) {
                vm->error = "I/O error: could not resolve path";
                return NULL;
            }
            s = path_join_text(cwd, (char *)a[0]->as.bytes.data);
        }
        Value *v = s ? value_data(V_STR, (uint8_t *)s, strlen(s)) : NULL;
        free(s);
        return v;
    }

    if (!strcmp(name, "file_exists")) {
        REQUIRE_PATH(a[0], "VM trap: path requires Str");
        struct stat status;
        return value_bool(stat((char *)a[0]->as.bytes.data, &status) == 0 &&
                          S_ISREG(status.st_mode));
    }

    if (!strcmp(name, "process_exit")) {
        REQUIRE(a[0]->kind == V_NAT, "VM trap: process status requires Nat");
        size_t code;
        if (!value_index(a[0], &code)) {
            vm->error = "VM trap: invalid process status";
            return NULL;
        }
        exit((int)code);
    }
    vm->error = "VM trap: unknown builtin";
    return NULL;
}
