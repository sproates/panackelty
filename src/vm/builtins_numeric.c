#include "builtins_internal.h"
#include "numeric.h"
#include "value.h"

#include <stdlib.h>
#include <string.h>

/* Language numeric constructors and exact checked conversions. */

Value *builtins_numeric_call(VM *vm, const char *name, Value **a)
{
    if (!strcmp(name, "$unit")) {
        return value_new(V_UNIT);
    }

    if (!strcmp(name, "quotient")) {
        REQUIRE(a[0]->kind == V_NAT && a[1]->kind == V_NAT,
                "VM trap: quotient requires Nat operands");
        REQUIRE(!pn_big_is_zero(&a[1]->as.integer), "VM trap: division by zero");
        PnBigInt q = {0}, r = {0};
        if (!pn_big_divmod(&q, &r, &a[0]->as.integer, &a[1]->as.integer)) {
            return NULL;
        }
        Value *out = value_big(V_NAT, &q);
        pn_big_free(&q);
        pn_big_free(&r);
        return out;
    }

    if (!strcmp(name, "nat") || !strcmp(name, "dec")) {
        REQUIRE(a[0]->kind == V_RAT, "VM trap: rational conversion requires Rat");
        const PnBigInt *n = &a[0]->as.rational.numerator, *d = &a[0]->as.rational.denominator;
        if (!strcmp(name, "nat")) {
            REQUIRE(n->sign >= 0 && pn_big_is_one(d), "VM trap: Rat is not an exact Nat");
            return value_big(V_NAT, n);
        }
        Decimal numerator = {.coefficient = *n}, denominator = {.coefficient = *d};
        return decimal_binary(3, &numerator, &denominator, &vm->error);
    }
    vm->error = "VM trap: unknown builtin";
    return NULL;
}
