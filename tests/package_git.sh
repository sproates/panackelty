#!/bin/sh
set -eu
work=$(mktemp -d "$PWD/build/p03b-https-probe.XXXXXX")
pid=
cleanup() { status=$?; if [ "$status" -ne 0 ]; then cat "$work/out" "$work/server.log" "$work/backend.log" 2>/dev/null || :; fi; if [ -n "$pid" ]; then kill "$pid" 2>/dev/null || :; wait "$pid" 2>/dev/null || :; fi; rm -rf "$work"; }
trap cleanup EXIT
trap 'exit 1' HUP INT TERM
mkdir "$work/source" "$work/cache"
export GIT_CONFIG_NOSYSTEM=1 GIT_CONFIG_GLOBAL=/dev/null
/usr/bin/git -C "$work/source" init -q --template=
printf 'hello\n' > "$work/source/data"
mkdir "$work/source/nested"
printf 'nested\n' > "$work/source/nested/value"
ln -s ../data "$work/source/nested/link"
printf 'data text eol=crlf filter=trap\n' > "$work/source/.gitattributes"
/usr/bin/git -C "$work/source" -c core.safecrlf=false add .
/usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm fixture
commit=$(/usr/bin/git -C "$work/source" rev-parse HEAD)
/usr/bin/git clone -q --bare "$work/source" "$work/repo.git"
cat > "$work/certificate.cnf" <<'CONF'
[req]
prompt=no
distinguished_name=dn
x509_extensions=ext
[dn]
CN=localhost
[ext]
subjectAltName=DNS:localhost
basicConstraints=critical,CA:TRUE
keyUsage=critical,keyCertSign,digitalSignature,keyEncipherment
CONF
openssl req -config "$work/certificate.cnf" -x509 -newkey rsa:2048 -nodes -keyout "$work/test.key" -out "$work/test.crt" -days 2 > "$work/cert.log" 2>&1
node tests/package_git_fixture.cjs "$work" > "$work/server.log" 2>&1 &
pid=$!
attempt=0
while [ ! -s "$work/port" ] && [ "$attempt" -lt 100 ]; do
  kill -0 "$pid"
  attempt=$((attempt+1))
  sleep .1
done
port=$(cat "$work/port")
mkdir "$work/hostile"
cat > "$work/hostile/git-remote-https" <<'HELPER'
#!/bin/sh
exit 99
HELPER
chmod +x "$work/hostile/git-remote-https"
mkdir -p "$work/hostile/templates/hooks"
cat > "$work/hostile/trap" <<TRAP
#!/bin/sh
touch "$work/executed-host-hook"
exit 99
TRAP
chmod +x "$work/hostile/trap"
cp "$work/hostile/trap" "$work/hostile/templates/hooks/post-checkout"
cat > "$work/hostile/config" <<CONFIG
[core]
  hooksPath = $work/hostile/templates/hooks
[credential]
  helper = !$work/hostile/trap
[filter "trap"]
  smudge = $work/hostile/trap
  required = true
CONFIG
HOME="$work/hostile" GIT_CONFIG_GLOBAL="$work/hostile/config" GIT_CONFIG_SYSTEM="$work/hostile/config" GIT_TEMPLATE_DIR="$work/hostile/templates" GIT_EXEC_PATH="$work/hostile" GIT_CONFIG_COUNT=1 GIT_CONFIG_KEY_0=url.file:///nonexistent/.insteadOf GIT_CONFIG_VALUE_0="https://localhost:$port/" PANACKELTY_BOOTSTRAP_ROOT="$PWD" bin/panack run tests/fixtures/package_git/acquire.panack "$work/cache" "https://localhost:$port/repo.git" "$commit" "$work/test.crt" /usr/bin/git sha1 inspect > "$work/out"
PANACKELTY_BOOTSTRAP_ROOT="$PWD" bin/panack compile tests/fixtures/package_git/acquire.panack -o "$work/acquire.bc" > "$work/compile.out"
node tests/fixtures/package_git/expect_snapshot.cjs "$work" "https://localhost:$port/repo.git" "$commit"
test ! -e "$work/executed-host-hook"
test -z "$(ls -A "$work/cache")"
printf '%s\n' 'HTTPS exact-commit snapshot and stage cleanup passed.'

run_snapshot() {
  PANACKELTY_BOOTSTRAP_ROOT="$PWD" bin/panack "$work/acquire.bc" "$work/cache" "$1" "$2" "$3" "$4" "$5" "$6" > "$work/out" 2> "$work/err"
}
reject_snapshot() {
  expected=$1; shift
  status=0
  run_snapshot "$@" || status=$?
  test "$status" = 1
  test "$(cat "$work/out")" = "$expected"
  test ! -s "$work/err"
  test -z "$(ls -A "$work/cache")"
}
url="https://localhost:$port/repo.git"
run_snapshot "$url" "$commit" "$work/test.crt" /usr/bin/git sha1 inspect
node tests/fixtures/package_git/expect_snapshot.cjs "$work" "$url" "$commit"
test -z "$(ls -A "$work/cache")"
printf '%s\n' 'PASS saved bytecode acquisition and cleanup'
reject_snapshot 'Git command failed with exit status 128; remote error text withheld to protect credentials' "$url" "$commit" none /usr/bin/git sha1 plain
reject_snapshot 'Git process failed: launch_failed' "$url" "$commit" "$work/test.crt" "$work/no-git" sha1 plain
reject_snapshot 'Git executable must be an absolute path' "$url" "$commit" "$work/test.crt" git sha1 plain
reject_snapshot 'Git command failed with exit status 128; remote error text withheld to protect credentials' "$url" 1111111111111111111111111111111111111111 "$work/test.crt" /usr/bin/git sha1 plain
for invalid in "http://localhost:$port/repo.git" "https://user:secret@localhost:$port/repo.git" "$url?secret=value" "$url#fragment"; do
  reject_snapshot 'Git requires a credential-free HTTPS URL and full commit identity' "$invalid" "$commit" "$work/test.crt" /usr/bin/git sha1 plain
done
printf '%s\n' 'PASS TLS verification, missing Git, exact commit and credential-free URL requirements'
# Every rejected remote tree must discard both private stages.
commit_tree() {
  /usr/bin/git -C "$work/source" add -A
  /usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm "$1"
  bad_commit=$(/usr/bin/git -C "$work/source" rev-parse HEAD)
  /usr/bin/git -C "$work/source" push -q "$work/repo.git" HEAD:refs/heads/fixture
}
ln -s /etc/passwd "$work/source/escape"
commit_tree escape
reject_snapshot 'Git symlink escapes the snapshot or contains a cycle' "$url" "$bad_commit" "$work/test.crt" /usr/bin/git sha1 plain
rm "$work/source/escape"
ln -s loop-two "$work/source/loop-one"
ln -s loop-one "$work/source/loop-two"
commit_tree cycle
reject_snapshot 'Git symlink escapes the snapshot or contains a cycle' "$url" "$bad_commit" "$work/test.crt" /usr/bin/git sha1 plain
rm "$work/source/loop-one" "$work/source/loop-two"
printf 'version https://git-lfs.github.com/spec/v1\noid sha256:0000000000000000000000000000000000000000000000000000000000000000\nsize 1\n' > "$work/source/lfs"
commit_tree lfs
reject_snapshot 'Git LFS content is unsupported' "$url" "$bad_commit" "$work/test.crt" /usr/bin/git sha1 plain
printf 'version https://git-lfs.github.com/spec/v1\r\noid sha256:0000000000000000000000000000000000000000000000000000000000000000\r\nsize 1\r\n' > "$work/source/lfs"
commit_tree lfs-crlf
reject_snapshot 'Git LFS content is unsupported' "$url" "$bad_commit" "$work/test.crt" /usr/bin/git sha1 plain
rm "$work/source/lfs"
/usr/bin/git -C "$work/source" update-index --add --cacheinfo "160000,$commit,submodule"
/usr/bin/git -C "$work/source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm submodule
bad_commit=$(/usr/bin/git -C "$work/source" rev-parse HEAD)
/usr/bin/git -C "$work/source" push -q "$work/repo.git" HEAD:refs/heads/fixture
reject_snapshot 'Git submodules are unsupported' "$url" "$bad_commit" "$work/test.crt" /usr/bin/git sha1 plain
# An earlier exact commit still produces the same snapshot after the branch moves.
run_snapshot "$url" "$commit" "$work/test.crt" /usr/bin/git sha1 inspect
node tests/fixtures/package_git/expect_snapshot.cjs "$work" "$url" "$commit"
printf '%s\n' 'PASS escaping/cyclic links, LFS and submodules fail with cleanup; exact commits survive moved branches'

for route in redirect auth interrupt; do
  reject_snapshot 'Git command failed with exit status 128; remote error text withheld to protect credentials' "https://localhost:$port/$route/repo.git" "$commit" "$work/test.crt" /usr/bin/git sha1 plain
done
printf '%s\n' 'PASS redirects, authentication requests and interrupted transport fail closed'
# SHA-256 repositories exercise the second full-object identity format.
mkdir "$work/sha-source"
/usr/bin/git -C "$work/sha-source" init -q --template= --object-format=sha256
printf 'sha256\n' > "$work/sha-source/data"
/usr/bin/git -C "$work/sha-source" add .
/usr/bin/git -C "$work/sha-source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm sha256
sha_commit=$(/usr/bin/git -C "$work/sha-source" rev-parse HEAD)
/usr/bin/git clone -q --bare "$work/sha-source" "$work/sha.git"
run_snapshot "https://localhost:$port/sha.git" "$sha_commit" "$work/test.crt" /usr/bin/git sha256 plain
test "$(cat "$work/out")" = ok
test -z "$(ls -A "$work/cache")"
printf '%s\n' 'PASS SHA-256 Git repository'

# Copy a binary blob larger than process stdout capacity through bounded chunks.
node - "$work/sha-source/data" <<'LARGE'
const fs=require('fs');
const bytes=Buffer.alloc(17*1024*1024+13);
for(let i=0;i<bytes.length;i++) bytes[i]=i%251;
fs.writeFileSync(process.argv[2],bytes);
LARGE
/usr/bin/git -C "$work/sha-source" add data
/usr/bin/git -C "$work/sha-source" -c user.name=sproates -c user.email=218187+sproates@users.noreply.github.com commit -qm large
large_commit=$(/usr/bin/git -C "$work/sha-source" rev-parse HEAD)
/usr/bin/git -C "$work/sha-source" push -q "$work/sha.git" HEAD:refs/heads/large
run_snapshot "https://localhost:$port/sha.git" "$large_commit" "$work/test.crt" /usr/bin/git sha256 hash
node - "$work" "https://localhost:$port/sha.git" "$large_commit" <<'LARGE'
const fs=require('fs'),crypto=require('crypto'),assert=require('assert');
const [root,url,commit]=process.argv.slice(2);
const len=n=>{const b=Buffer.alloc(8);b.writeBigUInt64BE(BigInt(n));return b;};
const source=crypto.createHash('sha256').update('PANACK-GIT-SOURCE\0v1\0');
for(const field of [url,'sha256',commit]) { const b=Buffer.from(field);source.update(len(b.length)).update(b); }
const bytes=fs.readFileSync(root+'/sha-source/data');
const tree=crypto.createHash('sha256').update('PANACK-PACKAGE-TREE\0v1\0').update('F').update(len(4)).update('data').update(len(bytes.length)).update(bytes).digest('hex');
assert.deepEqual(fs.readFileSync(root+'/out','utf8').trim().split('\n'),[source.digest('hex'),tree]);
LARGE
test -z "$(ls -A "$work/cache")"
printf '%s\n' 'PASS independent digest for 17 MiB binary blob, beyond process output limit'
# Publication is caller-owned and happens only after a complete snapshot succeeds.
mkdir "$work/published"
PANACKELTY_BOOTSTRAP_ROOT="$PWD" bin/panack "$work/acquire.bc" "$work/published" "$url" "$commit" "$work/test.crt" /usr/bin/git sha1 publish > "$work/out"
test "$(cat "$work/out")" = ok
test "$(cat "$work/published/snapshot/data")" = hello
test "$(readlink "$work/published/snapshot/nested/link")" = ../data
status=0
PANACKELTY_BOOTSTRAP_ROOT="$PWD" bin/panack "$work/acquire.bc" "$work/published" "$url" "$bad_commit" "$work/test.crt" /usr/bin/git sha1 publish > "$work/out" || status=$?
test "$status" = 1
test "$(ls -A "$work/published")" = snapshot
test "$(cat "$work/published/snapshot/data")" = hello
test "$(readlink "$work/published/snapshot/nested/link")" = ../data
printf '%s\n' 'PASS publication preserves links; rejected replacement preserves the previous snapshot'

# P03c public manifest, lock and shared-snapshot integration.
sh tests/package_git_integration.sh

# Diagnostics are part of the package acceptance surface on both native platforms.
sh tests/package_diagnostics.sh
