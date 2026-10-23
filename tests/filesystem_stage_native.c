#include "platform.h"
#include "filesystem_stage.h"
#include "gzip_stream.h"
#include "host_types.h"
#include <assert.h>
#include <fcntl.h>
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
int main(void)
{
    char temporary[] = "/tmp/panack-stage-native-XXXXXX";
    assert(mkdtemp(temporary)); char *root = realpath(temporary, NULL); assert(root);
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
    snprintf(file,sizeof(file),"%s/link",root);assert(!symlink("cache",file));
    size_t rejected=begin(&vm,root);
    write_chunk(&vm,rejected,"untouched",true);
    operation(&vm,"fs_stage_publish",rejected,"link",false);
    operation(&vm,"fs_stage_close",rejected,"ignored",true);
    assert(!unlink(file));
    snprintf(file,sizeof(file),"%s/cache/new",root);assert(!unlink(file));
    snprintf(file,sizeof(file),"%s/cache",root);assert(!rmdir(file));
    filesystem_stage_destroy(&vm);assert(!rmdir(root));free(root);

    uint8_t compressed[128];z_stream encoder={0};assert(deflateInit2(&encoder,6,Z_DEFLATED,31,8,Z_DEFAULT_STRATEGY)==Z_OK);
    encoder.next_in=(uint8_t *)"hello";encoder.avail_in=5;encoder.next_out=compressed;encoder.avail_out=sizeof(compressed);
    assert(deflate(&encoder,Z_FINISH)==Z_STREAM_END);size_t length=sizeof(compressed)-encoder.avail_out;deflateEnd(&encoder);
    for(size_t mode=0;mode<3;mode++) {
        Value *limit=value_size(mode==2?4:100);Value *open_args[]={limit};Value *opened=gzip_stream_call(&vm,"gzip_open",open_args);
        Value *h=payload(opened,"Ok");
        Value *data=value_data(V_BYTES,compressed,mode==1?length-1:length),*finish=value_bool(true);
        Value *args[]={h,data,finish};Value *r=gzip_stream_call(&vm,"gzip_step",args);
        payload(r,mode==0?"Ok":"Error");release(r);release(data);release(finish);release(opened);release(limit);
    }
    gzip_stream_destroy(&vm);
    puts("PASS confined staging, atomic replacement, symlink rejection, consumed handles, gzip trailer and output limits");
    return 0;
}
