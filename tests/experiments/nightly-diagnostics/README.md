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

The compiler jobs use the unchanged selected fixture:
`tests/runner/main.panack --case compiler_skeleton`. Its source subcase still
executes `./bin/panack run src/compiler/main.panack` through Panack's process
API, with the existing 90-second deadline and exact output assertions.
Preparation compiles the fixture runner and a current compiler artifact once.
The latter stands in for the nightly's already-built stage-two artifact only
in the compile/bytecode subcases; it does not bypass the source subcase.

Baseline and instrumented jobs each run alone and with the real sibling
`unit-runtime-probes` workload, with one worker as in the nightly. The sibling
is stopped after the selected fixture. Its exit status is recorded but is not
reported as a suite pass. This recreates the command, bounds and competing
workload, not the complete nightly's prior scheduling or cache state.

The Ruby script adds temporary native VM observation of coarse Panack compiler
function entry/return. It does not alter the seed or compiler source. Timings
are inclusive, with monotonic elapsed and process CPU seconds; nested rows
must not be summed. An unmatched entry identifies a phase still active when
the process was stopped. Output goes to a separate trace file so diagnostics
cannot change the fixture's stdout/stderr contracts. Process CPU and resident
memory snapshots are recorded every two seconds. Baseline results provide a
check on instrumentation overhead; different hosted machines remain a
comparison limitation. The outer 240-second watchdog bounds the experiment;
it does not replace or extend the fixture's 90-second deadline.

Evidence is printed in job logs and saved as artifacts. Production workflows,
compiler deadlines, package behaviour and release documentation are unchanged.
