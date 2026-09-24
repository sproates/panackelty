#ifndef PANACKELTY_PROGRAM_H
#define PANACKELTY_PROGRAM_H

#include "bigint.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Decoded bytecode owns its names, constants, and instruction arrays. */

typedef struct {
    uint8_t tag;
    PnBigInt number;
    int16_t exponent;
    char *text;
    size_t text_length;
    bool boolean;
} Constant;

typedef struct {
    uint8_t op, code, arity;
    uint32_t target;
    char *name, *name2;
    size_t count;
    char **items;
    Constant constant;
} Instruction;

typedef struct {
    char *name;
    bool pure;
    size_t param_count, ins_count;
    char **params;
    Instruction *ins;
} Function;

typedef struct {
    size_t count;
    Function *functions;
} Program;

/* Wire values are fixed by bytecode/FORMAT.md. */
typedef enum {
    OP_CONST = 0,
    OP_LOAD = 1,
    OP_STORE = 2,
    OP_POP = 3,
    OP_UNARY = 4,
    OP_BINARY = 5,
    OP_MAKE_RANGE = 6,
    OP_MAKE_ARRAY = 7,
    OP_INDEX_GET = 8,
    OP_INTERPOLATE = 9,
    OP_ITER_INIT = 10,
    OP_ITER_NEXT = 11,
    OP_MAKE_RECORD = 12,
    OP_FIELD_GET = 13,
    OP_MAKE_VARIANT = 14,
    OP_MATCH_VARIANT = 15,
    OP_MATCH_FAIL = 16,
    OP_CALL = 17,
    OP_JUMP_FALSE = 18,
    OP_JUMP = 19,
    OP_RETURN = 20,
    OP_CALL_VALUE = 21,
} Opcode;

Function *program_function(Program *p, const char *name);
/* Releases a decoded or partially decoded program, then resets it to empty. */
void free_program(Program *p);

#endif
