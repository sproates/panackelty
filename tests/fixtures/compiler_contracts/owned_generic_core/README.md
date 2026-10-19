# Generic owned methods in the toolchain core

The compiler-driver regression uses this isolated standard-library root to test
canonical Option/Result ownership and P05 built-in source-method registration
before P08 migrates production core sources.
It executes source and compilation through the native compiler driver, then runs
the saved artifact through `panack`. The installed launcher intentionally pins
its bundled stdlib and does not accept an environment override; this fixture
does not change that contract. Expected stdout is `7`, `9`, `yes`, `no`, `9`, `7`, `42`, `selected`, `true`,
`9`, `text`, `5`. None/Error constructors infer receiver arguments from their
fallbacks. The remaining calls exercise Map/Set/array/Str/Bytes source methods,
method type arguments and private core helpers. Existing global intrinsics remain
inside the fixture; owned intrinsic dispatch is P06.

The explicit `core_methods.panack` entry is invoked by the dedicated driver tests.
It is not a normal `main.panack` root: the repository-wide identity-root audit
checks those roots against the bundled production stdlib. This fixture requires
its isolated core, and all three source/compile/bytecode assertions remain active.
