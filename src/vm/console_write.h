#ifndef PANACKELTY_CONSOLE_WRITE_H
#define PANACKELTY_CONSOLE_WRITE_H

#include "vm.h"

typedef struct VMConsoleWrite VMConsoleWrite;

/* Independently owned, bounded console writes. Cancellation suppresses result
 * delivery; the worker retains its descriptor and slot until write returns. */
VMConsoleWrite *console_write_start(VM *vm, const Value *message, bool standard_error);
bool console_write_poll(VMConsoleWrite *write);
void console_write_wait(VMConsoleWrite *write);
Value *console_write_result(VMConsoleWrite *write);
void console_write_destroy(VMConsoleWrite *write);
size_t console_write_active_count(void);

#endif
