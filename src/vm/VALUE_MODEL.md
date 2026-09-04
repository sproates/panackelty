# Native VM value and memory model

This document freezes the representation used by the portable C11 seed VM. It
is an implementation contract beneath the language-level value semantics in
`src/bytecode/FORMAT.md`; it does not change bytecode version 7.

## Values

Every operand-stack slot, local, collection element, record field, variant
payload, and map entry is a `PnValue`: a small tagged union. `Bool` and `Void`
are immediate. All other values point to an immutable heap object carrying its
kind, reference count, and payload.

- `Nat` and `Int` use a sign plus little-endian base-1,000,000,000 limbs. Zero
  has no limbs and is never negative.
- `Dec` stores a signed arbitrary-precision coefficient and a signed base-10
  exponent. Arithmetic preserves exact scale; division succeeds only when the
  reduced denominator contains no prime factors other than two and five.
- `Str` owns validated UTF-8 bytes and a lazily computed code-point count.
- Arrays, maps, sets, records, variants, byte buffers, and ranges are immutable
  heap objects. Persistent operations allocate a new container and retain the
  referenced elements; they never mutate an input container.
- Iterators are frame-owned cursors retaining the iterable they traverse. They
  never escape into bytecode constants or serialized artifacts.

Names in decoded programs own their UTF-8 storage independently of runtime
values. Instructions refer to those immutable decoded names and operands.

## Ownership and reclamation

Heap objects use non-atomic reference counts because one VM invocation is
single-threaded. Creating or copying an owning `PnValue` retains its object;
discarding an owning value releases it. Releasing the final reference walks and
releases child values before freeing the object. Operand stacks, locals, call
arguments, returned values, containers, iterators, and decoded constants each
have explicit ownership.

The value graph cannot contain cycles: source values have no mutable references,
and every composite constructor receives already-complete children. Reference
counting therefore reclaims all reachable runtime allocations without a tracing
collector. Decoded programs are arena-owned and freed as one program after all
frames and values are released.

Allocation overflow and host memory exhaustion are fatal native-runner errors,
which the bytecode contract deliberately places outside language-level traps.
All size additions and multiplications are checked before allocation.

## Limits and safety

The native loader applies every version-7 limit before allocating or iterating
the corresponding input. It validates UTF-8, minimal integers, decimal BCD,
canonical function ordering, flags, opcodes, jump targets, calls, arities, and
purity before execution. Runtime operations retain independent type, bounds,
underflow, missing-key, match, stack, and exact-division checks because verified
bytecode is not assumed to have passed the source type checker.

The seed VM is portable C11 and uses only the C standard library plus the small
operating-system adaptation in the host-service implementation. No compiler,
Python runtime, third-party numeric library, or platform-specific value layout
is part of the native executable.
