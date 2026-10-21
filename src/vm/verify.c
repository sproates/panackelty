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

bool verify(Program *p, const char **error)
{
    p->verified_function_order = false;
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
    p->verified_function_order = true;
    return true;
}
