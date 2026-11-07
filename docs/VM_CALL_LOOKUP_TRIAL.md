# VM direct-call lookup trial

Local experiment and development adoption, 6 November 2026. The owner requested
using the optimisation on the current branch after local verification. No
bytecode format change or CI. Baseline source: `976adb6`. No integration merge
or release publication is implied.

## Change

Verification caches borrowed builtin/function pointers on direct CALL and
AWAIT_CALL instructions. Execution uses them only after verification succeeds.
Re-verification disables the cache before checking; a failed verification cannot
publish it. Runtime call arity and effect checks remain. Execution also invokes
an already selected builtin directly, avoiding a second registry lookup.

Names remain available for diagnostics. No opcodes, codec, bytecode version,
bootstrap bytes or language semantics change. Indirect call lookup, internal
builtin name dispatch and local-variable lookup remain unchanged. Manually
constructed, unverified native test programs retain the lookup fallback.

Loaded programs must remain immutable while these links are in use, as with the
existing verified function-order cache. Links borrow registry entries and the
owning Program's stable function storage; they require no independent cleanup.

## Measurement

Fresh baseline and candidate native VMs built separately with the same Makefile,
`cc -O2`, dependencies and bootstrap seed. The ordinary checkout VM is untouched.
End-to-end subprocess times include decoding, verification/link preparation,
source loading, compilation and output writing. No other test/build workload ran
concurrently with these benchmark pairs. Host scheduling and thermal variation
were not controlled; this is an exploratory measurement, not release acceptance.

| Workload | Baseline | Candidate | Improvement |
| --- | ---: | ---: | ---: |
| Compile an entry importing all 28 stdlib modules, five alternating pairs, median | 0.612209s | 0.387422s | 36.72% |
| Compile src/compiler/main.panack, one pair | 44.701294s | 29.306701s | 34.44% |

Stdlib baseline seconds: 0.982321, 0.589062, 0.612209, 0.608568, 0.615737.
Candidate seconds: 0.727269, 0.387149, 0.389767, 0.387422, 0.383837.
Order alternated baseline/candidate then candidate/baseline; first observations
are retained. Compiler pair ran baseline then candidate, so order effects remain
an uncertainty. No profiling instrumentation was active during timing.

All corresponding emitted artifacts are byte-identical. The newly compiled
compiler also equals the existing bootstrap seed:
`9fb5c3837e87901f12fe0768099dc8c81a5aaf531735ce53c84a6573a62f066d`.
Stdlib artifact SHA-256:
`868ae355ea5a9b830499c7f1fde4827e5e6dd5439fc43d09c290b31028ff1686`.
This proves unchanged output for these workloads, not universal equivalence.

Trade-off: two cached pointers add 16 bytes to every decoded Instruction on this
arm64 host (104 to 120 bytes, approximately 15.4% larger instruction storage).
The serialized artifact size is unchanged. Whole-process peak memory was not
measured; a compact representation is possible future work, not this experiment.

## Local checks and limitations

Passed:

- Strict native compilation with the existing warning/error flags.
- `test_modules call-links`: cached builtin/direct target identity, refresh when
  a target changes, failed verification disabling links, forged bytecode
  structures, frame ownership, nested calls and builtin results/errors.
- Compile and execute `async_await` and `owned_methods`; candidate and baseline
  execution match the existing expected stdout. Async fixture includes indirect
  async calls and nested suspension.
- Resume the owned-methods bytecode one instruction at a time.
- Decoder corpus using async fixture: 18,844 mutations and 1,346 truncations.
- Documentation links and whitespace review.

### Resolution of the initial failed probes

The TCP outbound fixture replaced a function's instruction body after verifying
its Program. The new body lacked resolved links. Re-verifying that changed test
program before execution fixes the failure and preserves all socket assertions.
The complete native module suite now passes using the normal checkout VM.
This enforces the same verified-program lifetime contract as normal loading;
it does not relax runtime checks or suppress a network failure.

The generic resumable-artifact helper supports synchronous print workloads,
stops on VM_WAITING, and expects a Void result. It is not the correct driver for
the async fixture. A new native regression verifies direct function and builtin
links, advances one instruction at a time, observes a pending fake host read,
supplies its typed completion and verifies successful Unit completion/cleanup.
That test runs in the full native module suite and passes. Source/saved-bytecode
async fixture execution also passed in the trial.

The normal checkout panack-vm has been rebuilt with the optimisation. Final local
`make check` passed on 6 November 2026 (exit 0, 787 seconds), including unit,
functional, bootstrap, packaging and release-smoke checks. The historical
120-second warning was emitted; this is not a matched before/after full-suite
benchmark and establishes no full-suite speedup or regression. Bootstrap stage 2
reproduced the existing seed SHA-256. Local development adoption is complete.
No compiler-source, bootstrap, wire-format, system installation or website change.
No CI or push. The full suite is being run once for development adoption rather
than for every exploratory measurement.

## Reproduction

Use separate build directories for baseline and candidate:
`make BUILD_DIR=build/lookup-trial-candidate build/lookup-trial-candidate/vm/panack-vm`.
Set PANACKELTY_STDLIB_PATH to the checkout's src/stdlib and
PANACKELTY_BOOTSTRAP_ROOT to the checkout. Compile with:

```sh
build/lookup-trial-candidate/vm/panack-vm run bootstrap/compiler-v9.bc \
  compile src/compiler/main.panack -o /tmp/lookup-compiler-candidate.bc
```

For stdlib, generate an entry importing every src/stdlib/*.panack module under a
unique alias, with `main(): Unit { () }`, inside the checkout's build directory.
Compile it afresh on every measurement. Do not time execution-result cache hits.
The local benchmark script and raw measurements are retained under
/tmp/panack-lookup-bench.cjs and /tmp/panack-lookup-results.json.

No website or public release claims change. This is a measured VM optimisation
candidate, not a new commitment to native compilation or incremental compilation.
