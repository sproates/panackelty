#ifndef PANACKELTY_FILESYSTEM_STAGE_H
#define PANACKELTY_FILESYSTEM_STAGE_H
#include "vm.h"
Value *filesystem_stage_call(VM *vm, const char *name, Value **arguments);
void filesystem_stage_destroy(VM *vm);
#endif
