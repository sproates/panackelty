# P01 built-in and source API register

Baseline: `5327c2b6789e47801a03e663b007fd2a6f9da0c9`. Scope: all 100 spellings accepted by `resolver.is_builtin`, plus the three special core source methods and the separate [82-declaration stdlib inventory](dispatch-stdlib-inventory.md). See [P01 results](p01-inventory.md) for evidence and decisions.

## Contract preservation

The current full signatures, effects, generic relationships and failure contracts are defined by `module_bodies.panack` (checked-identity calls), `checker.panack` (legacy frontend) and `purity.panack`. This register selects ownership and removal, not new parameter/result contracts. The [call-site ledger](p01-call-sites.md) records observed receiver/result types and resolved targets. Observed types are evidence, not a closed list of legal generic instantiations.

For each owned entry, remove the first explicit receiver slot and preserve the rest of the checked contract, including runtime trap behaviour. Copy the accepted type-family predicate from the existing checker into per-owner signatures or declaration-associated intrinsic validation; do not infer the entire accepted domain from observed callers. The source-method signatures are listed verbatim in the stdlib inventory.

All built-in receiver declarations belong to the toolchain core registry under D4. Intrinsic implementations remain internal and selected by declaration identity. Low-level host globals used to implement retained module APIs become internal declaration-associated primitives; module service APIs remain ordinary functions and gain no receiver form. `print`, `read_line`, constructors and the deliberately retained trapping adapters require explicit declarations/contracts, not generic receiver fallback.

## Every built-in spelling

| Current spelling | Destination / retained purpose | Delivery | Removal condition |
| --- | --- | --- | --- |
| `$method_add` | Set[T].add | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `$method_call` | callable family.call | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `$method_get` | Map[K,V].get | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `$method_has` | Map[K,V].has / Set[T].has | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `$method_map` | Array[T].map | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `$method_put` | Map[K,V].put | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `$method_reduce` | Array[T].reduce | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `$unit` | Unit literal/constructor lowering | P05/P06 | Retain explicit construction; remove receiver-sugar eligibility in P12 |
| `append` | Array[T].append | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `async_fake_read` | test-only async fixture primitive | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `byte_append` | Bytes.push | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `byte_get` | Bytes.at | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `byte_len` | Bytes.length | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `bytes` | Bytes constructor | P05/P06 | Retain explicit construction; remove receiver-sugar eligibility in P12 |
| `bytes_concat` | Bytes.join | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `command_args` | host.arguments | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `concat` | Array[T].concat | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `dec` | Rat.dec | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `duration_from_seconds` | time.from_seconds | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `duration_nanoseconds` | time.from_nanoseconds | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `duration_ticks` | Duration.ticks | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `environment_get` | environment.get | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `environment_has` | environment lookup implementation | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `eprint` | explicit stderr service preserving existing formatting | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `file_exists` | compiler/filesystem_support.exists | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_create_directory` | filesystem.create_directory | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_directories_overlap` | package_filesystem.directories_overlap | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_list` | filesystem.list_directory | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_metadata` | filesystem.metadata | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_read` | filesystem.read | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_read_beneath` | package_filesystem.read_beneath | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_read_beneath_async` | async_filesystem.read_beneath | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_remove_directory` | filesystem.remove_directory | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_remove_file` | filesystem.remove_file | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_root_identity_async` | async_filesystem.root_identity | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_temp_directory` | filesystem.temp_directory | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_temp_file` | filesystem.temp_file | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `fs_write` | filesystem.write | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `host_arguments_async` | async_host.arguments | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `host_decode_utf8` | host.decode_utf8 | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `host_exit_async` | async_host.exit | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `host_sleep` | host.sleep | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `host_stderr_async` | async_host.stderr | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `host_stdout_async` | async_host.stdout | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `http_transport_exchange` | HTTP transport module internal primitive | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `instant_add` | Instant.add | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `instant_before` | Instant.is_before | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `instant_difference` | Instant.difference | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `instant_now` | time.now | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `is_digit` | Str.is_digit | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `is_letter` | Str.is_letter | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `is_whitespace` | Str.is_whitespace | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `len` | Str.len / Array[T].len / Bytes.length | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `map` | Map constructor | P05/P06 | Retain explicit construction; remove receiver-sugar eligibility in P12 |
| `map_get` | Map[K,V].get | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `map_has` | Map[K,V].has | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `map_put` | Map[K,V].put | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `nat` | Rat.nat | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `nat_from_str` | Str.parse_nat | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `path_absolute` | Path.is_absolute | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `path_append` | Path.join | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `path_current` | path.current | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `path_directory` | Path.directory | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `path_display` | Path.display | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `path_filename` | Path.filename | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `path_from_native` | path.from_native | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `path_from_text` | path.from_text | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `path_is_absolute` | compiler/path_support.is_absolute | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `path_join` | compiler/path_support.join | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `path_native_bytes` | Path.native_bytes | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `path_parent` | compiler/path_support.parent | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `path_resolve` | compiler/path_support.resolve | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `path_suffix` | compiler/path_support.suffix | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `path_to_text` | Path.to_text | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `path_with_suffix` | compiler/path_support.with_suffix | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `print` | explicit core printing service; preserve generic display semantics | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `process_exit` | host.exit | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `process_run` | process.run | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `quotient` | Nat.divide | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `read_bytes` | compiler/filesystem_support.read_file_bytes | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `read_file` | compiler/filesystem_support.read_text | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `read_line` | explicit core input service | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `reverse` | Str.reverse | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `run_bytecode` | host.run/internal execution service | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `run_bytecode_args` | host.run | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `set` | Set constructor | P05/P06 | Retain explicit construction; remove receiver-sugar eligibility in P12 |
| `set_add` | Set[T].add | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `set_has` | Set[T].has | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `slice` | Str.slice | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `starts_with` | Str.starts_with | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `starts_with_at` | Str.starts_with_at | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `tcp_exchange` | HTTP transport module internal primitive | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `tcp_serve` | HTTP server module internal primitive | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `tcp_serve_stream` | HTTP service module internal primitive | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `tcp_server_read_chunk` | HTTP service module internal primitive | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `tcp_service` | tcp_service.serve | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `utf8_decode` | Bytes.decode_utf8 | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `utf8_encode` | Str.encode_utf8 | P06; P07 for callable/map/reduce | P12: all callers migrated; delete global alias and spelling-based dispatch |
| `write_bytes` | compiler/filesystem_support.write_file_bytes | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |
| `write_file` | explicit trapping text-write service (existing contract; do not replace with checked Bytes API) | P10/P11 | P12: bind retained service or internal primitive by declaration identity; delete obsolete public low-level alias |

## Special core source functions

| Existing source / lowered identity | Owned declaration | Required action |
| --- | --- | --- |
| `core_ends_with` / `$core_ends_with` | Str.ends_with | Move body in P08; delete parser/name/emission remapping in P12 |
| `core_first` / `$core_first` | Array[T].first | Move generic body in P08; preserve Option result |
| `core_sort_by` / `$core_sort_by` | Array[T].sort_by | Move generic body in P08; preserve stable sort and PureFn comparator |

## Source declarations and ordinary functions

The 82 stdlib declarations were re-extracted at this baseline; the existing table matches. Every owned row has its target name and existing signature there. Receiver calls inside retained service functions still migrate. Preserve module namespace APIs and their imports/re-exports; retiring a source wrapper must also update direct namespace callers and named references in the ledger.

All other declared source functions remain explicit functions unless the ledger identifies a receiver role. A legacy receiver call targeting a free function on a foreign/built-in owner must become an explicit function call, or an intentional rejection test if it specifically tests sugar. It cannot justify a foreign impl or global fallback. Existing `$owned$` declarations preserve their owner when P02 replaces the marker representation. Constructors and enum variants retain their separate declaration kind.

Compiler-only Str path adapters remain explicit module functions. They perform host/path-domain operations with existing trapping behaviour; converting them to Str methods would violate defining-module ownership and confuse ordinary text with path semantics. Their low-level backend moves to internal declaration-associated primitives; no new universal Str filesystem methods are introduced.

Named function references require special attention: when an API becomes a method, an existing `@api` reference must use a named adapter with the same signature/effect, or a retained ordinary service function. Bound-method references and closures remain out of scope.

## Compatibility decisions to carry into implementation

- Namespace functions named `get`, including `environment.get`, remain valid. This is separate from Server.get and Map.get.
- Map.get returns the existing value type and retains its current missing-key failure contract. Do not turn it into Option to simplify generic method implementation.
- `numeric.divide` and the `quotient` intrinsic converge on Nat.divide. Preserve the exact quotient contract; do not create two competing methods on Nat.
- `text.reversed` converges on Str.reverse, while byte spellings converge on push/join/length/at as the plan specifies.
- `path_is_absolute(Str)` is a compiler adapter operation; `path_absolute(Path)` becomes Path.is_absolute. Their similar spellings do not imply interchangeable signatures.
- `host.decode_utf8` is checked and stays a module service. Bytes.decode_utf8 retains the existing core decoding behaviour.
- PathError, DurationError and ClockError identity changes must update both body_intrinsic_type and the explicitly constructed PathError identity in body_check_path_core_call, not only source imports. HostError and filesystem/process types remain in their defining modules because their APIs remain module services.
- The implicit core surface currently reserves Option/Result and their variants. Moving additional error declarations into core must not accidentally make all their names implicit user globals. Preserve module re-export visibility.

## Receiver signatures and nontrivial result rules

These are the existing core contracts with the receiver removed, not additional APIs. Methods are pure except callable invocation whose effect comes from its callable owner, and service operations explicitly retained above. Exact guard/bounds checks remain in the selected declaration-associated implementation.

| Owner | Method contract |
| --- | --- |
| Str | len(): Nat; slice(start: Nat, end: Nat): Str; starts_with(prefix: Str): Bool; starts_with_at(prefix: Str, offset: Nat): Bool; reverse(): Str; ends_with(suffix: Str): Bool |
| Str | is_digit(): Bool; is_letter(): Bool; is_whitespace(): Bool; parse_nat(): Nat; encode_utf8(): Bytes |
| Array[T] | len(): Nat; append(value: T): [T]; first(): Option[T]; sort_by(less: PureFn[T,T,Bool]): [T] |
| Array[T] | concat(other: [U]) computes the existing element-type join of T and U; it is not simply concat([T]): [T] |
| Array[T] | map[U](callback: PureFn[T,U]): [U]; reduce[A](initial: A, callback: PureFn[A,T,A]): A |
| Map[K,V] | put(key: K, value: V): Map[K,V]; get(key: K): V; has(key: K): Bool |
| Set[T] | add(value: T): Set[T]; has(value: T): Bool |
| Bytes | push(value: Nat): Bytes; join(other: Bytes): Bytes; length(): Nat; at(index: Nat): Nat; decode_utf8(): Str |
| Nat | divide(divisor: Nat): Nat |
| Rat | nat(): Nat; dec(): Dec |

Array.concat uses body_join in the current checked pipeline. Preserve that result solver on the selected intrinsic declaration in P06. Do not add source-level Join types, permit arbitrary overloads or narrow the method to identical element types. Retain the current negative tests for non-joinable inputs and add a positive mixed Nat/Int case to the owned-call suite. Empty Array/Map/Set types retain their known family even when type arguments are inference holes; select by family and defer argument constraints, rather than treating the entire owner as unknown.

The previous handoff table incorrectly included array slice and text concat and omitted Bytes from len. P01 corrects that table to the checked compiler's existing domains. These are plan corrections, not language extensions.

## Explicit generic-call migration

Owner binders no longer appear in the method's explicit type-argument list. Three current source call sites need receiver annotations instead of copying their existing type arguments onto the new method:

- `examples/generic_functions.panack:18`: `[].first[Str]()` becomes a `[Str]`-typed empty binding followed by `->first()`.
- `tests/functional/cases/core_methods/main.panack:20`: the same empty-array first case.
- `tests/runner/stdlib_unit.panack:34`: `[].sort_by[Nat](@nat_less)` becomes a `[Nat]`-typed empty receiver followed by `->sort_by(@nat_less)`.

The embedded checker example at `tests/runner/compiler_checker_unit.panack:108` moves with that contract. The `map[Nat]` strings in source-span tests are parser fixtures and need their expected spans updated when arrow/dot syntax changes; they do not prove current intrinsic map accepts those explicit type arguments. Keep the distinction between owner binders and genuine method binders in P04/P07 tests.
