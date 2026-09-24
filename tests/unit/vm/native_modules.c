/* Direct contracts across the separately compiled VM modules.
 * Assertions stay enabled even when a release build supplies -DNDEBUG.
 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "builtins.h"
#include "decode.h"
#include "numeric.h"
#include "render.h"
#include "utf8.h"
#include "verify.h"
#include "vm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void values_own_copies_and_retain_children(void)
{
    uint8_t bytes[] = {'a', 0, 'b'};
    Value *child = value_data(V_BYTES, bytes, sizeof(bytes));
    assert(child && child->refs == 1);
    bytes[0] = 'z';
    assert(child->as.bytes.data[0] == 'a');

    Value *items[] = {child, child};
    Value *array = value_sequence(V_ARRAY, items, 2);
    assert(array && child->refs == 3);
    release(array);
    assert(child->refs == 1);

    char label[] = "payload";
    char *names[] = {label};
    Value *record = named_value(V_RECORD, "Box", names, items, 1);
    assert(record && child->refs == 2);
    label[0] = 'X';
    assert(strcmp(record->as.named.names[0], "payload") == 0);
    release(record);
    assert(child->refs == 1);
    release(child);
}

static void exact_arithmetic_borrows_operands(void)
{
    Value *left = value_size(7), *right = value_size(3);
    const char *error = NULL;
    Value *quotient = binary_value(3, left, right, &error);
    assert(quotient && quotient->kind == V_RAT && !error);
    assert(left->refs == 1 && right->refs == 1);
    size_t integer;
    assert(value_index(left, &integer) && integer == 7);
    assert(value_index(right, &integer) && integer == 3);

    Buffer rendered = {0};
    assert(render(&rendered, quotient, false));
    assert(strcmp(rendered.data, "7/3") == 0);
    free(rendered.data);
    release(quotient);

    Value *zero = value_size(0);
    assert(!binary_value(3, left, zero, &error));
    assert(strcmp(error, "VM trap: division by zero") == 0);
    assert(left->refs == 1 && zero->refs == 1);
    release(zero);
    release(left);
    release(right);
}

static void utf8_offsets_preserve_code_points(void)
{
    const uint8_t text[] = {'a', 0xc3, 0xa9, 0xf0, 0x9f, 0x98, 0x80};
    assert(utf8(text, sizeof(text)));
    assert(!utf8(text, sizeof(text) - 1));
    const uint8_t overlong[] = {0xc0, 0x80};
    const uint8_t surrogate[] = {0xed, 0xa0, 0x80};
    assert(!utf8(overlong, sizeof(overlong)));
    assert(!utf8(surrogate, sizeof(surrogate)));
    Value *value = value_data(V_STR, text, sizeof(text));
    assert(value && value->as.bytes.characters == 3 && !value->as.bytes.ascii);
    size_t start, end;
    assert(utf8_offset(value, 1, &start, &end) && start == 1 && end == 3);
    assert(utf8_offset(value, 2, &start, &end) && start == 3 && end == 7);
    assert(!utf8_offset(value, 3, &start, &end));
    release(value);
}

static void frames_release_arguments_on_return_and_trap(void)
{
    char *parameters[] = {"argument"};
    Instruction instructions[] = {
        {.op = OP_LOAD, .name = "argument"},
        {.op = OP_RETURN},
    };
    Function function = {.name = "main", .param_count = 1, .params = parameters,
                         .ins_count = 2, .ins = instructions};
    Program program = {.count = 1, .functions = &function};
    VM vm = {.program = &program};
    Value *argument = value_size(42);
    Value *result = execute(&vm, &function, &argument);
    assert(result == argument && !vm.error && argument->refs == 2);
    release(result);
    assert(argument->refs == 1);

    /* A missing second operand must release the operand already popped.
     * Refcounts detect leaks even on hosts without LeakSanitizer support.
     */
    const uint8_t operations[] = {OP_BINARY, OP_MAKE_RANGE, OP_INDEX_GET};
    for (size_t i = 0; i < sizeof(operations); i++) {
        vm.error = NULL;
        instructions[1].op = operations[i];
        assert(!execute(&vm, &function, &argument));
        assert(strcmp(vm.error, "VM trap: operand stack underflow") == 0);
        assert(argument->refs == 1);
    }

    /* Multi-value constructors and calls must discard every partially popped argument. */
    char *fields[] = {"first", "second"};
    const uint8_t constructors[] = {OP_MAKE_ARRAY, OP_MAKE_RECORD, OP_MAKE_VARIANT, OP_CALL};
    for (size_t i = 0; i < sizeof(constructors); i++) {
        vm.error = NULL;
        instructions[1] = (Instruction){.op = constructors[i], .count = 2, .arity = 2,
                                        .name = "missing", .name2 = "Missing", .items = fields};
        assert(!execute(&vm, &function, &argument));
        assert(strcmp(vm.error, "VM trap: operand stack underflow") == 0);
        assert(argument->refs == 1);
    }
    release(argument);
}

static void nested_calls_preserve_caller_ownership(void)
{
    char *parameters[] = {"value"};
    Instruction inner[] = {{.op = OP_LOAD, .name = "value"}, {.op = OP_RETURN}};
    Instruction outer[] = {{.op = OP_LOAD, .name = "value"},
                           {.op = OP_CALL, .name = "identity", .arity = 1},
                           {.op = OP_RETURN}};
    Function functions[] = {
        {.name = "identity", .param_count = 1, .params = parameters, .ins_count = 2, .ins = inner},
        {.name = "main", .param_count = 1, .params = parameters, .ins_count = 3, .ins = outer},
    };
    Program program = {.count = 2, .functions = functions};
    VM vm = {.program = &program};
    Value *argument = value_size(123);
    Value *result = execute(&vm, &functions[1], &argument);
    assert(result == argument && argument->refs == 2 && !vm.error);
    release(result);

    inner[1].op = OP_MATCH_FAIL;
    assert(!execute(&vm, &functions[1], &argument));
    assert(vm.error && argument->refs == 1);
    release(argument);
}

static void decode_mutations_release_partial_programs(void)
{
    /* Version 8: main() { CONST Void; RETURN }. Mutations never execute. */
    const uint8_t valid[] = {0x50, 0x41, 0x4e, 0x41, 0x43, 0x4b, 0x42, 0x43, 0,
                            0, 8, 0, 1, 0, 4, 'm', 'a', 'i', 'n', 0, 0,
                            0, 0, 0, 2, OP_CONST, 5, OP_RETURN};
    for (size_t length = 0; length < sizeof(valid); length++) {
        Program program;
        const char *error = NULL;
        assert(!decode(valid, length, &program, &error));
        assert(error);
        free_program(&program);
        assert(!program.functions && !program.count);
    }
    for (size_t position = 0; position < sizeof(valid); position++) {
        for (unsigned byte = 0; byte <= 255; byte++) {
            uint8_t mutated[sizeof(valid)];
            memcpy(mutated, valid, sizeof(valid));
            mutated[position] = (uint8_t)byte;
            Program program;
            const char *error = NULL;
            if (decode(mutated, sizeof(mutated), &program, &error)) {
                (void)verify(&program, &error);
            } else {
                assert(error);
            }
            free_program(&program);
            assert(!program.functions && !program.count);
        }
    }
}

static void nested_bytecode_releases_rejected_programs(void)
{
    /* Structurally valid, but the function is named fail rather than main. */
    const uint8_t invalid[] = {0x50, 0x41, 0x4e, 0x41, 0x43, 0x4b, 0x42, 0x43, 0,
                              0, 8, 0, 1, 0, 4, 'f', 'a', 'i', 'l', 0, 0,
                              0, 0, 0, 2, OP_CONST, 5, OP_RETURN};
    Value *bytes = value_data(V_BYTES, invalid, sizeof(invalid));
    Value *arguments = value_sequence(V_ARRAY, NULL, 0);
    Value *inputs[] = {bytes, arguments};
    const char *builtins[] = {"run_bytecode", "run_bytecode_args"};
    for (size_t i = 0; i < 2; i++) {
        VM vm = {0};
        assert(!builtin_call(&vm, builtins[i], inputs));
        assert(strcmp(vm.error, "bytecode has no main function") == 0);
        assert(bytes->refs == 1 && arguments->refs == 1);
    }
    release(bytes);
    release(arguments);
}

static void builtin_domains_preserve_errors_and_results(void)
{
    VM vm = {0};
    Value *unit = builtin_call(&vm, "$unit", NULL);
    assert(unit && unit->kind == V_UNIT && !vm.error);
    release(unit);
    Value *empty = builtin_call(&vm, "bytes", NULL);
    assert(empty && empty->kind == V_BYTES && !empty->as.bytes.length);
    release(empty);

    Value *wrong = value_size(7);
    Value *arguments[] = {wrong, wrong, wrong};
    const char *invalid[] = {"utf8_decode", "$method_has", "path_from_text", "host_sleep",
                             "fs_metadata", "run_bytecode", "nat"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
        vm.error = NULL;
        assert(!builtin_call(&vm, invalid[i], arguments));
        assert(vm.error && strstr(vm.error, "unknown builtin") == NULL);
        assert(wrong->refs == 1);
    }
    vm.error = NULL;
    assert(!builtin_call(&vm, "not_a_builtin", NULL));
    assert(strcmp(vm.error, "VM trap: unknown builtin") == 0);
    release(wrong);
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "registry") == 0) {
        for (int i = 2; i < argc; i++) {
            const Builtin *entry = builtin(argv[i]);
            assert(entry && entry->call);
            printf("%s %u %u\n", entry->name, entry->arity, entry->pure ? 1 : 0);
        }
        return 0;
    }
    values_own_copies_and_retain_children();
    exact_arithmetic_borrows_operands();
    utf8_offsets_preserve_code_points();
    frames_release_arguments_on_return_and_trap();
    nested_calls_preserve_caller_ownership();
    decode_mutations_release_partial_programs();
    nested_bytecode_releases_rejected_programs();
    builtin_domains_preserve_errors_and_results();
    puts("native module contracts: ok");
    return 0;
}
