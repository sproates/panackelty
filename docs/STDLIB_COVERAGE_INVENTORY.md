# Stdlib coverage inventory

28 November 2026. SCV1 is in progress. This inventory accounts for all 69
tracked stdlib source modules, but does not yet establish per-operation test
completeness. The [machine-readable inventory](STDLIB_COVERAGE_INVENTORY.json)
lists public function declarations, imports and candidate direct test consumers.
References are discovery evidence, not proof that a test runs or asserts the
relevant contract. Re-export and native method contracts still need review.

## Measurement investigation

The published baseline contains 138 unavailable stdlib lines:

| Module | Lines | Evidence |
| --- | ---: | --- |
| core.panack | 97 | Inventory function names use `$owned$…`; emitted core methods use `$core_$owned$…`. The current association helper uses the general symbol rule instead of the callable rule. A focused correction recovered 44 lines in the existing 42-assertion stdlib unit suite; unused methods can still remain un-emitted. |
| testing_commands.panack | 29 | Retained plans have function associations but mode 5 for one or more expressions on each affected line: no unique source probe. These are not proof of absent tests. |
| filesystem.panack | 6 | Same mode-5 association limitation. |
| process.panack | 5 | Same mode-5 association limitation. |
| time.panack | 1 | Same mode-5 association limitation. |

A diagnostic exporter found 66 mode-5 items in the scope fixture, all with zero
matching probes and all receiver items. Their source text is an imported namespace
qualifier (for example `errors`, `std_testing` or `std_process`), which is resolved
at compile time. These include the remaining 41 unavailable baseline lines.
The syntax inventory currently visits namespace receivers as expressions. Any
correction must distinguish these from actual runtime receivers, including local
shadowing; dropping every receiver or guessing execution would be wrong.
The core-method correction now uses the existing callable-symbol rule shared by
emission, rather than duplicating `$core_` naming in the exporter. A development
compiler passes the two-path CLI/exporter parity regression (732 items per path),
including one exact execution of `Str.ends_with` each time. Plan-cache integration
also passes with four distinct fresh sessions. Full validation and integration
remain outstanding; the released bootstrap seed has not been changed.

## Module accounting

“Direct consumers” counts source references, including fixtures; zero does not
mean untested because imports can be transitive and core is loaded implicitly.

| Module | Current measurement | Direct consumers | Declared public functions |
| --- | --- | ---: | ---: |
| [async_filesystem](../src/stdlib/async_filesystem.panack) | unmeasured | 0 | 2 |
| [async_host](../src/stdlib/async_host.panack) | unmeasured | 4 | 4 |
| [bounded_filesystem](../src/stdlib/bounded_filesystem.panack) | unmeasured | 2 | 3 |
| [bytes](../src/stdlib/bytes.panack) | eligible | 53 | 2 |
| [compiler_files](../src/stdlib/compiler_files.panack) | eligible | 8 | 4 |
| [compiler_paths](../src/stdlib/compiler_paths.panack) | eligible | 3 | 6 |
| [compression/gzip](../src/stdlib/compression/gzip.panack) | unmeasured | 1 | 3 |
| [compression/gzip/async](../src/stdlib/compression/gzip/async.panack) | unmeasured | 0 | 3 |
| [compression/gzip/errors](../src/stdlib/compression/gzip/errors.panack) | unmeasured | 2 | 2 |
| [console](../src/stdlib/console.panack) | unmeasured | 2 | 5 |
| [console/async](../src/stdlib/console/async.panack) | unmeasured | 2 | 5 |
| [console/errors](../src/stdlib/console/errors.panack) | unmeasured | 1 | 2 |
| [core](../src/stdlib/core.panack) | eligible | 6 | 27 |
| [digest](../src/stdlib/digest.panack) | unmeasured | 0 | 4 |
| [digest/sha256](../src/stdlib/digest/sha256.panack) | unmeasured | 1 | 5 |
| [digest/sha256/async](../src/stdlib/digest/sha256/async.panack) | unmeasured | 0 | 4 |
| [digest/sha256/errors](../src/stdlib/digest/sha256/errors.panack) | unmeasured | 2 | 2 |
| [encoding/utf8](../src/stdlib/encoding/utf8.panack) | unmeasured | 1 | 2 |
| [environment](../src/stdlib/environment.panack) | eligible | 25 | 1 |
| [environment/async](../src/stdlib/environment/async.panack) | unmeasured | 2 | 1 |
| [filesystem](../src/stdlib/filesystem.panack) | eligible | 7 | 14 |
| [filesystem/async](../src/stdlib/filesystem/async.panack) | unmeasured | 1 | 14 |
| [filesystem/confined](../src/stdlib/filesystem/confined.panack) | unmeasured | 1 | 3 |
| [filesystem/confined/async](../src/stdlib/filesystem/confined/async.panack) | unmeasured | 1 | 3 |
| [filesystem/errors](../src/stdlib/filesystem/errors.panack) | unmeasured | 1 | 2 |
| [filesystem/legacy](../src/stdlib/filesystem/legacy.panack) | unmeasured | 34 | 14 |
| [filesystem/legacy/async](../src/stdlib/filesystem/legacy/async.panack) | unmeasured | 4 | 14 |
| [filesystem/staging](../src/stdlib/filesystem/staging.panack) | unmeasured | 2 | 7 |
| [filesystem/staging/async](../src/stdlib/filesystem/staging/async.panack) | unmeasured | 1 | 7 |
| [filesystem_stage](../src/stdlib/filesystem_stage.panack) | unmeasured | 3 | 7 |
| [gzip](../src/stdlib/gzip.panack) | unmeasured | 0 | 3 |
| [host](../src/stdlib/host.panack) | eligible | 82 | 7 |
| [host/async](../src/stdlib/host/async.panack) | unmeasured | 5 | 0 |
| [http_runtime](../src/stdlib/http_runtime.panack) | unmeasured | 3 | 25 |
| [http_stream](../src/stdlib/http_stream.panack) | unmeasured | 2 | 3 |
| [http_transport](../src/stdlib/http_transport.panack) | eligible | 2 | 1 |
| [network/http_stream](../src/stdlib/network/http_stream.panack) | unmeasured | 1 | 3 |
| [network/http_stream/async](../src/stdlib/network/http_stream/async.panack) | unmeasured | 1 | 3 |
| [network/http_stream/errors](../src/stdlib/network/http_stream/errors.panack) | unmeasured | 2 | 2 |
| [network/tcp/async](../src/stdlib/network/tcp/async.panack) | unmeasured | 1 | 5 |
| [network/tcp/errors](../src/stdlib/network/tcp/errors.panack) | unmeasured | 1 | 4 |
| [package_filesystem](../src/stdlib/package_filesystem.panack) | unmeasured | 5 | 2 |
| [package_filesystem/async](../src/stdlib/package_filesystem/async.panack) | unmeasured | 0 | 1 |
| [path](../src/stdlib/path.panack) | eligible | 53 | 3 |
| [prelude](../src/stdlib/prelude.panack) | eligible | 4 | 0 |
| [process](../src/stdlib/process.panack) | eligible | 4 | 3 |
| [process/async](../src/stdlib/process/async.panack) | unmeasured | 3 | 1 |
| [process/errors](../src/stdlib/process/errors.panack) | unmeasured | 1 | 2 |
| [process/legacy](../src/stdlib/process/legacy.panack) | unmeasured | 26 | 2 |
| [process/types](../src/stdlib/process/types.panack) | unmeasured | 0 | 1 |
| [runtime/bytecode](../src/stdlib/runtime/bytecode.panack) | unmeasured | 1 | 0 |
| [state](../src/stdlib/state.panack) | unmeasured | 14 | 3 |
| [state/async](../src/stdlib/state/async.panack) | unmeasured | 3 | 3 |
| [tcp](../src/stdlib/tcp.panack) | eligible | 4 | 4 |
| [tcp_service](../src/stdlib/tcp_service.panack) | unmeasured | 0 | 1 |
| [testing](../src/stdlib/testing.panack) | eligible | 64 | 4 |
| [testing/filesystem](../src/stdlib/testing/filesystem.panack) | unmeasured | 0 | 2 |
| [testing/filesystem/async](../src/stdlib/testing/filesystem/async.panack) | unmeasured | 1 | 3 |
| [testing/process](../src/stdlib/testing/process.panack) | unmeasured | 0 | 1 |
| [testing/process/async](../src/stdlib/testing/process/async.panack) | unmeasured | 1 | 3 |
| [testing/reporting](../src/stdlib/testing/reporting.panack) | unmeasured | 0 | 1 |
| [testing/reporting/async](../src/stdlib/testing/reporting/async.panack) | unmeasured | 1 | 1 |
| [testing_async](../src/stdlib/testing_async.panack) | eligible | 5 | 1 |
| [testing_commands](../src/stdlib/testing_commands.panack) | eligible | 23 | 6 |
| [testing_files](../src/stdlib/testing_files.panack) | eligible | 28 | 3 |
| [time](../src/stdlib/time.panack) | eligible | 42 | 8 |
| [time/async](../src/stdlib/time/async.panack) | unmeasured | 2 | 2 |
| [time/errors](../src/stdlib/time/errors.panack) | unmeasured | 1 | 2 |
| [validation](../src/stdlib/validation.panack) | eligible | 1 | 1 |

## Next verification

Trace the listed consumers to their registered runners and assertions; attach
success, error, boundary and lifecycle cases per operation. Inspect lowered
probe associations for the remaining 41 unavailable lines. Reuse existing
fixtures in focused collection before introducing duplicate programs. No
percentage threshold is a task acceptance criterion.
