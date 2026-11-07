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
    V_ITER,
    V_HTTP_STATE,
    V_HTTP_WRITER,
    V_HTTP_CONNECTION
} ValueKind;

typedef struct Value Value;
typedef struct ArrayStorage ArrayStorage;
typedef struct CollectionIndex CollectionIndex;
typedef struct VMHttpState VMHttpState;
typedef struct VMHttpWriter VMHttpWriter;
typedef struct VMHttpConnection VMHttpConnection;

typedef struct {
    size_t count;
    Value **items;
    ArrayStorage *storage;
    CollectionIndex *index;
    size_t lookup_count;
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
    size_t refs; /* SIZE_MAX denotes an immutable immortal value. */
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
        VMHttpState *http_state;
        VMHttpWriter *http_writer;
        VMHttpConnection *http_connection;
    } as;
};

/* NULL is accepted by both reference operations. */
Value *retain(Value *value);
void release(Value *value);

Value *value_new(ValueKind kind);
/* Shared immutable Boolean; cannot fail. Balance ownership with retain/release as usual. */
Value *value_bool(bool value);
Value *value_void(void);
Value *value_big(ValueKind kind, const PnBigInt *number);
Value *value_size(size_t number);

/* Copies bytes. V_STR callers supply valid UTF-8; cached offsets assume that invariant. */
Value *value_data(ValueKind kind, const uint8_t *data, size_t length);
/* Copy borrowed byte spans into one owned result. NULL is allowed for empty spans. */
Value *value_data_join(ValueKind kind, const uint8_t *left, size_t left_length,
                      const uint8_t *right, size_t right_length);
/* Both inputs must be valid V_STR values with constructor-maintained metadata. */
Value *value_string_concat(const Value *left, const Value *right);
/* Valid Map/Set input: key/item position or SIZE_MAX. May memoize a lookup index. */
size_t value_collection_find(Value *collection, const Value *key);
/* Both inputs must be arrays; the result retains their elements independently. */
Value *value_array_concat(const Value *left, const Value *right);

/* Copies the container and retains each child; the input array stays caller-owned. */
Value *value_sequence(ValueKind kind, Value **items, size_t count);

/* Borrows both inputs and returns a new persistent array version. */
Value *value_array_append(Value *array, Value *item);

/* Copies names and retains fields/payloads. names is NULL for variants. */
Value *named_value(ValueKind kind, const char *name, char **names, Value **values, size_t count);

bool value_equal(const Value *left, const Value *right);
bool value_index(const Value *value, size_t *index);
char *copy_text(const char *text);

#endif
