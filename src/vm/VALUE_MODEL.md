# Native VM value and memory model

This document describes the representation used by the portable C11 seed VM. It
is an implementation contract beneath the language-level value semantics in
`src/bytecode/FORMAT.md`; it does not change bytecode version 8.

## Values

Every operand-stack slot, local, collection element, record field, variant
payload, and map entry points to a reference-counted `Value` with a tagged
union payload. `Bool` stores its flag inline; `Unit` and `Void` need no payload.
Their distinct tags prevent a successful Unit value from becoming a no-result
sentinel.

- `Nat` and `Int` use a sign plus little-endian base-1,000,000,000 limbs. Zero
  has no limbs and is never negative.
- `Rat` owns two arbitrary-precision integers. Construction reduces them by
  their greatest common divisor, makes the denominator positive, and normalizes
  zero to `0/1`. Destruction frees both integers.
- `Dec` stores a signed arbitrary-precision coefficient and a signed base-10
  exponent. Addition, subtraction and multiplication preserve exact scale; division
  succeeds only when the reduced denominator contains no prime factors other than
  two and five, and removes redundant fractional trailing zeros. Comparison reads
  coefficient digits without allocating aligned temporary values.
- `Str` owns UTF-8 bytes plus a code-point count and an ASCII flag computed
  once at construction. Length reads the count directly; ASCII indexing and
  slice/prefix offsets use byte offsets directly, while non-ASCII offsets retain
  UTF-8 traversal. These immutable fields are internal metadata, not serialized
  bytecode, and do not change code-point semantics or bounds checks.
- Arrays, maps, sets, records, variants, byte buffers, and ranges are immutable
  heap objects. Persistent operations allocate a new value and preserve every
  input's observable length and elements. Array append can share internal
  backing storage as described below; other containers retain children directly.
- Iterators are frame-owned cursors retaining the iterable they traverse. They
  never escape into bytecode constants or serialized artifacts.

Names in decoded programs own their UTF-8 storage independently of runtime
values. Instructions refer to those immutable decoded names and operands.

## Ownership and reclamation

Heap objects use non-atomic reference counts because one VM invocation is
single-threaded. Copying an owning `Value *` requires retaining its object;
discarding an owning value releases it. Releasing the final reference walks and
releases owned children or its shared array backing store before freeing the
object. Operand stacks, locals, call
arguments, returned values, containers, iterators, and decoded constants each
have explicit ownership.

An execution handle owns a growable array of frames rather than recursing through
the native C stack for Panackelty function calls. Suspended frames retain locals,
operands and iterators exactly as running frames do. Frame-array growth must not
leave cached pointers into the old allocation. Partial frames are registered
before argument binding so allocation failures release every installed reference.
Returns transfer one value to the caller, or to the execution's completed result;
traps, embedded exit and destruction release the remaining frame chain.
The internal task session owns each task execution and retains successful results
while joining children. Failed/cancelled scopes release those results. A fake
pending print owns one retained input until delivery or cancellation; stale queued
metadata owns no VM values. Session destruction releases every outstanding request,
frame and result. Program and invocation snapshots remain borrowed. There is no
external native producer in this experiment; real backend teardown is separate.

The synchronous adapter transfers its final result to its C caller. The resumable
API lends its result until handle destruction. Programs and host snapshots are
borrowed, must outlive the handle, and are not implicitly copied or freed by it.

The value graph cannot contain cycles: source values have no mutable references,
and every composite constructor receives already-complete children. Reference
counting therefore reclaims all reachable runtime allocations without a tracing
collector. Decoded programs own their individually allocated names, constants, and
instruction arrays; `free_program()` releases them after frames and values.
See the module headers for borrowing and ownership-transfer contracts.

Allocation overflow and host memory exhaustion are fatal native-runner errors,
which the bytecode contract deliberately places outside language-level traps.
All size additions and multiplications are checked before allocation.

## Persistent array append storage

Ordinary array construction keeps its exact-size owned element buffer. Appended
arrays use a separately reference-counted backing store with a count and
capacity. A prefix and its one-element extension can share that store while
each value keeps its own visible length. Only a store with one view and spare
capacity can be extended; an additional branch or capacity growth allocates an
independent buffer. Capacity grows geometrically for eligible elements, and
size arithmetic is checked before allocation.

The backing store retains each child once. At most two views share it. When
the prefix dies first, the extension keeps its elements. When the extension
dies first, its extra child is released immediately and the surviving prefix's
storage count is restored. Thus keeping a snapshot does not pin an unbounded
future suffix, and iteration, equality, rendering and indexing still use the
value's visible count.

Before sharing, a bounded inspection follows the appended item's ownership
edges through arrays, maps, sets, iterators, records and variants. It includes
any hidden suffix owned by a shared backing store, not just visible elements.
Reaching the proposed store or exhausting the 64-value inspection budget takes
the independent-copy path. This conservative rule prevents an appended element
from retaining a prefix backed by its own store, which would otherwise create
a reference cycle. Small independent nested collections can share safely. No language-visible mutation,
bytecode layout change or tracing collector is introduced.

Append allocates the result before modifying a shared store. The copying path
allocates all storage before retaining children. Allocation failure therefore
leaves both borrowed inputs unchanged; partial allocations are freed. Direct
native tests cover both release orders, branching, growth, nested aliases,
inspection fallback and size overflow. Fault sweeps cover each allocation site
in empty, shared, growing and nested append operations.

## Limits and safety

The native loader applies every version-8 limit before allocating or iterating
the corresponding input. It validates UTF-8, minimal integers, decimal BCD,
canonical function ordering, flags, opcodes, jump targets, calls, arities, and
purity before execution. Runtime operations retain independent type, bounds,
underflow, missing-key, match, stack, and exact-division checks because verified
bytecode is not assumed to have passed the source type checker.

The seed VM is portable C11 and uses only the C standard library plus the small
operating-system adaptation in the host-service implementation. No compiler,
third-party numeric library or platform-specific value layout
is part of the native executable.

## Opaque host-domain values

`V_PATH` owns length-delimited native bytes, with a trailing C NUL outside the
length. Its constructors reject empty data and interior NULs. `V_DURATION` and
`V_INSTANT` own arbitrary-precision signed integers, measured in nanoseconds.
Distinct tags enforce opacity; record or variant construction cannot create
these values. Existing reference counting releases their byte/integer storage.
Equality compares values within the same tag. Only explicit path conversion
exposes bytes; instants have no tick accessor and render as `<Instant>`.
These tags are runtime-only and are never accepted as serialized constants.

Allocation-failure tests exercise partial construction and frame cleanup. Stack
push and local assignment consume their input reference on both success and
failure; callers must not release it again. Failed constructors release retained
children and partially copied fields. Test-only allocation/syscall wrappers are
compiled into a separate executable and are absent from the production runner.
