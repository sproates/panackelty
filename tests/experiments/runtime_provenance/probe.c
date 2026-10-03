/* Test-only metadata observer. All values are computed by the real VM.
 * No Value references are retained, and event IDs never reuse storage identity. */
#include "../../../src/vm/execute.c"
#include "decode.h"
#include <assert.h>
#include <inttypes.h>
#include <sys/resource.h>
#include <stdio.h>
#include <time.h>

#define FRAME_LIMIT 32
#define SLOT_LIMIT 128
#define EDGE_LIMIT 8
#define STEP_LIMIT 10000000

typedef struct {
    uint64_t id, call, control, inputs[EDGE_LIMIT];
    Function *function; /* borrowed from Program; outlives the observer */
    size_t pc, value;
    unsigned op, code, input_count;
    const char *payload;
} Event;

typedef struct {
    uint64_t stack[SLOT_LIMIT], locals[SLOT_LIMIT], call, control;
    const char *names[SLOT_LIMIT]; /* borrowed Program strings */
    size_t stack_count, local_count;
} Origins;

typedef struct {
    Origins frames[FRAME_LIMIT];
    Event *events;
    size_t capacity, kept;
    uint64_t next, root;
    bool ring, values;
    const char *gap;
} Observer;

static uint64_t pop_origin(Origins *frame)
{
    return frame->stack_count ? frame->stack[--frame->stack_count] : 0;
}

static void scalar(Event *event, const Value *value, bool values)
{
    event->payload = "redacted";
    if (!values || !value) return;
    event->payload = "unsupported-value";
    if (value->kind == V_NAT && pn_big_fits_size(&value->as.integer, &event->value))
        event->payload = "Nat";
    else if (value->kind == V_BOOL) {
        event->value = value->as.boolean;
        event->payload = "Bool";
    }
}

static uint64_t save(Observer *observer, Event event)
{
    event.id = ++observer->next;
    if (observer->ring || observer->kept < observer->capacity) {
        size_t index = observer->ring ? (size_t)((event.id - 1) % observer->capacity)
                                      : observer->kept;
        observer->events[index] = event;
        if (observer->kept < observer->capacity) observer->kept++;
    }
    return event.id;
}

static const char *evidence(const Observer *observer, uint64_t id)
{
    if (!id) return "missing";
    if (id > observer->next) return "missing";
    if (!observer->ring && id > observer->capacity) return "discarded";
    if (observer->ring && observer->next - id >= observer->capacity) return "evicted";
    return "retained";
}

/* Refuse the whole observation frontier on unsupported machinery. Continuing
 * old shadow state through an unknown stack/control transform would invent edges. */
static bool supported(const Instruction *instruction, Program *program)
{
    switch (instruction->op) {
    case OP_CONST: case OP_LOAD: case OP_STORE: case OP_POP:
    case OP_UNARY: case OP_BINARY: case OP_JUMP_FALSE: case OP_JUMP: case OP_RETURN:
        return true;
    case OP_CALL:
        return !builtin(instruction->name) && program_function(program, instruction->name);
    default: return false;
    }
}

static VMExecutionStatus observe_step(Observer *observer, VMExecution *execution)
{
    size_t depth = execution->frame_count;
    Frame *frame = &execution->frames[depth - 1];
    /* Verification does not prove every control-flow path ends in RETURN.
     * Let the VM validate/trap falling off before reading an instruction. */
    if (frame->pc >= frame->function->ins_count) {
        if (!observer->gap) observer->gap = "trapped";
        return vm_execution_advance(execution, 1);
    }
    Instruction *instruction = &frame->function->ins[frame->pc];
    if (!observer->gap && (depth > FRAME_LIMIT || frame->stack_count >= SLOT_LIMIT ||
                          frame->local_count >= SLOT_LIMIT || instruction->arity > EDGE_LIMIT))
        observer->gap = "metadata-cap";
    if (!observer->gap && !supported(instruction, execution->vm->program))
        observer->gap = "unsupported-opcode";
    if (observer->gap) return vm_execution_advance(execution, 1);

    Origins *origins = &observer->frames[depth - 1];
    if (origins->stack_count != frame->stack_count) {
        observer->gap = "shadow-mismatch";
        return vm_execution_advance(execution, 1);
    }
    Event event = {.function = frame->function, .pc = frame->pc, .op = instruction->op,
                   .code = instruction->code, .call = origins->call,
                   .control = origins->control, .payload = "redacted"};
    size_t local = 0;
    switch (instruction->op) {
    case OP_LOAD:
        for (local = 0; local < origins->local_count; local++)
            if (!strcmp(origins->names[local], instruction->name)) break;
        event.inputs[event.input_count++] = local < origins->local_count ? origins->locals[local] : 0;
        break;
    case OP_BINARY:
        event.input_count = 2;
        event.inputs[1] = pop_origin(origins);
        event.inputs[0] = pop_origin(origins);
        break;
    case OP_UNARY: case OP_STORE: case OP_RETURN: case OP_JUMP_FALSE:
        event.inputs[event.input_count++] = pop_origin(origins);
        break;
    case OP_CALL:
        event.input_count = instruction->arity;
        for (size_t i = instruction->arity; i-- > 0;) event.inputs[i] = pop_origin(origins);
        break;
    case OP_POP: (void)pop_origin(origins); break;
    default: break;
    }
    /* Snapshot consumed values before the VM releases them. */
    if ((instruction->op == OP_RETURN || instruction->op == OP_STORE ||
         instruction->op == OP_JUMP_FALSE) && frame->stack_count)
        scalar(&event, frame->stack[frame->stack_count - 1], observer->values);
    VMExecutionStatus status = vm_execution_advance(execution, 1);
    if (status == VM_TRAPPED || status == VM_WAITING) {
        observer->gap = status == VM_TRAPPED ? "trapped" : "host-wait";
        return status;
    }
    if (instruction->op == OP_POP || instruction->op == OP_JUMP) return status;
    if (instruction->op == OP_CONST || instruction->op == OP_LOAD ||
        instruction->op == OP_BINARY || instruction->op == OP_UNARY) {
        Frame *after = &execution->frames[depth - 1];
        scalar(&event, after->stack[after->stack_count - 1], observer->values);
    }
    uint64_t id = save(observer, event);
    switch (instruction->op) {
    case OP_CONST: case OP_LOAD: case OP_BINARY: case OP_UNARY:
        origins->stack[origins->stack_count++] = id;
        break;
    case OP_STORE:
        for (local = 0; local < origins->local_count; local++)
            if (!strcmp(origins->names[local], instruction->name)) break;
        if (local == origins->local_count) origins->names[origins->local_count++] = instruction->name;
        origins->locals[local] = id;
        break;
    case OP_JUMP_FALSE: origins->control = id; break;
    case OP_CALL:
        if (depth == FRAME_LIMIT) { observer->gap = "metadata-cap"; break; }
        observer->frames[depth] = (Origins){.call = id, .control = origins->control};
        for (size_t i = 0; i < instruction->arity; i++) {
            observer->frames[depth].names[i] = execution->frames[depth].function->params[i];
            observer->frames[depth].locals[i] = event.inputs[i];
        }
        observer->frames[depth].local_count = instruction->arity;
        break;
    case OP_RETURN:
        if (depth == 1) observer->root = id;
        else {
            Origins *parent = &observer->frames[depth - 2];
            parent->stack[parent->stack_count++] = id;
        }
        break;
    default: break;
    }
    return status;
}

static int self_test(void)
{
    Event events[2] = {0};
    Observer observer = {.events = events, .capacity = 2, .ring = true};
    assert(save(&observer, (Event){.value = 6}) == 1);
    assert(save(&observer, (Event){.value = 7}) == 2);
    assert(save(&observer, (Event){.value = 42}) == 3);
    assert(!strcmp(evidence(&observer, 1), "evicted"));
    assert(!strcmp(evidence(&observer, 3), "retained"));
    assert(events[0].id == 3 && events[0].value == 42);
    assert(!strcmp(evidence(&observer, 0), "missing"));
    assert(!strcmp(evidence(&observer, 4), "missing"));
    observer = (Observer){.events = events, .capacity = 2};
    for (unsigned i = 0; i < 3; i++) (void)save(&observer, (Event){0});
    assert(!strcmp(evidence(&observer, 3), "discarded"));
    assert(events[0].id == 1 && events[1].id == 2);
    Origins origins = {.stack = {19, 27}, .stack_count = 2};
    assert(pop_origin(&origins) == 27 && pop_origin(&origins) == 19);
    assert(pop_origin(&origins) == 0);
    puts("runtime provenance retention unit checks passed");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "--self-test")) return self_test();
    if (argc < 4 || argc > 6) return 2;
    bool bulk = !strcmp(argv[2], "bulk"), step = !strcmp(argv[2], "step");
    if (!bulk && !step && strcmp(argv[2], "ring") && strcmp(argv[2], "prefix")) return 2;
    char *end = NULL;
    unsigned long capacity = strtoul(argv[3], &end, 10);
    if (!*argv[3] || *end || capacity < 1 || capacity > 65536) return 2;
    bool values = argc >= 5 && !strcmp(argv[4], "values");
    bool dump = argc == 6 && !strcmp(argv[5], "dump");
    FILE *file = fopen(argv[1], "rb");
    if (!file) return 2;
    if (fseek(file, 0, SEEK_END) || ftell(file) <= 0 || ftell(file) > MAX_ARTIFACT) { fclose(file); return 2; }
    size_t length = (size_t)ftell(file);
    rewind(file);
    uint8_t *data = malloc(length);
    if (!data) { fclose(file); return 2; }
    size_t got = fread(data, 1, length, file);
    fclose(file);
    Program program = {0};
    const char *error = NULL;
    bool valid = got == length && decode(data, got, &program, &error) && verify(&program, &error);
    free(data);
    if (!valid) { free_program(&program); return 2; }
    VM vm = {.program = &program};
    VMExecution *execution = vm_execution_create(&vm, program_function(&program, "main"), NULL, NULL, NULL, &error);
    if (!execution) { free_program(&program); return 2; }
    Observer *observer = NULL;
    if (!bulk && !step) {
        observer = calloc(1, sizeof(*observer));
        if (observer) observer->events = calloc(capacity, sizeof(Event));
        if (!observer || !observer->events) {
            free(observer); vm_execution_destroy(execution); free_program(&program); return 2;
        }
        observer->capacity = capacity;
        observer->ring = !strcmp(argv[2], "ring");
        observer->values = values;
    }
    clock_t started = clock();
    VMExecutionStatus status = VM_YIELDED;
    size_t ticks = 0;
    while (status == VM_YIELDED && ticks < STEP_LIMIT) {
        status = bulk ? vm_execution_advance(execution, STEP_LIMIT) :
                 step ? vm_execution_advance(execution, 1) : observe_step(observer, execution);
        ticks += bulk ? STEP_LIMIT : 1;
    }
    /* At most one event per step and ten million steps: uint64 IDs cannot wrap. */
    if (observer && status == VM_YIELDED && !observer->gap) observer->gap = "step-limit";
    double cpu = (double)(clock() - started) / CLOCKS_PER_SEC;
    Event result = {0};
    scalar(&result, execution->result, values);
    printf("summary\t%d\t%s\t%zu\t%.6f\t%zu\t%" PRIu64 "\t%s\t%" PRIu64 "\t%s\n",
           status, result.payload, result.value, cpu,
           observer ? sizeof(*observer) + capacity * sizeof(Event) : 0,
           observer ? observer->next : 0, observer && observer->gap ? observer->gap : "none",
           observer ? observer->root : 0, observer ? evidence(observer, observer->root) : "missing");
    struct rusage usage = {0};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
#ifdef __APPLE__
        printf("rss-bytes\t%ld\n", usage.ru_maxrss);
#else
        printf("rss-bytes\t%ld\n", usage.ru_maxrss * 1024);
#endif
    }
    if (dump && observer) {
        for (size_t i = 0; i < observer->kept; i++) {
            Event *event = &observer->events[i];
            printf("event\t%" PRIu64 "\t%s\t%zu\t%u\t%u\t%s\t%zu\t%" PRIu64 ":%s\t%" PRIu64 ":%s",
                   event->id, event->function->name, event->pc, event->op, event->code,
                   event->payload, event->value, event->call, evidence(observer, event->call),
                   event->control, evidence(observer, event->control));
            for (unsigned j = 0; j < event->input_count; j++)
                printf("\t%" PRIu64 ":%s", event->inputs[j], evidence(observer, event->inputs[j]));
            putchar('\n');
        }
    }
    if (observer) free(observer->events);
    free(observer);
    vm_execution_destroy(execution);
    free_program(&program);
    return status == VM_COMPLETED ? 0 : 1;
}
