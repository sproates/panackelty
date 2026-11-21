#ifndef PANACKELTY_PROCESS_ASYNC_H
#define PANACKELTY_PROCESS_ASYNC_H
#include "vm.h"
typedef struct VMProcess VMProcess;
VMProcess *process_async_start(VM *vm, Value **arguments, bool isolated);
bool process_async_poll(VMProcess *process);
void process_async_wait(VMProcess *process);
Value *process_async_result(VMProcess *process);
const char *process_async_error(VMProcess *process);
void process_async_destroy(VMProcess *process);
size_t process_async_active_count(void);
void process_async_drain(void);
#endif
