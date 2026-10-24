# P03b controlled Git acquisition

This packet supplies exact-commit snapshot acquisition in Panack. It is an
internal package-tool capability, not yet Git dependency support in `panack deps`.
P03c owns manifest/ref/lock integration; P03d owns cross-source acceptance.

## Ownership and bounds

Git is invoked through `process.run_isolated` with an absolute executable path,
explicit argv, a private bare repository and private HOME. System/global Git
configuration, inherited Git environment, credential helpers, prompts, hooks,
external protocols, submodules and automatic maintenance are disabled. Only
HTTPS is permitted. TLS verification remains enabled. An optional certificate
file is trusted tool configuration, never manifest-controlled policy.

The caller supplies SHA-1 or SHA-256 and a complete lowercase commit ID. Fetch
must produce that exact commit. `ls-tree` supplies inventory; `unpack-file`
extracts raw blobs without checkout attributes, filters or line-ending changes.
Panack validates all paths, modes, sizes and link targets before returning a
snapshot. Submodules and LFS pointers fail. Symlink expansion follows physical
component order, rejects escapes/cycles and preserves safe raw target bytes.
The source-key and tree-digest encodings are the P01 encodings.

Snapshots have at most 16,384 entries and 256 MiB of content. Git commands have
60-second timeouts and 16 MiB combined output limits. Files are copied and hashed
in 64 KiB chunks. Git's fetched pack and temporary object database have no byte
quota; post-fetch tree validation does not bound transfer or temporary disk use.
Redirects fail closed; use a direct HTTPS URL. No redirect is followed, which
satisfies the five-hop safety ceiling without providing redirect compatibility.
Git 2.29+ is required. Fetching a specific object also depends on the server
making that commit available; failure never substitutes another commit.

## Verification

- Thirty-four focused policy assertions cover inventory/path/size and link rules.
- Source and saved-bytecode HTTPS fixtures independently reproduce source keys
  and canonical tree digests, and inspect original file bytes and link targets.
- Hostile inherited URL rewrite, helper path, global/system configuration,
  templates and filter settings do not affect acquisition or execute a marker.
- SHA-1 and SHA-256 snapshots work. A 17 MiB binary blob exceeds the process
  output limit and still produces the independent expected digest.
- Untrusted TLS, absent/relative Git executable, unavailable commits, unsafe
  URLs, escaping/cyclic links, LFS, submodules, authentication, redirects and
  interrupted transport fail with stage cleanup.
- Publishing preserves links; a rejected replacement preserves the old snapshot.
- Compiler/native sources and bootstrap bytes are unchanged. Package handoff
  validation checks the separate build boundary.

LFS rejection includes current and historical markers, CRLF and leading Unicode
whitespace recognised by the [Git LFS parser](https://github.com/git-lfs/git-lfs/blob/main/lfs/pointer.go).
The earlier LF-only marker check was caught during self-review and corrected
before acceptance.

The first complete run failed an existing package-root race fixture: the compiler
returned its safe `cannot verify resolved project package manifest` diagnostic,
which the fixture did not classify. The fixture now counts that exact diagnostic
as a manifest-read rejection, retaining the no-mixed-output and observed-identity-change
assertions. The focused 40-attempt race passes. No compiler behaviour changed.

Final local validation and hosted Linux/macOS acceptance are pending. This record
does not claim P03b delivered until both pass and the delivery PR is merged.

## Performance and remaining work

Git runs only during explicit acquisition. Ordinary compiler execution is
unchanged. One blob is extracted at a time; per-file subprocess overhead makes
large collections of small files a throughput concern, not a measured regression
against a previous Git implementation. There was no previous Git implementation.
Existing validation timing concerns remain under GI#106; this packet does not
change that acceptance policy.

P03c must connect the staged result to the shared source-keyed cache, locks,
subdirectories and sibling/diamond graphs. It must retain package-specific
compiler symlink containment. P03b does not grant a sibling package implicit
source access. Website claims remain accurate for alpha.14; Git user guidance
waits for integration and the matching native release.
