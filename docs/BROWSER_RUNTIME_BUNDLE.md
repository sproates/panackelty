# Browser runtime dependency bundle

The browser playground is moving toward a separate downstream repository. Core
Panackelty therefore exposes a **versioned build-input bundle**, not a stable C
embedding ABI.

Run:

```sh
make browser-runtime-bundle
```

The generated `build/browser-runtime/` directory contains the portable VM
translation units and headers required by the browser build, the matching
self-hosted compiler seed, standard-library sources, bytecode contract, license
and a SHA-256 manifest.

The browser consumer supplies its own host-capability adapter. Native CLI,
filesystem/process host capabilities and TCP client/server implementations are
deliberately absent.

## Compatibility identity

Consumers pin the complete bundle. `MANIFEST` records:

- bundle format version;
- Panackelty release/development version;
- bytecode version;
- compiler-seed SHA-256;
- hashes of bundled VM and standard-library inputs.

Bytecode version alone is **not** a compatibility promise. During the developer
preview, two toolchain revisions using bytecode v9 may differ through additive
runtime intrinsics or standard-library/compiler changes. A downstream browser
build must update deliberately to a newly published bundle and rerun its own
browser suite.

This bundle packages matched build inputs as one dependency. It does not make
private VM C functions, structures or headers a supported third-party ABI.
