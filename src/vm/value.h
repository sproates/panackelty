#ifndef PANACKELTY_VALUE_H
#define PANACKELTY_VALUE_H

#include "bigint.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Reference-counted values. Constructors return one owned reference unless documented otherwise. */

typedef enum {
    V_PATH,
    V_DURATION,
    V_INSTANT,
    V_RAT,
    V_UNIT,
    V_NAT,
    V_INT,
    V_DEC,
    V_STR,
    V_BOOL,
    V_VOID,
    V_RANGE,
    V_ARRAY,
    V_RECORD,
    V_VARIANT,
    V_MAP,
    V_SET,
    V_BYTES,
    V_ITER
} ValueKind;

typedef struct Value Value;

typedef struct {
    size_t count;
    Value **items;
} Sequence;

typedef struct {
    PnBigInt coefficient;
    int exponent;
    bool negative_zero;
} Decimal;

typedef struct {
    char *name;
    size_t count;
    char **names;
    Value **values;
} NamedValues;

typedef struct {
    Value *iterable;
    size_t index;
    PnBigInt cursor;
} Iterator;

struct Value {
    size_t refs;
    ValueKind kind;

    union {
        PnBigInt integer;
        Decimal decimal;

        struct {
            PnBigInt numerator, denominator;
        } rational;

        struct {
            size_t length, characters;
            bool ascii;
            uint8_t *data;
        } bytes;

        bool boolean;

        struct {
            PnBigInt start, end;
        } range;

        Sequence sequence;
        NamedValues named;
        Iterator iterator;
    } as;
};

/* NULL is accepted by both reference operations. */
Value *retain(Value *value);
void release(Value *value);

Value *value_new(ValueKind kind);
Value *value_bool(bool value);
Value *value_void(void);
Value *value_big(ValueKind kind, const PnBigInt *number);
Value *value_size(size_t number);

/* Copies bytes. V_STR callers supply valid UTF-8; cached offsets assume that invariant. */
Value *value_data(ValueKind kind, const uint8_t *data, size_t length);

/* Copies the container and retains each child; the input array stays caller-owned. */
Value *value_sequence(ValueKind kind, Value **items, size_t count);

/* Copies names and retains fields/payloads. names is NULL for variants. */
Value *named_value(ValueKind kind, const char *name, char **names, Value **values, size_t count);

bool value_equal(const Value *left, const Value *right);
bool value_index(const Value *value, size_t *index);
char *copy_text(const char *text);

#endif
