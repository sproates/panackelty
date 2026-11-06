#include "verify.h"

#include "builtins.h"
#include "program.h"

#include <stdlib.h>
#include <string.h>

/* Semantic verification is independent of source compilation and runtime checks. */

bool verify_call_edge(const Function *caller, const Function *callee,
                      bool builtin_pure, bool builtin_async, bool builtin_local, bool awaited)
{
    bool async = callee ? callee->is_async : builtin_async;
    bool pure = callee ? callee->pure : builtin_pure;
    if (awaited) {
        return caller->is_async && !caller->pure && async;
    }
    bool local = callee ? callee->local_effect : builtin_local;
    if (caller->pure) return !async && pure;
    return !async && (!(caller->is_async || caller->local_effect) || pure || local);
}

static void infer_local_effects(Program *program)
{
    for (size_t i = 0; i < program->count; i++) program->functions[i].local_effect = false;
    bool changed;
    do {
        changed = false;
        for (size_t i = 0; i < program->count; i++) {
            Function *function = &program->functions[i];
            if (function->pure || function->is_async || function->local_effect) continue;
            bool safe = true, local = false;
            for (size_t j = 0; j < function->ins_count && safe; j++) {
                Instruction *instruction = &function->ins[j];
                if (instruction->op == OP_CALL_VALUE || instruction->op == OP_AWAIT_VALUE ||
                    instruction->op == OP_AWAIT_CALL) {
                    safe = false;
                } else if (instruction->op == OP_CALL) {
                    const Builtin *entry = builtin(instruction->name);
                    Function *callee = entry ? NULL : program_function(program, instruction->name);
                    bool pure = entry ? entry->pure : callee && callee->pure;
                    bool resource = entry ? builtin_is_local_state(entry) : callee && callee->local_effect;
                    safe = pure || resource;
                    local = local || resource;
                }
            }
            if (safe && local) {
                function->local_effect = true;
                changed = true;
            }
        }
    } while (changed);
}

typedef struct {
    const char *name;
    size_t slot;
} LocalName;

static int local_name_order(const void *left, const void *right)
{
    return strcmp(((const LocalName *)left)->name, ((const LocalName *)right)->name);
}

static size_t local_name_slot(const LocalName *names, size_t count, const char *name)
{
    size_t low = 0, high = count;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        int order = strcmp(name, names[middle].name);
        if (!order) return names[middle].slot;
        if (order < 0) high = middle;
        else low = middle + 1;
    }
    return SIZE_MAX;
}

static bool local_instruction(uint8_t op)
{
    return op == OP_LOAD || op == OP_STORE || op == OP_ITER_INIT || op == OP_ITER_NEXT;
}

/* Temporary sorted names avoid repeated runtime searches. No owned plan remains. */
static bool prepare_locals(Function *function)
{
    size_t count = function->param_count;
    for (size_t i = 0; i < function->ins_count; i++) {
        uint8_t op = function->ins[i].op;
        size_t extra = op == OP_ITER_NEXT ? 2 : local_instruction(op) ? 1 : 0;
        if (extra > SIZE_MAX - count) return false;
        count += extra;
    }
    function->local_count = 0;
    if (!count) return true;
    if (count > SIZE_MAX / sizeof(LocalName)) return false;
    LocalName *names = malloc(count * sizeof(*names));
    if (!names) return false;
    size_t used = 0;
    for (size_t i = 0; i < function->param_count; i++) {
        names[used++] = (LocalName){function->params[i], i};
    }
    for (size_t i = 0; i < function->ins_count; i++) {
        Instruction *in = &function->ins[i];
        if (local_instruction(in->op)) names[used++] = (LocalName){in->name, SIZE_MAX};
        if (in->op == OP_ITER_NEXT) names[used++] = (LocalName){in->name2, SIZE_MAX};
    }
    qsort(names, used, sizeof(*names), local_name_order);
    size_t unique = 0, slots = function->param_count;
    for (size_t i = 0; i < used;) {
        LocalName entry = names[i++];
        while (i < used && !strcmp(entry.name, names[i].name)) {
            if (names[i].slot != SIZE_MAX) entry.slot = names[i].slot;
            i++;
        }
        if (entry.slot == SIZE_MAX) entry.slot = slots++;
        names[unique++] = entry;
    }
    for (size_t i = 0; i < function->ins_count; i++) {
        Instruction *in = &function->ins[i];
        if (local_instruction(in->op)) in->local_slot = local_name_slot(names, unique, in->name);
        if (in->op == OP_ITER_NEXT) in->local_slot2 = local_name_slot(names, unique, in->name2);
    }
    function->local_count = slots;
    free(names);
    return true;
}

bool verify(Program *p, const char **error)
{
    p->verified_function_order = false;
    p->resolved_calls = false;
    p->resolved_locals = false;
    infer_local_effects(p);
    Function *main = program_function(p, "main");
    if (!main) {
        *error = "bytecode has no main function";
        return false;
    }
    if (main->param_count) {
        *error = "main must not have parameters";
        return false;
    }
    for (size_t i = 0; i < p->count; i++) {
        Function *function = &p->functions[i];
        if ((function->pure && function->is_async) || !function->name[0]) {
            *error = "invalid function signature";
            return false;
        }
        if (i && strcmp(p->functions[i - 1].name, function->name) >= 0) {
            *error = "functions are not in canonical order";
            return false;
        }
        for (size_t a = 0; a < function->param_count; a++) {
            for (size_t b = a + 1; b < function->param_count; b++) {
                if (!strcmp(function->params[a], function->params[b])) {
                    *error = "invalid function signature";
                    return false;
                }
            }
        }
        if (!function->ins_count) {
            *error = "bytecode function is empty";
            return false;
        }
        bool returned = false;
        for (size_t j = 0; j < function->ins_count; j++) {
            Instruction *instruction = &function->ins[j];
            if (instruction->op == OP_RETURN) {
                returned = true;
            }
            if (instruction->op == OP_INTERPOLATE && !instruction->count) {
                *error = "invalid INTERPOLATE operand";
                return false;
            }
            if ((instruction->op == OP_ITER_NEXT || instruction->op == OP_MATCH_VARIANT ||
                 instruction->op == OP_JUMP_FALSE || instruction->op == OP_JUMP) &&
                instruction->target >= function->ins_count) {
                *error = "invalid jump target";
                return false;
            }
            if (instruction->op > OP_AWAIT_VALUE ||
                (instruction->op == OP_AWAIT_VALUE && !function->is_async)) {
                *error = "invalid async instruction";
                return false;
            }
            if (instruction->op == OP_CALL || instruction->op == OP_AWAIT_CALL) {
                const Builtin *b = builtin(instruction->name);
                Function *called = program_function(p, instruction->name);
                if (!b && !called) {
                    *error = "call to unknown function";
                    return false;
                }
                instruction->resolved_builtin = b;
                instruction->resolved_function = b ? NULL : called;
                size_t arity = b ? b->arity : called->param_count;
                if (instruction->arity != arity) {
                    *error = "call arity mismatch";
                    return false;
                }
                if (!verify_call_edge(function, b ? NULL : called, b && b->pure,
                                      builtin_is_async(b), builtin_is_local_state(b),
                                      instruction->op == OP_AWAIT_CALL)) {
                    *error = function->pure ? "pure function calls impure function" : "invalid async call effect";
                    return false;
                }
            }
        }
        if (!returned) {
            *error = "bytecode function has no RETURN";
            return false;
        }
    }
    for (size_t i = 0; i < p->count; i++) {
        if (!prepare_locals(&p->functions[i])) {
            *error = "native VM out of memory";
            return false;
        }
    }
    p->verified_function_order = true;
    p->resolved_calls = true;
    p->resolved_locals = true;
    return true;
}
