#!/bin/sh
set -eu
work=$(mktemp -d tests/stdlib-nominal.XXXXXX)
trap 'rm -rf "$work"' 0
trap 'exit 1' HUP INT TERM
mkdir -p "$work/bin" "$work/bootstrap" "$work/src"
cp bin/panack "$work/bin/panack"
cp panack-vm "$work/panack-vm"
cp "${PANACK_TEST_COMPILER:-bootstrap/compiler-v9.bc}" "$work/bootstrap/compiler-v9.bc"
cp VERSION "$work/VERSION"
cp -R src/stdlib "$work/src/stdlib"
cat > "$work/src/stdlib/identity_left.panack" <<'SOURCE'
pub enum Outcome { Shared(Nat), Absent }
pub record Packet { value: Nat }
pub pure outcome(): Outcome { Shared(42) }
pub pure packet(): Packet { Packet(42) }
SOURCE
cat > "$work/src/stdlib/identity_right.panack" <<'SOURCE'
pub enum Outcome { Shared(Str, Bool), Absent }
pub record Packet { text: Str, valid: Bool }
pub pure outcome(): Outcome { Shared("right", true) }
pub pure packet(): Packet { Packet("right", true) }
SOURCE
cat > "$work/main.panack" <<'SOURCE'
import stdlib/identity_left as left
import stdlib/identity_right as right
import stdlib/identity_left::{Outcome.Shared as LeftShared, Outcome.Absent as LeftAbsent}
import stdlib/identity_right::{Outcome.Shared as RightShared, Outcome.Absent as RightAbsent}
import stdlib/filesystem as files
import stdlib/filesystem::{FileKind.Directory}
import stdlib/path as path
main(): Void {
  match left.outcome() { LeftShared(n) => print(n), LeftAbsent() => print("wrong") }
  match right.outcome() { RightShared(s,valid) => { print(s); print(valid) }, RightAbsent() => print("wrong") }
  print(left.packet().value)
  print(right.packet().text)
  print(right.packet().valid)
  match files.metadata(path.current()) { Ok(value) => print(value.kind == Directory()), Error(_) => print(false) }
}
SOURCE
# Use the public launcher with its own library; environment overrides are not
# a supported way to replace the library selected by the launcher.
panack() { "$work/bin/panack" "$@"; }
printf '42\nright\ntrue\n42\nright\ntrue\ntrue\n' > "$work/expected"
panack run "$work/main.panack" > "$work/output"
cmp "$work/expected" "$work/output"
panack compile "$work/main.panack" -o "$work/main.bc" > /dev/null
panack run "$work/main.bc" > "$work/output"
cmp "$work/expected" "$work/output"
printf '%s\n' 'Stdlib nominal identities: colliding enum/variant/record names retain distinct payloads and native file kinds remain compatible.'
