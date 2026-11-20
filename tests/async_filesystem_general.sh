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
  atomic = require(await fs.write_atomic(file, "abcdef".encode_utf8()))
  expect(require(await fs.read_chunk(file, 2, 3)) == "cde".encode_utf8())
  expect(require(await fs.read_chunk(file, 100000, 3)).length() == 0)
  match await fs.read_chunk(file, 0, 65537) {
    Ok(_) => expect(false),
    Error(problem) => expect(problem.code == "out_of_range")
  }
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
mkdir "$work/root"
printf 'confined' > "$work/root/file"
ln -s file "$work/root/link"
ln -s ../outside "$work/root/escape"
cat > "$work/confined.panack" <<'SOURCE'
import stdlib/filesystem/confined/async as fs
import stdlib/filesystem/confined as ordinary
import stdlib/host/async as host
import stdlib/path as path
pure require[T](result: Result[T,fs.HostError]): T {
  match result { Ok(value) => value, Error(_) => { empty: [T] = []; empty[0] } }
}
pure expect(value: Bool): Unit { checked = [0][if value { 0 } else { 1 }]; () }
async main(): Unit {
  arguments = await host.arguments()
  root = path.from_text(arguments[0]).value_or(path.current())
  file = path.from_text("file").value_or(path.current())
  link = path.from_text("link").value_or(path.current())
  identity: ordinary.FilesystemRootIdentity = require(await fs.root_identity(root))
  read = require(await fs.read(root, file, 100))
  expect(read.identity == identity)
  expect(read.contents == "confined".encode_utf8())
  expect(require(await fs.read_link(root, link)) == "file".encode_utf8())
  expect(require(await fs.read(root, link, 100)).contents == "confined".encode_utf8())
  escaping_link = path.from_text("escape").value_or(path.current())
  match await fs.read(root, escaping_link, 100) { Ok(_) => expect(false), Error(_) => () }
  escape = path.from_text("../outside").value_or(path.current())
  match await fs.read_link(root, escape) { Ok(_) => expect(false), Error(_) => () }
  printed = require(await host.stdout("confined: ok"))
  ()
}
SOURCE
printf 'confined: ok\n' > "$work/expected"
panack run "$work/confined.panack" "$work/root" > "$work/output"
cmp "$work/expected" "$work/output"
panack compile "$work/confined.panack" -o "$work/confined.bc" > /dev/null
./bin/panack run "$work/confined.bc" "$work/root" > "$work/output"
cmp "$work/expected" "$work/output"
printf 'Canonical confined async source/bytecode passed.\n'
sed -e 's@stdlib/filesystem/confined/async@stdlib/filesystem/confined@g' \
    -e 's@stdlib/host/async@stdlib/host@g' \
    -e 's/async main()/main()/g' -e 's/await //g' \
    -e 's/printed = require(host.stdout("confined: ok"))/host.stdout("confined: ok")/' \
    "$work/confined.panack" > "$work/confined-ordinary.panack"
panack run "$work/confined-ordinary.panack" "$work/root" > "$work/output"
cmp "$work/expected" "$work/output"
panack compile "$work/confined-ordinary.panack" -o "$work/confined-ordinary.bc" > /dev/null
./bin/panack run "$work/confined-ordinary.bc" "$work/root" > "$work/output"
cmp "$work/expected" "$work/output"
printf 'Canonical confined ordinary source/bytecode passed.\n'
cat > "$work/staging.panack" <<'SOURCE'
import stdlib/filesystem/staging as stage
import stdlib/filesystem/staging::{PublishMode.CreateOnly, PublishMode.ReplaceExisting}
import stdlib/filesystem/confined as confined
import stdlib/host as host
import stdlib/path as path
pure require[T](result: Result[T,stage.HostError]): T {
  match result { Ok(value) => value, Error(_) => { empty: [T] = []; empty[0] } }
}
pure expect(value: Bool): Unit { checked = [0][if value { 0 } else { 1 }]; () }
main(): Unit {
  args = host.arguments()
  root = path.from_text(args[0]).value_or(path.current())
  relative = path.from_text("payload").value_or(path.current())
  destination = path.from_text("published").value_or(path.current())
  first: DirectoryStage = require(stage.create(root))
  alias = first
  nested = path.from_text("nested").value_or(path.current())
  made_directory = require(stage.create_directory(first, nested))
  link = path.from_text("link").value_or(path.current())
  made_link = require(stage.create_symlink(first, link, relative))
  observed_path = require(stage.path(first))
  wrote = require(stage.write_chunk(first, relative, "first".encode_utf8(), 0))
  published = require(stage.publish(first, destination, CreateOnly()))
  match stage.discard(alias) { Ok(_) => expect(false), Error(_) => () }
  second = require(stage.create(root))
  wrote_again = require(stage.write_chunk(second, relative, "second".encode_utf8(), 0))
  match stage.publish(second, destination, CreateOnly()) { Ok(_) => expect(false), Error(_) => () }
  replaced = require(stage.publish(second, destination, ReplaceExisting()))
  published_root = root.join(destination).value_or(root)
  expect(require(confined.read(published_root, relative, 100)).contents == "second".encode_utf8())
  third = require(stage.create(root))
  discarded = require(stage.discard(third))
  host.stdout("staging: ok")
  ()
}
SOURCE
mkdir "$work/staging-source" "$work/staging-bytecode"
printf 'staging: ok\n' > "$work/expected"
panack run "$work/staging.panack" "$work/staging-source" > "$work/output"
cmp "$work/expected" "$work/output"
panack compile "$work/staging.panack" -o "$work/staging.bc" > /dev/null
./bin/panack run "$work/staging.bc" "$work/staging-bytecode" > "$work/output"
cmp "$work/expected" "$work/output"
cat > "$work/forged-stage.panack" <<'SOURCE'
import stdlib/filesystem/staging as stage
main(): Unit { ignored = stage.discard(1); () }
SOURCE
if panack check "$work/forged-stage.panack" > "$work/forged-error" 2>&1; then
  echo 'Forged numeric stage unexpectedly accepted' >&2
  exit 1
fi
grep 'DirectoryStage' "$work/forged-error" > /dev/null
printf 'Opaque staging source/bytecode and numeric rejection passed.\n'
cat > "$work/pure-stage.panack" <<'SOURCE'
import stdlib/filesystem/staging as stage
pure invalid(resource: DirectoryStage): Unit { ignored = stage.discard(resource); () }
main(): Unit { () }
SOURCE
if panack check "$work/pure-stage.panack" > "$work/pure-error" 2>&1; then
  echo 'Pure staging side effect unexpectedly accepted' >&2
  exit 1
fi
grep 'pure' "$work/pure-error" > /dev/null
cat > "$work/constructed-stage.panack" <<'SOURCE'
main(): Unit { forged = DirectoryStage(1); () }
SOURCE
if panack check "$work/constructed-stage.panack" > "$work/constructed-error" 2>&1; then
  echo 'Opaque stage constructor unexpectedly accepted' >&2
  exit 1
fi
grep 'DirectoryStage' "$work/constructed-error" > /dev/null
sed -e 's@import stdlib/filesystem/staging as stage@import stdlib/filesystem/staging/async as stage@' \
    -e 's@import stdlib/filesystem/confined as confined@import stdlib/filesystem/confined/async as confined@' \
    -e 's@import stdlib/host as host@import stdlib/host/async as host@' \
    -e 's/^main()/async main()/' \
    -e 's/stage\.\([a-z_]*\)(/await stage.\1(/g' \
    -e 's/confined.read(/await confined.read(/g' \
    -e 's/host.arguments(/await host.arguments(/g' \
    -e 's/host.stdout(/reported = await host.stdout(/g' \
    "$work/staging.panack" > "$work/staging-async.panack"
mkdir "$work/staging-async-source" "$work/staging-async-bytecode"
printf 'staging: ok\n' > "$work/expected"
panack run "$work/staging-async.panack" "$work/staging-async-source" > "$work/output"
cmp "$work/expected" "$work/output"
panack compile "$work/staging-async.panack" -o "$work/staging-async.bc" > /dev/null
./bin/panack run "$work/staging-async.bc" "$work/staging-async-bytecode" > "$work/output"
cmp "$work/expected" "$work/output"
printf 'Async opaque staging source/bytecode passed.\n'
