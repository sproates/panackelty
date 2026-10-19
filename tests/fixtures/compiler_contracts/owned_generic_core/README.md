# Generic owned methods in the toolchain core

The compiler-driver regression uses this isolated standard-library root to test
canonical Option/Result ownership before P08 migrates production core sources.
It executes source and compilation through the native compiler driver, then runs
the saved artifact through `panack`. The installed launcher intentionally pins
its bundled stdlib and does not accept an environment override; this fixture
does not change that contract. Expected stdout is `7`, `9`, `yes`, `no`, `9`, `7`; the last two calls infer
receiver arguments from the fallback on unannotated None/Error constructors.

The explicit `core_methods.panack` entry is invoked by the dedicated driver tests.
It is not a normal `main.panack` root: the repository-wide identity-root audit
checks those roots against the bundled production stdlib. This fixture requires
its isolated core, and all three source/compile/bytecode assertions remain active.
