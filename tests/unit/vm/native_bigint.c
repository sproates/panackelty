#include "bigint.h"
#include <stdio.h>
#include <stdlib.h>
int main(void) {
    PnBigInt a, b, product, quotient, remainder;
    if (!pn_big_from_digits(&a, "999999999999999999999999999999", 30, 1) ||
        !pn_big_from_u64(&b, 9) || !pn_big_mul(&product, &a, &b) ||
        !pn_big_divmod(&quotient, &remainder, &product, &b)) return 2;
    char *p = pn_big_string(&product), *q = pn_big_string(&quotient);
    if (!p || !q) return 3;
    printf("%s\n%s\n%u\n", p, q, pn_big_is_zero(&remainder));
    free(p); free(q); pn_big_free(&a); pn_big_free(&b);
    pn_big_free(&product); pn_big_free(&quotient); pn_big_free(&remainder);
    return 0;
}
