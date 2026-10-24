<!-- validation:positioning-copy-begin -->
# Panackelty

**Exact arithmetic. Guarded types. Compiler explanations. Self hosted.**

Panackelty is an evolving, statically checked programming language. It combines arbitrary-precision integers and exact
rational and decimal arithmetic with guarded domain types, explicit effects,
structured data, and a portable bytecode VM. The `panack explain` command shows
evidence for selected natural-number subtraction proofs and local call/await
effect boundaries. Successive self-hosted compiler builds are checked for
byte-identical output.

The name comes from [panackelty](https://en.wikipedia.org/wiki/Panackelty), a
traditional North East English dish particularly associated with Sunderland
and County Durham. It brings meat and vegetables together in one pan; **Panack**
is also the name of this project's command-line tool.

<p align="center">
  <img src="docs/assets/panackelty.jpg" width="800" alt="A home-cooked plate of panackelty with beef, potatoes, carrots, and peas in gravy">
</p>
<p align="center">
  <sub>
    A home-cooked plate of panackelty, the North East English dish behind the
    name. Photograph by the project author,
    <a href="docs/assets/README.md">licensed under CC BY 4.0</a>.
  </sub>
</p>

[Website](https://panackelty.com) · [Documentation](docs/README.md) · [Examples](examples/README.md) · [Roadmap](ROADMAP.md) · [Contributing](CONTRIBUTING.md)

<!-- validation:positioning-copy-end -->

## Get started

**Latest native release: [0.1.0-alpha.14](https://github.com/sproates/panackelty/releases/tag/v0.1.0-alpha.14).**
Available for Linux x86-64 and macOS Apple silicon. This is a developer preview:
source APIs and bytecode may change between releases.

1. [Install the release](docs/guides/INSTALLATION.md), with checksum verification.
2. [Take the language tour](docs/guides/LANGUAGE_TOUR.md) and run the examples.
3. [Use the toolchain](docs/guides/TOOLCHAIN.md) to check, compile and explain programs.

Alpha.14 adds type-owned methods and a routed HTTP server framework, with HTTP
API references and a playbook of runnable examples. It also includes local packages,
native HTTP clients, compiler explanations and a static file server.
Read the [release notes](CHANGELOG.md) and [compatibility policy](docs/releases/RELEASE_POLICY.md).

Alpha.14 includes [type-owned methods](docs/reference/SPEC.md#type-owned-methods-next) for records, enums and built-in types. The alpha.14 compiler uses owned dot calls, without receiver-to-global fallback; the temporary arrow syntax is removed.

For migration from earlier releases, see the
[type-owned method migration guide](docs/guides/METHOD_MIGRATION.md) and
[P15 consumer acceptance](docs/design/type-owned-dispatch/p15-consumer-acceptance.md).
Runnable [owned and generic method examples](examples/README.md#type-owned-methods-alpha14)
show custom APIs, Map coexistence and typed callbacks.

The unreleased `next` branch adds [explicit locks and local, archive and Git dependencies](docs/guides/TOOLCHAIN.md#locked-local-dependencies-on-next) through a separate package tool written in Panack.
Its generic host APIs also provide exact-environment process launch and confined
symbolic-link operations; see the [standard-library contracts](src/stdlib/README.md).
Git dependencies support pinned HTTPS commits and shared repository subdirectories on `next`.
Its generic `stdlib/http_stream` transport supports bounded blocking reads for
command-line consumers. Cross-source acceptance and final packaging remain in progress.

## Explore

| I want to… | Start here |
| --- | --- |
| Understand the language | [Language specification](docs/reference/SPEC.md) |
| Find standard-library APIs | [Standard library](src/stdlib/README.md) |
| Use HTTP in a program | [HTTP package](packages/http/README.md) |
| Serve a directory | [HTTP server tool](tools/http-server/README.md) |
| Understand local packages | [Package roots and manifests](docs/design/MODULE_PACKAGE_DESIGN.md#package-roots-manifest-and-source-distribution) |
| Understand compiler decisions | [Compiler explanations](docs/guides/COMPILER_EXPLANATIONS.md) |
| Build the compiler | [Development guide](docs/internals/DEVELOPMENT.md) |
| Understand the implementation | [Architecture](docs/internals/ARCHITECTURE.md) |
| See what is planned | [Roadmap](ROADMAP.md) |

## Contribute

See [contributor guidance](CONTRIBUTING.md) for the development workflow and
[security policy](SECURITY.md) for reporting vulnerabilities.
Panackelty is [MIT licensed](LICENSE); the photograph has its own attribution above.
The native digest adapter uses OpenSSL 3 or later under
[Apache 2.0](docs/licenses/OpenSSL-Apache-2.0.txt). Archive collision tables use
[Unicode data](docs/licenses/Unicode-License.txt).
