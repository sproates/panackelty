# Standard-library declaration inventory

[P01 inventory and compatibility audit](p01-inventory.md) records the checked source baseline, complete call-site register and corrections to the proposed API mapping.

Inspected baseline: `7aa31bcf99cdd9e4946718d13063f787e78662ce`. This is a source-extracted inventory of top-level function declarations, with proposed migration classifications. It is not a semantic call-site inventory and excludes compiler-provided built-ins, type declarations and re-exports. Packet P01 must add those and reconcile any baseline changes.

Owned entries lose their old exported receiver-function surface after callers migrate. Retained functions cannot be invoked using receiver sugar. All function bodies must still migrate their own receiver calls. Testing services remain module APIs; the operation on the locally defined TestWorkspace has a genuine owner.

| Source | Current signature | Proposed destination |
| --- | --- | --- |
| `src/stdlib/async_filesystem.panack:6` | `pub async read_beneath( root: Path, relative: Path, limit: Nat ): Result[FilesystemRootRead, HostError]` | Retain explicit module function |
| `src/stdlib/async_filesystem.panack:15` | `pub async root_identity(root: Path): Result[FilesystemRootIdentity, HostError]` | Retain explicit module function |
| `src/stdlib/async_host.panack:3` | `pub async arguments(): [Str]` | Retain explicit module function |
| `src/stdlib/async_host.panack:7` | `pub async stdout(message: Str): Result[Unit,HostError]` | Retain explicit module function |
| `src/stdlib/async_host.panack:11` | `pub async stderr(message: Str): Result[Unit,HostError]` | Retain explicit module function |
| `src/stdlib/async_host.panack:15` | `pub async exit(status: Nat): Unit` | Retain explicit module function |
| `src/stdlib/bytes.panack:1` | `pub pure empty(): Bytes` | Retain explicit module function |
| `src/stdlib/bytes.panack:5` | `pub pure push(buffer: Bytes, value: Nat): Bytes` | Owned: Bytes.push |
| `src/stdlib/bytes.panack:9` | `pub pure join(left: Bytes, right: Bytes): Bytes` | Owned: Bytes.join |
| `src/stdlib/bytes.panack:13` | `pub pure length(buffer: Bytes): Nat` | Owned: Bytes.length |
| `src/stdlib/bytes.panack:17` | `pub pure at(buffer: Bytes, index: Nat): Nat` | Owned: Bytes.at |
| `src/stdlib/core.panack:15` | `pure core_ends_with(value: Str, suffix: Str): Bool` | Owned: Str.ends_with |
| `src/stdlib/core.panack:29` | `pure core_first[T](values: [T]): Option[T]` | Owned: Array[T].first |
| `src/stdlib/core.panack:39` | `pure core_sort_by[T](values: [T], less: PureFn[T,T,Bool]): [T]` | Owned: Array[T].sort_by |
| `src/stdlib/environment.panack:1` | `pub get(name: Str): Option[Str]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:6` | `pub metadata(path: Path): Result[FileMetadata, HostError]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:10` | `pub read(path: Path, limit: Nat): Result[Bytes, HostError]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:14` | `pub write(path: Path, data: Bytes): Result[Unit, HostError]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:18` | `pub list_directory(path: Path): Result[[Path], HostError]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:22` | `pub remove_file(path: Path): Result[Unit, HostError]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:26` | `pub remove_directory(path: Path): Result[Unit, HostError]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:30` | `pub create_directory(path: Path): Result[Unit, HostError]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:34` | `pub temp_directory(parent: Path): Result[Path, HostError]` | Retain explicit module function |
| `src/stdlib/filesystem.panack:38` | `pub temp_file(parent: Path): Result[Path, HostError]` | Retain explicit module function |
| `src/stdlib/host.panack:4` | `pub run(data: Bytes, arguments: [Str]): Void` | Retain explicit module function |
| `src/stdlib/host.panack:8` | `pub arguments(): [Str]` | Retain explicit module function |
| `src/stdlib/host.panack:12` | `pub exit(status: Nat): Void` | Retain explicit module function |
| `src/stdlib/host.panack:16` | `pub stderr(message: Str): Void` | Retain explicit module function |
| `src/stdlib/host.panack:20` | `pub stdout(message: Str): Void` | Retain explicit module function |
| `src/stdlib/host.panack:24` | `pub sleep(duration: Duration): Result[Unit,HostError]` | Retain explicit module function |
| `src/stdlib/host.panack:28` | `pub decode_utf8(value: Bytes): Result[Str,HostError]` | Retain explicit module function |
| `src/stdlib/numeric.panack:2` | `pub pure divide(dividend: Nat, divisor: Nat): Nat` | Owned: Nat.divide |
| `src/stdlib/option.panack:1` | `pub pure value_or[T](value: Option[T], fallback: T): T` | Owned: Option[T].value_or |
| `src/stdlib/package_filesystem.panack:7` | `pub directories_overlap(left: Path, right: Path): Result[Bool, HostError]` | Retain explicit module function |
| `src/stdlib/package_filesystem.panack:11` | `pub read_beneath( root: Path, relative: Path, limit: Nat ): Result[FilesystemRootRead, HostError]` | Retain explicit module function |
| `src/stdlib/path.panack:4` | `pub pure from_text(value: Str): Result[Path, PathError]` | Retain explicit module function |
| `src/stdlib/path.panack:5` | `pub pure from_native(value: Bytes): Result[Path, PathError]` | Retain explicit module function |
| `src/stdlib/path.panack:6` | `pub pure join(base: Path, child: Path): Result[Path, PathError]` | Owned: Path.join |
| `src/stdlib/path.panack:7` | `pub pure to_text(value: Path): Result[Str, PathError]` | Owned: Path.to_text |
| `src/stdlib/path.panack:8` | `pub pure native_bytes(value: Path): Bytes` | Owned: Path.native_bytes |
| `src/stdlib/path.panack:9` | `pub pure display(value: Path): Str` | Owned: Path.display |
| `src/stdlib/path.panack:10` | `pub pure is_absolute(value: Path): Bool` | Owned: Path.is_absolute |
| `src/stdlib/path.panack:11` | `pub pure directory(value: Path): Path` | Owned: Path.directory |
| `src/stdlib/path.panack:12` | `pub pure filename(value: Path): Option[Path]` | Owned: Path.filename |
| `src/stdlib/path.panack:13` | `pub pure current(): Path` | Retain explicit module function |
| `src/stdlib/process.panack:6` | `pub run(executable: Path, arguments: [Str], stdin: Bytes, working_directory: Path, environment: [Str], timeout: Duration, output_limit: Nat): Result[ProcessOutput,HostError]` | Retain explicit module function |
| `src/stdlib/result.panack:1` | `pub pure value_or[T,E](value: Result[T,E], fallback: T): T` | Owned: Result[T,E].value_or |
| `src/stdlib/tcp_service.panack:15` | `pub async serve[T]( address: Str, port: Nat, worker: AsyncFn[T,Result[Bytes,Str]], context: T, limits: TcpServiceLimits, ready: AsyncFn[TcpServiceReady,Result[Unit,Str]] ): Result[TcpServiceSummary,Str]` | Retain explicit module function |
| `src/stdlib/testing.panack:5` | `pub pure expect(name: Str, condition: Bool, reason: Str): TestResult` | Retain explicit module function |
| `src/stdlib/testing.panack:13` | `pub pure equal_str(name: Str, actual: Str, expected: Str): TestResult` | Retain explicit module function |
| `src/stdlib/testing.panack:17` | `pub pure equal_nat(name: Str, actual: Nat, expected: Nat): TestResult` | Retain explicit module function |
| `src/stdlib/testing.panack:22` | `pub report(results: [TestResult]): Nat` | Retain explicit module function |
| `src/stdlib/testing_commands.panack:27` | `pub pure command_output(name: Str, actual: ProcessOutput, expected: ProcessOutput): TestResult` | Retain explicit module function |
| `src/stdlib/testing_commands.panack:52` | `pub pure command_result(name: Str, actual: Result[ProcessOutput,HostError], expected: ProcessOutput): TestResult` | Retain explicit module function |
| `src/stdlib/testing_commands.panack:63` | `pure harness_path(text: Str): Path` | Retain explicit private helper |
| `src/stdlib/testing_commands.panack:67` | `harness_write(destination: Str, contents: Bytes): Bool` | Retain explicit private helper |
| `src/stdlib/testing_commands.panack:79` | `pub harness_command(): Void` | Retain explicit module function |
| `src/stdlib/testing_commands.panack:115` | `pub capture_report(name: Str, command: TestCommand, expected: ProcessOutput, destination: Option[Path]): TestResult` | Retain explicit module function |
| `src/stdlib/testing_commands.panack:143` | `pub run_command(name: Str, command: TestCommand, expected: ProcessOutput): TestResult` | Retain explicit module function |
| `src/stdlib/testing_commands.panack:150` | `pub expect_command_error(name: Str, command: TestCommand, expected_code: Str): TestResult` | Retain explicit module function |
| `src/stdlib/testing_files.panack:7` | `pub workspace_create(parent: Path): Result[TestWorkspace,HostError]` | Retain explicit module function |
| `src/stdlib/testing_files.panack:15` | `pub workspace_remove_empty(workspace: TestWorkspace): Result[Unit,HostError]` | Owned: TestWorkspace.remove_empty |
| `src/stdlib/testing_files.panack:22` | `pub discover_fixtures(root: Path): Result[[Path],HostError]` | Retain explicit module function |
| `src/stdlib/text.panack:1` | `pub pure reversed(value: Str): Str` | Owned: Str.reverse |
| `src/stdlib/text.panack:5` | `pub pure encode_utf8(value: Str): Bytes` | Owned: Str.encode_utf8 |
| `src/stdlib/text.panack:9` | `pub pure decode_utf8(value: Bytes): Str` | Owned: Bytes.decode_utf8 |
| `src/stdlib/time.panack:4` | `pub pure ticks(value: Duration): Int` | Owned: Duration.ticks |
| `src/stdlib/time.panack:5` | `pub pure from_nanoseconds(value: Int): Duration` | Retain explicit module function |
| `src/stdlib/time.panack:6` | `pub pure from_seconds(value: Rat): Result[Duration,DurationError]` | Retain explicit module function |
| `src/stdlib/time.panack:9` | `pub now(): Result[Instant,ClockError]` | Retain explicit module function |
| `src/stdlib/time.panack:10` | `pub pure add_instant(value: Instant, delta: Duration): Instant` | Owned: Instant.add |
| `src/stdlib/time.panack:13` | `pub pure is_before_instant(left: Instant, right: Instant): Bool` | Owned: Instant.is_before |
| `src/stdlib/time.panack:16` | `pub pure difference(left: Instant, right: Instant): Duration` | Owned: Instant.difference |
| `src/stdlib/time.panack:20` | `pub pure seconds(value: Int): Duration` | Retain explicit module function |
| `src/stdlib/time.panack:23` | `pub pure milliseconds(value: Int): Duration` | Retain explicit module function |
| `src/stdlib/time.panack:26` | `pub pure add(left: Duration, right: Duration): Duration` | Owned: Duration.add |
| `src/stdlib/time.panack:29` | `pub pure subtract(left: Duration, right: Duration): Duration` | Owned: Duration.subtract |
| `src/stdlib/time.panack:32` | `pub pure scale(value: Duration, factor: Int): Duration` | Owned: Duration.scale |
| `src/stdlib/time.panack:35` | `pub pure is_before(left: Duration, right: Duration): Bool` | Owned: Duration.is_before |
| `src/stdlib/time.panack:38` | `pub pure as_seconds(value: Duration): Rat` | Owned: Duration.as_seconds |
| `src/stdlib/time.panack:41` | `pub pure divide(value: Duration, divisor: Int): Result[Duration,DurationError]` | Owned: Duration.divide |
| `src/stdlib/time.panack:45` | `pub pure ratio(left: Duration, right: Duration): Result[Rat,DurationError]` | Owned: Duration.ratio |

Inventory contains 82 declarations.

## Required call-site register in P01

For each declaration or compiler-provided receiver operation, record: original identity; old signatures and spellings; replacement owner/name/signature/effect; canonical defining module; retained-global rationale if applicable; all direct, receiver and function-reference callers; migration packet; wrapper removal packet; test evidence; completion state.

Obtain receiver candidates from the existing parsed/resolved DotCallExpr path, separating NamespaceCall from ValueMethodCall. Use a temporary read-only inventory probe if needed; do not infer ownership from the first parameter or regex alone. Count generated test source strings and public examples as consumers too.

P01 is complete only when every receiver call has a classified target, every built-in entry has a disposition, and conflicts have a recorded decision. The table above does not satisfy that gate by itself.
