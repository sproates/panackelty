#!/bin/sh
# Exercise Git detection and process failures through the public package command.
set -eu
root=$(pwd -P)
work=$(mktemp -d "$root/build/package-git-diagnostics.XXXXXX")
cleanup() { status=$?; if [ "$status" != 0 ]; then cat "$work/out" "$work/err" 2>/dev/null || :; fi; rm -rf "$work"; }
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
mkdir -p "$work/app/src" "$work/bin"
ln -s /usr/bin/dirname "$work/bin/dirname"
node - "$work" "$(cat VERSION)" <<'JS'
const fs=require('fs');const [w,version]=process.argv.slice(2);
fs.writeFileSync(w+'/app/panack.json',JSON.stringify({schema:1,package:'git.app',version:'1.0.0',language:'modules-v1',toolchain:version,source:'src',entry:'main.panack',exports:{},dependencies:{library:{git:{url:'https://example.test/repo.git',ref:'release'}}}}));
fs.writeFileSync(w+'/app/src/main.panack','main(): Void {}\n');
JS
cat > "$work/bin/git" <<SHIM
#!/bin/sh
mode=\$(/bin/cat '$work/mode')
if [ "\$1" = --version ]; then
  case "\$mode" in
    old) printf 'git version 2.28.9\n';;
    malformed-version) printf 'unrecognisable wrapper output\n';;
    *) printf 'git version 2.39.5 (Apple Git-154)\n';;
  esac
  exit 0
fi
printf 'command\n' >> '$work/commands'
case "\$mode" in
  missing) exit 2;;
  empty) exit 0;;
  ambiguous) printf '0123456789012345678901234567890123456789\trefs/heads/release\n0123456789012345678901234567890123456789\trefs/tags/release\n';;
  malformed-ref) printf 'broken record\n';;
  signal) kill -TERM \$\$;;
  *) printf 'secret-from-git-stderr\n' >&2; exit 128;;
esac
SHIM
chmod +x "$work/bin/git"
reject() {
  mode=$1; code=$2; shift 2
  printf '%s\n' "$mode" > "$work/mode"
  status=0
  PATH="$work/bin" "$root/bin/panack" deps lock --project "$work/app/panack.json" > "$work/out" 2> "$work/err" || status=$?
  test "$status" = 1
  test ! -s "$work/out"
  grep -F "error[$code]:" "$work/err" >/dev/null
  grep -F "Dependency name: library" "$work/err" >/dev/null
  ! grep -F 'secret-from-git-stderr' "$work/err"
  test ! -e "$work/app/panack.lock.json"
  test -z "$(find "$work/app/.panack_modules" -mindepth 1 -print)"
  if [ -n "${PANACK_DIAGNOSTICS_RECORD:-}" ]; then
    mkdir -p "$PANACK_DIAGNOSTICS_RECORD"
    node - "$work/err" "$work" "$PANACK_DIAGNOSTICS_RECORD/git-$mode.txt" <<'JS'
const fs=require('fs');const [file,root,out]=process.argv.slice(2);fs.writeFileSync(out,fs.readFileSync(file,'utf8').split(root).join('<workspace>'));
JS
  fi
}
reject old PKG_GIT_VERSION
node tests/package_diagnostic_capture.cjs git-old "$work/err" "$work"
grep -F 'Installed Git version: 2.28.9' "$work/err" >/dev/null
test ! -e "$work/commands"
reject malformed-version PKG_GIT_VERSION_UNKNOWN
test ! -e "$work/commands"
reject missing PKG_GIT_REF_NOT_FOUND
node tests/package_diagnostic_capture.cjs git-missing "$work/err" "$work"
grep -F 'Requested reference: release' "$work/err" >/dev/null
node - "$work/app/panack.json" <<'JS'
const fs=require('fs'),file=process.argv[2],m=JSON.parse(fs.readFileSync(file));delete m.dependencies.library.git.ref;fs.writeFileSync(file,JSON.stringify(m));
JS
reject empty PKG_GIT_REF_NOT_FOUND
grep -F 'configure a default branch' "$work/err" >/dev/null
node - "$work/app/panack.json" <<'JS'
const fs=require('fs'),file=process.argv[2],m=JSON.parse(fs.readFileSync(file));m.dependencies.library.git.ref='release';fs.writeFileSync(file,JSON.stringify(m));
JS
reject ambiguous PKG_GIT_REF_AMBIGUOUS
grep -F 'refs/heads/' "$work/err" >/dev/null
reject malformed-ref PKG_GIT_RESPONSE
reject signal PKG_GIT_INTERRUPTED
grep -F 'Signal: 15' "$work/err" >/dev/null
reject failed PKG_GIT_FETCH
grep -F 'Exit status: 128' "$work/err" >/dev/null
chmod -x "$work/bin/git"
reject launch PKG_GIT_START
rm "$work/bin/git"
reject absent PKG_GIT_REQUIRED
printf '%s\n' 'PASS Git version, reference and process diagnostics; no secrets, lock changes or abandoned stages'
