#ifndef PANACKELTY_HTTP_RUNTIME_H
#define PANACKELTY_HTTP_RUNTIME_H
#include "vm.h"

typedef struct VMHttpResource VMHttpResource;
struct VMHttpRuntime {
    VMExecution *owner;
    VMHttpResource *active, *pending;
    const char *error;
    bool exit_requested;
    size_t exit_status;
};

Value *http_runtime_listen(VM *vm, Value *handle, Function *worker, bool contextual,
                           bool outbound, bool filesystem, bool console);
bool http_runtime_request_close(struct VMHttpRuntime *runtime, Value *handle, bool destroy);
bool http_runtime_active(const struct VMHttpRuntime *runtime);
void http_runtime_pump(struct VMHttpRuntime *runtime, size_t budget, unsigned wait_ms);
void http_runtime_stop(struct VMHttpRuntime *runtime);
void http_runtime_destroy(struct VMHttpRuntime *runtime);
#endif
