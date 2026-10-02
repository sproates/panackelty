/* Test-only observer: use the actual VM dispatch loop and inspect its frame
 * immediately before each instruction. No public VM ABI or production trace.
 * execute.c is compiled here instead of linking execute.o. */
#include "../../../src/vm/execute.c"
#include "decode.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2 && !(argc == 3 && strcmp(argv[2], "--bulk") == 0)) return 2;
    bool single_step = argc == 2;
    FILE *file = fopen(argv[1], "rb");
    if (!file) return 2;
    if (fseek(file, 0, SEEK_END) || ftell(file) < 0) { fclose(file); return 2; }
    long length = ftell(file);
    if (length <= 0 || length > MAX_ARTIFACT) { fclose(file); return 2; }
    rewind(file);
    uint8_t *data = malloc((size_t)length);
    if (!data) { fclose(file); return 2; }
    size_t got = fread(data, 1, (size_t)length, file);
    fclose(file);
    Program program = {0};
    const char *error = NULL;
    bool valid = got == (size_t)length && decode(data, got, &program, &error) && verify(&program, &error);
    free(data);
    if (!valid) { free_program(&program); return 2; }
    VM vm = {.program = &program};
    VMExecution *execution = vm_execution_create(&vm, program_function(&program, "main"), NULL, NULL, NULL, &error);
    if (!execution) { free_program(&program); return 2; }
    int result = 2;
    for (size_t step = 0; step < 100000; step++) {
        if (!execution->frame_count) break;
        Frame *frame = &execution->frames[execution->frame_count - 1];
        Function *function = frame->function;
        size_t pc = frame->pc;
        VMExecutionStatus status = vm_execution_advance(execution, single_step ? 1 : 100000);
        if (status == VM_TRAPPED) {
            /* function belongs to program, not the cleared execution frames. */
            if (single_step) printf("trap\t%s\t%zu\t%s\n", function->name, pc, vm.error);
            else puts("trapped");
            result = 0;
            break;
        }
        if (status == VM_COMPLETED) { puts("completed"); result = 0; break; }
        if (status != VM_YIELDED) break;
    }
    vm_execution_destroy(execution);
    free_program(&program);
    return result;
}
