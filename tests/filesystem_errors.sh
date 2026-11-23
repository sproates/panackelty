#!/bin/sh
set -eu
work=$(mktemp -d tests/filesystem-errors.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
panack() {
  if [ -n "${PANACK_TEST_COMPILER:-}" ]; then
    PANACKELTY_STDLIB_PATH="$PWD/src/stdlib" ./panack-vm run "$PANACK_TEST_COMPILER" "$@"
  else "${PANACK_TEST_COMMAND:-./bin/panack}" "$@"; fi
}
mkdir "$work/root"
printf abc > "$work/root/file"
ln -s file "$work/root/link"
ln -s ../outside "$work/root/escape"
cat > "$work/template.panack" <<'SOURCE'
import stdlib/filesystemSUFFIX as files
import stdlib/filesystem/confinedSUFFIX as confined
import stdlib/filesystem/stagingSUFFIX as staging
import stdlib/filesystem/legacySUFFIX as legacy
import stdlib/filesystem/errors as errors
import stdlib/filesystem/errors::{FilesystemError, FilesystemErrorKind.NotFound,
  FilesystemErrorKind.NotDirectory, FilesystemErrorKind.NotRegularFile,
  FilesystemErrorKind.AlreadyExists, FilesystemErrorKind.DirectoryNotEmpty,
  FilesystemErrorKind.OutputLimitExceeded, FilesystemErrorKind.InvalidArgument,
  FilesystemErrorKind.InvalidStage, FilesystemErrorKind.DestinationConflict,
  FilesystemErrorKind.LimitExceeded, FilesystemErrorKind.OtherFailure,
  FilesystemErrorKind.FilesystemFailure, FilesystemErrorKind.UnsupportedHost,
  FilesystemOperation.Metadata, FilesystemOperation.Read, FilesystemOperation.ReadConfined,
  FilesystemOperation.ReadLink, FilesystemOperation.StagePublish, FilesystemOperation.StagePath,
  FilesystemOperation.StageCreateDirectory, FilesystemOperation.StageWriteChunk,
  FilesystemOperation.StageCreateSymlink, FilesystemOperation.StageDiscard}
import stdlib/filesystem/staging::{PublishMode.CreateOnly}
import stdlib/consoleSUFFIX as console
import stdlib/hostSUFFIX as host
import stdlib/host::{HostError}
import stdlib/path as path
pure expect(ok: Bool): Unit { check = [0][if ok { 0 } else { 1 }]; () }
pure p(text: Str): Path { path.from_text(text).value_or(path.current()) }
pure require[T,E](result: Result[T,E]): T {
  match result { Ok(value) => value, Error(_) => { missing: [T] = []; missing[0] } }
}
pure failed[T](result: Result[T,FilesystemError]): FilesystemError {
  match result { Error(problem) => problem, Ok(_) => { missing: [FilesystemError] = []; missing[0] } }
}
EFFECT main(): Unit {
  arguments = WAIT host.arguments()
  root = p(arguments[0])
  missing = require(root.join(p("missing")))
  file = require(root.join(p("file")))
  problem = failed(WAIT files.metadata(missing))
  expect(problem.kind == NotFound())
  expect(problem.operation == Metadata(missing))
  expect(problem.native.code == "not_found" && problem.native.native_code > 0)
  display = missing.display()
  number = problem.native.native_code
  expect(problem.render() == "Cannot read metadata for '${display}': the path or a required path component could not be found. Check the spelling and that the required files and directories exist. Host error number: ${number}.")
  match WAIT legacy.metadata(missing) {
    Ok(_) => expect(false), Error(raw) => expect(raw == problem.native)
  }
  limited = failed(WAIT files.read(file, 1))
  expect(limited.kind == OutputLimitExceeded() && limited.operation == Read(file,1))
  expect(failed(WAIT files.read(root,100)).kind == NotRegularFile())
  expect(failed(WAIT files.list_directory(file)).kind == NotDirectory())
  expect(failed(WAIT files.remove_directory(root)).kind == DirectoryNotEmpty())
  expect(failed(WAIT files.create_directory(root)).kind == AlreadyExists())
  expect(failed(WAIT files.read(file,134217729)).kind == LimitExceeded())
  confined_missing = failed(WAIT confined.read(root,p("missing"),10))
  expect(confined_missing.kind == NotFound())
  expect(confined_missing.operation == ReadConfined(root,p("missing"),10))
  escape = failed(WAIT confined.read(root,p("escape"),10))
  expect(escape.kind == InvalidArgument())
  link = failed(WAIT confined.read_link(root,p("missing-link")))
  expect(link.operation == ReadLink(root,p("missing-link")))
  expect(link.kind == FilesystemFailure())
  expect(link.native.code == "filesystem_error" && link.native.native_code > 0)
  stage = require(WAIT staging.create(root))
  conflict = failed(WAIT staging.publish(stage,p("file"),CreateOnly()))
  expect(conflict.kind == DestinationConflict() && conflict.operation == StagePublish(p("file"),false))
  expect(require(WAIT staging.path(stage)).display() != "")
  discarded = require(WAIT staging.discard(stage))
  stale = failed(WAIT staging.path(stage))
  expect(stale.kind == InvalidStage() && stale.operation == StagePath())
  expect(stale.render() == "Cannot get the staging directory's path: the staging handle is no longer usable. Use a live stage that has not been published or discarded.")
  expect(failed(WAIT staging.create_directory(stage,p("child"))).operation == StageCreateDirectory(p("child")))
  expect(failed(WAIT staging.write_chunk(stage,p("data"),"secret".encode_utf8(),7)).operation == StageWriteChunk(p("data"),7))
  expect(failed(WAIT staging.create_symlink(stage,p("link"),p("target"))).operation == StageCreateSymlink(p("link"),p("target")))
  expect(failed(WAIT staging.discard(stage)).operation == StageDiscard())
  raw = HostError("future-native-operation","future-code",77)
  future = failed(errors.convert[Unit](Error(raw),Metadata(missing)))
  expect(future.kind == OtherFailure() && future.native == raw)
  expect(failed(errors.convert[Unit](Error(HostError("fs_read","unsupported",0)),Read(file,1))).kind == UnsupportedHost())
  expect(failed(errors.convert[Unit](Error(HostError("fs_read","unsupported_host",0)),Read(file,1))).kind == UnsupportedHost())
  expect(errors.convert[Nat](Ok(42),Metadata(file)) == Ok(42))
  done = require(WAIT console.write_line("filesystem errors passed"))
  ()
}
SOURCE
printf 'filesystem errors passed\n' > "$work/expected"
for mode in ordinary async; do
  if [ "$mode" = async ]; then suffix=/async; effect=async; wait=await; else suffix=; effect=; wait=; fi
  sed -e "s@SUFFIX@$suffix@g" -e "s/EFFECT/$effect/g" -e "s/WAIT/$wait/g" "$work/template.panack" > "$work/main.panack"
  panack run "$work/main.panack" "$work/root" > "$work/output"
  cmp "$work/expected" "$work/output"
  panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
  panack run "$work/main.bc" "$work/root" > "$work/output"
  cmp "$work/expected" "$work/output"
  panack disasm "$work/main.bc" > /dev/null
done
printf '%s\n' 'Filesystem errors: ordinary/async paths, confinement, bounds, staging lifecycle, native preservation, exact rendering and compatibility passed.'
