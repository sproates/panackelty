# Panackelty runtime

The runtime is the deliberately small effectful boundary between VM programs
and the host operating system. It supplies terminal I/O, UTF-8 text and binary
file I/O, command arguments, environment snapshots, stderr and process status,
path operations, and verified nested bytecode execution, plus pure collection,
byte, string, and conversion built-ins.

Purity metadata is shared by the compiler and bytecode verifier. A pure function
cannot call an effectful runtime operation. The native VM implements these
services behind the same semantic contract.

`command_args`, `eprint`, `process_exit`, `path_resolve`, `file_exists`,
`run_bytecode`, and `run_bytecode_args` are effectful, as are `environment_has`
and `environment_get`.
Lexical path transforms (`path_parent`,
`path_join`, `path_suffix`, `path_with_suffix`, and `path_is_absolute`) are pure.
All path services reject embedded NUL bytes instead of allowing a C host to
silently truncate the value. Text reads validate UTF-8, binary reads preserve
all bytes, and filesystem failures cross the boundary as `I/O error` results.
Both nested-execution services decode and verify their byte buffer before
starting a new VM invocation. `run_bytecode` inherits arguments;
`run_bytecode_args` accepts an explicit argument snapshot. The stable service
names, signatures, snapshot behavior, and failure rules are frozen in
[`ABI.md`](ABI.md).

This directory contains contracts rather than a Python re-export layer. The
development oracle remains isolated under `src/bootstrap`; the public host
implementation is the native VM.
