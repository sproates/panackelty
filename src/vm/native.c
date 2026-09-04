#include "bigint.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern char   **environ;

#define MAX_ARTIFACT (16u * 1024u * 1024u)
#define MAX_FUNCTIONS 4096u
#define MAX_INSTRUCTIONS 4000000u
#define MAX_NAME 1024u
#define MAX_TEXT (1024u * 1024u)
#define MAX_DIGITS 4096u

typedef struct {
	const		uint8_t *data;
	size_t		len, at;
	const char     *error;
} Reader;
typedef struct {
	uint8_t		tag;
	PnBigInt	number;
	int16_t		exponent;
	char	       *text;
	size_t		text_length;
	bool		boolean;
} Constant;
typedef struct {
	uint8_t		op, code, arity;
	uint32_t	target;
	char	       *name, *name2;
	size_t		count;
	char	      **items;
	Constant	constant;
} Ins;
typedef struct {
	char	       *name;
	bool		pure;
	size_t		param_count, ins_count;
	char	      **params;
	Ins	       *ins;
} Fn;
typedef struct {
	size_t		count;
	Fn	       *functions;
} Program;

typedef enum {
	V_NAT, V_INT, V_DEC, V_STR, V_BOOL, V_VOID, V_RANGE, V_ARRAY, V_RECORD, V_VARIANT, V_MAP, V_SET, V_BYTES, V_ITER
} ValueKind;
typedef struct Value Value;
typedef struct {
	size_t		count;
	Value	      **items;
} Sequence;
typedef struct {
	PnBigInt	coefficient;
	int		exponent;
	bool		negative_zero;
} Decimal;
typedef struct {
	char	       *name;
	size_t		count;
	char	      **names;
	Value	      **values;
} NamedValues;
typedef struct {
	Value	       *iterable;
	size_t		index;
	PnBigInt	cursor;
} Iterator;
struct Value {
	size_t		refs;
	ValueKind	kind;
	union {
		PnBigInt	integer;
		Decimal		decimal;
		struct {
			size_t		length;
			uint8_t	       *data;
		}		bytes;
		bool		boolean;
		struct {
			PnBigInt	start, end;
		}		range;
		Sequence	sequence;
		NamedValues	named;
		Iterator	iterator;
	}		as;
};

static Value * value_new(ValueKind kind) {
	Value	       *v = calloc(1, sizeof(*v));
	if (v) {
		v->refs = 1;
		v->kind = kind;
	} return v;
}
static Value * retain(Value * v) {
	if (v)
		v->refs++;
	return v;
}
static void	release(Value * v) {
	if (!v || --v->refs)
		return;
	switch (v->kind) {
	case V_NAT:
	case V_INT:
		pn_big_free(&v->as.integer);
		break;
	case V_DEC:
		pn_big_free(&v->as.decimal.coefficient);
		break;
	case V_STR:
	case V_BYTES:
		free(v->as.bytes.data);
		break;
	case V_RANGE:
		pn_big_free(&v->as.range.start);
		pn_big_free(&v->as.range.end);
		break;
	case V_ARRAY:
	case V_MAP:
	case V_SET:
		for (size_t i = 0; i < v->as.sequence.count; i++)
			release(v->as.sequence.items[i]);
		free(v->as.sequence.items);
		break;
	case V_RECORD:
	case V_VARIANT:
		for (size_t i = 0; i < v->as.named.count; i++) {
			free(v->as.named.names ? v->as.named.names[i] : NULL);
			release(v->as.named.values[i]);
		} free(v->as.named.name);
		free(v->as.named.names);
		free(v->as.named.values);
		break;
	case V_ITER:
		release(v->as.iterator.iterable);
		pn_big_free(&v->as.iterator.cursor);
		break;
	default:
		break;
	} free(v);
}
static Value * value_bool(bool value){
	Value	       *v = value_new(V_BOOL);
	if (v)
		v->as.boolean = value;
	return v;
}
static Value * value_void(void){
	return value_new(V_VOID);
}
static Value * value_big(ValueKind kind, const PnBigInt * number){
	Value	       *v = value_new(kind);
	if (!v)
		return NULL;
	if (!pn_big_copy(&v->as.integer, number)) {
		free(v);
		return NULL;
	} return v;
}
static Value * value_size(size_t number) {
	PnBigInt	n;
	if (!pn_big_from_u64(&n, number))
		return NULL;
	Value	       *v = value_big(V_NAT, &n);
	pn_big_free(&n);
	return v;
}
static Value * value_data(ValueKind kind, const uint8_t * data, size_t length){
	Value	       *v = value_new(kind);
	if (!v)
		return NULL;
	v->as.bytes.data = malloc(length + 1);
	if (!v->as.bytes.data) {
		free(v);
		return NULL;
	} if (length)
		memcpy(v->as.bytes.data, data, length);
	v->as.bytes.data[length] = 0;
	v->as.bytes.length = length;
	return v;
}
static Value * value_sequence(ValueKind kind, Value * *items, size_t count) {
	Value	       *v = value_new(kind);
	if (!v)
		return NULL;
	v->as.sequence.items = calloc(count, sizeof(Value *));
	if (count && !v->as.sequence.items) {
		free(v);
		return NULL;
	} v->as.sequence.count = count;
	for (size_t i = 0; i < count; i++)
		v->as.sequence.items[i] = retain(items[i]);
	return v;
}
static int	decimal_compare(const Decimal * a, const Decimal * b);
static bool	value_equal(const Value * a, const Value * b){
	if (a->kind != b->kind)
		return false;
	switch (a->kind) {
	case V_NAT:
	case V_INT:
		return pn_big_compare(&a->as.integer, &b->as.integer) == 0;
	case V_DEC:
		return decimal_compare(&a->as.decimal, &b->as.decimal) == 0;
	case V_STR:
	case V_BYTES:
		return a->as.bytes.length == b->as.bytes.length && !memcmp(a->as.bytes.data, b->as.bytes.data, a->as.bytes.length);
	case V_BOOL:
		return a->as.boolean == b->as.boolean;
	case V_VOID:
		return true;
	case V_ARRAY:
	case V_SET:
	case V_MAP:
		if (a->as.sequence.count != b->as.sequence.count)
			return false;
		for (size_t i = 0; i < a->as.sequence.count; i++)
			if (!value_equal(a->as.sequence.items[i], b->as.sequence.items[i]))
				return false;
		return true;
	case V_RECORD:
	case V_VARIANT:
		if (strcmp(a->as.named.name, b->as.named.name) || a->as.named.count != b->as.named.count)
			return false;
		for (size_t i = 0; i < a->as.named.count; i++)
			if (!value_equal(a->as.named.values[i], b->as.named.values[i]))
				return false;
		return true;
	default:
		return a == b;
	}
}

typedef struct {
	char	       *data;
	size_t		length, capacity;
} Buffer;
static bool	buffer_add(Buffer * b, const char *s, size_t n){
	if (n > SIZE_MAX - b->length - 1)
		return false;
	size_t		need = b->length + n + 1;
	if (need > b->capacity) {
		size_t		cap = b->capacity ? b->capacity : 64;
		while (cap < need) {
			if (cap > SIZE_MAX / 2)
				return false;
			cap *= 2;
		} char	       *next = realloc(b->data, cap);
		if (!next)
			return false;
		b->data = next;
		b->capacity = cap;
	} memcpy(b->data + b->length, s, n);
	b->length += n;
	b->data[b->length] = 0;
	return true;
}
static bool	buffer_text(Buffer * b, const char *s){
	return buffer_add(b, s, strlen(s));
}
static bool	render(Buffer * b, const Value * v, bool display);
static bool	render_decimal(Buffer * b, const Decimal * d){
	char	       *raw = pn_big_string(&d->coefficient);
	if (!raw)
		return false;
	char	       *digits = raw;
	bool		negative = raw[0] == '-' || d->negative_zero;
	if (raw[0] == '-')
		digits++;
	size_t		n = strlen(digits);
	bool		ok = true;
	if (negative)
		ok = buffer_text(b, "-");
	if (ok && d->exponent >= 0) {
		ok = buffer_text(b, digits);
		for (int i = 0; ok && i < d->exponent; i++)
			ok = buffer_text(b, "0");
	} else if (ok) {
		size_t		places = (size_t) (-d->exponent);
		if (places < n) {
			ok = buffer_add(b, digits, n - places) && buffer_text(b, ".") && buffer_add(b, digits + n - places, places);
		} else {
			ok = buffer_text(b, "0.");
			for (size_t i = n; ok && i < places; i++)
				ok = buffer_text(b, "0");
			if (ok)
				ok = buffer_text(b, digits);
		}
	} free(raw);
	return ok;
}
static bool	render(Buffer * b, const Value * v, bool display){
	char	       *number = NULL;
	switch (v->kind) {
	case V_NAT:
	case V_INT:
		number = pn_big_string(&v->as.integer);
		if (!number)
			return false;
		{
			bool ok = buffer_text(b, number);
			free(number);
			return ok;
		}
	case V_DEC:
		return render_decimal(b, &v->as.decimal);
	case V_STR:
			if (display && !buffer_text(b, "\""))
				return false;
		if (!buffer_add(b, (char *)v->as.bytes.data, v->as.bytes.length))
			return false;
		return !display || buffer_text(b, "\"");
	case V_BOOL:
		return buffer_text(b, v->as.boolean ? "true" : "false");
	case V_VOID:
		return buffer_text(b, "void");
	case V_BYTES:{
			if (!buffer_text(b, "bytes("))
				return false;
			char		hex[3];
			for (size_t i = 0; i < v->as.bytes.length; i++) {
				snprintf(hex, sizeof(hex), "%02x", v->as.bytes.data[i]);
				if (!buffer_text(b, hex))
					return false;
			} return buffer_text(b, ")");
	} case V_ARRAY:
	case V_SET:{
			if (!buffer_text(b, v->kind == V_SET ? "set{" : "["))
				return false;
			for (size_t i = 0; i < v->as.sequence.count; i++) {
				if (i && !buffer_text(b, ", "))
					return false;
				if (!render(b, v->as.sequence.items[i], true))
					return false;
			} return buffer_text(b, v->kind == V_SET ? "}" : "]");
	} case V_MAP:
		if (!buffer_text(b, "{"))
			return false;
		for (size_t i = 0; i < v->as.sequence.count; i += 2) {
			if (i && !buffer_text(b, ", "))
				return false;
			if (!render(b, v->as.sequence.items[i], true) || !buffer_text(b, ": ") || !render(b, v->as.sequence.items[i + 1], true))
				return false;
		} return buffer_text(b, "}");
	case V_VARIANT:
		if (!buffer_text(b, v->as.named.name) || !buffer_text(b, "("))
			return false;
		for (size_t i = 0; i < v->as.named.count; i++) {
			if (i && !buffer_text(b, ", "))
				return false;
			if (!render(b, v->as.named.values[i], true))
				return false;
		} return buffer_text(b, ")");
	case V_RECORD:
		if (!buffer_text(b, v->as.named.name) || !buffer_text(b, "("))
			return false;
		for (size_t i = 0; i < v->as.named.count; i++) {
			if (i && !buffer_text(b, ", "))
				return false;
			if (!buffer_text(b, v->as.named.names[i]) || !buffer_text(b, ": ") || !render(b, v->as.named.values[i], true))
				return false;
		} return buffer_text(b, ")");
	default:
		return buffer_text(b, "<value>");
	}
}

static bool
big_abs_copy(PnBigInt * out, const PnBigInt * in)
{
	if (!pn_big_copy(out, in))
		return false;
	if (out->len)
		out->sign = 1;
	return true;
}
static bool	big_gcd(PnBigInt * out, const PnBigInt * left, const PnBigInt * right){
	PnBigInt	a, b;
	if (!big_abs_copy(&a, left) || !big_abs_copy(&b, right))
		return false;
	while (!pn_big_is_zero(&b)) {
		PnBigInt	q, r;
		if (!pn_big_divmod(&q, &r, &a, &b)) {
			pn_big_free(&a);
			pn_big_free(&b);
			return false;
		} pn_big_free(&q);
		pn_big_free(&a);
		a = b;
		b = r;
		if (b.len)
			b.sign = 1;
	} pn_big_free(&b);
	*out = a;
	return true;
}
static bool	decimal_align(const Decimal * a, const Decimal * b, PnBigInt * left, PnBigInt * right, int *exponent){
	*exponent = a->exponent < b->exponent ? a->exponent : b->exponent;
	if (!pn_big_copy(left, &a->coefficient) || !pn_big_copy(right, &b->coefficient))
		return false;
	if (!pn_big_pow10(left, (size_t) (a->exponent - *exponent)) || !pn_big_pow10(right, (size_t) (b->exponent - *exponent))) {
		pn_big_free(left);
		pn_big_free(right);
		return false;
	} return true;
}
static Value * value_decimal(PnBigInt * coefficient, int exponent){
	Value	       *v = value_new(V_DEC);
	if (!v)
		return NULL;
	v->as.decimal.coefficient = *coefficient;
	v->as.decimal.exponent = exponent;
	pn_big_init(coefficient);
	return v;
}
static Value * decimal_binary(uint8_t op, const Decimal * a, const Decimal * b, const char **error){
	PnBigInt	left, right, result, q, r;
	int		exponent;
	if (op == 0 || op == 1 || op == 4) {
		if (!decimal_align(a, b, &left, &right, &exponent))
			return NULL;
		if (op == 0) {
			if (!pn_big_add(&result, &left, &right))
				goto fail_align;
		} else if (op == 1) {
			if (!pn_big_sub(&result, &left, &right))
				goto fail_align;
		} else {
			if (pn_big_is_zero(&right)) {
				*error = "VM trap: division by zero";
				goto fail_align;
			} if (!pn_big_divmod(&q, &r, &left, &right))
				goto fail_align;
			pn_big_free(&q);
			result = r;
		} pn_big_free(&left);
		pn_big_free(&right);
		return value_decimal(&result, exponent);
	}
	if (op == 2) {
		if (!pn_big_mul(&result, &a->coefficient, &b->coefficient))
			return NULL;
		return value_decimal(&result, a->exponent + b->exponent);
	}
	if (op == 3) {
		if (pn_big_is_zero(&b->coefficient)) {
			*error = "VM trap: division by zero";
			return NULL;
		} PnBigInt	gcd, den, num;
		if (!big_gcd(&gcd, &a->coefficient, &b->coefficient) || !pn_big_divmod(&num, &r, &a->coefficient, &gcd)) {
			return NULL;
		} pn_big_free(&r);
		if (!pn_big_divmod(&den, &r, &b->coefficient, &gcd)) {
			pn_big_free(&num);
			return NULL;
		} pn_big_free(&r);
		pn_big_free(&gcd);
		if (den.sign < 0) {
			den.sign = -den.sign;
			num.sign = -num.sign;
		} size_t	twos = 0, fives = 0;
		while (!pn_big_is_one(&den)) {
			PnBigInt	probe;
			if (!pn_big_copy(&probe, &den)) {
				pn_big_free(&num);
				pn_big_free(&den);
				return NULL;
			} uint32_t	rem = pn_big_div_small(&probe, 2);
			if (!rem) {
				pn_big_free(&den);
				den = probe;
				twos++;
				continue;
			} pn_big_free(&probe);
			if (!pn_big_copy(&probe, &den)) {
				pn_big_free(&num);
				pn_big_free(&den);
				return NULL;
			} rem = pn_big_div_small(&probe, 5);
			if (!rem) {
				pn_big_free(&den);
				den = probe;
				fives++;
				continue;
			} pn_big_free(&probe);
			*error = "VM trap: non-terminating decimal division requires explicit rounding";
			pn_big_free(&num);
			pn_big_free(&den);
			return NULL;
		}
		pn_big_free(&den);
		size_t		places = twos > fives ? twos : fives;
		for (size_t i = twos; i < places; i++) {
			if (!pn_big_mul_small(&num, 2)) {
				pn_big_free(&num);
				return NULL;
			}
		}
		for (size_t i = fives; i < places; i++) {
			if (!pn_big_mul_small(&num, 5)) {
				pn_big_free(&num);
				return NULL;
			}
		}
		return value_decimal(&num, a->exponent - b->exponent - (int)places);
	}
	return NULL;
fail_align:pn_big_free(&left);
	pn_big_free(&right);
	return NULL;
}
static int	decimal_compare(const Decimal * a, const Decimal * b){
	PnBigInt	left, right;
	int		exponent;
	if (!decimal_align(a, b, &left, &right, &exponent))
		return 0;
	(void)exponent;
	int		result = pn_big_compare(&left, &right);
	pn_big_free(&left);
	pn_big_free(&right);
	return result;
}

typedef struct {
	const char     *name;
	uint8_t		arity;
	bool		pure;
} Builtin;
static const	Builtin BUILTINS[] = {
	{"print", 1, false}, {"read_line", 0, false}, {"read_file", 1, false}, {"write_file", 2, false},
	{"len", 1, true}, {"append", 2, true}, {"concat", 2, true}, {"slice", 3, true},
	{"starts_with", 2, true}, {"starts_with_at", 3, true}, {"reverse", 1, true}, {"is_digit", 1, true},
	{"is_letter", 1, true}, {"is_whitespace", 1, true}, {"map", 0, true}, {"map_put", 3, true},
	{"map_has", 2, true}, {"map_get", 2, true}, {"$method_put", 3, true}, {"$method_has", 2, true},
	{"$method_get", 2, true}, {"set", 0, true}, {"set_add", 2, true}, {"set_has", 2, true},
	{"$method_add", 2, true}, {"bytes", 0, true}, {"byte_append", 2, true}, {"bytes_concat", 2, true},
	{"byte_len", 1, true}, {"byte_get", 2, true}, {"utf8_encode", 1, true}, {"utf8_decode", 1, true},
	{"read_bytes", 1, false}, {"write_bytes", 2, false}, {"nat_from_str", 1, true},
	{"command_args", 0, false}, {"environment_has", 1, false}, {"environment_get", 1, false},
	{"eprint", 1, false}, {"process_exit", 1, false}, {"path_resolve", 1, false},
	{"path_parent", 1, true}, {"path_join", 2, true}, {"path_suffix", 1, true},
	{"path_with_suffix", 2, true}, {"path_is_absolute", 1, true}, {"file_exists", 1, false},
	{"run_bytecode", 1, false}, {"run_bytecode_args", 2, false}
};

static void
fail(Reader * r, const char *message)
{
	if (!r->error)
		r->error = message;
}
static bool
take(Reader * r, size_t n, const uint8_t * *out)
{
	if (n > r->len - r->at) {
		fail(r, "truncated data");
		return false;
	}
	*out = r->data + r->at;
	r->at += n;
	return true;
}
static bool
u8(Reader * r, uint8_t * out)
{
	const		uint8_t *p;
	if (!take(r, 1, &p))
		return false;
	*out = p[0];
	return true;
}
static bool	u16(Reader * r, uint16_t * out) {
	const		uint8_t *p;
	if (!take(r, 2, &p))
		return false;
	*out = (uint16_t) ((p[0] << 8) | p[1]);
	return true;
}
static bool
u32(Reader * r, uint32_t * out)
{
	const		uint8_t *p;
	if (!take(r, 4, &p))
		return false;
	*out = ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3];
	return true;
}

static bool
utf8(const uint8_t * s, size_t n)
{
	size_t		i = 0;
	while (i < n) {
		uint32_t	c = s[i++];
		size_t		need = 0;
		uint32_t	min = 0;
		if (c < 0x80)
			continue;
		if ((c & 0xe0) == 0xc0) {
			need = 1;
			c &= 0x1f;
			min = 0x80;
		} else if ((c & 0xf0) == 0xe0) {
			need = 2;
			c &= 0x0f;
			min = 0x800;
		} else if ((c & 0xf8) == 0xf0) {
			need = 3;
			c &= 7;
			min = 0x10000;
		} else
			return false;
		if (need > n - i)
			return false;
		while (need--) {
			if ((s[i] & 0xc0) != 0x80)
				return false;
			c = (c << 6) | (s[i++] & 0x3f);
		}
		if (c < min || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff))
			return false;
	} return true;
}
static char *
text(Reader * r, bool name)
{
	uint32_t	n32 = 0;
	uint16_t	n16 = 0;
	size_t		n;
	if (name) {
		if (!u16(r, &n16))
			return NULL;
		n = n16;
		if (n > MAX_NAME) {
			fail(r, "name exceeds limit");
			return NULL;
		}
	} else {
		if (!u32(r, &n32))
			return NULL;
		n = n32;
		if (n > MAX_TEXT) {
			fail(r, "text exceeds limit");
			return NULL;
		}
	}
	const		uint8_t *p;
	if (!take(r, n, &p))
		return NULL;
	if (!utf8(p, n)) {
		fail(r, "invalid UTF-8");
		return NULL;
	}
	char	       *v = malloc(n + 1);
	if (!v) {
		fail(r, "out of memory");
		return NULL;
	} memcpy(v, p, n);
	v[n] = 0;
	return v;
}
static bool	read_nat(Reader * r, PnBigInt * out) {
	uint16_t	n;
	const		uint8_t *p;
	pn_big_init(out);
	if (!u16(r, &n))
		return false;
	size_t		max_bytes = (MAX_DIGITS * 3322u + 7999u) / 8000u;
	if (n > max_bytes) {
		fail(r, "integer exceeds digit limit");
		return false;
	} if (!take(r, n, &p))
		return false;
	if (n && p[0] == 0) {
		fail(r, "non-minimal integer");
		return false;
	} if (!pn_big_from_bytes(out, p, n)) {
		fail(r, "out of memory");
		return false;
	} char	       *digits = pn_big_string(out);
	if (!digits) {
		fail(r, "out of memory");
		return false;
	} if (strlen(digits) > MAX_DIGITS)
		fail(r, "integer exceeds digit limit");
	free(digits);
	return !r->error;
}
static bool	read_decimal(Reader * r, Constant * out) {
	uint8_t		sign;
	uint16_t	ex, digits;
	const		uint8_t *p;
	if (!u8(r, &sign) || !u16(r, &ex) || !u16(r, &digits))
		return false;
	out->boolean = sign != 0;
	out->exponent = (int16_t) ex;
	if (sign > 1 || !digits || out->exponent > 4096 || out->exponent < -4096 || digits > MAX_DIGITS) {
		fail(r, "invalid decimal");
		return false;
	} size_t	bytes = (digits + 1) / 2;
	if (!take(r, bytes, &p))
		return false;
	char	       *value = malloc(digits);
	if (!value) {
		fail(r, "out of memory");
		return false;
	} for (size_t i = 0; i < digits; i++) {
		uint8_t		d = (i & 1) ? p[i / 2] & 15 : p[i / 2] >> 4;
		if (d > 9) {
			free(value);
			fail(r, "invalid decimal digit");
			return false;
		} value[i] = (char)('0' + d);
	} if ((digits & 1) && (p[bytes - 1] & 15) != 15) {
		free(value);
		fail(r, "invalid decimal padding");
		return false;
	} if (digits > 1 && value[0] == '0') {
		free(value);
		fail(r, "non-minimal decimal coefficient");
		return false;
	} bool		ok = pn_big_from_digits(&out->number, value, digits, sign ? -1 : 1);
	free(value);
	if (!ok)
		fail(r, "out of memory");
	return ok;
}
static bool	read_constant(Reader * r, Constant * out) {
	uint8_t		sign;
	memset(out, 0, sizeof(*out));
	pn_big_init(&out->number);
	if (!u8(r, &out->tag))
		return false;
	switch (out->tag) {
	case 0:
		return read_nat(r, &out->number);
	case 1:
		if (!u8(r, &sign) || sign > 1) {
			fail(r, "invalid integer sign");
			return false;
		} if (!read_nat(r, &out->number))
			return false;
		if (sign && pn_big_is_zero(&out->number)) {
			fail(r, "negative zero integer");
			return false;
		} if (sign)
			out->number.sign = -1;
		return true;
	case 2:
		return read_decimal(r, out);
	case 3: {
		size_t		start = r->at;
		out->text = text(r, false);
		if (out->text)
			out->text_length = r->at - start - 4;
		return out->text != NULL;
	}
	case 4:
		if (!u8(r, &sign) || sign > 1) {
			fail(r, "invalid Bool constant");
			return false;
		} out->boolean = sign != 0;
		return true;
	case 5:
		return true;
	default:
		fail(r, "unknown constant tag");
		return false;
	}
}
static char **	strings(Reader * r, uint16_t count, bool names){
	char	      **items = calloc(count, sizeof(char *));
	if (count && !items) {
		fail(r, "out of memory");
		return NULL;
	} for (uint16_t i = 0; i < count && !r->error; i++)
		items[i] = text(r, names);
	if (r->error) {
		for (uint16_t i = 0; i < count; i++)
			free(items[i]);
		free(items);
		return NULL;
	} return items;
}
static bool
instruction(Reader * r, Ins * in)
{
	uint16_t	count;
	uint32_t	ignored;
	memset(in, 0, sizeof(*in));
	if (!u8(r, &in->op))
		return false;
	switch (in->op) {
	case 0:
		return read_constant(r, &in->constant);
	case 1:
	case 2:
	case 10:
	case 13:
		in->name = text(r, true);
		return !r->error;
	case 3:
	case 6:
	case 8:
	case 16:
	case 20:
		return true;
	case 21:
		return u8(r, &in->arity);
	case 4:
		if (!u8(r, &in->code) || in->code > 1) {
			fail(r, "unknown unary operator");
			return false;
		} return true;
	case 5:
		if (!u8(r, &in->code) || in->code > 12) {
			fail(r, "unknown binary operator");
			return false;
		} return true;
	case 7:
		if (!u16(r, &count))
			return false;
		in->count = count;
		return true;
	case 9:
		if (!u16(r, &count))
			return false;
		in->count = count;
		in->items = strings(r, count, false);
		return !r->error;
	case 11:
		in->name = text(r, true);
		in->name2 = text(r, true);
		return in->name && in->name2 && u32(r, &in->target);
	case 12:
		in->name = text(r, true);
		if (!in->name || !u16(r, &count))
			return false;
		in->count = count;
		in->items = strings(r, count, true);
		return !r->error;
	case 14:
		in->name = text(r, true);
		in->name2 = text(r, true);
		if (!in->name || !in->name2 || !u16(r, &count))
			return false;
		in->count = count;
		return true;
	case 15:
		in->name = text(r, true);
		return in->name && u32(r, &in->target);
	case 17:
		in->name = text(r, true);
		return in->name && u8(r, &in->arity);
	case 18:
	case 19:
		return u32(r, &in->target);
	default:
		(void)ignored;
		fail(r, "unknown bytecode opcode");
		return false;
	}
}
static void	free_program(Program * p) {
	if (!p)
		return;
	for (size_t i = 0; i < p->count; i++) {
		Fn	       *f = &p->functions[i];
		free(f->name);
		for (size_t j = 0; j < f->param_count; j++)
			free(f->params[j]);
		free(f->params);
		for (size_t j = 0; j < f->ins_count; j++) {
			Ins	       *in = &f->ins[j];
			free(in->name);
			free(in->name2);
			for (size_t k = 0; k < in->count; k++)
				free(in->items ? in->items[k] : NULL);
			free(in->items);
			pn_big_free(&in->constant.number);
			free(in->constant.text);
		} free(f->ins);
	} free(p->functions);
	memset(p, 0, sizeof(*p));
}
static bool
decode(const uint8_t * data, size_t length, Program * p, const char **error)
{
	memset(p, 0, sizeof(*p));
	Reader		r = {data, length, 0, NULL};
	const		uint8_t *magic;
	uint16_t	version, count;
	if (length > MAX_ARTIFACT) {
		*error = "artifact exceeds size limit";
		return false;
	} if (!take(&r, 9, &magic) || memcmp(magic, "PANACKBC\0", 9)) {
		*error = "not a Panackelty bytecode file";
		return false;
	}
	if (!u16(&r, &version)) {
		*error = r.error;
		return false;
	} if (version != 7) {
		*error = "unsupported bytecode version";
		return false;
	} if (!u16(&r, &count)) {
		*error = r.error;
		return false;
	} if (count > MAX_FUNCTIONS) {
		*error = "function count exceeds limit";
		return false;
	}
	p->count = count;
	p->functions = calloc(count, sizeof(Fn));
	if (count && !p->functions) {
		*error = "out of memory";
		return false;
	} size_t	total = 0;
	for (size_t i = 0; i < count && !r.error; i++) {
		Fn	       *f = &p->functions[i];
		uint8_t		flags, params;
		uint32_t	ins;
		f->name = text(&r, true);
		if (!f->name || !u8(&r, &flags) || flags > 1 || !u8(&r, &params)) {
			if (!r.error)
				fail(&r, "invalid function flags");
			break;
		} f->pure = flags == 1;
		f->param_count = params;
		f->params = calloc(params, sizeof(char *));
		for (size_t j = 0; j < params && !r.error; j++)
			f->params[j] = text(&r, true);
		if (!u32(&r, &ins))
			break;
		if (ins > 1000000u || total + ins > MAX_INSTRUCTIONS) {
			fail(&r, "instruction count exceeds limit");
			break;
		} total += ins;
		f->ins_count = ins;
		f->ins = calloc(ins, sizeof(Ins));
		if (ins && !f->ins) {
			fail(&r, "out of memory");
			break;
		} for (size_t j = 0; j < ins && !r.error; j++)
			instruction(&r, &f->ins[j]);
	}
	if (!r.error && r.at != r.len)
		fail(&r, "trailing data");
	if (r.error) {
		*error = r.error;
		free_program(p);
		return false;
	} return true;
}
static const	Builtin *builtin(const char *name){
	for (size_t i = 0; i < sizeof(BUILTINS) / sizeof(*BUILTINS); i++)
		if (!strcmp(name, BUILTINS[i].name))
			return &BUILTINS[i];
	return NULL;
}
static Fn * function(Program * p, const char *name){
	for (size_t i = 0; i < p->count; i++)
		if (!strcmp(name, p->functions[i].name))
			return &p->functions[i];
	return NULL;
}
static bool	verify(Program * p, const char **error){
	Fn	       *main = function(p, "main");
	if (!main) {
		*error = "bytecode has no main function";
		return false;
	} if (main->param_count) {
		*error = "main must not have parameters";
		return false;
	}
	for (size_t i = 0; i < p->count; i++) {
		Fn	       *f = &p->functions[i];
		if (!f->name[0]) {
			*error = "invalid function signature";
			return false;
		} if (i && strcmp(p->functions[i - 1].name, f->name) >= 0) {
			*error = "functions are not in canonical order";
			return false;
		}
		for (size_t a = 0; a < f->param_count; a++) {
			for (size_t b = a + 1; b < f->param_count; b++) {
				if (!strcmp(f->params[a], f->params[b])) {
					*error = "invalid function signature";
					return false;
				}
			}
		}
		if (!f->ins_count) {
			*error = "bytecode function is empty";
			return false;
		} bool		returned = false;
		for (size_t j = 0; j < f->ins_count; j++) {
			Ins	       *in = &f->ins[j];
			if (in->op == 20)
				returned = true;
			if (in->op == 9 && !in->count) {
				*error = "invalid INTERPOLATE operand";
				return false;
			} if ((in->op == 11 || in->op == 15 || in->op == 18 || in->op == 19) && in->target >= f->ins_count) {
				*error = "invalid jump target";
				return false;
			} if (in->op == 17) {
				const		Builtin *b = builtin(in->name);
				Fn	       *called = function(p, in->name);
				if (!b && !called) {
					*error = "call to unknown function";
					return false;
				} size_t	arity = b ? b->arity : called->param_count;
				if (in->arity != arity) {
					*error = "call arity mismatch";
					return false;
				} if (f->pure && !((b && b->pure) || (called && called->pure))) {
					*error = "pure function calls impure function";
					return false;
				}
			}
		}
		if (!returned) {
			*error = "bytecode function has no RETURN";
			return false;
		}
	} return true;
}

typedef struct {
	char	       *name;
	Value	       *value;
} Local;
typedef struct {
	Program	       *program;
	int		argc;
	char	      **argv;
	size_t		env_count;
	char	      **environment;
	const char     *error;
} VM;
typedef struct {
	Fn	       *function;
	size_t		pc, stack_count, stack_capacity, local_count, local_capacity;
	Value	      **stack;
	Local	       *locals;
} Frame;
static char    *copy_text(const char *text_value){
	size_t		n = strlen(text_value);
	char	       *copy = malloc(n + 1);
	if (copy)
		memcpy(copy, text_value, n + 1);
	return copy;
}
static bool	stack_push(Frame * f, Value * v) {
	if (f->stack_count == f->stack_capacity) {
		size_t		cap = f->stack_capacity ? f->stack_capacity * 2 : 16;
		Value	      **next = realloc(f->stack, cap * sizeof(Value *));
		if (!next)
			return false;
		f->stack = next;
		f->stack_capacity = cap;
	} f->stack[f->stack_count++] = v;
	return true;
}
static Value * stack_pop(Frame * f, VM * vm) {
	if (!f->stack_count) {
		vm->error = "VM trap: operand stack underflow";
		return NULL;
	} return f->stack[--f->stack_count];
}
static Value * local_get(Frame * f, const char *name){
	for (size_t i = f->local_count; i-- > 0;)
		if (!strcmp(f->locals[i].name, name))
			return f->locals[i].value;
	return NULL;
}
static bool	local_put(Frame * f, const char *name, Value * value){
	for (size_t i = 0; i < f->local_count; i++) {
		if (!strcmp(f->locals[i].name, name)) {
			release(f->locals[i].value);
			f->locals[i].value = value;
			return true;
		}
	}
	if (f->local_count == f->local_capacity) {
		size_t		cap = f->local_capacity ? f->local_capacity * 2 : 16;
		Local	       *next = realloc(f->locals, cap * sizeof(Local));
		if (!next)
			return false;
		f->locals = next;
		f->local_capacity = cap;
	} f->locals[f->local_count].name = copy_text(name);
	if (!f->locals[f->local_count].name)
		return false;
	f->locals[f->local_count++].value = value;
	return true;
}
static void	frame_free(Frame * f) {
	for (size_t i = 0; i < f->stack_count; i++)
		release(f->stack[i]);
	for (size_t i = 0; i < f->local_count; i++) {
		free(f->locals[i].name);
		release(f->locals[i].value);
	} free(f->stack);
	free(f->locals);
}
static size_t utf8_count(const uint8_t * s, size_t n){
	size_t		count = 0;
	for (size_t i = 0; i < n; i++)
		if ((s[i] & 0xc0) != 0x80)
			count++;
	return count;
}
static bool
utf8_offset(const uint8_t * s, size_t n, size_t index, size_t * start, size_t * end)
{
	size_t		cp = 0, i = 0;
	while (i < n) {
		size_t		at = i;
		uint8_t		c = s[i++];
		if (c >= 0x80)
			i += (c & 0xe0) == 0xc0 ? 1 : (c & 0xf0) == 0xe0 ? 2 : 3;
		if (cp++ == index) {
			*start = at;
			*end = i;
			return true;
		}
	} return false;
}
static bool	comparable(const Value * a, const Value * b){
	return (a->kind == V_DEC && b->kind == V_DEC) || ((a->kind == V_NAT || a->kind == V_INT) && (b->kind == V_NAT || b->kind == V_INT)) || (a->kind == V_STR && b->kind == V_STR);
}
static int	compare_values(const Value * a, const Value * b){
	if (a->kind == V_DEC && b->kind == V_DEC)
		return decimal_compare(&a->as.decimal, &b->as.decimal);
	if ((a->kind == V_NAT || a->kind == V_INT) && (b->kind == V_NAT || b->kind == V_INT))
		return pn_big_compare(&a->as.integer, &b->as.integer);
	if (a->kind == V_STR && b->kind == V_STR) {
		size_t		n = a->as.bytes.length < b->as.bytes.length ? a->as.bytes.length : b->as.bytes.length;
		int		c = memcmp(a->as.bytes.data, b->as.bytes.data, n);
		return c ? c : (a->as.bytes.length > b->as.bytes.length) - (a->as.bytes.length < b->as.bytes.length);
	} return 0;
}
static Value * binary_value(uint8_t op, Value * a, Value * b, VM * vm) {
	if (op == 11 || op == 12) {
		if (a->kind != V_BOOL || b->kind != V_BOOL) {
			vm->error = "VM trap: Boolean operator requires Bool";
			return NULL;
		} return value_bool(op == 11 ? a->as.boolean && b->as.boolean : a->as.boolean || b->as.boolean);
	} if (op >= 5) {
		bool		result;
		if (op == 5 || op == 6)
			result = value_equal(a, b) ^ (op == 6);
		else {
			if (!comparable(a, b)) {
				vm->error = "VM trap: incompatible comparison operands";
				return NULL;
			} int		cmp = compare_values(a, b);
			result = op == 7 ? cmp < 0 : op == 8 ? cmp <= 0 : op == 9 ? cmp > 0 : cmp >= 0;
		} return value_bool(result);
	} if (op == 0 && a->kind == V_STR && b->kind == V_STR) {
		if (a->as.bytes.length > SIZE_MAX - b->as.bytes.length)
			return NULL;
		size_t		n = a->as.bytes.length + b->as.bytes.length;
		uint8_t	       *data = malloc(n);
		if (!data)
			return NULL;
		memcpy(data, a->as.bytes.data, a->as.bytes.length);
		memcpy(data + a->as.bytes.length, b->as.bytes.data, b->as.bytes.length);
		Value	       *v = value_data(V_STR, data, n);
		free(data);
		return v;
	} if (a->kind == V_DEC && b->kind == V_DEC)
		return decimal_binary(op, &a->as.decimal, &b->as.decimal, &vm->error);
	if ((a->kind == V_NAT || a->kind == V_INT) && (b->kind == V_NAT || b->kind == V_INT)) {
		PnBigInt	result, q, r;
		bool		ok = false;
		if (op == 0)
			ok = pn_big_add(&result, &a->as.integer, &b->as.integer);
		else if (op == 1)
			ok = pn_big_sub(&result, &a->as.integer, &b->as.integer);
		else if (op == 2)
			ok = pn_big_mul(&result, &a->as.integer, &b->as.integer);
		else {
			if (pn_big_is_zero(&b->as.integer)) {
				vm->error = "VM trap: division by zero";
				return NULL;
			} if (!pn_big_divmod(&q, &r, &a->as.integer, &b->as.integer))
				return NULL;
			if (op == 3) {
				result = q;
				pn_big_free(&r);
			} else {
				if (!pn_big_is_zero(&r) && a->as.integer.sign != b->as.integer.sign) {
					PnBigInt	adjusted;
					if (!pn_big_add(&adjusted, &r, &b->as.integer)) {
						pn_big_free(&q);
						pn_big_free(&r);
						return NULL;
					} pn_big_free(&r);
					r = adjusted;
				} result = r;
				pn_big_free(&q);
			} ok = true;
		} if (!ok)
			return NULL;
		ValueKind	kind = a->kind == V_INT || b->kind == V_INT ? V_INT : V_NAT;
		if (kind == V_NAT && result.sign < 0) {
			pn_big_free(&result);
			vm->error = "VM trap: Nat underflow";
			return NULL;
		} Value	       *v = value_big(kind, &result);
		pn_big_free(&result);
		return v;
	} vm->error = "VM trap: incompatible binary operands";
	return NULL;
}
static Value * constant_value(const Constant * c){
	if (c->tag == 0)
		return value_big(V_NAT, &c->number);
	if (c->tag == 1)
		return value_big(V_INT, &c->number);
	if (c->tag == 2) {
		PnBigInt	n;
		if (!pn_big_copy(&n, &c->number))
			return NULL;
		Value	       *v = value_decimal(&n, c->exponent);
		if (v && c->boolean && pn_big_is_zero(&v->as.decimal.coefficient))
			v->as.decimal.negative_zero = true;
		return v;
	} if (c->tag == 3)
		return value_data(V_STR, (uint8_t *) c->text, c->text_length);
	if (c->tag == 4)
		return value_bool(c->boolean);
	return value_void();
}
static Value * execute(VM * vm, Fn * fn, Value * *arguments);
static bool	value_index(const Value * v, size_t * index){
	return v->kind == V_NAT && pn_big_fits_size(&v->as.integer, index);
}
static Value * read_stream(FILE * file, ValueKind kind, bool *io_error) {
	Buffer		b = {0};
	char		chunk[4096];
	size_t		n;
	*io_error = false;
	while ((n = fread(chunk, 1, sizeof(chunk), file)) > 0) {
		if (!buffer_add(&b, chunk, n)) {
			free(b.data);
			return NULL;
		}
	}
	if (ferror(file)) {
		free(b.data);
		*io_error = true;
		return NULL;
	} Value	       *v = value_data(kind, (uint8_t *) (b.data ? b.data : ""), b.length);
	free(b.data);
	return v;
}
static char    *path_parent_text(const char *path){
	char	       *copy = copy_text(path);
	if (!copy)
		return NULL;
	size_t		n = strlen(copy);
	while (n > 1 && copy[n - 1] == '/')
		copy[--n] = 0;
	char	       *slash = strrchr(copy, '/');
	if (!slash) {
		free(copy);
		return copy_text(".");
	} if (slash == copy)
		slash[1] = 0;
	else
		*slash = 0;
	return copy;
}
static char    *path_suffix_text(const char *path){
	const char     *slash = strrchr(path, '/'), *base = slash ? slash + 1 : path,
		       *dot = strrchr(base, '.');
	return copy_text(dot && dot != base ? dot : "");
}
static char    *path_normalize(const char *path){
	bool		absolute = path[0] == '/';
	char	       *copy = copy_text(path), **parts = NULL;
	size_t		count = 0, cap = 0;
	if (!copy)
		return NULL;
	char	       *at = copy;
	while (*at) {
		while (*at == '/')
			at++;
		if (!*at)
			break;
		char	       *start = at;
		while (*at && *at != '/')
			at++;
		if (*at)
			*at++ = 0;
		if (!strcmp(start, "."))
			continue;
		if (!strcmp(start, "..") && count && strcmp(parts[count - 1], "..")) {
			count--;
			continue;
		} if (count == cap) {
			cap = cap ? cap * 2 : 8;
			char	      **next = realloc(parts, cap * sizeof(char *));
			if (!next) {
				free(parts);
				free(copy);
				return NULL;
			} parts = next;
		} parts[count++] = start;
	} Buffer	out = {0};
	if (absolute)
		buffer_text(&out, "/");
	for (size_t i = 0; i < count; i++) {
		if (i)
			buffer_text(&out, "/");
		buffer_text(&out, parts[i]);
	} if (!out.length)
		buffer_text(&out, absolute ? "/" : ".");
	free(parts);
	free(copy);
	return out.data;
}
static char    *path_join_text(const char *left, const char *right){
	if (right[0] == '/')
		return path_normalize(right);
	Buffer		b = {0};
	buffer_text(&b, left);
	if (b.length && b.data[b.length - 1] != '/')
		buffer_text(&b, "/");
	buffer_text(&b, right);
	char	       *result = path_normalize(b.data);
	free(b.data);
	return result;
}
static const char *environment_value(VM * vm, const char *name){
	size_t		n = strlen(name);
	for (size_t i = 0; i < vm->env_count; i++)
		if (!strncmp(vm->environment[i], name, n) && vm->environment[i][n] == '=')
			return vm->environment[i] + n + 1;
	return NULL;
}
#define REQUIRE(condition,message) do { if(!(condition)){vm->error=(message);return NULL;} } while(0)
#define REQUIRE_PATH(value,message) do { \
	REQUIRE((value)->kind == V_STR, (message)); \
	REQUIRE(!memchr((value)->as.bytes.data, 0, (value)->as.bytes.length), \
	        "VM trap: path contains NUL byte"); \
} while(0)
static Value * builtin_call(VM * vm, const char *name, Value * *a){
	if (!strcmp(name, "print") || !strcmp(name, "eprint")) {
		Buffer		b = {0};
		if (!render(&b, a[0], false))
			return NULL;
		FILE	       *out = !strcmp(name, "print") ? stdout : stderr;
		fprintf(out, "%s\n", b.data ? b.data : "");
		free(b.data);
		return value_void();
	}
	if (!strcmp(name, "read_line")) {
		Buffer		b = {0};
		int		c;
		while ((c = fgetc(stdin)) != EOF && c != '\n') {
			if (c != '\r') {
				char		ch = (char)c;
				if (!buffer_add(&b, &ch, 1)) {
					free(b.data);
					return NULL;
				}
			}
		}
		Value *v = value_data(V_STR, (uint8_t *) (b.data ? b.data : ""), b.length);
		free(b.data);
		return v;
	}
	if (!strcmp(name, "read_file") || !strcmp(name, "read_bytes")) {
		REQUIRE_PATH(a[0], "VM trap: file path requires Str");
		FILE	       *file = fopen((char *)a[0]->as.bytes.data, "rb");
		if (!file) {
			vm->error = "I/O error: could not read file";
			return NULL;
		} bool		io_error;
		Value	       *v = read_stream(file, !strcmp(name, "read_file") ? V_STR : V_BYTES, &io_error);
		fclose(file);
		if (!v) {
			vm->error = io_error ? "I/O error: could not read file" : "native VM out of memory";
			return NULL;
		}
		if (v && v->kind == V_STR && !utf8(v->as.bytes.data, v->as.bytes.length)) {
			release(v);
			vm->error = "I/O error: file is not valid UTF-8";
			return NULL;
		} return v;
	}
	if (!strcmp(name, "write_file") || !strcmp(name, "write_bytes")) {
		REQUIRE_PATH(a[0], "VM trap: file path requires Str");
		REQUIRE((!strcmp(name, "write_file") && a[1]->kind == V_STR) ||
		        (!strcmp(name, "write_bytes") && a[1]->kind == V_BYTES),
		        "VM trap: invalid file contents");
		FILE	       *file = fopen((char *)a[0]->as.bytes.data, "wb");
		if (!file) {
			vm->error = "I/O error: could not write file";
			return NULL;
		} bool		ok = fwrite(a[1]->as.bytes.data, 1, a[1]->as.bytes.length, file) == a[1]->as.bytes.length;
		if (fclose(file) != 0)
			ok = false;
		if (!ok) {
			vm->error = "I/O error: could not write file";
			return NULL;
		} return value_void();
	}
	if (!strcmp(name, "len")) {
		REQUIRE(a[0]->kind == V_STR || a[0]->kind == V_BYTES || a[0]->kind == V_ARRAY,
		        "VM trap: len requires Str, Bytes, or Array");
		size_t		n = a[0]->kind == V_STR ? utf8_count(a[0]->as.bytes.data, a[0]->as.bytes.length) : a[0]->kind == V_BYTES ? a[0]->as.bytes.length : a[0]->as.sequence.count;
		return value_size(n);
	}
	if (!strcmp(name, "append")) {
		REQUIRE(a[0]->kind == V_ARRAY, "VM trap: append requires Array");
		size_t		n = a[0]->as.sequence.count;
		Value	      **items = malloc((n + 1) * sizeof(Value *));
		if (!items)
			return NULL;
		memcpy(items, a[0]->as.sequence.items, n * sizeof(Value *));
		items[n] = a[1];
		Value	       *v = value_sequence(V_ARRAY, items, n + 1);
		free(items);
		return v;
	}
	if (!strcmp(name, "concat")) {
		REQUIRE(a[0]->kind == V_ARRAY && a[1]->kind == V_ARRAY,
		        "VM trap: concat requires arrays");
		size_t		n = a[0]->as.sequence.count, m = a[1]->as.sequence.count;
		Value	      **items = malloc((n + m) * sizeof(Value *));
		if (!items && n + m)
			return NULL;
		memcpy(items, a[0]->as.sequence.items, n * sizeof(Value *));
		memcpy(items + n, a[1]->as.sequence.items, m * sizeof(Value *));
		Value	       *v = value_sequence(V_ARRAY, items, n + m);
		free(items);
		return v;
	}
	if (!strcmp(name, "slice")) {
		REQUIRE(a[0]->kind == V_STR && a[1]->kind == V_NAT && a[2]->kind == V_NAT,
		        "VM trap: slice requires Str and Nat bounds");
		size_t		start, end, bs, be;
		if (!value_index(a[1], &start) || !value_index(a[2], &end) || start > end) {
			vm->error = "VM trap: invalid string slice";
			return NULL;
		} size_t	count = utf8_count(a[0]->as.bytes.data, a[0]->as.bytes.length);
		if (end > count) {
			vm->error = "VM trap: invalid string slice";
			return NULL;
		} if (start == count)
			bs = a[0]->as.bytes.length;
		else if (!utf8_offset(a[0]->as.bytes.data, a[0]->as.bytes.length, start, &bs, &be))
			return NULL;
		if (end == count)
			be = a[0]->as.bytes.length;
		else {
			size_t		ignored;
			if (!utf8_offset(a[0]->as.bytes.data, a[0]->as.bytes.length, end, &be, &ignored))
				return NULL;
		} return value_data(V_STR, a[0]->as.bytes.data + bs, be - bs);
	}
	if (!strcmp(name, "starts_with") || !strcmp(name, "starts_with_at")) {
		REQUIRE(a[0]->kind == V_STR && a[1]->kind == V_STR,
		        "VM trap: starts_with requires Str");
		if (!strcmp(name, "starts_with_at"))
			REQUIRE(a[2]->kind == V_NAT, "VM trap: starts_with_at requires Nat offset");
		size_t		offset = 0, bs = 0, be;
		if (!strcmp(name, "starts_with_at")) {
			if (!value_index(a[2], &offset))
				return value_bool(false);
			if (offset == utf8_count(a[0]->as.bytes.data, a[0]->as.bytes.length))
				bs = a[0]->as.bytes.length;
			else if (!utf8_offset(a[0]->as.bytes.data, a[0]->as.bytes.length, offset, &bs, &be))
				return value_bool(false);
		} bool		result = a[1]->as.bytes.length <= a[0]->as.bytes.length - bs && !memcmp(a[0]->as.bytes.data + bs, a[1]->as.bytes.data, a[1]->as.bytes.length);
		return value_bool(result);
	}
	if (!strcmp(name, "reverse")) {
		REQUIRE(a[0]->kind == V_STR, "VM trap: reverse requires Str");
		size_t		n = a[0]->as.bytes.length, source = 0, destination = n;
		uint8_t       *data = malloc(n ? n : 1);
		if (!data)
			return NULL;
		while (source < n) {
			uint8_t		c = a[0]->as.bytes.data[source];
			size_t		width = c < 0x80 ? 1 : (c & 0xe0) == 0xc0 ? 2 : (c & 0xf0) == 0xe0 ? 3 : 4;
			destination -= width;
			memcpy(data + destination, a[0]->as.bytes.data + source, width);
			source += width;
		}
		Value	       *v = value_data(V_STR, data, n);
		free(data);
		return v;
	}
	if (!strcmp(name, "is_digit") || !strcmp(name, "is_letter") || !strcmp(name, "is_whitespace")) {
		REQUIRE(a[0]->kind == V_STR, "VM trap: character class requires Str");
		uint8_t		c = a[0]->as.bytes.length == 1 ? a[0]->as.bytes.data[0] : 0;
		bool		result = !strcmp(name, "is_digit") ? c >= '0' && c <= '9' : !strcmp(name, "is_letter") ? (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') : c == ' ' || c == '\t' || c == '\r' || c == '\n';
		return value_bool(result);
	}
	if (!strcmp(name, "map") || !strcmp(name, "set") || !strcmp(name, "bytes"))
		return !strcmp(name, "bytes") ? value_data(V_BYTES, NULL, 0) : value_sequence(!strcmp(name, "map") ? V_MAP : V_SET, NULL, 0);
	if (!strcmp(name, "$method_has"))
		REQUIRE(a[0]->kind == V_MAP || a[0]->kind == V_SET,
		        "VM trap: has requires Map or Set");
	if (!strcmp(name, "map_has") || !strcmp(name, "map_get") || !strcmp(name, "$method_get") ||
	    (!strcmp(name, "$method_has") && a[0]->kind == V_MAP)) {
		REQUIRE(a[0]->kind == V_MAP, "VM trap: map operation requires Map");
		for (size_t i = a[0]->as.sequence.count; i >= 2; i -= 2)
			if (value_equal(a[0]->as.sequence.items[i - 2], a[1]))
				return !strcmp(name, "map_has") || !strcmp(name, "$method_has") ? value_bool(true) : retain(a[0]->as.sequence.items[i - 1]);
		if (!strcmp(name, "map_has") || !strcmp(name, "$method_has"))
			return value_bool(false);
		vm->error = "VM trap: map key was not found";
		return NULL;
	}
	if (!strcmp(name, "map_put") || !strcmp(name, "$method_put")) {
		REQUIRE(a[0]->kind == V_MAP, "VM trap: map_put requires Map");
		size_t		old = a[0]->as.sequence.count, count = 0;
		Value	      **items = malloc((old + 2) * sizeof(Value *));
		if (!items)
			return NULL;
		for (size_t i = 0; i < old; i += 2) {
			if (!value_equal(a[0]->as.sequence.items[i], a[1])) {
				items[count++] = a[0]->as.sequence.items[i];
				items[count++] = a[0]->as.sequence.items[i + 1];
			}
		}
		items[count++] = a[1];
		items[count++] = a[2];
		Value	       *v = value_sequence(V_MAP, items, count);
		free(items);
		return v;
	}
	if (!strcmp(name, "set_has") || !strcmp(name, "$method_has")) {
		REQUIRE(a[0]->kind == V_SET, "VM trap: set_has requires Set");
		for (size_t i = 0; i < a[0]->as.sequence.count; i++)
			if (value_equal(a[0]->as.sequence.items[i], a[1]))
				return value_bool(true);
		return value_bool(false);
	}
	if (!strcmp(name, "set_add") || !strcmp(name, "$method_add")) {
		REQUIRE(a[0]->kind == V_SET, "VM trap: set_add requires Set");
		size_t		n = a[0]->as.sequence.count;
		for (size_t i = 0; i < n; i++)
			if (value_equal(a[0]->as.sequence.items[i], a[1]))
				return retain(a[0]);
		Value	      **items = malloc((n + 1) * sizeof(Value *));
		if (!items)
			return NULL;
		memcpy(items, a[0]->as.sequence.items, n * sizeof(Value *));
		items[n] = a[1];
		Value	       *v = value_sequence(V_SET, items, n + 1);
		free(items);
		return v;
	}
	if (!strcmp(name, "byte_append")) {
		REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_NAT,
		        "VM trap: byte_append requires Bytes and Nat");
		size_t		byte;
		if (!value_index(a[1], &byte) || byte > 255) {
			vm->error = "VM trap: byte value is outside 0..255";
			return NULL;
		} size_t	n = a[0]->as.bytes.length;
		uint8_t	       *data = malloc(n + 1);
		if (!data)
			return NULL;
		memcpy(data, a[0]->as.bytes.data, n);
		data[n] = (uint8_t) byte;
		Value	       *v = value_data(V_BYTES, data, n + 1);
		free(data);
		return v;
	}
	if (!strcmp(name, "bytes_concat")) {
		REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_BYTES,
		        "VM trap: bytes_concat requires Bytes");
		size_t		n = a[0]->as.bytes.length, m = a[1]->as.bytes.length;
		uint8_t	       *data = malloc(n + m);
		if (!data && n + m)
			return NULL;
		memcpy(data, a[0]->as.bytes.data, n);
		memcpy(data + n, a[1]->as.bytes.data, m);
		Value	       *v = value_data(V_BYTES, data, n + m);
		free(data);
		return v;
	}
	if (!strcmp(name, "byte_len")) {
		REQUIRE(a[0]->kind == V_BYTES, "VM trap: byte_len requires Bytes");
		return value_size(a[0]->as.bytes.length);
	}
	if (!strcmp(name, "byte_get")) {
		REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_NAT,
		        "VM trap: byte_get requires Bytes and Nat");
		size_t		i;
		if (!value_index(a[1], &i) || i >= a[0]->as.bytes.length) {
			vm->error = "VM trap: byte index is out of bounds";
			return NULL;
		} return value_size(a[0]->as.bytes.data[i]);
	}
	if (!strcmp(name, "utf8_encode")) {
		REQUIRE(a[0]->kind == V_STR, "VM trap: utf8_encode requires Str");
		return value_data(V_BYTES, a[0]->as.bytes.data, a[0]->as.bytes.length);
	}
	if (!strcmp(name, "utf8_decode")) {
		REQUIRE(a[0]->kind == V_BYTES, "VM trap: utf8_decode requires Bytes");
		if (!utf8(a[0]->as.bytes.data, a[0]->as.bytes.length)) {
			vm->error = "VM trap: invalid UTF-8";
			return NULL;
		} return value_data(V_STR, a[0]->as.bytes.data, a[0]->as.bytes.length);
	}
	if (!strcmp(name, "nat_from_str")) {
		REQUIRE(a[0]->kind == V_STR, "VM trap: nat_from_str requires Str");
		if (!a[0]->as.bytes.length) {
			vm->error = "VM trap: text is not a Nat";
			return NULL;
		}
		for (size_t i = 0; i < a[0]->as.bytes.length; i++) {
			if (a[0]->as.bytes.data[i] < '0' || a[0]->as.bytes.data[i] > '9') {
				vm->error = "VM trap: text is not a Nat";
				return NULL;
			}
		}
		PnBigInt n;
		if (!pn_big_from_digits(&n, (char *)a[0]->as.bytes.data, a[0]->as.bytes.length, 1))
			return NULL;
		Value	       *v = value_big(V_NAT, &n);
		pn_big_free(&n);
		return v;
	}
	if (!strcmp(name, "command_args")) {
		Value	      **items = calloc((size_t) vm->argc, sizeof(Value *));
		if (vm->argc && !items)
			return NULL;
		for (int i = 0; i < vm->argc; i++)
			items[i] = value_data(V_STR, (uint8_t *) vm->argv[i], strlen(vm->argv[i]));
		Value	       *v = value_sequence(V_ARRAY, items, (size_t) vm->argc);
		for (int i = 0; i < vm->argc; i++)
			release(items[i]);
		free(items);
		return v;
	}
	if (!strcmp(name, "environment_has")) {
		REQUIRE(a[0]->kind == V_STR, "VM trap: environment key requires Str");
		return value_bool(environment_value(vm, (char *)a[0]->as.bytes.data) != NULL);
	}
	if (!strcmp(name, "environment_get")) {
		REQUIRE(a[0]->kind == V_STR, "VM trap: environment key requires Str");
		const char     *value = environment_value(vm, (char *)a[0]->as.bytes.data);
		if (!value) {
			vm->error = "VM trap: environment variable was not found";
			return NULL;
		} return value_data(V_STR, (uint8_t *) value, strlen(value));
	}
	if (!strcmp(name, "path_parent")) {
		REQUIRE_PATH(a[0], "VM trap: path requires Str");
		char	       *s = path_parent_text((char *)a[0]->as.bytes.data);
		Value	       *v = s ? value_data(V_STR, (uint8_t *) s, strlen(s)) : NULL;
		free(s);
		return v;
	}
	if (!strcmp(name, "path_join")) {
		REQUIRE_PATH(a[0], "VM trap: path_join requires Str");
		REQUIRE_PATH(a[1], "VM trap: path_join requires Str");
		char	       *s = path_join_text((char *)a[0]->as.bytes.data, (char *)a[1]->as.bytes.data);
		Value	       *v = s ? value_data(V_STR, (uint8_t *) s, strlen(s)) : NULL;
		free(s);
		return v;
	}
	if (!strcmp(name, "path_suffix")) {
		REQUIRE_PATH(a[0], "VM trap: path requires Str");
		char	       *s = path_suffix_text((char *)a[0]->as.bytes.data);
		Value	       *v = s ? value_data(V_STR, (uint8_t *) s, strlen(s)) : NULL;
		free(s);
		return v;
	}
	if (!strcmp(name, "path_with_suffix")) {
		REQUIRE_PATH(a[0], "VM trap: path_with_suffix requires Str");
		REQUIRE_PATH(a[1], "VM trap: path_with_suffix requires Str");
		char	       *path = copy_text((char *)a[0]->as.bytes.data);
		char	       *suffix = path_suffix_text(path);
		if (!path || !suffix) {
			free(path);
			free(suffix);
			return NULL;
		} size_t	base = strlen(path) - strlen(suffix);
		Buffer		b = {0};
		buffer_add(&b, path, base);
		buffer_add(&b, (char *)a[1]->as.bytes.data, a[1]->as.bytes.length);
		Value	       *v = value_data(V_STR, (uint8_t *) b.data, b.length);
		free(path);
		free(suffix);
		free(b.data);
		return v;
	}
	if (!strcmp(name, "path_is_absolute")) {
		REQUIRE_PATH(a[0], "VM trap: path requires Str");
		return value_bool(a[0]->as.bytes.length && a[0]->as.bytes.data[0] == '/');
	}
	if (!strcmp(name, "path_resolve")) {
		REQUIRE_PATH(a[0], "VM trap: path requires Str");
		char	       *s;
		if (a[0]->as.bytes.length && a[0]->as.bytes.data[0] == '/')
			s = path_normalize((char *)a[0]->as.bytes.data);
		else {
			char		cwd[4096];
			if (!getcwd(cwd, sizeof(cwd))) {
				vm->error = "I/O error: could not resolve path";
				return NULL;
			} s = path_join_text(cwd, (char *)a[0]->as.bytes.data);
		} Value	       *v = s ? value_data(V_STR, (uint8_t *) s, strlen(s)) : NULL;
		free(s);
		return v;
	}
	if (!strcmp(name, "file_exists")) {
		REQUIRE_PATH(a[0], "VM trap: path requires Str");
		struct stat	status;
		return value_bool(stat((char *)a[0]->as.bytes.data, &status) == 0 && S_ISREG(status.st_mode));
	}
	if (!strcmp(name, "process_exit")) {
		REQUIRE(a[0]->kind == V_NAT, "VM trap: process status requires Nat");
		size_t		code;
		if (!value_index(a[0], &code)) {
			vm->error = "VM trap: invalid process status";
			return NULL;
		} exit((int)code);
	}
	if (!strcmp(name, "run_bytecode")) {
		REQUIRE(a[0]->kind == V_BYTES, "VM trap: run_bytecode requires Bytes");
		Program		nested;
		const char     *error = NULL;
		if (!decode(a[0]->as.bytes.data, a[0]->as.bytes.length, &nested, &error) || !verify(&nested, &error)) {
			vm->error = error;
			return NULL;
		} VM		child = *vm;
		child.program = &nested;
		child.error = NULL;
		Value	       *result = execute(&child, function(&nested, "main"), NULL);
		if (!result) {
			vm->error = child.error;
			free_program(&nested);
			return NULL;
		} release(result);
		free_program(&nested);
		return value_void();
	}
	if (!strcmp(name, "run_bytecode_args")) {
		REQUIRE(a[0]->kind == V_BYTES && a[1]->kind == V_ARRAY,
		    "VM trap: run_bytecode_args requires Bytes and [Str]");
		Program		nested;
		const char     *error = NULL;
		if (!decode(a[0]->as.bytes.data, a[0]->as.bytes.length, &nested, &error) || !verify(&nested, &error)) {
			vm->error = error;
			return NULL;
		}
		char	      **arguments = calloc(a[1]->as.sequence.count, sizeof(char *));
		if (a[1]->as.sequence.count && !arguments) {
			free_program(&nested);
			return NULL;
		}
		for (size_t i = 0; i < a[1]->as.sequence.count; i++) {
			Value	       *argument = a[1]->as.sequence.items[i];
			if (argument->kind != V_STR) {
				free(arguments);
				free_program(&nested);
				vm->error = "VM trap: run_bytecode_args requires [Str]";
				return NULL;
			}
			arguments[i] = (char *) argument->as.bytes.data;
		}
		VM		child = *vm;
		child.program = &nested;
		child.argc = (int) a[1]->as.sequence.count;
		child.argv = arguments;
		child.error = NULL;
		Value	       *result = execute(&child, function(&nested, "main"), NULL);
		free(arguments);
		if (!result) {
			vm->error = child.error;
			free_program(&nested);
			return NULL;
		}
		release(result);
		free_program(&nested);
		return value_void();
	}
	vm->error = "VM trap: native host service is not implemented";
	return NULL;
}
#undef REQUIRE
static Value * named_value(ValueKind kind, const char *name, char **names, Value * *values, size_t count){
	Value	       *v = value_new(kind);
	if (!v)
		return NULL;
	v->as.named.name = copy_text(name);
	v->as.named.count = count;
	v->as.named.names = kind == V_RECORD ? calloc(count, sizeof(char *)) : NULL;
	v->as.named.values = calloc(count, sizeof(Value *));
	if (!v->as.named.name || (count && !v->as.named.values) || (kind == V_RECORD && count && !v->as.named.names)) {
		release(v);
		return NULL;
	} for (size_t i = 0; i < count; i++) {
		if (kind == V_RECORD)
			v->as.named.names[i] = copy_text(names[i]);
		v->as.named.values[i] = retain(values[i]);
	} return v;
}
static Value * execute(VM * vm, Fn * fn, Value * *arguments) {
	Frame		f = {0};
	f.function = fn;
	for (size_t i = 0; i < fn->param_count; i++)
		if (!local_put(&f, fn->params[i], retain(arguments[i])))
			goto oom;
	while (f.pc < fn->ins_count && !vm->error) {
		Ins	       *in = &fn->ins[f.pc++];
		Value	       *a = NULL, *b = NULL, *v = NULL;
		size_t		index, start, end;
		switch (in->op) {
		case 0:
			v = constant_value(&in->constant);
			if (!v || !stack_push(&f, v))
				goto oom;
			break;
		case 1:
			v = local_get(&f, in->name);
			if (!v) {
				vm->error = "VM trap: uninitialized local";
				break;
			} if (!stack_push(&f, retain(v)))
				goto oom;
			break;
		case 2:
			v = stack_pop(&f, vm);
			if (v && !local_put(&f, in->name, v))
				goto oom;
			break;
		case 3:
			v = stack_pop(&f, vm);
			release(v);
			break;
		case 4:
			a = stack_pop(&f, vm);
			if (!a)
				break;
			if (in->code == 1) {
				if (a->kind != V_BOOL) {
					vm->error = "VM trap: ! requires Bool";
				} else
					v = value_bool(!a->as.boolean);
			} else if (a->kind == V_DEC) {
				PnBigInt	n;
				if (pn_big_copy(&n, &a->as.decimal.coefficient)) {
					n.sign = -n.sign;
					v = value_decimal(&n, a->as.decimal.exponent);
				}
			} else if (a->kind == V_NAT || a->kind == V_INT) {
				PnBigInt	n;
				if (pn_big_copy(&n, &a->as.integer)) {
					n.sign = -n.sign;
					v = value_big(V_INT, &n);
					pn_big_free(&n);
				}
			} else
				vm->error = "VM trap: unary - requires numeric value";
			release(a);
			if (v && !stack_push(&f, v))
				goto oom;
			break;
		case 5:
			b = stack_pop(&f, vm);
			a = stack_pop(&f, vm);
			if (!a || !b) {
				release(a);
				release(b);
				break;
			} v = binary_value(in->code, a, b, vm);
			release(a);
			release(b);
			if (v && !stack_push(&f, v))
				goto oom;
			break;
		case 6:
			b = stack_pop(&f, vm);
			a = stack_pop(&f, vm);
			if (!a || !b)
				break;
			if (a->kind != V_NAT || b->kind != V_NAT) {
				vm->error = "VM trap: range bounds require Nat";
				release(a);
				release(b);
				break;
			}
			v = value_new(V_RANGE);
			if (!v || !pn_big_copy(&v->as.range.start, &a->as.integer) || !pn_big_copy(&v->as.range.end, &b->as.integer)) {
				release(a);
				release(b);
				release(v);
				goto oom;
			} release(a);
			release(b);
			if (!stack_push(&f, v))
				goto oom;
			break;
		case 7:	{
				Value	      **items = calloc(in->count, sizeof(Value *));
				if (in->count && !items)
					goto oom;
				for (size_t i = in->count; i-- > 0;)
					items[i] = stack_pop(&f, vm);
				if (vm->error) {
					for (size_t i = 0; i < in->count; i++)
						release(items[i]);
					free(items);
					break;
				} v = value_sequence(V_ARRAY, items, in->count);
				for (size_t i = 0; i < in->count; i++)
					release(items[i]);
				free(items);
				if (!v || !stack_push(&f, v))
					goto oom;
				break;
			}
		case 8:
			b = stack_pop(&f, vm);
			a = stack_pop(&f, vm);
			if (!a || !b)
				break;
			if (!value_index(b, &index)) {
				vm->error = "VM trap: invalid index";
			} else if (a->kind == V_ARRAY) {
				if (index >= a->as.sequence.count)
					vm->error = "VM trap: index is out of bounds";
				else
					v = retain(a->as.sequence.items[index]);
			} else if (a->kind == V_BYTES) {
				if (index >= a->as.bytes.length)
					vm->error = "VM trap: index is out of bounds";
				else
					v = value_size(a->as.bytes.data[index]);
			} else if (a->kind == V_STR) {
				if (!utf8_offset(a->as.bytes.data, a->as.bytes.length, index, &start, &end))
					vm->error = "VM trap: index is out of bounds";
				else
					v = value_data(V_STR, a->as.bytes.data + start, end - start);
			} else
				vm->error = "VM trap: value is not indexable";
			release(a);
			release(b);
			if (v && !stack_push(&f, v))
				goto oom;
			break;
		case 9:	{
				size_t		count = in->count ? in->count - 1 : 0;
				Value	      **values = calloc(count, sizeof(Value *));
				if (count && !values)
					goto oom;
				for (size_t i = count; i-- > 0;)
					values[i] = stack_pop(&f, vm);
				Buffer		out = {0};
				for (size_t i = 0; i < in->count && !vm->error; i++) {
					if (!buffer_text(&out, in->items[i]))
						goto oom;
					if (i < count && !render(&out, values[i], false))
						goto oom;
				} for (size_t i = 0; i < count; i++)
					release(values[i]);
				free(values);
				v = value_data(V_STR, (uint8_t *) (out.data ? out.data : ""), out.length);
				free(out.data);
				if (!v || !stack_push(&f, v))
					goto oom;
				break;
			}
		case 10:
			a = stack_pop(&f, vm);
			if (!a)
				break;
			v = value_new(V_ITER);
			if (!v) {
				release(a);
				goto oom;
			} v->as.iterator.iterable = a;
			pn_big_init(&v->as.iterator.cursor);
			if (a->kind == V_RANGE && !pn_big_copy(&v->as.iterator.cursor, &a->as.range.start)) {
				release(v);
				goto oom;
			} if (!local_put(&f, in->name, v))
				goto oom;
			break;
		case 11:
			v = local_get(&f, in->name);
			if (!v || v->kind != V_ITER) {
				vm->error = "VM trap: invalid iterator";
				break;
			} a = v->as.iterator.iterable;
			if (a->kind == V_RANGE) {
				if (pn_big_compare(&v->as.iterator.cursor, &a->as.range.end) >= 0) {
					f.pc = in->target;
					break;
				} b = value_big(V_NAT, &v->as.iterator.cursor);
				if (!pn_big_add_small(&v->as.iterator.cursor, 1)) {
					release(b);
					goto oom;
				}
			} else if (a->kind == V_ARRAY) {
				if (v->as.iterator.index >= a->as.sequence.count) {
					f.pc = in->target;
					break;
				} b = retain(a->as.sequence.items[v->as.iterator.index++]);
			} else if (a->kind == V_BYTES) {
				if (v->as.iterator.index >= a->as.bytes.length) {
					f.pc = in->target;
					break;
				} b = value_size(a->as.bytes.data[v->as.iterator.index++]);
			} else {
				vm->error = "VM trap: value is not iterable";
				break;
			} if (!b || !local_put(&f, in->name2, b))
				goto oom;
			break;
		case 12:{
				Value	      **values = calloc(in->count, sizeof(Value *));
				if (in->count && !values)
					goto oom;
				for (size_t i = in->count; i-- > 0;)
					values[i] = stack_pop(&f, vm);
				v = named_value(V_RECORD, in->name, in->items, values, in->count);
				for (size_t i = 0; i < in->count; i++)
					release(values[i]);
				free(values);
				if (!v || !stack_push(&f, v))
					goto oom;
				break;
			}
		case 13:
			a = stack_pop(&f, vm);
			if (!a)
				break;
			if (a->kind != V_RECORD) {
				vm->error = "VM trap: field access requires a record";
			} else
				for (size_t i = 0; i < a->as.named.count; i++)
					if (!strcmp(a->as.named.names[i], in->name)) {
						v = retain(a->as.named.values[i]);
						break;
			}
		if (!v && !vm->error)
				vm->error = "VM trap: record has no field";
			release(a);
			if (v && !stack_push(&f, v))
				goto oom;
			break;
		case 14:{
				Value	      **values = calloc(in->count, sizeof(Value *));
				if (in->count && !values)
					goto oom;
				for (size_t i = in->count; i-- > 0;)
					values[i] = stack_pop(&f, vm);
				v = named_value(V_VARIANT, in->name2, NULL, values, in->count);
				for (size_t i = 0; i < in->count; i++)
					release(values[i]);
				free(values);
				if (!v || !stack_push(&f, v))
					goto oom;
				break;
			}
		case 15:
			a = stack_pop(&f, vm);
			if (!a)
				break;
			if (a->kind != V_VARIANT) {
				vm->error = "VM trap: match subject is not an enum";
			} else if (strcmp(a->as.named.name, in->name))
				f.pc = in->target;
			else {
				for (size_t i = 0; i < a->as.named.count; i++)
					if (!stack_push(&f, retain(a->as.named.values[i]))) {
						release(a);
						goto oom;
					}
			}
			release(a);
			break;
		case 16:
			vm->error = "VM trap: enum match was not exhaustive";
			break;
		case 17:{
				Value	      **args = calloc(in->arity, sizeof(Value *));
				if (in->arity && !args)
					goto oom;
				for (size_t i = in->arity; i-- > 0;)
					args[i] = stack_pop(&f, vm);
				if (vm->error) {
					for (size_t i = 0; i < in->arity; i++)
						release(args[i]);
					free(args);
					break;
				} const		Builtin *built = builtin(in->name);
				v = built ? builtin_call(vm, in->name, args) : execute(vm, function(vm->program, in->name), args);
				for (size_t i = 0; i < in->arity; i++)
					release(args[i]);
				free(args);
				if (v && !stack_push(&f, v))
					goto oom;
				break;
			}
		case 18:
			a = stack_pop(&f, vm);
			if (!a)
				break;
			if (a->kind != V_BOOL)
				vm->error = "VM trap: conditional requires Bool";
			else if (!a->as.boolean)
				f.pc = in->target;
			release(a);
			break;
		case 19:
			f.pc = in->target;
			break;
		case 20:
			v = stack_pop(&f, vm);
			if (!v)
				break;
			frame_free(&f);
			return v;
		case 21:{
			Value	      **args = calloc(in->arity, sizeof(Value *));
			if (in->arity && !args)
				goto oom;
			for (size_t i = in->arity; i-- > 0;)
				args[i] = stack_pop(&f, vm);
			Value	       *callable = stack_pop(&f, vm);
			if (vm->error) {
				for (size_t i = 0; i < in->arity; i++)
					release(args[i]);
				free(args);
				release(callable);
				break;
			}
			if (!callable || callable->kind != V_STR) {
				vm->error = "VM trap: indirect call requires a callable";
			} else {
				char	       *callee = malloc(callable->as.bytes.length + 1);
				if (!callee) {
					for (size_t i = 0; i < in->arity; i++)
						release(args[i]);
					free(args);
					release(callable);
					goto oom;
				}
				memcpy(callee, callable->as.bytes.data, callable->as.bytes.length);
				callee[callable->as.bytes.length] = 0;
				const Builtin *built = builtin(callee);
				Fn	       *called = function(vm->program, callee);
				if (!built && !called)
					vm->error = "VM trap: indirect call target was not found";
				else if (in->arity != (built ? built->arity : called->param_count))
					vm->error = "VM trap: indirect call arity mismatch";
				else if (fn->pure && !((built && built->pure) || (called && called->pure)))
					vm->error = "VM trap: pure function invokes impure callable";
				else
					v = built ? builtin_call(vm, callee, args) : execute(vm, called, args);
				free(callee);
			}
			for (size_t i = 0; i < in->arity; i++)
				release(args[i]);
			free(args);
			release(callable);
			if (v && !stack_push(&f, v))
				goto oom;
			break;
		}
		default:
			vm->error = "VM trap: unknown instruction";
			break;
		}
	}
	if (!vm->error)
		vm->error = "VM trap: function did not return";
	frame_free(&f);
	return NULL;
oom:	vm->error = "native VM out of memory";
	frame_free(&f);
	return NULL;
}
static bool	read_file(const char *path, uint8_t * *data, size_t * length){
	FILE	       *f = fopen(path, "rb");
	if (!f)
		return false;
	if (fseek(f, 0, SEEK_END) || (*length = (size_t) ftell(f)) > MAX_ARTIFACT || fseek(f, 0, SEEK_SET)) {
		fclose(f);
		return false;
	} *data = malloc(*length ? *length : 1);
	bool		ok = *data && fread(*data, 1, *length, f) == *length;
	fclose(f);
	return ok;
}
static bool	snapshot_environment(char ***out, size_t * count){
	*count = 0;
	while (environ[*count])
		(*count)++;
	*out = calloc(*count, sizeof(char *));
	if (*count && !*out)
		return false;
	for (size_t i = 0; i < *count; i++) {
		(*out)[i] = copy_text(environ[i]);
		if (!(*out)[i]) {
			for (size_t j = 0; j < i; j++)
				free((*out)[j]);
			free(*out);
			*out = NULL;
			*count = 0;
			return false;
		}
	} return true;
}
static void	free_environment(char **values, size_t count){
	for (size_t i = 0; i < count; i++)
		free(values[i]);
	free(values);
}
int		main(int argc, char **argv){
	if (argc < 3 || (strcmp(argv[1], "check") && strcmp(argv[1], "run"))) {
		fprintf(stderr, "usage: panack-vm <check|run> <program.bc> [arguments...]\n");
		return 2;
	} uint8_t      *data = NULL;
	size_t		length = 0;
	if (!read_file(argv[2], &data, &length)) {
		fprintf(stderr, "error: could not read artifact\n");
		return 1;
	} Program	p;
	const char     *error = NULL;
	bool		ok = decode(data, length, &p, &error) && verify(&p, &error);
	free(data);
	if (!ok) {
		fprintf(stderr, "error: %s\n", error);
		return 1;
	} if (!strcmp(argv[1], "check")) {
		free_program(&p);
		puts("ok");
		return 0;
	} char	      **environment = NULL;
	size_t		env_count = 0;
	if (!snapshot_environment(&environment, &env_count)) {
		fprintf(stderr, "error: native VM out of memory\n");
		free_program(&p);
		return 1;
	} VM		vm = {&p, argc - 3, argv + 3, env_count, environment, NULL};
	Value	       *result = execute(&vm, function(&p, "main"), NULL);
	if (!result) {
		fprintf(stderr, "error: %s\n", vm.error ? vm.error : "native VM failure");
		free_environment(environment, env_count);
		free_program(&p);
		return 1;
	} release(result);
	free_environment(environment, env_count);
	free_program(&p);
	return 0;
}
