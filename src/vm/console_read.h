#ifndef PANACKELTY_CONSOLE_READ_H
#define PANACKELTY_CONSOLE_READ_H
#include "vm.h"
typedef struct VMConsoleRead VMConsoleRead;
VMConsoleRead *console_read_start(VM *vm, const Value *limit);
bool console_read_poll(VMConsoleRead *operation);
void console_read_wait(VMConsoleRead *operation);
Value *console_read_result(VMConsoleRead *operation);
void console_read_accept(VMConsoleRead *operation);
void console_read_destroy(VMConsoleRead *operation);
Value *console_input_call(VM *vm, const char *name, Value **arguments);
Value *console_legacy_read_line(VM *vm);
size_t console_read_active_count(void);
#endif
