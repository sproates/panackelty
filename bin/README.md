# Command launcher

`panack` locates and runs the native VM and matching compiler seed.

From a source checkout, build with `make` and invoke `./bin/panack`.
An installed release provides the same command in its installation's `bin/`.
The launcher resolves symlinks and locates its resources relative to itself,
so the current working directory does not select the compiler or standard library.

See the [toolchain guide](../docs/guides/TOOLCHAIN.md) for commands and
[development guide](../docs/internals/DEVELOPMENT.md) for source builds.
