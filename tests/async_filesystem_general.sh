#!/bin/sh
set -eu
work=$(mktemp -d tests/async-filesystem-general.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
    if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
        PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
    else
        ./bin/panack "$@"
    fi
}
cat > "$work/main.panack" <<'SOURCE'
import stdlib/filesystem/async as fs
import stdlib/filesystem as ordinary
import stdlib/filesystem::{FileKind.RegularFile}
import stdlib/host/async as host
import stdlib/path as path

pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}
pure require[T](result: Result[T,fs.HostError]): T {
  match result {
    Ok(value) => value,
    Error(problem) => { failed: [T] = []; failed[0] }
  }
}
async main(): Unit {
  arguments = await host.arguments()
  parent = path.from_text(arguments[0]).value_or(path.current())
  directory = require(await fs.create_temp_directory(parent))
  file = require(await fs.create_temp_file(directory))
  payload = "async filesystem".encode_utf8()
  wrote = require(await fs.write(file, payload))
  expect(require(await fs.read(file, 1024)) == payload)
  info: ordinary.FileMetadata = require(await fs.metadata(file))
  expect(info.size == payload.length())
  expect(info.kind == RegularFile())
  expect(require(await fs.list_directory(directory)).len() == 1)
  expect(require(await fs.directories_overlap(directory, directory)))
  child = directory.join(path.from_text("child").value_or(path.current())).value_or(directory)
  created = require(await fs.create_directory(child))
  expect(require(await fs.list_directory(directory)).len() == 2)
  expect(require(await fs.directories_overlap(directory, child)))
  match await fs.read(file, 134217729) {
    Ok(value) => expect(false),
    Error(problem) => expect(problem.code == "out_of_range")
  }
  removed_child = require(await fs.remove_directory(child))
  removed_file = require(await fs.remove_file(file))
  match await fs.read(file, 1024) {
    Ok(value) => expect(false),
    Error(problem) => expect(problem.code == "not_found")
  }
  removed_directory = require(await fs.remove_directory(directory))
  reported = require(await host.stdout("async filesystem general: ok"))
  ()
}
SOURCE
printf 'async filesystem general: ok\n' > "$work/expected"
panack run "$work/main.panack" "$work" > "$work/output"
cmp "$work/expected" "$work/output"
panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
./bin/panack run "$work/main.bc" "$work" > "$work/output"
cmp "$work/expected" "$work/output"
printf 'General async filesystem source/bytecode, bounds, errors and shared types passed.\n'
cat > "$work/ordinary.panack" <<'SOURCE'
import stdlib/filesystem as fs
import stdlib/host as host
import stdlib/path as path
pure expect(value: Bool): Unit {
  checked = [0][if value { 0 } else { 1 }]
  ()
}
pure require[T](result: Result[T,fs.HostError]): T {
  match result { Ok(value) => value, Error(problem) => { failed: [T] = []; failed[0] } }
}
main(): Void {
  parent = path.from_text(host.arguments()[0]).value_or(path.current())
  directory = require(fs.create_temp_directory(parent))
  file = require(fs.create_temp_file(directory))
  written = require(fs.write_atomic(file, "abcdef".encode_utf8()))
  expect(require(fs.read_chunk(file, 2, 3)) == "cde".encode_utf8())
  expect(require(fs.directories_overlap(directory, directory)))
  removed_file = require(fs.remove_file(file))
  removed_directory = require(fs.remove_directory(directory))
  print("ordinary filesystem canonical names: ok")
}
SOURCE
printf 'ordinary filesystem canonical names: ok\n' > "$work/expected"
panack run "$work/ordinary.panack" "$work" > "$work/output"
cmp "$work/expected" "$work/output"
panack compile "$work/ordinary.panack" -o "$work/ordinary.bc" > /dev/null
./bin/panack run "$work/ordinary.bc" "$work" > "$work/output"
cmp "$work/expected" "$work/output"
printf 'Ordinary canonical filesystem source/bytecode passed.\n'
