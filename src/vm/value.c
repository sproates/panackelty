#include "value.h"

#include "numeric.h"
#include "http_state.h"

#include <stdlib.h>
#include <string.h>

/* Reference-counted values. Constructors return one owned reference unless documented otherwise. */

/* At most two array views share append storage: a prefix and its one-element
 * extension. A further append forks until one view dies. This bounds retention
 * and lets dropping an extension promptly release its otherwise hidden child.
 */
struct ArrayStorage {
    size_t refs, count, capacity;
    Value **items;
};

Value *value_new(ValueKind kind)
{
    Value *v = calloc(1, sizeof(*v));
    if (v) {
        v->refs = 1;
        v->kind = kind;
    }
    return v;
}

Value *retain(Value *v)
{
    if (v && v->refs != SIZE_MAX) {
        v->refs++;
    }
    return v;
}

void release(Value *v)
{
    if (!v || v->refs == SIZE_MAX || --v->refs) {
        return;
    }
    switch (v->kind) {
    case V_HTTP_CONNECTION:
        http_connection_free(v->as.http_connection);
        break;
    case V_HTTP_STATE:
        http_state_free(v->as.http_state);
        break;
    case V_HTTP_WRITER:
        http_writer_free(v->as.http_writer);
        break;
    case V_NAT:
    case V_INT:
    case V_DURATION:
    case V_INSTANT:
        pn_big_free(&v->as.integer);
        break;
    case V_RAT:
        pn_big_free(&v->as.rational.numerator);
        pn_big_free(&v->as.rational.denominator);
        break;
    case V_DEC:
        pn_big_free(&v->as.decimal.coefficient);
        break;
    case V_STR:
    case V_BYTES:
    case V_PATH:
        free(v->as.bytes.data);
        break;
    case V_RANGE:
        pn_big_free(&v->as.range.start);
        pn_big_free(&v->as.range.end);
        break;
    case V_ARRAY:
    case V_MAP:
    case V_SET:
        if (v->as.sequence.index) free(v->as.sequence.index);
        if (v->as.sequence.storage) {
            ArrayStorage *storage = v->as.sequence.storage;
            if (--storage->refs == 0) {
                for (size_t i = 0; i < storage->count; i++) {
                    release(storage->items[i]);
                }
                free(storage->items);
                free(storage);
            } else if (v->as.sequence.count == storage->count) {
                /* Only the shorter view survives; discard the hidden suffix. */
                release(storage->items[--storage->count]);
                storage->items[storage->count] = NULL;
            }
            break;
        }
        for (size_t i = 0; i < v->as.sequence.count; i++) {
            release(v->as.sequence.items[i]);
        }
        free(v->as.sequence.items);
        break;
    case V_RECORD:
    case V_VARIANT:
        for (size_t i = 0; i < v->as.named.count; i++) {
            free(v->as.named.names ? v->as.named.names[i] : NULL);
            release(v->as.named.values ? v->as.named.values[i] : NULL);
        }
        free(v->as.named.name);
        free(v->as.named.names);
        free(v->as.named.values);
        break;
    case V_ITER:
        release(v->as.iterator.iterable);
        pn_big_free(&v->as.iterator.cursor);
        break;
    default:
        break;
    }
    free(v);
}

/* Immortal immutable values: retain/release never write their reference counts. */
static Value boolean_values[2] = {
    {.refs = SIZE_MAX, .kind = V_BOOL, .as.boolean = false},
    {.refs = SIZE_MAX, .kind = V_BOOL, .as.boolean = true},
};

Value *value_bool(bool value)
{
    return &boolean_values[value ? 1 : 0];
}

Value *value_void(void)
{
    return value_new(V_VOID);
}

Value *value_big(ValueKind kind, const PnBigInt *number)
{
    Value *v = value_new(kind);
    if (!v) {
        return NULL;
    }
    if (!pn_big_copy(&v->as.integer, number)) {
        free(v);
        return NULL;
    }
    return v;
}

Value *value_size(size_t number)
{
    PnBigInt n;
    if (!pn_big_from_u64(&n, number)) {
        return NULL;
    }
    Value *v = value_big(V_NAT, &n);
    pn_big_free(&n);
    return v;
}

Value *value_data(ValueKind kind, const uint8_t *data, size_t length)
{
    return value_data_join(kind, data, length, NULL, 0);
}

static Value *allocate_data_join(ValueKind kind, const uint8_t *left, size_t left_length,
                                 const uint8_t *right, size_t right_length)
{
    if (left_length == SIZE_MAX || right_length > SIZE_MAX - left_length - 1) {
        return NULL;
    }
    size_t length = left_length + right_length;
    Value *v = value_new(kind);
    if (!v) {
        return NULL;
    }
    v->as.bytes.data = malloc(length + 1);
    if (!v->as.bytes.data) {
        free(v);
        return NULL;
    }
    if (left_length) {
        memcpy(v->as.bytes.data, left, left_length);
    }
    if (right_length) {
        memcpy(v->as.bytes.data + left_length, right, right_length);
    }
    v->as.bytes.data[length] = 0;
    v->as.bytes.length = length;
    return v;
}

Value *value_data_join(ValueKind kind, const uint8_t *left, size_t left_length,
                      const uint8_t *right, size_t right_length)
{
    Value *v = allocate_data_join(kind, left, left_length, right, right_length);
    if (!v) return NULL;
    if (kind == V_STR) {
        const uint8_t *data = v->as.bytes.data;
        v->as.bytes.ascii = true;
        for (size_t i = 0; i < v->as.bytes.length; i++) {
            if ((data[i] & 0xc0) != 0x80) {
                v->as.bytes.characters++;
            }
            if (data[i] >= 0x80) {
                v->as.bytes.ascii = false;
            }
        }
    }
    return v;
}

Value *value_string_concat(const Value *left, const Value *right)
{
    Value *v = allocate_data_join(V_STR, left->as.bytes.data, left->as.bytes.length,
                                  right->as.bytes.data, right->as.bytes.length);
    if (!v) return NULL;
    /* Valid string character counts cannot exceed the checked byte lengths. */
    v->as.bytes.characters = left->as.bytes.characters + right->as.bytes.characters;
    v->as.bytes.ascii = left->as.bytes.ascii && right->as.bytes.ascii;
    return v;
}

Value *value_array_concat(const Value *left, const Value *right)
{
    size_t n = left->as.sequence.count, m = right->as.sequence.count;
    if (m > SIZE_MAX - n || n + m > SIZE_MAX / sizeof(Value *)) {
        return NULL;
    }
    Value *v = value_new(V_ARRAY);
    if (!v) {
        return NULL;
    }
    v->as.sequence.items = calloc(n + m, sizeof(Value *));
    if (n + m && !v->as.sequence.items) {
        free(v);
        return NULL;
    }
    v->as.sequence.count = n + m;
    for (size_t i = 0; i < n; i++) {
        v->as.sequence.items[i] = retain(left->as.sequence.items[i]);
    }
    for (size_t i = 0; i < m; i++) {
        v->as.sequence.items[n + i] = retain(right->as.sequence.items[i]);
    }
    return v;
}

Value *value_sequence(ValueKind kind, Value **items, size_t count)
{
    Value *v = value_new(kind);
    if (!v) {
        return NULL;
    }
    v->as.sequence.items = calloc(count, sizeof(Value *));
    if (count && !v->as.sequence.items) {
        free(v);
        return NULL;
    }
    v->as.sequence.count = count;
    for (size_t i = 0; i < count; i++) {
        v->as.sequence.items[i] = retain(items[i]);
    }
    return v;
}

/* Follow ownership edges, including a shared view's hidden suffix. Sharing is
 * safe only when the appended item cannot retain this backing store. Bound the
 * traversal so deep or wide values conservatively copy without unbounded work.
 */
static bool append_independent(const Value *item, const ArrayStorage *target, size_t *budget)
{
    if (*budget == 0) {
        return false;
    }
    --*budget;
    switch (item->kind) {
    case V_HTTP_CONNECTION:
    case V_HTTP_STATE:
    case V_HTTP_WRITER:
        /* Opaque resources can retain container values; do not share append
         * storage across an ownership boundary we cannot inspect here. */
        return false;
    case V_ARRAY:
    case V_MAP:
    case V_SET: {
        const ArrayStorage *storage = item->as.sequence.storage;
        if (storage == target) {
            return false;
        }
        size_t count = storage ? storage->count : item->as.sequence.count;
        for (size_t i = 0; i < count; i++) {
            if (!append_independent(item->as.sequence.items[i], target, budget)) {
                return false;
            }
        }
        return true;
    }
    case V_ITER:
        return append_independent(item->as.iterator.iterable, target, budget);
    case V_RECORD:
    case V_VARIANT:
        for (size_t i = 0; i < item->as.named.count; i++) {
            if (!append_independent(item->as.named.values[i], target, budget)) {
                return false;
            }
        }
        return true;
    default:
        return true;
    }
}

Value *value_array_append(Value *array, Value *item)
{
    size_t count = array->as.sequence.count;
    size_t limit = SIZE_MAX / sizeof(Value *);
    if (count >= limit) {
        return NULL;
    }
    Value *result = value_new(V_ARRAY);
    if (!result) {
        return NULL;
    }
    ArrayStorage *storage = array->as.sequence.storage;
    size_t budget = 64;
    if (storage && storage->refs == 1 && count < storage->capacity &&
        append_independent(item, storage, &budget)) {
        /* Allocate the result before changing storage: OOM leaves inputs intact. */
        storage->items[count] = retain(item);
        storage->count = count + 1;
        storage->refs++;
    } else {
        storage = calloc(1, sizeof(*storage));
        if (!storage) {
            free(result);
            return NULL;
        }
        size_t capacity = count < 8 ? 8 : count <= limit / 2 ? count * 2 : limit;
        storage->items = malloc(capacity * sizeof(Value *));
        if (!storage->items) {
            free(storage);
            free(result);
            return NULL;
        }
        storage->refs = 1;
        storage->count = count + 1;
        storage->capacity = capacity;
        for (size_t i = 0; i < count; i++) {
            storage->items[i] = retain(array->as.sequence.items[i]);
        }
        storage->items[count] = retain(item);
    }
    result->as.sequence.count = count + 1;
    result->as.sequence.items = storage->items;
    result->as.sequence.storage = storage;
    return result;
}

struct CollectionIndex {
    size_t capacity;
    size_t positions[]; /* Slot position plus one; zero denotes an empty bucket. */
};

static uint64_t collection_hash_byte(uint64_t hash, uint8_t byte)
{
    return (hash ^ byte) * UINT64_C(1099511628211);
}

/* Only hash domains whose representation agrees with value_equal. */
static bool collection_hash(const Value *key, uint64_t *hash)
{
    uint64_t h = collection_hash_byte(UINT64_C(14695981039346656037), (uint8_t)key->kind);
    switch (key->kind) {
    case V_STR:
    case V_BYTES:
    case V_PATH:
        for (size_t i = 0; i < key->as.bytes.length; i++) h = collection_hash_byte(h, key->as.bytes.data[i]);
        break;
    case V_NAT:
    case V_INT:
    case V_DURATION:
    case V_INSTANT:
        h = collection_hash_byte(h, (uint8_t)key->as.integer.sign);
        for (size_t i = 0; i < key->as.integer.len; i++) {
            uint32_t limb = key->as.integer.limbs[i];
            for (size_t j = 0; j < 4; j++) h = collection_hash_byte(h, (uint8_t)(limb >> (8 * j)));
        }
        break;
    case V_BOOL:
        h = collection_hash_byte(h, key->as.boolean);
        break;
    case V_UNIT:
    case V_VOID:
        break;
    default:
        return false;
    }
    *hash = h;
    return true;
}

static CollectionIndex *collection_index_build(const Value *collection, size_t stride)
{
    size_t entries = collection->as.sequence.count / stride;
    if (entries > SIZE_MAX / 2) return NULL;
    size_t capacity = 1;
    while (capacity < entries * 2) {
        if (capacity > SIZE_MAX / 2) return NULL;
        capacity *= 2;
    }
    if (capacity > (SIZE_MAX - sizeof(CollectionIndex)) / sizeof(size_t)) return NULL;
    CollectionIndex *index = calloc(1, sizeof(*index) + capacity * sizeof(size_t));
    if (!index) return NULL;
    index->capacity = capacity;
    for (size_t i = 0; i < collection->as.sequence.count; i += stride) {
        Value *key = collection->as.sequence.items[i];
        uint64_t hash;
        if (!collection_hash(key, &hash)) {
            free(index);
            return NULL;
        }
        size_t bucket = (size_t)hash & (capacity - 1);
        while (index->positions[bucket]) {
            size_t prior = index->positions[bucket] - 1;
            if (value_equal(collection->as.sequence.items[prior], key)) break;
            bucket = (bucket + 1) & (capacity - 1);
        }
        /* Preserve last map binding and first set occurrence, even in native fixtures. */
        if (stride == 2 || !index->positions[bucket]) index->positions[bucket] = i + 1;
    }
    return index;
}

size_t value_collection_find(Value *collection, const Value *key)
{
    Sequence *sequence = &collection->as.sequence;
    size_t stride = collection->kind == V_MAP ? 2 : 1;
    /* Trial thresholds: avoid allocating for small or briefly queried values. */
    if (!sequence->index && sequence->count / stride >= 32 && sequence->lookup_count != SIZE_MAX) {
        if (++sequence->lookup_count >= 8) {
            sequence->index = collection_index_build(collection, stride);
            if (!sequence->index) sequence->lookup_count = SIZE_MAX;
        }
    }
    uint64_t hash;
    if (sequence->index && collection_hash(key, &hash)) {
        CollectionIndex *index = sequence->index;
        size_t bucket = (size_t)hash & (index->capacity - 1);
        while (index->positions[bucket]) {
            size_t position = index->positions[bucket] - 1;
            if (value_equal(sequence->items[position], key)) return position;
            bucket = (bucket + 1) & (index->capacity - 1);
        }
        return SIZE_MAX;
    }
    if (stride == 2) {
        for (size_t i = sequence->count; i >= 2; i -= 2) {
            if (value_equal(sequence->items[i - 2], key)) return i - 2;
        }
    } else {
        for (size_t i = 0; i < sequence->count; i++) {
            if (value_equal(sequence->items[i], key)) return i;
        }
    }
    return SIZE_MAX;
}

bool value_equal(const Value *a, const Value *b)
{
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
    case V_NAT:
    case V_INT:
    case V_DURATION:
    case V_INSTANT:
        return pn_big_compare(&a->as.integer, &b->as.integer) == 0;
    case V_RAT:
        return pn_big_compare(&a->as.rational.numerator, &b->as.rational.numerator) == 0 &&
               pn_big_compare(&a->as.rational.denominator, &b->as.rational.denominator) == 0;
    case V_DEC:
        return decimal_compare(&a->as.decimal, &b->as.decimal) == 0;
    case V_STR:
    case V_BYTES:
    case V_PATH:
        return a->as.bytes.length == b->as.bytes.length &&
               !memcmp(a->as.bytes.data, b->as.bytes.data, a->as.bytes.length);
    case V_BOOL:
        return a->as.boolean == b->as.boolean;
    case V_UNIT:
    case V_VOID:
        return true;
    case V_ARRAY:
    case V_SET:
    case V_MAP:
        if (a->as.sequence.count != b->as.sequence.count) {
            return false;
        }
        for (size_t i = 0; i < a->as.sequence.count; i++) {
            if (!value_equal(a->as.sequence.items[i], b->as.sequence.items[i])) {
                return false;
            }
        }
        return true;
    case V_RECORD:
    case V_VARIANT:
        if (strcmp(a->as.named.name, b->as.named.name) || a->as.named.count != b->as.named.count) {
            return false;
        }
        for (size_t i = 0; i < a->as.named.count; i++) {
            if (!value_equal(a->as.named.values[i], b->as.named.values[i])) {
                return false;
            }
        }
        return true;
    default:
        return a == b;
    }
}

char *copy_text(const char *text_value)
{
    size_t n = strlen(text_value);
    char *copy = malloc(n + 1);
    if (copy) {
        memcpy(copy, text_value, n + 1);
    }
    return copy;
}

bool value_index(const Value *v, size_t *index)
{
    return v->kind == V_NAT && pn_big_fits_size(&v->as.integer, index);
}

Value *named_value(ValueKind kind, const char *name, char **names, Value **values, size_t count)
{
    Value *v = value_new(kind);
    if (!v) {
        return NULL;
    }
    v->as.named.name = copy_text(name);
    v->as.named.count = count;
    v->as.named.names = kind == V_RECORD ? calloc(count, sizeof(char *)) : NULL;
    v->as.named.values = calloc(count, sizeof(Value *));
    if (!v->as.named.name || (count && !v->as.named.values) ||
        (kind == V_RECORD && count && !v->as.named.names)) {
        release(v);
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        if (kind == V_RECORD) {
            v->as.named.names[i] = copy_text(names[i]);
            if (!v->as.named.names[i]) {
                release(v);
                return NULL;
            }
        }
        v->as.named.values[i] = retain(values[i]);
    }
    return v;
}
