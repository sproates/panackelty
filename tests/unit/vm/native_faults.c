#ifdef NDEBUG
#undef NDEBUG
#endif
#include "builtins.h"
#include "decode.h"
#include "fault_injection.h"
#include "numeric.h"
#include "render.h"
#include "verify.h"
#include "vm.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

static Value *operand;
static uint8_t operation;
static Value *left, *right;
static size_t sweeps;

/* Discover each operation's successful allocation count, then fail every site.
 * One final successful run also checks that failures did not corrupt inputs.
 */
static void sweep(const char *name, void (*operation_fn)(void))
{
    size_t baseline = fault_live();
    fault_reset(0);
    operation_fn();
    size_t count = fault_attempts();
    assert(fault_live() == baseline);
    for (size_t failure = 1; failure <= count; failure++) {
        if (getenv("PANACK_FAULT_TRACE")) {
            fprintf(stderr, "fault: %s allocation %zu/%zu\n", name, failure, count);
        }
        fault_reset(failure);
        operation_fn();
        assert(fault_triggered());
        if (fault_live() != baseline) {
            fprintf(stderr, "leak: %s allocation %zu: baseline %zu, live %zu\n", name, failure,
                    baseline, fault_live());
            abort();
        }
        sweeps++;
    }
    fault_reset(0);
    operation_fn();
    assert(fault_live() == baseline);
}

static void decode_program(void)
{
    /* identity(value) { LOAD value; RETURN }, plus main with a decimal constant.
     * Names, parameters, instructions, decimal digits and bigint limbs allocate.
     */
    const uint8_t bytes[] = {
        'P', 'A',      'N',       'A', 'C',  'K',  'B', 'C', 0,       0,    8,        0,   2,   0,
        8,   'i',      'd',       'e', 'n',  't',  'i', 't', 'y',     1,    1,        0,   5,   'v',
        'a', 'l',      'u',       'e', 0,    0,    0,   2,   OP_LOAD, 0,    5,        'v', 'a', 'l',
        'u', 'e',      OP_RETURN, 0,   4,    'm',  'a', 'i', 'n',     0,    0,        0,   0,   0,
        2,   OP_CONST, 2,         0,   0xff, 0xfe, 0,   3,   0x12,    0x3f, OP_RETURN};
    Program program;
    const char *error = NULL;
    bool decoded = decode(bytes, sizeof(bytes), &program, &error);
    if (!fault_triggered()) {
        assert(decoded && verify(&program, &error));
    } else {
        assert(!decoded && error);
    }
    free_program(&program);
    free_program(&program);
}

static void construct_record(void)
{
    char *fields[] = {"first", "second"};
    Value *items[] = {operand, operand};
    Value *result = named_value(V_RECORD, "Pair", fields, items, 2);
    if (!fault_triggered()) {
        assert(result && operand->refs == 3);
    } else {
        assert(!result);
    }
    release(result);
    assert(operand->refs == 1);
}

static void construct_array(void)
{
    Value *items[] = {operand, operand};
    Value *result = value_sequence(V_ARRAY, items, 2);
    if (!fault_triggered()) {
        assert(result && operand->refs == 3);
    } else {
        assert(!result);
    }
    release(result);
    assert(operand->refs == 1);
}

static void construct_text(void)
{
    Value *result = value_data(V_STR, (const uint8_t *)"hello", 5);
    assert((result != NULL) == !fault_triggered());
    release(result);
}

static void arithmetic(void)
{
    const char *error = NULL;
    Value *result = binary_value(operation, left, right, &error);
    if (!fault_triggered()) {
        assert(result && !error);
    } else {
        assert(!result);
    }
    release(result);
    assert(left->refs == 1 && right->refs == 1);
}

static Value *integer(const char *digits)
{
    PnBigInt number = {0};
    assert(pn_big_from_digits(&number, digits, strlen(digits), 1));
    Value *result = value_big(V_INT, &number);
    pn_big_free(&number);
    assert(result);
    return result;
}

static void frames(void)
{
    /* Grow both stack and locals past their first two capacity boundaries,
     * overwrite existing locals, then leave references on the stack at return.
     */
    char names[40][16];
    char *parameters[] = {"argument"};
    Instruction instructions[164] = {0};
    size_t at = 0;
    for (size_t i = 0; i < 40; i++) {
        instructions[at++] = (Instruction){.op = OP_LOAD, .name = "argument"};
    }
    for (size_t i = 0; i < 40; i++) {
        snprintf(names[i], sizeof(names[i]), "local%zu", i);
        instructions[at++] = (Instruction){.op = OP_STORE, .name = names[i]};
    }
    for (size_t i = 0; i < 40; i++) {
        instructions[at++] = (Instruction){.op = OP_LOAD, .name = "argument"};
        instructions[at++] = (Instruction){.op = OP_STORE, .name = names[i]};
    }
    instructions[at++] = (Instruction){.op = OP_LOAD, .name = "argument"};
    instructions[at++] = (Instruction){.op = OP_LOAD, .name = "argument"};
    instructions[at++] = (Instruction){.op = OP_RETURN};
    Function function = {.name = "main",
                         .params = parameters,
                         .param_count = 1,
                         .ins = instructions,
                         .ins_count = at};
    Program program = {.functions = &function, .count = 1};
    VM vm = {.program = &program};
    Value *result = execute(&vm, &function, &operand);
    if (!fault_triggered()) {
        assert(result == operand && !vm.error);
    } else {
        assert(!result && vm.error);
    }
    release(result);
    assert(operand->refs == 1);
}

static Value *decimal(const char *digits, int exponent)
{
    PnBigInt coefficient = {0};
    assert(pn_big_from_digits(&coefficient, digits, strlen(digits), 1));
    Value *result = value_decimal(&coefficient, exponent);
    assert(result);
    return result;
}

static Value *process_inputs[7];

static void process_operation(void)
{
    VM vm = {0};
    Value *result = builtin_call(&vm, "process_run", process_inputs);
    if (!fault_triggered()) {
        assert(result && !strcmp(result->as.named.name, "Ok"));
        Value *output = result->as.named.values[0]->as.named.values[2];
        assert(output->as.bytes.length == 7 && !memcmp(output->as.bytes.data, "payload", 7));
    } else if (result) {
        assert(!strcmp(result->as.named.name, "Error"));
    }
    release(result);
    assert(!fault_descriptors() && !fault_children());
}

static void host_failures(void)
{
    process_inputs[0] = value_data(V_PATH, (const uint8_t *)"/bin/cat", 8);
    process_inputs[1] = value_sequence(V_ARRAY, NULL, 0);
    process_inputs[2] = value_data(V_BYTES, (const uint8_t *)"payload", 7);
    process_inputs[3] = value_data(V_PATH, (const uint8_t *)"/", 1);
    process_inputs[4] = value_sequence(V_ARRAY, NULL, 0);
    process_inputs[5] = value_size(3000000000ULL);
    process_inputs[5]->kind = V_DURATION;
    process_inputs[6] = value_size(1024);
    sweep("process", process_operation);

    const char *calls[] = {"clock_gettime", "pipe", "fork", "poll", "read", "write"};
    for (size_t i = 0; i < sizeof(calls) / sizeof(*calls); i++) {
        size_t occurrences = !strcmp(calls[i], "pipe")            ? 4
                             : !strcmp(calls[i], "clock_gettime") ? 2
                                                                  : 1;
        for (size_t occurrence = 1; occurrence <= occurrences; occurrence++) {
            size_t baseline = fault_live();
            fault_reset(0);
            fault_syscall(calls[i], occurrence, EIO);
            process_operation();
            assert(fault_triggered() && fault_live() == baseline);
        }
    }
    /* Interrupted polling and pipe I/O must recover without losing bytes. */
    const char *interrupted[] = {"poll", "read", "write"};
    for (size_t i = 0; i < 3; i++) {
        fault_reset(0);
        fault_syscall(interrupted[i], 1, EINTR);
        VM vm = {0};
        Value *result = builtin_call(&vm, "process_run", process_inputs);
        assert(fault_triggered() && result && !strcmp(result->as.named.name, "Ok"));
        Value *output = result->as.named.values[0]->as.named.values[2];
        assert(output->as.bytes.length == 7 && !memcmp(output->as.bytes.data, "payload", 7));
        release(result);
        assert(!fault_descriptors() && !fault_children());
    }
    for (size_t i = 0; i < 7; i++) {
        release(process_inputs[i]);
    }

    fault_reset(0);
    VM vm = {0};
    fault_syscall("clock_gettime", 1, EIO);
    Value *result = builtin_call(&vm, "instant_now", NULL);
    assert(fault_triggered() && result && !strcmp(result->as.named.name, "Error"));
    assert(!strcmp(result->as.named.values[0]->as.named.name, "ClockUnavailable"));
    release(result);

    Value *duration = value_size(0);
    duration->kind = V_DURATION;
    for (size_t i = 0; i < 2; i++) {
        fault_reset(0);
        fault_syscall("nanosleep", 1, i ? EIO : EINTR);
        result = builtin_call(&vm, "host_sleep", &duration);
        assert(fault_triggered() && result);
        assert(!strcmp(result->as.named.name, i ? "Error" : "Ok"));
        release(result);
    }
    release(duration);
    fault_reset(0);
}

static Value *file_inputs[2];
static const char *file_operation;

static void file_call(void)
{
    VM vm = {0};
    Value *result = builtin_call(&vm, file_operation, file_inputs);
    if (!fault_triggered()) {
        assert(result && !strcmp(result->as.named.name, "Ok"));
    } else if (result) {
        assert(!strcmp(result->as.named.name, "Error"));
    }
    release(result);
    assert(!fault_descriptors());
}

static void filesystem_failures(void)
{
    char path[] = "/tmp/panack-vm-fault-XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0 && write(fd, "data", 4) == 4 && !close(fd));
    file_inputs[0] = value_data(V_PATH, (const uint8_t *)path, strlen(path));
    file_inputs[1] = value_size(1024);
    file_operation = "fs_read";
    sweep("filesystem read", file_call);
    const char *read_calls[] = {"open", "fstat", "read"};
    for (size_t i = 0; i < 3; i++) {
        size_t baseline = fault_live();
        fault_reset(0);
        fault_syscall(read_calls[i], 1, EACCES);
        file_call();
        assert(fault_triggered() && baseline == fault_live());
    }
    fault_reset(0);
    release(file_inputs[1]);
    file_inputs[1] = value_data(V_BYTES, (const uint8_t *)"data", 4);
    file_operation = "fs_write";
    sweep("filesystem write", file_call);
    const char *write_calls[] = {"open", "fstat", "ftruncate", "write"};
    for (size_t i = 0; i < 4; i++) {
        size_t baseline = fault_live();
        fault_reset(0);
        fault_syscall(write_calls[i], 1, EIO);
        file_call();
        assert(fault_triggered() && baseline == fault_live());
    }
    fault_reset(0);
    release(file_inputs[0]);
    release(file_inputs[1]);
    assert(!unlink(path));
}

static Value *nested_inputs[2];
static const char *nested_builtin;

static void nested_call(void)
{
    VM vm = {0};
    Value *result = builtin_call(&vm, nested_builtin, nested_inputs);
    if (!fault_triggered()) {
        assert(result && result->kind == V_VOID && !vm.error);
    } else {
        assert(!result);
    }
    release(result);
}

static void nested_failures(void)
{
    /* Child main returns command_args(), exercising borrowed argument lifetimes. */
    const uint8_t bytes[] = {'P', 'A', 'N', 'A',     'C', 'K', 'B', 'C', 0,        0,   8,
                             0,   1,   0,   4,       'm', 'a', 'i', 'n', 0,        0,   0,
                             0,   0,   2,   OP_CALL, 0,   12,  'c', 'o', 'm',      'm', 'a',
                             'n', 'd', '_', 'a',     'r', 'g', 's', 0,   OP_RETURN};
    Value *argument = value_data(V_STR, (const uint8_t *)"argument", 8);
    nested_inputs[0] = value_data(V_BYTES, bytes, sizeof(bytes));
    nested_inputs[1] = value_sequence(V_ARRAY, &argument, 1);
    release(argument);
    nested_builtin = "run_bytecode";
    sweep("nested bytecode", nested_call);
    nested_builtin = "run_bytecode_args";
    sweep("nested arguments", nested_call);
    release(nested_inputs[0]);
    release(nested_inputs[1]);
}

static uint8_t *artifact;
static size_t artifact_length;
static Program loaded;
static bool expect_trap;

static void decode_artifact(void)
{
    Program program;
    const char *error = NULL;
    bool ok = decode(artifact, artifact_length, &program, &error);
    if (!fault_triggered()) {
        assert(ok && verify(&program, &error));
    } else {
        assert(!ok && error);
    }
    free_program(&program);
}

static void execute_artifact(void)
{
    VM vm = {.program = &loaded};
    Value *result = execute(&vm, program_function(&loaded, "main"), NULL);
    if (fault_triggered() || expect_trap) {
        assert(!result && vm.error);
    } else {
        size_t value;
        assert(result && !vm.error && value_index(result, &value) && value == 42);
    }
    release(result);
}

static void check_artifact(const char *mode, const char *path)
{
    FILE *file = fopen(path, "rb");
    assert(file && !fseek(file, 0, SEEK_END));
    long length = ftell(file);
    assert(length > 0 && length < 65536 && !fseek(file, 0, SEEK_SET));
    artifact_length = (size_t)length;
    artifact = malloc(artifact_length);
    assert(artifact && fread(artifact, 1, artifact_length, file) == artifact_length);
    assert(!fclose(file));
    if (!strcmp(mode, "decode")) {
        sweep("rich decode", decode_artifact);
    } else {
        const char *error = NULL;
        assert(decode(artifact, artifact_length, &loaded, &error) && verify(&loaded, &error));
        expect_trap = !strcmp(mode, "trap");
        sweep("rich execute", execute_artifact);
        free_program(&loaded);
    }
    free(artifact);
    assert(!fault_live());
    printf("native fault contracts: %zu allocation failures checked\n", sweeps);
}

int main(int argc, char **argv)
{
    if (argc == 3) {
        check_artifact(argv[1], argv[2]);
        return 0;
    }
    sweep("decode", decode_program);
    operand = value_size(42);
    assert(operand);
    sweep("record", construct_record);
    sweep("array", construct_array);
    sweep("text", construct_text);
    sweep("frames", frames);
    release(operand);

    left = integer("999999999999999999999999999");
    right = integer("123456789");
    for (operation = 0; operation <= 4; operation++) {
        sweep("integer arithmetic", arithmetic);
    }
    release(left);
    release(right);
    left = decimal("12345", -3);
    right = decimal("25", -2);
    for (operation = 0; operation <= 10; operation++) {
        sweep("decimal arithmetic", arithmetic);
    }
    release(left);
    release(right);
    host_failures();
    filesystem_failures();
    nested_failures();
    assert(fault_live() == 0);
    printf("native fault contracts: %zu allocation failures checked\n", sweeps);
    return 0;
}
