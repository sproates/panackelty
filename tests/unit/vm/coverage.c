/* Literal dispatcher contracts: counters must preserve execution semantics. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "coverage.h"
#include "vm.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void calls_and_traps(void)
{
    Instruction main_code[] = {{.op = OP_CALL, .name = "child"}, {.op = OP_RETURN}};
    Instruction child_code[] = {{.op = OP_CONST, .constant = {.tag = 5}}, {.op = OP_RETURN}};
    Function functions[] = {
        {.name = "main", .ins = main_code, .ins_count = 2},
        {.name = "child", .ins = child_code, .ins_count = 2}
    };
    Program program = {.count = 2, .functions = functions};
    for (unsigned trap = 0; trap < 2; trap++) {
        child_code[1].op = trap ? OP_MATCH_FAIL : OP_RETURN;
        VMCoverage *coverage = vm_coverage_create(&program, 4, UINT64_MAX);
        assert(coverage && vm_coverage_bytes(coverage) >= 4 * sizeof(VMCoverageCell));
        VM vm = {.program = &program, .coverage = coverage};
        const char *error;
        VMExecution *run = vm_execution_create(&vm, functions, NULL, NULL, NULL, &error);
        assert(run && !error);
        assert(vm_execution_advance(run, 0) == VM_YIELDED);
        assert(vm_coverage_cell(coverage, 0, 0)->attempted == 0);
        assert(vm_execution_advance(run, 1) == VM_YIELDED);
        assert(vm_coverage_cell(coverage, 0, 0)->attempted == 1);
        assert(vm_coverage_cell(coverage, 0, 0)->completed == 0);
        assert(vm_coverage_entries(coverage, 0) == 1 && vm_coverage_entries(coverage, 1) == 1);
        VMExecutionStatus terminal = trap ? VM_TRAPPED : VM_COMPLETED;
        assert(vm_execution_advance(run, 100) == terminal);
        assert(vm_execution_advance(run, 100) == terminal);
        assert(vm_coverage_cell(coverage, 0, 0)->completed == !trap);
        assert(vm_coverage_cell(coverage, 0, 1)->attempted == !trap);
        assert(vm_coverage_cell(coverage, 1, 1)->attempted == 1);
        assert(vm_coverage_cell(coverage, 1, 1)->completed == !trap);
        assert(!vm_coverage_flags(coverage));
        FILE *output = tmpfile();
        size_t size;
        assert(output && vm_coverage_raw_size(coverage, 1, 1, &size));
        assert(vm_coverage_write(coverage, output, (const uint8_t *)"a", 1, (const uint8_t *)"i", 1));
        assert(ftell(output) == (long)size);
        assert(!fseek(output, -2, SEEK_END));
        assert(fgetc(output) == (trap ? 2 : 1) && fgetc(output) == 0);
        fclose(output);
        assert(vm_execution_destroy(run));
        vm_coverage_destroy(coverage);
        VM plain = {.program = &program};
        Value *result = execute(&plain, functions, NULL);
        assert((result == NULL) == (trap != 0));
        release(result);
    }
    assert(!vm_coverage_create(&program, 3, UINT64_MAX));
    assert(!vm_coverage_create(&program, 4, 0));
    assert(!vm_coverage_create(&program, VM_COVERAGE_MAX_CELLS + 1, UINT64_MAX));
}

static void branches_and_overflow(void)
{
    /* Equal branch/fallthrough destinations must remain distinguishable. */
    Instruction code[] = {{.op = OP_CONST, .constant = {.tag = 4, .boolean = false}},
                          {.op = OP_JUMP_FALSE, .target = 2},
                          {.op = OP_CONST, .constant = {.tag = 5}}, {.op = OP_RETURN}};
    Function fn = {.name = "main", .ins = code, .ins_count = 4};
    Program program = {.count = 1, .functions = &fn};
    for (unsigned truth = 0; truth < 2; truth++) {
        code[0].constant.boolean = truth != 0;
        VMCoverage *coverage = vm_coverage_create(&program, 4, 1);
        VM vm = {.program = &program, .coverage = coverage};
        Value *result = execute(&vm, &fn, NULL);
        assert(result && !vm.error);
        release(result);
        const VMCoverageCell *branch = vm_coverage_cell(coverage, 0, 1);
        assert(branch->attempted == 1 && branch->completed == 1);
        assert(branch->fallthrough == truth && branch->branched == !truth);
        vm_coverage_attempt(coverage, 0, 1);
        assert(branch->attempted == 1 && vm_coverage_flags(coverage) == 2);
        vm_coverage_start(coverage, &program);
        assert(vm_coverage_flags(coverage) == 3);
        size_t size;
        assert(!vm_coverage_raw_size(coverage, VM_COVERAGE_MAX_RAW, 1, &size));
        vm_coverage_destroy(coverage);
    }
}

static bool pending(void *context, Value *value, const char **error)
{
    (void)context; (void)value; (void)error;
    return true;
}

static void suspended_completion(void)
{
    Instruction code[] = {{.op = OP_CONST, .constant = {.tag = 5}},
                          {.op = OP_CALL, .name = "print", .arity = 1}, {.op = OP_RETURN}};
    Function fn = {.name = "main", .ins = code, .ins_count = 3};
    Program program = {.count = 1, .functions = &fn};
    for (unsigned fail = 0; fail < 2; fail++) {
        VMCoverage *coverage = vm_coverage_create(&program, 3, UINT64_MAX);
        VM vm = {.program = &program, .coverage = coverage};
        const char *error;
        VMExecution *run = vm_execution_create_pending(&vm, &fn, NULL, pending, NULL, &error);
        assert(run && vm_execution_advance(run, 100) == VM_WAITING);
        assert(vm_execution_advance(run, 100) == VM_WAITING);
        assert(vm_coverage_cell(coverage, 0, 1)->attempted == 1);
        assert(vm_coverage_cell(coverage, 0, 1)->completed == 0);
        assert(vm_execution_complete_print(run, fail ? "test failure" : NULL));
        assert(!vm_execution_complete_print(run, NULL));
        assert(vm_coverage_cell(coverage, 0, 1)->completed == !fail);
        assert(vm_execution_advance(run, 100) == (fail ? VM_TRAPPED : VM_COMPLETED));
        assert(vm_execution_destroy(run));
        vm_coverage_destroy(coverage);
    }
}

static void typed_read_completion(void)
{
    Instruction code[] = {{.op = OP_CONST, .constant = {.tag = 4}},
                          {.op = OP_AWAIT_CALL, .name = "async_fake_read", .arity = 1},
                          {.op = OP_POP}, {.op = OP_CONST, .constant = {.tag = 5}},
                          {.op = OP_RETURN}};
    Function fn = {.name = "reader", .is_async = true, .ins = code, .ins_count = 5};
    Program program = {.count = 1, .functions = &fn};
    VMCoverage *coverage = vm_coverage_create(&program, 5, UINT64_MAX);
    VM vm = {.program = &program, .coverage = coverage};
    const char *error;
    VMExecution *run = vm_execution_create_async(&vm, &fn, NULL, pending, NULL, &error);
    assert(run && vm_execution_advance(run, 100) == VM_WAITING);
    assert(vm_coverage_cell(coverage, 0, 1)->completed == 0);
    Value *result = vm_fake_read_result(false);
    assert(result && vm_execution_complete_read(run, result));
    release(result);
    assert(vm_coverage_cell(coverage, 0, 1)->completed == 1);
    assert(vm_execution_advance(run, 100) == VM_COMPLETED);
    assert(vm_coverage_cell(coverage, 0, 1)->attempted == 1);
    assert(vm_execution_destroy(run));
    vm_coverage_destroy(coverage);
}

void coverage_contracts(void)
{
    calls_and_traps();
    branches_and_overflow();
    suspended_completion();
    typed_read_completion();
}
