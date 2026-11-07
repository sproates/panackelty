# Hosted incremental compiler cache trial

This is a manual experiment in the existing Validation profile workflow. Select
`incremental_cache_trial=true` on the development branch. Its ordinary six-component
profile jobs are skipped. No development push or PR automatically runs this trial.

Two Ubuntu 22.04 jobs build their own native VM from the same commit. The cold job
compiles the compiler with an empty private cache and compares its bytecode with
a fresh compilation. It saves only the compiler cache, under a key unique to
this workflow run and attempt. The warm job restores that exact key, verifies
the payload digest passed directly by the cold job, and measures unchanged
compilation and a real function-body edit. Both results must match fresh builds.
The source edit is restored even when compilation fails.

[GitHub's cache documentation](https://docs.github.com/en/actions/reference/workflows-and-actions/dependency-caching)
describes cache scope and key matching. This experiment uses exact keys without
fallback prefixes. Compiler entries also check source, toolchain and path identity.
The payload digest prevents a restored cache key alone from serving as trust in
checked bodies. The jobs use the repository's normal hosted workspace path; this
does not establish portable reuse across arbitrary paths or package root identities.

`build/compiler-cache-trial/measurements.json` records command durations and output
hashes. Workflow steps separately expose restore/save overhead. Four local harness
tests cover fresh-output comparison, restored payload tampering, mismatched
artifacts and source restoration after failure.

The experiment does not enable cache use in ordinary Check, bootstrap proofs,
release validation, coverage collection, or package commands. A successful trial
would establish reuse between two jobs of this one run on Linux, not a general
promise of CI speedup or persistence across every runner environment. Adoption
requires selecting actual repeated compiler workloads and weighing transfer cost
against measured savings. Current status: prepared; hosted execution pending.
