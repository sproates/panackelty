# Compiler progress watchdog

The compiler-source fixture checks a complete self-compilation. A single
90-second deadline conflated accumulated healthy work with a stuck compiler.
Scheduled run 40295801266 exceeded that deadline; the precise cause remains
unconfirmed. Focused experiments found healthy compilations between roughly
54 and 79 seconds on hosted machines. No individual module pass approached
90 seconds. This supports adopting progress bounds, not claiming a root cause.

`run.sh run src/compiler/main.panack` preserves the fixture's exit status and
byte-exact stdout/stderr assertions. `run.sh compile SOURCE -o OUTPUT` covers
its standalone compile path when a verified bootstrap artifact is unavailable.
Both use the existing seed and native VM implementation. The fixture's outer
600-second process bound is a last-resort backstop, not a speed requirement.

The native supervisor gives each module's lexing, parsing, signature checking,
guard checking and body checking a separate 90-second allowance. Function
observations never renew a module deadline. Loading/preparation, effect
checking, code generation and serialisation also have bounded work units.
Parent clocks pause for explicitly bounded children and resume their remaining
allowance afterwards. Startup and time outside reported stages remain bounded.
An endlessly repeating sequence of completed units is ultimately stopped by
the enclosing process/job limit. Performance expectations remain separate.

The observer is compiled into a **separate test VM**. Compiler-specific names,
value inspection and fault injection live here, not in production VM code.
The execution engine exposes compile-time test hooks; those hooks and frame
fields are absent from normal, installed and packaged VMs. No compiler seed
regeneration or additional host capability is required. The supervisor is a
native development helper, so normal validation gains no Node/Python dependency.
`make compiler-watchdog-build` compiles one extra execution object, links the
test VM against existing native objects, and builds the small supervisor.
Standalone fixture execution prepares these helpers incrementally.

Evidence is retained in `build/validation/compiler-watchdog/run.*` (under the
selected `BUILD_DIR` when overridden). Each invocation records:

- `progress.tsv`: process, entry/return, operation, monotonic/CPU clocks,
  inclusive durations, module path and function name.
- `stdout.txt` and `stderr.txt`: the original compiler streams.
- `result.txt`: command, elapsed time, completed units, child status and any
  watchdog failure explanation.

Nested timings overlap and must not be summed. A function event covers binding
its root block; later constraint checks may continue, so failures honestly say
“last function observed”. A failure reports the active stage/module, elapsed
active work, last observed function, last completed unit and evidence directory.
The fixture retains this explanation in its failure output. Hosted validation
archives the evidence alongside timing reports, including on failure.
Scheduled workflows use the default branch's workflow definition: the added
artifact paths take effect for scheduled runs when this workflow change reaches
`main`. Before then, the failure explanation is still included in the job log,
but the full trace directory is not uploaded by the older workflow.

Trace records, nesting and output sizes are bounded. Missing, malformed,
partial or mismatched progress cannot silently turn into a successful test.
The supervisor terminates the child process group and escalates to SIGKILL
when required. It also cleans up the group when interrupted.

`make compiler-watchdog-check` runs deterministic budget tests and actual
subprocess tests: healthy cumulative work, function heartbeats that cannot
renew a module budget, silent children ignoring TERM, missing/partial progress,
launch failure, five real compiler phase/function stalls, and healthy recovery.
Injected stalls use a two-second allowance; they do not slow the suite by
waiting for the normal deadline. These contracts run in `unit-runtime-native`.
Full compiler-source success remains covered by the existing fixture.

The next unexpected failure should be investigated from these records. A
successful watchdog run does not explain the earlier aggregate timeout, and
increasing an allowance is not a substitute for examining the active work.
