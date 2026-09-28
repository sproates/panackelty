# AI-assisted delivery pilot — 2026-09-29

Work record: [issue #86](https://github.com/sproates/panackelty/issues/86).
Baseline: `f1b083f0be3a70ae0677feb268f56d3b556b7807` (merged assessment PR #87).
The user authorised merge and experiment execution on 2026-09-29. This report
completes the bounded delivery arm; implementation priorities and any further
experiment still require review. [ROADMAP.md](../ROADMAP.md) owns current status.

## Result

Panackelty passed the independent initial and maintenance acceptance cases in
both repeats of the inventory and exact-ledger tasks: **four successful trials
out of six**. Both HTTP trials were blocked by missing networking support.
Python passed all six trials at both stages. No submission received hidden-test
feedback or coordinator-written fixes between stages.

This shows that fresh coding-agent sessions can deliver and change these small
Panackelty applications using repository documentation. It does not demonstrate
agent preference, superiority over Python, production readiness or broad adoption.
The HTTP result confirms a known capability gap rather than discovering a new one.

| Task | Panackelty initial → maintained | Python initial → maintained | Meaning |
| --- | --- | --- | --- |
| File inventory, then suffix filter | 2/2 → 2/2 | 2/2 → 2/2 | Current typed filesystem APIs supported complete small CLI delivery |
| Exact ledger, then refunds | 2/2 → 2/2 | 2/2 → 2/2 | Exact totals and signed changes were achievable in both languages |
| JSON HTTP service, then prefix query | 0/2 → 0/2, blocked | 2/2 → 2/2 | Panackelty lacked networking; no replacement-language service was allowed |

Do not average individual assertions across tasks: an absent HTTP deliverable
fails its delivery gate once, whereas a running service has many request cases.
The task-level comparison above counts the blocked task fully.

## Method and limits

- Twelve fresh-context participant sessions, same inherited model configuration,
  three briefs × two languages × two repeats. Six sessions ran concurrently per
  repeat on the same host. Repeats are exploratory, not statistically independent
  evidence across models. Exact serving revision and sampling seed were unavailable.
- Panackelty 0.1.0-alpha.9/bytecode 8; Python 3.12.14; Node 24.19.0 evaluator.
  Native toolchain was prebuilt. Setup/install friction was not measured.
  Both languages had local documentation/help access, no network or package
  installation, standard-library-only application code and no foreign-language
  application delegation. Documentation corpus and model familiarity differ.
- Participants used separate directories but shared a machine and filesystem.
  Non-inspection of other trials/evaluator was an instruction, not an OS isolation
  guarantee. Commands/outputs were logged; the evaluator was withheld from
  participants, and no evaluation results were fed back before maintenance.
- Initial artifacts were snapshotted before the fixed maintenance request, then
  final artifacts were snapshotted and independently checked. Participants could
  write self-tests, including Python test drivers for Panackelty; that measures
  application delivery, not a Python-free development experience.
- The proposed 15-minute and 40-command limits were enforced at the command
  wrapper. Every trial finished within five minutes and at most 15 commands.
  The proposed 12,000-token cap could only be requested in the prompt: the agent
  interface exposed neither a hard token limit nor token/cost telemetry. It was
  **not verified or enforced**. This deviation was disclosed before trial launch;
  no token count, billing cost or cost advantage is claimed.
- Elapsed figures include dispatch, concurrent contention, coordinator evaluation
  and waiting for the maintenance prompt. Command counts are shell batches, not
  semantic actions or token counts. Neither supports a precise speed ratio.
- Tests were specified independently of submitted code; object-key order was
  made semantically irrelevant before any submission was evaluated. A deliberately
  empty inventory implementation failed all nine final evaluator cases.
- Tests cover bounded fixtures, malformed input, exact outputs and later changes.
  They are not exhaustive. In particular, root/sandbox restrictions prevented
  privilege-drop permission tests; no independent permission-denied, load,
  race, production HTTP security or all-input-size certification is claimed.
- The language-choice arm, TypeScript/Node.js comparison and independent human
  onboarding were not run. No implementation or repository setting was changed.

## Per-trial observations

Times are seconds from recorded trial dispatch to coordinator snapshot; counts
include documentation, implementation and self-test shell batches.

| Trial | Initial cases | Final cases | Commands | Initial / final elapsed |
| --- | --- | --- | --- | --- |
| inventory-panack-1 | 6/6 | 9/9 | 10 | 108 / 177 |
| inventory-panack-2 | 6/6 | 9/9 | 11 | 124 / 194 |
| inventory-python-1 | 6/6 | 9/9 | 3 | 83 / 143 |
| inventory-python-2 | 6/6 | 9/9 | 3 | 70 / 124 |
| ledger-panack-1 | 13/13 | 14/14 | 15 | 204 / 288 |
| ledger-panack-2 | 13/13 | 14/14 | 13 | 154 / 286 |
| ledger-python-1 | 13/13 | 14/14 | 5 | 108 / 166 |
| ledger-python-2 | 13/13 | 14/14 | 3 | 107 / 181 |
| http-panack-1 | Blocked | Blocked | 6 | 97 / 143 |
| http-panack-2 | Blocked | Blocked | 5 | 107 / 140 |
| http-python-1 | 12/12 | 15/15 | 5 | 166 / 222 |
| http-python-2 | 12/12 | 15/15 | 4 | 144 / 195 |

Python used fewer shell batches on the supported CLI tasks. This is a useful
friction signal, with familiarity and documentation reads as confounders; it is
not a controlled measurement of model intelligence or language performance.

## What required repair or extra work

| Observation | Evidence | Implication |
| --- | --- | --- |
| Numeric proof/notation friction | inventory-panack-1's suffix change triggered a Nat subtraction diagnostic; ledger-panack-1 repaired separators and guarded digit conversion | Improve guidance and library operations around common tasks; retain safety checks rather than weakening them |
| Surprising interpolation | inventory-panack-2 initially emitted field interpolation literally, then bound the field to a local variable | Clarify supported interpolation and investigate helpful feedback; a successful compile alone did not establish correct output |
| API discovery guesses | Both Panackelty ledger agents tried a nonexistent `stdlib/io.panack` before finding documented APIs | A concise discoverable API index/examples may help earlier than a large editor integration |
| Repeated utility implementation | Both Panackelty ledgers wrote a merge sort; both inventory changes used reverse plus starts_with for suffix matching | Sorting and suffix helpers are concrete small library candidates; evaluate public naming and contracts |
| Exact arithmetic was not exclusive | Both Python ledger agents used arbitrary-precision integer thousandths and passed the exact-money cases | Do not claim an exactness advantage from these results alone |
| HTTP delivery stopped at a capability boundary | Both Panackelty agents inspected ABI/docs and recorded a missing networking interface | Host/network integration is necessary for this application class; smarter diagnostics cannot supply it |
| Python also needed review | HTTP agents adjusted port parsing/body handling after inspection; no independent evaluation failures remained | Passing examples or initial self-tests is not enough for either language |

Raw nonzero command counts are not repair counts: negative probes, usage tests
and failed searches intentionally return nonzero. Command traces and participant
notes preserve the distinction. The coordinator supplied only fixed briefs and
maintenance requests, with **zero implementation repair interventions**.

## Human readability and maintenance review

The coordinator reviewed both languages with the same qualitative rubric:
clear names/units, cohesive helpers, explicit validation/error boundaries,
necessary versus incidental complexity, and locality of the maintenance change.
This was not a blinded or independent user study; no numerical readability score
is justified.

- Both languages separated CLI handling from useful work and retained exact
  arithmetic. Maintenance changes preserved the independent initial cases.
- Panackelty inventory code expresses error handling explicitly but nests several
  Result matches. Its suffix workaround is understandable once explained, yet
  less direct than a named operation. A Result-propagation design may help but
  was not tested and should not be rushed into syntax.
- Both Panackelty ledgers contain hand-written parsing/sorting. One uses byte
  codes and `% 48` after digit validation; both convert signed magnitudes through
  rational-to-natural conversion. These choices passed the cases but add reading
  burden. The other ledger duplicates final-line accumulation logic.
- Python uses existing regex, sorting and suffix APIs, reducing application code.
  Compact syntax alone is not proof of maintainability. Its HTTP implementations
  use different server/concurrency choices despite passing the same bounded tests.
- Natural-number safety, exactness and explicit effects remain useful constraints;
  the experiment suggests helping developers express intent within them.

## Recommended next priorities — for user review

1. **One small library-ergonomics PR:** scoped sorting and literal suffix support,
   with documented examples and meaningful edge-case tests. Both tasks repeated
   this work, making this a stronger immediate candidate than speculative
   autocomplete or a broad diagnostics framework. Estimate S–M; review existing
   API/method conventions and name collisions first. Success is replacing the
   bespoke trial helpers while preserving all initial/maintenance acceptance.
2. **A bounded host/network integration design:** define host-controlled lifetime,
   errors, ownership, permission boundaries and one minimal networking path.
   Estimate S–M investigation, large delivery. Avoid committing simultaneously
   to a browser backend, mobile toolkit, full FFI and asynchronous language model.
3. **Targeted compiler/API assistance:** investigate the observed interpolation,
   conversion/proof and discovery friction; structured diagnostic output remains
   useful but was not demonstrated to be the largest blocker. Estimate S–M per
   bounded outcome, with false-positive and backward-compatibility checks.

Namespaces/packages and source mapping/coverage remain important enabling work,
with the dependencies in the [assessment](ADOPTION_ASSESSMENT.md). This small
pilot does not rank every backlog item or justify indefinite deferral of them.
Further comparative trials should answer a specific remaining question, preferably
rerunning the affected tasks after a useful change rather than expanding the
benchmark merely to gather more numbers.

## Evidence and validation

[Evidence index](experiments/agent-pilot-2026-09-29/README.md) provides public
result data and provenance, plus instructions for requesting the private raw
archive. Automatic approval review blocked public upload of prompts, logs and
environment details, so the raw archive was preserved privately. The public
summary alone cannot reproduce the acceptance suite. Restoration was checked
by replaying one Panackelty inventory and one Python HTTP submission successfully.
Full model reasoning and billing records were unavailable; archived commands/notes
are not presented as complete model transcripts.

Repository changes are reports, frozen evidence and roadmap state updates.
Production validation retains its existing language/dependency policy. Experiment
completion is ready for review; issue #86 remains open until this report and the
repository completion summary are merged with explicit permission.
