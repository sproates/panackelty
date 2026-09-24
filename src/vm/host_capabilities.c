#include "platform.h"

#include "buffer.h"
#include "builtins_internal.h"
#include "host_capabilities.h"
#include "host_types.h"
#include "utf8.h"
#include "value.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
extern char **environ;
#define HC_MAX_NS UINT64_C(31536000000000000)
#define HC_MAX_BYTES (16u * 1024u * 1024u)

/* Typed POSIX services. Each call owns its descriptors and child-process cleanup. */

static Value *hc_text(const char *s)
{
    return value_data(V_STR, (const uint8_t *)s, strlen(s));
}

static const char *hc_errno(int e)
{
    switch (e) {
    case ENOENT:
        return "not_found";
    case EACCES:
    case EPERM:
        return "permission_denied";
    case EEXIST:
        return "already_exists";
    case ENOTDIR:
        return "not_directory";
    case EISDIR:
        return "is_directory";
    case ENOTEMPTY:
        return "not_empty";
    case ELOOP:
        return "symlink_loop";
    default:
        return "io_error";
    }
}

static Value *hc_error(const char *op, const char *code, int e)
{
    char *fields[] = {"operation", "code", "native_code"};
    Value *values[] = {hc_text(op), hc_text(code ? code : hc_errno(e)), value_size((size_t)e)};
    Value *record = NULL;
    if (values[0] && values[1] && values[2]) {
        record = named_value(V_RECORD, "HostError", fields, values, 3);
    }
    for (size_t i = 0; i < 3; i++) {
        release(values[i]);
    }
    return host_variant("Error", record);
}

static Value *hc_unit(void)
{
    return host_variant("Ok", value_new(V_UNIT));
}

static const char *hc_duration(Value *v, size_t *ns)
{
    if (v->as.integer.sign < 0) {
        return "negative_duration";
    }
    if (!pn_big_fits_size(&v->as.integer, ns) || *ns > HC_MAX_NS) {
        return "out_of_range";
    }
    return NULL;
}

static bool hc_clock(uint64_t *ns)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t)) {
        return false;
    }
    *ns = (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
    return true;
}

static bool hc_string(Value *v, ValueKind kind)
{
    return v->kind == kind && !memchr(v->as.bytes.data, 0, v->as.bytes.length);
}

static int hc_path_compare(const void *left, const void *right)
{
    Value *a = *(Value *const *)left, *b = *(Value *const *)right;
    size_t n = a->as.bytes.length < b->as.bytes.length ? a->as.bytes.length : b->as.bytes.length;
    int result = memcmp(a->as.bytes.data, b->as.bytes.data, n);
    return result ? result
                  : (a->as.bytes.length > b->as.bytes.length) -
                        (a->as.bytes.length < b->as.bytes.length);
}

static void hc_close(int *fd)
{
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

static void hc_free_strings(char **items)
{
    if (items) {
        for (size_t i = 0; items[i]; i++) {
            free(items[i]);
        }
        free(items);
    }
}

static Value *hc_process(VM *vm, Value **a)
{
    REQUIRE(hc_string(a[0], V_PATH) && hc_string(a[3], V_PATH), "VM trap: process requires Path");
    REQUIRE(a[1]->kind == V_ARRAY && a[2]->kind == V_BYTES && a[4]->kind == V_ARRAY &&
                a[5]->kind == V_DURATION && a[6]->kind == V_NAT,
            "VM trap: invalid process operands");
    size_t timeout = 0, limit = 0;
    const char *problem = hc_duration(a[5], &timeout);
    if (problem) {
        return hc_error("process_run", problem, 0);
    }
    if (!pn_big_fits_size(&a[6]->as.integer, &limit) || limit > HC_MAX_BYTES ||
        a[2]->as.bytes.length > HC_MAX_BYTES || a[1]->as.sequence.count > 4096 ||
        a[4]->as.sequence.count > 4096) {
        return hc_error("process_run", "out_of_range", 0);
    }
    for (size_t i = 0; i < a[1]->as.sequence.count; i++) {
        REQUIRE(a[1]->as.sequence.items[i]->kind == V_STR,
                "VM trap: process argument requires Str");
        if (!hc_string(a[1]->as.sequence.items[i], V_STR)) {
            return hc_error("process_run", "invalid_argument", 0);
        }
    }
    for (size_t i = 0; i < a[4]->as.sequence.count; i++) {
        Value *entry = a[4]->as.sequence.items[i];
        REQUIRE(entry->kind == V_STR, "VM trap: environment entry requires Str");
        char *equal = strchr((char *)entry->as.bytes.data, '=');
        if (!hc_string(entry, V_STR) || !equal || equal == (char *)entry->as.bytes.data) {
            return hc_error("process_run", "invalid_environment", 0);
        }
        for (size_t j = 0; j < i; j++) {
            char *prior = (char *)a[4]->as.sequence.items[j]->as.bytes.data;
            size_t n = (size_t)(equal - (char *)entry->as.bytes.data);
            if (!strncmp(prior, (char *)entry->as.bytes.data, n) && prior[n] == '=') {
                return hc_error("process_run", "invalid_environment", 0);
            }
        }
    }
    if (!timeout) {
        return hc_error("process_run", "timeout", 0);
    }
    size_t inherited = 0;
    inherited = vm->env_count;
    char **argv = calloc(a[1]->as.sequence.count + 2, sizeof(char *));
    char **env = calloc(inherited + a[4]->as.sequence.count + 1, sizeof(char *));
    if (!argv || !env) {
        free(argv);
        free(env);
        return NULL;
    }
    argv[0] = strdup((char *)a[0]->as.bytes.data);
    bool allocated = argv[0] != NULL;
    for (size_t i = 0; i < a[1]->as.sequence.count && allocated; i++) {
        argv[i + 1] = strdup((char *)a[1]->as.sequence.items[i]->as.bytes.data);
        allocated = argv[i + 1] != NULL;
    }
    size_t count = 0;
    for (size_t i = 0; i < inherited && allocated; i++) {
        bool replaced = false;
        for (size_t j = 0; j < a[4]->as.sequence.count; j++) {
            char *entry = (char *)a[4]->as.sequence.items[j]->as.bytes.data;
            size_t n = (size_t)(strchr(entry, '=') - entry);
            if (!strncmp(vm->environment[i], entry, n) && vm->environment[i][n] == '=') {
                replaced = true;
            }
        }
        if (!replaced) {
            env[count] = strdup(vm->environment[i]);
            allocated = env[count++] != NULL;
        }
    }
    for (size_t i = 0; i < a[4]->as.sequence.count && allocated; i++) {
        env[count] = strdup((char *)a[4]->as.sequence.items[i]->as.bytes.data);
        allocated = env[count++] != NULL;
    }
    if (!allocated) {
        hc_free_strings(argv);
        hc_free_strings(env);
        return NULL;
    }
    int pipes[4][2] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}};
    int error = 0, status = 0;
    pid_t child = -1;
    bool reaped = false;
    Buffer output[2] = {{0}, {0}};
    uint64_t start = 0;
    if (!hc_clock(&start)) {
        error = errno;
        problem = "clock_unavailable";
        goto finish;
    }
    for (size_t i = 0; i < 4; i++) {
        if (pipe(pipes[i])) {
            error = errno;
            goto finish;
        }
        /* Move descriptors above stderr so closed inherited standard streams work. */
        for (size_t j = 0; j < 2; j++) {
            if (pipes[i][j] <= STDERR_FILENO) {
                int moved = fcntl(pipes[i][j], F_DUPFD, 3);
                if (moved < 0) {
                    error = errno;
                    goto finish;
                }
                close(pipes[i][j]);
                pipes[i][j] = moved;
            }
            if (fcntl(pipes[i][j], F_SETFD, FD_CLOEXEC) < 0) {
                error = errno;
                goto finish;
            }
        }
    }
    child = fork();
    if (child < 0) {
        error = errno;
        goto finish;
    }
    if (child == 0) {
        int child_error = 0;
        if (setpgid(0, 0) || chdir((char *)a[3]->as.bytes.data) ||
            dup2(pipes[0][0], STDIN_FILENO) < 0 || dup2(pipes[1][1], STDOUT_FILENO) < 0 ||
            dup2(pipes[2][1], STDERR_FILENO) < 0) {
            child_error = errno;
        }
        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 0; j < 2; j++) {
                if (!(i == 3 && j == 1)) {
                    close(pipes[i][j]);
                }
            }
        }
        if (!child_error) {
            execve(argv[0], argv, env);
            child_error = errno;
        }
        ssize_t ignored = write(pipes[3][1], &child_error, sizeof(child_error));
        (void)ignored;
        _exit(127);
    }
    /* Child also sets its group before exec; this closes the parent-side race. */
    (void)setpgid(child, child);
    hc_close(&pipes[0][0]);
    hc_close(&pipes[1][1]);
    hc_close(&pipes[2][1]);
    hc_close(&pipes[3][1]);
    for (size_t i = 0; i < 4; i++) {
        int fd = pipes[i][i == 0 ? 1 : 0];
        if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
            error = errno;
            goto finish;
        }
    }
    size_t sent = 0;
    /* Block SIGPIPE only around writes and consume a newly generated signal. */
    while (!reaped || pipes[1][0] >= 0 || pipes[2][0] >= 0 || pipes[3][0] >= 0) {
        uint64_t now;
        if (!hc_clock(&now)) {
            error = errno;
            problem = "clock_unavailable";
            goto finish;
        }
        if (now - start >= timeout) {
            problem = "timeout";
            goto finish;
        }
        if (sent == a[2]->as.bytes.length) {
            hc_close(&pipes[0][1]);
        }
        struct pollfd fds[4] = {{pipes[0][1], POLLOUT, 0},
                                {pipes[1][0], POLLIN, 0},
                                {pipes[2][0], POLLIN, 0},
                                {pipes[3][0], POLLIN, 0}};
        uint64_t remaining = (timeout - (now - start) + 999999) / 1000000;
        int wait_ms = remaining > 10 ? 10 : (int)remaining;
        if (poll(fds, 4, wait_ms) < 0) {
            if (errno == EINTR) {
                continue;
            }
            error = errno;
            goto finish;
        }
        if (fds[0].revents) {
            sigset_t mask, old, pending;
            sigemptyset(&mask);
            sigaddset(&mask, SIGPIPE);
            if (sigprocmask(SIG_BLOCK, &mask, &old)) {
                error = errno;
                goto finish;
            }
            sigpending(&pending);
            size_t n = a[2]->as.bytes.length - sent;
            if (n > 4096) {
                n = 4096;
            }
            ssize_t wrote = write(pipes[0][1], a[2]->as.bytes.data + sent, n);
            int write_error = errno;
            if (wrote < 0 && write_error == EPIPE && !sigismember(&pending, SIGPIPE)) {
                sigset_t after;
                sigpending(&after);
                if (sigismember(&after, SIGPIPE)) {
                    int caught;
                    sigwait(&mask, &caught);
                }
            }
            sigprocmask(SIG_SETMASK, &old, NULL);
            if (wrote > 0) {
                sent += (size_t)wrote;
            } else if (wrote < 0 && write_error == EPIPE) {
                hc_close(&pipes[0][1]);
            } else if (wrote < 0 && write_error != EAGAIN && write_error != EINTR) {
                error = write_error;
                goto finish;
            }
        }
        for (size_t i = 1; i < 4; i++) {
            if (fds[i].revents) {
                uint8_t chunk[8192];
                ssize_t n = read(pipes[i][0], chunk, sizeof(chunk));
                if (n == 0) {
                    hc_close(&pipes[i][0]);
                } else if (n < 0) {
                    if (errno != EAGAIN && errno != EINTR) {
                        error = errno;
                        goto finish;
                    }
                } else if (i == 3) {
                    if ((size_t)n != sizeof(int)) {
                        problem = "io_error";
                        goto finish;
                    }
                    memcpy(&error, chunk, sizeof(error));
                    problem = "launch_failed";
                    goto finish;
                } else {
                    if (output[0].length + output[1].length + (size_t)n > limit) {
                        problem = "output_limit";
                        goto finish;
                    }
                    if (!buffer_add(&output[i - 1], (const char *)chunk, (size_t)n)) {
                        problem = "out_of_memory";
                        goto finish;
                    }
                }
            }
        }
        if (!reaped) {
            pid_t result = waitpid(child, &status, WNOHANG);
            if (result == child) {
                reaped = true;
            } else if (result < 0 && errno != EINTR) {
                error = errno;
                goto finish;
            }
        }
    }
    child = -1;
finish:
    if (child > 0) {
        kill(-child, SIGKILL);
        if (!reaped) {
            kill(child, SIGKILL);
        }
        while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
        }
    }
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 2; j++) {
            hc_close(&pipes[i][j]);
        }
    }
    hc_free_strings(argv);
    hc_free_strings(env);
    Value *result;
    if (problem || error) {
        result = hc_error("process_run", problem, error);
    } else {
        char *fields[] = {"exit_code", "signal", "stdout", "stderr"};
        Value *values[] = {value_size(WIFEXITED(status) ? (size_t)WEXITSTATUS(status) : 0),
                           value_size(WIFSIGNALED(status) ? (size_t)WTERMSIG(status) : 0),
                           value_data(V_BYTES, (const uint8_t *)output[0].data, output[0].length),
                           value_data(V_BYTES, (const uint8_t *)output[1].data, output[1].length)};
        Value *record = NULL;
        if (values[0] && values[1] && values[2] && values[3]) {
            record = named_value(V_RECORD, "ProcessOutput", fields, values, 4);
        }
        for (size_t i = 0; i < 4; i++) {
            release(values[i]);
        }
        result = host_variant("Ok", record);
    }
    free(output[0].data);
    free(output[1].data);
    return result;
}

Value *host_capability_call(VM *vm, const char *name, Value **a)
{
    if (!strcmp(name, "process_run")) {
        return hc_process(vm, a);
    }
    if (!strcmp(name, "host_decode_utf8")) {
        REQUIRE(a[0]->kind == V_BYTES, "VM trap: UTF-8 decode requires Bytes");
        if (!utf8(a[0]->as.bytes.data, a[0]->as.bytes.length)) {
            return hc_error(name, "invalid_utf8", 0);
        }
        return host_variant("Ok", value_data(V_STR, a[0]->as.bytes.data, a[0]->as.bytes.length));
    }
    if (!strcmp(name, "host_sleep")) {
        REQUIRE(a[0]->kind == V_DURATION, "VM trap: sleep requires Duration");
        size_t ns;
        const char *problem = hc_duration(a[0], &ns);
        if (problem) {
            return hc_error(name, problem, 0);
        }
        struct timespec delay = {(time_t)(ns / 1000000000), (long)(ns % 1000000000)};
        while (nanosleep(&delay, &delay)) {
            if (errno != EINTR) {
                return hc_error(name, NULL, errno);
            }
        }
        return hc_unit();
    }
    REQUIRE(hc_string(a[0], V_PATH), "VM trap: filesystem requires Path");
    char *path = (char *)a[0]->as.bytes.data;
    if (!strcmp(name, "fs_read")) {
        REQUIRE(a[1]->kind == V_NAT, "VM trap: read limit requires Nat");
        size_t limit;
        if (!pn_big_fits_size(&a[1]->as.integer, &limit) || limit > HC_MAX_BYTES) {
            return hc_error(name, "out_of_range", 0);
        }
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            return hc_error(name, NULL, errno);
        }
        struct stat info;
        if (fstat(fd, &info)) {
            int e = errno;
            close(fd);
            return hc_error(name, NULL, e);
        }
        if (!S_ISREG(info.st_mode)) {
            close(fd);
            return hc_error(name, "not_regular_file", 0);
        }
        Buffer data = {0};
        int e = 0;
        const char *problem = NULL;
        for (;;) {
            char chunk[8192];
            ssize_t n = read(fd, chunk, sizeof(chunk));
            if (!n) {
                break;
            }
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                e = errno;
                break;
            }
            if ((size_t)n > limit - data.length) {
                problem = "output_limit";
                break;
            }
            if (!buffer_add(&data, chunk, (size_t)n)) {
                problem = "out_of_memory";
                break;
            }
        }
        if (close(fd) && !e) {
            e = errno;
        }
        Value *result =
            e || problem
                ? hc_error(name, problem, e)
                : host_variant("Ok", value_data(V_BYTES, (uint8_t *)data.data, data.length));
        free(data.data);
        return result;
    }
    if (!strcmp(name, "fs_write")) {
        REQUIRE(a[1]->kind == V_BYTES, "VM trap: write requires Bytes");
        if (a[1]->as.bytes.length > HC_MAX_BYTES) {
            return hc_error(name, "out_of_range", 0);
        }
        int fd = open(path, O_WRONLY | O_CREAT | O_NONBLOCK, 0666);
        if (fd < 0) {
            return hc_error(name, NULL, errno);
        }
        struct stat info;
        int e = 0;
        if (fstat(fd, &info)) {
            e = errno;
        } else if (!S_ISREG(info.st_mode)) {
            close(fd);
            return hc_error(name, "not_regular_file", 0);
        }
        if (!e && ftruncate(fd, 0)) {
            e = errno;
        }
        size_t at = 0;
        while (!e && at < a[1]->as.bytes.length) {
            ssize_t n = write(fd, a[1]->as.bytes.data + at, a[1]->as.bytes.length - at);
            if (n > 0) {
                at += (size_t)n;
            } else if (n < 0 && errno == EINTR) {
                continue;
            } else {
                e = n < 0 ? errno : EIO;
            }
        }
        if (close(fd) && !e) {
            e = errno;
        }
        return e ? hc_error(name, NULL, e) : hc_unit();
    }
    if (!strcmp(name, "fs_metadata")) {
        struct stat info;
        if (lstat(path, &info)) {
            return hc_error(name, NULL, errno);
        }
        const char *kind = S_ISREG(info.st_mode)   ? "RegularFile"
                           : S_ISDIR(info.st_mode) ? "Directory"
                           : S_ISLNK(info.st_mode) ? "SymbolicLink"
                                                   : "OtherFile";
        char *fields[] = {"kind", "size"};
        Value *values[] = {named_value(V_VARIANT, kind, NULL, NULL, 0),
                           value_size(info.st_size < 0 ? 0 : (size_t)info.st_size)};
        Value *record = values[0] && values[1]
                            ? named_value(V_RECORD, "FileMetadata", fields, values, 2)
                            : NULL;
        release(values[0]);
        release(values[1]);
        return host_variant("Ok", record);
    }
    if (!strcmp(name, "fs_list")) {
        DIR *dir = opendir(path);
        if (!dir) {
            return hc_error(name, NULL, errno);
        }
        Value **items = NULL;
        size_t count = 0, bytes = 0;
        int e = 0;
        const char *problem = NULL;
        for (;;) {
            errno = 0;
            struct dirent *entry = readdir(dir);
            if (!entry) {
                e = errno;
                break;
            }
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
                continue;
            }
            size_t n = strlen(entry->d_name);
            bytes += n;
            if (bytes > HC_MAX_BYTES || count >= 65536) {
                problem = "output_limit";
                break;
            }
            Value **grown = realloc(items, (count + 1) * sizeof(Value *));
            if (!grown) {
                problem = "out_of_memory";
                break;
            }
            items = grown;
            items[count] = value_data(V_PATH, (uint8_t *)entry->d_name, n);
            if (!items[count]) {
                problem = "out_of_memory";
                break;
            }
            count++;
        }
        if (closedir(dir) && !e) {
            e = errno;
        }
        if (count > 1) {
            qsort(items, count, sizeof(Value *), hc_path_compare);
        }
        Value *result = e || problem ? hc_error(name, problem, e)
                                     : host_variant("Ok", value_sequence(V_ARRAY, items, count));
        for (size_t i = 0; i < count; i++) {
            release(items[i]);
        }
        free(items);
        return result;
    }
    if (!strcmp(name, "fs_temp_file") || !strcmp(name, "fs_temp_directory")) {
        Buffer template = {0};
        if (!buffer_text(&template, path) || !buffer_text(&template, "/panack-XXXXXX")) {
            free(template.data);
            return NULL;
        }
        bool directory = !strcmp(name, "fs_temp_directory");
        int e = 0;
        if (directory) {
            if (!mkdtemp(template.data)) {
                e = errno;
            }
        } else {
            int fd = mkstemp(template.data);
            if (fd < 0) {
                e = errno;
            } else if (close(fd)) {
                e = errno;
                unlink(template.data);
            }
        }
        Value *result =
            e ? hc_error(name, NULL, e)
              : host_variant("Ok", value_data(V_PATH, (uint8_t *)template.data, template.length));
        if (!result && !e) {
            if (directory) {
                rmdir(template.data);
            } else {
                unlink(template.data);
            }
        }
        free(template.data);
        return result;
    }
    int result = !strcmp(name, "fs_create_directory") ? mkdir(path, 0777)
                 : !strcmp(name, "fs_remove_file")    ? unlink(path)
                                                      : rmdir(path);
    return result ? hc_error(name, NULL, errno) : hc_unit();
}
