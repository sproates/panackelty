# Install Panackelty

## Start writing Panackelty

The developer preview is designed to be downloaded and run directly. The
download contains everything needed to check, compile, and run programs: the
`panack` command, native VM, self-hosted compiler, and standard library.

Developer-preview archives for `0.1.0-alpha.14` are available from the
[GitHub release](https://github.com/sproates/panackelty/releases/tag/v0.1.0-alpha.14).
The [release policy](../releases/RELEASE_POLICY.md) defines the preview's support and
compatibility boundaries.

### Upgrading

Use the alpha.14 compiler, VM and standard library together. Recompile saved
programs with the matching toolchain. See the [release notes](../../CHANGELOG.md)
for namespace, package and standard-library migration guidance. Older migration
notes remain in the changelog under their original release.

### System requirements

The native alpha.14 release supports:

- Ubuntu 22.04 or newer on x86-64
- macOS 14 or newer on Apple silicon (arm64)
- A terminal, `curl` for the commands below, and `tar` for unpacking the download
- `sha256sum` on Linux or `shasum` on macOS for verifying the download

Download both the archive matching the operating system and processor and its
adjacent `.sha256` file from the Releases page:

| System | Archive |
| --- | --- |
| Linux x86-64 | `panackelty-0.1.0-alpha.14-linux-x86_64.tar.gz` |
| macOS arm64 | `panackelty-0.1.0-alpha.14-macos-arm64.tar.gz` |

Windows and other architectures are not part of the initial preview.

### Package dependencies in the upcoming alpha.15 release

Git is optional. Git 2.29 or newer is required to fetch Git dependencies,
including indirect dependencies. Local packages and HTTPS archive packages do
not otherwise require Git. Verified cached Git packages can be used without
invoking Git. This package workflow is currently available on `next`; the
alpha.14 installation described here does not include it.

The alpha.15 distribution includes the separate package tool alongside the
compiler. Keep the complete toolchain directory together when moving it; linking
`bin/panack` onto PATH is supported. No source checkout, compiler build tools,
command-line `curl` or command-line `tar` are needed to use package commands.
The download instructions themselves still use `curl` and `tar`.

On Ubuntu 22.04, the VM needs the runtime packages `libcurl4`, `zlib1g` and
`libssl3`, plus `ca-certificates` for HTTPS trust. These are runtime requirements;
the `-dev` packages are only needed for source builds. On supported macOS,
curl and zlib come from the operating system and OpenSSL crypto is linked into
the VM. Users of the macOS download do not need Homebrew or an OpenSSL install.

Run `panack deps --help` for the package workflow, or `panack deps restore
--help` for the same reference from a specific command. Package commands require
the matching `share/panackelty/panack-packages-v1.bc` artifact. Running previously
compiled bytecode does not require that artifact.

### Optional: install with one command

This is an additional route for readers who prefer an installer. The manual
archive instructions below remain supported. The public installer supports the
same Ubuntu and macOS release targets.

```sh
curl --proto '=https' --proto-redir '=https' --tlsv1.2 --fail --location --show-error https://raw.githubusercontent.com/sproates/panackelty/main/scripts/install.sh | sh -s -- --version 0.1.0-alpha.14
```

The command executes the [inspectable installer](../../scripts/install.sh) from the
repository's current `main`. The explicit `--version` selects alpha.14; the installer also defaults to alpha.14. It checks the
supported OS/architecture, downloads the release and its SHA-256 file over HTTPS,
and verifies the checksum before unpacking or activating it. Trust includes
GitHub, this repository and your local tools. Same-source checksums detect damaged
or mismatched downloads; they do **not** independently authenticate a compromised
publisher. To review before executing (and avoid a pipeline hiding curl's exit
status), download a local copy first:

```sh
curl --proto '=https' --proto-redir '=https' --tlsv1.2 --fail --location --show-error --output install.sh https://raw.githubusercontent.com/sproates/panackelty/main/scripts/install.sh
# Read install.sh, then execute your reviewed local copy:
sh install.sh --version 0.1.0-alpha.14
```

For a pinned installer revision, replace `main` in the URL with a reviewed full
commit ID. No `sudo` is needed. The dedicated directory
`~/.local/opt/panackelty-installer` owns the downloaded toolchains; keep your
programs elsewhere. The command link is `~/.local/bin/panack`. An existing manual
installation or another command at that path causes a safe refusal; keep using
that installation or move/remove it yourself before choosing this alternative.
The script never edits shell startup files. Use the printed absolute command
path, or explicitly opt in for the current shell:

```sh
export PATH="$HOME/.local/bin:$PATH"
```

Add that line to your chosen shell startup file only if you want a persistent
PATH change. `command -v panack` shows which installation your shell finds. After
installation, use the Hello World program below, replacing
`./panackelty/bin/panack` with `"$HOME/.local/bin/panack"`.

With your downloaded, reviewed `install.sh`, select a published version, repeat
an installation, upgrade (or return to a retained version), and remove:

```sh
sh install.sh --version 0.1.0-alpha.14
sh install.sh --uninstall
```

A version must provide the same supported archive layout. Repeating a version
checks it against a fresh verified download and refuses locally modified files.
Activation switches a symlink atomically after verification and version and native compiler smoke
tests. Upgrades retain older toolchains until uninstall; uninstall removes the
entire dedicated installer directory and its owned command link. It refuses
unrelated command links. It does not remove projects or edit PATH configuration.

Handled interruptions clean the staging directory and lock. After a power loss
or uncatchable kill, first ensure no installer is still running, then remove only
`~/.local/opt/panackelty-installer/.lock` and any `.stage.*` directories inside
that dedicated directory, and rerun. A completed release left before activation
is reused after verification; the previously active command stays usable until
activation. If a first-ever installation stopped before creating `.installer`,
remove the empty dedicated directory before retrying. Network, checksum, layout
and smoke-test failures never replace the active toolchain.

### Install a downloaded release

Start in a new, empty directory you own. Copy the complete block for your
platform into a POSIX-compatible shell (such as Bash or zsh). Each block runs in
a subshell and stops on any error, so a failed download or checksum prevents
extraction without changing your terminal's shell settings. No administrator
permissions or source checkout are needed.

#### Linux x86-64

<!-- release-install-linux-x86_64-begin -->
```sh
(
  set -eu
  release=https://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.14
  archive=panackelty-0.1.0-alpha.14-linux-x86_64.tar.gz
  curl --fail --location --show-error --output "$archive" "$release/$archive"
  curl --fail --location --show-error --output "$archive.sha256" "$release/$archive.sha256"
  sha256sum -c "$archive.sha256"
  tar -xzf "$archive"
  ./panackelty/bin/panack --version
)
```
<!-- release-install-linux-x86_64-end -->

#### macOS arm64 (Apple silicon)

<!-- release-install-macos-arm64-begin -->
```sh
(
  set -eu
  release=https://github.com/sproates/panackelty/releases/download/v0.1.0-alpha.14
  archive=panackelty-0.1.0-alpha.14-macos-arm64.tar.gz
  curl --fail --location --show-error --output "$archive" "$release/$archive"
  curl --fail --location --show-error --output "$archive.sha256" "$release/$archive.sha256"
  shasum -a 256 -c "$archive.sha256"
  tar -xzf "$archive"
  ./panackelty/bin/panack --version
)
```
<!-- release-install-macos-arm64-end -->

The checksum command must report the archive as `OK`, followed by
`panack 0.1.0-alpha.14 (bytecode 9)`. If either download or verification fails,
stop and resolve that error before continuing; do not extract or run the archive.
Both archives expand into `panackelty/`. Keep that whole directory together.

### Write and run a first program

Stay in the directory containing `panackelty/` and save this as `hello.panack`:

<!-- quick-start-program-begin -->
```panackelty
pure greeting(name: Str, answer: Nat): Str {
  "Hello, ${name}. The answer is ${answer}."
}

main(): Void {
  print(greeting("Ada", 42))
}
```
<!-- quick-start-program-end -->

Check the program, run its source, compile it to `hello.bc`, and run the saved
bytecode. The `./panackelty/bin/panack` path uses your extracted toolchain directly;
there is no need to put it on `PATH` first:

<!-- release-install-commands-begin -->
```sh
./panackelty/bin/panack --version
./panackelty/bin/panack check hello.panack
./panackelty/bin/panack run hello.panack
./panackelty/bin/panack compile hello.panack
./panackelty/bin/panack run hello.bc
```
<!-- release-install-commands-end -->

Primary lexer, parser, name, and type failures identify their owning source as
`file:line:column`, including when the error is in an imported module.
Positioned errors also show the source line and a caret. Tabs expand to
four-column stops; Unicode and control characters appear as `\u{hex}` escapes
to keep the caret aligned.

The alpha.14 toolchain produces the output below.

<!-- quick-start-output-begin -->
```text
panack 0.1.0-alpha.14 (bytecode 9)
ok
Hello, Ada. The answer is 42.
wrote hello.bc
Hello, Ada. The answer is 42.
```
<!-- quick-start-output-end -->

`panack hello.panack` is shorthand for `panack run hello.panack`. Functions need
no `fn` keyword, and the final expression in a non-`Void` function is its return
value.

### Optional: use `panack` from any directory

After the local check/run succeeds, you can move the whole toolchain into your
home directory and link its command onto `PATH`. Start beside `panackelty/` and
only use this first-install procedure if neither destination already exists;
for an existing installation, use the upgrade procedure below.

<!-- release-install-path-begin -->
```sh
mkdir -p "$HOME/.local/opt" "$HOME/.local/bin"
mv panackelty "$HOME/.local/opt/panackelty"
ln -s "$HOME/.local/opt/panackelty/bin/panack" "$HOME/.local/bin/panack"
export PATH="$HOME/.local/bin:$PATH"
```

<!-- release-install-path-end -->

Add the final `export` command to the shell's startup file if `~/.local/bin` is
not already on `PATH`. The complete toolchain remains under
`~/.local/opt/panackelty`.

You can now use `panack check hello.panack` and `panack run hello.panack` from
the directory containing your program, or pass its path from another directory.

### Upgrade or remove Panackelty

For the optional home-directory installation above, download and verify the new
archive, extract it in a working directory, and replace the installed directory
while keeping the command link:

```sh
mv panackelty "$HOME/.local/opt/panackelty.new"
mv "$HOME/.local/opt/panackelty" "$HOME/.local/opt/panackelty.old"
mv "$HOME/.local/opt/panackelty.new" "$HOME/.local/opt/panackelty"
panack --version
rm -rf "$HOME/.local/opt/panackelty.old"
```

Do not begin if either temporary path already exists. If the new version does
not run correctly, move `panackelty.old` back before deleting it. To remove
Panackelty completely:

```sh
rm "$HOME/.local/bin/panack"
rm -rf "$HOME/.local/opt/panackelty"
```

### Building the unreleased package tool

A source build of `next` includes a separate Panack package-tool artifact.
Install the libcurl, OpenSSL 3 and zlib development files described in the
[development guide](../internals/DEVELOPMENT.md), then use `make` and `make install`.
The installed launcher, compiler, package tool and standard library must come
from the same build. The package tool is not required to run already compiled
bytecode. Projects with dependencies use the
[explicit lock workflow](TOOLCHAIN.md#locked-local-dependencies-on-next).
