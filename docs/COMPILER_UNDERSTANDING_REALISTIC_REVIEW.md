# Compiler explanations: realistic usability review

**Design mock-up, not implemented output.** This is the first realistic review
case for GI#180: Compiler understanding. The short examples established notation
and wording; they did not establish usability. None of the command extensions
below are approved or implemented. The recorded source inventory remains the
completeness boundary.

## Subsequent interaction decision

The owner rejected line-coordinate navigation as the human workflow and selected
an interactive terminal prototype. The commands below remain historical review
candidates, not an approved implementation plan. Their evidence and wording
remain useful for evaluating direct selection, expansion and following origins.
The local prototype uses Panack for UI/navigation and fixture evidence, plus a
small native terminal-event adapter. That fixture version remains available. A separate
live demo now uses retained compiler evidence and final preparation diagnostics,
with reload and optional sharing of one terminal session to a phone. The live
demo is not a shipped command or evidence-completeness claim. Physical-phone
rehearsal and usability acceptance remain pending.

## The real function

`hash_archive_file` in `src/packages/acquisition.panack`, lines 68–93, calculates
an archive checksum. It validates a path, reads chunks, tracks a size limit,
updates hashing state, and returns either a checksum or a package problem.
It is ordinary package-tool code, not a fixture written to make Explain look good.

The source below is copied verbatim. Its long lines and nested calls are part of
the review. The line numbers in every proposed explanation are the original
repository line numbers, not numbers relative to this excerpt.

```panackelty
pub hash_archive_file(name: Str): Result[Str, PackageProblem] {
  match path.from_text(name) {
    Error(problem) => Error(problem_label(package_problem("PKG_FILESYSTEM", "Cannot read the archive because its path cannot be represented on this system."), "Source archive", name)),
    Ok(file) => {
      mut hash: Sha256 = sha256_new()
      mut offset: Nat = 0
      mut done: Bool = false
      mut failure: Option[PackageProblem] = None()
      while !done && failure == None() {
        match bounded.read_chunk(file, offset, 65536) {
          Error(problem) => { failure = Some(package_problem("PKG_FILESYSTEM", "Cannot read the source archive: " + host_detail(problem.code))) },
          Ok(bytes) => {
            offset = offset + bytes.length()
            if offset > 67108864 { failure = Some(package_problem("PKG_ARCHIVE_COMPRESSED_LIMIT", "Cannot read the source archive because the archive exceeds Panack's 64 MiB compressed-size limit.")) }
            else { hash = sha256_feed(hash, bytes) }
            done = bytes.length() == 0
          }
        }
      }
      match failure {
        Some(problem) => { sha256_close(hash); Error(problem) },
        None() => package_result(sha256_finish(hash), "PKG_TOOL")
      }
    }
  }
}
```

## What was actually observed

At source revision `aa9ef94a08322d99bf242487b987471a0ac355c7`, the existing
checkout launcher and checked-in compiler artifact accepted the selected
function/project. The command printed **1,025 lines / 43,849 UTF-8 bytes** to
stdout and exited 0. It took 7.61 seconds in one local run; this is an observation,
not a benchmark or a target. No compiler rebuild was performed.

Compiler artifact SHA-256:
`adc4f7c4d9e3b84c3030bcdb704c13d58f2055a2ec22a48c6351763aa06985d6`.
The executable disassembly was also captured. This does not establish that an
unbuilt compiler source revision has exactly the same behavior as that artifact.

Reproduction from the repository root:

```sh
PANACKELTY_BOOTSTRAP_ROOT="$PWD" bin/panack explain src/packages/main.panack --function project/src/packages/acquisition.panack:hash_archive_file
```

This repository's source imports require its established bootstrap-root setting.
Without it, the existing command treats the entry directory as the root and
fails to resolve `project/src/...` imports. The initial failed invocation was
corrected before collecting the successful baseline. This setting is specific
to the compiler repository's development layout; do not add it to ordinary
user-project instructions or silently propose it as a new public interface.

The baseline contains 119 retained decision/boundary entries, with repeated
whole-block excerpts and empty-family messages. It even describes some accepted
joins using an “unavailable” input type. That is evidence of missing inference
history/presentation, not permission to invent a reason in the renderer.

A separate copied source tree was changed only at line 83:

```diff
-            done = bytes.length() == 0
+            done = bytes.length()
```

The existing compiler rejected it with `assignment done is Nat, expected Bool`
at line 83, column 20, exit 1. The original source tree was not modified.

## The proposed experience

A complete **overview**, a **source-focused explanation**, and **all details**
serve different questions. The overview must say that it is an overview and show
how to obtain the complete evidence. It cannot silently become the complete
explanation while hiding accepted decisions.

- Keep `explain SOURCE --function NAME` as the entry point; propose an overview
  as its default output.
- Propose `--at LINE COLUMN` to select the smallest checked construct containing
  that position and show its decision plus the facts it depends on. It is a
  source query, not an instruction number. Show the selected expression and
  location so the user can verify the selection. No nearest-neighbour guessing.
- Propose `--details` for every retained decision in the selected function,
  grouped by source. No elision, arbitrary item cap or silent suppression.
  It remains long; the full candidate is printed below, not hidden behind a
  screenshot or collapsed preview. Relevant generic holes/solutions must be
  explained from retained facts added by the implementation, not guessed from
  final types.
- Keep the existing `--type-at` contract until deliberately changed; these
  proposed options do not silently reinterpret that existing switch.
- For compilation queries, keep the source-position `locate` proposal, but lead
  with source and transformations. Propose `--instructions` only when actual
  instruction detail is wanted, with source correspondence displayed alongside.
  Instruction indices are secondary identifiers for exact queries, not the
  organising principle of the explanation.

Before using the command examples below, set the repository-root environment
in the current shell with `export PANACKELTY_BOOTSTRAP_ROOT="$PWD"`. The inline
assignment in the reproduction command applies only to that invocation. Each stdout block is the entire output for that query,
not a hand-selected excerpt. Empty stderr and exit 0 apply to the accepted
queries. The failure query includes stderr and exit status explicitly.

### Question 1: “Did this function pass, and where should I look?”

```sh
bin/panack explain src/packages/main.panack --function hash_archive_file
```

Proposed complete stdout:

```text
hash_archive_file: accepted
Source: src/packages/acquisition.panack, lines 68 through 93.
View: overview. The program was checked; no package operations were run.

Result: Result[Str, PackageProblem]
  Both outer match branches produce this type, matching the declared return type.

Checks completed
  The three matches cover their variants without duplicate arms.
  Four local initializers and five assignments match their declared types.
  The while and if conditions produce Bool.
  All 20 call sites satisfy their argument and effect requirements.

Source sections
  Lines 69–70: match path.from_text(name); Error arm.
  Lines 72–75: declarations of hash, offset, done and failure.
  Line 76: while !done && failure == None().
  Lines 77–78: match bounded.read_chunk(file, offset, 65536); Error arm.
  Lines 80–83: assignments and if offset > 67108864.
  Lines 87–90: match failure; Some and None arms.

For one expression, repeat the command with --at LINE COLUMN.
For every decision in this function, repeat it with --details.
```

This output says nothing about whether a particular archive can be read, whether
hashing will succeed, or whether every execution terminates. Those are not
established by the type/effect checks. Source-section labels help navigation;
they are not claimed as an inferred proof of the programmer's intent.

### Question 2: “Why is failure == None() allowed?”

```sh
bin/panack explain src/packages/main.panack --function hash_archive_file --at 76 22
```

Proposed complete stdout:

```text
failure == None() => Bool — accepted
Source: src/packages/acquisition.panack, line 76, column 22.
View: selected expression and its dependencies. The function is accepted.

failure has type Option[PackageProblem], declared on line 75.
None() supplies no payload, so it does not determine its element type alone.
For this comparison, the other operand fixes that type as PackageProblem.
Both operands therefore have type Option[PackageProblem].

Equality accepts these matching types and produces Bool.
The enclosing && combines that Bool with !done, which is also Bool.
The resulting Bool satisfies the while condition.

This does not establish that failure is None() at runtime.
It establishes that comparing failure with None() is well typed.
```

Implementation obligation: retain the equality/constructor constraint and its
source dependency (C03). The current transcript's “right operand: unavailable”
is not enough to produce this explanation honestly. No solver is to be added
inside the renderer.

### Question 3: “Why does the final return have PackageProblem rather than Str as its error type?”

```sh
bin/panack explain src/packages/main.panack --function hash_archive_file --at 89 19
```

Proposed complete stdout:

```text
package_result(sha256_finish(hash), "PKG_TOOL") => Result[Str, PackageProblem]
Source: src/packages/acquisition.panack, line 89, column 19.
View: selected expression and its dependencies. The function is accepted.

sha256_finish expects Sha256; hash has that type.
Its declared result is Result[Str, Str].

package_result[T] expects Result[T, Str] and a Str code.
The first argument fixes T as Str. "PKG_TOOL" satisfies the code parameter.
The declared return type of package_result is Result[T, PackageProblem],
so this call produces Result[Str, PackageProblem].

The other arm, Error(problem) on line 88, carries PackageProblem and leaves
its success type unspecified. Combining the arms fixes that success type as Str.
Both arms therefore produce Result[Str, PackageProblem], matching this
function's declared return type.

Effects: sha256_finish is ordinary and package_result is pure.
Both calls are permitted inside ordinary hash_archive_file.
```

The contract explains the type change. It is not a proof of how the other
function implements error conversion or of its runtime success.

### Question 4: “What broke after I changed done = bytes.length()?”

Use the copied failing tree with the same function selector. Proposed complete
stdout for the default overview:

```text
hash_archive_file: rejected
Source: src/packages/acquisition.panack, lines 68 through 93.
View: overview. One type error was found in this function.

Line 83: done = bytes.length()
  bytes.length() produces Nat, but done was declared Bool on line 74.
  Panack rejected the assignment because Nat is not compatible with Bool.
  If you meant to test whether the chunk is empty, compare its length with 0.

The function's result type still matches Result[Str, PackageProblem].
That successful check does not make the invalid assignment acceptable.
Effects were not checked for this function because its body failed type checking.
Other completed type decisions remain available; they are not discarded.

For the assignment's evidence, repeat the command with --at 83 20.
For every retained decision, repeat it with --details.
```

Existing stderr diagnostic, shown here with the fixture root abbreviated to
`/review/failing-project` (this is the only path substitution, not an output cap):

```text
error: /review/failing-project/src/packages/acquisition.panack:83:20: assignment done is Nat, expected Bool
  83 |             done = bytes.length()
     |                    ^
```

Exit status 1. The actual baseline already preserves the rejected assignment;
the overview's wording and arrangement are the proposed changes. Guidance about
comparing with 0 is appropriate to this reviewed edit, but a general renderer
must not assert intent. It may say “If you meant to test whether the chunk is
empty, compare its length with 0.” Use that conditional wording in implementation.

### Question 5: “How was the loop compiled?”

Proposed command, after creating a validated bytecode/map pair:

```sh
bin/panack locate packages.bc --source src/packages/main.panack --source-map packages.pmap --source-file project/src/packages/acquisition.panack --source-at 76 7
```

Proposed complete stdout:

```text
while !done && failure == None() — how this loop is compiled
Source: src/packages/acquisition.panack, lines 76 through 86.
Function: hash_archive_file. The bytecode and source map were validated.
View: compilation overview.

Condition: !done && failure == None()
  Panack reads done and negates it.
  If that is false, the condition is false without comparing failure with None().
  Otherwise Panack compares failure with None().
  When the condition is false, execution continues at match failure on line 87.

Body: match bounded.read_chunk(file, offset, 65536)
  Panack calls read_chunk and selects the Error or Ok arm of its result.
  The Error arm assigns failure.
  The Ok arm updates offset, takes the size-check branch and assigns done.
  After the body finishes, execution returns to the condition.

Generated control flow
  Panack adds branches for &&, the loop and the match arms.
  Each match arm finishes with a Void result; the loop discards that result.
  The match also has a defensive failure instruction for an unmatched variant.
  Both declared Result variants are covered by this source match.

Add --instructions to see the actual instructions alongside their source.
```

This is a description of actual lowering, not a claim that the compiler reasons
about archive acquisition as a domain. `hash` may also be assigned in the Ok arm;
its nested if is represented by the source-led size-check description. A query
for that if must expose both branches, not skip the hashing branch.

### Question 6: “Show the actual instructions for updating offset.”

```sh
bin/panack locate packages.bc --source src/packages/main.panack --source-map packages.pmap --source-file project/src/packages/acquisition.panack --source-at 80 13 --instructions
```

Proposed complete stdout:

```text
Source: offset = offset + bytes.length()
File: src/packages/acquisition.panack, line 80, column 13.
Function: hash_archive_file. The bytecode and source map were validated.

Source operation          Actual instruction
Read offset               LOAD offset
Read bytes                LOAD bytes
Get bytes.length()        CALL byte_len 1
Add the two Nat values    BINARY +
Store the result in offset STORE offset

byte_len is the built-in operation used for Bytes.length().
The final STORE implements assignment; it does not come from a separate
expression written by the programmer.

Instruction positions in this artifact: 56 through 60.
```

These five instructions and positions were checked against captured disassembly,
not invented for the mock-up. The displayed formatting/source correspondence is
proposed. Current maps do not yet attribute the STORE; packet F must supply its
truthful origin before this query can claim validated complete attribution.
Indices can change when the source or compiler changes; the renderer must use
the validated artifact, never this example's numbers.

## Complete --details candidate

Command:

```sh
bin/panack explain src/packages/main.panack --function hash_archive_file --details
```

This is intentionally the entire proposed detailed output. There are no omitted
sections, collapsed groups or “more…” placeholders. Calls sharing a rule are
listed individually. Call signatures show required argument types in order;
the text explicitly states acceptance and effect permission. Literal strings
are identified by their source location/type instead of copying long error text
multiple times. The unabridged source is printed earlier in this document.

```text
hash_archive_file: accepted
Source: src/packages/acquisition.panack, lines 68 through 93.
View: all decisions in this function, grouped by source.
The complete project was checked. No package operations were run.

Types used below
  PackageProblem is the record from src/packages/diagnostic.panack.
  Sha256 is the record from src/packages/sha256.panack.
  Path and PathError come from the path API; HostError comes from the host API.
  Short names are used because these types are unambiguous in this function.

Line 69: path.from_text(name) => Result[Path, PathError]
  One argument is required; one was supplied. name is Str, as required.
  from_text is pure; the call is allowed in this ordinary function.

Lines 69–92: match path.from_text(name)
  Result has Error and Ok variants. Both occur once, so the match is exhaustive.
  Error(problem), line 70, binds one PathError; its payload count matches.
  Ok(file), line 71, binds one Path; its payload count matches.
  Both patterns belong to this Result type and neither duplicates an earlier arm.
  The Error arm produces Result with PackageProblem as its error type.
  The Ok arm produces Result[Str, PackageProblem]. Combining the arms fixes
  the Error arm's unspecified success type as Str.
  The combined result is Result[Str, PackageProblem], matching the return declaration.

Line 70: construct the path-error result
  package_problem, column 43: (Str, Str) => PackageProblem.
    Both string arguments match; 2 arguments required and supplied. Pure call allowed.
  problem_label, column 29: (PackageProblem, Str, Str) => PackageProblem.
    The constructed problem, label and name match; 3 arguments required and supplied.
    Pure call allowed.
  Error, column 23: (PackageProblem) => Result[Str, PackageProblem].
    One payload is required and supplied; the error type is PackageProblem.
    The success type Str comes from the enclosing result constraint. Pure constructor allowed.

Lines 72–75: local initializers
  sha256_new(), line 72, column 26 => Sha256.
    No arguments required or supplied. Ordinary call allowed.
    Its result matches the declared type of mutable hash.
  0, line 73, column 25 => Nat; matches the declared type of mutable offset.
  false, line 74, column 24 => Bool; matches the declared type of mutable done.
  None(), line 75, column 45 => Option[PackageProblem].
    No payload required or supplied. The failure annotation supplies PackageProblem.
    Pure constructor allowed; the result matches mutable failure's declared type.
  These four initializers produce values suitable for storage.

Line 76: !done && failure == None() => Bool
  !done, column 13: done is Bool; Boolean negation produces Bool.
  None(), column 33: no payload required or supplied; pure constructor allowed.
    The comparison with failure supplies its Option element type, PackageProblem.
  failure == None(), column 22: both operands are Option[PackageProblem].
    Equality accepts matching types and produces Bool.
  && receives Bool on both sides and produces Bool, as required by while.
  These are type decisions, not a claim about the condition's runtime value.

Line 77: bounded.read_chunk(file, offset, 65536) => Result[Bytes, HostError]
  Required arguments are Path, Nat, Nat. file, offset and 65536 match in order.
  Three arguments required and supplied. Ordinary call allowed.

Lines 77–85: match the read result
  Error and Ok are the two Result variants. Each occurs once; the match is exhaustive.
  Error(problem), line 78, binds one HostError; its payload count matches.
  Ok(bytes), line 79, binds one Bytes; its payload count matches.
  Both patterns belong to the subject type and do not duplicate an earlier arm.
  Both arm bodies produce Void, so their combined result is Void.

Line 78: construct and record the read failure
  problem.code, column 129 => Str: HostError has a field code of type Str.
  host_detail, column 117: (Str) => Str.
    The field value matches; one argument required and supplied. Pure call allowed.
  String addition, column 80: both operands are Str; concatenation produces Str.
  package_problem, column 46: (Str, Str) => PackageProblem.
    The code and constructed message match; 2 arguments required and supplied.
    Pure call allowed.
  Some, column 41: (PackageProblem) => Option[PackageProblem].
    One payload required and supplied; its type fixes PackageProblem. Pure constructor allowed.
  The assignment matches failure's Option[PackageProblem] type.

Line 80: offset = offset + bytes.length()
  bytes.length(), column 31 => Nat.
    The receiver is Bytes; no explicit arguments required or supplied.
    The lowered built-in receives that Bytes value as its one argument. Pure call allowed.
  offset + bytes.length(), column 22 => Nat: both numeric operands are Nat.
  The assigned Nat matches offset's declared type. offset is mutable.

Lines 81–82: if offset > 67108864
  The comparison, line 81, column 16, has two Nat operands and produces Bool.
  That satisfies the if condition's Bool requirement.
  Both branch bodies produce Void, so their combined result is Void.

Line 81: record the size-limit failure
  package_problem, column 51: (Str, Str) => PackageProblem.
    Both string arguments match; 2 arguments required and supplied. Pure call allowed.
  Some, column 46: (PackageProblem) => Option[PackageProblem].
    One payload required and supplied; its type fixes PackageProblem. Pure constructor allowed.
  The assignment matches failure's Option[PackageProblem] type.

Line 82: hash = sha256_feed(hash, bytes)
  sha256_feed, column 27: (Sha256, Bytes) => Sha256.
    Both argument types match; 2 arguments required and supplied. Ordinary call allowed.
  The assigned Sha256 matches hash's declared type. hash is mutable.

Line 83: done = bytes.length() == 0
  bytes.length(), column 20 => Nat.
    The receiver is Bytes; no explicit arguments required or supplied.
    The lowered built-in receives that Bytes value as its one argument. Pure call allowed.
  Equality has two Nat operands and produces Bool.
  The assigned Bool matches done's declared type. done is mutable.

Lines 87–90: match failure => Result[Str, PackageProblem]
  failure has type Option[PackageProblem], whose variants are Some and None.
  Each occurs once; both patterns belong to Option, and the match is exhaustive.
  Some(problem), line 88, binds one PackageProblem; its payload count matches.
  None(), line 89, binds no payload; its payload count matches.

Line 88: close the hash and return the recorded problem
  sha256_close, column 28: (Sha256) => Unit.
    hash matches; one argument required and supplied. Ordinary call allowed.
    The Unit result is discarded by this expression statement.
  Error, column 48: (PackageProblem) => Result[Str, PackageProblem].
    One payload required and supplied. Pure constructor allowed.
    Its success type is fixed as Str when the match-arm results are combined.

Line 89: finish the hash and adapt the result type
  sha256_finish, column 34: (Sha256) => Result[Str, Str].
    hash matches; one argument required and supplied. Ordinary call allowed.
  package_result, column 19: (Result[T, Str], Str) => Result[T, PackageProblem].
    The first argument supplies T = Str. The second argument is Str, as required.
    Two arguments required and supplied. Pure call allowed.
    After substitution, this call produces Result[Str, PackageProblem].
  Combining this arm with Error(problem) produces Result[Str, PackageProblem].

Function result
  The outer match also produces Result[Str, PackageProblem].
  This matches hash_archive_file's declared return type on line 68.

Effects
  All 20 call sites above were checked. Their pure or ordinary effects are
  permitted in this ordinary function. There are no await boundaries here.

No numeric guard conversion, Nat subtraction obligation, explicit type-argument
count check or collection-callback contract occurs in this selected function.
This is absence of those operations, not an unavailable explanation.
```

## Completeness ledger for this review

Every currently retained entry is accounted for below. This does not declare the
compiler complete: missing inference/constraint causes still belong to packet B,
and missing source origins to packet F. The mock-up must become checked snapshots
backed by retained facts, not a manually maintained explanatory script.

| Existing entry family | Count | Detailed output location |
| --- | ---: | --- |
| Return compatibility | 1 | Outer match and Function result |
| Call argument compatibility | 26 | Every listed call signature and its matching arguments |
| Annotated initializer compatibility | 4 | Lines 72–75 |
| Assignment compatibility | 5 | Lines 78, 80, 81, 82, 83 |
| Result joins | 4 | Outer match, read-result match, size if, final match |
| Operator checks | 7 | Three at line 76; string addition at 78; addition at 80; comparison at 81; equality at 83 |
| Match/type requirements and payload types | 32 | Three subject/membership/uniqueness/payload/exhaustiveness groups, while condition, if condition and problem.code field |
| Call arity | 20 | Every call, including constructors and two lowered Bytes.length calls |
| Effect boundary | 20 | Every call's permission, with final cross-check |
| **Total captured entries** | **119** | **No existing entry family silently omitted** |

New retained facts may change this count. Never freeze 119 as the acceptance
count for the completed compiler. Assert decisions against source sites and
reachable outcomes, and detect missing evidence rather than merely checking a
fixed total. The full baseline output is evidence of existing behavior, not the
target presentation.

## Review decision

First review the complete overview and one focused query: can a programmer find
the right source section and understand the relevant reason without reading
all details? Then inspect the full detailed output for repetition and omissions.
Finally inspect the source-led compilation query and its optional actual
instructions. Do not accept the entire experience solely because one paragraph
sounds good.

A meaningful failure of this mock-up should change selection/organisation before
renderer implementation. The next review case, after agreement on this approach,
is a real compiler function with generic collections/callbacks and matching.
It must not be reshaped to fit this case. The original noninteractive-only proposal is superseded by the interaction
decision above; the evidence-completeness requirement remains.
