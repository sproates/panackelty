#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "platform.h"
#include "filesystem_stage.h"
#include "builtins_internal.h"
#include "host_types.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef __EMSCRIPTEN__
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/syscall.h>
#endif
/* Handles belong to one VM invocation. Slots are never reused, so a closed
 * handle cannot acquire a later stage. No package or archive policy lives here. */
#define STAGE_LIMIT 64
#define CHUNK_LIMIT 65536
struct Stage { int parent, directory; char *parent_path; char name[64]; bool live; };
struct VMFilesystemStages { struct Stage slots[STAGE_LIMIT]; size_t count; };
#endif

static Value *stage_error(const char *operation, const char *code, int error)
{
    char *fields[] = {"operation", "code", "native_code"};
    Value *values[] = {value_data(V_STR, (const uint8_t *)operation, strlen(operation)),
        value_data(V_STR, (const uint8_t *)code, strlen(code)), value_size(error < 0 ? 0 : (size_t)error)};
    Value *record = values[0] && values[1] && values[2] ? named_value(V_RECORD, "HostError", fields, values, 3) : NULL;
    for (size_t i = 0; i < 3; i++) release(values[i]);
    return host_variant("Error", record);
}
#ifndef __EMSCRIPTEN__
static Value *stage_ok(void) { return host_variant("Ok", value_new(V_UNIT)); }
static char *stage_path(Value *value)
{
    if (value->kind != V_PATH || !value->as.bytes.length || value->as.bytes.length > 4096 ||
        memchr(value->as.bytes.data, 0, value->as.bytes.length)) { errno = EINVAL; return NULL; }
    char *text = malloc(value->as.bytes.length + 1);
    if (text) { memcpy(text, value->as.bytes.data, value->as.bytes.length); text[value->as.bytes.length] = 0; }
    return text;
}
/* Open every ancestor, refusing symlinks. Relative '..' is deliberately not
 * accepted by the confined API; callers can canonicalise a trusted parent. */
static int stage_directory(int base, const char *path)
{
    int current = path[0] == '/' ? open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC) : dup(base);
    if (current < 0) return -1;
    char *copy = strdup(path), *save = NULL;
    if (!copy) { close(current); return -1; }
    for (char *part = strtok_r(copy, "/", &save); part; part = strtok_r(NULL, "/", &save)) {
        if (!strcmp(part, ".")) continue;
        if (!strcmp(part, "..")) { errno = EINVAL; close(current); free(copy); return -1; }
        int next = openat(current, part, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        int error = errno; close(current); current = next; errno = error;
        if (current < 0) break;
    }
    free(copy); return current;
}
static int stage_parent(int base, char *path, char **leaf)
{
    if (path[0] == '/' || strchr(path, '\\')) { errno = EINVAL; return -1; }
    char *last = strrchr(path, '/');
    *leaf = last ? last + 1 : path;
    if (!**leaf || !strcmp(*leaf, ".") || !strcmp(*leaf, "..")) { errno = EINVAL; return -1; }
    if (!last) return dup(base);
    *last = 0;
    return stage_directory(base, path);
}
static int stage_remove(int parent, const char *name)
{
    int fd = openat(parent, name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) return -1;
    DIR *directory = fdopendir(fd);
    if (!directory) { close(fd); return -1; }
    int error = 0;
    for (;;) {
        errno = 0; struct dirent *entry = readdir(directory);
        if (!entry) { if (errno) error = errno; break; }
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        struct stat info;
        if (fstatat(fd, entry->d_name, &info, AT_SYMLINK_NOFOLLOW)) { error = errno; break; }
        int result = S_ISDIR(info.st_mode) ? stage_remove(fd, entry->d_name) : unlinkat(fd, entry->d_name, 0);
        if (result) { error = errno; break; }
    }
    closedir(directory);
    if (!error && unlinkat(parent, name, AT_REMOVEDIR)) error = errno;
    errno = error; return error ? -1 : 0;
}
static void stage_release(struct Stage *stage)
{
    if (!stage->live) return;
    if (stage->name[0]) (void)stage_remove(stage->parent, stage->name);
    close(stage->directory); close(stage->parent); free(stage->parent_path);
    stage->live = false;
}
#endif
void filesystem_stage_destroy(VM *vm)
{
#ifndef __EMSCRIPTEN__
    if (!vm->filesystem_stages) return;
    for (size_t i = 0; i < vm->filesystem_stages->count; i++) stage_release(&vm->filesystem_stages->slots[i]);
    free(vm->filesystem_stages); vm->filesystem_stages = NULL;
#else
    (void)vm;
#endif
}
Value *filesystem_stage_call(VM *vm, const char *name, Value **a)
{
#ifdef __EMSCRIPTEN__
    (void)vm; (void)a;
    return stage_error(name, "unsupported", 0);
#else
    if (!strcmp(name, "fs_read_chunk") || !strcmp(name, "fs_atomic_write")) {
        char *full = stage_path(a[0]);
        if (!full) return stage_error(name, "invalid_path", errno);
        char *slash = strrchr(full, '/');
        char *leaf = slash ? slash + 1 : full;
        if (!*leaf || !strcmp(leaf, ".") || !strcmp(leaf, "..")) { free(full); return stage_error(name,"invalid_path",0); }
        char *parent_path = ".";
        if (slash) { *slash = 0; parent_path = slash == full ? "/" : full; }
        int cwd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        int parent = cwd < 0 ? -1 : stage_directory(cwd, parent_path);
        if (cwd >= 0) close(cwd);
        if (parent < 0) { int e=errno; free(full); return stage_error(name,"invalid_parent",e); }
        Value *result = NULL;
        int error = 0;
        if (!strcmp(name,"fs_read_chunk")) {
            size_t offset, limit;
            if (a[1]->kind != V_NAT || a[2]->kind != V_NAT ||
                !pn_big_fits_size(&a[1]->as.integer,&offset) || !pn_big_fits_size(&a[2]->as.integer,&limit) ||
                limit > CHUNK_LIMIT || offset > 256u*1024u*1024u) error=EINVAL;
            else {
                int file=openat(parent,leaf,O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC);
                if(file<0) error=errno;
                else {
                    struct stat info;
                    if(fstat(file,&info)) error=errno;
                    else if(!S_ISREG(info.st_mode)) error=EINVAL;
                    uint8_t buffer[CHUNK_LIMIT]; ssize_t count=-1;
                    if(!error) {
                        do { count=pread(file,buffer,limit,(off_t)offset); } while(count<0 && errno==EINTR);
                        if(count<0)error=errno;
                    }
                    if(close(file)&&!error)error=errno;
                    if(!error)result=host_variant("Ok",value_data(V_BYTES,buffer,(size_t)count));
                }
            }
        } else {
            if(a[1]->kind!=V_BYTES || a[1]->as.bytes.length>32u*1024u*1024u)error=EINVAL;
            char temporary[64];int file=-1;
            for(unsigned attempt=0;!error && attempt<128;attempt++) {
                snprintf(temporary,sizeof(temporary),".panack-write-%ld-%u",(long)getpid(),attempt);
                file=openat(parent,temporary,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
                if(file>=0)break;
                if(errno!=EEXIST)error=errno;
            }
            if(file<0 && !error)error=EEXIST;
            if(file>=0) {
                size_t at=0;
                while(!error && at<a[1]->as.bytes.length) {
                    ssize_t count=write(file,a[1]->as.bytes.data+at,a[1]->as.bytes.length-at);
                    if(count>0)at+=(size_t)count;
                    else if(count<0&&errno==EINTR)continue;
                    else error=count<0?errno:EIO;
                }
                if(close(file)&&!error)error=errno;
                struct stat target;
                int exists=fstatat(parent,leaf,&target,AT_SYMLINK_NOFOLLOW);
                if(!error && ((!exists&&!S_ISREG(target.st_mode)) || (exists&&errno!=ENOENT)))error=EINVAL;
                if(!error&&renameat(parent,temporary,parent,leaf))error=errno;
                if(error)unlinkat(parent,temporary,0);
                else result=stage_ok();
            }
        }
        close(parent);free(full);
        return error?stage_error(name,"filesystem_error",error):result;
    }
    if (!strcmp(name, "fs_stage_begin")) {
        char *path = stage_path(a[0]);
        if (!path) return stage_error(name, "invalid_path", errno);
        int cwd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        int parent = cwd < 0 ? -1 : stage_directory(cwd, path);
        if (cwd >= 0) close(cwd);
        if (parent < 0) { int e = errno; free(path); return stage_error(name, "invalid_parent", e); }
        if (!vm->filesystem_stages) vm->filesystem_stages = calloc(1, sizeof(*vm->filesystem_stages));
        if (!vm->filesystem_stages || vm->filesystem_stages->count == STAGE_LIMIT) {
            close(parent); free(path); return stage_error(name, "resource_limit", 0);
        }
        size_t slot = vm->filesystem_stages->count++;
        struct Stage *stage = &vm->filesystem_stages->slots[slot];
        int created = -1;
        for (unsigned attempt = 0; attempt < 128; attempt++) {
            snprintf(stage->name, sizeof(stage->name), ".panack-stage-%ld-%zu-%u", (long)getpid(), slot, attempt);
            created = mkdirat(parent, stage->name, 0700);
            if (!created || errno != EEXIST) break;
        }
        if (created) { int e = errno; close(parent); free(path); return stage_error(name, "create_failed", e); }
        int directory = openat(parent, stage->name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (directory < 0) { int e = errno; unlinkat(parent, stage->name, AT_REMOVEDIR); close(parent); free(path); return stage_error(name, "open_failed", e); }
        stage->parent = parent; stage->directory = directory; stage->parent_path = path; stage->live = true;
        return host_variant("Ok", value_size(slot + 1));
    }
    size_t handle;
    if (a[0]->kind != V_NAT || !pn_big_fits_size(&a[0]->as.integer, &handle) || !handle ||
        !vm->filesystem_stages || handle > vm->filesystem_stages->count || !vm->filesystem_stages->slots[handle - 1].live)
        return stage_error(name, "invalid_handle", 0);
    struct Stage *stage = &vm->filesystem_stages->slots[handle - 1];
    if (!strcmp(name, "fs_stage_close")) { stage_release(stage); return stage_ok(); }
    if (!strcmp(name, "fs_stage_path")) {
        size_t length = strlen(stage->parent_path) + strlen(stage->name) + 2;
        char *path = malloc(length);
        if (!path) return NULL;
        snprintf(path, length, "%s/%s", stage->parent_path, stage->name);
        Value *result = host_variant("Ok", value_data(V_PATH, (const uint8_t *)path, strlen(path)));
        free(path); return result;
    }
    char *path = stage_path(a[1]), *leaf = NULL;
    if (!path) return stage_error(name, "invalid_path", errno);
    if (!strcmp(name, "fs_stage_publish")) {
        if (strchr(path, '/') || !strcmp(path, ".") || !strcmp(path, "..") || a[2]->kind != V_BOOL) {
            free(path); return stage_error(name, "invalid_argument", 0);
        }
        struct stat source, held, target;
        if (fstat(stage->directory, &held) || fstatat(stage->parent, stage->name, &source, AT_SYMLINK_NOFOLLOW) ||
            held.st_dev != source.st_dev || held.st_ino != source.st_ino) {
            free(path); return stage_error(name, "stage_changed", 0);
        }
        int exists = fstatat(stage->parent, path, &target, AT_SYMLINK_NOFOLLOW);
        if ((exists && errno != ENOENT) || (!exists && (!S_ISDIR(target.st_mode) || !a[2]->as.boolean))) {
            free(path); return stage_error(name, "destination_conflict", errno);
        }
        int result;
#ifdef __APPLE__
        result = renameatx_np(stage->parent, stage->name, stage->parent, path, exists ? RENAME_EXCL : RENAME_SWAP);
#elif defined(__linux__)
        result = (int)syscall(SYS_renameat2, stage->parent, stage->name, stage->parent, path, exists ? 1 : 2);
#else
        errno = ENOTSUP; result = -1;
#endif
        int error = errno; free(path);
        if (result) return stage_error(name, "publication_failed", error);
        /* The exchange commits publication. Cleanup of the old generation is
         * separate; never report a publication failure after a successful swap. */
        if (exists) stage->name[0] = 0;
        stage_release(stage);
        return stage_ok();
    }
    int parent = stage_parent(stage->directory, path, &leaf);
    if (parent < 0) { int e = errno; free(path); return stage_error(name, "invalid_parent", e); }
    int error = 0;
    if (!strcmp(name, "fs_stage_directory")) {
        if (mkdirat(parent, leaf, 0700)) error = errno;
    } else if (!strcmp(name, "fs_stage_write")) {
        size_t offset;
        if (a[2]->kind != V_BYTES || a[2]->as.bytes.length > CHUNK_LIMIT || a[3]->kind != V_NAT ||
            !pn_big_fits_size(&a[3]->as.integer, &offset) || offset > 256u * 1024u * 1024u) error = EINVAL;
        else {
            int flags = O_WRONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK;
            if (!offset) flags |= O_CREAT | O_EXCL;
            int file = openat(parent, leaf, flags, 0600);
            if (file < 0) error = errno;
            else {
                struct stat info;
                if (fstat(file, &info)) error = errno;
                else if (!S_ISREG(info.st_mode) || info.st_nlink != 1 || (size_t)info.st_size != offset) error = EINVAL;
                size_t at = 0;
                while (!error && at < a[2]->as.bytes.length) {
                    ssize_t n = pwrite(file, a[2]->as.bytes.data + at, a[2]->as.bytes.length - at, (off_t)(offset + at));
                    if (n > 0) at += (size_t)n;
                    else if (n < 0 && errno == EINTR) continue;
                    else error = n < 0 ? errno : EIO;
                }
                if (close(file) && !error) error = errno;
            }
        }
    } else error = EINVAL;
    close(parent); free(path);
    return error ? stage_error(name, "filesystem_error", error) : stage_ok();
#endif
}
