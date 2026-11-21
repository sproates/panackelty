# Standard-library domain errors

21 November 2026. RM#162: Real-program fixes. Development branch contract;
not a claim about the released alpha.15 library.

## Shared rules

Ordinary and async operations return the same nominal error type. Domain modules
re-export their error type and kind; the corresponding errors module owns the
record, enum, pure conversion and render method. Callers can inspect kind without
parsing a message, and can use render() for a complete human-readable explanation.
The native field retains the original HostError unchanged: operation, code and
native_code. Unknown codes map to OtherFailure; they are not silently relabelled
as a specific known cause. Native numeric errors remain available, not guessed
from platform-specific errno values. Render appends a nonzero host error number.

This batch implements console, sleep and process errors in Panack. Native ABI,
worker ownership, limits, cancellation and successful result values do not change.
Filesystem, digest/gzip and network errors remain separate acceptance work.
Network error delivery must preserve detailed TLS causes, not only rename Str.

## Console

console and console/async return ConsoleError from all five operations. Read
returns Result[Option[Bytes],ConsoleError]; writes return Result[Unit,ConsoleError].
ConsoleOperation is Read(Nat), Write, WriteLine, WriteError or WriteErrorLine.
ConsoleError stores operation, kind and native. Read carries the requested byte
limit. Output errors do not retain or print the message that failed to write.

| Native code | ConsoleErrorKind | Explanation |
| --- | --- | --- |
| out_of_range on read | InvalidReadLimit | Require 1 through 65,536 bytes |
| out_of_range on write | OutputTooLarge | Message exceeds 16,384 UTF-8 bytes before the optional newline |
| busy | ReadBusy | Another stdin read still owns the single reservation |
| io_error | IOFailure | Host could not complete I/O; native detail is retained |
| resource_limit | ResourceLimit | Worker admission/allocation/startup failed |
| unsupported_host | UnsupportedHost | Host lacks the operation |
| Any other code | OtherFailure | Unknown failure, preserved without speculation |

For console.read(0), render returns:

> Cannot read standard input (requested limit: 0 bytes): the read limit must be from 1 through 65536 bytes. Choose a limit within that range.

For an oversized standard-output message:

> Cannot write standard output: the message exceeds 16384 UTF-8 bytes before the optional newline. Write smaller chunks.

## Sleep

Both time.sleep and time/async.sleep return Result[Unit,SleepError]. SleepError
stores duration, kind and native. Clock reads retain their existing ClockError;
a failed sleep is not a failed clock read by definition.

SleepErrorKind is NegativeDuration, DurationTooLarge, TimerFailure,
UnsupportedHost or OtherFailure. negative_duration and out_of_range map to the
first two. io_error or clock_unavailable maps to TimerFailure, which does not
claim whether the failure occurred while reading a clock or waiting. The host's
365-day maximum remains; zero succeeds. Timer limits can also be constrained by
the host deadline representation.

For a negative duration:

> Cannot sleep for -1 nanoseconds: the duration is negative. Use zero or a positive duration.

## Processes

Both process.run and process/async.run return Result[ProcessOutput,ProcessError].
ProcessError retains executable, working_directory, timeout, output_limit, kind
and native. It does not retain arguments, environment values, stdin or partial
output. A normal nonzero exit or signal remains a successful ProcessOutput.

| Native failure | ProcessErrorKind |
| --- | --- |
| negative_duration; zero timeout; out_of_range with timeout above 365 days | InvalidTimeout |
| Other out_of_range | LimitExceeded |
| invalid_argument | InvalidArgument |
| invalid_environment | InvalidEnvironment |
| launch_failed | LaunchFailed |
| timeout with a positive timeout | TimedOut |
| output_limit | OutputLimitExceeded |
| clock_unavailable | ClockUnavailable |
| resource_limit or out_of_memory | ResourceLimit |
| cancelled | Cancelled |
| Known host I/O/path/permission failures | HostFailure |
| unsupported_host | UnsupportedHost |
| Other code | OtherFailure |

For a zero timeout:

> Cannot run process '/bin/sh' with working directory '/': the timeout must be positive and within the supported range. Choose a timeout greater than zero and no longer than 365 days.

For a 10 ms timeout that actually elapsed:

> Cannot run process '/bin/sh' with working directory '/': the process did not complete within the requested 10000000 nanoseconds. The process group was stopped and partial output is not returned.

A launch failure identifies both executable and working directory. The native
adapter cannot always distinguish which failed, so render suggests checking both
and their permissions. It does not assert that the executable is missing.
Limits that the native adapter groups together remain LimitExceeded; the wrapper
does not invent a more precise cause. Cancellation-before-launch may produce no
process, so Cancelled does not claim a child was started or killed.

## Migration and acceptance

The new canonical alpha.16 APIs now use their domain errors. Replace annotations
using console.HostError, time.HostError for sleep, or process.HostError for the
Command API with ConsoleError, SleepError or ProcessError. Replace direct native
code/operation/native_code access with error.native.code and the corresponding
native fields. Prefer error.kind for application decisions. A deliberately
compatible adapter can return error.native without losing any host detail.

Released host.sleep, legacy host output and process/legacy positional APIs retain
their existing error contracts. testing/reporting returns Result[Nat,ConsoleError].
The testing/process compatibility helper explicitly unwraps ProcessError.native
because its historical command_result/expect_command_error contract uses HostError.
No existing error identity is recreated under a new import spelling.

Focused tests exercise real console bounds, invalid and elapsed timers, process
launch/environment/output-limit failures, nonzero successful exits and unknown
native-detail preservation. They check exact representative rendering and shared
types in ordinary/async source and saved bytecode. Final local make check passed in 621 seconds (unit 385s, bootstrap 82s, functional
167s), including packaging and installed smoke. A relocated installation with
spaces in its path passes the complete error fixtures and unrelated-directory
source, saved-bytecode and disassembly checks. Ordinary and sanitizer artifact
checks pass. The first full run caught three omitted installation-inventory entries;
those entries were corrected and the complete packaged source set was checked
before the passing run. Integration remains pending. No compiler or VM source change is
required; a compiler candidate is byte-for-byte identical to the saved seed.
