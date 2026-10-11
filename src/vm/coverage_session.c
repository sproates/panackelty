#include "platform.h"
#include "coverage_session.h"
#include <stdlib.h>
#include <string.h>
#ifndef __wasi__
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#define SESSION_ROOTS 16
#define SESSION_PATH 4096
#define SESSION_ID 192
#define SESSION_CHILDREN 4096

typedef struct {
    uint8_t *artifact, *inventory;
    size_t artifact_size, inventory_size;
} Identity;
typedef struct {
    char directory[SESSION_PATH];
    uint8_t nonce[16];
    Identity identities[SESSION_ROOTS];
    size_t count, references;
} Session;
struct VMCoverageRun {
    Session *session;
    VMCoverage *collector;
    char id[SESSION_ID];
    size_t identity, children;
    bool root;
};
static bool number(FILE *f, size_t v)
{
    uint8_t b[4] = {(uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v};
    return fwrite(b, 1, 4, f) == 4;
}
static bool read_number(FILE *f, size_t *v)
{
    uint8_t b[4];
    if (fread(b, 1, 4, f) != 4) return false;
    *v = ((size_t)b[0] << 24) | ((size_t)b[1] << 16) | ((size_t)b[2] << 8) | b[3];
    return true;
}
static bool read_blob(FILE *f, uint8_t **data, size_t *size)
{
    if (!read_number(f, size) || *size > VM_COVERAGE_MAX_RAW) return false;
    *data = malloc(*size ? *size : 1);
    return *data && fread(*data, 1, *size, f) == *size;
}
#ifndef __wasi__
static bool read_path(const char *path, uint8_t **data, size_t *size)
{
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    bool ok = !fseek(f, 0, SEEK_END);
    long end = ok ? ftell(f) : -1;
    ok = end >= 0 && (unsigned long)end <= VM_COVERAGE_MAX_RAW && !fseek(f, 0, SEEK_SET);
    if (ok) {
        *size = (size_t)end;
        *data = malloc(*size ? *size : 1);
        ok = *data && fread(*data, 1, *size, f) == *size;
    }
    fclose(f);
    return ok;
}
#endif
static FILE *member(Session *s, const char *prefix, const char *id, const char *mode)
{
    char path[SESSION_PATH + SESSION_ID + 32];
    int n = snprintf(path, sizeof(path), "%s/%s%s", s->directory, prefix, id);
    return n > 0 && (size_t)n < sizeof(path) ? fopen(path, mode) : NULL;
}
/* A shared reservation budget bounds the complete process tree. The lock is
 * held only during this small update; no VM execution or subprocess wait occurs
 * under it. Closing the stream releases the process-owned POSIX lock. */
static bool budget(Session *s, size_t executions, size_t bytes)
{
#ifdef __wasi__
    (void)s; (void)executions; (void)bytes;
    return false;
#else
    FILE *f = member(s, "budget", "", "r+b");
    if (!f) return false;
    struct flock lock = {.l_type = F_WRLCK, .l_whence = SEEK_SET};
    size_t count = 0, size = 0;
    bool ok = fcntl(fileno(f), F_SETLKW, &lock) == 0 &&
        read_number(f, &count) && read_number(f, &size) && count <= SESSION_CHILDREN &&
        executions <= SESSION_CHILDREN - count && size <= 268435456 && bytes <= 268435456 - size;
    if (ok) ok = !fseek(f, 0, SEEK_SET) && number(f, count + executions) && number(f, size + bytes);
    if (fclose(f)) ok = false;
    return ok;
#endif
}
static void drop(Session *s)
{
    if (!s || --s->references) return;
    for (size_t i = 0; i < s->count; i++) { free(s->identities[i].artifact); free(s->identities[i].inventory); }
    free(s);
}
static bool marker(Session *s, const char *prefix, const char *id, const char *kind)
{
    FILE *f = member(s, prefix, id, "wbx");
    if (!f) return false;
    bool ok = fwrite(kind, 1, strlen(kind), f) == strlen(kind);
    if (fclose(f)) ok = false;
    return ok;
}
static bool valid_id(const char *id)
{
    size_t n = strlen(id);
    if (!n || n >= SESSION_ID || id[0] != '0') return false;
    if (n == 1) return true;
    if (id[1] != '.' || n == 2) return false;
    bool start = true;
    for (size_t i = 2; i < n; i++) {
        if (id[i] == '.') {
            if (start) return false;
            start = true;
        } else {
            if (id[i] < '0' || id[i] > '9' || (start && id[i] == '0')) return false;
            start = false;
        }
    }
    if (start) return false;
    return true;
}
static VMCoverageRun *claim(Session *s, const char *id, size_t identity, Program *program)
{
    if (identity >= s->count || !marker(s, "claim-", id, "claimed\n")) return NULL;
    VMCoverageRun *run = calloc(1, sizeof(*run));
    if (!run) return NULL;
    run->collector = vm_coverage_create(program, VM_COVERAGE_MAX_CELLS, UINT64_MAX);
    size_t size;
    Identity *source = &s->identities[identity];
    if (!run->collector || !vm_coverage_raw_size(run->collector, source->artifact_size, source->inventory_size, &size) || size > VM_COVERAGE_MAX_RAW - 40 - strlen(id)) {
        vm_coverage_destroy(run->collector); free(run); return NULL;
    }
    if (!budget(s, 0, size + 40 + strlen(id))) {
        vm_coverage_destroy(run->collector); free(run); return NULL;
    }
    run->session = s; s->references++;
    run->identity = identity;
    memcpy(run->id, id, strlen(id) + 1);
    return run;
}
static bool reserve(VMCoverageRun *parent, const char *kind, char *id)
{
    if (!parent) return false;
    if (parent->children >= SESSION_CHILDREN) { vm_coverage_gap(parent->collector); return false; }
    int n = snprintf(id, SESSION_ID, "%s.%zu", parent->id, ++parent->children);
    if (n <= 0 || n >= SESSION_ID || !budget(parent->session, 1, 0) || !marker(parent->session, "expect-", id, kind)) {
        vm_coverage_gap(parent->collector); return false;
    }
    return true;
}
bool vm_coverage_run_matches(VMCoverageRun *run, const uint8_t *data, size_t length)
{
    Identity *source = &run->session->identities[run->identity];
    return source->artifact_size == length && !memcmp(source->artifact, data, length);
}
VMCoverage *vm_coverage_run_collector(VMCoverageRun *run) { return run ? run->collector : NULL; }
VMCoverageRun *vm_coverage_child(VMCoverageRun *parent, Program *program,
                                const uint8_t *artifact, size_t length, const char *kind)
{
    if (!parent) return NULL;
    char id[SESSION_ID];
    if (!reserve(parent, kind, id)) return NULL;
    size_t identity = parent->identity;
    if (artifact) {
        identity = parent->session->count;
        for (size_t i = 0; i < parent->session->count; i++) {
            Identity *source = &parent->session->identities[i];
            if (source->artifact_size == length && !memcmp(source->artifact, artifact, length)) identity = i;
        }
    }
    VMCoverageRun *run = claim(parent->session, id, identity, program);
    if (!run) vm_coverage_gap(parent->collector);
    return run;
}
char *vm_coverage_process_ticket(VMCoverageRun *parent)
{
    char id[SESSION_ID];
    if (!reserve(parent, "process\n", id)) return NULL;
    size_t size = strlen(parent->session->directory) + strlen(id) + 2;
    char *ticket = malloc(size);
    if (ticket) snprintf(ticket, size, "%s/%s", parent->session->directory, id);
    else vm_coverage_gap(parent->collector);
    return ticket;
}
VMCoverageRun *vm_coverage_session_create(const char *directory, size_t count,
                                         const char **artifacts, const char **inventories, Program *program)
{
#ifdef __wasi__
    (void)directory; (void)count; (void)artifacts; (void)inventories; (void)program;
    return NULL;
#else
    if (!count || count > SESSION_ROOTS) return NULL;
    Session *s = calloc(1, sizeof(*s));
    if (!s) return NULL;
    s->references = 1; s->count = count;
    int n;
    if (directory[0] == '/') n = snprintf(s->directory, SESSION_PATH, "%s", directory);
    else {
        char cwd[SESSION_PATH];
        if (!getcwd(cwd, sizeof(cwd))) { drop(s); return NULL; }
        n = snprintf(s->directory, SESSION_PATH, "%s/%s", cwd, directory);
    }
    bool ok = n > 0 && n < SESSION_PATH;
    size_t total = 0;
    for (size_t i = 0; i < count && ok; i++) {
        Identity *source = &s->identities[i];
        ok = read_path(artifacts[i], &source->artifact, &source->artifact_size) &&
            read_path(inventories[i], &source->inventory, &source->inventory_size);
        total += source->artifact_size + source->inventory_size;
        ok = ok && total <= VM_COVERAGE_MAX_RAW - 35 - 8 * s->count;
        for (size_t j = 0; j < i && ok; j++) {
            Identity *prior = &s->identities[j];
            ok = prior->artifact_size != source->artifact_size || memcmp(prior->artifact, source->artifact, source->artifact_size) != 0;
        }
    }
    FILE *random = ok ? fopen("/dev/urandom", "rb") : NULL;
    ok = random && fread(s->nonce, 1, 16, random) == 16;
    if (random) fclose(random);
    if (ok) ok = mkdir(s->directory, 0700) == 0;
    FILE *f = ok ? member(s, "registry", "", "wbx") : NULL;
    ok = f && fwrite("PANACKSESSION1\n", 1, 15, f) == 15 && fwrite(s->nonce, 1, 16, f) == 16 && number(f, count);
    for (size_t i = 0; i < count && ok; i++) {
        Identity *source = &s->identities[i];
        ok = number(f, source->artifact_size) && fwrite(source->artifact, 1, source->artifact_size, f) == source->artifact_size &&
            number(f, source->inventory_size) && fwrite(source->inventory, 1, source->inventory_size, f) == source->inventory_size;
    }
    if (f && fclose(f)) ok = false;
    FILE *limit = ok ? member(s, "budget", "", "wbx") : NULL;
    ok = limit && number(limit, 1) && number(limit, 0);
    if (limit && fclose(limit)) ok = false;
    VMCoverageRun *run = ok && marker(s, "expect-", "0", "root\n") ? claim(s, "0", 0, program) : NULL;
    if (run) run->root = true;
    drop(s);
    return run;
#endif
}
VMCoverageRun *vm_coverage_session_inherit(const char *ticket, const uint8_t *artifact, size_t length, Program *program)
{
    const char *slash = strrchr(ticket, '/');
    if (!slash || (size_t)(slash - ticket) >= SESSION_PATH || !valid_id(slash + 1)) return NULL;
    Session *s = calloc(1, sizeof(*s));
    if (!s) return NULL;
    s->references = 1;
    memcpy(s->directory, ticket, (size_t)(slash - ticket));
    FILE *f = member(s, "registry", "", "rb");
    uint8_t magic[15];
    bool ok = f && fread(magic, 1, 15, f) == 15 && !memcmp(magic, "PANACKSESSION1\n", 15) &&
        fread(s->nonce, 1, 16, f) == 16 && read_number(f, &s->count) && s->count && s->count <= SESSION_ROOTS;
    if (!ok) s->count = 0;
    size_t total = 0, identity = SESSION_ROOTS;
    for (size_t i = 0; i < s->count && ok; i++) {
        Identity *source = &s->identities[i];
        ok = read_blob(f, &source->artifact, &source->artifact_size) && read_blob(f, &source->inventory, &source->inventory_size);
        total += source->artifact_size + source->inventory_size;
        ok = ok && total <= VM_COVERAGE_MAX_RAW - 35 - 8 * s->count;
        if (ok && length == source->artifact_size && !memcmp(artifact, source->artifact, length)) identity = i;
    }
    if (ok) ok = fgetc(f) == EOF && !ferror(f);
    if (f) fclose(f);
    f = ok ? member(s, "expect-", slash + 1, "rb") : NULL;
    char expected[9] = {0};
    ok = f && fread(expected, 1, 9, f) == 8 && !memcmp(expected, "process\n", 8);
    if (f) fclose(f);
    VMCoverageRun *run = ok ? claim(s, slash + 1, identity, program) : NULL;
    drop(s);
    return run;
}
bool vm_coverage_run_close(VMCoverageRun *run)
{
    if (!run) return true;
    Identity *source = &run->session->identities[run->identity];
    FILE *f = member(run->session, "raw-", run->id, "wbx");
    bool ok = f && fwrite("PANACKEXEC1\n", 1, 12, f) == 12 && fwrite(run->session->nonce, 1, 16, f) == 16 &&
        number(f, strlen(run->id)) && fwrite(run->id, 1, strlen(run->id), f) == strlen(run->id) && number(f, run->identity) && number(f, run->children) &&
        vm_coverage_write(run->collector, f, source->artifact, source->artifact_size, source->inventory, source->inventory_size);
    if (f && fclose(f)) ok = false;
    if (run->root && ok) ok = marker(run->session, "closed", "", "closed\n");
    vm_coverage_destroy(run->collector); drop(run->session); free(run);
    return ok;
}
