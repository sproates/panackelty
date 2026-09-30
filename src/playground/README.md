# Browser playground

This optional browser build supplies the website's `/playground/` page. It replaces the isolated
Emscripten experiment with a pinned native WASI SDK compiler and a JavaScript
WASI host. The ordinary native CLI, seed, bytecode v9 and `make check` remain
independent of these tools. Public deployment follows the normal reviewed Pages flow.

## Build and test

Use Node 24/npm, the official **WASI SDK 34.0** binary distribution and its bundled
Clang 23.1.0-wasi-sdk. The Linux x86_64 archive SHA-256 is
`b761e3a0721dbae9c09a0059e5fdb2bf917d1b4a8a7b430fb3b5aafb0984b2c4`.
The [upstream SDK](https://github.com/WebAssembly/wasi-sdk/releases/tag/wasi-sdk-34)
supplies a compiled Clang/linker/sysroot; building Panackelty does not invoke an
interpreter through this SDK. This does not claim LLVM's own source build/test
process has no interpreter dependencies. CI downloads and verifies the archive.

From the repository root:

```sh
npm run setup --prefix src/playground
WASI_SDK_PATH=/absolute/path/to/wasi-sdk-34.0-x86_64-linux npm run build --prefix src/playground
make native
npm test --prefix src/playground
node build/playground-tools/node_modules/@playwright/test/cli.js install --with-deps chromium firefox webkit
npm run test:browser --prefix src/playground
```

Setup uses `npm ci --ignore-scripts` with the committed lockfile and puts all
packages under `build/playground-tools`. `make clean` removes dependencies and
generated assets; rerun setup/build afterward. No generated Wasm is committed.
The build executes the native compiler/linker with an SDK-only PATH and records
source/seed/artifact hashes in `build/playground/assets/<sha256>/provenance.json`. CI builds twice
and compares the Wasm bytes. `server.mjs` is a loopback-only static test server.
The build also assembles `build/website` for browser tests and review, using
the actual homepage, shared styles and `/playground/` paths. Run
`node src/playground/server.mjs` to serve it at http://127.0.0.1:4174 locally.
The review tree does not contain a coverage report; production assembly adds
the separately validated report.

The Pages workflow builds the playground from the selected validated website
revision, assembles it with website and coverage assets, and runs real-browser
tests against that complete artifact before upload/deployment. PR runs test the
website preview without deploying. Build or browser failures prevent publication.
The separate playground workflow retains deterministic-build and native-compatibility
checks. The SDK installer is optional and never a native `make check` prerequisite.

The example selector replaces the editor only when Load example is pressed;
it never executes automatically. Source/output use text controls, and code stays
on the device. Each run starts fresh. The nine selectable examples are executed
by all three browser test projects, with navigation, recovery and narrow-layout checks.
Each loaded example has an explanation, an expected-output disclosure and a
suggested edit. The expanded programs cover exact invoices, guarded values,
collection processing, Option/Result handling and a combined order summary.
Expected output retains decimal scale; it is not a currency formatter.

Runtime dependencies are locked to `@bjorn3/browser_wasi_shim` 0.4.2; Playwright
1.63.0 is test-only. The shim's MIT notice accompanies generated assets. We use
only its memory filesystem, never its optional device-storage adapter. No CDN
runtime imports, user JavaScript evaluation or server-side compilation are used.
Fixed invocation arguments are ASCII; exposing arbitrary Unicode command-line
arguments requires addressing the pinned shim's argument-byte-length behavior.

## Contract and limitations

Each Run creates a module worker. The existing compiler seed compiles the source
and stdlib in one VM instance, then a fresh VM verifies/executes the bytecode.
Stop, timeout, completion and errors terminate the worker. Stale worker replies
are ignored. The page renders output as text, not markup. Limits are 32 KiB UTF-8
source and combined output, a 1 MiB compiler artifact, a 2 MiB C stack, 256 MiB
linear memory per VM instance and a 15-second foreground timer. JavaScript/files
use additional memory; garbage collection and background timers are not bounded.
These controls are not a security certification or a total-tab memory limit.

Only the compiler's preallocated output file may receive filesystem writes.
Runtime filesystem writes fail. Input files are read-only and device files are
not mounted; standard input is EOF. The complete typed native host dispatcher is
unavailable (processes, typed filesystem services, sleep, host UTF-8 decode),
using an explicit trap. Legacy bootstrap reads/path operations remain available
inside the disposable filesystem. No real networking or persistent REPL exists.

The browser-profile C adapter is selected explicitly, never through the native
VM wildcard. The core compiler/VM implementation and original corpus expectations
are unchanged. Fourteen forged native-host operand cases retain rejection but
have the profile's explicit unsupported-capability diagnostic; 131 cases must
match native status/stdout/stderr exactly. Tests name the only allowed differences.

Actual browser CI covers Chromium, Firefox and WebKit, including a narrow WebKit
viewport, real module workers, asset loading, stdlib/exactness/Unicode, diagnostics,
literal output, stop/restart, timeout, input/output limits and load errors.
WebKit on Linux is not physical iPhone/Safari validation. Review the workflow's
actual result; having test code alone is not evidence that engines passed.

## Core-library integration

The text, collections and result examples use import-free core types and methods. The browser uses the same compiler seed and core module as native execution; no browser-only source rewriting is involved. Direct compiler filesystem fixtures must supply the bundled stdlib, including core.panack. Rebuild both compiler and stdlib browser assets together.

## Deployment caching

The build hashes the complete staged asset set, including the entry template,
examples, module graph, worker, VM, compiler, stdlib, shared styles and notices.
It publishes assets under `assets/<sha256>/` and rewrites the entry document's
local asset links. Relative imports and worker fetches remain within that version.
`asset-version.txt` identifies the directory for packaging and validation; runtime
code does not fetch a mutable manifest. Rebuilding clears the generated output,
so obsolete unversioned files are not published alongside the replacement.

A new entry document therefore selects one matching dependency set even when a
browser retains the previous deployment in its HTTP cache. The browser regression
uses a real caching server, warms the cache, switches deployments and verifies new
examples and library execution in Chromium, Firefox and WebKit. Assets are not
rewritten in place under an existing hash. Pages can still cache the entry HTML;
this does not force an already open tab to update or preserve every old asset set.
An old page whose dependencies are no longer available fails visibly and needs a
reload after the entry document refreshes. No service worker is installed.
