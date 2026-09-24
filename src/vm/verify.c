#include "verify.h"

#include "builtins.h"
#include "program.h"

#include <stdlib.h>
#include <string.h>

/* Semantic verification is independent of source compilation and runtime checks. */

bool verify(Program *p, const char **error)
{
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
        if (!function->name[0]) {
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
            if (instruction->op == OP_CALL) {
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
                if (function->pure && !((b && b->pure) || (called && called->pure))) {
                    *error = "pure function calls impure function";
                    return false;
                }
            }
        }
        if (!returned) {
            *error = "bytecode function has no RETURN";
            return false;
        }
    }
    return true;
}
