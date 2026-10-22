# Module, package and HTTP design

Design outcome for [RM#109: Module and package design](../roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-109)
and [GI#234: Module and package design](https://github.com/sproates/panackelty/issues/234),
within [RM#108: Modules programme](../roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#rm-108).

This document records the P1 design accepted in PR#240 and the follow-on programme
decisions. Its tables remain design contracts rather than general feature evidence.
Alpha.13 ships the namespace, local-package, migrated standard-library and HTTP
client/server deliveries (P2–P6). P8 release acceptance is complete. The former
P7 package-management proposal was deferred from alpha.14; the owner later selected
the full GI#200 implementation for alpha.15. Its current contract is in
[PACKAGE_MANAGEMENT_CONTRACT.md](PACKAGE_MANAGEMENT_CONTRACT.md). See the
[release acceptance](../releases/ALPHA13_RELEASE_ACCEPTANCE.md) and
[language specification](../reference/SPEC.md) for current guarantees.

The delivery descriptions and estimates below are dated design history; proposed
future facilities are not promises that the current release implements them.

PR#386 completed P3 local-package acceptance. PR#372 adds confined package-root reads. PR#374 wires strict manifests to
explicit `check`, `compile` and `run --project` commands with relocation-stable
module identity. PR#375 adds exported-library checking; PR#376 adds direct local
library dependencies and two-app acceptance; PR#377–#382 establish physical-root
identity, overlap rejection, bounded graph resolution, conflict rejection and
exact graph limits; PR#383 covers source-count and byte boundaries; PR#384 makes
the root-replacement race acceptance reliable. The final integration fixture checks, runs and compiles two applications from an
unrelated working directory, moves their complete sibling tree and shared
multi-file library, then rechecks/runs both and executes bytecode compiled before
the move. PR#404 completes P4's standard-library API namespace migration on
`next`; PR#450 delivers the first end-to-end P5 client slice. Native transport
lifecycle evidence now includes deterministic deadline/readiness boundary and
cancellation/cleanup tests, plus a late TLS server-flight timeout fixture for
source and saved bytecode; late DNS completion remains open. An initial
libcurl version/build/license snapshot is recorded in
[NATIVE_HTTP_CLIENT_DESIGN.md](NATIVE_HTTP_CLIENT_DESIGN.md); dependency pinning,
complete transitive license inventory and clean-host archive acceptance remain
open. The separate clean-installed release acceptance remains in P8.
## Current implementation audit

Audit baseline: core revision `b32267e` (2026-10-03). The table records the
pre-P3 implementation audit; it is historical source inspection, not current
acceptance evidence.

| Boundary | Observed behavior and evidence | Consequence |
| --- | --- | --- |
| Parsing | [parser.panack](../../src/compiler/parser.panack) stores `ImportDeclaration(Str)` and unqualified declaration names; `parse_import_path` accepts quoted or slash-separated paths | No alias, visibility, selective import or exported-module metadata exists |
| Loading | [loader.panack](../../src/compiler/loader.panack), `load_project_mode` roots `project/` at the entry file's parent and obtains `stdlib/` from `PANACKELTY_STDLIB_PATH`; `load_project_module` traverses imports, keeps a visited set and rejects cycles | File discovery exists; package manifests and root selection do not |
| Name lookup | [resolver.panack](../../src/compiler/resolver.panack), `append_program_declarations` concatenates all declarations; `collect_symbols` rejects duplicate global names | A dependency's private-looking helper is globally exposed; two libraries cannot independently define the same helper name |
| Paths | [host.c](../../src/vm/host.c), `path_join_text`/`path_resolve` normalize lexically; `file_exists` uses `stat` | Existing canonical spellings are not a physical symlink identity or a containment boundary; package path confinement needs new host evidence |
| Types and methods | [checker.panack](../../src/compiler/checker.panack) indexes functions/types by name; parser lowers ordinary receiver calls to named calls; core methods have special identities | Qualification must survive parsing until resolution; changing only imports would be incorrect |
| Effects and emission | [purity.panack](../../src/compiler/purity.panack) resolves callees; [emitter.panack](../../src/compiler/emitter.panack) erases generic arguments into ordinary bytecode functions | All consumers must use resolved declaration identities consistently; no separate linker is required |
| Core library | Loader injects toolchain `core.panack` once and internalizes selected core algorithms; [stdlib guide](../../src/stdlib/README.md) documents implicit Option/Result and methods | Package copies must not create a second core identity or override reserved types/intrinsics |
| Evidence/tooling | [source_maps.panack](../../src/compiler/source_maps.panack), [explanations.panack](../../src/compiler/explanations.panack) and driver use source roots and function names | Diagnostics, explain/locate and source identities need explicit namespace/package migration |
| Existing tests | [import fixtures](../../tests/fixtures/compiler_contracts/imports), [module functional case](../../tests/functional/cases/modules/main.panack), [native module tests](../../tests/unit/vm/native_modules.c) | Migrate source fixtures deliberately and retain their intended behavioral checks; existing passing imports do not establish visibility or package safety |
| Networking | [TCP specification](../reference/SPEC.md#native-tcp-exchange-development-toolchain) describes numeric IPv4, half-close/EOF exchange; finite server reads request EOF before invoking a handler | Neither service supplies HTTP framing, DNS or TLS. HTTP requires new transport work, not a wrapper over current EOF services |

Current quoted relative imports may use parent components; invalid logical
`project/` and `stdlib/` paths reject them. Do not reinterpret the specification's
logical-path restrictions as a guarantee that all existing file reads stay below
an entry directory. The [launcher](../../bin/panack) supplies the active bundled stdlib;
package mode will retain that toolchain ownership.

## Selected language model

| Term | Selected meaning |
| --- | --- |
| File | One UTF-8 `.panack` source file |
| Module | Exactly one file, identified by its owning package and normalized path under its source root; no partial modules |
| Namespace | A module's compile-time name scope, bound locally by an import alias; not a runtime object or an independently reopenable declaration block |
| Package | A named, versioned source tree with one manifest, a source root, explicit exported module paths and direct dependencies |
| Application | A package with one entry module and `main`; libraries have no entry target |

No namespace declaration, wildcard import, implicit directory index, ambient
search path, executable build script, public registry or separately compiled
package artifact is introduced. Directory nesting forms import paths; it does not
automatically export ancestor/child namespaces. Whole-program source compilation
still emits one verified bytecode program executed only by the VM.

### Imports, visibility and lookup

Proposed grammar examples:

```panackelty
import project/math as arithmetic
import geometry/vector                         // local alias vector
import geometry/vector::{Vector, length as magnitude}
pub import project/api::{Response, get}         // explicit selective re-export
pub import project/errors as errors            // explicit namespace re-export
```

`project/` denotes the importing package's own source root, including when used
inside a dependency. `stdlib/` is reserved for the active toolchain. Other leading
segments are aliases in that package's direct-dependency table. There is no
fallback to the application's dependencies or the working directory. A quoted
relative import is permitted inside the same package and follows the same alias
and export rules; it cannot cross the package root.

An ordinary import binds only a namespace alias, defaulting to the final module
segment. A selective import binds only its listed exported declarations, with
optional individual aliases. Aliases exist only in the importing module and must
not collide with declarations, other import bindings, builtins, type parameters or
locals visible at the point of use. Existing no-shadowing rules remain; repeated
identical import bindings are errors rather than order-dependent exceptions.
Importing one module through distinct aliases is valid and loads it once.

Declarations are private to their module by default. `pub` applies to functions,
records, enums and guarded type declarations. A public record exposes its
constructor and every field; a public enum exposes its type and variants. P2 does
not add per-field privacy, opaque source records or extension methods. An ordinary
import exposes only `pub` declarations even within a package. Package internals
may import other internal modules, but consumers may import only manifest-listed
exported module paths. `pub` inside an unexported module does not make that module
an external entry point.

A public declaration's externally visible signature must use publicly reachable
types: core types, own public types or types exposed by explicit public imports.
Reject private-type leakage through fields, variants, parameters, returns and
generic arguments. A public guarded type must retain its checkable definition;
its guard cannot rely on an inaccessible helper in the exported contract. The
first implementation should diagnose that case rather than silently erase a
constraint. Public wrappers may call private helpers in their bodies.

Imports are never transitively opened. Re-exports preserve the original declaration
identity, type/effect contract and diagnostic origin. A module namespace re-export
allows `api.errors.InvalidUrl`; it does not make `InvalidUrl` unqualified. Export
cycles are ordinary import cycles and are rejected. Selective re-exports cannot
rename two origins to the same public name, re-export private declarations or
bypass a dependency's module export list.

`arithmetic.add(1, 2)` is a qualified call, `vector.Vector` a qualified type,
`@arithmetic.add` a named function reference, and `vector.Vector(1, 2)` a constructor.
Qualified enum variants are used in expressions and patterns, for example
`errors.HttpError.Timeout`. A public enum's variants may also be selected explicitly
using a qualified selector such as `import project/errors::{HttpError.Timeout}`;
its default local binding is `Timeout`, subject to the same conflict rules.
Variant names remain unique within their declaring module, preserving the current
no-duplicate-variant rule; own-module variants have bare bindings. Qualified
`Enum.Variant` spelling identifies that declaration without introducing a second
identity. Selecting a public enum exposes its type; selecting one of its variants
is explicit. Type parameters stay lexically scoped; unrelated module types must
not capture or prohibit a function-local type parameter. `api.Box[Nat]` and
`api.map[Nat](../...)` use
existing explicit generic argument rules after resolving the declaration.

Resolver binding distinguishes a namespace receiver from a value receiver before
lowering dots. Namespace bindings cannot be used as values or shadowed by values. Resolve an
alias before applying special `.get`/`.first` core-method lowering; `api.get(...)`
is a module call even when a core method has the same spelling.
`value.name(args)` retains receiver-first semantics: search the current module's
ordinary callable bindings, including explicit selective imports, then perform
existing argument/generic/effect checks. Merely importing an alias does not install
its functions as methods. Prefer `arithmetic.add(value, n)` when no unqualified
binding exists. Core type-directed methods retain their current reserved rules;
there is no new argument-dependent lookup or overload priority.

Each declaration gets a stable compiler identity `(package, module, local name)`.
Use a tagged identity representation (not a name beginning with `$`, which the
current checker treats as an inference placeholder). Use that identity for callable
references, nominal records/enums/guarded types,
generic substitutions, purity, lowering, emitted names and source/effect evidence.
Equal spelling across modules never implies equal nominal types; an alias or
re-export of one type remains that same type. Importing a function cannot change
its pure/ordinary/async classification; qualified async calls require `await`,
and indirect callable contracts retain all existing effect restrictions.

## Package roots, manifest and source distribution

Select a strict JSON manifest named `panack.json`, schema version 1. JSON avoids an
additional configuration language; implement its bounded parser in the project
language/toolchain, without an interpreter dependency or executing package code.
P3 sets explicit limits before enabling loading: manifests up to 65,536 bytes, each source module up to 1 MiB, and one loaded source closure up to 256 files and 8 MiB. The closure bounds include the automatically loaded toolchain core. The loader boundaries must be tested at their exact limits; the package CLI must also prove oversized source files fail closed before parsing.
The concrete schema below is the P1 baseline, not today's CLI format:

```json
{
  "schema": 1,
  "package": "example.http",
  "version": "0.1.0",
  "language": "modules-v1",
  "toolchain": "IMPLEMENTING_RELEASE",
  "source": "src",
  "exports": {"client": "client.panack", "server": "server.panack"},
  "dependencies": {}
}
```

`IMPLEMENTING_RELEASE` is an explicit placeholder, not an accepted version value.
On implementation it must be an exact supporting toolchain release identifier;
P3 initially accepts exact matching versions rather than inventing a compatibility
range. Package versions are exact `major.minor.patch` with optional prerelease,
used as identity, not resolved by a semver solver. The package name is a stable
ASCII dotted identifier; dependency aliases are single source identifiers.
Reserved aliases `project` and `stdlib` cannot appear in dependencies.

Required keys are those shown; libraries omit `entry`. Applications additionally
supply `"entry": "main.panack"`. `source` is relative to the manifest directory;
entry and export paths are relative to source. Export keys are logical slash paths
of identifiers; values end in `.panack`. Multiple keys may intentionally expose
the same module, preserving its identity. `dependencies` maps a local import alias
to `{"path": "../http"}` in P3; the path points to a directory containing a manifest
and resolves relative to the declaring manifest. External local dependencies must
therefore be declared explicitly, never reached by source import traversal.
Nested local dependencies use the same rule: an alias is visible only while
loading modules owned by the package whose manifest declares it. Repeated edges
to the same physical package reuse one package identity. Reject dependency cycles,
conflicting package identities/versions and physically overlapping source roots
throughout the resolved graph. Bound a graph to 64 package manifests including
the application, 256 dependency edges and 32 package levels including the
application.

Reject duplicate JSON keys, unknown keys/schema/language, non-string path values,
empty identities, conflicting aliases, absolute paths, NULs, invalid UTF-8 and
invalid suffixes. No interpolation, environment variables, globbing, URL lookup or
search precedence. `source`, `entry` and exports must remain within their owning
roots after physical resolution; reject symlinks escaping a root and detect
symlink aliases consistently. Parent segments are allowed only in explicitly
configured dependency paths, not in manifest-owned source/entry/export paths.
A dependency root is a separate allowed root; its contents are still confined.
Reject overlapping physical source roots across packages so a file cannot acquire
two different owners.

Identity during one compilation uses physical root identity plus module-relative
path to load each source once. The graph allows only one physical root for each
declared package name/version and one version per package name. Different roots
claiming the same identity are a diagnostic even when content looks equal; two
aliases pointing at the same physical root are allowed. Reject conflicting
versions with both dependency paths rather than choosing whichever was loaded
first. Reject package dependency cycles and module import cycles independently,
with an ordered import/dependency chain. Root identity requires a new host operation
or equivalent verified file-handle evidence; existing lexical `path_resolve` is
insufficient. Bound graph size/depth and source bytes; report the exceeded limit.

The emitted semantic identity uses declared package name/version and normalized
module path, never absolute checkout paths. Source maps retain physical origin
separately, with portable package identifiers. Relocating an unchanged graph must
preserve executable bytes and portable identities; changing module content must
invalidate recorded source evidence. Main may not discover two instances of its
own package through a dependency alias. Only the selected application entry's
`main` is an entry point; dependency functions named `main` have no launch role.

A source package contains `panack.json`, `src/`, `tests/`, documentation and license;
optional `resources/` are inert data. No automatic resource loading or runtime
working-directory change occurs. Applications locate runtime resources explicitly;
P3's library examples use pure/source-only data. Tests use separate application
manifests with explicit dependencies on the library and any test helpers; no
implicit development dependency namespace or production export widening. Ship
source trees initially. Integrity, offline locks, immutable Git acquisition and vendoring
were deferred beyond the original P1–P6 deliveries; current GI#200 scope is
defined in [PACKAGE_MANAGEMENT_CONTRACT.md](PACKAGE_MANAGEMENT_CONTRACT.md). A
binary ABI, registry and separate linker remain outside the selected programme.

### Two applications consuming one local library

Proposed layout: sibling directories `geometry/`, `measure/` and `draw/`.
`geometry/panack.json` exports `vector` from `src/vector.panack`; each application's
manifest declares `"dependencies": {"geometry": {"path": "../geometry"}}`,
`"entry": "main.panack"`, and its own package name. Both use the same library
without copying it. `geometry/src/vector.panack`:

```panackelty
import project/internal/math as internal
pub record Vector { x: Nat, y: Nat }
pub pure sum(value: Vector): Nat { add_components(value) }
pure add_components(value: Vector): Nat { internal.add(value.x, value.y) }
```

`geometry/src/internal/math.panack` defines `pub pure add(a: Nat, b: Nat): Nat { a + b }`.
That module is not in the manifest's exports, so other geometry modules can import
it but applications cannot import `geometry/internal/math`. This makes the shared
library a concrete two-module dependency.

Each app's `src/main.panack` can contain:

```panackelty
import geometry/vector as vector
main(): Void { print(vector.sum(vector.Vector(20, 22))) }
```

Expected future output is `42` in both apps. A call to
`vector.add_components(...)` must fail as private. Another dependency may define
its own `Vector`; passing it to `vector.sum` must fail as a distinct type.
Moving the entire sibling tree preserves dependency resolution and output. Moving
only the library without updating its declaring dependency path must produce a
missing-dependency diagnostic, not a fallback search.

## Compatibility, bootstrap and migration

Select one new namespace model with a coordinated breaking preview migration.
There is no permanent flat-import mode, automatic compatibility alias surface or
mixed-semantics graph. Existing `panack check FILE`, `compile FILE` and `run FILE`
remain commands, but their source now uses the namespace rules above. Without a
manifest, the entry's directory is a single implicit application root: `project/`
resolves there, and dependencies are limited to its local files and toolchain
stdlib. Its compiler package identity is a reserved standalone identity scoped to
that compilation, never a dependency package or user-declared package name.

New proposed `panack check --project PATH/panack.json`, `compile --project ...` and
`run --project ...` explicitly select a manifest and its entry. Do not discover a
nearby manifest silently. Library validation uses `check --project` without an
entry to check every exported module and reachable closure. Compiling/running a
library without an entry is an error. Explicit root selection changes dependency
configuration, not language semantics. New flags/output options require driver
and installed-toolchain tests.

Migrate the self-hosted compiler, stdlib, tests, examples, tutorials and package
inputs together: declare public boundaries, give imports aliases or explicit
selective bindings, qualify cross-module types/calls and remove accidental
transitive visibility. Keep private helpers private; do not mechanically mark all
declarations public to restore flattening. Old source may require edits. Record
this breaking change in the supporting preview release notes; old released
artifacts remain usable for their documented versions, with no promise that old
source compiles unchanged using the new toolchain.

The stdlib remains toolchain-owned. P2 supplies an audited explicit export inventory
needed for initial namespace compilation; P4 chooses its final public grouping and
re-export migration. Core identities stay reserved/shared, including Option/Result
constructors and methods; copied core source gains no core privilege. Transitional
export metadata, if needed to bootstrap source `pub` declarations, is a temporary
build input, not a second installed language mode. Remove it when the migrated
compiler and library compile from the new seed; P4 cannot close while obsolete
adapters or prefixed compatibility wrappers remain without a separately accepted
reason.

Implementation sequence: retain module/import nodes and unresolved qualified
syntax; construct export and binding tables; resolve to unique declaration IDs;
then use the same checked identities throughout purity, emission and diagnostics.
Preserve source spans at each step. Extend public `explain`/`locate` function
selectors to unambiguous package/module-qualified identities, retaining old
unqualified selectors only when unique. Never select the first duplicate silently.

Bootstrap P2 using the existing audited seed to compile the new frontend while
its implementation sources still use seed-supported syntax. Then produce and
audit a namespace-capable seed, migrate implementation sources, and prove the
fresh fixed point. A temporary bootstrap bridge may exist only for this sequence,
with its exact purpose/removal criterion recorded; it must not become permanent
user-facing dual semantics. Before P3's manifest-root support, the bootstrap build
stages a temporary root-level entry file in an isolated source snapshot, importing
the migrated compiler driver by an explicit alias and calling its public entry
function. The snapshot root contains the compiler/bytecode closure; it must not
use `src/compiler/` as a confinement root and then permit escaping `../` imports.
Migrate shared stdlib references to the reserved toolchain root. Remove the staged
entry/snapshot after the build; replace this bootstrap root staging with an
explicit compiler manifest in P3. Apply the same explicit-root discipline to
source test harnesses needing repository-wide imports. Compare fresh
stage-2/stage-3 artifacts and include independent
fixtures. Update compiler sources, tests, examples, packaged stdlib, docs and
browser assets in selected migration slices. Namespace semantics run in shared
compiler bytecode, not browser source rewriting; native HTTP availability is a
separate platform capability. No bytecode format bump is assumed: P2 must prove
unique lowered names and native/browser conformance under v9 or explicitly justify
a version change. No namespace, HTTP or package claim belongs in the current SPEC
or website until its implementation/release acceptance is met.

## HTTP client and server boundary

Select native-first, bounded HTTP/1.1 packages with ordinary public data types and
explicit `Result` failures. The protocol parser/serializer and typed facade belong
to source packages. Runtime transport supplies bounded duplex I/O, DNS and verified
TLS through internal async operations; source users receive no raw socket handles.
Existing EOF TCP operations retain their contracts and cannot serve as this backend.
No new source spawn/resource ownership syntax is a prerequisite.

### P5 client contract

First client milestone supports GET over `http` and `https` to ASCII DNS hostnames
and numeric IPv4, with explicit ports and origin-form paths/query. Reject userinfo,
fragments, control characters, non-ASCII hostnames and IPv6 in this initial contract.
Do not silently reinterpret unsupported URLs. HTTPS always verifies certificate
chain, validity and hostname, with SNI for hostnames. Never fall back to plaintext
or offer an insecure switch. System trust is the production default; isolated
fixtures use an explicitly supplied test trust bundle with identical verification.

Return a `Response` containing a status `Nat`, ordered repeated `Header` values
(name and value strings), and a `Bytes` body. Status codes including 4xx/5xx are
successful protocol responses, not transport errors. Validate status range,
header names/values and framing before delivery. Preserve repeated header order;
header lookup is ASCII case-insensitive. No automatic redirect, cookie store,
proxy discovery, retry, authentication or decompression; callers see exact body
bytes. Send `Accept-Encoding: identity` and `Connection: close`. One connection
per operation; no persistent pool, HTTP/2, HTTP/3, websocket or streaming API.

The parser must accept bounded informational responses, then exactly one final
response, using HTTP/1.1 Content-Length, valid chunked framing or connection-close
framing where permitted. Handle body-forbidden statuses. Reject conflicting
Content-Length, Transfer-Encoding plus Content-Length, invalid chunk lengths,
truncated bodies and extra framed bytes. Bound headers, informational responses,
trailers and body; chunk framing is excluded from returned body bytes and trailers
are validated then discarded in the first API. Parsing success never depends on
the remote server half-closing before receiving a request.

Proposed public API shapes (future declarations; bodies omitted):

```text
Header { name: Str, value: Str }
Response { status: Nat, headers: [Header], body: Bytes }
ClientLimits { timeout_ms: Nat, header_limit: Nat, body_limit: Nat }
HttpError = InvalidUrl(Str) | InvalidLimits | DnsFailure | ConnectFailure
          | TlsFailure | Timeout | ProtocolError(Str) | LimitExceeded(Str)
          | Cancelled | UnsupportedHost
async get(url: Str): Result[Response,HttpError]
async get_with_limits(url: Str, limits: ClientLimits): Result[Response,HttpError]
```

Error categories are stable; diagnostic detail must not expose credentials or
platform-dependent numbers as contractual strings. Initial hard maxima: overall
operation 60 seconds, 64 KiB response headers/trailers together, 1 MiB response
body and eight informational responses. Limits must be positive and within hard
maxima. `get(url)` uses a 10-second total deadline, 64 KiB headers and 1 MiB body;
`get_with_limits` permits explicit bounded overrides without overloading. The total deadline starts before DNS and covers connect, TLS and body.
Execution destruction cancels and releases all operation resources, even though
no source caller remains to receive `Cancelled` in that case.

Example future application, consuming only the exported client module:

```panackelty
import http/client as http
pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}
async main(): Unit {
  result = await http.get("https://example.com/")
  match result {
    Ok(response) => expect(response.status >= 100),
    Error(error) => expect(false)
  }
  ()
}
```

This syntax illustrates module qualification and the current async effect boundary.
The pure assertion helper follows the existing network examples; P8's executable
walkthrough must assert the fixture's exact status, headers and bytes. An async
example must not call ordinary blocking `print` merely to look runnable. Deterministic local HTTP/TLS fixtures are the
acceptance oracle; this public URL is illustrative, not a required CI dependency.

### P6 server contract

First server binds numeric IPv4 and implements cleartext HTTP/1.1 only. Server TLS
termination is deliberately excluded; reverse proxies are an optional deployment
choice, not acceptance evidence for native server TLS. This differs from the
mandatory verified HTTPS client. Each connection accepts one bounded request and
returns one response with `Connection: close`; no pipelining or keepalive.
> Historical alpha.13 design below: the alpha.14 development branch replaces
> these HTTP server facades with the [routed framework](../../packages/http/README.md#server-alpha14-development-branch).
> The finite server snippet is retained as design history, not a current example.

Support GET, HEAD and POST, headers and bounded bodies with strict Content-Length or
chunked framing. Parse incrementally without requiring EOF; reject ambiguous
framing, unsupported transfer codings and oversize input before handler activation.
Unknown valid methods produce 405; unsupported protocol versions produce an
appropriate error response and close. Invalid input never reaches user handlers.

Source handler shape: `AsyncFn[Request,Result[Response,HttpError]]`. Request contains
method, raw target, ordered headers and body bytes; it performs no implicit URL
routing/decoding or filesystem access. The server serializer owns wire framing and
rejects handler-supplied framing contradictions or header injection. Do not expose
internal connection state as constructible records.

For HEAD, handlers return the same representation as for GET. The server validates
its headers and bounded body, preserves the representation Content-Length, and
omits the body on the wire. This also applies to parser errors once the exact
`HEAD ` method token is available, handler failures and invalid handler responses.
`serialize_head_response` exposes that serializer behaviour for package consumers;
`serialize_response` retains its existing full-response behaviour. The serialized
GET representation must still fit the 1 MiB package limit. The configured transport
response limit applies to the bytes actually sent.

Proposed facade shape (future APIs):

```text
Request { method: Str, target: Str, headers: [Header], body: Bytes }
ServerLimits { clients: Nat, concurrency: Nat, request_limit: Nat,
               response_limit: Nat, client_timeout_ms: Nat,
               admission_timeout_ms: Nat, drain_ms: Nat }
async serve(address: Str, port: Nat,
            handler: AsyncFn[Request,Result[Response,HttpError]],
            limits: ServerLimits): Result[[Result[Unit,HttpError]],HttpError]
```

Export Header/Response/HttpError through both facades from one shared module, so
`client.Response` and `server.Response` are the same nominal type. Future server
example:

```panackelty
import http/server as http
import stdlib/text as text
async handle(request: http.Request): Result[http.Response,http.HttpError] {
  Ok(http.Response(200, [], text.encode_utf8("ok")))
}
async main(): Unit {
  report = await http.serve("127.0.0.1", 8080, @handle,
    http.ServerLimits(1, 1, 4096, 4096, 5000, 10000, 1000))
  // P8 asserts every per-client report in the acceptance harness.
  ()
}
```

P6 keeps current finite server admission/concurrency/deadline/drain
semantics and hard maxima where applicable, plus a 64 KiB aggregate header limit.
Stop admission at the count, admission deadline or host stop request; close the
listener, drain accepted handlers to the earlier per-client/drain deadline, cancel
remaining work, then return ordered reports. Destroying the owner cancels immediately.
Each child owns its request/response buffers and nested client operation; no callback
or resolver/TLS operation may retain freed execution state.

P6 is sequenced before P5 by owner decision on 2026-10-14. The existing
`tcp_serve` receives one byte buffer only after the peer closes its write side;
HTTP clients normally keep the connection open while waiting for the response.
Therefore the existing API cannot serve HTTP unchanged, although its bounded
listener, polling, child-execution and shutdown ownership are reusable. The P6
owner-selected design exposes bounded incremental per-client reads through the
existing server owner, so the Panackelty HTTP package parses request framing and
constructs the public Request value. The handler's serialized response returns
through the owner's bounded write path. The new read operation must be scoped to
its active child execution, deadline and client; it must not expose a forgeable
socket handle or permit reads outside a server handler. Keep P5 client
interoperability as a later P8 check once P5 exists.

P6 is planned as three implementation slices:

1. **Incremental TCP service:** add an async server worker with a typed context
   and a bounded read-chunk operation available only to that active child.
   Preserve the existing EOF-delimited `tcp_serve` contract, listener ownership,
   per-client deadlines, cumulative request limits, cancellation and bounded
   response writes. Test source and saved bytecode with a client that leaves its
   write side open, plus bounds and out-of-scope rejection.
2. **Panackelty HTTP package:** define shared request/response/error types and
   source-owned incremental HTTP/1.1 request parsing and response serialization.
   Add the finite `http/server` handler facade, strict framing and header/body
   limits, method/version failures, and handler/error mapping. Keep parser and
   serializer tests independent of the native socket owner.
3. **Interoperability and acceptance:** exercise the packaged source API with
   independent clients, malformed/partial and oversized inputs, concurrency,
   disconnect, shutdown and port-conflict cases. Add the runnable example and
   installed-package/source/bytecode checks. P5-client interoperability is
   added after P5 exists.

These slices partition P6's existing scope; they do not add HTTP TLS, keepalive,
pipelining, HTTP/2, a framework or P5/libcurl dependency.

### Native prerequisite decisions and acceptance gates

P5 uses libcurl as the initial native client transport for the alpha.13 proof of
concept. Fold the needed feasibility checks into the first implementation layer;
do not create a separate backend bake-off before implementation. Keep the HTTP
package and host adapter independent of libcurl so the proof can be replaced if
implementation reveals a concrete incompatibility. This choice does not commit
future releases to libcurl. Retaining the source parser is required: a backend
that owns HTTP framing would need an explicit design amendment, not a hidden
change to this contract. Do not implement cryptography or certificate validation
in Panackelty.

DNS must be bounded by the operation deadline and cancellable without blocking the
VM event loop. A worker/resolver must be joined/quiesced or detached from execution
memory before destruction; unresolved work may never deliver into freed state.
TLS handshakes and socket reads/writes progress through cooperative polling with
explicit buffer/connection ownership. Native capability gates deny unavailable
operations in embedded/browser hosts; no silent browser fetch substitution. Audit
malformed bytecode arguments before allocation/I/O, fault-inject cleanup paths,
and test resolver/TLS late completion and deadline-versus-readiness races.

If libcurl cannot meet these gates in the supported builds, stop and revisit the
backend choice; do not downgrade hostname HTTPS to numeric-address HTTP and call
the first milestone done. Record the tested libcurl version, build configuration,
license and transitive dependencies with the implementation evidence. P6 reuses
the accepted bounded stream/ownership mechanism but does not use libcurl as a
server backend or block initial client acceptance. Public package facade,
transport contract and installed release support must be tested together.

#### Libcurl proof-of-concept boundary

Owner decision, 2026-10-14: use libcurl as the initial P5 client backend for the
alpha.13 proof of concept. This is an implementation choice for the proof, not a
permanent dependency commitment. Keep it behind a replaceable internal adapter;
revisit the choice if implementation evidence shows a hard incompatibility with
the security, lifecycle, packaging or platform requirements below.

Use libcurl only for hostname resolution, connection setup and verified TLS in
connect-only mode. Panackelty sends and receives raw connection bytes through
libcurl's raw I/O operations; the source package continues to construct requests,
parse HTTP framing and enforce response limits. Do not use libcurl's HTTP transfer
API for this design. Do not expose libcurl types, option names or error strings in
Panackelty source APIs, bytecode contracts or public error variants.

The runtime adapter owns opaque libcurl handles and exposes only the transport
operations needed by the VM. With the multi interface, an easy handle remains
attached while raw I/O is active; non-ready sends and receives return to the
cooperative runtime for readiness polling and retry. Cleanup must remove and
destroy handles before releasing operation state. Prove that connection setup,
including DNS, does not block the VM event loop, and that cancellation and late
completion cannot retain or access freed execution state. The chosen asynchronous
resolver configuration is part of the tested build, not an ambient system
assumption.

Keep the HTTP parser and public package contract independent of the adapter. Add
a transport conformance suite for connect, raw duplex I/O, verified hostname TLS,
timeouts, cancellation and cleanup; retain separate backend-free parser tests.
This allows a later native transport to replace libcurl without exposing a
libcurl migration to Panackelty programs. Replacement still requires rerunning the
same conformance and package acceptance tests. Pin and test the TLS backend and
trust configuration on Linux and macOS, and make proxy behavior explicit rather
than inheriting unexpected environment settings. The implementation must show
that the dependency can be reproducibly built and included in interpreter-free
native packages.

## Positive and negative acceptance plan

This matrix records intended positive and negative evidence across P2–P7; it is
not a claim that every row is complete. P3's scoped local-package acceptance is
summarized above. HTTP, ownership and P7 reproducibility rows remain future work.
Use focused frontend/host tests and complete programs through `panack`; compare
source and bytecode results, bootstrap stages and packaged/relocated toolchains.

| Area | Positive evidence | Negative evidence |
| --- | --- | --- |
| Module scopes | Two modules each define private `helper`; explicit aliases select the right public result | Private access, duplicate alias/declaration, hidden transitive name and namespace-as-value fail with origin-aware diagnostics |
| Public API | Selective aliases, nested namespace re-export and repeated aliases retain identity | Re-export private/unexported module, conflicting public names and leaked private signature type fail |
| Types/generics | Qualified records/enums/guarded types, constructors/patterns, generic calls and re-export identity | Same-spelled distinct nominal types mismatch; nested generic arguments containing same-spelled distinct types and wrong generic arity retain useful qualified diagnostics |
| Methods/effects | Explicitly selected callable supports receiver syntax; qualified pure/async calls and references preserve contracts | Namespace alias never becomes value receiver; missing unqualified method, pure-to-impure call and bare async call fail |
| Graph identity | Diamond dependency shares one module/core; lexical and physical aliases load once | Module cycle, dependency cycle, two roots claiming same package, version conflict and fake core reject deterministically |
| Root resolution | Two apps share local library; relocate sibling tree/install; run from unrelated working directory | Missing manifest/module, export miss, absolute/parent/NUL paths, symlink escape, atomic package-root replacement, and physically overlapping source roots across packages reject |
| Manifest | Minimal library/application and deliberate multiple exports round-trip through parsed representation | Duplicate/unknown keys, malformed JSON, wrong schema/version/language, graph/depth/byte limits and incompatible toolchain fail before compilation |
| Migration | Migrated fixtures/compiler/stdlib/examples pass under one namespace model; public CLI stays stable; new seed reaches fixed point | Old flat/transitive assumptions fail clearly rather than silently resolve; no nearest-manifest surprise or shipped dual semantics |
| Tooling/bootstrap | Qualified explain/locate, precise spans, fresh fixed points and relocated deterministic executable bytes | Ambiguous selector, stale source map, wrong package identity and old runtime capability reject rather than misattribute |
| HTTP client | Hostname HTTP/HTTPS, trusted certificate, status/header/body, repeated headers, chunked/length/close bodies | Unknown host, certificate chain/hostname/expiry rejection, truncation, framing ambiguity, timeout, limits and unsupported URL fail explicitly |
| HTTP server | Real client sends complete HTTP request without write-half-close; GET/HEAD/POST handler, ordered reports, finite shutdown | Request smuggling/framing ambiguity, header injection, admission/response bounds and handler failure; invalid requests never invoke handler |
| Ownership | Repeated success/failure, cancellation, concurrent server/nested client and relocation of installed package | Fault-injected allocation/I/O, DNS/TLS late completion and cancellation during each phase leave no live sockets/children/callback references |
| P7 reproducibility | Locked clean and offline vendored graph reproduces source identities/output | Missing/stale lock, changed digest, corrupt source, unavailable uncached artifact and conflicting package identity fail closed |

For path containment, check and open must refer to the same validated object, or
fail when it changes; a check-then-unconfined-open implementation is insufficient.
P3 tests must include symlink replacement races where the host supports them.
Package confinement is a source-loading rule, not a sandbox for running package
code. Existing host capability controls still apply.

## Dependency reproduction

This dated P7 proposal is historical input only. The Alpha.15 owner decision and
P01 specification supersede its unresolved source formats, lock lifecycle,
offline behavior and acceptance details. Use the current
[package-management contract](PACKAGE_MANAGEMENT_CONTRACT.md) for implementation.
Tests with separate
application manifests have separately reproducible graphs; production dependencies
cannot acquire test-only packages by incidental load order.

## Delivery slices and remaining uncertainty

Estimates include meaningful tests, docs, bootstrap, packaging and review. They are
coarse PR ranges, not deadlines or authorisation. Keep the current programme
weights unchanged; revise only from implementation evidence with an explicit
baseline change. A larger PR count is not a larger completion percentage.

| Stage | Proposed slices / acceptance | Estimate and dependencies |
| --- | --- | --- |
| P1 / RM#109 | Current audit, selected semantics/migration, examples, gates and estimates in this document | Medium; 1 design PR; accepted only on merge |
| P2 / RM#41 | AST/bindings and diagnostics; qualified type/effect/emission identity; stdlib adapter/bootstrap/tooling conformance | Large; original 3–5 PR estimate requires reassessment; remaining count unquantified pending effect/emission, guard/general inference, migration and acceptance decomposition |
| P3 / RM#43 | Strict manifest/explicit CLI; physical root confinement/graph identity; two-app and relocated-install acceptance | Large; 2–4 PRs after P1/P2; host path identity/race safety is a prerequisite, not existing capability |
| P4 / RM#42 | Public stdlib inventory/names; explicit exports/re-exports and prefix migration; remove bootstrap adapter with coordinated source migration verified | Medium; 1–2 PRs after P2/P3; preserve core identity and existing API behavior |
| P5 / RM#110 | Libcurl-backed bounded DNS/TLS/duplex host mechanism; source protocol/facade; independent HTTP/HTTPS client acceptance | Large; 4–7 PRs after P1–P3; re-estimate from implementation evidence |
| P6 / RM#111 | Request parser/serializer; finite async handler integration; adversarial lifecycle and packaged walkthrough | Large; 2–4 PRs after P1–P3; P5 interoperability is checked when P5 is delivered |
| P7 / RM#44 | Lock/digest model; bounded immutable Git restore; explicit vendor/offline workflow; corrupt/conflicting graph and cross-machine acceptance | Large; 3–5 PRs independently after P1/P3; remote acquisition/backend packaging add uncertainty |
| P8 / RM#112 | Language/local-package acceptance checkpoint after P1–P4; HTTP-client checkpoint after P5; final all-outcomes acceptance including P6/P7 pinned/offline reproduction | Medium; 1–2 acceptance/release PRs, provisional and to be reassessed across three checkpoints; final closure waits for P2–P7 |

The sequence selected on 2026-10-04 placed a language/local-package release
after P4 and before HTTP. The owner superseded that release ordering on
2026-10-14: no public release or core promotion occurs before SP1 implementation
is complete. The P1–P4 installed-package checks remain final release-acceptance
evidence and do not block P5. P8 retains three acceptance checkpoints:

1. **Language/local-package checkpoint after P1–P4:** a clean installed toolchain
   builds and runs two independent applications consuming one local library;
   relocated toolchain/package trees, namespace/stdlib migration, executed docs,
   fresh bootstrap and required native/browser baseline conformance pass. This
   may use a locally or CI-built release archive; it does not promote core or
   publish a release, and it does not block P5. Complete this evidence before
   final alpha.13 release acceptance. Neither HTTP nor P7 is required for this
   checkpoint.
2. **HTTP-client gate after P5:** validate the independent local-package client
   against the bounded HTTP/HTTPS, TLS rejection, timeout, malformed-response and
   cleanup contracts. The server (P6) and dependency reproduction (P7) may follow.
3. **Final all-outcomes checkpoint:** accept every task, including P6's server
   lifecycle/walkthrough and P7's pinned remote restore and verified offline
   reproduction, with clean/relocated workflows, executed docs and release
   candidate evidence. Publish alpha.13 only after all scoped SP1 outcomes and
   final release acceptance pass. Earlier checkpoints do not close P8 or the
   programme as a whole.

These checkpoints partition existing P8 acceptance without changing scope or
weights. P7 is independent once P1/P3 are accepted, not a prerequisite for P5.
No version is allocated and the final release does not imply leaving alpha.
Reassess P5's estimate as implementation evidence accumulates.
Browser namespace conformance belongs to P2/P4; browser HTTP support is not
implied. GI#166 typed service exchange and GI#162 preview-server replacement
remain separate work. The [roadmap checkpoint](../roadmap/records/modules-packages-and-http-programme-sp1--2026-10-03.md#namespace-release-checkpoint)
reconciles this decision with the earlier namespace and intervening alpha.11
release decisions; each merge and release still needs its normal approval.

P1 design acceptance evidence consists of this source-grounded audit, concrete
language/package decisions, two-app and HTTP examples, migration/ownership rules,
positive/negative gates and the staged estimates, plus independent revision-pinned
review and required repository validation in its delivery PR. No executable feature
is claimed. No post-merge live acceptance is required for this design. Website
impact: none, because shipped syntax, release artifacts and offered capabilities
are unchanged; future implementation deliveries must record adoption follow-ups.
