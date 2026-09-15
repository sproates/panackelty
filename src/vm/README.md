# Panackelty virtual machine

The Panackelty VM is a stack machine. Every call frame owns a program counter, local
variables, and an operand stack. Its instruction set covers constants, local
access, arithmetic, collection and algebraic-data construction, iteration,
control flow, calls, and returns.

Two independent implementations enforce this contract. The portable C11 seed
VM in this directory is the bootstrap execution target. The transitional
Python VM in `src/bootstrap/panackelty.py` remains a development oracle used for
differential conformance.

The VM trusts neither source compilation nor bytecode files. Serialized
artifacts are verified before execution, and safety checks such as bounds
checking and `Nat` underflow remain enforced at runtime.

The frozen version-7 execution semantics and instruction stack effects live in
[`../bytecode/FORMAT.md`](../bytecode/FORMAT.md). The VM converts invalid dynamic
bytecode state into a Panackelty trap so host-language indexing, lookup, type,
and arithmetic exceptions do not cross the runtime boundary. A shared forged
runtime corpus exercises indirect-call validation, `Nat` underflow, zero
division, invalid byte and UTF-8 values, missing map keys, and single- and
multi-value stack underflow against both VM implementations.

The native representation, ownership, allocation, and reclamation rules are
specified in [`VALUE_MODEL.md`](VALUE_MODEL.md).

`native.c` is the portable C11 seed executable. Its `check` command performs
bounded version-7 decoding and independent semantic verification; `run`
executes verified artifacts with the reference-counted value model, exact
numerics, persistent collections, UTF-8 operations, and stable host ABI. It
accepts and runs the complete compiler and standard-library artifacts and
consumes the same malformed vectors as the bootstrap loader. Build it with
`make native`. The build defaults to `CFLAGS=-O2`, retaining strict C11 and
warning checks. `CC`, `CPPFLAGS`, `CFLAGS`, `LDFLAGS`, and `LDLIBS` are
configurable. Run `make clean` before changing flags, for example before
`make native CFLAGS="-O0 -g"` for debugging. Optimisation does not change the
bytecode contract or disable runtime verification.

String construction records code-point count and whether every byte is ASCII.
Length is constant time, and ASCII indexing, slicing, and prefix-offset lookup
avoid rescanning the string. Non-ASCII offsets retain UTF-8 traversal. Every
string-producing operation uses the same constructor, including concatenation,
interpolation, slicing, reversal, decoding, and host inputs; bounds and UTF-8
validation remain in place.
