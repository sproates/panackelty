import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT = Path(__file__).resolve().parents[3]
VM = PROJECT / "src/vm"


class NativeBigIntTests(unittest.TestCase):
    def test_arithmetic_exceeds_host_word_size(self):
        source = r'''
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
'''
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            test = root / "test.c"
            executable = root / "test"
            test.write_text(source, encoding="utf-8")
            compile_result = subprocess.run(
                [
                    "cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                    "-I", str(VM), str(test), str(VM / "bigint.c"),
                    "-o", str(executable),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            result = subprocess.run(
                [str(executable)], capture_output=True, text=True
            )

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(
            result.stdout,
            "8999999999999999999999999999991\n"
            "999999999999999999999999999999\n"
            "1\n",
        )


if __name__ == "__main__":
    unittest.main()
