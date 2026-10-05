/* Test-only counters around the real dispatcher. No production instrumentation.
 * The report is separate from program stdout and names functions by table index. */
#include "../../../src/vm/execute.c"
#include "decode.h"
#include "host.h"
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#define CELL_LIMIT 1000000
#define STEP_LIMIT 10000000

typedef struct {
    uint64_t attempts, yes, no;
} Counter;

static bool nested_call(Frame *frame, Instruction *instruction)
{
    const char *name = NULL;
    if (instruction->op == OP_CALL || instruction->op == OP_AWAIT_CALL) {
        name = instruction->name;
    } else if ((instruction->op == OP_CALL_VALUE || instruction->op == OP_AWAIT_VALUE) &&
               frame->stack_count > instruction->arity) {
        Value *callable = frame->stack[frame->stack_count - instruction->arity - 1];
        if (callable->kind == V_STR &&
            !memchr(callable->as.bytes.data, 0, callable->as.bytes.length)) {
            name = (const char *)callable->as.bytes.data;
        }
    }
    return name && (!strcmp(name, "run_bytecode") || !strcmp(name, "run_bytecode_args"));
}

int main(int argc, char **argv)
{
    if (argc != 4 || (strcmp(argv[3], "count") && strcmp(argv[3], "bulk") &&
                      strcmp(argv[3], "limit"))) {
        return 2;
    }
    bool bulk = !strcmp(argv[3], "bulk");
    size_t budget = !strcmp(argv[3], "limit") ? 2 : STEP_LIMIT;
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        return 2;
    }
    if (fseek(file, 0, SEEK_END) || ftell(file) <= 0 || ftell(file) > MAX_ARTIFACT) {
        fclose(file);
        return 2;
    }
    size_t length = (size_t)ftell(file);
    rewind(file);
    uint8_t *data = malloc(length);
    if (!data) {
        fclose(file);
        return 2;
    }
    size_t got = fread(data, 1, length, file);
    fclose(file);
    Program program = {0};
    const char *error = NULL;
    bool valid = got == length && decode(data, got, &program, &error) && verify(&program, &error);
    free(data);
    if (!valid) {
        free_program(&program);
        return 2;
    }
    size_t cells = 0;
    for (size_t i = 0; i < program.count; i++) {
        if (program.functions[i].ins_count > CELL_LIMIT - cells) {
            free_program(&program);
            return 2;
        }
        cells += program.functions[i].ins_count;
    }
    Counter *counts = calloc(cells ? cells : 1, sizeof(*counts));
    size_t *offsets = calloc(program.count + 1, sizeof(*offsets));
    uint64_t *entries = calloc(program.count ? program.count : 1, sizeof(*entries));
    if (!counts || !offsets || !entries) {
        free(counts);
        free(offsets);
        free(entries);
        free_program(&program);
        return 2;
    }
    for (size_t i = 0; i < program.count; i++) {
        offsets[i + 1] = offsets[i] + program.functions[i].ins_count;
    }
    char **environment = NULL;
    size_t env_count = 0;
    if (!snapshot_environment(&environment, &env_count)) {
        free(counts);
        free(offsets);
        free(entries);
        free_program(&program);
        return 2;
    }
    VM vm = {.program = &program, .environment = environment, .env_count = env_count};
    Function *main_function = program_function(&program, "main");
    VMExecution *execution = vm_execution_create(&vm, main_function, NULL, NULL, NULL, &error);
    if (!execution) {
        free_environment(environment, env_count);
        free(counts);
        free(offsets);
        free(entries);
        free_program(&program);
        return 2;
    }
    /* Match the synchronous CLI's host behavior. Child execute() calls remain
     * unobserved, so explicitly invalidate whole-run completeness at that edge. */
    execution->embedded = false;
    entries[main_function - program.functions] = 1;
    VMExecutionStatus status = VM_YIELDED;
    const char *gap = "none";
    size_t steps = 0, waits = 0;
    clock_t start = clock();
    while (status == VM_YIELDED && steps < budget) {
        size_t depth = execution->frame_count;
        if (!depth) {
            break;
        }
        Frame *frame = &execution->frames[depth - 1];
        size_t index = (size_t)(frame->function - program.functions);
        if (!bulk && frame->pc < frame->function->ins_count) {
            Instruction *instruction = &frame->function->ins[frame->pc];
            Counter *counter = &counts[offsets[index] + frame->pc];
            counter->attempts++;
            if (nested_call(frame, instruction)) {
                gap = "nested-execution";
            }
            if (instruction->op == OP_JUMP_FALSE && frame->stack_count) {
                Value *condition = frame->stack[frame->stack_count - 1];
                if (condition->kind == V_BOOL) {
                    if (condition->as.boolean) {
                        counter->yes++;
                    } else {
                        counter->no++;
                    }
                }
            }
        }
        status = vm_execution_advance(execution, bulk ? budget : 1);
        steps += bulk && status == VM_YIELDED ? budget : 1;
        if (!bulk && execution->frame_count > depth) {
            Function *called = execution->frames[execution->frame_count - 1].function;
            entries[called - program.functions]++;
        }
        if (status == VM_WAITING) {
            waits++;
            if (execution->waiting_read && !execution->tcp) {
                Value *result = vm_fake_read_result(execution->fake_fail);
                vm_execution_complete_read(execution, result);
                release(result);
                status = execution->status;
            } else {
                gap = "unsupported-host-wait";
                break;
            }
        }
    }
    if (status == VM_YIELDED) {
        gap = "step-limit";
    }
    double cpu = (double)(clock() - start) / CLOCKS_PER_SEC;
    file = fopen(argv[2], "w");
    int result = 2;
    if (file) {
        fprintf(file, "coverage-probe-v1\n");
        for (size_t i = 0; i < program.count; i++) {
            fprintf(file, "function\t%zu\t%" PRIu64 "\t", i, entries[i]);
            for (const unsigned char *p = (const unsigned char *)program.functions[i].name; *p; p++) {
                fprintf(file, "%02x", *p);
            }
            fputc('\n', file);
            for (size_t pc = 0; pc < program.functions[i].ins_count; pc++) {
                Counter *counter = &counts[offsets[i] + pc];
                fprintf(file, "pc\t%zu\t%zu\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n",
                        i, pc, counter->attempts, counter->yes, counter->no);
            }
        }
        size_t scalar = 0;
        const char *kind = "other";
        if (execution->result && execution->result->kind == V_NAT &&
            pn_big_fits_size(&execution->result->as.integer, &scalar)) {
            kind = "Nat";
        }
        fprintf(file, "end\t%d\t%s\t%s\t%zu\t%zu\t%.6f\t%zu\t%zu\t%zu\n", status, gap,
                kind, scalar, waits, cpu, cells * sizeof(Counter) +
                (program.count + 1) * sizeof(size_t) + program.count * sizeof(uint64_t),
                program.count, cells);
        bool failed = ferror(file) != 0;
        if (fclose(file)) {
            failed = true;
        }
        if (!failed) {
            result = status == VM_COMPLETED ? 0 : 1;
        }
    }
    if (vm.error) {
        fprintf(stderr, "error: %s\n", vm.error);
    }
    vm_execution_destroy(execution);
    free_environment(environment, env_count);
    free(counts);
    free(offsets);
    free(entries);
    free_program(&program);
    return result;
}
