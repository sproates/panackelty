# Focused nightly investigation

Temporary branch experiment for scheduled run 40295801266. Do not merge this
instrumentation into the product. The workflow runs on pushes to
`ci/nightly-failure-diagnostics` and supports manual dispatch once registered.

The upload jobs use macos-14 and the exact upload action revision from the
failure, with the same two filenames, retention and compression defaults.
The TSV contents are representative, not recovered originals. Three fresh
runners make independent upload attempts. DNS observations happen afterwards,
so the probe does not warm the resolver before the upload. The local JavaScript
action receives GitHub's runtime endpoint and logs only its hostname and DNS
answers, never credentials or signed upload URLs. A successful attempt does
not establish the cause of the earlier intermittent failure. Runner image
versions and the original job's elapsed age may differ.

The original baseline experiment is retained in `compiler.cjs`. Run 40316511166
passed the selected fixture with its original 90-second deadline: 76.9/79.2s
alone/concurrent on the baseline machine and 53.7/56.1s on the traced machine.
Different machines prevent attributing that difference to instrumentation.

The current workflow experiments with a progress watchdog. It directly runs
the failing source subcase's command, `./bin/panack run src/compiler/main.panack`,
and compares its exit status, stdout and stderr against the existing fixture.
It deliberately bypasses the test runner's aggregate 90-second process timeout.
This is a prototype supervisor, not a production process-API change. The
compile/bytecode subcases are not exercised by this experiment.

Each module's lexing, parsing, signature checking, guard checking and body
checking has its own 90-second allowance. Function observations identify the
last body encountered, but do not renew the module allowance. Loading and
preparation account separately for work outside those module passes; code
generation, effect checking and serialisation have bounded stages. A parent
stage's clock pauses during explicitly bounded child stages, then resumes its
remaining allowance. Startup and gaps between stages are also bounded. The
90-second allowance is a conservative hang safeguard inherited for this
experiment, not an agreed performance target. The ten-minute CI job limit
remains the final backstop against endless sequences of apparently valid work.

`instrument.rb` temporarily observes execution of the existing seed in the VM.
It does not change the seed or compiler source. Trace rows record process ID,
entry/return, compiler operation, monotonic time, process CPU time, inclusive
elapsed/CPU durations, module path and function name. Function rows measure
binding of the root block, not every subsequent constraint check. The watchdog
therefore reports the **last function observed**, not an unproven active function.
Nested timings overlap and must not be summed. Output is written separately
from compiler stdout/stderr. Missing or malformed progress fails the experiment.

`watchdog.test.cjs` proves that healthy module work may exceed the former total
deadline, function activity cannot hide a stuck module, late completions still
fail, and startup, missing evidence and unresponsive processes are handled.
`compiler-watchdog.cjs faults` injects real compiler stalls in signature/body
checking, a named function, code generation and serialisation. These tests use
a two-second allowance, assert the reported location, and then run a healthy
compile to verify recovery. Termination escalates to killing the process group.

The current workflow runs the source command alone and alongside the actual
`unit-runtime-probes` sibling workload with one worker. The sibling is stopped
after the command and its status is not presented as a suite pass. Preparation
builds a current compiler for the sibling's final probe only. These experiments
do not recreate the nightly's preceding scheduling, cache state or runner age.

Evidence is printed in job logs and saved as artifacts. Production workflows,
compiler deadlines, package behaviour and release documentation are unchanged.
