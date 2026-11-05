# P03c Git package integration

This delivery connects P03b snapshots to the package tool's public commands.
P03d cross-source acceptance and P04 installed workflow integration remain open.
GI#568 and GI#200 are not completed by this slice.

## Behaviour and boundaries

Git manifests accept HTTPS URLs, optional branch/tag/full-commit selectors and
optional repository subdirectories. Default HEAD and annotated tags resolve only
on explicit initial lock or update. Short object IDs, revision expressions,
ambiguous branch/tag names and unknown source fields fail. Locks record full
SHA-1/SHA-256 commits and canonical subdirectories. Restore does not resolve refs.

One source-keyed complete snapshot is shared by all packages at that repository
commit. Relative sibling dependencies become Git lock edges; a diamond resolves
to one physical package root. Symlinked subdirectories are physically resolved
inside the snapshot. Compilation separately enforces package-private source
containment. Conflicting selective updates fail before lock/cache publication.

`git_refs`, `git_acquisition` and `git_cache` live under `src/packages`. The
package-owned manifest records evolve without changing the compiler's frozen
parser. The resolved descriptor, compiler/native sources, bytecode version and
bootstrap bytes are unchanged. New production modules are explicitly unmeasured
by the source-coverage corpus; no eligible-source denominator or threshold changes.

Git 2.29+ is discovered on PATH for acquisition only. It receives an exact private
environment. The caller's optional `CURL_CA_BUNDLE` is passed as trusted transport
configuration; no ambient Git configuration or credentials are inherited.
Direct HTTPS URLs are required because Git redirects fail closed. The P03b
16,384-entry and 256 MiB expanded limits remain; a fetched pack has no pre-fetch
byte quota. Each Git subprocess retains its 60-second and 16 MiB output bounds.

## Acceptance evidence

The existing Git test route now includes `tests/package_git_integration.sh`.
It runs complete programs through public project check/compile/run and saved
bytecode against a private TLS Git peer. Assertions cover:

- Annotated tags, default HEAD, full branch names and full commits.
- SHA-1/SHA-256 repositories, root packages and repeated aliases.
- Independent source-key calculation, canonical linked subdirectories, one shared
  snapshot, sibling lock edges and diamond package/root deduplication.
- No Git invocation or TCP connection for warm ordinary commands and restore;
  cold offline rejection without a connection.
- Moved tags preserving old locks, exact-commit cold restore and explicit update.
- Corrupt-cache rejection and repair; failed selective updates preserving lock
  and state; successful update of all affected aliases.
- Snapshot escape rejection, package-private link containment and stage cleanup.

The existing manifest, Git and lock unit runners cover source schema/ref parsing,
peeling, ambiguity, truncated responses and canonical locked subdirectories.
Local `make check` passed on 25 October 2026 in 699 seconds: unit 404 seconds,
bootstrap 254 seconds, release smoke 16 seconds and quick start 1 second.
The focused unit runners passed 47 Git, 41 manifest and 22 lock assertions.
Coverage-manifest/policy tests passed (13 tests). The package-tool handoff test
proved a behaviour edit rebuilds no compiler or seed. Compiler stages retain
`aab8b0557c9525ac1e917d093e1c44452ba746fde59d880b7ea3e5c2a24509df`;
the stdlib fixed point remains
`823a5f93fe2886a44b680cb09c1d634bdf6aa8fc0f5cdff57510cfe743eb8a0d`.
Exact-head hosted gates are still required before merge.

The first full run stopped on the existing root-replacement race harness's
unrecognised `panack: cannot inspect lockfile` diagnostic. The tool rejects that
changing-root lookup safely. The harness now counts that exact diagnostic
separately, retaining all mixed-output and identity-change assertions. Its
focused 40-attempt rerun passed with 39 identity changes before the successful
full run. No product failure or assertion was bypassed.

## Performance and remaining work

Ref resolution is reused within one graph traversal for the same URL/selector.
One snapshot is acquired per URL/object-format/commit, regardless of package
count. Warm verification hashes each distinct snapshot once per traversal using
64 KiB file reads. Cold acquisition also verifies the staged tree before graph
resolution; this deliberately adds a bounded second read to obtain a validated
link inventory for physical package-root resolution.

The focused fixture measured cold lock at 0.67 seconds and warm project run at
0.18 seconds (0.19 seconds in the full run). It records elapsed/user/system timings.
These are a small local HTTPS diamond, not production throughput estimates or a
before/after speed claim. P03d retains the wider cross-source and supported-platform
failure matrix; P04 retains installed-artifact, relocation and all-source acceptance.
Website publication waits for that acceptance and a matching native release.
