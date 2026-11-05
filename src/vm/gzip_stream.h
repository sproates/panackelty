#ifndef PANACKELTY_GZIP_STREAM_H
#define PANACKELTY_GZIP_STREAM_H
#include "vm.h"
Value *gzip_stream_call(VM *vm, const char *name, Value **arguments);
void gzip_stream_destroy(VM *vm);
#endif
