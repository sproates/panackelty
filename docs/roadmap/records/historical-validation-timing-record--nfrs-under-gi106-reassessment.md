[Roadmap index](../README.md) · [Current release and next work](../../../ROADMAP.md)

This is a retained planning record. Dates and checkpoints below describe their
original context; see the [roadmap](../../../ROADMAP.md) for current release status.

## Historical validation timing record — NFRs under GI#106 reassessment

Roadmap item: [RM#28: Incremental and modular builds](independent-website-publishing.md#rm-28).

The observations below preserve the timing comparisons used when they were
recorded. Owner direction on 2026-10-12 supersedes their former 120s/15s/75s/60s
acceptance language: these figures are historical evidence, not active NFRs or
gates. Current assessment and completion are recorded under [RM#28 / GI#106](independent-website-publishing.md#rm-28);
only the user's explicit statement that they are happy completes that work.

| Measurement | Latest recorded evidence | Outstanding issue |
| --- | --- | --- |
| Local platform-installation instructions (2026-10-03; warm rerun after clean harness correction) | 119s; unit 84s; functional 4s; canonical checks passed | Existing unit 15s warning persists; retain profiling reminder under #106 |
| Full cold hosted CI | Five follow-up runs: 88/106/107/103/97s, median 103s | Hosted scheduling/completion has no proven hard upper bound |
| Local coverage-host cutover (2026-10-02; Linux workspace) | 196s; unit 135s; canonical checks passed | Existing full/unit budget warnings persist; retain profiling reminder under #106 |
| Local clean check, append experiment | 107s | Environment-specific result, not a universal guarantee |
| Local clean check, coverage publication | 145s, then final 144s; unit phase 99s | Existing clean/unit warning budgets still exceeded in this workspace |
| Local clean check, adoption/workflow docs (2026-09-29) | 147s; conventions follow-up 152s (unit 104s); all checks passed | Existing clean/unit budget warnings persist; remains non-blocking |
| Local assessment validation (2026-09-29; native build already present) | 144s; all checks passed | Existing full-check budget warning persists; remains non-blocking |
| Local sorting/suffix validation (2026-09-29; clean build) | 148s; unit 102s; all canonical checks passed | Existing full-check/unit budget warnings persist; remains non-blocking |
| Local execution-design validation (2026-09-29; clean build) | 148s; unit 102s; all canonical checks passed | Existing full-check/unit budget warnings persist; remains non-blocking |
| Local resumable-VM validation (2026-09-29; clean rebuild, native prerequisites prepared first) | 150s; unit 105s; all canonical checks passed | Existing full-check/unit budget warnings persist; remains non-blocking |
| Local task/lifecycle validation sample (2026-09-29; macOS arm64, clean build) | 103s; unit 69s; canonical checks passed | Full check within 120s; unit warning persists; cross-host timings are not directly comparable |
| Local async-interface proposal validation (2026-09-29; macOS arm64, clean build) | 106s; unit 70s; canonical checks passed | Full check within 120s; existing unit warning remains non-blocking |
| Local validation sample (2026-09-29; macOS arm64, clean build) | 104s; unit 70s; canonical checks passed | Full check within 120s; existing unit warning remains non-blocking |
| Local async implementation validation (2026-09-29; Linux workspace) | Final isolated checks passed in 360s and 358s; initial 370s sample overlapped sanitizer work | Existing full-check/unit warnings persist |
| Controlled clean comparison (#104; same Linux workspace) | Pre-async 341s / unit 231s; merged async 362s / unit 245s; both passed | Most cost predates async; prioritise bounded scheduling and retain all coverage |
| Bounded local overlap (#104; same Linux workspace) | 346s / unit 224s; all checks passed; 16s (4.4%) faster in one sample | Full/unit budgets still exceeded; bootstrap warned at 61s / 60s |
| Focused local VM check | Cached median 25.598s; test-edit median 32.993s | Both above the 15s focused target |

Earlier “budget reached” statements describe their dated snapshots only. The
docs-only route, shared validated probes, bounded workers, CI partitioning and
LLVM setup improvements remain delivered work. Historical observations and the
detailed optimisation sequence are retained in
[the profiling report](../../../tests/VALIDATION_PROFILE.md#archived-roadmap-performance-history--2026-09-28).
