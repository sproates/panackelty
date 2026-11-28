#include "fault_injection.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "console_read.h"
#include "value.h"
#include <assert.h>
#include <string.h>

void console_read_faults(void)
{
    fault_reset(0);
    int saved = dup(STDIN_FILENO);
    assert(saved >= 0);
    Value *limit = value_size(1);
    size_t baseline = fault_live(), tested = 0;
    for (size_t failure = 1; failure < 100; failure++) {
        int descriptors[2];
        assert(pipe(descriptors) == 0 && dup2(descriptors[0], STDIN_FILENO) >= 0);
        close(descriptors[0]);
        assert(write(descriptors[1], "x", 1) == 1); close(descriptors[1]);
        VM vm = {0};
        fault_reset(failure);
        VMConsoleRead *operation = console_read_start(&vm, limit);
        if (operation) {
            console_read_wait(operation);
            Value *result = console_read_result(operation);
            release(result);
            // Deliberately do not acknowledge: even a failed result allocation
            // must leave exactly the original byte available for recovery.
            console_read_destroy(operation);
        }
        bool triggered = fault_triggered();
        fault_reset(0);
        Value *recovered = console_input_call(&vm, "console_input", &limit);
        assert(recovered && !strcmp(recovered->as.named.name, "Ok"));
        Value *option = recovered->as.named.values[0];
        assert(!strcmp(option->as.named.name, "Some"));
        Value *bytes = option->as.named.values[0];
        assert(bytes->as.bytes.length == 1 && bytes->as.bytes.data[0] == 'x');
        release(recovered);
        recovered = console_input_call(&vm, "console_input", &limit);
        assert(recovered && !strcmp(recovered->as.named.values[0]->as.named.name, "None"));
        release(recovered);
        assert(!console_read_active_count() && fault_live() == baseline && fault_descriptors() == 0);
        tested++;
        if (!triggered) break;
    }
    assert(tested > 8 && tested < 100);
    release(limit);
    assert(dup2(saved, STDIN_FILENO) >= 0); close(saved);
}
