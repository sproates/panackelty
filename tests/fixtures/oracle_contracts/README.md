# Fixed independent oracle expectations

Captured from the tests at `8337aac`, before retiring their live Python oracle.
`inventory.json` records exact counts, source paths and SHA-256 digests. These are
reviewed fixtures, never regenerated during validation. Runtime tests do not use
Python; the captured data retains the independence of its original calculation.

- `integer.stdin/stdout`: Random seed `0x50414E`; boundaries 0, 1, 10^9−1,
  10^9, 10^9+1, 10^18−1, 10^18, 2^64−1, 10^81−1, 10^81; Cartesian pairs with
  (++), (−+), (+−), (−−) signs plus 60 pairs sampled from [−10^100,10^100) and
  [−10^50,10^50). Each row is mode/op/a/exponent/b/exponent. Operations 0–3
  are add, subtract, multiply, truncating division; zero divisors are excluded.
  Results use independent Python integer arithmetic; quotient truncates toward
  zero and remainder is a−quotient×b. All 1,800 rows are checked exactly.
- `decimal.stdin/stdout`: Seed `0xDEC`; original 24 boundary coefficient/exponent
  cases plus 80 random pairs with coefficients in [−10^24,10^24), exponents in
  [−40,40). For each: add/subtract/multiply/compare (op 5). Six extra divisions
  use divisors 2, 5, 8, 125, 2^30, 5^20. All 422 results come from exact Fraction
  arithmetic. Finite fractions are rendered independently by factoring the
  denominator into powers of 2 and 5. Native output is compared after string-only
  removal of insignificant zeros; no awk numeric arithmetic touches the digits.
- `rational.panack/stdout`: Seed 812; 24 original pairs, a in [−10^25,10^25],
  b,d in [1,10^12], c in [1,10^25], four operations each. Fraction gives the
  exact reduced numerator/denominator for all 96 expectations.
- `builtin.names/stdout`: All 82 stage-0 builtin names, arities and purity flags.
  The native C lookup must also find a non-null handler for each name.
- `driver-*.hex`, `stdlib.hex`, `euler001.hex`: Bytes produced by the independent
  stage-0 compiler before retirement; original sources are listed in the inventory.
  Native compilation must reproduce them exactly. Direct compiler integration
  also compares the driver and public command for all three module graphs.
- `codec-disassembly.stdout`: Original stage-0 emitter listing for the codec
  disassembly source. The self-hosted decoder must match it exactly.

To change a fixture, independently establish the correct result, review its
source/bytecode semantics, update its digest and the retirement inventory, then
run the ordinary and instrumented native contracts. Do not refresh expected
values from the native output merely to resolve a mismatch. The original test
algorithms remain available in Git at the recorded commit.

See `tests/ORACLE_REPLACEMENT.md` for the full ownership and retirement audit.
