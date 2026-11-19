#include "platform.h"
#include "filesystem_stage.h"
#include "gzip_stream.h"
#include "host_types.h"
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

static Value *path(const char *p) { return value_data(V_PATH, (const uint8_t *)p, strlen(p)); }
static Value *payload(Value *result, const char *variant)
{
    assert(result && result->kind == V_VARIANT && !strcmp(result->as.named.name, variant));
    assert(result->as.named.count == 1); return result->as.named.values[0];
}
static size_t natural(Value *value) { size_t n; assert(pn_big_fits_size(&value->as.integer, &n)); return n; }
static size_t begin(VM *vm, const char *root)
{
    Value *p = path(root), *args[] = {p}; Value *r = filesystem_stage_call(vm, "fs_stage_begin", args);
    size_t handle = natural(payload(r, "Ok")); release(r); release(p); return handle;
}
static void operation(VM *vm, const char *name, size_t handle, const char *relative, bool ok)
{
    Value *args[] = {value_size(handle), path(relative), value_bool(true)};
    Value *r = filesystem_stage_call(vm, name, args); payload(r, ok ? "Ok" : "Error"); release(r);
    for (size_t i=0;i<3;i++) release(args[i]);
}
static void write_chunk(VM *vm, size_t handle, const char *relative, bool ok)
{
    Value *args[] = {value_size(handle), path(relative), value_data(V_BYTES,(const uint8_t *)"hello",5),value_size(0)};
    Value *r=filesystem_stage_call(vm,"fs_stage_write",args);payload(r,ok?"Ok":"Error");release(r);
    for(size_t i=0;i<4;i++)release(args[i]);
}

static void link_operation(VM *vm, size_t handle, const char *relative, const char *target, bool ok)
{
    Value *args[] = {value_size(handle), path(relative), path(target)};
    Value *result = filesystem_stage_call(vm, "fs_stage_symlink", args);
    payload(result, ok ? "Ok" : "Error");
    release(result);
    for (size_t i = 0; i < 3; i++) {
        release(args[i]);
    }
}

static void read_link(VM *vm, const char *root, const char *relative, const char *expected)
{
    Value *args[] = {path(root), path(relative)};
    Value *result = filesystem_stage_call(vm, "fs_read_link", args);
    Value *value = payload(result, expected ? "Ok" : "Error");
    if (expected) {
        assert(value->kind == V_BYTES && value->as.bytes.length == strlen(expected));
        assert(!memcmp(value->as.bytes.data, expected, strlen(expected)));
    }
    release(result);
    release(args[0]);
    release(args[1]);
}

static void staged_links_preserve_bytes_and_confine_destinations(const char *root)
{
    VM vm = {0};
    size_t handle = begin(&vm, root);
    operation(&vm, "fs_stage_directory", handle, "nested", true);
    write_chunk(&vm, handle, "nested/data", true);
    link_operation(&vm, handle, "nested/link", "data", true);
    link_operation(&vm, handle, "dangling", "absent", true);
    link_operation(&vm, handle, "outside", "/outside/not-opened", true);
    link_operation(&vm, handle, "raw", "\xff", true);
    link_operation(&vm, handle, "parent", "nested", true);
    link_operation(&vm, handle, "nested/link", "different", false);
    link_operation(&vm, handle, "../escaped", "data", false);
    link_operation(&vm, handle, "nested/../../escaped", "data", false);
    link_operation(&vm, handle, "parent/escaped", "data", false);
    link_operation(&vm, handle, "/escaped", "data", false);
    write_chunk(&vm, handle, "parent/escaped", false);
    Value *arg = value_size(handle), *args[] = {arg};
    Value *result = filesystem_stage_call(&vm, "fs_stage_path", args);
    Value *value = payload(result, "Ok");
    char *stage = strndup((char *)value->as.bytes.data, value->as.bytes.length);
    assert(stage);
    release(result);
    release(arg);
    read_link(&vm, stage, "nested/link", "data");
    read_link(&vm, stage, "dangling", "absent");
    read_link(&vm, stage, "outside", "/outside/not-opened");
    read_link(&vm, stage, "raw", "\xff");
    read_link(&vm, stage, "nested/data", NULL);
    read_link(&vm, stage, "parent/link", NULL);
    read_link(&vm, stage, "../outside", NULL);
    read_link(&vm, stage, "/outside", NULL);
    operation(&vm, "fs_stage_close", handle, "ignored", true);
    link_operation(&vm, handle, "closed", "data", false);
    assert(access(stage, F_OK) != 0);
    free(stage);
    filesystem_stage_destroy(&vm);
}

static void failed_discard_retains_ownership(const char *root)
{
    VM vm = {0};
    size_t handle = begin(&vm, root);
    write_chunk(&vm, handle, "private", true);
    Value *args[] = {value_size(handle)};
    Value *result = filesystem_stage_call(&vm, "fs_stage_path", args);
    char *original = strdup((char *)payload(result, "Ok")->as.bytes.data);
    assert(original);
    release(result);
    char moved[8192];
    snprintf(moved, sizeof(moved), "%s-held", original);
    assert(rename(original, moved) == 0 && mkdir(original, 0700) == 0);
    result = filesystem_stage_call(&vm, "fs_stage_close", args);
    Value *error = payload(result, "Error");
    assert(!strcmp((char *)error->as.named.values[1]->as.bytes.data, "cleanup_failed"));
    release(result);
    /* A substituted directory must survive, and the original remains owned. */
    assert(rmdir(original) == 0 && rename(moved, original) == 0);
    write_chunk(&vm, handle, "still-owned", true);
    operation(&vm, "fs_stage_close", handle, "ignored", true);
    assert(access(original, F_OK) != 0);
    operation(&vm, "fs_stage_close", handle, "ignored", false);
    release(args[0]);
    free(original);
    filesystem_stage_destroy(&vm);
}

static void published_cleanup_failure(const char *root)
{
    /* Root can bypass mode bits; the identity-substitution test above runs for
     * every user, while this permission failure requires an ordinary user. */
    if (geteuid() == 0) return;
    VM vm = {0};
    char target[4096];
    snprintf(target, sizeof(target), "%s/locked-old", root);
    assert(mkdir(target, 0700) == 0);
    size_t handle = begin(&vm, root);
    write_chunk(&vm, handle, "published", true);
    Value *args[] = {value_size(handle)};
    Value *result = filesystem_stage_call(&vm, "fs_stage_path", args);
    char *old = strdup((char *)payload(result, "Ok")->as.bytes.data);
    assert(old);
    release(result);
    release(args[0]);
    char locked[8192];
    snprintf(locked, sizeof(locked), "%s/locked", target);
    assert(mkdir(locked, 0) == 0);
    FILE *diagnostic = tmpfile();
    int saved = dup(STDERR_FILENO);
    assert(diagnostic && saved >= 0);
    assert(fflush(stderr) == 0 && dup2(fileno(diagnostic), STDERR_FILENO) >= 0);
    operation(&vm, "fs_stage_publish", handle, "locked-old", true);
    assert(fflush(stderr) == 0 && dup2(saved, STDERR_FILENO) >= 0);
    assert(close(saved) == 0);
    rewind(diagnostic);
    char message[8192] = {0};
    assert(fread(message, 1, sizeof(message) - 1, diagnostic) > 0);
    assert(strstr(message, old) && strstr(message, "errno"));
    assert(fclose(diagnostic) == 0);
    operation(&vm, "fs_stage_path", handle, "ignored", false);
    snprintf(locked, sizeof(locked), "%s/locked", old);
    assert(chmod(locked, 0700) == 0);
    filesystem_stage_destroy(&vm);
    assert(access(old, F_OK) != 0);
    char file[8192];
    snprintf(file, sizeof(file), "%s/published", target);
    assert(access(file, F_OK) == 0 && unlink(file) == 0 && rmdir(target) == 0);
    free(old);
}

static void opaque_stage_identity(const char *root)
{
    VM first = {0}, second = {0};
    Value *parent[] = {path(root)};
    Value *created = filesystem_stage_resource_call(&first, "stage_create", parent);
    Value *handle = retain(payload(created, "Ok"));
    assert(handle->kind == V_DIRECTORY_STAGE);
    release(created);
    Value *args[] = {handle};
    Value *result = filesystem_stage_resource_call(&first, "stage_path", args);
    payload(result, "Ok");
    release(result);
    created = filesystem_stage_resource_call(&second, "stage_create", parent);
    Value *other = retain(payload(created, "Ok"));
    release(created);
    result = filesystem_stage_resource_call(&second, "stage_path", args);
    payload(result, "Error");
    release(result);
    Value *forged[] = {value_size(1)};
    result = filesystem_stage_resource_call(&first, "stage_path", forged);
    payload(result, "Error");
    release(result);
    release(forged[0]);
    /* Keep the value alive across table destruction and recreation. */
    filesystem_stage_destroy(&first);
    created = filesystem_stage_resource_call(&first, "stage_create", parent);
    release(created);
    result = filesystem_stage_resource_call(&first, "stage_discard", args);
    payload(result, "Error");
    release(result);
    release(handle);
    Value *owned[] = {other};
    Value *alias = retain(other);
    result = filesystem_stage_resource_call(&second, "stage_discard", owned);
    payload(result, "Ok");
    release(result);
    owned[0] = alias;
    result = filesystem_stage_resource_call(&second, "stage_path", owned);
    payload(result, "Error");
    release(result);
    release(alias);
    release(other);
    release(parent[0]);
    filesystem_stage_destroy(&first);
    filesystem_stage_destroy(&second);
}

static void *leased_stage_worker(void *context)
{
    VMStageLease *lease = context;
    /* All values here belong to this thread, not the cancelled execution. */
    Value *args[] = {path("worker-finished")};
    Value *result = filesystem_stage_leased_call(lease, "fs_stage_directory", args);
    payload(result, "Ok");
    release(result);
    release(args[0]);
    filesystem_stage_lease_release(lease);
    return NULL;
}

static void stage_lease_survives_vm_destroy(const char *root)
{
    VM vm = {0};
    Value *parent[] = {path(root)};
    Value *created = filesystem_stage_resource_call(&vm, "stage_create", parent);
    Value *handle = retain(payload(created, "Ok"));
    release(created);
    release(parent[0]);
    Value *args[] = {handle};
    Value *result = filesystem_stage_resource_call(&vm, "stage_path", args);
    char *directory = strdup((char *)payload(result, "Ok")->as.bytes.data);
    assert(directory);
    release(result);
    Value *failure = NULL;
    VMStageLease *lease = filesystem_stage_acquire(&vm, handle, &failure);
    assert(lease && !failure);
    assert(!filesystem_stage_acquire(&vm, handle, &failure));
    Value *error = payload(failure, "Error");
    assert(!strcmp((char *)error->as.named.values[1]->as.bytes.data, "busy"));
    release(failure);
    result = filesystem_stage_resource_call(&vm, "stage_discard", args);
    error = payload(result, "Error");
    assert(!strcmp((char *)error->as.named.values[1]->as.bytes.data, "busy"));
    release(result);
    filesystem_stage_destroy(&vm);
    release(handle);
    assert(access(directory, F_OK) == 0);
    /* Start only after the owner and all source values are gone: no timing race
     * is needed to prove the worker has independent native ownership. */
    pthread_t worker;
    assert(pthread_create(&worker, NULL, leased_stage_worker, lease) == 0);
    assert(pthread_join(worker, NULL) == 0);
    assert(access(directory, F_OK) != 0);
    free(directory);
}

static void stage_table_admission(const char *root)
{
    VM owners[33] = {0};
    Value *args[] = {path(root)};
    for (size_t i = 0; i < 32; i++) {
        Value *result = filesystem_stage_resource_call(&owners[i], "stage_create", args);
        payload(result, "Ok");
        release(result);
    }
    Value *result = filesystem_stage_resource_call(&owners[32], "stage_create", args);
    Value *error = payload(result, "Error");
    assert(!strcmp((char *)error->as.named.values[1]->as.bytes.data, "resource_limit"));
    release(result);
    filesystem_stage_destroy_deferred(&owners[0]);
    struct timespec delay = {.tv_nsec = 1000000};
    for (size_t i = 0; i < 5000 && filesystem_stage_pending_cleanup(); i++) nanosleep(&delay, NULL);
    assert(!filesystem_stage_pending_cleanup());
    result = filesystem_stage_resource_call(&owners[32], "stage_create", args);
    payload(result, "Ok");
    release(result);
    for (size_t i = 1; i < 33; i++) filesystem_stage_destroy_deferred(&owners[i]);
    for (size_t i = 0; i < 5000 && filesystem_stage_pending_cleanup(); i++) nanosleep(&delay, NULL);
    assert(!filesystem_stage_pending_cleanup());
    release(args[0]);
}

static void staging_lifetime(const char *root)
{
    VM vm = {0};
    size_t handle = begin(&vm, root);
    write_chunk(&vm, handle, "payload", true);
    char destination[4096];
    snprintf(destination, sizeof(destination), "%s/occupied", root);
    assert(mkdir(destination, 0700) == 0);
    Value *args[] = {value_size(handle), path("occupied"), value_bool(false)};
    Value *result = filesystem_stage_call(&vm, "fs_stage_publish", args);
    Value *error = payload(result, "Error");
    assert(!strcmp((char *)error->as.named.values[1]->as.bytes.data, "destination_conflict"));
    release(result);
    /* A rejected publication leaves the stage usable, with its contents intact. */
    operation(&vm, "fs_stage_path", handle, "ignored", true);
    write_chunk(&vm, handle, "after-rejection", true);
    assert(rmdir(destination) == 0);
    result = filesystem_stage_call(&vm, "fs_stage_publish", args);
    payload(result, "Ok");
    release(result);
    for (size_t i = 0; i < 3; i++) release(args[i]);
    operation(&vm, "fs_stage_path", handle, "ignored", false);
    write_chunk(&vm, handle, "after-publication", false);
    operation(&vm, "fs_stage_publish", handle, "second", false);
    size_t unpublished = begin(&vm, root);
    write_chunk(&vm, unpublished, "private", true);
    Value *pending[] = {value_size(unpublished)};
    result = filesystem_stage_call(&vm, "fs_stage_path", pending);
    char *unpublished_path = strdup((char *)payload(result, "Ok")->as.bytes.data);
    assert(unpublished_path);
    release(result);
    release(pending[0]);
    filesystem_stage_destroy(&vm);
    assert(access(unpublished_path, F_OK) != 0);
    free(unpublished_path);
    /* VM shutdown must never remove a successfully published directory. */
    char file[8192];
    snprintf(file, sizeof(file), "%s/payload", destination);
    assert(access(file, F_OK) == 0 && unlink(file) == 0);
    snprintf(file, sizeof(file), "%s/after-rejection", destination);
    assert(access(file, F_OK) == 0 && unlink(file) == 0);
    assert(rmdir(destination) == 0);
}

int main(void)
{
    char temporary[] = "/tmp/panack-stage-native-XXXXXX";
    assert(mkdtemp(temporary)); char *root = realpath(temporary, NULL); assert(root);
    staged_links_preserve_bytes_and_confine_destinations(root);
    staging_lifetime(root);
    stage_table_admission(root);
    stage_lease_survives_vm_destroy(root);
    opaque_stage_identity(root);
    failed_discard_retains_ownership(root);
    published_cleanup_failure(root);
    VM vm={0}; size_t handle=begin(&vm,root);
    operation(&vm,"fs_stage_directory",handle,"nested",true);
    write_chunk(&vm,handle,"nested/main.panack",true);
    write_chunk(&vm,handle,"../escape",false);
    write_chunk(&vm,handle,"nested/main.panack",false);
    operation(&vm,"fs_stage_publish",handle,"cache",true);
    operation(&vm,"fs_stage_close",handle,"ignored",false);
    size_t replacement=begin(&vm,root);
    write_chunk(&vm,replacement,"new",true);
    operation(&vm,"fs_stage_publish",replacement,"cache",true);
    char file[4096]; snprintf(file,sizeof(file),"%s/cache/new",root);assert(access(file,F_OK)==0);
    snprintf(file,sizeof(file),"%s/cache/nested",root);assert(access(file,F_OK)!=0);
    size_t cancelled = begin(&vm, root);
    write_chunk(&vm, cancelled, "partial", true);
    operation(&vm, "fs_stage_close", cancelled, "ignored", true);
    operation(&vm, "fs_stage_publish", cancelled, "cache", false);
    snprintf(file, sizeof(file), "%s/cache/new", root);
    assert(access(file, F_OK) == 0);
    snprintf(file, sizeof(file), "%s/cache/partial", root);
    assert(access(file, F_OK) != 0);
    snprintf(file,sizeof(file),"%s/link",root);assert(!symlink("cache",file));
    size_t rejected=begin(&vm,root);
    write_chunk(&vm,rejected,"untouched",true);
    operation(&vm,"fs_stage_publish",rejected,"link",false);
    operation(&vm,"fs_stage_close",rejected,"ignored",true);
    assert(!unlink(file));
    snprintf(file,sizeof(file),"%s/cache/new",root);assert(!unlink(file));
    snprintf(file,sizeof(file),"%s/cache",root);assert(!rmdir(file));
    /* A retained directory descriptor must not publish through a path whose
     * parent was replaced after staging began. Cleanup still uses the held fd. */
    size_t moved = begin(&vm, root);
    write_chunk(&vm, moved, "private", true);
    char renamed[4096];
    snprintf(renamed, sizeof(renamed), "%s-moved", root);
    assert(!rename(root, renamed));
    assert(!mkdir(root, 0700));
    operation(&vm, "fs_stage_publish", moved, "cache", false);
    operation(&vm, "fs_stage_close", moved, "ignored", true);
    assert(!rmdir(renamed));
    filesystem_stage_destroy(&vm);assert(!rmdir(root));free(root);

    uint8_t compressed[128];z_stream encoder={0};assert(deflateInit2(&encoder,6,Z_DEFLATED,31,8,Z_DEFAULT_STRATEGY)==Z_OK);
    encoder.next_in=(uint8_t *)"hello";encoder.avail_in=5;encoder.next_out=compressed;encoder.avail_out=sizeof(compressed);
    assert(deflate(&encoder,Z_FINISH)==Z_STREAM_END);size_t length=sizeof(compressed)-encoder.avail_out;deflateEnd(&encoder);
    for(size_t mode=0;mode<4;mode++) {
        if (mode == 3) compressed[length - 8] ^= 1; /* Corrupt the CRC32 trailer. */
        Value *limit=value_size(mode==2?4:100);Value *open_args[]={limit};Value *opened=gzip_stream_call(&vm,"gzip_open",open_args);
        Value *h=payload(opened,"Ok");
        Value *data=value_data(V_BYTES,compressed,mode==1?length-1:length),*finish=value_bool(true);
        Value *args[]={h,data,finish};Value *r=gzip_stream_call(&vm,"gzip_step",args);
        payload(r,mode==0?"Ok":"Error");release(r);release(data);release(finish);release(opened);release(limit);
    }
    compressed[length - 8] ^= 1;
    gzip_stream_destroy(&vm);
    /* Concatenated gzip members share one decoder and one output budget. */
    uint8_t members[256];
    memcpy(members, compressed, length);
    memcpy(members + length, compressed, length);
    Value *limit = value_size(10), *open_args[] = {limit};
    Value *opened = gzip_stream_call(&vm, "gzip_open", open_args);
    Value *decoder = payload(opened, "Ok");
    size_t consumed = 0, produced = 0;
    bool finished = false;
    for (size_t step = 0; step < 8 && !finished; step++) {
        Value *data = value_data(V_BYTES, members + consumed, 2 * length - consumed);
        Value *finish = value_bool(true), *args[] = {decoder, data, finish};
        Value *result = gzip_stream_call(&vm, "gzip_step", args);
        Value *chunk = payload(result, "Ok");
        assert(chunk->kind == V_RECORD && chunk->as.named.count == 3);
        consumed += natural(chunk->as.named.values[0]);
        produced += chunk->as.named.values[1]->as.bytes.length;
        finished = chunk->as.named.values[2]->as.boolean;
        release(result);
        release(data);
        release(finish);
    }
    assert(finished && consumed == 2 * length && produced == 10);
    release(opened);
    release(limit);
    gzip_stream_destroy(&vm);
    puts("PASS confined staging, atomic replacement, symlink rejection, consumed handles, gzip trailer and output limits");
    return 0;
}
